#include"heat_dec.h"

int main()
{
	heat_dec heat("G:\\文档\\研究生毕设\\院自筹项目\\测试图片\\1温度与图像之间的转化\\FLIR2026.jpg");
	//heat_dec heat("G:\\文档\\研究生毕设\\院自筹项目\\测试图片\\3渗水发热图片\\FLIR2010.jpg");
	heat.detect();

	cvWaitKey(0);
	return 0;
}