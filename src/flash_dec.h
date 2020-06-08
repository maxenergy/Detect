#pragma once
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include "socket_connect_v2.h"
#include "temporaldatam.h"
#include "capture.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

vector<double> f_num(const vector<Point> &);				//面积

class flash_dec :public basedec
{
public:
    flash_dec(shared_ptr<capture> mc, Server* sv) : mycapture(mc), alctrl(mc, sv, 2),temporalctrl(20,25,1)
    {
        logfile.open("log_falsh.txt",ios::trunc);
        //temporalctrl.addf("area", f_num);
        temporalctrl.ZEROF();
        temporalctrl.SETF(NUM);
        temporalctrl.SETF(AREA);
    }
    int detect();																//返回值：0正常；1疑似渗水
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
};



