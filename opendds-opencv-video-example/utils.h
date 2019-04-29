#pragma once


#include "VideoC.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

using namespace cv;

typedef unsigned char byte;


byte* matToBytes(Mat image);
Mat bytesToMat(byte* bytes, int cols, int rows, int channels);
bool setMatToFrame(Mat& mat, Video::Frame& frame);
Mat getMatFromFrame(Video::Frame& frame);

//test
ostream& printOctetSeq(Video::OctetSeq seq);

void msleep(unsigned long milisecond);

