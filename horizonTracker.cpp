#include "horizonTracker.h"
#include "uart.cpp"
#include "i2c.cpp"

bool enableDelay = false;

const cv::Point negOne(-1, -1);
const cv::Size imgSize(320/2, 240/2);
const cv::Scalar black(0);

const cv::Mat erodeKernel = cv::getStructuringElement( cv::MORPH_RECT ,
                                    cv::Size( 2*erodeValue + 1, 2*erodeValue+1 ),
                                    cv::Point( erodeValue, erodeValue ) );
const cv::Mat dilateKernel = cv::getStructuringElement( cv::MORPH_RECT ,
                                        cv::Size( 2*dilateValue + 1, 2*dilateValue+1 ),
                                        cv::Point( dilateValue, dilateValue ) );


const cv::Mat lowRed(imgSize, CV_8UC3, cv::Scalar(0,0,200));
const cv::Mat red(imgSize, CV_8UC3, cv::Scalar(0,0,255));

void processVideo(cv::Mat&, cv::Mat&);
std::vector<std::vector<cv::Point> > findBiggestThree(cv::Mat&);
double getAngleFromLargestLine(std::vector<std::vector<cv::Point> >,cv::Mat &);
double getTime();
void range(cv::Mat &, cv::Mat&, int);

std::string currentfolderframes;

const int fourthHeight = imgSize.height / 4;
int framesCount = 0;
const int frameRate = 15;

/*void handle_message(const std::string & message)
{
    printf(">>> %s\n", message.c_str());
    if(message == "world") { ws->close(); }
}*/

int main(int argc, char** argv) {
  if(argc > 1)
  {
    if(std::string(argv[1]).compare("--USE-STILL") == 0)
    {
      mode = USESTILL;
      if(!(argc > 2)){
        printf("Please specify an image path\n");
        return -1;
      }
    }
    else
    {
      if(std::string(argv[1]).compare("--MASTER") == 0)
      {
         mode = MASTER;
         enableDelay = true;
      }
      else if(std::string(argv[1]).compare("--SERF") == 0)
      {
        mode = SERF;
        enableDelay = true;
      }
      if(argc > 2)
      {
        if(std::string(argv[2]).compare("--NO-DELAY") == 0)
        {
          enableDelay = false;
        }
        else
        {
          printf("What? I don't understand your second flag...\n");
          return -1;
        }
      }
    }
  }

  if(mode == UNDEF)
  {
    printf("Please specify a mode:\n --MASTER\n --MASTER --NO-DELAY\n --SERF\n --SERF --NO-DELAY\n --USE-STILL\n");
    return -1;
  }

  if(mode == MASTER)
  {
    openi2c();
    compassInit();
  }

  UARTInit();

  char const * currentfolder = std::to_string(getTime()).c_str();
  currentfolderframes = std::string(currentfolder)+"/frames";
  mkdir(currentfolder, 0777);
  mkdir(currentfolderframes.c_str(), 0777);
  cv::Mat frame;
  std::ofstream horizonTrackerData;

    if(mode == MASTER)
    {
      horizonTrackerData.open(std::string(currentfolder)+"/frame.txt");
      horizonTrackerData << "frame" << "\t" << "time" << "\t" << "angle1" << "\t" << "angle2" << "\t" << "compass x" << "\t" << "compass y" << "\t" << "compass z" << "\n";
      horizonTrackerData.flush();
    }
    else if(mode == SERF)
    {
      horizonTrackerData.open(std::string(currentfolder)+"/frame.txt");
      horizonTrackerData << "frame" << "\t" << "time" << "\t" << "angle2" << "\n";
      horizonTrackerData.flush();
    }


  cv::VideoCapture cap(0);
  if(mode == USESTILL)
  {
    frame = cv::imread(argv[2]);
  }
  else
  {
    if(!cap.isOpened()) {
     std::cout << "yo this didn't open" << std::endl;
       return -1;
    }
    cap.set(CV_CAP_PROP_FPS, frameRate);
    cap >> frame;
  }

  std::cout << "Ready! Waiting for takeoff!" << std::endl;
  #ifdef __ARMEL__
    if(enableDelay == true)
    {
      if(mode == MASTER )
      {
        if(!bcm2835_init())
        {
           std::cout << "Unable to init GPIO" << std::endl;
           return -1;
        }

        bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
        while(bcm2835_gpio_lev(PIN) == LOW)
        {
          usleep(1000);
        }
        writeStartByte();
      }
      else if(mode == SERF)
      {
        while (!(readStartByte() > 0))
        {
          usleep(1000);
        }
      }
    }
  #endif
  std::cout << "LAUNCH DETECTED!" << std::endl;


  // ws = WebSocket::from_url("ws://localhost:8126/foo", std::string());
  //assert(ws);
//  cv::namedWindow("Horizon Tracker",1);

  for(;;) {
    printf("framesCount: %d\n", framesCount);
    if(mode != USESTILL)
    {
      cap >> frame;
    }

    cv::resize(frame, frame, imgSize, 0, 0, cv::INTER_CUBIC);
    short x, y, z;
    x=0;
    y=0;
    z=0;
    if(mode == MASTER)
    {
       readCompass(x, y, z);
    }
    cv::Mat canny;
    processVideo(frame, canny);
    std::vector<std::vector<cv::Point> > biggestThreeContours = findBiggestThree(canny);
    double angleFromLine = getAngleFromLargestLine(biggestThreeContours, canny);
    //imshow("Horizon Tracker", canny);
    if(framesCount % 10 == 0)
    {
      cv::imwrite(currentfolderframes+"/cameraImage" + std::to_string(framesCount) + ".jpg", frame);
      cv::imwrite(currentfolderframes+"/canny" + std::to_string(framesCount) + ".jpg", canny);
    }

    if(mode == MASTER)
    {
      //double serfsAngle;
      //readAngleData(serfsAngle);
      horizonTrackerData << framesCount << "\t" << (framesCount*1000)/frameRate << "\t" << angleFromLine << "\t"  << "broken"  << "\t"  << x/2048.0*360 << "\t" << y/2048.0*360 << "\t" << z/2048.0*360 << "\n";
      horizonTrackerData.flush();
    }
    else if(mode == SERF)
    {
    horizonTrackerData << framesCount << "\t" << (framesCount*1000)/frameRate << "\t" << angleFromLine << "\n";
    horizonTrackerData.flush();
    }
    //std::cout << angleFromLine << std::endl;
    //std::ostringstream strs;
    //strs << angleFromLine;
    // ws->send(strs.str());
    //while (ws->getReadyState() != WebSocket::CLOSED) {
    //    ws->poll();
    //   ws->dispatch(handle_message);
    //std::cout << "i'm stuck" << std::endl;
    //}
    framesCount++;
    if(mode == USESTILL)
    {
      int keyCode = cv::waitKey(10);
      if(keyCode >= 0 && keyCode != 255) {
        return 0;
      }
    }
  }
  //delete ws;
  return 0;
}

