#include "heat_dec.h"
#include "fire_dec.h"
#include "water_seepage_dec.h"
#include "flash_dec.h"
#include "server.h"
#include <sys/time.h>
#include <unistd.h>
#define TEST true
typedef server::counter counter;

int fps();

int main()
{
    /************************** Part one begin **********************************************/
    //建立现场设备连接以及，设置视频流。
    shared_ptr<capture> mc = make_shared<capture>();
    if(!mc->SDK_Connect()){
        cout<<"connect failed"<<endl;
        return 0;
    }
    else
        cout<<"connect successfully"<<endl;
    if(!mc->Vedio_Stream_Set()){
        cout<<"vedio set failed"<<endl;
        return 0;
    }
    else
        cout<<"vedio set successfully"<<endl;

        /************** test only begin ******************/
    if(TEST)
        cout<<"First test"<<endl;
    namedWindow("RGB");
    namedWindow("IR");
    namedWindow("UV");
    while(TEST && cvWaitKey(10) != 27)
    {
        //测试FPS读取
        int Fps = fps();

        //从设备更新图像数据
        mc->Vedio_Update();

        //测试视频流读取
        imshow("RGB",mc->srcrgb);
        imshow("IR",mc->srcir);
        imshow("UV",mc->srcuv);

        //测试温度读取
        WORD center_gray=mc->getgray(340,240);
        float center_tem=mc->Get_tem(center_gray);
        cout<<"center tempreture is:    "<< fixed<< setprecision(4)<<center_tem<<"  FPS: "<<Fps<<endl;

        usleep(25000);
    }
    destroyAllWindows();
        /************** test only end  ******************/

    /************************** Part one end   **********************************************/



//    /************************** Part two-1 begin ********************************************/
//    //建立网络通信连接
//    socketinit();
//    server s_server;
//    //s_server.s_connect("39.108.229.151", 8010);
//    s_server.s_connect("127.0.0.1", 8010);
//    s_server.send_buff_push(login_mes("server", "123456"));
//    s_server.setbasefile("data/");

//        /************** test only begin ******************/
//    if(TEST)
//        cout<<"Second test"<<endl;
//    while(TEST && cvWaitKey(10) != 27)
//    {
//        //从设备更新图像数据
//        mc->Vedio_Update();

//        s_server.send_buff_push(mc->srcrgb,1);
//        s_server.send_buff_push(mc->srcir,2);
//        s_server.send_buff_push(mc->srcuv,3);

//        usleep(25000);
//    }
//        /************** test only end  ******************/

//    /************************** Part two-1 end   ********************************************/



////    /************************** Part three begin ********************************************/
////    //建立检测功能模块
////    heat_dec heat_dec(mc);
////    fire_dec fire_dec(mc);
////    water_seepage_dec water_dec(mc);
////    flash_dec flash_dec(mc);

////    //检测流程
////    while(true)
////    {
////        int Fps = fps();
////        cout<<"Fps : "<<Fps<<endl;

////        //从设备更新图像数据
////        mc->Vedio_Update();

////        //检测此帧图像
//////        //明火检测
//////        int Result_fire = fire_dec.detect();
//////        if(Result_fire)
//////        {
//////            state_mes mes;
//////            mes.settime_now();
//////            record_time begin(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec-1);
//////            record_time end(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec+1);

//////            string vedio_name=mes.tostring();
//////            string command = "mkdir -p " + s_server.getbasefile() + vedio_name;
//////            system(command.c_str());
//////            vedio_name=s_server.getbasefile() + vedio_name+"/"+vedio_name+"_vedio.mp4";

//////            mc->Vedio_record(begin,end,1,vedio_name);
//////            Mat re=fire_dec.result_pic.clone();

//////            s_server.savefault(mes,mc->srcrgb,re,mc->srcuv);
//////            s_server.send_decinf(3,mes,mc->srcrgb,re,mc->srcuv,vedio_name);
//////            continue;
//////        }
//////        if(TEST)
//////        {
//////            cout<<"Result_fire: "<<Result_fire<<endl;
//////            if(Result_fire)
//////            {
//////                Mat re=fire_dec.result_pic;
//////                imshow("Result_fire",re);
//////            }
//////        }

////        //放电检测
////        int Result_flash = flash_dec.detect();
////        if(Result_flash)
////        {
////            state_mes mes;
////            mes.settime_now();
////            record_time begin(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec-1);
////            record_time end(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec+1);

////            string vedio_name=mes.tostring();
////            string command = "mkdir -p " + s_server.getbasefile() +  vedio_name;
////            system(command.c_str());
////            vedio_name= s_server.getbasefile() + vedio_name+"/"+vedio_name+"_vedio.mp4";

////            mc->Vedio_record(begin,end,1,vedio_name);
////            Mat re=flash_dec.result_pic.clone();

////            s_server.savefault(mes,mc->srcrgb,mc->srcir,re);
////            s_server.send_decinf(3,mes,mc->srcrgb,mc->srcir,re,vedio_name);
////            continue;
////        }
////        if(TEST)
////        {
////            cout<<"Result_flash: "<<Result_flash<<endl;
////            if(Result_flash)
////            {
////                Mat re=flash_dec.result_pic;
////                imshow("Result_flash",re);
////            }
////        }

//////        //过热检测
//////        int Result_heat = heat_dec.detect();
//////        if(Result_heat)
//////        {
//////            state_mes mes;
//////            mes.settime_now();
//////            record_time begin(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec-1);
//////            record_time end(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec+1);

//////            string vedio_name=mes.tostring();
//////            string command = "mkdir -p " + s_server.getbasefile() +  vedio_name;
//////            system(command.c_str());
//////            vedio_name= s_server.getbasefile() + vedio_name+"/"+vedio_name+"_vedio.mp4";

//////            mc->Vedio_record(begin,end,1,vedio_name);
//////            Mat re=heat_dec.result_pic.clone();

//////            s_server.savefault(mes,mc->srcrgb,re,mc->srcuv);
//////            s_server.send_decinf(3,mes,mc->srcrgb,re,mc->srcuv,vedio_name);
//////            continue;
//////        }
//////        if(TEST)
//////        {
//////            cout<<"Result_heat: "<<Result_heat<<endl;
//////            if(Result_heat)
//////            {
//////                Mat re=heat_dec.result_pic;
//////                imshow("Result_heat",re);
//////            }
//////        }



//////        //渗水检测
//////        int Result_water = water_dec.detect();
//////        if(Result_water)
//////        {
//////            state_mes mes;
//////            mes.settime_now();
//////            record_time begin(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec-1);
//////            record_time end(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec+1);

//////            string vedio_name=mes.tostring();
//////            string command = "mkdir -p " + s_server.getbasefile() +  vedio_name;
//////            system(command.c_str());
//////            vedio_name= s_server.getbasefile() + vedio_name+"/"+vedio_name+"_vedio.mp4";

//////            mc->Vedio_record(begin,end,1,vedio_name);
//////            Mat re=water_dec.result_pic.clone();

//////            s_server.savefault(mes,mc->srcrgb,re,mc->srcuv);
//////            s_server.send_decinf(3,mes,mc->srcrgb,re,mc->srcuv,vedio_name);
//////            continue;
//////        }
//////        if(TEST)
//////        {
//////            cout<<"Result_water: "<<Result_water<<endl;
//////            if(Result_water)
//////            {
//////                Mat re=water_dec.result_pic;
//////                imshow("Result_water",re);
//////            }
//////        }

//////        //保存实时数据，用作实验分析。
//////        //s_server.saveaf(mc->srcrgb,mc->srcir,mc->srcuv,counter(),heat_dec.s_contour,flash_dec.s_contour);
////    }
////    /************************** Part three end   ******************************************/



//    /************************** Part two-2 end   ******************************************/
//    s_server.disconnect();
//    socketclose();
//    /************************** Part two-2 end   ******************************************/
    return 0;
}


int fps()
{
    static int FPS=0;
    static struct timeval timeStart;
    static int frameCount = -1;

    ++frameCount;

    if(frameCount==20)
    {
        //更新FPS
        struct timeval timeEnd;
        gettimeofday(&timeEnd, NULL );
        double diff=(timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;
        FPS=20.0/diff;

        //归零
        frameCount=0;
    }

    if(frameCount==0)
        gettimeofday(&timeStart, NULL );

    return FPS;
}

