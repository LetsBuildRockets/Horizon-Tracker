#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
//#include "easywsclient/easywsclient.hpp"
//#include "easywsclient/easywsclient.cpp"
#include <assert.h>
#include <stdio.h>
#include <string>
#include <sstream>

//using easywsclient::WebSocket;
//static WebSocket::pointer ws = NULL;

#define PI 3.14159
#define contrastValue 2.7
#define erodeValue 10
#define dilateValue 20
#define epsilonValue 10

cv::Mat erodeKernel = cv::getStructuringElement( cv::MORPH_ELLIPSE,
                                    cv::Size( 2*erodeValue + 1, 2*erodeValue+1 ),
                                    cv::Point( erodeValue, erodeValue ) );
cv::Mat dilateKernel = cv::getStructuringElement( cv::MORPH_ELLIPSE,
                                        cv::Size( 2*dilateValue + 1, 2*dilateValue+1 ),
                                        cv::Point( dilateValue, dilateValue ) );

void processVideo(cv::Mat, cv::Mat&);
std::vector<std::vector<cv::Point> > findBiggestThree(cv::Mat);
double getAngleFromLargestLine(std::vector<std::vector<cv::Point> >);

/*void handle_message(const std::string & message)
{
    printf(">>> %s\n", message.c_str());
    if (message == "world") { ws->close(); }
}*/

int main(/*int argc, char** argv*/)
{
    std::cout << "hi there" << std::endl;
    cv::VideoCapture cap(0);
    if(!cap.isOpened()) {
	     std::cout << "yo this didn't open" << std::endl;
 	     return -1;
	    }
   // ws = WebSocket::from_url("ws://localhost:8126/foo", std::string());
    //assert(ws);
  //cv::namedWindow("Horizon Tracker",1);
   for(;;)
   {
        cv::Mat frame;
      //  frame = cv::imread(argv[1]);
        cv::resize(frame, frame, cv::Size(640, 360), 0, 0, cv::INTER_CUBIC);
        cap >> frame;
        std::cout << "Width : " << frame.size().width << std::endl;
        std::cout << "Height: " << frame.size().height << std::endl;
        cv::Mat canny;
        processVideo(frame, canny);
      //  imshow("Horizon Tracker", canny);
  //     std::cout << "broken canny" << std::endl;
    //   std::vector<std::vector<cv::Point> > biggestThreeContours = findBiggestThree(canny);
    //   std::cout << "broken contours" << std::endl;
       /*if(biggestThreeContours.size() >= 3) {
           double angleFromLine = getAngleFromLargestLine(biggestThreeContours);
           std::cout << angleFromLine << std::endl;
       }*/
       std::cout << "hi" << std::endl;
       //std::ostringstream strs;
       //strs << angleFromLine;
       // ws->send(strs.str());
        //while (ws->getReadyState() != WebSocket::CLOSED) {
      //    ws->poll();
       //   ws->dispatch(handle_message);
          //std::cout << "i'm stuck" << std::endl;
        //}
      int keyCode = cv::waitKey(0);
       if(keyCode >= 0 && keyCode != 255) {
         std::cout << keyCode << std::endl;
         break;
       }
   }
   //delete ws;
   return 0;
}

void processVideo(cv::Mat src, cv::Mat& dst)
{
  src.convertTo(dst, -1, contrastValue, 0);
  std::cout << "contrast" << std::endl;
  cv::medianBlur(dst, dst, 3);
  std::cout << "blur" << std::endl;
  cv::Sobel(dst, dst, -1, 1, 1, 7);
  std::cout << "sobel" << std::endl;
  cv::inRange(dst, cv::Scalar(0, 0, 255), cv::Scalar(0, 0, 255), dst);
  std::cout << "inrange" << std::endl;
  cv::dilate(dst, dst, dilateKernel);
  std::cout << "dilate" << std::endl;
  cv::erode(dst, dst, erodeKernel, cv::Point(-1, -1), 2);
  std::cout << "erode" << std::endl;
  cv::Canny(dst, dst, 0, 255, 3);
  std::cout << "canny" << std::endl;
}

std::vector<std::vector<cv::Point> > findBiggestThree(cv::Mat cannyMatrix)
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
double getAngleFromLargestLine(std::vector<std::vector<cv::Point> > biggestThree)
{
  std::vector<std::vector<cv::Point> > approximatedContours;
  approximatedContours.resize(3);
  cv::approxPolyDP(biggestThree[0], approximatedContours[0], epsilonValue, false);
  cv::approxPolyDP(biggestThree[1], approximatedContours[1], epsilonValue, false);
  cv::approxPolyDP(biggestThree[2], approximatedContours[2], epsilonValue, false);
  //cv::drawContours(dst, biggestThree, -1, cv::Scalar(100, 100, 100), 2);
  int largestDistance = 0;
  cv::Point startPoint;
  cv::Point endPoint;

  for(int x = 0; x < 3; x++)
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
  double opposite = endPoint.y-startPoint.y;
  double adjacent = endPoint.x-startPoint.x;
  double angle = -atan(opposite/adjacent)*180/PI;
  return angle;
}
