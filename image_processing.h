#pragma once
#include <iostream>
#include "opencv2/opencv.hpp"

void InRange(cv::Mat src, cv::Mat dst, int l_h, int l_s, int l_v, int u_h, int u_s, int u_v);


void Mean_Shift(cv::Mat& bp, cv::Rect& roi, int maxCount, double epsilon, cv::Point& Center, cv::Point& gCenter);

void Hist(cv::Mat& src, const int* ch, const float** ranges, float* h_);

void Back_Project(cv::Mat& hsv, const int* ch, cv::Mat& Histogram, cv::Mat& bp);