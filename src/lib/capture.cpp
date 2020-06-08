#include "capture.h"
#include "fcntl.h"
#include <pthread.h>
#include <unistd.h>

mutex keyrgb, keyir, keyuv, keytp;
bool trgb = true, tir = true, tuv = true, ttp = true;
struct pic_rgb{};
struct pic_ir{};
struct pic_uv{};
void CALLBACK g_fSerialDataCallBack(LONG lSerialHandle, char *pRecvDataBuffer, DWORD dwBufSize, DWORD dwUser);
template <class pic>
inline void stream_lock(pic)
{}
template <>
inline void stream_lock<pic_rgb>(pic_rgb)
{
    keyrgb.lock();
}
template <>
inline void stream_lock<pic_ir>(pic_ir)
{
    keyir.lock();
}
template <>
inline void stream_lock<pic_uv>(pic_uv)
{
    keyuv.lock();
}
inline void tm_lock()
{
    keytp.lock();
}

template <class pic>
inline void stream_unlock(pic)
{}
template <>
inline void stream_unlock<pic_rgb>(pic_rgb)
{
    keyrgb.unlock();
}
template <>
inline void stream_unlock<pic_ir>(pic_ir)
{
    keyir.unlock();
}
template <>
inline void stream_unlock<pic_uv>(pic_uv)
{
    keyuv.unlock();
}
inline void tm_unlock()
{
    keytp.unlock();
}

template <class pic>
inline bool stream_t(pic)
{}
template <>
inline bool stream_t<pic_rgb>(pic_rgb)
{
    return trgb;
}
template <>
inline bool stream_t<pic_ir>(pic_ir)
{
    return tir;
}
template <>
inline bool stream_t<pic_uv>(pic_uv)
{
    return tuv;
}
inline bool tm_t()
{
    return ttp;
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
void setpic(Mat &src)
{}
template<>
void setpic<pic_rgb>(Mat &src)
{
    YUV_srcrgb=src.clone();
}
template<>
void setpic<pic_ir>(Mat &src)
{
    YUV_srcir=src.clone();
}
template<>
void setpic<pic_uv>(Mat &src)
{
    YUV_srcuv=src.clone();
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

            //                        Mat step;
            //                        cvtColor(YUVImage, step, COLOR_YUV2BGR_YV12);
            //                        imshow("test",step);
            //                        cvWaitKey(10);
            YUVImage.~Mat();
        }
        stream_unlock(pic());
    }
}

LONG lPort_rgb;
LONG lPort_ir;
LONG lPort_uv;
template<class pic>
inline LONG &get_lPort(pic)
{}
template<>
inline LONG &get_lPort<pic_rgb>(pic_rgb)
{
    return lPort_rgb;
}
template<>
inline LONG &get_lPort<pic_ir>(pic_ir)
{
    return lPort_ir;
}
template<>
inline LONG &get_lPort<pic_uv>(pic_uv)
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
    unsigned char strImageVersion[24] = {0};
    NetDev_GetVersion(strImageVersion);
    cout<<"IR SDK Version:"<<strImageVersion<<endl;
    char strTempVersion[24] = {0};
    TEMPALG_GetVersion(strTempVersion);
    cout<<"TEM SDK Version:"<<strTempVersion<<endl;
    NetDev_Init();

}

