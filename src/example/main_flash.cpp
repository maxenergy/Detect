#include"flash_dec.h"

int main()
{
    shared_ptr<capture> mc = make_shared<capture>("/home/zxb/SRC_C/Detect/testpic/uv/good.avi",2);
//	flash_dec fdec(mc);
//	fdec.detect();
//	cvWaitKey(0);
	return 0;
}
