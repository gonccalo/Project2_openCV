#pragma once
#include <iostream>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "TrakingBuffer.h"

using namespace cv;
using namespace std;

#define CONTROL_WINDOW "Control"
#define MAIN_WINDOW    "Object tracking"
#define CAL_WINDOW     "Calibration window"
#define BY_COLOR  0
#define BY_SHAPE  1
#define BY_BOTH   2
#define ESC      27
#define SPACEBAR 32

#define CAMERA							//comentar esta linha para usar um ficheiro de video em vez da camera
#define VIDEO_FILE "video_test2.mp4"
#define MAX_OBJECTS  10
#define MIN_AREA    400  //20*20

typedef struct controls {
	int filter_type = 0;

	int iLowH = 90;
	int iHighH = 179;

	int iLowS = 100;
	int iHighS = 255;

	int iLowV = 100;
	int iHighV = 255;

	int param2 = 10;

	bool mouseDrag = false;
	bool selected = false;
	bool moving = false;
	Point init_point;
	Point current_point;
	Rect choosen_zone;
}Controls_val;

void do_morph(Mat img);
Mat filter_color_objects(Mat image, Controls_val* colors);
Point max_area(Mat can, Mat img);
void trackbar_callback(int __, void *c);
vector<Vec3f> detect_circles(Mat img, int param2 = 50);
Vec3f detect_max_circle(Mat img, Controls_val* ctrl);
void filter_type_callb(int v, void*);
void mouse_callb(int ev, int x, int y, int flags, void* p);
void color_filter_calibrate(Mat frameHSV, Mat frame, Controls_val* c);