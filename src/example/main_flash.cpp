#include"flash_dec.h"

int main()
{
	flash_dec fdec("G:/文档/研究生毕设/院自筹项目/测试图片/紫外/good.avi");
	fdec.detect();
	cvWaitKey(0);
	return 0;
}