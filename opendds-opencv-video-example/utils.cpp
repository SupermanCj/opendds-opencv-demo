
#include <iostream>
#include "VideoC.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include "utils.h"
#ifdef linux
#include <unistd.h>
#endif

using namespace std;
using namespace cv;

typedef unsigned char byte;
const int scale = 8;

byte* matToBytes(Mat image) {
	int size = image.rows * image.cols * image.channels();
	byte* bytes = new byte[size];
	std::memcpy(bytes, image.data, size * sizeof(byte));
	return bytes;
}

Mat bytesToMat(byte* bytes, int cols, int rows, int channels) {
	int nType;
	switch (channels)
	{
	case 0:
		nType = CV_8UC1;
		break;
	case 8:
		nType = CV_8UC2;
		break;
	case 16:
		nType = CV_8UC3;
		break;
	case 24:
		nType = CV_8UC4;
		break;
	default:
		nType = CV_8UC3;
		break;
	}
	return Mat(rows, cols, nType, bytes).clone();

}


bool setMatToFrame(Mat& image, Video::Frame& frame) {
	Mat mat;
	resize(image, mat, Size(image.cols / scale, image.rows / scale), 0.0, 0.0);
	frame.rows = mat.rows;
	frame.cols = mat.cols;
	frame.channels = mat.channels();
	unsigned int size = frame.rows * frame.cols * frame.channels;
	frame.frame_bytes.length(size);
	byte* bytes = matToBytes(mat);
	/*
	for (int i = 0; i < frame.frame_bytes.length(), i++) {
		frame.frame_bytes[i] = bytes[i];
	}
	*/
	memcpy(frame.frame_bytes.get_buffer(), bytes, size);
	delete[] bytes;
	return true;
}

Mat getMatFromFrame(Video::Frame& frame) {
	Mat mat = bytesToMat(frame.frame_bytes.get_buffer(), frame.cols, frame.rows, frame.channels);
	resize(mat,
		mat, Size(mat.cols * scale, mat.rows * scale), 0.0, 0.0);
	return mat;
}


ostream& printOctetSeq(Video::OctetSeq seq) {
	char* str = new char[seq.length() + 1];
	for (int i = 0; i < (long)seq.length(); i++) {
		str[i] = seq[i];
	}
	str[seq.length()] = '\0';
	std::cout << str;
	delete[] str;
	return cout;
}

void msleep(unsigned long milisecond)
{
#if defined _WIN32||WIN32||_WIN64||WIN64
	Sleep(milisecond);
#elif defined linux||_linux||__linux__
	usleep(milisecond*1000)
#endif
}


