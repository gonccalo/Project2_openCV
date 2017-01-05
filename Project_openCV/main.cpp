#include <iostream>


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "TrakingBuffer.h"

using namespace cv;
using namespace std;


void do_morph(Mat img) {
	Mat ker = getStructuringElement(MORPH_ELLIPSE, Size(8, 8));
	morphologyEx(img, img, MORPH_OPEN, ker, Point(-1, -1), 2);
}

Mat filter_red_objects(Mat image, int lowH, int highH, int lowS, int highS, int lowV, int highV) {
	Mat tmp_lower;
	Mat tmp_upper;
	Mat result;
	inRange(image, cv::Scalar(lowV, lowS, lowH), cv::Scalar(highV, highS, highH),result);
	//inRange(image, cv::Scalar(160, 100, 100), cv::Scalar(179, 255, 255), tmp_upper);
	//addWeighted(tmp_lower, 1.0, tmp_upper, 1.0, 0.0, result);
	GaussianBlur(result, result, cv::Size(9, 9), 2, 2);
	return result;
}

Point max_area(Mat can, Mat img) {
	double Amax_area = 0.0;
	double Atmp_area = 1.0;
	Point center;
	vector<vector<Point> > contours;
	vector<Vec4i> hier;
	findContours(can, contours, hier, CV_RETR_CCOMP, CHAIN_APPROX_SIMPLE);
	if (contours.empty()) {
		return Point(-1, -1);
	}
	drawContours(img, contours, -1, cv::Scalar(0, 255, 0));
	for (int i = 0; i >= 0; i = hier[i][0]) {
		Moments moment = moments((Mat)contours[i]);
		Atmp_area = moment.m00;
		//cout << "Area: " << Atmp_area << endl;
		if (Atmp_area > Amax_area && Atmp_area > (10 * 10)) {
			center = Point(moment.m10 / Atmp_area, moment.m01 / Atmp_area);
			Amax_area = Atmp_area;
		}
	}
	if (Amax_area > 0) {
		return center;
	}
	else return Point(-1, -1);
}

int main(int argc, char** argv)
{
	VideoCapture cap; //capture the video from web cam
	cap.open(0);
	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the web cam" << endl;
		return -1;
	}
	cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('B', 'G', 'R', '3'));

	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
	namedWindow("Video1", CV_WINDOW_AUTOSIZE);

	int iLowH = 90;
	int iHighH = 179;

	int iLowS = 100;
	int iHighS = 255;

	int iLowV = 100;
	int iHighV = 255;

	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);
	
	TrakingBuffer buff = TrakingBuffer();
	
	while (true)
	{
		Mat imgOriginal;
		cap >> imgOriginal; // read a new frame from video
		if (imgOriginal.empty()) {
			cap.set(CV_CAP_PROP_POS_FRAMES, 0);
			continue;
		}
		Mat new_img;
		cvtColor(imgOriginal, new_img, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
		
		//filter colors
		new_img = filter_red_objects(new_img,iLowH, iHighH, iLowS, iHighS, iLowV, iHighV);
		
		//erosion and dilation operations
		do_morph(new_img);

		//Mat can;
		//Canny(new_img, can, 50, 200);

		if (new_img.empty())
		{
			imshow("Video1", imgOriginal); //show the original image
			imshow("Threshold", new_img);
			if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
			{
				cout << "esc key is pressed by user" << endl;
				break;
			}
			continue;
		}
		
		Point center = max_area(new_img, imgOriginal);
		if (center.x != -1 && center.y != -1)
		{
			putText(imgOriginal, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
			buff.put(&center);
			for (int i = 0; i < 32; i++) {
				Point* tmp = buff.get(i);
				if (tmp == NULL)
					break;
				circle(imgOriginal, *tmp, 5, Scalar(255, 0, 0));
			}
		}
		imshow("Video1", (Mat)imgOriginal); //show the original image
		imshow("Threshold", new_img);

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
	return 0;
}