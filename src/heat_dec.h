#pragma once
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include "capture.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include"tinystr.h"
#include"tinyxml.h"

using namespace cv;
using namespace std;

class heat_dec:public basedec
{
    struct Tconf
    {
        Point position;
        float tdev;								//设备检测点的温度
        double tother;								//同环境设备参考温度
        double temperature_rise;					//温升：被测设备表面温度和环境温度参照体表面温度之差。
        double temperature_difference;				//温差：不同被测设备或同一被测设备不同部位之间的温度差。
        double relative_temperature_difference;		//相对温差：两个对应测点之间的温差与其中较热点的温升之比的百分数。
    };
public:
    heat_dec(shared_ptr<capture> mc) : mycapture(mc)
    {
        readxml("/home/zxb/SRC_C/Detect/conf.xml");
    }
    int detect();								//测试接口，测试主要流程。

    //主要流程方法
    void culate(WORD *inputData,const vector<vector<Point>> &dev_contour);	//计算设备的温度特征值。
    int faultdetect();											//返回值：0正常；1一般缺陷；2严重缺陷；3危机缺陷
    void drawre(Tconf &tf);

private:
    shared_ptr<capture> mycapture;

    //警报阈值以及标志，三元数组代表三种级别的故障。
    vector<pair<double, double>> tdev_range[3];			//设备温度的警报范围
    vector<pair<double, double>> trise_range[3];		//温升的警报范围
    vector<pair<double, double>> tdifference_range[3];	//温差的警报范围
    vector<pair<double, double>> trd_range[3];			//相对温差的警报范围

    double tenv;
    vector<Tconf> tconf;							//设备的红外数据

    //辅助型变量和函数
    void readxml(string filename);				//读取配置文件，导入警报阈值

};

