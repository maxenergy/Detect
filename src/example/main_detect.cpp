#include "heat_dec.h"
#include "fire_dec.h"
#include "water_seepage_dec.h"
#include "flash_dec.h"
#include "server.h"
#include <sys/time.h>
#include <unistd.h>

typedef server::counter counter;

int fps();

int main()
{
    //建立设备连接
    //shared_ptr<capture> mc = make_shared<capture>("/home/zxb/SRC_C/Detect/testpic/water_and_heat/data.txt", 0);
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

    //建立网络通信连接
    socketinit();
    server s_server;
    //s_server.s_connect("39.108.229.151", 8010);
    s_server.s_connect("127.0.0.1", 8010);
    s_server.send_buff_push(login_mes("server", "123456"));

    //建立检测功能模块
    heat_dec heat_dec(mc);
    fire_dec fire_dec(mc);
    water_seepage_dec water_dec(mc);
    flash_dec flash_dec(mc);

    //检测流程
    while(true)
    {
        int Fps = fps();

        //从设备更新图像数据
        mc->Vedio_Update();

        //检测此帧图像
        //过热检测
        int Result_heat = heat_dec.detect();
        if(Result_heat)
        {
            state_mes mes;
            mes.settime_now();
            record_time begin(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec-1);
            record_time end(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec+1);

            string vedio_name=mes.tostring();
            string command = "mkdir -p " + vedio_name;
            system(command.c_str());
            vedio_name=vedio_name+"/"+vedio_name+"_rgb.mp4";

            mc->Vedio_record(begin,end,1,vedio_name);
            Mat re=heat_dec.result_pic.clone();

            s_server.savefault(mes,mc->srcrgb,re,mc->srcuv);
            s_server.send_decinf(3,mes,mc->srcrgb,re,mc->srcuv,vedio_name);
        }

        //明火检测
        int Result_fire = fire_dec.detect();
        if(Result_fire)
        {
            state_mes mes;
            mes.settime_now();
            record_time begin(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec-1);
            record_time end(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec+1);

            string vedio_name=mes.tostring();

            Mat re=heat_dec.result_pic.clone();

            vedio_name=vedio_name+"/"+vedio_name+"_vedio.mp4";
            string command = "mkdir -p " + vedio_name;
            system(command.c_str());
            mc->Vedio_record(begin,end,1,vedio_name);

            s_server.savefault(mes,mc->srcrgb,re,mc->srcuv);
            s_server.send_decinf(3,mes,mc->srcrgb,re,mc->srcuv,vedio_name);
        }

        //渗水检测
        int Result_water = water_dec.detect();
        if(Result_water)
        {
//            state_mes mes;
//            mes.settime_now();
//            record_time begin(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec-1);
//            record_time end(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec+1);

//            string vedio_name=mes.tostring();

//            Mat re=heat_dec.result_pic.clone();

//            vedio_name=vedio_name+"/"+vedio_name+"_rgb.mp4";
//            string command = "mkdir -p " + vedio_name;
//            system(command.c_str());
//            mc->Vedio_record(begin,end,1,vedio_name);

//            s_server.savefault(mes,mc->srcrgb,re,mc->srcuv);
//            s_server.send_decinf(3,mes,mc->srcrgb,re,mc->srcuv,vedio_name);
        }

        //放电检测
        int Result_flash = flash_dec.detect();
        if(Result_flash)
        {
//            state_mes mes;
//            mes.settime_now();
//            record_time begin(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec-1);
//            record_time end(mes.year,mes.mon,mes.day,mes.hour,mes.min,mes.sec+1);

//            string vedio_name=mes.tostring();

//            Mat re=heat_dec.result_pic.clone();

//            vedio_name=vedio_name+"/"+vedio_name+"_rgb.mp4";
//            string command = "mkdir -p " + vedio_name;
//            system(command.c_str());
//            mc->Vedio_record(begin,end,1,vedio_name);

//            s_server.savefault(mes,mc->srcrgb,mc->srcir,re);
//            s_server.send_decinf(3,mes,mc->srcrgb,mc->srcir,re,vedio_name);
        }
        s_server.saveaf(mc->srcrgb,mc->srcir,mc->srcuv,counter(),heat_dec.s_contour,counter());
    }

    s_server.disconnect();
    socketclose();
    return 0;
}

//int main()
//{
////    state_mes mes;
////    mes.settime_now();
////    mes.mode=1;
////    mes.save("text.txt");

////    state_mes mes2;
////    mes2.getfromfile("text.txt");
////    cout<<mes2.tostring();

//    vector<vector<Point>> vv;
//    vector<Point> v1(10,Point(1,1));
//    vector<Point> v2(12,Point(2,2));
//    vector<Point> v3(14,Point(4,3));
//    vv.push_back(v1);
//    vv.push_back(v2);
//    vv.push_back(v3);
//    savecontour(vv,"test2.txt");
//    getcontour(vv,"test2.txt");

//}

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