bool capture::SDK_Connect()
{
    NET_DVR_DEVICEINFO_V30 struDeviceInfo;
    lUserID = NET_DVR_Login_V30("192.168.1.2", 8000, "admin", "asdf1234", &struDeviceInfo);
    if (lUserID < 0)
    {
        printf("Login nvr error, %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return false;
    }
    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, nullptr,g_ExceptionCallBack, nullptr);

    //IR
    IRUserID=NetDev_Connect("192.168.1.84",CONNECT_TYPE_ULIRNET);
    if(IRUserID==-1)
    {
        cout<<"IR connect error ,id is "<<NetDev_GetLastError()<<endl;
        return false;
    }

    // uv
    lUserID_UV = NET_DVR_Login_V30("192.168.1.3", 8000, "admin", "asdf1234", &struDeviceInfo);
    if (lUserID_UV < 0)
    {
        printf("Login uv error, %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return false;
    }
        //建立透明通道
    int iSelSerialIndex = 2; //1:232 串口；2:485 串口
    lTranHandle = NET_DVR_SerialStart(lUserID_UV, iSelSerialIndex, g_fSerialDataCallBack, lUserID_UV);//设置回调函数获取透传数据
    if (lTranHandle < 0)
    {
        printf("NET_DVR_SerialStart error, %d\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID_UV);
        NET_DVR_Cleanup();
        return false;
    }
    _Get_envtem();
    return true;
}

bool capture::Vedio_Stream_Set()
{
    LONG lRealPlayHandle;
    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    //struPlayInfo.hPlayWnd = 0;      //需要 SDK 解码时句柄设为有效值,仅取流不解码时可设为空
    struPlayInfo.dwStreamType = 0;  //0-主码流,1-子码流,2-码流 3,3-码流 4,以此类推
    struPlayInfo.dwLinkMode = 0;    //0- TCP 方式,1- UDP 方式,2- 多播方式,3- RTP 方式,4-RTP/RTSP,5-RSTP/HTTP
    struPlayInfo.bBlocked = 1;      //0- 非阻塞取流,1- 阻塞取流
    struPlayInfo.dwDisplayBufNum = 1;

    struPlayInfo.lChannel = 33;      //预览通道号
    lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, g_RealDataCallBack_V30<pic_rgb>, nullptr);
    if (lRealPlayHandle < 0){
        printf("NET_DVR_RealPlay_V40 port1 error\n");
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }

    struPlayInfo.lChannel = 34;      //预览通道号
    lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, g_RealDataCallBack_V30<pic_ir>, nullptr);
    if (lRealPlayHandle < 0){
        printf("NET_DVR_RealPlay_V40 port2 error\n");
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }

    struPlayInfo.lChannel = 35;      //预览通道号
    lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, g_RealDataCallBack_V30<pic_uv>, nullptr);
    if (lRealPlayHandle < 0){
        printf("NET_DVR_RealPlay_V40 port3 error\n");
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }

    //IR
    NetDev_SetRealStreamCallBack(IRUserID,funRealStream,(int *)this);
    BOOL ret = NetDev_StartRealStream(IRUserID,STREAM_TYPE_GRAYDATA);
    if(ret == FALSE){
        cout<<"IR:开始获取视频失败"<<endl;
        return false;
    }
        // 设置定时获取和保存最高温度的线程

    if(pthread_create(&timerthid,NULL,rdsavemaxt,this) != 0)
    {
        cout<<"init rdsavemaxt thread failed!"<<endl;
        return false;
    }
    return true;
}

void capture::Vedio_Update()
{
    lock();

	if (!YUV_srcrgb.empty() && !YUV_srcir.empty() && !YUV_srcuv.empty())
	{
		cvtColor(YUV_srcrgb, srcrgb, COLOR_YUV2BGR_YV12);
        resize(srcrgb, srcrgb, Size(640, 480));
		cvtColor(YUV_srcir, srcir, COLOR_YUV2BGR_YV12);
        resize(srcir, srcir, Size(640, 480));
		cvtColor(YUV_srcuv, srcuv, COLOR_YUV2BGR_YV12);
        resize(srcuv, srcuv, Size(640, 480));
		
		rd_time.settimenow();
		if (laset_sec < rd_time)
		{
			laset_sec = rd_time;
			if (!record_part_rgb.empty())
				record_rgb.push_back(pair<record_time, vector<Mat>>(rd_time, record_part_rgb));
			if (!record_part_ir.empty())
				record_ir.push_back(pair<record_time, vector<Mat>>(rd_time, record_part_ir));
			if (!record_part_uv.empty())
				record_uv.push_back(pair<record_time, vector<Mat>>(rd_time, record_part_uv));

			record_part_rgb.clear();
			record_part_ir.clear();
			record_part_uv.clear();

			if (record_rgb.size() > 10)
				record_rgb.pop_front();
			if (record_ir.size() > 10)
				record_ir.pop_front();
			if (record_uv.size() > 10)
				record_uv.pop_front();
		}
		else
		{
			record_part_rgb.push_back(srcrgb);
			record_part_ir.push_back(srcir);
			record_part_uv.push_back(srcuv);
		}
	}

    //IR
    cm_RawHead=m_RawHead;

    cm_dwDataSize=m_dwDataSize;
    cm_pData=(BYTE *)alloca(cm_dwDataSize);
    memcpy(cm_pData,m_pData,cm_dwDataSize);

    cTempParaSize = TempParaSize;
    cpTempPara=(unsigned char *)alloca(cTempParaSize);
    memcpy(cpTempPara,pTempPara,cTempParaSize);

    unlock();
}


