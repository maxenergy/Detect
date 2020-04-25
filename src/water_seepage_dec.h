#pragma once
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include "socket_connect_v2.h"
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
    water_seepage_dec(shared_ptr<capture> mc, Server* sv) : mycapture(mc), alctrl(mc, sv, 2),temporalctrl(200,16,3)
    {
        logfile.open("log_warter.txt",ios::trunc);
        temporalctrl.ZEROF();
        temporalctrl.SETF(AREA);
    }
    int detect();																//返回值：0正常；1疑似渗水
    void dec_w(WORD *inputData, vector<vector<Point>> &suspicious_contour);	//对可疑区域进行筛选判断
    int faultdetect();

    void save_and_send(State_mes mes, Mat rgb, Mat ir, Mat uv, string basef,int pt)
	{
        alctrl.save_and_send(mes, rgb, ir, uv, basef, pt);
	}
	void clear()
	{
		alctrl.clear();
		temporalctrl.clear();
	}
	void setstaytime(int t)
	{
		alctrl.setstaytime(t);
	}
	bool isstay()
	{
		return alctrl.isstay();
	}

private:
    shared_ptr<capture> mycapture;
	alarm_ctrl alctrl;

    temporaldatam_v2 temporalctrl;
    queue<pair<queue<unsigned long>, queue<double>>> f1;
};

