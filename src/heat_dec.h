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
		double tdev;								//�豸������¶�
		double tother;								//ͬ�����豸�ο��¶�
		double temperature_rise;					//�����������豸�����¶Ⱥͻ����¶Ȳ���������¶�֮�
		double temperature_difference;				//�²��ͬ�����豸��ͬһ�����豸��ͬ��λ֮����¶Ȳ
		double relative_temperature_difference;		//����²������Ӧ���֮����²������н��ȵ������֮�ȵİٷ�����
	};
public:
	heat_dec(string Matname):mycapture(Matname,1)
	{
		src = mycapture.getframe();
		imshow("src", src);
	}
	void detect();								//���Խӿڣ�������Ҫ���̡�

	//��Ҫ���̷���
	void culate(double **inputData,const vector<vector<Point>> &dev_contour);	//�����豸���¶�����ֵ��
	int faultdetect() const;											//����ֵ��0������1һ��ȱ�ݣ�2����ȱ�ݣ�3Σ��ȱ��

private:
	capture mycapture;

	//������ֵ�Լ���־����Ԫ����������ּ���Ĺ��ϡ�
	vector<pair<double, double>> tdev_range[3];			//�豸�¶ȵľ�����Χ
	vector<pair<double, double>> trise_range[3];		//�����ľ�����Χ
	vector<pair<double, double>> tdifference_range[3];	//�²�ľ�����Χ
	vector<pair<double, double>> trd_range[3];			//����²�ľ�����Χ

	double tenv;
	vector<Tconf> tconf;							//�豸�ĺ�������

	//�����ͱ����ͺ���
	void readxml(string filename);				//��ȡ�����ļ������뾯����ֵ

};

