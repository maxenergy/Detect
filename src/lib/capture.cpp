#include "capture.h"

mutex keyrgb, keyir, keyuv, keytp;
bool trgb = true, tir = true, tuv = true, ttp = true;

Point centerofV(const vector<Point> &p)
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

Mat capture::getframe()
{
	if (capture_mode == 0)
	{
        string str;
        getline(txtfile, str);
        if (!str.empty())
        {
            char c_str[str.size()];
            int nf=0;
            for(auto ptr=str.begin();ptr!=str.end()-1;ptr++)
                c_str[nf++]=*ptr;
            c_str[nf]='\0';
            current_mat = imread(c_str);
            return current_mat;
        }
        else
			return Mat();
	}
	else if (capture_mode == 1)
		return current_mat;
	else if (capture_mode == 2)
	{
		if (vcapture.read(current_mat))
			return current_mat;
		else
			return Mat();
	}
}

void capture::lock()
{
	trgb = false;
	tir = false;
	tuv = false;
	ttp = false;

	keyrgb.lock();
	keyir.lock();
	keyuv.lock();
	keytp.lock();
}

void capture::unlock()
{
	keyrgb.unlock();
	keyir.unlock();
	keyuv.unlock();
	keytp.unlock();

	trgb = true;
	tir = true;
	tuv = true;
	ttp = true;
}

double **basedec::Graytodigit(Mat inputMat, double tmax, double tmin)
{
	Mat inputMatG;
	cvtColor(inputMat, inputMatG, CV_RGB2GRAY);

	int nl = inputMatG.rows;
	int nc = inputMatG.cols;

	double **ptr = new double*[nl];
	for (int i = 0;i < nl;i++)
		ptr[i] = new double[nc];

	for (int y = 29;y < 209;y++)
	{
		pair<Vec3b, double> pa(inputMat.at<Vec3b>(y, 310), tmax - (y - 29)*(tmax - tmin) / (208 - 29));
		mapoft.push_back(pa);
	}

	Vec3b pix;
	for (int j = 0;j < nl;j++)
	{
		for (int i = 0;i < nc;i++)
		{
			double dist = 10000;
			double T = 0;
			pix = inputMat.at<Vec3b>(j, i);
			int B = pix.val[0];
			int G = pix.val[1];
			int R = pix.val[2];
			for (auto i : mapoft)
			{
				double dis = sqrt((B - i.first.val[0])*(B - i.first.val[0]) + (G - i.first.val[1])*(G - i.first.val[1]) + (R - i.first.val[2])*(R - i.first.val[2]));
				if (dis < dist)
				{
					dist = dis;
					T = i.second;
				}
			}
			ptr[j][i] = T;
		}
	}

	//310 29		310 208

	return ptr;
}

double basedec::gettdev(Mat src,Point point)
{
	if (point.x<2 || point.x>src.cols - 1 || point.y<2 || point.y>src.rows - 1)
		return -1;
	Vec3b pix[9];
	pix[0] = src.at<Vec3b>(point.y, point.x);
	pix[1] = src.at<Vec3b>(point.y, point.x - 1);
	pix[2] = src.at<Vec3b>(point.y - 1, point.x - 1);
	pix[3] = src.at<Vec3b>(point.y + 1, point.x - 1);
	pix[4] = src.at<Vec3b>(point.y + 1, point.x);
	pix[5] = src.at<Vec3b>(point.y - 1, point.x);
	pix[6] = src.at<Vec3b>(point.y + 1, point.x + 1);
	pix[7] = src.at<Vec3b>(point.y, point.x + 1);
	pix[8] = src.at<Vec3b>(point.y - 1, point.x + 1);

	double sumT = 0;
	for (int n = 0;n < 9;n++)
	{
		int B = pix[n].val[0];
		int G = pix[n].val[1];
		int R = pix[n].val[2];
		double dist = 10000;
		double T = 0;
		for (auto i : mapoft)
		{
			double dis = sqrt((B - i.first.val[0])*(B - i.first.val[0]) + (G - i.first.val[1])*(G - i.first.val[1]) + (R - i.first.val[2])*(R - i.first.val[2]));
			if (dis < dist)
			{
				dist = dis;
				T = i.second;
			}
		}
		sumT += T;
	}
	sumT /= 9;
	return sumT;
}

vector<vector<Point>> basedec::get_suspicious_area(Mat src, suspiciousconf conf)
{
	vector<vector<Point>> sv;
	cvtColor(src, gray, CV_BGR2GRAY);

	int g_nStructRlementSize = conf.dandesize;//ƒ⁄∫Àæÿ’Ûµƒ≥ﬂ¥Á      
	Mat element = getStructuringElement(MORPH_RECT, Size(2 * g_nStructRlementSize + 1, 2 * g_nStructRlementSize + 1), Point(g_nStructRlementSize, g_nStructRlementSize));

	if (conf.dande == 1)
		dilate(gray, gray, element);	//∏Ø ¥
	else if (conf.dande == 2)
		erode(gray, gray, element);		//≈Ú’Õ
	else if (conf.dande == 3)
	{
		dilate(gray, gray, element);	//∏Ø ¥°˙≈Ú’Õ
		erode(gray, gray, element);
	}

	imshow("g_dstimage", gray);


	//‘ˆº”∂‘±»∂»
	uchar fa = conf.contrast_min;		//<50 black
	uchar fb = conf.contrast_max;		//>200 white
	double k = 255 / (fb - fa);
	for (int y = 0; y < gray.rows; y++)
	{
		for (int x = 0; x < gray.cols; x++)
		{
			if (gray.at<uchar>(y, x) < fa)
				gray.at<uchar>(y, x) = 0;
			else if (gray.at<uchar>(y, x) >fb)
				gray.at<uchar>(y, x) = 255;
			else
				gray.at<uchar>(y, x) = k*(gray.at<uchar>(y, x) - fa);
		}
	}
	
	

	imshow("src", src);
	imshow("gray", gray);
	
	if (conf.th == 0)
		threshold(gray, TH, 0, 255, CV_THRESH_OTSU);
	else
		threshold(gray, TH, conf.th, 255, THRESH_BINARY);
	
	imshow("TH", TH);

	vector<Vec4i> hierarchy;
	//≤È’“¬÷¿™
	findContours(TH, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);

	cout << "Contours: " << contours.size() << std::endl;
	vector<vector<Point>>::const_iterator itContours = contours.begin();
	for (; itContours != contours.end(); ++itContours)
	{
		//cout << "Size: " << itContours->size() << std::endl;
		if (itContours->size() > conf.minnum && itContours->size() < TH.rows * 2 + TH.cols * 2 - 20)
		{
			Point Pc = centerofV(*itContours);
			if (Pc.x > TH.cols / 5 && Pc.x<4 * TH.cols / 5 && Pc.y>TH.rows / 5 && Pc.y < 4 * TH.rows / 5)
			{
				sv.push_back(*itContours);
				//cout << "Size: " << itContours->size() << std::endl;
			}
		}
	}

	// draw black contours on white image

	Mat result(TH.size(), CV_8UC3, cv::Scalar(255, 255, 255));
	drawContours(result, contours, -1, cv::Scalar(0, 0, 0), 1);
	drawContours(result, sv, -1, Scalar(0, 0, 255), 1);
	imshow("re", result);
	cvWaitKey(0);

	//cout << sv.size() << endl;
	return sv;
}

int basedec::faultdetect() const
{
	return 0;
}