pair<float, Point> capture::Area_tem(const vector<Point>& counter, char tem_type, char area_type)
{
	Rect rect = boundingRect(counter);
    int conorx=rect.x;
    int conory=rect.y;
    int Height=rect.height;
    int Width=rect.width;
    int endx=conorx+rect.width;
    int endy=conory+rect.height;
    unsigned short * temdata=(unsigned short *)cm_pData;

	if (area_type == 0)		//	精确区域
	{
        bool** Eptr = new bool* [endy+1];
        for (int n = 0; n < endy+1; n++)
            Eptr[n] = new bool[endx+1];

        for (int row = conory; row < endy; row++)
            for (int col = conorx; col < endx; col++)		// 是否返回距离值，如果是false，1表示在内面，0表示在边界上，-1表示在外部，true返回实际距离,返回数据是double类型
                Eptr[row][col] = pointPolygonTest(counter, Point2f(col, row), false) == 1;
		
		vector <pair<Point, unsigned short>> topk;
        for (int row = conory; row < endy; row++)
            for (int col = conorx; col < endx; col++)
				if (Eptr[row][col])
                    topk.push_back(pair<Point, unsigned short>(Point(col, row), temdata[row *640 + col]));

		if (tem_type < 0)
		{
			sort(topk.begin(), topk.end(), [](pair<Point, double> x, pair<Point, double> y) { return x.second < y.second; });
			tem_type = -tem_type;
		}
		else if (tem_type > 0)
		{
			sort(topk.begin(), topk.end(), [](pair<Point, double> x, pair<Point, double> y) { return x.second > y.second; });
		}

		unsigned long Tsum = 0;
		Point peak;
		int nsum = 0;
		if (tem_type == 0)
		{
			for (auto ptr = topk.begin(); ptr != topk.end(); ptr++)
			{
				Tsum += ptr->second;
				peak += ptr->first;
				++nsum;
			}
		}
		else
		{
			for (auto ptr = topk.begin(); ptr != topk.end() && nsum < tem_type; ptr++)
			{
				Tsum += ptr->second;
				peak += ptr->first;
				++nsum;
			}
		}

        for (int n = 0; n < endy+1; n++)
			delete[] Eptr[n];
		delete[] Eptr;

        if(nsum==0)
            return pair<float, Point>(0, Point(0,0));
		return pair<float, Point>(Get_tem(Tsum / nsum), peak / nsum);
	}
	else					//	外接矩形
	{
		vector <pair<Point, unsigned short>> topk;

        for (int row = conory; row < endy; row++)
            for (int col = conorx; col < endx; col++)
                    topk.push_back(pair<Point, unsigned short>(Point(col, row), temdata[row * 640 + col]));
		if (tem_type < 0)
		{
			sort(topk.begin(), topk.end(), [](pair<Point, double> x, pair<Point, double> y) { return x.second < y.second; });
			tem_type = -tem_type;
		}
		else if (tem_type > 0)
		{
			sort(topk.begin(), topk.end(), [](pair<Point, double> x, pair<Point, double> y) { return x.second > y.second; });
		}

		unsigned long Tsum = 0;
		Point peak;
		int nsum = 0;
		if (tem_type == 0)
		{
			for (auto ptr = topk.begin(); ptr != topk.end(); ptr++)
			{
				Tsum += ptr->second;
				peak += ptr->first;
				++nsum;
			}
		}
		else
		{
			for (auto ptr = topk.begin(); ptr != topk.end() && nsum < tem_type; ptr++)
			{
				Tsum += ptr->second;
				peak += ptr->first;
				++nsum;
			}
		}

        if(nsum==0)
            return pair<float, Point>(0, Point(0,0));
        else {
            return pair<float, Point>(Get_tem(Tsum / nsum), peak / nsum);
        }

	}
}

// 485接口返回的环境温度
static double stenv = 0;
void capture::_Get_envtem()
{
    //通过透明通道发送数据
    LONG lSerialChan = 1;//使用485 时该值有效，从1 开始；232 时设置为0
    unsigned char szSendBuf[8] = {0x01,0x04,0x01,0x90,0x00,0x01,0x30,0x1B};
    if (!NET_DVR_SerialSend(lTranHandle, lSerialChan, (char*)szSendBuf, sizeof(szSendBuf)))//szSendBuf 为发送数据的缓冲区
    {
        printf("NET_DVR_SerialSend error, %d\n", NET_DVR_GetLastError());
        NET_DVR_SerialStop(lTranHandle);
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
    }
    sleep(1);
    tenv = stenv;
}

