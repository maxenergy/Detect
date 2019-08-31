#include"fire_dec.h"

void fire_dec::detect()
{
	src = mycapture->getframe();
	temporalctrl.addf("area", f_area);
	temporalctrl.addf("perimeter", f_perimeter);
	temporalctrl.addf("circle", f_circle);
	temporalctrl.addf("cofc", f_cofc);
	//digitdata = Graytodigit(src, 41, 31);
	//double centerT = gettdev(160, 120);
	//cout << centerT;
	while (!src.empty())
	{
		imshow("src", src);
		suspiciousconf conf(0, 0, 50, 200, 0, 60);
		s_contour = get_suspicious_area(src, conf);
		temporalctrl.update(s_contour);

		f1 = temporalctrl.return_f(0, "area");
		f2 = temporalctrl.return_f(0, "perimeter");
		f3 = temporalctrl.return_f(0, "circle");
		f4 = temporalctrl.return_f(0, "cofc");

		//cout << "####################################################" << endl;
		//cout << "特征:"<<"cofc" << endl;
		//for (auto i : f3)
		//{
		//	for (auto i2 : i)
		//	{
		//		cout << i2.first << ":" << i2.second[0] << "	";
		//	}
		//	cout << endl;
		//}
		//cout << "####################################################" << endl;

		failure_alarm_flag = faultdetect();
		cout << "设备状态类型为：" << failure_alarm_flag << endl << endl;

		cvWaitKey(20);
		src = mycapture->getframe();
	}
}

int fire_dec::faultdetect() const
{
	int result = 0;
	vector<vector<Point>> pwater;
	for (int n = 0;n < f1.size();++n)
	{
		int sum[4] = { 0 };
		int lasttimeid[3] = { -2 };
		double last[3] = { 0 };
		for (auto &i : f1[n])					//根据面积变化的情况确定是否是渗水区域，可能还需要调整
		{
			double th = 30;
			if (i.first != lasttimeid[0] && (i.second[0] - last[0] > th || i.second[0] - last[0] < -th))
				++sum[0];
			lasttimeid[0] = i.first;
			last[0] = i.second[0];
		}
		for (auto &i : f2[n])					//根据周长变化的情况确定是否是渗水区域，可能还需要调整
		{
			double th = 8;
			if (i.first != lasttimeid[1] && (i.second[0] - last[1] > th || i.second[0] - last[1] < -th))
				++sum[1];
			lasttimeid[1] = i.first;
			last[1] = i.second[0];
		}
		for (auto &i : f3[n])					//根据似圆度变化的情况确定是否是渗水区域，可能还需要调整
		{
			double th = 0.02;
			if (i.first != lasttimeid[2] && (i.second[0] - last[2] > th || i.second[0] - last[2] < -th))
				++sum[2];

			lasttimeid[2] = i.first;
			last[2] = i.second[0];
		}
		for (auto ptr1 = f4[n].begin(), ptr2 = f4[n].begin()++;ptr1 != f4[n].end();ptr1++)
		{
			double th = 0.003;
			if (++ptr2 == f4[n].end())
				break;
			//cout << "culdis "<<n<<"	" << culdis(ptr1->second, ptr2->second) << endl;
			if (culdis(ptr1->second, ptr2->second) > th)
				++sum[3];
		}




		int th = 3;
		if (sum[0] > th && sum[1] > th && sum[2] > th && sum[3] > th )			//符合条件，判断发生故障，将最后一个轮廓绘制出来。
		{
			vector<vector<Point>> pall = temporalctrl.v_return(n);
			vector<vector<Point>>::const_iterator vstep;
			for (auto i = pall.begin();i != pall.end();i++)
			{
				if (i->size() != 0)
					vstep = i;
			}
			pwater.push_back(*vstep);
			result = 1;
		}
	}

	Mat drawing(TH.size(), CV_8UC3, cv::Scalar(255, 255, 255));
	drawContours(drawing, contours, -1, Scalar(0, 0, 0), 1);
	drawContours(drawing, pwater, -1, Scalar(0, 0, 255), 1);
	imshow("result", drawing);
	return result;
}



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
