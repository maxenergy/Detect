#include"flash_dec.h"

int main()
{
	shared_ptr<capture> mc = make_shared<capture>("G:/�ĵ�/�о�������/Ժ�Գ���Ŀ/����ͼƬ/����/good.avi",2);
	flash_dec fdec(mc);
	fdec.detect();
	cvWaitKey(0);
	return 0;
}