void range(cv::Mat & src, cv::Mat & canny, int offset) {
  cv::Mat dst;
  cv::Mat hsv;
//  src.convertTo(dst, -1, contrastValue, 0);
//  cv::medianBlur( dst, dst, 3 );
  cvtColor(src, hsv, CV_BGR2HSV);
  unsigned char inc = 200;
  for(int i = 0; i < hsv.rows; i++) {
    cv::Vec3b* pixel = hsv.ptr<cv::Vec3b>(i);
    for(int j = 0; j < hsv.cols; j++) {
      if(pixel[j][1] <= 255-inc)
      {
        pixel[j][1] = pixel[j][1] + inc;
      }
      else
      {
        pixel[j][1] = 255;
      }
    }
  }
    cvtColor(hsv, dst, CV_HSV2BGR);
  cv::blur(dst, dst, cv::Size(3, 3));
//  imshow("Horizon Tracker", dst);
  cv::Sobel(dst, dst, -1, 1, 1, 7);
  for(int i = 0; i < dst.rows; i++)
  {
    cv::Vec3b* pixel = dst.ptr<cv::Vec3b>(i);
    uchar* newPixel = canny.ptr<uchar>(i+offset);
    for(int j = 0; j < dst.cols; j++) {
      if(pixel[j][0] > 200) {
        //if((!pixel[j][2]) && (!pixel[j][1])) {
          newPixel[j] = 255;
      //  }
      }
    }
  }
  //imshow("Horizon Tracker", canny);
}
void processVideo(cv::Mat & src, cv::Mat& dst)
{
  // printf("src: %x,  dst: %x  ", &src, &dst);
  cv::Mat canny(imgSize, CV_8UC1, black);
  //cv::Mat img1 = cv::Mat(src, cv::Rect(0, 0*fourthHeight, imgSize.width, fourthHeight));
  //cv::Mat img2 = cv::Mat(src, cv::Rect(0, 1*fourthHeight, imgSize.width, fourthHeight));
  //cv::Mat img3 = cv::Mat(src, cv::Rect(0, 2*fourthHeight, imgSize.width, fourthHeight));
  //cv::Mat img4 = cv::Mat(src, cv::Rect(0, 3*fourthHeight, imgSize.width, fourthHeight));

  //std::thread tOne (range, std::ref(img1), std::ref(canny), 0*fourthHeight);
  //std::thread tTwo (range, std::ref(img2), std::ref(canny), 1*fourthHeight);
  //std::thread tThree (range, std::ref(img3), std::ref(canny), 2*fourthHeight);
  //std::thread tFour (range, std::ref(img4), std::ref(canny), 3*fourthHeight);
  //tOne.join();
  //tTwo.join();
  //tThree.join();
  //tFour.join();


  range(src, canny, 0);
  //imshow("Horizon Tracker", canny);
  if(framesCount % 10 == 0)
  {
     cv::imwrite(currentfolderframes + "/inter" + std::to_string(framesCount) + ".jpg", canny);
  }

  cv::dilate(canny, canny, dilateKernel);
  cv::erode(canny, canny, erodeKernel, negOne, 1);
  cv::Canny(canny, dst, 0, 255, 3);

}

