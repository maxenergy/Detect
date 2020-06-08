#pragma once
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include "socket_connect_v2.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "temporaldatam.h"
#include "capture.h"

using namespace std;
using namespace cv;

class fire_dec :public basedec
{
public:
        fire_dec(shared_ptr<capture> mc, Server* sv) : mycapture(mc), alctrl(mc, sv, 1), temporalctrl(60, 36, 1)
	{
		logfile.open("log_fire.txt", ios::trunc);
		temporalctrl.ZEROF();
		temporalctrl.SETF(AREA);
		temporalctrl.SETF(PERIMETER);
		temporalctrl.SETF(CIRCLE);
                temporalctrl.SETF(COC);
	}
	int detect();
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
	queue<pair<queue<unsigned long>, queue<double>>> f2;
	queue<pair<queue<unsigned long>, queue<double>>> f3;
	queue<pair<queue<unsigned long>, queue<double>>> f4;



};


double culdis(const vector<double> &v1, const vector<double> &v2);  //计算两个dct向量之间的距离
