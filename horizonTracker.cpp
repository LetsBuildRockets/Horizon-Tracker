#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include "easywsclient/easywsclient.hpp"
#include "easywsclient/easywsclient.cpp"
#include <assert.h>
#include <stdio.h>
#include <string>
#include <sstream>

int thresholdValue;
int minLineValue;
int maxLineValue;
cv::Mat frame;

using easywsclient::WebSocket;
static WebSocket::pointer ws = NULL;

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

cv::Mat processVideo(cv::Mat);
std::vector<std::vector<cv::Point> > findBiggestThree(cv::Mat);
double getAngleFromLargestLine(std::vector<std::vector<cv::Point> >);
void transformUsingHough(cv::Mat);
void on_trackbar(int, void*);

void handle_message(const std::string & message)
{
    printf(">>> %s\n", message.c_str());
    if (message == "world") { ws->close(); }
}

int main(int argc, char** argv)
{
    frame  = cv::imread(argv[1]);
    //if(!cap.isOpened())
    //   return -1;
    //ws = WebSocket::from_url("ws://localhost:8126/foo", std::string());
    //assert(ws);
   cv::namedWindow("Lines",1);
   thresholdValue = 85;
   minLineValue = 50;
   maxLineValue = 60;
   cv::createTrackbar("Threshold Trackbar", "Lines", &thresholdValue, 100, on_trackbar);
   cv::createTrackbar("MinLineLength Trackbar", "Lines", &minLineValue, 200, on_trackbar);
   cv::createTrackbar("MaxLineGap Trackbar", "Lines", &maxLineValue, 200, on_trackbar);
   on_trackbar(0, 0);
   //for(;;)
   //{

      // cv::Mat frame;
      // cap >> frame;
    /*   cv::Mat canny = processVideo(frame);
       std::vector<std::vector<cv::Point> > biggestThreeContours = findBiggestThree(canny);
       double angleFromLine = getAngleFromLargestLine(biggestThreeContours); */
    //   std::cout << angleFromLine << std::endl;
      // std::ostringstream strs;
       //strs << angleFromLine;
        //ws->send(strs.str());
        //while (ws->getReadyState() != WebSocket::CLOSED) {
          //ws->poll();
          //ws->dispatch(handle_message);
          //std::cout << "i'm stuck" << std::endl;
        //}
       /*imshow("Horizon Tracker", canny);
       int keyCode = cv::waitKey(30);
       if(keyCode >= 0 && keyCode != 255) {
         std::cout << keyCode << std::endl;
         break;
       }
   }
   delete ws;*/
   cv::waitKey(0);
   return 0;
}
void on_trackbar(int, void*)
{
  cv::Mat canny = processVideo(frame);
  std::vector<std::vector<cv::Point> > biggestThreeContours = findBiggestThree(canny);
  double angleFromLine = getAngleFromLargestLine(biggestThreeContours);
}
cv::Mat processVideo(cv::Mat src)
{
  cv::Mat dst;
  src.convertTo(dst, -1, contrastValue, 0);
  cv::medianBlur(dst, dst, 11);
  cv::Sobel(dst, dst, -1, 1, 1, 7);
  cv::inRange(dst, cv::Scalar(0, 0, 255), cv::Scalar(0, 0, 255), dst);
  cv::dilate(dst, dst, dilateKernel);
  cv::erode(dst, dst, erodeKernel, cv::Point(-1, -1), 2);
  cv::Canny(dst, dst, 0, 255, 3);
  transformUsingHough(dst);
  return dst;
}

void transformUsingHough(cv::Mat img)
{
  std::vector<cv::Vec4i> lines;
  cv::Mat color_dst;
  cv::cvtColor( img, color_dst, CV_GRAY2BGR );
  cv::HoughLinesP(img, lines, 1, CV_PI/180, thresholdValue, minLineValue, maxLineValue);
  for(int i = 0; i < lines.size(); i++ )
    {
       cv::Scalar clr(rand() % 255,rand() % 255,rand() % 255);
        cv::line( color_dst, cv::Point(lines[i][0], lines[i][1]), cv::Point(lines[i][2], lines[i][3]), clr, 1, 8 );
    }
    cv::imshow("Lines", color_dst);
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
