#include"flash_dec.h"

int main()
{
	flash_dec fdec("G:/�ĵ�/�о�������/Ժ�Գ���Ŀ/����ͼƬ/����/good.avi");
	fdec.detect();
	cvWaitKey(0);
	return 0;
}