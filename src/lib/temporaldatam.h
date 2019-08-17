#pragma once
#define Thofisnear 16		//�ж�����key�ٽ�����ֵ


#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>


using namespace std;
using namespace cv;

struct element
{
	element(int it, const vector<Point> &iv) :timeid(it), value(iv), plast(NULL)		//Ԫ�ؽڵ��ʼ��
	{}
	element(const element &e) :timeid(e.timeid), value(e.value), plast(NULL)
	{}
	int timeid;						//ʱ���ʶ
	vector<Point> value;			//һ���պ�����
	shared_ptr<element> plast;		//ָ����һ��Ԫ�ص�ָ��
};

template<class T>
class qofe
{
private:
	shared_ptr<element> pfirst;							
	shared_ptr<element> pend;
	T flag;

public:
	qofe():pfirst(nullptr),pend(nullptr)		//Ĭ�Ϲ��캯��
	{}

	//�����뷵��λ�ñ�־
	void setflag(T i)
	{
		flag = i;
	}
	T getflag() const
	{
		return flag;
	}
	
	//���ض�βԪ�ص�ָ��
	shared_ptr<element> getend()	const		
	{
		return pend;
	}

	//�ڶ�β����һ��Ԫ��
	void push_back(int timeid,const vector<Point> &elem)			
	{
		if (pfirst == nullptr)
		{
			pfirst = make_shared<element>(timeid, elem);
			pend = pfirst;
			pfirst->plast = nullptr;
			return;
		}
		pfirst->plast = make_shared<element>(timeid, elem);
		pfirst = pfirst->plast;
		pfirst->plast = nullptr;
	}

	//���أ��ڶ�β����һ��Ԫ��
	void push_back(const element &e)					
	{
		if (pfirst == nullptr)
		{
			pfirst = make_shared<element>(e);
			pend = pfirst;
			pfirst->plast = nullptr;
			return;
		}
		pfirst->plast = make_shared<element>(e);
		pfirst = pfirst->plast;
		pfirst->plast = nullptr;
	}

	//�������׵�һ��Ԫ��
	void pop()
	{
		if (pend != nullptr)
		{
			pend = pend->plast;
			if (pend == nullptr)
				pfirst = nullptr;
		}
	}

	void insert(int,const vector<Point> &);		//��ʱ��˳�����һ��Ԫ��
	void insert(const qofe &);					//�ڶ����в�����һ������
	int numofe()	const;						//���ض�������ЧԪ�صĸ���
	bool isempty()	const;						//���ض����Ƿ�Ϊ��
	vector<vector<Point>> v_return() const;		//���ض����������(����timeid)
};


class feature
{
public:
	feature() :name(), func(nullptr)
	{}
	feature(string str, vector<double>(*f)(const vector<Point> &)) :name(str), func(f)
	{}
	void updatedata(const vector<qofe<Point>> &);
	vector<vector<pair<int, vector<double>>>> data;

private:
	string name;
	vector<double> (*func)(const vector<Point> &);

};


class temporaldatam
{
public:
	temporaldatam(int bufferlength = 20, int hz = 1) :que(), featurectrl(), idtime(0), qlength(bufferlength), Hz(hz)	//��ʼ������
	{}

	typedef vector<vector<pair<int, vector<double>>>> fstruct;
	//##################
	//	�ӿ�
	//##################

	//��������
	bool addf(string name, vector<double>(*func)(const vector<Point> &));				//�������Ϊname������
	bool delf(string);																	//ɾ������Ϊname������
	vector<vector<pair<int, vector<double>>>> return_f(int flag, string name = NULL);	//��������Ϊname����������

	//����ʱ�������
	void update(const vector<vector<Point>> &);											//�����������������״̬
	vector<vector<Point>> v_return(int n) const;												//���ص�n����������

private:
	int qlength;															//�������еĴ��ڳ���
	int Hz;																	//����Ƶ�ʣ�Hz��ͼƬȡһ��
	int nhz = 0;
	vector<qofe<Point>> que;
	map<string, feature> featurectrl;

	typedef vector<pair<Point, vector<element>>> insertdata;
	int idtime;

	//���й���
	void addq(element vque);												//����һ������
	bool deleteq(Point keyid);												//ɾ��keyΪkeyid�Ķ���,ɾ���ɹ�����true��û��keyid����false
	void push_back(const insertdata &);										//�����ж�βѹ��һ������
	void pop(int, int);														//�����ж��׵���һ������
	void mergeq(qofe<Point> &output, qofe<Point> &input);					//�ں��������У�output=output+input
	bool isnear(qofe<Point> &output, qofe<Point> &input);					//�ж��������е�key�Ƿ��㹻��������㹻�����ں�

	//��������
	void updatef();															//������������

	//����ʱ�������
	insertdata inputdata(const vector<vector<Point>> &inc);					//������Ŀ������������е�ÿһ��Ԫ�ؽ��в����滮
	double disofp(Point pa, Point pb);										//��������key֮��ľ���
	Point culkeyofc(const vector<Point> &p) const;							//����һ���պ�������key
};