void capture::fire_filter(vector<vector<Point>>& counters, float th_top, float th_bottom, char size_top, char size_bottom)
{
    Mat result(srcir.size(), CV_8UC1, 255);

    for (auto& counter : counters)
    {
        //	根据火焰高温特性，筛选出包含疑似火焰的区域。
        Rect rect = boundingRect(counter);
        int conorx = rect.x;
        int conory = rect.y;
        int Height = rect.height;
        int Width = rect.width;
        int endx = conorx + rect.width;
        int endy = conory + rect.height;
        unsigned short* temdata = (unsigned short*)cm_pData;

        vector <pair<Point, unsigned short>> topk;

        for (int row = conory; row < endy; row++)
            for (int col = conorx; col < endx; col++)
                topk.push_back(pair<Point, unsigned short>(Point(col, row), temdata[row * 640 + col]));

        sort(topk.begin(), topk.end(), [](pair<Point, double> x, pair<Point, double> y) { return x.second > y.second; });

        unsigned long Tsum = 0;
        Point peak;
        int nsum = 0;

        for (auto ptr = topk.begin(); ptr != topk.end() && nsum < size_top; ptr++)
        {
            Tsum += ptr->second;
            peak += ptr->first;
            ++nsum;
        }

        if (nsum == 0 || Get_tem(Tsum / nsum) <= th_top)
            continue;

        //	滤除低温点，并在Mat中标记高温点。
        int step=0;
        while (true)
        {
            Tsum = 0;
            nsum = 0;

            for (auto ptr = topk.rbegin()+step; ptr != topk.rend() && nsum < size_bottom; ++ptr)
            {
                Tsum += ptr->second;
                ++nsum;
            }
            if (nsum == 0 )
                break;

            if(Get_tem(Tsum / nsum) > th_bottom)
            {
                for(int n = 0 ; n < topk.size() - step ; ++n)
                    result.at<uchar>(topk[n].first.y,topk[n].first.x) = 0;

                break;

            }
            else
                step+=nsum;

        }
    }



    Mat reth;
    threshold(result, reth, 100, 255, THRESH_BINARY);
    vector<Vec4i> hierarchy;
    findContours(reth, counters, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);

    //简单过滤
    vector<vector<Point>> sv;
    vector<vector<Point>>::const_iterator itContours = counters.begin();
    for (; itContours != counters.end(); ++itContours)
    {
        if (itContours->size() > 25 && itContours->size() < srcir.rows * 2 + srcir.cols * 2 - 20)
        {
            sv.push_back(*itContours);
        }
    }
    counters=sv;


    imshow("fire filter",result);
}

// 软件方法获取录像
bool capture::Vedio_record(record_time begin, record_time end, int port, string filename)
{
	bool result = false;
	list<pair<record_time, vector<Mat>>>* ptr = nullptr;
	if (port == 33)
		ptr = &record_rgb;
	else if (port == 34)
		ptr = &record_ir;
	else if (port == 35)
		ptr = &record_uv;

    VideoWriter video(filename, 0x00000021, ptr->front().second.size(), ptr->front().second.front().size());
	for (auto& i : *ptr)
	{
		if (begin <= i.first && i.first <= end)
		{
			for (auto& ii : i.second)
			{
				video << ii;
			}
		}
	}

	video.release();
	return false;
}

