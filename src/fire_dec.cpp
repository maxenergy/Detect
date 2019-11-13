#include"fire_dec.h"

int fire_dec::detect()
{
    src = mycapture->srcir;
    digitdata=(unsigned short *)mycapture->cm_pData;
    if (!src.empty())
    {
        suspiciousconf conf(0, 0, 50, 200, 0, 60);
        s_contour = get_suspicious_area(src, conf);
        if (s_contour.size()>0)
            logout=true;
        else
            logout=false;

        if(logout)
        {
            logfile<<"##### begin detect    ######    <- "<<temporalctrl.gettimestamp_now()-21<<"\n";
            logfile<<"s_counter size is: "<<s_contour.size()<<"\n";
        }
        temporalctrl.pushCounter(s_contour);

        f1 = temporalctrl.getfeature(AREA);
        f2 = temporalctrl.getfeature(PERIMETER);
        f3 = temporalctrl.getfeature(CIRCLE);
        //f4 = temporalctrl.getfeature(COC);

        failure_alarm_flag = faultdetect();
        if(failure_alarm_flag!=0)
            temporalctrl.clear();
        if(logout)
        {
            logfile<<"result is: "<<failure_alarm_flag<<"\n";
            logfile<<"##### end detect    #####\n"<<endl;
        }
        return failure_alarm_flag;
    }
    if(logout)
    {
        logfile<<"src is empty \n";
        logfile<<"##### end detect    #####\n"<<endl;
    }
    return 0;
}

int fire_dec::faultdetect()
{
    int result = 0;
    vector<vector<Point>> pwater;

    if(logout)
        logfile<<"counters size is: "<<f1.size()<<"\n";

    int index=0;
    while(!f1.empty())
    {
        int sum[4] = { 0 };
        unsigned long lasttimeid[4] = { 0 };
        double last[4] = { 0 };

        queue<unsigned long> &t1=f1.front().first;
        queue<double> &i1=f1.front().second;
        if(logout)
            logfile<<"  index "<<index<<" counter size is "<<t1.size()<<"\n";
        while(!i1.empty())                      //根据面积变化的情况确定是否是明火区域，可能还需要调整
        {
            double th = 30;
            if (t1.front() != lasttimeid[0] && (i1.front() - last[0] > th || i1.front() - last[0] < -th))
                ++sum[0];
            lasttimeid[0] = t1.front();
            last[0] = i1.front();
            t1.pop();
            i1.pop();
        }

        queue<unsigned long> &t2=f2.front().first;
        queue<double> &i2=f2.front().second;
        while(!i2.empty())                      //根据周长变化的情况确定是否是明火区域，可能还需要调整
        {
            double th = 8;
            if (t2.front() != lasttimeid[1] && (i2.front() - last[1] > th || i2.front() - last[1] < -th))
                ++sum[1];
            lasttimeid[1] = t2.front();
            last[1] = i2.front();
            t2.pop();
            i2.pop();
        }

        queue<unsigned long> &t3=f3.front().first;
        queue<double> &i3=f3.front().second;
        while(!i3.empty())                      //根据似圆度变化的情况确定是否是明火区域，可能还需要调整
        {
            double th = 0.02;
            if (t3.front() != lasttimeid[2] && (i3.front() - last[2] > th || i3.front() - last[2] < -th))
                ++sum[2];

            lasttimeid[2] = t3.front();
            last[2] = i3.front();
            t3.pop();
            i3.pop();
        }

        //        queue<unsigned long> &t4=f4.front().first;
        //        queue<double> &i4=f4.front().second;
        //        while(!i4.empty())					//根据轮廓描述子变化的情况确定是否是明火区域，可能还需要调整
        //        {
        //            double th = 0.02;
        //            if (t4.front() != lasttimeid[3] && (i4.front() - last[3] > th || i4.front() - last[3] < -th))
        //                ++sum[3];

        //            lasttimeid[3] = t4.front();
        //            last[3] = i4.front();
        //            t4.pop();
        //            i4.pop();
        //        }



        if(logout)
            logfile<<"      sum is: "<<sum[0]<<"  "<<sum[1]<<"  "<<sum[2]<<"  "<<sum[3]<<"\n";
        int th = 3;
        if (sum[0] > th && sum[1] > th && sum[2] > th )//&& sum[3] > th )			//符合条件，判断发生故障，将最后一个轮廓绘制出来。
        {
            pwater.push_back(temporalctrl.getlastcounter(index));
            result = 1;
        }

        index++;
        f1.pop();
        f2.pop();
        f3.pop();
        //f4.pop();
    }


    result_pic=src.clone();
    for(auto v : pwater)
    {
        Rect rect = boundingRect(v);
        rectangle(result_pic, rect, Scalar(0, 0, 255));
    }

    return result;
}


//所有特征值的平方和开根号
double culdis(const vector<double> &v1, const vector<double> &v2)
{
    double result = 0;
    int sum = 0;
    for (auto ptr1 = v1.begin(), ptr2 = v2.begin();ptr1 != v1.end();ptr1++, ptr2++)
    {
        result += (*ptr2 - *ptr1)*(*ptr2 - *ptr1);
        sum++;
    }
    if (sum > 1)
        return sqrt(result);
    else
        return 0;
}
