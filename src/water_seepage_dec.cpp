#include"water_seepage_dec.h"


//测试接口，测试主要流程。
void water_seepage_dec::detect()
{
	temporalctrl.addf("area", f_area);
	//temporalctrl.addf("perimeter", f_perimeter);
	//temporalctrl.addf("circle", f_circle);
	while (1)
	{
		src = mycapture.getframe();
		if (src.empty())
			break;
		digitdata = Graytodigit(src, 31, 24);

		suspiciousconf conf(1, 1, 50, 200, 210, 50);
		s_contour = get_suspicious_area(src, conf);

		dec_w(digitdata, s_contour);

		temporalctrl.update(s_contour);

		f1 = temporalctrl.return_f(0, "area");
		cout << "####################################################" << endl;
		cout << "特征area：" << endl;
		for (auto i : f1)
		{
			for (auto i2 : i)
			{
				cout <<i2.first <<":"<< i2.second[0] << "	";
			}
			cout << endl;
		}
		cout << "####################################################" << endl;

		failure_alarm_flag = faultdetect();

		cout << "设备状态类型为：" << failure_alarm_flag << endl << endl;

		//cvWaitKey(0);
	}
}
//对可疑区域进行筛选判断
void water_seepage_dec::dec_w(double **inputData, vector<vector<Point>> &suspicious_contour)
{
	vector<vector<Point>> hull(suspicious_contour.size());
	vector<vector<Point>> hullo;
	for (size_t i = 0; i < suspicious_contour.size(); i++)
	{
		convexHull(Mat(suspicious_contour[i]), hull[i], false);
		Point Pc = centerofV(suspicious_contour[i]);
		vector<Point> step;
		for (int n = 0;n < hull[i].size();++n)
		{
			Point ph = hull[i][n];
			if (ph.x < Pc.x)
				ph.x = ph.x - 2;
			else
				ph.x = ph.x + 2;

			if (ph.y < Pc.y)
				ph.y = ph.y - 2;
			else
				ph.y = ph.y + 2;
			step.push_back(ph);
		}
		hullo.push_back(step);
	}

//	Mat raw_dist = Mat::zeros(src.size(), CV_32FC1);

	int **Eptr = new int *[src.rows];
	int **Eptro = new int *[src.rows];
	int **Eptri = new int *[src.rows];
	for (int n = 0;n < src.rows;n++)
	{
		Eptr[n] = new int[src.cols];
		Eptro[n] = new int[src.cols];
		Eptri[n] = new int[src.cols];
	}

	vector<vector<Point>> water_are;
	for (size_t i = 0; i < suspicious_contour.size(); i++)
	{
		double sumti = 0, numi = 0;
		double sumto = 0, numo = 0;
		for (int row = 0; row < src.rows; row++)
		{
			for (int col = 0; col < src.cols; col++)
			{
				// 是否返回距离值，如果是false，1表示在内面，0表示在边界上，-1表示在外部，true返回实际距离,返回数据是double类型
				Eptr[row][col] = pointPolygonTest(suspicious_contour[i], Point2f(col, row), false);
				Eptri[row][col] = pointPolygonTest(hull[i], Point2f(col, row), false);
				Eptro[row][col] = pointPolygonTest(hullo[i], Point2f(col, row), false);
				
				if (Eptr[row][col] == 1)
				{
					sumti += inputData[row][col];
					++numi;
				}
				if (Eptro[row][col] == 1 && Eptri[row][col] == -1)
				{
					sumto += inputData[row][col];
					++numo;
				}
				//if(Eptro[row][col]==1)
				//	raw_dist.at<float>(row, col) = 125;
				//if (Eptri[row][col] == 1)
				//	raw_dist.at<float>(row, col) = 0;
			}
		}
		sumti = sumti / numi;
		sumto = sumto / numo;
		if (sumto > sumti&&sumto - sumti > 0.5)
		{
			water_are.push_back(suspicious_contour[i]);
		}
		//cout << "sumti=" << sumti << "	sumto=" << sumto << endl;
		//imshow("raw_dist", raw_dist);
		//cvWaitKey(0);
	}
	/*Mat drawing(TH.size(), CV_8UC3, cv::Scalar(255, 255, 255));
	drawContours(drawing, suspicious_contour, -1, Scalar(0, 0, 0), 1);
	drawContours(drawing, water_are, -1, Scalar(0, 0, 255), 2);
	imshow("Hull", drawing);*/
	suspicious_contour = water_are;

	for (int n = 0;n < src.rows;n++)
	{
		delete[] Eptr[n];
		delete[] Eptro[n];
		delete[] Eptri[n];
	}
	delete[] Eptr;
	delete[] Eptro;
	delete[] Eptri;
}
//返回值：0正常；1疑似渗水
int water_seepage_dec::faultdetect()
{
	int result = 0;
	vector<vector<Point>> pwater;
	for (int n = 0;n < f1.size();++n)
	{
		int sum = 0;
		int lasttimeid = -2;
		double last = 0;
		for (auto &i : f1[n])					//根据面积变化的情况确定是否是渗水区域，可能还需要调整
		{
			if (i.first != lasttimeid && (i.second[0] - last > 100 || i.second[0] - last < -100))
				++sum;
			lasttimeid = i.first;
			last = i.second[0];
		}
		if (sum > 2)
		{
			vector<vector<Point>> pall = temporalctrl.v_return(n);
			vector<Point> vstep;
			for (auto &i : pall)
			{
				if (i.size() != 0)
					vstep = i;
			}
			pwater.push_back(vstep);
			result = 1;
		}
	}

	Mat drawing(TH.size(), CV_8UC3, cv::Scalar(255, 255, 255));
	drawContours(drawing, contours, -1, Scalar(0, 0, 0), 1);
	drawContours(drawing, pwater, -1, Scalar(0, 0, 255), 1);
	imshow("result", drawing);
	return result;
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
