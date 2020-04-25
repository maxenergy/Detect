#include "flash_dec.h"

int flash_dec::detect()
{
	alctrl.update();
    src = mycapture->srcuv;
    if (!src.empty())
	{
        suspiciousconf conf(3, 10, 50, 200, 0, 50);
		s_contour=get_suspicious_area(src, conf);
        if (s_contour.size()>0)
            logout=true;
        else
            logout=false;

        if(logout)
        {
            logfile<<"##### begin detect    ######    <- "<<temporalctrl.gettimestamp_now()<<"\n";
            logfile<<"s_counter size is: "<<s_contour.size()<<"\n";
        }

        temporalctrl.pushCounter(s_contour);

        f1 = temporalctrl.getfeature(NUM);

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

int flash_dec::faultdetect()
{
	int result = 0;
	result_counters.clear();

    if(logout)
        logfile<<"counters size is: "<<f1.size()<<"\n";

    int index=0;
    while(!f1.empty())
    {
        int sum = 0;
        unsigned long lasttimeid = 0;
        double last = 0;

        queue<unsigned long> &t=f1.front().first;
        queue<double> &i=f1.front().second;
        if(logout)
            logfile<<"  index "<<index<<" counter size is "<<t.size()<<"\n";
        while(!i.empty())
        {
            if(t.front()!=lasttimeid)
                ++sum;
            lasttimeid=t.front();
            last=1;
            t.pop();
            i.pop();
        }

        if(logout)
            logfile<<"      sum is: "<<sum<<"\n";
        if(sum>=2)
        {
			result_counters.push_back(temporalctrl.getlastcounter(index));
            result = 1;
        }
        index++;
        f1.pop();
    }


    result_pic=src.clone();
    for(auto v : result_counters)
    {
        //cout<<pwater.size();
        Rect rect = boundingRect(v);
        rectangle(result_pic, rect, Scalar(0, 0, 255));
    }

	return result;
}

vector<double> f_num(const vector<Point> &c)
{
	vector<double> result;
	result.push_back(1);
	return result;
}
