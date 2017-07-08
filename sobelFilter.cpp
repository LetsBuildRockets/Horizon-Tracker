#include <opencv2/opencv.hpp>

int erodeValue;
int dilateValue;
int epsilonVal;

cv::Mat src;
cv::Mat dst;

void on_trackbar(int, void*)
{
  cv::Mat element = cv::getStructuringElement( cv::MORPH_ELLIPSE,
                                      cv::Size( 2*erodeValue + 1, 2*erodeValue+1 ),
                                      cv::Point( erodeValue, erodeValue ) );
  cv::Mat element1 = cv::getStructuringElement( cv::MORPH_ELLIPSE,
                                          cv::Size( 2*dilateValue + 1, 2*dilateValue+1 ),
                                          cv::Point( dilateValue, dilateValue ) );

  src.convertTo(dst, -1, 2.7, 0);
  cv::Sobel(dst, dst, -1, 1, 1, 7);
  cv::inRange(dst, cv::Scalar(0, 0, 255), cv::Scalar(0, 0, 255), dst);
  cv::dilate(dst, dst, element1);
  cv::erode(dst, dst, element, cv::Point(-1, -1), 2);
  cv::approxPolyDP(dst, dst, epsilonVal, true);
  cv::imshow("Sobel Filter", dst);
  cv::resizeWindow("Sobel Filter", 600, 600);
}

int main(int argc, char** argv)
{
  src = cv::imread("img.jpg");
  cv::namedWindow("Sobel Filter", 1);
  erodeValue = 13;
  dilateValue = 17;
  epsilonVal = 100;
  cv::createTrackbar("Erode Trackbar", "Sobel Filter", &erodeValue, 20, on_trackbar);
  cv::createTrackbar("Dilate Trackbar", "Sobel Filter", &dilateValue, 20, on_trackbar);
  cv::createTrackbar("Epsilon Trackbar", "Sobel Filter", &epsilonVal, 500, on_trackbar);
  on_trackbar(0, 0);
  cv::waitKey(0);
  return 0;
}
