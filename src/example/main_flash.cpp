#include"flash_dec.h"

int main()
{
	shared_ptr<capture> mc = make_shared<capture>("G:/文档/研究生毕设/院自筹项目/测试图片/紫外/good.avi",2);
	flash_dec fdec(mc);
	fdec.detect();
	cvWaitKey(0);
	return 0;
}