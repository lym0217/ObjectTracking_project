#include "image_processing.h"

using namespace cv;
using namespace std;

void InRange(Mat src, Mat dst, int l_h, int l_s, int l_v, int u_h, int u_s, int u_v) {

	vector<Mat> hsv_planes;
	split(src, hsv_planes);
	Mat h = hsv_planes[0]; // H channel
	Mat s = hsv_planes[1]; // S channel
	Mat v = hsv_planes[2]; // V channel

	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {

			if ((h.data[i * h.cols + j] >= l_h && h.data[i * h.cols + j] <= u_h)
				&& (s.data[i * s.cols + j] >= l_s && s.data[i * s.cols + j] <= u_s)
				&& (v.data[i * v.cols + j] >= l_v && v.data[i * v.cols + j] <= u_v))
			{
				dst.data[i * dst.cols + j] = 255;
			}

			else dst.data[i * dst.cols + j] = 0;
		}
	}
}


void Mean_Shift(cv::Mat& bp, cv::Rect& roi, int maxCount, double epsilon, cv::Point& Center, cv::Point& gCenter) {
	unsigned char* searchWindow_Data = new unsigned char[roi.width * roi.height]; //Search Window

	Moments m;
	Mat searchWindow;
	int i;

	for (i = 0; i < maxCount; i++) {

		for (int y = 0; y < roi.height; y++) {
			for (int x = 0; x < roi.width; x++) {
				searchWindow_Data[roi.width * y + x] = bp.data[bp.cols * (roi.y + y) + (roi.x + x)];
			}
		}

		searchWindow = Mat(Size(roi.width, roi.height), CV_8UC1, searchWindow_Data);

		m = moments(searchWindow, false);

		int curRect_x = roi.x;
		int curRect_y = roi.y;

		float f_gCenter_x = m.m10 / m.m00;
		float f_gCenter_y = m.m01 / m.m00;
		float f_Center_x = roi.width / 2.0f;
		float f_Center_y = roi.height / 2.0f;
		float f_dx = f_gCenter_x - f_Center_x;
		float f_dy = f_gCenter_y - f_Center_y;

		if (f_dx * f_dx + f_dy * f_dy < epsilon) 
			break;

		int gCenter_x = cvRound(m.m10 / m.m00);     // Window 좌표계 무게중심 x
		int gCenter_y = cvRound(m.m01 / m.m00);     // Window 좌표계 무게중심 y
		int Center_x = cvRound(roi.width / 2);      // Window 좌표계, Window중심 x
		int Center_y = cvRound(roi.height / 2);     // Window 좌표계, Window중심 y
		int dx = gCenter_x - Center_x;
		int dy = gCenter_y - Center_y;


		// dx, dy -> (Window중심->무게중심) 벡터

		gCenter = Point(curRect_x + gCenter_x, curRect_y + gCenter_y);
		Center = Point(curRect_x + Center_x, curRect_y + Center_y);

		roi.x = max(1, min(bp.cols - roi.width, curRect_x + dx));
		roi.y = max(1, min(bp.rows - roi.height, curRect_y + dy));
	}

	delete[] searchWindow_Data;
}

void Hist(Mat& src, const int* ch, const float** ranges, float* h_) {
	vector<Mat> hsv_planes;
	split(src, hsv_planes);

	for (int y = 0; y < src.rows; y++) {
		for (int x = 0; x < src.cols; x++) {

			if ((hsv_planes[ch[0]].data[hsv_planes[ch[0]].cols * y + x] >= ranges[0][0]
				&& hsv_planes[ch[0]].data[hsv_planes[ch[0]].cols * y + x] < ranges[0][1]))
			{
				h_[hsv_planes[ch[0]].data[hsv_planes[ch[0]].cols * y + x]]++;
			}

		}
	}
}

void Back_Project(cv::Mat& hsv, const int* ch, cv::Mat& Histogram, cv::Mat& bp) {
	vector<Mat> m_hsv;
	split(hsv, m_hsv);

	float* hData = (float*)Histogram.data;

	for (int y = 0; y < bp.rows; y++) {
		for (int x = 0; x < bp.cols; x++) {
			bp.data[bp.cols * y + x] = (unsigned char)hData[m_hsv[ch[0]].data[m_hsv[ch[0]].cols * y + x]];
		}
	}
}