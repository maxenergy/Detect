#include "heat_dec.h"
#include "fire_dec.h"
#include "water_seepage_dec.h"
#include "flash_dec.h"
#include "socket_connect_v2.h"
#include <sys/time.h>
#include <unistd.h>
#define TEST true

int fps();
void savemat(const Mat& input, string filename);
void savecontour(const vector<vector<Point>>& input, string filename);

void getmat(Mat& input, string filename);
void getcontour(vector<vector<Point>>& input, string filename);
void saveaf(string base, Mat& rgb, Mat& ir, Mat& uv, const vector<vector<Point>>& crgb, const vector<vector<Point>>& cir, const vector<vector<Point>>& cuv);
void readaf(string base, Mat& rgb, Mat& ir, Mat& uv, vector<vector<Point>>& crgb, vector<vector<Point>>& cir, vector<vector<Point>>& cuv, State_mes& mes);



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
    _SLEEP(500);
    /************** test only begin ******************/
    if(TEST)
    {
        cout<<"First test"<<endl;
        namedWindow("RGB");
        namedWindow("IR");
        namedWindow("UV");
        int loop=80;
        while(TEST && loop>0)
        {
            loop--;
            //测试FPS读取
            int Fps = fps();

            //从设备更新图像数据
            mc->Vedio_Update();

            namedWindow("RGB");
            namedWindow("IR");
            namedWindow("UV");
            //测试视频流读取
            if(!mc->srcrgb.empty()&&!mc->srcir.empty()&&!mc->srcuv.empty())
            {
                imshow("RGB",mc->srcrgb);
                imshow("IR",mc->srcir);
                imshow("UV",mc->srcuv);
            }
            //测试温度读取
            WORD center_gray=mc->getgray(320,240);
            float center_tem=mc->Get_tem(center_gray);
            cout<<"center tempreture is:    "<< fixed<< setprecision(4)<<center_tem<<"  FPS: "<<Fps<<endl;
            cvWaitKey(1);
        }
        destroyAllWindows();
        cvWaitKey(1);
    }
    /************** test only end  ******************/

    /************************** Part one end   **********************************************/



        /************************** Part two-1 begin ********************************************/
        //建立网络通信连接
		Server s_server;
		s_server.s_connect("127.0.0.1", 8010);
		//s_server.s_connect("39.108.229.151", 8010);
		Login_mes login_mes("server", "123456");
		s_server.sendbuff_push(login_mes);
		while (!s_server.recvbuff_pop(login_mes));
		if (login_mes.confirm != 1)
		{
			cout << "Login failed" << endl;
			return 1;
		}
		cout << "login successfully!" << endl;
		login_mes.confirm = 1;
		s_server.sendbuff_push(login_mes);
        s_server.setbasefile("/home/zxb/SRC_C/data/Server_data/");
            /************** test only begin ******************/
        //if(TEST)
        //    cout<<"Second test"<<endl;
        //int loop=0;
        //while(TEST && loop>0)
        //{
        //    loop--;
        //    //从设备更新图像数据
        //    mc->Vedio_Update();

        //    if(!mc->srcrgb.empty()&&!mc->srcir.empty()&&!mc->srcuv.empty())
        //    {
        //        s_server.send_buff_push(mc->srcrgb,1);
        //        s_server.send_buff_push(mc->srcir,2);
        //        s_server.send_buff_push(mc->srcuv,3);
        //    }

        //    usleep(25000);
        //}
            /************** test only end  ******************/

        /************************** Part two-1 end   ********************************************/



        /************************** Part three begin ********************************************/
        //建立检测功能模块
        heat_dec heat_dec(mc, &s_server);
        fire_dec fire_dec(mc, &s_server);
        water_seepage_dec water_dec(mc, &s_server);
        flash_dec flash_dec(mc, &s_server);

        //检测流程
        while(true)
        {
            //int Fps = fps();
            //cout<<"Fps : "<<Fps<<endl;

            //从设备更新图像数据
            mc->Vedio_Update();
            //检测此帧图像

//            //明火检测
//            int Result_fire = fire_dec.detect();
//            if(true)
//            {
//                if(Result_fire==1)
//                    cout<<"Result_fire: "<<Result_fire<<endl;
//                imshow("Result_flash",fire_dec.result_pic);
//            }
//            if (Result_fire && !fire_dec.isstay())
//            {
//                State_mes mes;
//                mes.mode=2;
//                mes.settime_now();

//                Mat re = fire_dec.result_pic.clone();
//                fire_dec.save_and_send(mes, mc->srcrgb, re, mc->srcuv, s_server.getbasefile(),33);


//                fire_dec.setstaytime(15);
//                heat_dec.clear();
//                heat_dec.setstaytime(20);
//                water_dec.clear();
//                water_dec.setstaytime(20);
//                flash_dec.clear();
//                flash_dec.setstaytime(20);
//            }
            //saveaf("/home/zxb/SRC_C/lab_data/fire/", mc->srcrgb, mc->srcir, mc->srcuv, vector<vector<Point>>(), fire_dec.result_counters, vector<vector<Point>>());


//            //放电检测
//            int Result_flash = flash_dec.detect();
//            if(true)
//            {
//                if(Result_flash==1)
//                    cout<<"Result_flash: "<<Result_flash<<endl;
//                imshow("Result_flash",flash_dec.result_pic);
//            }
//            if(Result_flash && !flash_dec.isstay())
//            {
//                cout<<"in"<<endl;
//                State_mes mes;
//                mes.mode=4;
//                mes.settime_now();

//                Mat re=flash_dec.result_pic.clone();
//                flash_dec.save_and_send(mes, mc->srcrgb, mc->srcir, re, s_server.getbasefile(),35);

//                flash_dec.setstaytime(15);
//            }
            //saveaf("/home/zxb/SRC_C/lab_data/flash/", mc->srcrgb, mc->srcir, mc->srcuv, vector<vector<Point>>(), vector<vector<Point>>(), flash_dec.result_counters);


//            //过热检测
//            Command cmd;
//            if(s_server.recvbuff_pop(cmd) && cmd.type == 1)
//                heat_dec.readxml("/home/zxb/SRC_C/Detect/conf.xml");
//            int Result_heat = heat_dec.detect();
//            if(true)
//            {
//                if(Result_heat!=0)
//                   cout<<"Result_heat: "<<Result_heat<<endl;
//                imshow("Result_heat",heat_dec.result_pic);
//            }
//            if(Result_heat && !heat_dec.isstay())
//            {
//                State_mes mes;
//                mes.settime_now();
//                mes.mode=1;

//                Mat re=heat_dec.result_pic.clone();
//                heat_dec.save_and_send(mes, mc->srcrgb, re, mc->srcuv, s_server.getbasefile(),34);

//                heat_dec.setstaytime(15);
//            }
//            saveaf("/home/zxb/SRC_C/lab_data/heat/", mc->srcrgb, mc->srcir, mc->srcuv, vector<vector<Point>>(), heat_dec.result_counters, vector<vector<Point>>());


//            //渗水检测
//            int Result_water = water_dec.detect();
//            if(true)
//            {
//                if(Result_water==1)
//                    cout<<"Result_water: "<<Result_water<<endl;
//                imshow("Result_water",water_dec.result_pic);
//            }
//            if(Result_water && !water_dec.isstay())
//            {
//                State_mes mes;
//                mes.mode=3;
//                mes.settime_now();

//                Mat re=water_dec.result_pic.clone();
//                water_dec.save_and_send(mes, mc->srcrgb, re, mc->srcuv, s_server.getbasefile(),33);

//                water_dec.setstaytime(15);
//            }
            //saveaf("/home/zxb/SRC_C/lab_data/water/", mc->srcrgb, mc->srcir, mc->srcuv, vector<vector<Point>>(), water_dec.result_counters, vector<vector<Point>>());

//            for(int n=1; n<=266 ; ++n)
//            {
//                Mat rgb;
//                Mat ir;
//                Mat uv;
//                vector<vector<Point>> crgb;
//                vector<vector<Point>> cir;
//                vector<vector<Point>> cuv;
//                State_mes mes;

//                readaf("/home/zxb/SRC_C/lab_data/water/", rgb , ir, uv,  crgb, cir, cuv, mes);
//                cout<<mes.tostring()<<endl;
//                if(!cir.empty())
//                {
//                    Rect rect = boundingRect(cir.front());
//                    rectangle(ir, rect, Scalar(0, 0, 255),3);
//                }
//                imshow("ir",ir);
//                cvWaitKey(60);
//            }

            cvWaitKey(60);
        }
        /************************** Part three end   ******************************************/

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




