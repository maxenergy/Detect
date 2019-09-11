#ifndef _TEMPALG_SDK_H_
#define _TEMPALG_SDK_H_

#include <stdio.h>

//�ϰ汾�ӿ�
extern "C" int Temperature_GetFrameInfo(int CalType,unsigned char *pBuffer,unsigned long BufSize,
			unsigned char **pData,unsigned long *DataSize,unsigned char **pTempPara,unsigned long *TempParaSize); //�ɹ�����0��ʧ�ܷ���-1
extern "C" float Temperature_GetTempFromGray( unsigned short nGray,
	float fEmissivity, float fDistance, unsigned char *pTempPara,unsigned long TempParaSize,int CalType); //�����¶�


typedef struct
{
	int nLensID;					// ��ͷID
	int nClassMinTemp;				// ���µ������
	int nClassMaxTemp;				// ���µ������
	int nDistance100;				// �����100��
	int nHumidity100;				// ����ʪ��100��
	int nAmbientTempManu100;			//�����¶�100��
	unsigned char nGpsEnable;					//GPSʹ�ܱ��
	unsigned nGpsLatitude[40];				//GPSγ��
	unsigned nGpsLongitude[40];				//GPS����
	int nReserve[20];				//Ԥ��λ
}UserTempPara;


typedef float(*Gray2Temp_Init_Back)(int nGray,float fEmissity, float fDistance,char* pAlgBuffer);
typedef float(*Gray2Temp_Real_Back)(int nGray,float fEmissity, float fDistance,char* pAlgBuffer);
typedef double(*GetIT_Back)(char* pAlgBuffer);


//��ȡ�㷨�汾
extern "C" void TEMPALG_GetVersion(char *libtempsdk_version);

//��ȡ�㷨�汾
extern "C" void TEMPALG_GetVersionMinute(char *libtempsdk_version);

//��ȡĳ�㷨���Ƿ�֧��,1��ʾ֧�֣�0��ʾ��֧��
extern "C" int TEMPALG_IsAlgSurport(int nAlgType);

//����ǰ�ĳ�ʼ����һ֡���ݱ����ʼ��һ�Σ�Ҳ����ֻ��ʼ��һ��
extern "C" float TEMPALG_Gray2Temp_Init(int nAlgType,int nGray,float fEmissity, float fDistance,char* pAlgBuffer);

//ʵ�ʲ��£������ȳ�ʼ��һ��
extern "C" extern "C" float TEMPALG_Gray2Temp_Real(int nAlgType,int nGray,float fEmissity, float fDistance,char* pAlgBuffer);

//�Ҷ�ֱ��ת�¶�
extern "C" float TEMPALG_Gray2Temp_Once(int nAlgType,int nGray,float fEmissity, float fDistance,char* pAlgBuffer);

//��ȡ����ʱ��
extern "C" double TEMPALG_GetIT(int nAlgType,char* pAlgBuffer);

//��ȡ���³�ʼ���ĺ���ָ��
extern "C" Gray2Temp_Init_Back TEMPALG_Gray2Temp_Init_GET(int nAlgType);

//��ȡʵ�ʲ��µĺ���ָ��
extern "C" Gray2Temp_Real_Back TEMPALG_Gray2Temp_Real_GET(int nAlgType);

//��ȡ����ʱ�亯��ָ��
extern "C" GetIT_Back TEMPALG_GetIT_GET(int nAlgType);

//�¶�ת�Ҷ�
extern "C" int TEMPALG_Temp2Gray(Gray2Temp_Real_Back gray2temp_realtime, char *pTempPara, float fEmissivity,
					int nMinTemp100, int nMaxTemp100, int nMinGray, int nMaxGray, int nTemp100);

extern "C" int TEMPALG_GetTempPara(int nAlgType, char* pAlgBuffer, UserTempPara *para);

#endif 