// 硬件方法获取录像
bool capture::Vedio_record_nvr(record_time begin,record_time end,int port,string filename)
{

    NET_DVR_PLAYCOND struDownloadCond={0};
    struDownloadCond.dwChannel=port; //通道号
    struDownloadCond.struStartTime.dwYear = begin.year; //开始时间
    struDownloadCond.struStartTime.dwMonth = begin.month;
    struDownloadCond.struStartTime.dwDay = begin.day;
    struDownloadCond.struStartTime.dwHour = begin.hour;
    struDownloadCond.struStartTime.dwMinute = begin.min;
    struDownloadCond.struStartTime.dwSecond =begin.sec;

    struDownloadCond.struStopTime.dwYear= end.year; //结束时间
    struDownloadCond.struStopTime.dwMonth = end.month;
    struDownloadCond.struStopTime.dwDay = end.day;
    struDownloadCond.struStopTime.dwHour = end.hour;
    struDownloadCond.struStopTime.dwMinute = end.min;
    struDownloadCond.struStopTime.dwSecond= end.sec;
    //---------------------------------------
    //按时间下载
    int hPlayback = NET_DVR_GetFileByTime_V40(lUserID,const_cast<char *>(filename.c_str()),&struDownloadCond);
    //int hPlayback = NET_DVR_GetFileByTime_V40(lUserID,"33.mp4",&struDownloadCond);

    cout<<"step 1"<<endl;
    if(hPlayback < 0)
    {
        cout<<"Vedio failed,last error"<<NET_DVR_GetLastError()<<endl;
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }
    //---------------------------------------
    //开始
    if(!NET_DVR_PlayBackControl_V40(hPlayback, NET_DVR_PLAYSTART,NULL, 0, NULL,NULL))
    {
        cout<<"play back control failed "<< NET_DVR_GetLastError()<< endl;
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }

    cout<<"开始下载录像#####"<<endl;
    int nPos = 0;
    for(nPos = 0; nPos < 100&&nPos>=0; nPos = NET_DVR_GetDownloadPos(hPlayback))
    {
        cout<<"进度："<<nPos<<endl;
        usleep(5000);
    }
    cout<<"进度："<<100<<endl;
    cout<<"下载录像成功#####"<<endl;


    if(!NET_DVR_StopPlayBack(hPlayback))
    {
        printf("failed to stop file [%d]\n",NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }
    return true;
}

// 定时器定时调用的函数
void *rdsavemaxt(void *captureptr)
{
    capture *ptr = (capture *)captureptr;
    pthread_detach(pthread_self());
    while(true)
    {
        sleep(60);
        ptr->_rdsavemaxt();
    }
    ptr ->_Get_envtem();
}
void capture::_rdsavemaxt()
{
    int result;
    int outsize = 0;
    NetDev_GetConfig(IRUserID,ULIR_MEASURE_GET_MAXTEMP,&result,sizeof(result),&outsize);

    // 生成当前文件名
    time_t timep;
    time(&timep);
    struct tm* nowTime = localtime(&timep);
    nowTime->tm_year += 1900;
    nowTime->tm_mon += 1;
    string filename = string("/home/zxb/SRC_C/data/Server_plotdata/") + to_string(nowTime->tm_year) + "-" + to_string(nowTime->tm_mon) + "-" +to_string(nowTime->tm_mday);
    // 打开文件并添加数据,然后关闭
    int fd = open(filename.c_str(),O_WRONLY|O_CREAT|O_APPEND,S_IRWXU|S_IRWXO|S_IRWXG);
    string strt =to_string(nowTime->tm_hour + nowTime->tm_min/60.0) + "," + to_string(result) + "\n";
    write(fd,strt.c_str(),strt.size());
    close(fd);
}



void capture::SDK_Close()
{
    //注销用户
    //NET_DVR_Logout(lUserID);
    //释放 SDK 资源
    NET_DVR_SerialStop(lTranHandle);
    NET_DVR_Cleanup();
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



//base class

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
    int test=false;
    vector<vector<Point>> sv;

    //灰度化
    cvtColor(src, gray, CV_BGR2GRAY);

    if(test)
    {
        imshow("src", src);
        imshow("gray", gray);
    }

    //形态学处理
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

    if(test && conf.dande > 0)
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

    //二值化
    if (conf.th == 0)
        threshold(gray, TH, 0, 255, CV_THRESH_OTSU);
    else
        threshold(gray, TH, conf.th, 255, THRESH_BINARY);

    if(test)
        imshow("TH", TH);

    //查找轮廓
    vector<Vec4i> hierarchy;
    findContours(TH, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);

    //简单过滤
    vector<vector<Point>>::const_iterator itContours = contours.begin();
    for (; itContours != contours.end(); ++itContours)
    {
        if (itContours->size() > conf.minnum && itContours->size() < TH.rows * 2 + TH.cols * 2 - 20)
        {
            Point Pc = centerofV(*itContours);
            if (Pc.x > TH.cols / 5 && Pc.x<4 * TH.cols / 5 && Pc.y>TH.rows / 5 && Pc.y < 4 * TH.rows / 5)
            {
                sv.push_back(*itContours);
            }
        }
    }
    //cout << sv.size() << endl;

    //设置result picture
    Mat result(TH.size(), CV_8UC3, cv::Scalar(255, 255, 255));
    drawContours(result, contours, -1, cv::Scalar(0, 0, 0), 1);
    drawContours(result, sv, -1, Scalar(0, 0, 255), 1);
    //if(test)
        imshow("可疑轮廓", result);

    return sv;
}

int basedec::faultdetect()
{
    return 0;
}

//回调透传数据函数的外部实现，获取当前环境温度
void CALLBACK g_fSerialDataCallBack(LONG lSerialHandle, char *pRecvDataBuffer, DWORD dwBufSize, DWORD dwUser)
{
    //…… 处理接收到的透传数据，pRecvDataBuffer 中存放接收到的数据
    unsigned short result = (((unsigned short)pRecvDataBuffer[3])<<8) + (unsigned short)pRecvDataBuffer[4];
    stenv = (double)result / 10;
}
