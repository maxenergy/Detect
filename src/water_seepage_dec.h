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
	water_seepage_dec(string Filename):mycapture(Filename,0)
	{

	}
	void detect();																//����ֵ��0������1������ˮ
	void dec_w(double **inputData, vector<vector<Point>> &suspicious_contour);	//�Կ����������ɸѡ�ж�
	int faultdetect();

private:
	capture mycapture;
	temporaldatam temporalctrl;
	temporaldatam::fstruct f1;
};

vector<double> f_area(const vector<Point> &);				//���
vector<double> f_perimeter(const vector<Point> &);			//�ܳ�
vector<double> f_circle(const vector<Point> &);				//��Բ��