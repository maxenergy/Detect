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
	fire_dec(string Filename):mycapture(Filename,2)
	{
	}
	void detect();
	int faultdetect() const;
private:
	capture mycapture;
	temporaldatam temporalctrl{20,1};
	temporaldatam::fstruct f1;
	temporaldatam::fstruct f2;
	temporaldatam::fstruct f3;
	temporaldatam::fstruct f4;

	
	
};


//�����������㺯��
vector<double> f_area(const vector<Point> &);				//���
vector<double> f_perimeter(const vector<Point> &);			//�ܳ�
vector<double> f_circle(const vector<Point> &);				//��Բ��
vector<double> f_cofc(const vector<Point> &);				//�߽�仯��

double culdis(const vector<double> &v1, const vector<double> &v2);  //��������dct����֮��ľ���