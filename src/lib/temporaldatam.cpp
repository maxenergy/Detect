#include"temporaldatam.h"

/*
		[[[[[[[		←	pfirst队首（入）
		[[[[[[[
		[[[[[[[
		[[[[[[[
		[[[[[[[
		[[[[[[[		←	pend队尾（出）
*/



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


void temporaldatam_v2::pushCounter(const vector<Counter> & vc)
{
	hz_cur = (hz_cur + 1) % Hz;
	if (hz_cur != 0)
		return;

    for (auto &c : vc)		//循环添加可疑轮廓集合中的每一个轮廓
    {
        int dis = 99999;
        Que *ptrq = nullptr;
        for (auto &q : que)
        {
            int d = distense_label(q.label, cul_label(c));
            if (d < dis)
            {
                dis = d;
                ptrq = &q;
            }
        }
        if (ptrq == nullptr || dis > Distense*Distense)	//	如果无法把轮廓归队	创建新队列
        {
            Que newq;

            newq.counters.push(c);
            newq.timestamp.push(timestamp_now);
            if (ISF(AREA))
                newq.area.push(f_area(c));
            if (ISF(PERIMETER))
                newq.perimeter.push(f_perimeter(c));
            if (ISF(CIRCLE))
                newq.circle.push(f_circle(c));
            if (ISF(NUM))
                newq.num.push(f_num(c));
            if (ISF(COC))
                newq.cofc.push(f_cofc(c));
            newq.label = cul_label(c);

            que.push_back(newq);
        }
        else							//	如果可以把轮廓归队	归队
        {
            ptrq->counters.push(c);
            ptrq->timestamp.push(timestamp_now);
            if (ISF(AREA))
                ptrq->area.push(f_area(c));
            if (ISF(PERIMETER))
                ptrq->perimeter.push(f_perimeter(c));
            if (ISF(CIRCLE))
                ptrq->circle.push(f_circle(c));
            if (ISF(NUM))
                ptrq->num.push(f_num(c));
            if (ISF(COC))
                ptrq->cofc.push(f_cofc(c));
            ptrq->label = ptrq->label*0.34 + cul_label(c)*0.66;
        }
    }
    update();
}

int temporaldatam_v2::update()
{
    int mode = 0;
    timestamp_now++;

    //	如果队列太长了就pop
    for (auto &q : que)
    {
        if (q.timestamp.front() < timestamp_now - qlength)
        {
            q.counters.pop();
            q.timestamp.pop();
            if (ISF(AREA))
                q.area.pop();
            if (ISF(PERIMETER))
                q.perimeter.pop();
            if (ISF(CIRCLE))
                q.circle.pop();
            if (ISF(NUM))
                q.num.pop();
            if (ISF(COC))
                q.cofc.pop();
        }
    }

    //	如果队列为空就删除
    auto p = que.begin();
    while (p != que.end())
    {
        if (p->counters.empty())
        {
            p = que.erase(p);
            continue;
        }
        p++;
    }

    //	如果两个队列太近了就删除元素少的
    for (int n = 0;n < que.size();n++)
        for (int m = n + 1; m < que.size();m++)
            if (distense_label(que[n].label, que[m].label) <= Distense*Distense) {
                cout << "发生碰撞" << endl;
                if (que[n].counters.size() > que[m].counters.size()) {
                    que.erase(que.begin() + m);
                    m--;
                }
                else {
                    que.erase(que.begin() + n);
                    n--;
                    break;
                }
            }


    return mode;
}

temporaldatam_v2::Label temporaldatam_v2::cul_label(const Counter &p) const
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


void alarm_ctrl::updata()
{
    if(alarm_time.isempty())
        return;

    state_mes now;
    now.settime_now();
    record_time cur_time(now.year,now.mon,now.day,now.hour,now.min,now.sec-wait_time);

    if(cur_time<=alarm_time)
        return;
    state_mes mes;
    mes.getfromfile(mesname);

}

void alarm_ctrl::savefault(state_mes mes,Mat rgb,Mat ir,Mat uv,string basefile)
{
    state_mes now;
    now.settime_now();
    record_time cur_time(now.year,now.mon,now.day,now.hour,now.min,now.sec);

    if(cur_time < stay_time || !alarm_time.isempty())
        return;

    alarm_time=cur_time;
    string filename=mes.tostring();
    string command = "mkdir -p " + basefile + filename;
    system(command.c_str());

    mesname = basefile + filename+"/"+filename+"_mes.txt";
    rgbname = basefile + filename+"/"+filename+"_rgb.jpg";
    irname = basefile + filename+"/"+filename+"_ir.jpg";
    uvname = basefile + filename+"/"+filename+"_uv.jpg";
    vedioname = basefile + filename+"/"+filename+"_vedio.mp4";

    mes.save(mesname);
    imwrite(rgbname,rgb);
    imwrite(irname,ir);
    imwrite(uvname,uv);
}



