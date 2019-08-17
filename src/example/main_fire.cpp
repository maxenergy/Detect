#include"fire_dec.h"

int main()
{
	fire_dec ob("G:/SRC_C/Detect/testpic/火焰/FLIR2102.mp4");
	ob.detect();
	//cvWaitKey(0);

	//Mat srcImage = imread("G:\\文档\\研究生毕设\\院自筹项目\\测试图片\\1温度与图像之间的转化\\FLIR2026.jpg");
	//cvtColor(srcImage, srcImage, CV_BGR2GRAY);
	//imshow("原图像", srcImage);


	////定义DCT系数的三个通道
	//Mat DCT(srcImage.size(), CV_64FC1);
	//
	////进行DCT变换
	//dct(Mat_<double>(srcImage), DCT);
	//

	//DCT = Mat_<uchar>(DCT);
	//

	//imshow("DCT图像", DCT);

	waitKey();
	return 0;
}