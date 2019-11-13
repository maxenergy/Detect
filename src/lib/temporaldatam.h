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

struct element
{
	element(int it, const vector<Point> &iv) :timeid(it), value(iv), plast(NULL)		//元素节点初始化
	{}
	element(const element &e) :timeid(e.timeid), value(e.value), plast(NULL)
	{}
	int timeid;						//时间标识
	vector<Point> value;			//一个闭合轮廓
	shared_ptr<element> plast;		//指向上一个元素的指针
};

template<class T>
class qofe
{
private:
	shared_ptr<element> pfirst;							
	shared_ptr<element> pend;
	T flag;

public:
	qofe():pfirst(nullptr),pend(nullptr)		//默认构造函数
	{}

	//设置与返回位置标志
	void setflag(T i)
	{
		flag = i;
	}
	T getflag() const
	{
		return flag;
	}
	
	//返回队尾元素的指针
	shared_ptr<element> getend()	const		
	{
		return pend;
	}

	//在队尾插入一个元素
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

	//重载，在队尾插入一个元素
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

	//弹出队首的一个元素
	void pop()
	{
		if (pend != nullptr)
		{
			pend = pend->plast;
			if (pend == nullptr)
				pfirst = nullptr;
		}
	}

	void insert(int,const vector<Point> &);		//按时间顺序插入一个元素
	void insert(const qofe &);					//在队列中插入另一个队列
	int numofe()	const;						//返回队列中有效元素的个数
	bool isempty()	const;						//返回队列是否为空
	vector<vector<Point>> v_return() const;		//返回队列里的轮廓(不含timeid)
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
	temporaldatam(int bufferlength = 20, int hz = 1) :que(), featurectrl(), idtime(0), qlength(bufferlength), Hz(hz)	//初始化函数
	{}

	typedef vector<vector<pair<int, vector<double>>>> fstruct;
	//##################
	//	接口
	//##################

	//特征管理
	bool addf(string name, vector<double>(*func)(const vector<Point> &));				//添加名字为name的特征
	bool delf(string);																	//删除名字为name的特征
	vector<vector<pair<int, vector<double>>>> return_f(int flag, string name = NULL);	//返回名字为name的特征数据

	//更新时序管理类
	void update(const vector<vector<Point>> &);											//输入可疑轮廓，更新状态
    vector<vector<Point>> v_return(int n) const;                                        //返回第n个轮廓集合

private:
	int qlength;															//轮廓序列的窗口长度
	int Hz;																	//采样频率：Hz张图片取一张
	int nhz = 0;
	vector<qofe<Point>> que;
	map<string, feature> featurectrl;

	typedef vector<pair<Point, vector<element>>> insertdata;
	int idtime;

	//队列管理
	void addq(element vque);												//增加一个队列
	bool deleteq(Point keyid);												//删除key为keyid的队列,删除成功返回true，没有keyid返回false
	void push_back(const insertdata &);										//在所有队尾压入一个序列
	void pop(int, int);														//在所有队首弹出一个序列
	void mergeq(qofe<Point> &output, qofe<Point> &input);					//融合两个队列，output=output+input
	bool isnear(qofe<Point> &output, qofe<Point> &input);					//判断两个队列的key是否足够近，如果足够近则融合

	//特征管理
	void updatef();															//更新所有特征

	//更新时序管理类
	insertdata inputdata(const vector<vector<Point>> &inc);					//对输入的可疑轮廓集合中的每一个元素进行操作规划
	double disofp(Point pa, Point pb);										//返回两个key之间的距离
	Point culkeyofc(const vector<Point> &p) const;							//计算一个闭合轮廓的key
};

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