void savemat(const Mat& input, string filename)
{
	imwrite(filename, input);
}

void savecontour(const vector<vector<Point>>& input, string filename)
{
	std::ofstream outFile;
	outFile.open(filename);
	for (const vector<Point>& i : input)
	{
		for (const Point& ii : i)
		{
			outFile << ii;
		}
		outFile << '<';
		outFile << "\n";
	}
}



void getmat(Mat& input, string filename)
{
	input = imread(filename);
}

void getcontour(vector<vector<Point>>& input, string filename)
{
	std::ifstream inFile;
	inFile.open(filename);
	vector<Point> step;
	while (!inFile.eof())
	{
		char c;
		int x, y;
		inFile >> c;
		if (c == '[')
		{
			inFile >> x >> c >> y >> c;
			step.push_back(Point(x, y));
		}
		else if (c == '<')
		{
			input.push_back(step);
			step.clear();
		}

	}

}


void saveaf(string base, Mat& rgb, Mat& ir, Mat& uv, const vector<vector<Point>>& crgb, const vector<vector<Point>>& cir, const vector<vector<Point>>& cuv)
{
	static int index = 0;

	++index;

	string statename = base + to_string(index) + "_mes.txt";
	string rgbname = base + to_string(index) + "_rgb.jpg";
	string irname = base + to_string(index) + "_ir.jpg";
	string uvname = base + to_string(index) + "_uv.jpg";
	string crgbname = base + to_string(index) + "_rgb.txt";
	string cirname = base + to_string(index) + "_ir.txt";
	string cuvname = base + to_string(index) + "_uv.txt";

	State_mes mes;
	mes.settime_now();
	mes.save(statename);
	savemat(rgb, rgbname);
	savemat(ir, irname);
	savemat(uv, uvname);
	savecontour(crgb, crgbname);
	savecontour(cir, cirname);
	savecontour(cuv, cuvname);
}

void readaf(string base, Mat& rgb, Mat& ir, Mat& uv, vector<vector<Point>>& crgb, vector<vector<Point>>& cir, vector<vector<Point>>& cuv, State_mes& mes)
{
	static int index = 0;

	++index;

	string statename = base + to_string(index) + "_mes.txt";
	string rgbname = base + to_string(index) + "_rgb.jpg";
	string irname = base + to_string(index) + "_ir.jpg";
	string uvname = base + to_string(index) + "_uv.jpg";
	string crgbname = base + to_string(index) + "_rgb.txt";
	string cirname = base + to_string(index) + "_ir.txt";
	string cuvname = base + to_string(index) + "_uv.txt";

    mes.getfromfile(statename);
    getmat(rgb, rgbname);
    getmat(ir, irname);
    getmat(uv, uvname);
	getcontour(crgb, crgbname);
	getcontour(cir, cirname);
    getcontour(cuv, cuvname);
}