std::vector<std::vector<cv::Point> > findBiggestThree(cv::Mat & cannyMatrix)
{
  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
  std::vector<cv::Point> emptyContour;
  std::vector<std::vector<cv::Point> > biggestThree (3, emptyContour);
  cv::findContours(cannyMatrix, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cv::Point(0,0));
  for (int i = 0; i < contours.size(); i++)
  {
    if(contours[i].size() > biggestThree[2].size())
    {
      if(contours[i].size() > biggestThree[1].size())
      {
        if(contours[i].size() > biggestThree[0].size())
        {
          biggestThree.insert(biggestThree.begin()+0, contours[i]);
          biggestThree.erase(biggestThree.begin()+4);
        }
        else
        {
        biggestThree.insert(biggestThree.begin()+1, contours[i]);
        biggestThree.erase(biggestThree.begin()+4);
        }
      }
      else
      {
        biggestThree.insert(biggestThree.begin()+2, contours[i]);
        biggestThree.erase(biggestThree.begin()+4);
      }
    }
  }
  return biggestThree;
}

double getAngleFromLargestLine(std::vector<std::vector<cv::Point> > biggestThree, cv::Mat & dst)
{
  int numberofContours = 0;
  std::vector<std::vector<cv::Point> > approximatedContours;
  approximatedContours.resize(3);
  for(int i = 0; i < 3; i++) {
    if(biggestThree[i].size() > 0 )
    {
    numberofContours++;
    cv::approxPolyDP(biggestThree[i], approximatedContours[i], epsilonValue, false);
    }
  }
  //cv::drawContours(dst, biggestThree, -1, cv::Scalar(100, 100, 100), 2);
  int largestDistance = 0;
  cv::Point startPoint(0,0);
  cv::Point endPoint(0,0);

  for(int x = 0; x < numberofContours; x++)
  {
    for(int i = 0; i < approximatedContours[x].size()-1; i++)
    {
      int distance = pow((approximatedContours[x][i].x - approximatedContours[x][i+1].x),2) + pow((approximatedContours[x][i].y - approximatedContours[x][i+1].y),2);
      if(distance > largestDistance)
      {
        largestDistance = distance;
        startPoint.x = approximatedContours[x][i].x;
        startPoint.y = approximatedContours[x][i].y;
        endPoint.x = approximatedContours[x][i+1].x;
        endPoint.y = approximatedContours[x][i+1].y;
      }
    }
  }
  cv::line(dst, startPoint, endPoint, cv::Scalar(255,255,255),3);
  double opposite = endPoint.y-startPoint.y;
  double adjacent = endPoint.x-startPoint.x;
  double angle = -atan(opposite/adjacent)*180/PI;
  return angle;
}

double getTime()
{
  gettimeofday(&tp, NULL);
  double ms = (long long)tp.tv_sec * 1000 + (long long)tp.tv_usec / 1000.0;
  return ms;
}
