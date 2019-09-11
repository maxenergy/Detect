#ifndef _TEMPALG_SDK_H_
#define _TEMPALG_SDK_H_

#include <stdio.h>

//老版本接口
extern "C" int Temperature_GetFrameInfo(int CalType,unsigned char *pBuffer,unsigned long BufSize,
			unsigned char **pData,unsigned long *DataSize,unsigned char **pTempPara,unsigned long *TempParaSize); //成功返回0，失败返回-1
extern "C" float Temperature_GetTempFromGray( unsigned short nGray,
	float fEmissivity, float fDistance, unsigned char *pTempPara,unsigned long TempParaSize,int CalType); //返回温度


typedef struct
{
	int nLensID;					// 镜头ID
	int nClassMinTemp;				// 测温档最低温
	int nClassMaxTemp;				// 测温档最高温
	int nDistance100;				// 距离的100倍
	int nHumidity100;				// 环境湿度100倍
	int nAmbientTempManu100;			//环境温度100倍
	unsigned char nGpsEnable;					//GPS使能标记
	unsigned nGpsLatitude[40];				//GPS纬度
	unsigned nGpsLongitude[40];				//GPS经度
	int nReserve[20];				//预留位
}UserTempPara;


typedef float(*Gray2Temp_Init_Back)(int nGray,float fEmissity, float fDistance,char* pAlgBuffer);
typedef float(*Gray2Temp_Real_Back)(int nGray,float fEmissity, float fDistance,char* pAlgBuffer);
typedef double(*GetIT_Back)(char* pAlgBuffer);


//获取算法版本
extern "C" void TEMPALG_GetVersion(char *libtempsdk_version);

//获取算法版本
extern "C" void TEMPALG_GetVersionMinute(char *libtempsdk_version);

//获取某算法号是否支持,1表示支持，0表示不支持
extern "C" int TEMPALG_IsAlgSurport(int nAlgType);

//测温前的初始化，一帧数据必须初始化一次，也可以只初始化一次
extern "C" float TEMPALG_Gray2Temp_Init(int nAlgType,int nGray,float fEmissity, float fDistance,char* pAlgBuffer);

//实际测温，必须先初始化一次
extern "C" extern "C" float TEMPALG_Gray2Temp_Real(int nAlgType,int nGray,float fEmissity, float fDistance,char* pAlgBuffer);

//灰度直接转温度
extern "C" float TEMPALG_Gray2Temp_Once(int nAlgType,int nGray,float fEmissity, float fDistance,char* pAlgBuffer);

//获取积分时间
extern "C" double TEMPALG_GetIT(int nAlgType,char* pAlgBuffer);

//获取测温初始化的函数指针
extern "C" Gray2Temp_Init_Back TEMPALG_Gray2Temp_Init_GET(int nAlgType);

//获取实际测温的函数指针
extern "C" Gray2Temp_Real_Back TEMPALG_Gray2Temp_Real_GET(int nAlgType);

//获取积分时间函数指针
extern "C" GetIT_Back TEMPALG_GetIT_GET(int nAlgType);

//温度转灰度
extern "C" int TEMPALG_Temp2Gray(Gray2Temp_Real_Back gray2temp_realtime, char *pTempPara, float fEmissivity,
					int nMinTemp100, int nMaxTemp100, int nMinGray, int nMaxGray, int nTemp100);

extern "C" int TEMPALG_GetTempPara(int nAlgType, char* pAlgBuffer, UserTempPara *para);

#endif 
