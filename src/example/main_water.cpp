#include"water_seepage_dec.h"

int main()
{
	shared_ptr<capture> mc = make_shared<capture>("G:\\SRC_C\\RGBT_calibration\\water_seepage_dec\\data.txt", 0);
	water_seepage_dec water(mc);
	water.detect();

	cvWaitKey(0);
	return 0;
}