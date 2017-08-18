#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef _HORIZON_TRACKER_H_
#define _HORIZON_TRACKER_H_
#ifdef __ARMEL_
  #include <bcm2835.h>
  #define PIN RPI_BPLUS_GPIO_J8_37
#endif

struct timeval tp;

#define PI 3.14159
#define contrastValue 2.7
#define erodeValue 8
#define dilateValue 10
#define epsilonValue 10

enum Mode { UNDEF, SERF, MASTER, USESTILL };
Mode mode = UNDEF;

#endif
