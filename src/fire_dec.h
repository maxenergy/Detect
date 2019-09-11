#pragma once
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "temporaldatam.h"
#include "capture.h"

using namespace std;
using namespace cv;

class fire_dec:public basedec
{
public:
	fire_dec(shared_ptr<capture> mc):mycapture(mc)
	{
	}
	void detect();
	int faultdetect() const;
private:
	shared_ptr<capture> mycapture;
	temporaldatam temporalctrl{20,1};
	temporaldatam::fstruct f1;
	temporaldatam::fstruct f2;
	temporaldatam::fstruct f3;
	temporaldatam::fstruct f4;

	
	
};


double culdis(const vector<double> &v1, const vector<double> &v2);  //计算两个dct向量之间的距离
