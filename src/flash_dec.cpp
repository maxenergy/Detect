#include "flash_dec.h"

void flash_dec::detect()
{
	temporalctrl.addf("area", f_num);
	src = mycapture.getframe();
	while (!src.empty())
	{
		src = mycapture.getframe();
		suspiciousconf conf(1, 6, 50, 200, 0, 50);
		s_contour=get_suspicious_area(src, conf);
		temporalctrl.update(s_contour);

		f1 = temporalctrl.return_f(0, "area");
		cout << "####################################################" << endl;
		cout << "����area��" << endl;
		for (auto i : f1)
		{
			for (auto i2 : i)
			{
				cout << i2.first << ":" << i2.second[0] << "	";
			}
			cout << endl;
		}
		cout << "####################################################" << endl;
		//imshow("src", src);
		failure_alarm_flag = faultdetect();
		cout << "�豸״̬����Ϊ��" << failure_alarm_flag << endl << endl;
		cvWaitKey(20);
	}

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
		for (auto &i : f1[n])					//��������仯�����ȷ���Ƿ�����ˮ���򣬿��ܻ���Ҫ����
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

	Mat drawing(TH.size(), CV_8UC3, cv::Scalar(255, 255, 255));
	drawContours(drawing, contours, -1, Scalar(0, 0, 0), 1);
	drawContours(drawing, pwater, -1, Scalar(0, 0, 255), 1);
	imshow("result", drawing);
	return result;
}

vector<double> f_num(const vector<Point> &c)
{
	vector<double> result;
	result.push_back(1);
	return result;
}