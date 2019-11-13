#include "flash_dec.h"

int flash_dec::detect()
{
    src = mycapture->srcuv;
    if (!src.empty())
	{
        suspiciousconf conf(3, 4, 50, 200, 0, 50);
		s_contour=get_suspicious_area(src, conf);
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
	vector<vector<Point>> pwater;

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
        if(sum>3)
        {
            pwater.push_back(temporalctrl.getlastcounter(index));
            result = 1;
        }
        index++;
        f1.pop();
    }

//	for (int n = 0;n < f1.size();++n)
//	{
//		int sum = 0;
//		int lasttimeid = -2;
//		double last = 0;

//        for (auto &i : f1[n])
//		{
//			//if (i.first != lasttimeid && (i.second[0] - last > 100 || i.second[0] - last < -100))
//				++sum;
//			lasttimeid = i.first;
//			last = i.second[0];
//		}
//		if (sum > 3)
//		{
//			vector<vector<Point>> pall = temporalctrl.v_return(n);
//			vector<Point> vstep;
//			for (auto &i : pall)
//			{
//				if (i.size() != 0)
//					vstep = i;
//			}
//			pwater.push_back(vstep);
//			result = 1;
//		}
//	}

//	Mat drawing(TH.size(), CV_8UC3, cv::Scalar(255, 255, 255));
//	drawContours(drawing, contours, -1, Scalar(0, 0, 0), 1);
//	drawContours(drawing, pwater, -1, Scalar(0, 0, 255), 1);
//	imshow("result", drawing);

    result_pic=src.clone();
    for(auto v : pwater)
    {
        cout<<pwater.size();
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
