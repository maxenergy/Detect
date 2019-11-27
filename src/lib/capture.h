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
    int dande;				//0无 1腐蚀 2膨胀 3腐蚀+膨胀
    int dandesize;			//核大小
    int contrast_min;		//锐化min
    int contrast_max;		//锐化max
    int th;					//0大律法 n二值化阈值
    int minnum;				//轮廓点数最小阈值
};

struct record_time
{
    record_time():year(0),month(0),day(0),hour(0),min(0),sec(0)
    {}

    record_time(int iyear,int imonth, int iday, int ihour,int imin, int isec)
    {
        set(iyear, imonth,  iday,  ihour, imin,  isec);
    }

    record_time(const tm *p)
    {
        year=p->tm_year + 1900;
        month = p->tm_mon + 1;
        day=p->tm_mday;
        hour=p->tm_hour;
        min=p->tm_min;
        sec=p->tm_sec;
    }

    bool operator<(const record_time &m)
    {
        if(year<m.year)
            return true;
        else if(month<m.month)
            return true;
        else if(day<m.day)
            return true;
        else if(hour<m.hour)
            return true;
        else if(min<m.min)
            return true;
        else if(sec<m.sec)
            return true;
        else
            return false;
    }
    bool operator<=(const record_time &m)
    {
        if(year==m.year && month==m.month && day==m.day && hour==m.hour && min==m.min && sec==m.sec)
            return true;
        if(operator<(m))
            return true;
        return false;
    }

    void clear()
    {
        year=0;
        month=0;
        day=0;
        hour=0;
        min=0;
        sec=0;
    }

    bool isempty()
    {
        if(year==0 &&month==0 &&day==0 &&hour==0 &&min==0 && sec==0)
            return true;
        else
            return false;
    }

    void set(int iyear,int imonth, int iday, int ihour,int imin, int isec)
    {
        year=iyear;
        month=imonth;
        day=iday;
        hour=ihour;
        min=imin;
        sec=isec;

        if(iyear>2000 && iyear<3000 && imonth>0 && imonth<13 && iday>0 && iday<32 && ihour>=0 && ihour<24 && imin>=0 && imin<60 && isec>=0 && isec<60)
            return;

        while(true){
            if(iyear>2000 && iyear<3000 && imonth>0 && imonth<13 && iday>0 && iday<32 && ihour>=0 && ihour<24 && imin>=0 && imin<60 && isec>=0 && isec<60)
                break;
            if(isec>59){
                isec-=60;
                imin+=1;
            }
            if(isec<0){
                isec+=60;
                imin-=1;
            }

            if(imin>59){
                imin-=60;
                ihour+=1;
            }
            if(imin<0){
                imin+=60;
                ihour-=1;
            }

            if(ihour>23){
                ihour-=24;
                iday+=1;
            }
            if(ihour<0){
                ihour+=24;
                iday-=1;
            }

        }

        if(iyear>2000 && iyear<3000 && imonth>0 && imonth<13 && iday>0 && iday<32 && ihour>=0 && ihour<24 && imin>=0 && imin<60 && isec>=0 && isec<60)
            return;
        else{
            year=0;
            month=0;
            day=0;
            hour=0;
            min=0;
            sec=0;
        }
    }

    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
};

class capture
{
public:
    Mat srcrgb;
    Mat srcir;
    Mat srcuv;

    NET_DEV_RAWFILEHEAD cm_RawHead;
    BYTE * cm_pData ;
    unsigned long cm_dwDataSize ;
    unsigned char *cpTempPara;
    unsigned long cTempParaSize;


    //初始化
    void SDK_Init();
    //登录设备
    bool SDK_Connect();
    //开启视频流实时更新
    bool Vedio_Stream_Set();
    //更新当前帧
    void Vedio_Update();

    //度温度接口
    //获取（x，y）的灰度值
    unsigned short getgray(int x,int y)
	{
		return ((WORD*)cm_pData)[x + 640 * y];
	}
	//将灰度值转化为温度
    float Get_tem(unsigned short nGray)
	{
		return Temperature_GetTempFromGray(nGray, 0.96, 0, cpTempPara, cTempParaSize, cm_RawHead.nCalcType); //返回温度
	}
	//获取一个区域的温度信息
	//	tem_type:   <0 n个点的平均最低温度  0平均温度  >0 n个点的最平均高温度
	//	area_type： 0精确区域 1粗略外接矩形
	pair<float,Point> Area_tem(const vector<Point> &counter,char tem_type,char area_type);

    //录像接口
    bool Vedio_record(record_time begin,record_time end,int port,string filename);
    //释放资源
    void SDK_Close();
private:
    int capture_mode;
    ifstream txtfile;		//mode0 : 读取图片序列
    Mat current_mat;		//mode1 : 读取静态图片
    VideoCapture vcapture;	//mode2	: 读取视频

    //设备的 user id
    LONG lUserID;
    short IRUserID;



public:
    capture()
    {
        SDK_Init();
    }
    capture(string Filename, int mode):capture_mode(mode)
    {
        if (mode == 0)
            txtfile.open(Filename);
        else if (mode == 1)
            current_mat = imread(Filename);
        else if (mode==2)
            vcapture.open(Filename);

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
    //unlock
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
    virtual int faultdetect();


    vector<pair<Vec3b, double>> mapoft;         //温度对照表
    unsigned short *digitdata;                  //温度图

    Mat src;									//读入的源图片
    Mat gray;									//源图片的灰度图
    Mat TH;										//原图片的二值图
    Mat result_pic;                             //result picture
    vector<vector<Point>> contours;				//对二值图边缘检测的结果
    vector<vector<Point>> s_contour;			//可疑区域
    int failure_alarm_flag;						//故障标志位

    ofstream logfile;
    bool logout =false;

};

Point centerofV(const vector<Point> &p);


