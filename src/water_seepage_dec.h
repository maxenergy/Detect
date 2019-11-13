#pragma once
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include "temporaldatam.h"
#include "capture.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

class water_seepage_dec:public basedec
{
public:
    water_seepage_dec(shared_ptr<capture> mc) :mycapture(mc),temporalctrl(5,16,1)
    {
        logfile.open("log_warter.txt",ios::trunc);
        temporalctrl.ZEROF();
        temporalctrl.SETF(AREA);
    }
    int detect();																//返回值：0正常；1疑似渗水
    void dec_w(WORD *inputData, vector<vector<Point>> &suspicious_contour);	//对可疑区域进行筛选判断
    int faultdetect();

private:
    shared_ptr<capture> mycapture;
    temporaldatam_v2 temporalctrl;
    queue<pair<queue<unsigned long>, queue<double>>> f1;
};

