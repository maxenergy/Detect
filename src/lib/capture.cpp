#include "capture.h"
#include <unistd.h>

mutex keyrgb, keyir, keyuv, keytp;
bool trgb = true, tir = true, tuv = true, ttp = true;
struct pic_rgb{};
struct pic_ir{};
struct pic_uv{};
template <class pic>
void stream_lock(pic)
{}
template <>
void stream_lock<pic_rgb>(pic_rgb)
{
    keyrgb.lock();
}
template <>
void stream_lock<pic_ir>(pic_ir)
{
    keyir.lock();
}
template <>
void stream_lock<pic_uv>(pic_uv)
{
    keyuv.lock();
}
void tm_lock()
{
    keytp.lock();
}

template <class pic>
void stream_unlock(pic)
{}
template <>
void stream_unlock<pic_rgb>(pic_rgb)
{
    keyrgb.unlock();
}
template <>
void stream_unlock<pic_ir>(pic_ir)
{
    keyir.unlock();
}
template <>
void stream_unlock<pic_uv>(pic_uv)
{
    keyuv.unlock();
}
void tm_unlock()
{
    keytp.unlock();
}

template <class pic>
bool stream_t(pic)
{}
template <>
bool stream_t<pic_rgb>(pic_rgb)
{
    return trgb;
}
template <>
bool stream_t<pic_ir>(pic_ir)
{
    return tir;
}
template <>
bool stream_t<pic_uv>(pic_uv)
{
    return tuv;
}
bool tm_t()
{
    return ttp;
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

    int g_nStructRlementSize = conf.dandesize;//内核矩阵的尺寸
    Mat element = getStructuringElement(MORPH_RECT, Size(2 * g_nStructRlementSize + 1, 2 * g_nStructRlementSize + 1), Point(g_nStructRlementSize, g_nStructRlementSize));

    if (conf.dande == 1)
        dilate(gray, gray, element);	//腐蚀
    else if (conf.dande == 2)
        erode(gray, gray, element);		//膨胀
    else if (conf.dande == 3)
    {
        dilate(gray, gray, element);	//腐蚀→膨胀
        erode(gray, gray, element);
    }

    imshow("g_dstimage", gray);


    //增加对比度
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
    //查找轮廓
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

void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    char tempbuf[256] = {0};
    switch(dwType)
    {
    case EXCEPTION_RECONNECT:
        //预览时重连
        printf("----------reconnect--------%d\n", time(NULL));
        break;
    default:
        break;
    }
}


Mat YUV_srcrgb;
Mat YUV_srcir;
Mat YUV_srcuv;
template<class pic>
void setpic(Mat src)
{}
template<>
void setpic<pic_rgb>(Mat src)
{
    YUV_srcrgb=src.clone();
}
template<>
void setpic<pic_ir>(Mat src)
{
    YUV_srcrgb=src.clone();
}
template<>
void setpic<pic_uv>(Mat src)
{
    YUV_srcrgb=src.clone();
}

template<class pic>
void CALLBACK DecCBFun(int nPort, char* pBuf, int nSize, FRAME_INFO* pFrameInfo, void *, int nReserved2)
{
    if(stream_t(pic()))
    {
        stream_lock(pic());
        if (pFrameInfo->nType == T_YV12)
        {
            Mat YUVImage(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, (unsigned char*)pBuf);
            setpic<pic>(YUVImage);
//            Mat step;
//            cvtColor(YUVImage, step, COLOR_YUV2BGR_YV12);
//            imshow("test",step);
//            cvWaitKey(10);
            YUVImage.~Mat();
        }
        stream_unlock(pic());
    }
}

LONG lPort_rgb;
LONG lPort_ir;
LONG lPort_uv;
template<class pic>
LONG &get_lPort(pic)
{}
template<>
LONG &get_lPort<pic_rgb>(pic_rgb)
{
    return lPort_rgb;
}
template<>
LONG &get_lPort<pic_ir>(pic_ir)
{
    return lPort_ir;
}
template<>
LONG &get_lPort<pic_uv>(pic_uv)
{
    return lPort_uv;
}

