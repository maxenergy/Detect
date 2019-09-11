#pragma once
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <mutex>
#include "HCNetSDK.h"
#include "LinuxPlayM4.h"
#include "IUlirNetDevSDK.h"
#include "TempAlgSDK.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

//typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef unsigned char byte;

using namespace std;
using namespace cv;

struct suspiciousconf
{
	suspiciousconf(int de, int desize, int c_min, int c_max, int t, int minn)
	{
		dande = de;
		dandesize = desize;
		contrast_min = c_min;
		contrast_max = c_max;
		th = t;
		minnum = minn;
	}
	int dande;				//1：腐蚀 2膨胀 3腐蚀+膨胀
	int dandesize;			//核大小
	int contrast_min;		//锐化min
	int contrast_max;		//锐化max
	int th;					//0大律法 n二值化阈值
	int minnum;				//轮廓点数最小阈值
};

class capture
{
public:
	Mat srcrgb;
	Mat srcir;
	Mat srcuv;
    BYTE * tmdata ;
    unsigned long tmdata_size ;

	//初始化
	void SDK_Init();
	//登录设备
    bool SDK_Connect();
	//开启视频流实时更新
    bool Vedio_Stream_Set();
	//更新当前帧
	void Vedio_Update();
	//度温度接口
    float Get_tem(unsigned short nGray);
	//录像接口
    bool Vedio_record();
    //释放资源
    void SDK_Close();
private:
	int capture_mode;
	ifstream txtfile;		//mode0 : 读取图片序列
	Mat current_mat;		//mode1 : 读取静态图片
	VideoCapture vcapture;	//mode2	: 读取视频
    LONG lUserID;
    short IRUserID;


public:
	capture(string Filename, int mode):capture_mode(mode)
	{
		if (mode == 0)
			txtfile.open(Filename);
		else if (mode == 1)
			current_mat = imread(Filename);
		else if (mode==2)
			vcapture.open(Filename);
        SDK_Init();
	}
	~capture()
	{
		if (capture_mode == 2)
			vcapture.release();
        SDK_Close();
	}
	Mat getcurrentmat()
	{
		return current_mat;
	}

	Mat getframe();

	//lock
	void lock();
	//unlocdk
	void unlock();
};

class basedec
{
public:
	basedec()
	{

	}
	virtual ~basedec()
	{

	}
	double **Graytodigit(Mat inputMat, double tmax, double tmin);			//将红外图像转化为数据表。
	double gettdev(Mat src,Point point);
	double gettdev(int x, int y)
	{
		return gettdev(src,Point(x, y));
	}

	virtual vector<vector<Point>> get_suspicious_area(Mat src, suspiciousconf conf);				//返回可疑区域的轮廓
	virtual int faultdetect() const;


	Mat src;									//读入的源图片
	Mat gray;									//源图片的灰度图
	Mat TH;										//原图片的二值图
	vector<vector<Point>> contours;				//对二值图边缘检测的结果
	vector<vector<Point>> s_contour;			//可疑区域
	vector<pair<Vec3b, double>> mapoft;
	double **digitdata;							//温度图
	int failure_alarm_flag;						//故障标志位
	
};

Point centerofV(const vector<Point> &p);
