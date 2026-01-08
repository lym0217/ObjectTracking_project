#include "image_processing.h"
using namespace std;
using namespace cv;

int rect_x, rect_y, rect_W, rect_H;
bool isPressing = false;
int step = 0;

Rect roi;

Mat Frame;
Mat Frame_hsv;
Mat ROI_img;
Mat Mask_img;

Point Center(0,0);
Point gCenter(0,0);

float h[180] = { 0 };

int channels[] = { 0 };
int hsize[] = { 180 };
float range1[] = { 0, 180 };
const float* histRange[] = { range1 };
Mat Histogram;

bool isPlay = true;


void onMouse(int evt, int x, int y, int flags, void* parm) {
	if (evt == EVENT_LBUTTONDOWN) {
		isPressing = true;
		rect_x = x;
		rect_y = y;
		if (!isPlay) {
			circle(Frame, Point(rect_x, rect_y), 1, Scalar(0, 255, 0), -1);
			imshow("Video", Frame);
		}
		step = 1;
	}
	else if (evt == EVENT_MOUSEMOVE) {
		if (isPressing) {
			rect_W = x - rect_x;
			rect_H = y - rect_y;

			step = 2;
		}

	}
	else if (evt == EVENT_LBUTTONUP) {
		isPressing = false;

		rect_W = x - rect_x;
		rect_H = y - rect_y;

		if (!isPlay) {
			rectangle(Frame, Point(rect_x, rect_y), Point(rect_x + rect_W, rect_y + rect_H), Scalar(0, 255, 0), 1);
			imshow("Video", Frame);

			roi = Rect(rect_x + 1, rect_y + 1, rect_W - 2, rect_H - 2);
			cvtColor(Frame, Frame_hsv, COLOR_BGR2HSV);
			ROI_img = Mat(Frame_hsv, roi);


			Hist(ROI_img, channels, histRange, h);
			Histogram = Mat(Size(1, hsize[0]), CV_32FC1, h);							 // 구현 meanShift


			//calcHist(&ROI_img, 1, channels, Mask_img, Histogram, 1, hsize, histRange); // 라이브러리 meanShift
			normalize(Histogram, Histogram, 0, 255, NORM_MINMAX);

			//imshow("ROI_Image", ROI_img);
			step = 4;
		}

		else
			step = 3;
	}
}

int main() {
	VideoCapture cap("apple.gif");

	if (!cap.isOpened()) {
		cout << "video error" << endl;
		return 0;
	}

	namedWindow("Video", 1);
	setMouseCallback("Video", onMouse, &Frame);

	while (true) {
		int keyValue;

		if (!isPlay)
			keyValue = waitKey(0);
		else
			keyValue = waitKey(70);

		if (keyValue == ' ') {
			isPlay = !isPlay;
			if (isPlay) printf("Play\n");
			else printf("Pause\n");
		}

		if (!cap.read(Frame))
			break;

		switch (step) {
		case 1:
			if (isPlay)
				circle(Frame, Point(rect_x, rect_y), 10, Scalar(0, 255, 0), -1);
			break;
		// Mouse Down

		case 2:
			if (isPlay)
				rectangle(Frame, Point(rect_x, rect_y), Point(rect_x + rect_W, rect_y + rect_H), Scalar(0, 255, 0), 1);
			break;
		// Mouse Move

		case 3:
			if (isPlay) {
				roi = Rect(rect_x, rect_y, rect_W, rect_H);
				cvtColor(Frame, Frame_hsv, COLOR_BGR2HSV);
				ROI_img = Mat(Frame_hsv, roi);

				Hist(ROI_img, channels, histRange, h);
				Histogram = Mat(Size(1, hsize[0]), CV_32FC1, h);							 // 구현 Histogram

				//calcHist(&ROI_img, 1, channels, Mask_img, Histogram, 1, hsize, histRange); // 라이브러리 Histogram
				normalize(Histogram, Histogram, 0, 255, NORM_MINMAX);
				// H성분 Histogram -> Gray 0 ~ 255 (Matching)

				//imshow("ROI_Image", ROI_img);

				step++;
			}
			
			break;
		// Mouse Up

		case 4:
			cvtColor(Frame, Frame_hsv, COLOR_BGR2HSV);
			/*Mat bp;
			calcBackProject(&Frame_hsv, 1, channels, Histogram, bp, histRange);*/ // 라이브러리 Back Project


			Mat bp = Mat::zeros(Size(Frame_hsv.size()), CV_8UC1);
			Back_Project(Frame_hsv, channels, Histogram, bp);					  // 구현 Back Project

			// ROI 영상 Histogram을 통해 현재 Frame_hsv 영상을 BackProject -> bp

			for (int y = 0; y< bp.rows; y++) {
				for (int x = 0; x < bp.cols; x++) {
					if (bp.at<uchar>(y, x) < 40)
						bp.at<uchar>(y,x) = 0;
				}
			}
			// Back Project 영상 Thresholding (value = 40)
			imshow("bp", bp);



			//meanShift(bp, roi, TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 10, 1)); // 라이브러리 meanShift

			Mean_Shift(bp, roi, 10, 1.0, Center, gCenter);										// 구현 meanShift

			// Tracking


			rectangle(Frame, roi, Scalar(0, 0, 255), 2);
			putText(Frame, "Tracking", Point(roi.x, max(13, roi.y - 13)), 0, 0.5, Scalar(0, 255, 255), 2);


			circle(Frame, gCenter, 2, Scalar(0, 255, 0), -1); // 무게중심 Point
			circle(Frame, Center, 2, Scalar(255, 0, 0), -1); // Window 중심 Point

			break;
		}


		imshow("Video", Frame);
	}

}