template<class pic>
void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize,void* dwUser)
{
    HWND hWnd=0;
    LONG &Port=get_lPort(pic());
    switch (dwDataType)
    {
    case NET_DVR_SYSHEAD: //系统头
        if (!PlayM4_GetPort(&Port))
            break;
        //第一次回调的是系统头,将获取的播放库 port 号赋值给全局 port,下次回调数据时即使用此 port 号播放
        if (dwBufSize > 0){
            if (!PlayM4_SetStreamOpenMode(Port, STREAME_REALTIME))//设置实时流播放模式
                break;
            if (!PlayM4_OpenStream(Port, pBuffer, dwBufSize, 1024*1024)) //打开流接口
                break;
            if (!PlayM4_SetDecCallBack(Port,DecCBFun<pic>))
                break;
            if (!PlayM4_Play(Port, hWnd)) //播放开始
                break;
        }
        break;
    case NET_DVR_STREAMDATA:
        //码流数据
        if (dwBufSize > 0 && Port != -1){
            if (!PlayM4_InputData(Port, pBuffer, dwBufSize))
                break;
        }
        break;
    default:
        //其他数据
        if (dwBufSize > 0 && Port != -1){
            if (!PlayM4_InputData(Port, pBuffer, dwBufSize))
                break;
        }
        break;
    }
}

NET_DEV_RAWFILEHEAD m_RawHead;
BYTE * m_pData ;
unsigned long m_dwDataSize ;
unsigned char *pTempPara;
unsigned long TempParaSize;

void CALLBACK funRealStream(int dwDataType, char* pDataBuffer, int dwDataSize,int *dwUser, short shPort)
{
    switch(dwDataType)
    {
    case AVI_HEAD:
    case AVI_DATA:
        break;
    case GRAY_HEAD://保存数据头
        if(sizeof(m_RawHead) == dwDataSize)
        {
            memcpy(&m_RawHead,pDataBuffer,dwDataSize);
        }
        break;
    case GRAY_DATA://解析数据并显示
        if(tm_t())
        {
            tm_lock();
            Temperature_GetFrameInfo(m_RawHead.nCalcType,(byte*)pDataBuffer,dwDataSize,&m_pData,&m_dwDataSize,&pTempPara,&TempParaSize);
            tm_unlock();
        }
        break;
    default:
        break;
    }
}

void capture::SDK_Init()
{
    NET_DVR_Init();
    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);

    //IR
    //    unsigned char strImageVersion[24] = {0};
    //    NetDev_GetVersion(strImageVersion);
    //    cout<<"IR SDK Version:"<<strImageVersion;
    //    char strTempVersion[24] = {0};
    //    TEMPALG_GetVersion(strTempVersion);
    //    cout<<"TEM SDK Version:"<<strTempVersion;
    //    NetDev_Init();

}

bool capture::SDK_Connect()
{
    NET_DVR_DEVICEINFO_V30 struDeviceInfo;
    lUserID = NET_DVR_Login_V30("192.0.0.63", 8000, "admin", "123456ABC", &struDeviceInfo);
    if (lUserID < 0)
    {
        printf("Login error, %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return false;
    }
    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, nullptr,g_ExceptionCallBack, nullptr);

    //IR
    //    IRUserID=NetDev_Connect("ip",CONNECT_TYPE_ULIRNET);
    //    if(IRUserID==-1)
    //    {
    //        cout<<"IR connect error";
    //        return false;
    //    }

    return true;
}

