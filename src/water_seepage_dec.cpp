#include"water_seepage_dec.h"

//测试接口，测试主要流程。
int water_seepage_dec::detect()
{
    src = mycapture->srcir;
    digitdata=(unsigned short *)mycapture->cm_pData;

    if (!src.empty())
	{
		suspiciousconf conf(1, 1, 50, 200, 210, 50);
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

		dec_w(digitdata, s_contour);

        temporalctrl.pushCounter(s_contour);

        f1 = temporalctrl.getfeature(AREA);


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
//对可疑区域进行筛选判断
void water_seepage_dec::dec_w(WORD *inputData, vector<vector<Point>> &suspicious_contour)
{
	vector<vector<Point>> result;
	for (const vector<Point>& dev_contour : suspicious_contour)
	{
		float temi = mycapture->Area_tem(dev_contour, 16, 0).first;
		float temo = mycapture->Area_tem(dev_contour, 16, 1).first;
		if (temo - temi > 0.5)
			result.push_back(dev_contour);
	}
	suspicious_contour = result;

//	vector<vector<Point>> hull(suspicious_contour.size());
//	vector<vector<Point>> hullo;
//	for (size_t i = 0; i < suspicious_contour.size(); i++)
//	{
//		convexHull(Mat(suspicious_contour[i]), hull[i], false);
//		Point Pc = centerofV(suspicious_contour[i]);
//		vector<Point> step;
//		for (int n = 0;n < hull[i].size();++n)
//		{
//			Point ph = hull[i][n];
//			if (ph.x < Pc.x)
//				ph.x = ph.x - 2;
//			else
//				ph.x = ph.x + 2;
//
//			if (ph.y < Pc.y)
//				ph.y = ph.y - 2;
//			else
//				ph.y = ph.y + 2;
//			step.push_back(ph);
//		}
//		hullo.push_back(step);
//	}
//
////	Mat raw_dist = Mat::zeros(src.size(), CV_32FC1);
//
//	int **Eptr = new int *[src.rows];
//	int **Eptro = new int *[src.rows];
//	int **Eptri = new int *[src.rows];
//	for (int n = 0;n < src.rows;n++)
//	{
//		Eptr[n] = new int[src.cols];
//		Eptro[n] = new int[src.cols];
//		Eptri[n] = new int[src.cols];
//	}
//
//	vector<vector<Point>> water_are;
//	for (size_t i = 0; i < suspicious_contour.size(); i++)
//	{
//        unsigned long sumti = 0, numi = 0;
//        unsigned long sumto = 0, numo = 0;
//        float avrti,avrto;
//		for (int row = 0; row < src.rows; row++)
//		{
//			for (int col = 0; col < src.cols; col++)
//			{
//				// 是否返回距离值，如果是false，1表示在内面，0表示在边界上，-1表示在外部，true返回实际距离,返回数据是double类型
//				Eptr[row][col] = pointPolygonTest(suspicious_contour[i], Point2f(col, row), false);
//				Eptri[row][col] = pointPolygonTest(hull[i], Point2f(col, row), false);
//				Eptro[row][col] = pointPolygonTest(hullo[i], Point2f(col, row), false);
//				
//				if (Eptr[row][col] == 1)
//				{
//                    sumti += (unsigned short)inputData[row*640+col];
//					++numi;
//				}
//				if (Eptro[row][col] == 1 && Eptri[row][col] == -1)
//				{
//                    sumto += (unsigned short)inputData[row*640+col];
//					++numo;
//				}
//				//if(Eptro[row][col]==1)
//				//	raw_dist.at<float>(row, col) = 125;
//				//if (Eptri[row][col] == 1)
//				//	raw_dist.at<float>(row, col) = 0;
//			}
//		}
//        avrti =  mycapture->Get_tem(sumti / numi);
//        avrto = mycapture->Get_tem(sumto / numo);
//
//        if (avrto > avrti&&avrto - avrti > 0.5)   // 0.5为内外温差阈值
//		{
//			water_are.push_back(suspicious_contour[i]);
//		}
//		//cout << "sumti=" << sumti << "	sumto=" << sumto << endl;
//		//imshow("raw_dist", raw_dist);
//		//cvWaitKey(0);
//	}
//	/*Mat drawing(TH.size(), CV_8UC3, cv::Scalar(255, 255, 255));
//	drawContours(drawing, suspicious_contour, -1, Scalar(0, 0, 0), 1);
//	drawContours(drawing, water_are, -1, Scalar(0, 0, 255), 2);
//	imshow("Hull", drawing);*/
//	suspicious_contour = water_are;
//
//	for (int n = 0;n < src.rows;n++)
//	{
//		delete[] Eptr[n];
//		delete[] Eptro[n];
//		delete[] Eptri[n];
//	}
//	delete[] Eptr;
//	delete[] Eptro;
//	delete[] Eptri;
}
//返回值：0正常；1疑似渗水
int water_seepage_dec::faultdetect()
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

        while(!i.empty())               //根据面积变化的情况确定是否是渗水区域，可能还需要调整
        {
            int th=100;
            if(t.front()!=lasttimeid&& (i.front() - last > th || i.front() - last < -th))
                ++sum;
            lasttimeid=t.front();
            last=i.front();
            t.pop();
            i.pop();
        }

        if(logout)
            logfile<<"      sum is: "<<sum<<"\n";
        if(sum>2)
        {
            pwater.push_back(temporalctrl.getlastcounter(index));
            result = 1;
        }
        index++;
        f1.pop();
    }

    result_pic=src.clone();
    for(auto v : pwater)
    {
        Rect rect = boundingRect(v);
        rectangle(result_pic, rect, Scalar(0, 0, 255));
    }

	return result;
}


