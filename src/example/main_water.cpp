#include"water_seepage_dec.h"

int main()
{
	water_seepage_dec water("G:\\SRC_C\\RGBT_calibration\\water_seepage_dec\\data.txt");
	water.detect();

	cvWaitKey(0);
	return 0;
}