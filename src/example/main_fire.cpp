#include"fire_dec.h"

int main()
{
    shared_ptr<capture> mc = make_shared<capture>("/home/zxb/SRC_C/Detect/testpic/fire/FLIR2102.mp4", 2);
	fire_dec ob(mc);
	ob.detect();
	//cvWaitKey(0);

	//Mat srcImage = imread("G:\\�ĵ�\\�о�������\\Ժ�Գ���Ŀ\\����ͼƬ\\1�¶���ͼ��֮���ת��\\FLIR2026.jpg");
	//cvtColor(srcImage, srcImage, CV_BGR2GRAY);
	//imshow("ԭͼ��", srcImage);


	////����DCTϵ��������ͨ��
	//Mat DCT(srcImage.size(), CV_64FC1);
	//
	////����DCT�任
	//dct(Mat_<double>(srcImage), DCT);
	//

	//DCT = Mat_<uchar>(DCT);
	//

	//imshow("DCTͼ��", DCT);

	waitKey();
	return 0;
}
