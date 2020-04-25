#include"water_seepage_dec.h"

//测试接口，测试主要流程。
int water_seepage_dec::detect()
{
	alctrl.update();
    src = mycapture->srcir;
    digitdata=(unsigned short *)mycapture->cm_pData;

    if (!src.empty())
	{
        logout=false;
        suspiciousconf conf(1, 3, 50, 200, 210, 25);
		s_contour = get_suspicious_area(src, conf);


		dec_w(digitdata, s_contour);

        Mat result(TH.size(), CV_8UC3, cv::Scalar(255, 255, 255));
        drawContours(result, s_contour, -1, cv::Scalar(0, 0, 255), 3);
        imshow("filter",result);



        if(logout)
        {
            logfile<<"##### begin detect    ######    <- "<<temporalctrl.gettimestamp_now()<<"\n";
            logfile<<"s_counter size is: "<<s_contour.size()<<"\n";
        }

        temporalctrl.pushCounter(s_contour);

        f1 = temporalctrl.getfeature(AREA);

		failure_alarm_flag = faultdetect();

        if(failure_alarm_flag!=0)
            ;//temporalctrl.clear();
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
//对可疑区域进行筛选判断
void water_seepage_dec::dec_w(WORD *inputData, vector<vector<Point>> &suspicious_contour)
{
	vector<vector<Point>> result;

    Mat resultm(TH.size(), CV_8UC3, cv::Scalar(255, 255, 255));
    drawContours(resultm, contours, -1, cv::Scalar(0, 0, 0), 1);
    vector<vector<Point>> test;

	for (const vector<Point>& dev_contour : suspicious_contour)
	{
        float temi = mycapture->Area_tem(dev_contour, 10, 0).first;
        float temo = mycapture->Area_tem(dev_contour, 15, 1).first;

        if (temo - temi > 2){
            logout=true;

            logfile<<"temo "<<temo<<"  temi    "<<temi<<endl;
			result.push_back(dev_contour);
        }
	}
    suspicious_contour = result;

    drawContours(resultm, test, -1, Scalar(0, 0, 255), 1);
    //imshow("dec_w",resultm);
}
//返回值：0正常；1疑似渗水
int water_seepage_dec::faultdetect()
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

        while(!i.empty())               //根据面积变化的情况确定是否是渗水区域，可能还需要调整
        {
            int th=16;
            if(t.front()!=lasttimeid&& (i.front() - last >= th || i.front() - last <= -th))
                ++sum;
            lasttimeid=t.front();
            last=i.front();
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
        Rect rect = boundingRect(v);
        rectangle(result_pic, rect, Scalar(0, 0, 255));
    }

	return result;
}


