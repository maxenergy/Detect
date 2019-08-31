#include"temporaldatam.h"

/*
		[[[[[[[		←	pfirst队首（入）
		[[[[[[[
		[[[[[[[
		[[[[[[[
		[[[[[[[
		[[[[[[[		←	pend队尾（出）
*/

template<class T>
void qofe<T>::insert(int timeid,const vector<Point> &elem)
{
	shared_ptr<element> newe = make_shared<element> (timeid, elem);
	for (shared_ptr<element> p = pend;p != NULL;p = p->plast)
	{
		if (p->timeid == timeid)
		{
			newe->plast = p->plast;
			p->plast = newe;
			break;
		}
	}
}

template<class T>
void qofe<T>::insert(const qofe &q)
{
	for (shared_ptr<element> p = q.pend;p != NULL;p = p->plast)
	{
		if (!p->value.empty())
			insert(p->timeid, p->value);
	}
}

template<class T>
int qofe<T>::numofe() const
{
	int num = 0;
	for (shared_ptr<element> p = pend;p != NULL;p = p->plast)
		if (!p->value.empty()) ++num;
	return num;
}

template<class T>
bool qofe<T>::isempty() const
{
	for (shared_ptr<element> p = pend;p != NULL;p = p->plast)
		if (!p->value.empty()) return false;
	return true;
}

template<class T>
vector<vector<Point>> qofe<T>::v_return() const
{
	vector<vector<Point>> result;
	for (shared_ptr<element> p = pend;p != nullptr;p = p->plast)
		if(!p->value.empty())
			result.push_back(p->value);
	return result;
}


void feature::updatedata(const vector<qofe<Point>> &q)
{
	data.clear();
	for (qofe<Point> i : q)
	{
		vector<pair<int, vector<double>>> vstep;
		for (shared_ptr<element> pi = i.getend();pi != NULL;pi = pi->plast)
		{
			vstep.push_back(pair<int, vector<double>>(pi->timeid,func(pi->value)));
		}
		data.push_back(vstep);
	}
}


void temporaldatam::addq(element vque)
{
	qofe<Point> q;
//	q.second.push_back(-1, vector<Point>());
	q.push_back(vque);
	q.setflag(culkeyofc(vque.value));
	que.push_back(q);
}

bool temporaldatam::deleteq(Point keyid)
{
	for (auto p = que.begin();p != que.end();p++)
	{
		if (p->getflag() == keyid)
		{
			que.erase(p);
			return true;
		}
	}
	return false;
}

void temporaldatam::push_back(const insertdata &data)
{
	auto pofque = que.begin();
	auto pofdata = data.begin();
	for (;pofdata != data.end();pofque++, pofdata++)
		for (auto i : pofdata->second)
			pofque->push_back(i);
}

void temporaldatam::pop(int timeid,int quelength)
{
	int bridge = 0;							//要pop出的边界值
	if (timeid < quelength)
		bridge = timeid + 1000 - quelength;
	else
		bridge = timeid - quelength;

	for (auto pofque = que.begin();pofque != que.end();pofque++)
	{
		auto pstep = pofque->getend();
		if (pstep == nullptr)
			continue;
		if (pstep->timeid <= bridge&&pstep->timeid > bridge - 5)
		{
			auto pstep2 = pstep->plast;
			if (pstep2 == nullptr)
			{
				pofque->pop();
				continue;
			}

			while (pstep->timeid == pstep2->timeid)
			{
				pofque->pop();
				pstep = pofque->getend();
				pstep2 = pstep->plast;
				if (pstep2 == nullptr)
					break;
			}
			pofque->pop();
		}
	}
}

void temporaldatam::mergeq(qofe<Point> &output, qofe<Point> &input)
{
	output.insert(input);
	output.setflag((input.getflag() + output.getflag()) / 2);
}

