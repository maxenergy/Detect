#include"heat_dec.h"

int main()
{
	heat_dec heat("G:\\�ĵ�\\�о�������\\Ժ�Գ���Ŀ\\����ͼƬ\\1�¶���ͼ��֮���ת��\\FLIR2026.jpg");
	//heat_dec heat("G:\\�ĵ�\\�о�������\\Ժ�Գ���Ŀ\\����ͼƬ\\3��ˮ����ͼƬ\\FLIR2010.jpg");
	heat.detect();

	cvWaitKey(0);
	return 0;
}