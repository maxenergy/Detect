#include "flash_dec.h"

int flash_dec::detect()
{
    src = mycapture->srcuv;
    if (!src.empty())
	{
		suspiciousconf conf(1, 6, 50, 200, 0, 50);
		s_contour=get_suspicious_area(src, conf);
		temporalctrl.update(s_contour);

		f1 = temporalctrl.return_f(0, "area");

//		cout << "####################################################" << endl;
//		cout << "特征area：" << endl;
//		for (auto i : f1)
//		{
//			for (auto i2 : i)
//			{
//				cout << i2.first << ":" << i2.second[0] << "	";
//			}
//			cout << endl;
//		}
//		cout << "####################################################" << endl;

		failure_alarm_flag = faultdetect();
		cout << "设备状态类型为：" << failure_alarm_flag << endl << endl;
        cvWaitKey(20);
        return failure_alarm_flag;
	}
    return 0;

}

int flash_dec::faultdetect()
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
			//if (i.first != lasttimeid && (i.second[0] - last > 100 || i.second[0] - last < -100))
				++sum;
			lasttimeid = i.first;
			last = i.second[0];
		}
		if (sum > 3)
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

//	Mat drawing(TH.size(), CV_8UC3, cv::Scalar(255, 255, 255));
//	drawContours(drawing, contours, -1, Scalar(0, 0, 0), 1);
//	drawContours(drawing, pwater, -1, Scalar(0, 0, 255), 1);
//	imshow("result", drawing);

    result_pic=src.clone();
    for(auto v : pwater)
    {
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
