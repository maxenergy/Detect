#include"water_seepage_dec.h"
#include <unistd.h>

int main()
{
    shared_ptr<capture> mc = make_shared<capture>("/home/zxb/SRC_C/Detect/testpic/water_and_heat/data.txt", 0);
    if(!mc->SDK_Connect())
    {
        cout<<"connect failed"<<endl;
        return 0;
    }
    if(!mc->Vedio_Stream_Set())
    {
        cout<<"vedio set failed"<<endl;
        return 0;
    }
    cout<<"stream begin";

//	water_seepage_dec water(mc);
//	water.detect();

    sleep(5);
	return 0;
}