bool capture::Vedio_Stream_Set()
{
    LONG lRealPlayHandle;
    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    struPlayInfo.hPlayWnd = 0; //需要 SDK 解码时句柄设为有效值,仅取流不解码时可设为空
    struPlayInfo.lChannel = 1; //预览通道号
    struPlayInfo.dwStreamType = 0; //0-主码流,1-子码流,2-码流 3,3-码流 4,以此类推
    struPlayInfo.dwLinkMode = 0;//0- TCP 方式,1- UDP 方式,2- 多播方式,3- RTP 方式,4-RTP/RTSP,5-RSTP/HTTP
    struPlayInfo.bBlocked = 1;

    //0- 非阻塞取流,1- 阻塞取流
    lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, g_RealDataCallBack_V30<pic_rgb>, nullptr);
    if (lRealPlayHandle < 0){
        printf("NET_DVR_RealPlay_V40 port1 error\n");
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }

    struPlayInfo.lChannel = 2; //预览通道号
    //lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, g_RealDataCallBack_V30<pic_ir>, nullptr);
    if (lRealPlayHandle < 0){
        printf("NET_DVR_RealPlay_V40 port2 error\n");
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }

    struPlayInfo.lChannel = 3; //预览通道号
    //lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, g_RealDataCallBack_V30<pic_uv>, nullptr);
    if (lRealPlayHandle < 0){
        printf("NET_DVR_RealPlay_V40 port3 error\n");
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }

    //IR
    //    BOOL ret = NetDev_StartRealStream(IRUserID,STREAM_TYPE_GRAYDATA);
    //    if(ret == FALSE){
    //        cout<<"IR:开始获取视频失败"<<endl;
    //        return false;
    //    }
    //    NetDev_SetRealStreamCallBack(IRUserID,funRealStream,(int *)this);
    return true;
}

void capture::Vedio_Update()
{
    lock();
    cvtColor(YUV_srcrgb, srcrgb, COLOR_YUV2BGR_YV12);
    cvtColor(YUV_srcir, srcir, COLOR_YUV2BGR_YV12);
    cvtColor(YUV_srcuv, srcuv, COLOR_YUV2BGR_YV12);

    //IR
    //    tmdata_size = m_dwDataSize;
    //    tmdata=(BYTE *)alloca(tmdata_size);
    //    memcpy(tmdata,m_pData,tmdata_size);
    unlock();
}


float capture::Get_tem(unsigned short nGray)
{
    return Temperature_GetTempFromGray(nGray, 0.96, 0, pTempPara,TempParaSize,m_RawHead.nCalcType); //返回温度
}

bool capture::Vedio_record()
{
    usleep(1000);
    NET_DVR_VOD_PARA struVodPara={0};
    struVodPara.dwSize=sizeof(struVodPara);
    struVodPara.struIDInfo.dwChannel=1; //通道号
    struVodPara.hWnd=0; //回放窗口

    struVodPara.struBeginTime.dwYear = 2013; //开始时间
    struVodPara.struBeginTime.dwMonth = 6;
    struVodPara.struBeginTime.dwDay = 14;
    struVodPara.struBeginTime.dwHour = 9;
    struVodPara.struBeginTime.dwMinute = 0;
    struVodPara.struBeginTime.dwSecond =0;

    struVodPara.struEndTime.dwYear = 2013; //结束时间
    struVodPara.struEndTime.dwMonth = 6;
    struVodPara.struEndTime.dwDay = 14;
    struVodPara.struEndTime.dwHour = 10;
    struVodPara.struEndTime.dwMinute = 7;
    struVodPara.struEndTime.dwSecond = 0;
    //---------------------------------------
    //按时间回放
    int hPlayback;
    hPlayback = NET_DVR_PlayBackByTime_V40(lUserID, &struVodPara);
    if(hPlayback < 0)
    {
        printf("NET_DVR_PlayBackByTime_V40 fail,last error %d\n",NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }
    //---------------------------------------
    //开始
    if(!NET_DVR_PlayBackControl_V40(hPlayback, NET_DVR_PLAYSTART,NULL, 0, NULL,NULL))
    {
        printf("play back control failed [%d]\n",NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }
    usleep(1000);
    //millisecond
    if(!NET_DVR_StopPlayBack(hPlayback))
    {
        printf("failed to stop file [%d]\n",NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }
    return true;
}

void capture::SDK_Close()
{
    //注销用户
    NET_DVR_Logout(lUserID);
    //释放 SDK 资源
    NET_DVR_Cleanup();
}
