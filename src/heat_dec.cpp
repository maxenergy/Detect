#include"heat_dec.h"


//读取配置文件，导入警报阈值
void heat_dec::readxml(string filename)
{
    TiXmlDocument* Document = new TiXmlDocument();
    if (!Document->LoadFile(filename.c_str()))
    {
        cerr << "无法加载xml文件！" << endl;
        return;
    }
    TiXmlElement* RootElement = Document->RootElement();
    TiXmlElement* NextElement = RootElement->FirstChildElement();

    int n = 0;
    for (;NextElement != NULL;NextElement = NextElement->NextSiblingElement())
    {
        TiXmlElement* RuleElement = NextElement->FirstChildElement();
        for (;RuleElement != NULL;RuleElement = RuleElement->NextSiblingElement())
        {
            const char* arr = RuleElement->Attribute("type");
            TiXmlElement *floorlimit = RuleElement->FirstChildElement();
            float floor_limit = atof(floorlimit->GetText());
            TiXmlElement *uplimit = floorlimit->NextSiblingElement();
            float up_limit = atof(uplimit->GetText());

            if (string(arr) == "tdev")
                tdev_range[n].push_back(pair<double, double>(floor_limit, up_limit));
            if (string(arr) == "trise")
                trise_range[n].push_back(pair<double, double>(floor_limit, up_limit));
            if (string(arr) == "tdifference")
                tdifference_range[n].push_back(pair<double, double>(floor_limit, up_limit));
            if (string(arr) == "trd")
                trd_range[n].push_back(pair<double, double>(floor_limit, up_limit));
        }
        ++n;
    }

    delete Document;
}

//测试接口，测试主要流程。
int heat_dec::detect()
{
    ++frame_num;
    src=mycapture->srcir;
    digitdata=(unsigned short *)mycapture->cm_pData;
    tenv = 18.3;        //环境温度

    if(!src.empty())
    {
        suspiciousconf conf(1, 1, 0, 255, 100, 50);
        s_contour = get_suspicious_area(src, conf);
        if (s_contour.size()>0)
            logout=true;
        else
            logout=false;

        if(logout)
        {
            logfile<<"##### begin detect    ######    <- "<<frame_num<<"\n";
            logfile<<"s_counter size is: "<<s_contour.size()<<"\n";
            logfile<<"环境温度为: "<<tenv<<"\n";
        }
        if (s_contour.size() > 0)
            culate(digitdata, s_contour); //对所有可疑区域，分别计算温度特征

        failure_alarm_flag=faultdetect();   //检测各个温度特征是否超标

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

//计算设备的温度特征值。
void heat_dec::culate(WORD *inputData,const vector<vector<Point>> &dev_contours)
{

    //Mat raw_dist = Mat::zeros(src.size(), CV_32FC1);
    //int **Eptr = new int *[src.rows];
    //for (int n = 0;n < src.rows;n++)
    //    Eptr[n] = new int[src.cols];

    for (const vector<Point> &dev_contour : dev_contours)
    {
    //    for (int row = 0; row < src.rows; row++)
    //    {
    //        for (int col = 0; col < src.cols; col++)
    //        {
    //            // 是否返回距离值，如果是false，1表示在内面，0表示在边界上，-1表示在外部，true返回实际距离,返回数据是double类型
    //            double	dist = pointPolygonTest(dev_contour, Point2f(col, row), false);
    //            Eptr[row][col] = dist;
    //            //cout << Eptr[row][col] << "	";
    //            raw_dist.at<float>(row, col) = static_cast<float>(dist);
    //        }
    //        //cout << endl;
    //    }
    //    //imshow("raw_dist", raw_dist);
    //    vector <pair<Point, unsigned short>> topk;
    //    for (int row = 0; row < src.rows; row++)
    //        for (int col = 0; col < src.cols; col++)
    //            if (Eptr[row][col] == 1)
    //                topk.push_back(pair<Point, unsigned short>(Point(col, row), (unsigned short)inputData[row*640+col]));

    //    sort(topk.begin(), topk.end(), [](pair<Point, double> x, pair<Point, double> y) { return x.second > y.second; });
    //    int nnum = 0;
    //    unsigned long Tsum = 0;
    //    int nsum = 0;
    //    Point center;
    //    for (auto ptr = topk.begin();ptr != topk.end() && nnum < 30;ptr++)
    //    {
    //        Tsum += ptr->second;
    //        center += ptr->first;
    //        ++nsum;
    //    }

        //if (nsum != 0)
        //{
            Tconf tc;
            /*tc.position = center / nsum;
            tc.tdev = mycapture->Get_tem(Tsum / nsum);*/
			pair<float, Point> step = mycapture->Area_tem(dev_contour, 30, 1);
			tc.position = step.second;
			tc.tdev = step.first;
            tc.tother = tenv + 2;
            logfile << "   位置为：" << tc.position << "\n";
            logfile << "   设备温度为：" << tc.tdev << "℃" << "\n";


            tc.temperature_rise = tc.tdev - tenv;
            tc.temperature_difference = tc.tdev - tc.tother;
            tc.relative_temperature_difference = tc.temperature_difference / tc.temperature_rise;
            logfile << "   设备温升为：" << tc.temperature_rise << "℃" << "\n";
            logfile << "   设备温差为：" << tc.temperature_difference << "℃" << "\n";
            logfile << "   设备相对温差为：" << tc.relative_temperature_difference << "\n\n";

            tconf.push_back(tc);
        //}
    }

    //for (int n = 0;n < src.rows;n++)
    //    delete[] Eptr[n];
    //delete[] Eptr;
}

//返回值：0正常；1一般缺陷；2严重缺陷；3危机缺陷
int heat_dec::faultdetect()
{
    cout << endl;
    result_pic=src.clone();
    for (Tconf itc : tconf)
    {
        for (int n = 0;n < 3;n++)
        {
            for (auto i : tdev_range[n])
                if (itc.tdev > i.first && itc.tdev < i.second){
                    drawre(itc);
                    return n + 1;
                }
            for (auto i : trise_range[n])
                if (itc.temperature_rise > i.first&&itc.temperature_rise < i.second){
                    drawre(itc);
                    return n + 1;
                }
            for (auto i : tdifference_range[n])
                if (itc.temperature_difference > i.first&&itc.temperature_difference < i.second){
                    drawre(itc);
                    return n + 1;
                }
            for (auto i : trd_range[n])
                if (itc.relative_temperature_difference > i.first&&itc.relative_temperature_difference < i.second){
                    drawre(itc);
                    return n + 1;
                }
        }
    }
    return 0;
}

void heat_dec::drawre(Tconf &tf)
{
    Rect rect;
    rect.x=tf.position.x-50;
    rect.y=tf.position.y-50;
    rect.width=100;
    rect.height=100;
    if(rect.x<=0)
        rect.x=1;
    if(rect.x>=640)
        rect.x=539;
    if(rect.y<=0)
        rect.y=1;
    if(rect.y>=480)
        rect.y=379;
    rectangle(result_pic, rect, Scalar(0, 0, 255));
}

