#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>

#define PI 3.14159

int erodeValue;
int dilateValue;
int epsilonVal;

cv::Mat src;
cv::Mat dst;

int getMinPoint(std::vector<cv::Point>);
void on_trackbar(int, void*)
{
  cv::Mat element = cv::getStructuringElement( cv::MORPH_ELLIPSE,
                                      cv::Size( 2*erodeValue + 1, 2*erodeValue+1 ),
                                      cv::Point( erodeValue, erodeValue ) );
  cv::Mat element1 = cv::getStructuringElement( cv::MORPH_ELLIPSE,
                                          cv::Size( 2*dilateValue + 1, 2*dilateValue+1 ),
                                          cv::Point( dilateValue, dilateValue ) );

  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
  std::vector<cv::Point> emptyContour;
  std::vector<std::vector<cv::Point> > biggestThree (3, emptyContour);

  src.convertTo(dst, -1, 2.7, 0);
  cv::medianBlur(dst, dst, 3);
  cv::Sobel(dst, dst, -1, 1, 1, 7);
  cv::inRange(dst, cv::Scalar(0, 0, 255), cv::Scalar(0, 0, 255), dst);
  cv::dilate(dst, dst, element1);
  cv::erode(dst, dst, element, cv::Point(-1, -1), 2);
  cv::Canny(dst, dst, 0, 255, 3);
  cv::findContours(dst, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cv::Point(0,0));
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

  //cv::Mat superawesomecontourmatrix(dst.size(), CV_8UC3, cv::Scalar(0,0,0));
  //cv::drawContours(superawesomecontourmatrix, contours, largestContourIndex, cv::Scalar(0, 0, 255), 5);
  std::vector<std::vector<cv::Point> > moreContours;
  moreContours.resize(3);
  cv::approxPolyDP(biggestThree[0], moreContours[0], epsilonVal, false);
  cv::approxPolyDP(biggestThree[1], moreContours[1], epsilonVal, false);
  cv::approxPolyDP(biggestThree[2], moreContours[2], epsilonVal, false);
  cv::drawContours(dst, biggestThree, -1, cv::Scalar(100, 100, 100), 2);
  int largestDistance = 0;
  cv::Point startPoint;
  cv::Point endPoint;

  for(int x = 0; x < 3; x++)
  {
    for(int i = 0; i < moreContours[x].size()-1; i++)
    {
      int distance = (moreContours[x][i].x - moreContours[x][i+1].x) + (moreContours[x][i].y - moreContours[x][i+1].y);
      if (distance > largestDistance)
      {
        largestDistance = distance;
        startPoint.x = moreContours[x][i].x;
        startPoint.y = moreContours[x][i].y;
        endPoint.x = moreContours[x][i+1].x;
        endPoint.y = moreContours[x][i+1].y;
      }
    }
  }

  double opposite = endPoint.y-startPoint.y;
  double adjacent = endPoint.x-startPoint.x;
  double angle = -atan(opposite/adjacent)*180/PI;
  std::cout << angle << std::endl;

  int lowestPoint = getMinPoint(biggestThree[0]);
  int lowestPointIndex = 0;
  for(int i = 1; i < 3; i++)
  {
    int minimum = getMinPoint(biggestThree[i]);
    if(lowestPoint < minimum)
    {
      lowestPoint = minimum;
      lowestPointIndex = i;
    }
  }
  //cv::drawContours(dst, biggestThree, lowestPointIndex, cv::Scalar(255, 255, 255), 5);
  cv::drawContours(dst, moreContours, -1, cv::Scalar(255, 255, 255), 2);
  cv::line(dst, startPoint, endPoint, cv::Scalar(200, 200, 200), 6);
  cv::imshow("Sobel Filter", dst);
  cv::resizeWindow("Sobel Filter", 600, 600);
}
int getMinPoint(std::vector<cv::Point> contour)
{
  int lowestPoint = contour[0].y;
  for (int i = 0; i < contour.size(); i++)
  {
    if(contour[i].y > lowestPoint) {
      lowestPoint = contour[i].y;
    }
  }
  return lowestPoint;
}
int main(int argc, char** argv)
{
  src = cv::imread(argv[1]);
  cv::namedWindow("Sobel Filter", 1);
  erodeValue = 10;
  dilateValue = 20;
  epsilonVal = 10;
  cv::createTrackbar("Erode Trackbar", "Sobel Filter", &erodeValue, 20, on_trackbar);
  cv::createTrackbar("Dilate Trackbar", "Sobel Filter", &dilateValue, 30, on_trackbar);
  cv::createTrackbar("Epsilon Trackbar", "Sobel Filter", &epsilonVal, 500, on_trackbar);
  on_trackbar(0, 0);
  cv::waitKey(0);
  return 0;
}
