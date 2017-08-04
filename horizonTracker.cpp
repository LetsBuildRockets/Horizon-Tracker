#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <thread>
//#include "easywsclient/easywsclient.hpp"
//#include "easywsclient/easywsclient.cpp"
#include <assert.h>
#include <stdio.h>
#include <string>
#include <sstream>

struct timeval tp;
//using easywsclient::WebSocket;
//static WebSocket::pointer ws = NULL;

#define PI 3.14159
#define contrastValue 2.7
#define erodeValue 8
#define dilateValue 10
#define epsilonValue 10

const cv::Point negOne(-1, -1);
const cv::Size imgSize(320, 240);
const cv::Scalar black(0);

const cv::Mat erodeKernel = cv::getStructuringElement( cv::MORPH_RECT ,
                                    cv::Size( 2*erodeValue + 1, 2*erodeValue+1 ),
                                    cv::Point( erodeValue, erodeValue ) );
const cv::Mat dilateKernel = cv::getStructuringElement( cv::MORPH_RECT ,
                                        cv::Size( 2*dilateValue + 1, 2*dilateValue+1 ),
                                        cv::Point( dilateValue, dilateValue ) );


const cv::Mat lowRed(imgSize, CV_8UC3, cv::Scalar(0,0,200));
const cv::Mat red(imgSize, CV_8UC3, cv::Scalar(0,0,255));

void processVideo(cv::Mat, cv::Mat&);
std::vector<std::vector<cv::Point> > findBiggestThree(cv::Mat&);
double getAngleFromLargestLine(std::vector<std::vector<cv::Point> >,cv::Mat &);
long getTime();
void range(cv::Mat &, cv::Mat&, int, int);


int endOne = imgSize.height >> 2;
int endTwo = imgSize.height >> 1;
int endThree = 3 * imgSize.height >> 2;

/*void handle_message(const std::string & message)
{
    printf(">>> %s\n", message.c_str());
    if (message == "world") { ws->close(); }
}*/

int main(int argc, char** argv) {
  /*cv::VideoCapture cap(0);
  if(!cap.isOpened()) {
   std::cout << "yo this didn't open" << std::endl;
     return -1;
  }*/
  // ws = WebSocket::from_url("ws://localhost:8126/foo", std::string());
  //assert(ws);
  long lasttime = getTime();
  float totalFPS = 0;
  int framesCount = 0;
  //cv::namedWindow("Horizon Tracker",1);
  for(;;) {
      cv::Mat frame;
      frame = cv::imread(argv[1]);
    // cap >> frame;
      cv::resize(frame, frame, imgSize, 0, 0, cv::INTER_CUBIC);
      cv::Mat canny;
      processVideo(frame, canny);
  //     std::cout << "broken canny" << std::endl;
     std::vector<std::vector<cv::Point> > biggestThreeContours = findBiggestThree(canny);
     double angleFromLine = getAngleFromLargestLine(biggestThreeContours, canny);
     //imshow("Horizon Tracker", canny);
     //std::cout << angleFromLine << std::endl;
     //std::ostringstream strs;
     //strs << angleFromLine;
     // ws->send(strs.str());
      //while (ws->getReadyState() != WebSocket::CLOSED) {
    //    ws->poll();
     //   ws->dispatch(handle_message);
        //std::cout << "i'm stuck" << std::endl;
      //}
    long now = getTime();
    float localFPS = 1000.0/(now - lasttime);
    framesCount++;
    totalFPS += localFPS;
    printf("fps: %f\n", totalFPS/framesCount);
    lasttime = now;
    /*int keyCode = cv::waitKey(1);
    if(keyCode >= 0 && keyCode != 255) {
      return 0;
    }*/
  }
  //delete ws;
  return 0;
}

void range(cv::Mat & dst, cv::Mat & canny, int start, int end) {
  for(int i = start; i < end; i++) {
    cv::Vec3b* pixel = dst.ptr<cv::Vec3b>(i);
    uchar* newPixel = canny.ptr<uchar>(i);
    for(int j = 0; j < dst.cols; j++) {
      if(pixel[j][2] > 200) {
        if((!pixel[j][0]) && (!pixel[j][1])) {
          newPixel[j] = 255;
        }
      }
    }
  }
}
void processVideo(cv::Mat src, cv::Mat& dst)
{
  cv::Mat canny(imgSize, CV_8UC1, black);
  src.convertTo(dst, -1, contrastValue, 0);
  cv::medianBlur(dst, dst, 3);
  cv::Sobel(dst, dst, -1, 1, 1, 7);

  /*std::thread tOne (range, std::ref(dst), std::ref(canny), 0, endOne);
  std::thread tTwo (range, std::ref(dst), std::ref(canny), endOne+1, endTwo);
  std::thread tThree (range, std::ref(dst), std::ref(canny), endTwo+1, endThree);
  std::thread tFour (range, std::ref(dst), std::ref(canny), endThree+1, dst.rows);
  tOne.join();
  tTwo.join();
  tThree.join();
  tFour.join();*/
  //range(dst, canny, 0, dst.rows);
  cv::inRange(dst, lowRed, red, canny);
  cv::dilate(canny, canny, dilateKernel);
  cv::erode(canny, canny, erodeKernel, negOne, 2);
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
        else {
        biggestThree.insert(biggestThree.begin()+1, contours[i]);
        biggestThree.erase(biggestThree.begin()+4);
        }
      }
        else {
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
    if(biggestThree[i].size() > 0 ) {
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
      int distance = (approximatedContours[x][i].x - approximatedContours[x][i+1].x) + (approximatedContours[x][i].y - approximatedContours[x][i+1].y);
      if (distance > largestDistance)
      {
        largestDistance = distance;
        startPoint.x = approximatedContours[x][i].x;
        startPoint.y = approximatedContours[x][i].y;
        endPoint.x = approximatedContours[x][i+1].x;
        endPoint.y = approximatedContours[x][i+1].y;
      }
    }
  }
  //cv::line(dst, startPoint, endPoint, cv::Scalar(255,255,255),3);
  double opposite = endPoint.y-startPoint.y;
  double adjacent = endPoint.x-startPoint.x;
  double angle = -atan(opposite/adjacent)*180/PI;
  return angle;
}

long getTime(){
  gettimeofday(&tp, NULL);
  long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
  return ms;
}
