#pragma once
#define Thofisnear 16		//判断两个key临近的阈值


#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <queue>
#include "capture.h"
#include "socket_connect.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#define AREA 0b00000001
#define PERIMETER 0b00000010
#define CIRCLE 0b00000100
#define NUM 0b00001000
#define COC 0b00010000

using namespace std;
using namespace cv;

//库提供的基本特征，用户还可以自行扩张。
vector<double> f_area(const vector<Point> &);				//  面积
vector<double> f_perimeter(const vector<Point> &);			//  周长
vector<double> f_circle(const vector<Point> &);				//  似圆度
vector<double> f_cofc(const vector<Point> &);				//  边界变化量

class temporaldatam_v2
{
public:
    typedef vector<Point> Counter;                      //  轮廓
    typedef Point Label;				//  轮廓队列的位置
    typedef unsigned long Timestamp;                    //  时间戳

    struct Que						//  轮廓队列
    {
        queue<Counter> counters;
        queue<Timestamp> timestamp;
        queue<double> area;
        queue<double> perimeter;
        queue<double> circle;
        queue<double> num;
        queue<double> cofc;

        Label label;
    };

public:
    temporaldatam_v2(int bufferlength = 20, int Distense = 16, int hz = 1) :
        qlength(bufferlength), Distense(Distense), Hz(hz)
    {}

    //操作接口
    void pushCounter(const vector<Counter> &c);                         //  向时序特征管理器添加新的轮廓
    int update();							//  一些更新工作
    void clear()                                                        //  清空队列
    {
        que.clear();
    }
    Timestamp gettimestamp_now()					//  返回当前时间戳
    {
        return timestamp_now;
    }
    queue<pair<queue<Timestamp>, queue<double>>> getfeature(char F)	//  返回指定的特征
    {
        queue<pair<queue<Timestamp>, queue<double>>> result;
        switch (F)
        {
        case AREA:
            for (Que &q : que)
                result.push(pair<queue<Timestamp>, queue<double>>(q.timestamp, q.area));
            break;
        case PERIMETER:
            for (Que &q : que)
                result.push(pair<queue<Timestamp>, queue<double>>(q.timestamp, q.perimeter));
            break;
        case CIRCLE:
            for (Que &q : que)
                result.push(pair<queue<Timestamp>, queue<double>>(q.timestamp, q.circle));
            break;
        case NUM:
            for (Que &q : que)
                result.push(pair<queue<Timestamp>, queue<double>>(q.timestamp, q.num));
            break;
        case COC:
            for (Que &q : que)
                result.push(pair<queue<Timestamp>, queue<double>>(q.timestamp, q.cofc));
            break;
        default:
            break;
        }
        return result;
    }
    const Counter &getlastcounter(int n)					//	返回指定队列的最后一个轮廓和其时间戳
    {
        return que[n].counters.back();
    }

    void ZEROF()
    {
        Features = 0;
    }
    void SETF(char c)
    {
        Features |= c;
    }
    void DELF(char c)
    {
        Features &= ~c;
    }
    bool ISF(char c)
    {
        return Features&c;
    }

private:
    vector<Que> que;				//	队列集合
    Timestamp timestamp_now = 21;

    int qlength;				//	轮廓序列的窗口长度
    int Hz;					//	采样频率：Hz张图片取一张
	int hz_cur = 0;
    int Distense;				//	归队距离
    char Features = 0;				//	特征选择
    int nhz = 0;



private:
    Label cul_label(const Counter &p) const;
    int distense_label(const Label &l1, const Label &l2) const
    {
        return (l1.x - l2.x)*(l1.x - l2.x) + (l1.y - l2.y)*(l1.y - l2.y);
    }


    double f_area(const Counter &c)				//面积
    {
        return contourArea(c, false);
    }
    double f_perimeter(const Counter &c)			//周长
    {
        return arcLength(c, true);
    }
    double f_circle(const Counter &c)				//似圆度
    {
        double a = contourArea(c, false);
        double p = arcLength(c, true);
        return 4 * 3.14*a / (p*p);
    }
    double f_num(const Counter &c)
    {
        return 1;
    }
    double f_cofc(const Counter &c)				//边界变化量
    {
        vector<double> X;
        vector<double> Y;

        for (Point i : c)
        {
            X.push_back(i.x);
            Y.push_back(i.y);
        }
        if (c.size() % 2 != 0) //DCT仅支持偶数个数据
        {
            X.push_back(X.front());
            Y.push_back(Y.front());
        }

        Mat input1(X), input2(Y);
        input1.convertTo(input1, CV_64FC1);
        input2.convertTo(input2, CV_64FC1);
        Mat output1(input1.size(), input1.type());
        Mat output2(input2.size(), input2.type());
        dct(input1, output1);
        dct(input2, output2);

        /*cout << input1.size() << endl;
        for (int n = 0;n < input1.rows;n++)
        {
        double x = input1.at<double>(n, 0);
        double y = input2.at<double>(n, 0);
        cout << "(" << x << "," << y << ")	";
        }*/

        double x1 = output1.at<double>(0, 0);
        double y1 = output2.at<double>(0, 0);
        double f1 = sqrt(x1*x1 + y1*y1);
        for (int n = 0;n < input1.rows && n < 9;n++)
        {
            double x = output1.at<double>(n, 0);
            double y = output2.at<double>(n, 0);
            double qf = sqrt(x*x + y*y) / f1;
            //cout << "(" << x << "," << y << ")	";
            //result.push_back(qf);
        }
        return 0;
    }
};

class alarm_ctrl
{
public:
    alarm_ctrl(shared_ptr<capture> mc,int wt):mycapture(mc),wait_time(wt)
    {}
    ~alarm_ctrl()
    {}

    //更新状态
    void updata();
    //保存故障信息
    void savefault(state_mes mes,Mat rgb,Mat ir,Mat uv,string basefile);

    //清除发送状态
    void clear()
    {
    }
    //设置硬直时间
    void setstaytime(int t)
    {
    }
private:

private:
    //待发送故障时间
    record_time alarm_time;
    //发送等待时间
    int wait_time;
    //硬直时间
    record_time stay_time;

    shared_ptr<capture> mycapture;
    string mesname;
    string rgbname;
    string irname;
    string uvname;
    string vedioname;
};