bool temporaldatam::isnear(qofe<Point> &output, qofe<Point> &input)
{
	double dis = disofp(output.getflag(), input.getflag());
	if (dis <= Thofisnear*Thofisnear)
	{
		mergeq(output, input);
		deleteq(input.getflag());
		return true;
	}
	return false;
}

bool temporaldatam::addf(string name, vector<double>(*func)(const vector<Point> &))
{
	if (featurectrl.find(name) != featurectrl.end())
		return false;
	feature f(name, func);
	featurectrl[name] = f;
	return true;
}

bool temporaldatam::delf(string name)
{
	if (featurectrl.erase(name) == 0)
		return false;
	else
		return true;
}

void temporaldatam::updatef()
{
	for (auto &i : featurectrl)
	{
		i.second.updatedata(que);
	}
}

vector<vector<pair<int, vector<double>>>> temporaldatam::return_f(int flag, string name)
{
	if (featurectrl.find(name) != featurectrl.end())
		return featurectrl[name].data;
	return vector<vector<pair<int, vector<double>>>>();
}

Point temporaldatam::culkeyofc(const vector<Point> &p) const
{
	int sumx = 0;
	int sumy = 0;
	for (Point i : p)
	{
		sumx += i.x;
		sumy += i.y;
	}
	return Point(sumx / p.size(), sumy / p.size());
}

temporaldatam::insertdata temporaldatam::inputdata(const vector<vector<Point>> &inc)
{
	insertdata data(que.size());
	int n = 0;
	for (auto i : que)
		data[n++].first = i.getflag();
	
	for (vector<Point> i : inc)
	{
		int np = -1;
		double dis = 999999.0;
		Point pcen = culkeyofc(i);
		int nl = 0;
		for (auto &i2 : que)
		{
			double d = disofp(pcen, i2.getflag());
			if (d < dis)
			{
				np = nl;
				dis = d;
			}
			++nl;
		}
		//cout << "dis:" << dis<<endl;
		if (dis <= Thofisnear*Thofisnear)
		{
			data[np].second.push_back(element(idtime, i));
		}
		else
		{
			addq(element(idtime, i));
			cout << "增加了一个新元素，标识为：" << pcen << endl;
		}
	}
	return data;
}

void temporaldatam::update(const vector<vector<Point>> &inc)
{
	nhz = ++nhz % Hz;
	if  (nhz == 0)
	{
		insertdata data = inputdata(inc);
		cout << "que的size=" << que.size() << "  data的size=" << data.size() << endl;
		push_back(data);
		pop(idtime, qlength);
		for (int n = 0;n < que.size();n++)
		{
			if (que[n].isempty())
			{
				deleteq(que[n].getflag());
				--n;
			}
		}
		for (int n = 0;n < que.size();n++)
			for (int m = 0;m < que.size();m++)
				if (m != n)
					if (isnear(que[n], que[m]))
					{
						cout << "发生合并" << endl;
						m--;
						if (n > m) n--;
					}
		idtime = ++idtime % 1000;
		updatef();
	}
}

vector<vector<Point>> temporaldatam::v_return(int n) const
{
	qofe<Point> q = que[n];
	return q.v_return();
}

double temporaldatam::disofp(Point pa, Point pb)
{
	double dis = double(pa.x - pb.x)*double(pa.x - pb.x) + double(pa.y - pb.y)*double(pa.y - pb.y);
	return dis;
}


vector<double> f_area(const vector<Point> &c)
{
    vector<double> result;
    result.push_back(contourArea(c, false));
    return result;
}

vector<double> f_perimeter(const vector<Point> &c)
{
    vector<double> result;
    result.push_back(arcLength(c, true));
    return result;
}

vector<double> f_circle(const vector<Point> &c)
{
    vector<double> result;
    double a = contourArea(c, false);
    double p = arcLength(c, true);
    result.push_back(4 * 3.14*a / (p*p));
    return result;
}

vector<double> f_cofc(const vector<Point> &c)
{
    vector<double> result;

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
        result.push_back(qf);
    }
    return result;
}
