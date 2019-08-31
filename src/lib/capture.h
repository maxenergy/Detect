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
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

typedef unsigned long DWORD;
typedef unsigned char BYTE;

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
	int dande;				//1����ʴ 2���� 3��ʴ+����
	int dandesize;			//�˴�С
	int contrast_min;		//��min
	int contrast_max;		//��max
	int th;					//0���ɷ� n��ֵ����ֵ
	int minnum;				//����������С��ֵ
};

class capture
{
public:
	Mat srcrgb;
	Mat srcir;
	Mat srcuv;
	struct struct_tp
	{
		unsigned char *m_pImageBuffer;
		DWORD m_ImageBufferSize;
		BYTE *m_pData;
		DWORD m_dwDataSize;
		
	}srctp;

	//��ʼ��
	void SDK_Init();
	//��¼�豸
	void SDK_Connect();
	//������Ƶ��ʵʱ����
	void Vedio_Stream_Set();
	//���µ�ǰ֡
	void Vedio_Update();
	//���¶Ƚӿ�
	float Get_tem(int gray);
	//¼��ӿ�
	void Vedio_record();

private:
	int capture_mode;
	ifstream txtfile;		//mode0 : ��ȡͼƬ����
	Mat current_mat;		//mode1 : ��ȡ��̬ͼƬ
	VideoCapture vcapture;	//mode2	: ��ȡ��Ƶ
public:
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
	double **Graytodigit(Mat inputMat, double tmax, double tmin);			//������ͼ��ת��Ϊ���ݱ�
	double gettdev(Mat src,Point point);
	double gettdev(int x, int y)
	{
		return gettdev(src,Point(x, y));
	}

	virtual vector<vector<Point>> get_suspicious_area(Mat src, suspiciousconf conf);				//���ؿ������������
	virtual int faultdetect() const;


	Mat src;									//�����ԴͼƬ
	Mat gray;									//ԴͼƬ�ĻҶ�ͼ
	Mat TH;										//ԭͼƬ�Ķ�ֵͼ
	vector<vector<Point>> contours;				//�Զ�ֵͼ��Ե���Ľ��
	vector<vector<Point>> s_contour;			//��������
	vector<pair<Vec3b, double>> mapoft;
	double **digitdata;							//�¶�ͼ
	int failure_alarm_flag;						//���ϱ�־λ
	
};

Point centerofV(const vector<Point> &p);
