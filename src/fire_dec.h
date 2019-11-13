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
        fire_dec(shared_ptr<capture> mc):mycapture(mc),temporalctrl(20,16,1)
	{
            logfile.open("log_fire.txt",ios::trunc);
            temporalctrl.ZEROF();
            temporalctrl.SETF(AREA);
            temporalctrl.SETF(PERIMETER);
            temporalctrl.SETF(CIRCLE);
            //temporalctrl.SETF(COC);

	}
    int detect();
    int faultdetect();
private:
	shared_ptr<capture> mycapture;

        temporaldatam_v2 temporalctrl;
        queue<pair<queue<unsigned long>, queue<double>>> f1;
        queue<pair<queue<unsigned long>, queue<double>>> f2;
        queue<pair<queue<unsigned long>, queue<double>>> f3;
        queue<pair<queue<unsigned long>, queue<double>>> f4;

	
	
};


double culdis(const vector<double> &v1, const vector<double> &v2);  //计算两个dct向量之间的距离
