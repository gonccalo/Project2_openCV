#include "main.h"

int main(int argc, char** argv)
{
	VideoCapture cap; //capture the video from web cam
	cap.open("video_test2.mp4");
	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the web cam" << endl;
		return -1;
	}
	cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('B', 'G', 'R', '3'));
	TrakingBuffer buff = TrakingBuffer();
	Controls_val* filter_controls = new Controls_val();
	bool pause = false;
	int pressed_key = -1;
	Mat imgOriginal;
	Mat processed;
	Mat new_img_hsv;
	namedWindow("Video1", CV_WINDOW_AUTOSIZE);
	setMouseCallback("Video1", mouse_callb, filter_controls);

	namedWindow(CONTROL_WINDOW, CV_WINDOW_NORMAL);
	createTrackbar("Filter type", CONTROL_WINDOW, &filter_controls->filter_type, 2, filter_type_callb, filter_controls);
	filter_type_callb(0, filter_controls);
	Mat pickedColor(200, 300, CV_8UC3);
	pickedColor.setTo(Scalar(filter_controls->iHighH, filter_controls->iHighS, filter_controls->iHighV));
	cvtColor(pickedColor, pickedColor, CV_HSV2BGR);
	imshow(CONTROL_WINDOW, pickedColor);
	
	
	while (true)
	{
		if (!pause) {
			cap >> imgOriginal; // read a new frame from video
			if (imgOriginal.empty()) {
				cap.set(CV_CAP_PROP_POS_FRAMES, 0);
				continue;
			}
			processed = imgOriginal.clone();
			cvtColor(processed, new_img_hsv, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
		}
		else {
			processed = imgOriginal.clone();
			cvtColor(processed, new_img_hsv, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
			color_filter_calibrate(new_img_hsv, processed, filter_controls);
		}
		
		switch (filter_controls->filter_type)
		{
		case BY_COLOR:
		{
			//filter colors
			new_img_hsv = filter_color_objects(new_img_hsv, filter_controls);
			//erosion and dilation operations
			do_morph(new_img_hsv);
			if (new_img_hsv.empty())
				break;

			Point center = max_area(new_img_hsv, processed);
			if (center.x != -1 && center.y != -1)
			{
				putText(processed, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				buff.put(&center);
				int sumx = 0;
				int sumy = 0;
				for (int i = 0; i < 31; i++) {
					Point* tmp = buff.get(i);
					Point* tmp2 = buff.get(i + 1);
					if (tmp == NULL || tmp2 == NULL)
						break;
					sumx += tmp->x - tmp2->x;
					sumy += tmp->y - tmp2->y;
					line(processed, *tmp2, *tmp, Scalar(0, 255, 0), (sqrt(32 / (i + 1)) * 2.5));
				}
				arrowedLine(processed, center, center + (Point(sumx / 32, sumy / 32) * 20), Scalar(255, 0, 0), 3);
			}
			break;
		}
		case BY_SHAPE:
		{
			Vec3f max_circleS;
			max_circleS = detect_max_circle(processed, filter_controls);
			circle(processed, Point(max_circleS[0], max_circleS[1]), max_circleS[2], Scalar(255, 0, 0), 2);
			break;
		}
		case BY_BOTH:
		{
			//filter colors
			new_img_hsv = filter_color_objects(new_img_hsv, filter_controls);
			//erosion and dilation operations
			do_morph(new_img_hsv);
			
			Vec3f max_circle;
			max_circle = detect_max_circle(new_img_hsv, filter_controls);
			circle(processed, Point(max_circle[0], max_circle[1]), max_circle[2], Scalar(255, 0, 0), 2);
			break;
		}
		}

		imshow("Video1", (Mat)processed); //show the original image
		imshow("Threshold", new_img_hsv);

		pressed_key = waitKey(10);
		switch (pressed_key)
		{
		case ESC:
			cout << "esc key is pressed by user" << endl;
			return 0;
		case SPACEBAR:
			pause = !pause;
			break;
		default:
			break;
		}
	}
	return 0;
}

void filter_type_callb(int v, void* c) {
	Controls_val* filter_controls = (Controls_val*)c;
	destroyWindow(CONTROL_WINDOW);
	namedWindow(CONTROL_WINDOW, CV_WINDOW_NORMAL);
	createTrackbar("Filter type", CONTROL_WINDOW, &filter_controls->filter_type, 2, filter_type_callb, filter_controls);
	switch (v)
	{
	case 2:
		createTrackbar("Param2", CONTROL_WINDOW, &filter_controls->param2, 200);
	case 0:
		createTrackbar("LowH", CONTROL_WINDOW, &filter_controls->iLowH, 179, trackbar_callback, filter_controls); //Hue (0 - 179)
		createTrackbar("HighH", CONTROL_WINDOW, &filter_controls->iHighH, 179, trackbar_callback, filter_controls);

		createTrackbar("LowS", CONTROL_WINDOW, &filter_controls->iLowS, 255, trackbar_callback, filter_controls); //Saturation (0 - 255)
		createTrackbar("HighS", CONTROL_WINDOW, &filter_controls->iHighS, 255, trackbar_callback, filter_controls);

		createTrackbar("LowV", CONTROL_WINDOW, &filter_controls->iLowV, 255, trackbar_callback, filter_controls); //Value (0 - 255)
		createTrackbar("HighV", CONTROL_WINDOW, &filter_controls->iHighV, 255, trackbar_callback, filter_controls);
		break;
	case 1:
		createTrackbar("Param2", CONTROL_WINDOW, &filter_controls->param2, 200);
		break;
	default:
		break;
	}
}

void mouse_callb(int ev, int x, int y, int flags, void * p)
{
	Controls_val* c = (Controls_val*)p;
	if (ev == CV_EVENT_LBUTTONDOWN &&  c->mouseDrag == false)
	{
		c->init_point = Point(x, y);
		c->mouseDrag = true;
	}
	if (ev == CV_EVENT_MOUSEMOVE && c->mouseDrag == true)
	{
		//keep track of current mouse point
		c->moving = true;
		c->current_point = Point(x, y);
	}
	if (ev == CV_EVENT_LBUTTONUP && c->mouseDrag == true)
	{
		//set rectangle ROI to the rectangle that the user has selected
		c->choosen_zone = Rect(c->init_point, c->current_point);
		//reset boolean variables
		c->mouseDrag = false;
		c->selected = true;
		c->moving = false;
	}
}

void color_filter_calibrate(Mat frameHSV, Mat frame, Controls_val * c)
{
	Controls_val* ctrl = (Controls_val*)c;
	if (c->selected) {
		c->selected = false;
		if (c->choosen_zone.width < 1 || c->choosen_zone.height < 1) return;
		int maxH = -1, maxS = -1, maxV = -1;
		int minH = 256, minS = 256, minV = 256;
		for (int j = c->choosen_zone.y; j < c->choosen_zone.y + c->choosen_zone.height; j++) {
			Vec3b* pixel = frameHSV.ptr<Vec3b>(j);
			for (int i = c->choosen_zone.x; i < c->choosen_zone.x + c->choosen_zone.width; i++)
			{
				maxH = (pixel[i][0] > maxH) ? pixel[i][0] : maxH;
				minH = (pixel[i][0] < minH) ? pixel[i][0] : minH;

				maxS = (pixel[i][1] > maxS) ? pixel[i][1] : maxS;
				minS = (pixel[i][1] < minS) ? pixel[i][1] : minS;

				maxV = (pixel[i][2] > maxV) ? pixel[i][2] : maxV;
				minV = (pixel[i][2] < minV) ? pixel[i][2] : minV;
			}
		}
		c->iHighH = maxH;
		c->iLowH  = minH;

		c->iHighS = maxS;
		c->iLowS  = minS;

		c->iHighV = maxV;
		c->iLowV  = minV;
	}
	else if (c->moving) {
		rectangle(frame, c->init_point, c->current_point, Scalar(0, 0, 255), 2);
	}
}


/* stackoverflow.com/questions/9860667/writing-robust-color-and-size-invariant-circle-detection-with-opencv-based-on */
vector<Vec3f> detect_circles(Mat img, int param2) {
	vector<Vec3f> result;
	int min_dist = 300;
	if (img.channels() == 1) {
		GaussianBlur(img, img, cv::Size(9, 9), 2, 2);
		Canny(img, img, 5, 150);
		GaussianBlur(img, img, cv::Size(9, 9), 2, 2);
		//Canny(img, img, 5, 70, 3);
		HoughCircles(img, result, HOUGH_GRADIENT, 2, min_dist, 2, param2, 30, img.rows / 2);
		return result;
	}
	Mat rgb[] = {
		Mat(img.rows, img.cols, IPL_DEPTH_8U, 1),
		Mat(img.rows, img.cols, IPL_DEPTH_8U, 1),
		Mat(img.rows, img.cols, IPL_DEPTH_8U, 1)
	};
	//Mat hsv_channels[3];
	GaussianBlur(img, img, cv::Size(9, 9), 2, 2);
	GaussianBlur(img, img, cv::Size(9, 9), 2, 2);
	split(img, rgb);
	adaptiveThreshold(rgb[0], rgb[0], 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 55, 7);
	do_morph(rgb[0]);
	adaptiveThreshold(rgb[1], rgb[1], 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 55, 7);
	do_morph(rgb[1]);
	adaptiveThreshold(rgb[2], rgb[2], 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 55, 7);
	do_morph(rgb[2]);
	rgb[0] = rgb[0] & rgb[1];
	rgb[0] = rgb[0] & rgb[2];
	Canny(rgb[0], rgb[0], 5, 70);
	GaussianBlur(rgb[0], rgb[0], cv::Size(9, 9), 2, 2);
	HoughCircles(rgb[0], result, HOUGH_GRADIENT, 2, min_dist, 2, param2, 20,img.rows/3);
	putText(img, "Number of circles: " + to_string(result.size()), Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
	//imshow("TEST", proc);
	return result;
}

Vec3f detect_max_circle(Mat img, Controls_val* ctrl) {
	if (ctrl->param2 <= 0) return Vec3f();
	vector<Vec3f> result = detect_circles(img, ctrl->param2);
	if (result.empty()) return Vec3f();
	int max_radius = 0;
	int max_circle_index;
	for (int i = 0; i < result.size(); i++) {
		if (result[i][2] > max_radius) {
			max_radius = result[i][2];
			max_circle_index = i;
		}
	}
	return result[max_circle_index];
}

Point max_area(Mat can, Mat img) {
	double Amax_area = 0.0;
	double Atmp_area = 1.0;
	Point center;
	vector<vector<Point>> contours;
	vector<Vec4i> hier;
	findContours(can, contours, hier, CV_RETR_CCOMP, CHAIN_APPROX_SIMPLE);
	if (contours.empty()) {
		return Point(-1, -1);
	}
	//drawContours(img, contours, -1, cv::Scalar(0, 255, 0));
	vector<Point> max_contour;
	for (int i = 0; i >= 0; i = hier[i][0]) {
		Moments moment = moments((Mat)contours[i]);
		Atmp_area = moment.m00;
		if (Atmp_area > Amax_area && Atmp_area > (20 * 20)) {
			center = Point(moment.m10 / Atmp_area, moment.m01 / Atmp_area);
			Amax_area = Atmp_area;
			max_contour = contours[i];
		}
	}
	if (Amax_area > 0) {
		Point2f c_enclosing(0.0, 0.0);
		float radius;
		minEnclosingCircle(max_contour, c_enclosing, radius);
		circle(img, c_enclosing, radius, Scalar(0, 255, 0), 2);
		return center;
	}
	else return Point(-1, -1);
}

Mat filter_color_objects(Mat image, Controls_val* colors) {
	//Mat tmp_lower;
	//Mat tmp_upper;
	Mat result;
	//GaussianBlur(image, image, cv::Size(9, 9), 2, 2);
	inRange(image, cv::Scalar(colors->iLowH, colors->iLowS, colors->iLowV), cv::Scalar(colors->iHighH, colors->iHighS, colors->iHighV), result);
	//inRange(image, cv::Scalar(160, 100, 100), cv::Scalar(179, 255, 255), tmp_upper);
	//addWeighted(tmp_lower, 1.0, tmp_upper, 1.0, 0.0, result);
	GaussianBlur(result, result, cv::Size(9, 9), 2, 2);
	return result;
}

void do_morph(Mat img) {
	Mat ker = getStructuringElement(MORPH_ELLIPSE, Size(8, 8));
	morphologyEx(img, img, MORPH_OPEN, ker, Point(-1, -1), 2);
}

void trackbar_callback(int __, void* c) {
	Controls_val* colors = (Controls_val*)c;
	Mat newTmpPick(200, 300, CV_8UC3);
	if((colors->iHighH - colors->iLowH) <= 0 || (colors->iHighS - colors->iLowS) <= 0 || (colors->iHighV - colors->iLowV) <= 0) return;
	int max_step = 200/(colors->iHighH - colors->iLowH);
	int val = 0;
	int step = 1;
	if (max_step < 1) return;
	for (int i = 0; i < newTmpPick.rows; i++) {
		step = 1;
		val = colors->iLowH;
		for (int j = 0; j < newTmpPick.cols; j++){
			newTmpPick.ptr(i, j)[0] = val;
			newTmpPick.ptr(i, j)[1] = 255;
			newTmpPick.ptr(i, j)[2] = 255;
			if (step++ >= max_step) {
				val += 1;
				step = 1;
			}
		}
	}
	//newTmpPick.setTo(Scalar(colors->iHighH, colors->iHighS, colors->iHighV));
	cvtColor(newTmpPick, newTmpPick, CV_HSV2BGR);
	imshow(CONTROL_WINDOW, newTmpPick);
}
