#pragma once
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include "temporaldatam.h"
#include "capture.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

class flash_dec :public basedec
{
public:
	flash_dec(shared_ptr<capture> mc) : mycapture(mc)
	{

	}
	void detect();																//返回值：0正常；1疑似渗水
	int faultdetect();

private:
	shared_ptr<capture> mycapture;
	temporaldatam temporalctrl;
	temporaldatam::fstruct f1;
};

vector<double> f_num(const vector<Point> &);				//面积

