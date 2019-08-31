#include"water_seepage_dec.h"

int main()
{
    shared_ptr<capture> mc = make_shared<capture>("/home/zxb/SRC_C/Detect/testpic/water_and_heat/data.txt", 0);
	water_seepage_dec water(mc);
	water.detect();

	cvWaitKey(0);
	return 0;
}
