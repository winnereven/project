/**
 * File name: OpticalDevice.h
 * Author: Even
 * Email: 871454583@qq.com
 * Version: 1.0
 * Created on: 2017年10月4日
 * Description: 最后修改  去掉KJ接受器过载信号。
 */

#ifndef DEVICE_OPTICALDEVICE_H_
#define DEVICE_OPTICALDEVICE_H_

#include "../common.h"
#include "../sys/gpio.h"
#include "../sys/watchdog.h"
#include "../utils/DevUtils.h"

#define LED_SHINE_QUICK_COUNT	5	// *LED_DELAY_US
#define LED_SHINE_SLOW_COUNT	15
#define LED_SHINE_WORK			8
#define LED_SHINE_WORK_SOCKET	30
#define LED_DELAY_US	100*1000
#define HIGHTEMP	45//高温读数

const char* LED_CHANNEL[] = {LED1,LED2,LED3,LED4,LED5,LED6};
const uchar LEDNUM = 6;
const char* LIGHT_MODE[] = {G1BW1, G1BW2, G2BW1, G2BW2};
const uchar GMNUM = 4;
const char* KJ_MODE[] = {KJ1, KJ2};
const uchar KJNUM = 2;
const char* GMK_TEMP[] = {GMK1_T, GMK2_T,DYK_T};//高电平表示低温 ，低电平表示高温
const uchar GMTNUM = 3;

const bool LEDLIGHT = true;//高电平亮灯
const uchar DISENABLE = 0X00;//对IO口读取为0x00

class OpticalDevice {
public:
	OpticalDevice();
	~OpticalDevice();

	uchar TestLightMode(); // 检测光模块,得到背板输入信号并存储
	void TestAllMode();//检测温度以及过载部分
//	void Start();	// 启动主线程,led显示
	void SetLedStatus(uint8_t data[]);	// 设置8通道led状态
	void SetLedStatus(int iLed,uint8_t data);
	void FengShanCtrl(uchar ctrl);//控制风扇，DISENABLE 为不转
	uchar FengShanRead();
	void SetFPGARst(bool rsten);	// 重启FPGA
	void SetFeedDogInSocket(bool on);	// 设置喂狗位置，true:在Socket线程喂狗，false:在主线程喂狗
	void FeedDog();	// 喂狗
	void FeedDogInDevice();//在设备类中喂狗
	// LED初始化
	void InitLED(bool stu);
	// 看门狗初始化
	void InitWatchDog();
	// 更新LED状态
	void LedRefresh();
	// 读取IO口状态
	void InputStatus(uchar stu);

//	int m_fdUART1_FPGA, m_fdUART2_LIGHT1,m_fdUART4_LIGHT2;
//	int m_fdUART1_FPGA_EN,m_fdUART4_LIGHT2_EN;

private:
	int m_fdLightMode[GMNUM];		//入，4路光模块，readonly
	uchar m_nLightModeStatus[GMNUM];	// 0-无，1-有 ，4路光模块状态
	int m_fdKJ[KJNUM];				//入，两路，接受器功率过载检测 ，readonly
	uchar m_nKJStatus[KJNUM]; 		// 0-无，1-有，2路接受器过载
	int m_fdGMK_T[GMTNUM];			//入，3路温度传感器，readonly
	uchar m_nGMK_TStatus[GMTNUM+2]; 	// 0-无，1-有，3路温度是否超标+LM75温度+风扇状态
	int m_fdFSKZ;					//出，风扇控制，writeonly
	int m_fdLedWork[LEDNUM];	// LED1 LED2
	int m_fdWatchDog, m_fdFPGA_RST, m_fdEE_WP;
	uchar m_nLedStatus[LEDNUM];	// 0或其他-灭，1-慢闪，2-快闪，3-长亮
	int m_nLedFlag[LEDNUM];		// LED闪烁计数
	bool m_nLedWorkFlag;
	bool m_bFeedDogInSocket;	// true:在Socket线程喂狗，false:在当前线程喂狗

	void __SetGpioOn(int fd, bool on);
	string __GetDevName(int fd);



};

inline OpticalDevice::OpticalDevice() :
		m_fdWatchDog(-1), m_nLedWorkFlag(1), m_bFeedDogInSocket(false) {
	printf("OpticalDevice object created!\n");
	//4路光模块 只读
	for (int i = 0; i < GMNUM; i++) {
		m_nLightModeStatus[i] = 0;
		m_fdLightMode[i] = DevUtils::OpenDev(LIGHT_MODE[i],O_RDONLY);
	}
	//	两路过载 只读
	for (int i = 0; i < KJNUM; i++) {
		m_nKJStatus[i] = 0;
		m_fdKJ[i] = DevUtils::OpenDev(KJ_MODE[i],O_RDONLY);
	}
	//3路温度
	for (int i = 0; i < GMTNUM; i++) {
		m_nGMK_TStatus[i] = 0;
		m_fdGMK_T[i] = DevUtils::OpenDev(GMK_TEMP[i],O_RDONLY);
	}

	m_fdFSKZ = 	DevUtils::OpenDev(FSKZ);
	m_fdFPGA_RST = DevUtils::OpenDev(FPGA_RST);
	//__SetGpioOn(m_fdFPGA_RST, 0);//不启动FPGA
	m_fdEE_WP = DevUtils::OpenDev(EE_WP);


	for (int i = 0; i < LEDNUM; i++) {
		m_nLedStatus[i] = 0;
		m_fdLedWork[i] = DevUtils::OpenDev(LED_CHANNEL[i]);
	}



#if WATCH_DOG_EN
	InitWatchDog();
#endif
#if TEMP_EN
	InitIIC();
#endif

}

inline OpticalDevice::~OpticalDevice() {
	for (int i = 0; i < GMNUM; i++) {
		DevUtils::CloseDev(m_fdLightMode[i]);
	}

	DevUtils::CloseDev(m_fdFSKZ);
	DevUtils::CloseDev(m_fdFPGA_RST);
	DevUtils::CloseDev(m_fdEE_WP);

	for (int i = 0; i < GMTNUM; i++) {
		DevUtils::CloseDev(m_fdGMK_T[i]);
	}


	for (int i = 0; i < KJNUM; i++) {
		DevUtils::CloseDev(m_fdKJ[i]);
	}

	for (int i = 0; i < LEDNUM; i++) {
		DevUtils::CloseDev(m_fdLedWork[i]);
	}


#if WATCH_DOG_EN
	DevUtils::CloseDev(m_fdWatchDog);
#endif

	printf("FeederDevice object destroyed!\n");
}
inline uchar OpticalDevice::TestLightMode(){
	int ret;
//	uchar buf;
	for (int i = 0; i < GMNUM; i++) {
		ret = read(m_fdLightMode[i], &m_nLightModeStatus[i], 1);
		if (ret < 0)
			handle_err_log("%s %s", LIGHT_MODE[i], "read");
	}
	uchar temp = 0x0f;
	for(int i =0 ;i<GMNUM;i++){
		if(m_nLightModeStatus[i] == DISENABLE)
		{
			temp = temp & (temp^(0x01<<i));
			//TODO 光模块没有接入
		}
	}
	return temp;

}
inline void OpticalDevice::TestAllMode(){
	int ret;

//	for (int i = 0; i < GMNUM; i++) {
//		ret = read(m_fdLightMode[i], &m_nLightModeStatus[i], 1);
//		if (ret < 0)
//			handle_err_log("%s %s", LIGHT_MODE[i], "read");
//	}
	for (int i = 0; i < KJNUM; i++) {
		ret = read(m_fdKJ[i], &m_nKJStatus[i], 1);
		if (ret < 0)
			handle_err_log("%s %s", KJ_MODE[i], "read");
	}
	for (int i = 0; i < GMTNUM; i++) {
		ret = read(m_fdGMK_T[i], &m_nGMK_TStatus[i], 1);
		if (ret < 0)
			handle_err_log("%s %s", GMK_TEMP[i], "read");
	}
	float temp = IICReadTemp();
//	printf("temp is %f\n",temp);
	if (temp > HIGHTEMP){
		m_nGMK_TStatus[3] = 0x00;//00表示高温
	}else{
		m_nGMK_TStatus[3] = 0x01;//01表示低温
	}


}

inline void OpticalDevice::SetLedStatus(int i,uint8_t idate) {
		m_nLedStatus[i] = idate;
}


inline void OpticalDevice::SetLedStatus(uint8_t data[]) {
	for (int i = 0; i < LEDNUM; i++) {
		m_nLedStatus[i] = data[i];
	}
}

inline void OpticalDevice::InputStatus(uchar stu){
	TestAllMode();
	uchar cout = 0;
//	stu |= 0x0f;
//	for (int i = 0; i < GMNUM; i++) {
//		if(m_nLightModeStatus[i] == DISENABLE)
//		{
//			stu = stu & (stu^(0x01<<i));
//			//TODO 光模块没有介入
//		}
//	}
	/******
	for (int i = 0; i < KJNUM; i++) {
		if(m_nKJStatus[i] != DISENABLE)
		{
			//TODO 过载处理暂时屏蔽

			stu = stu | (0x10<<i);
			m_nLedStatus[i+1] = 2;
		}
		else
		{

			if(i){
				if(stu&0x0C)//非零表示2#有通道
					m_nLedStatus[2] = 3;
				else
					m_nLedStatus[2] = 0;
			}
			else{
				if(stu&0x03)//非零表示1#有通道
					m_nLedStatus[1] = 3;
				else
					m_nLedStatus[1] = 0;
			}
		}
	}****/
	if(stu & 0x3f)
	{
		switch(stu & 0x13)
		{
			case 0x00:m_nLedStatus[1] = 0;break;//表示没有通道存在
			case 0x01:
			case 0x02:
			case 0x03:m_nLedStatus[1] = 3;break;//表示通道存在但是没有开启
			default:m_nLedStatus[1] = 1;break;//慢闪表示激光源开启中
		}
		switch(stu & 0x2c)
		{
			case 0x00:m_nLedStatus[2] = 0;break;//表示没有通道存在
			case 0x04:
			case 0x08:
			case 0x0c:m_nLedStatus[2] = 3;break;//表示通道存在但是没有开启
			default:m_nLedStatus[2] = 1;break;//慢闪表示激光源开启中
		}

	}else//两个模块都没有连接
	{
		m_nLedStatus[1] = 0;
		m_nLedStatus[2] = 0;
	}


	for (int i = 0; i < GMTNUM+1; i++) {
		if(m_nGMK_TStatus[i] == DISENABLE)//表示有高温
		{
			FengShanCtrl(DISENABLE+1);
		}
		else
		{
			cout++;
		}
	}

	if(cout == GMTNUM+1)
	{
		time(&now);
		timenow = localtime(&now);
		if((timenow->tm_sec)<1)
			FengShanCtrl(DISENABLE);
//		printf("cloud is closed\n");
	}
//	printf("cont is %d\n",cout);
//	cout = 0;
//	return stu;
}
inline uchar OpticalDevice::FengShanRead(){

	return m_nGMK_TStatus[GMTNUM+1];

}
inline void OpticalDevice::FengShanCtrl(uchar ctrl){
	if(ctrl == DISENABLE){
		__SetGpioOn(m_fdFSKZ, 0);
		m_nGMK_TStatus[GMTNUM+1]=DISENABLE;
		//printf("the fs is close!\n");
	}
	else
	{
		__SetGpioOn(m_fdFSKZ, 1);
		m_nGMK_TStatus[GMTNUM+1]=DISENABLE+1;
		//__SetGpioOn(m_fdFSKZ, 1);
		//printf("the fs is open!\n");

	}
}
inline void OpticalDevice::SetFPGARst(bool rsten) {
	if(rsten)
		__SetGpioOn(m_fdFPGA_RST, false);//先低
	usleep(500);
	__SetGpioOn(m_fdFPGA_RST, true);//后高
}

inline void OpticalDevice::SetFeedDogInSocket(bool on) {
	m_bFeedDogInSocket = on;
}
inline void OpticalDevice::FeedDogInDevice(){
	if(!m_bFeedDogInSocket)
	{
		FeedDog();
		__SetGpioOn(m_fdFPGA_RST, 0);//不启动FPGA
	}
}
inline void OpticalDevice::FeedDog() {
	write(m_fdWatchDog, "\0", 1);
}

inline void OpticalDevice::LedRefresh() {
	for (int i = 0; i < LEDNUM; i++) {
		m_nLedFlag[i]++;
		switch (m_nLedStatus[i]) {
		case 1:
			if (m_nLedFlag[i] >= LED_SHINE_SLOW_COUNT) {
				m_nLedFlag[i] = 0;
				__SetGpioOn(m_fdLedWork[i], LEDLIGHT);
			} else
				__SetGpioOn(m_fdLedWork[i], !LEDLIGHT);
			break;
		case 2:
			if (m_nLedFlag[i] >= LED_SHINE_QUICK_COUNT) {
				m_nLedFlag[i] = 0;
				__SetGpioOn(m_fdLedWork[i], LEDLIGHT);
			} else
				__SetGpioOn(m_fdLedWork[i], !LEDLIGHT);
			break;
		case 3:
			m_nLedFlag[i] = 0;
			__SetGpioOn(m_fdLedWork[i], LEDLIGHT);
			break;
		default:
			m_nLedFlag[i] = 0;
			__SetGpioOn(m_fdLedWork[i], !LEDLIGHT);
			break;
		}
	}

}

inline void OpticalDevice::__SetGpioOn(int fd, bool on) {
	int ret = ioctl(fd, on ? SET_GPIO_HIGHT : SET_GPIO_LOW);
	string dev_name = __GetDevName(fd);
	if (ret < 0)
		handle_err_log("%s %s", dev_name.c_str(), "ioctl");
}

inline string OpticalDevice::__GetDevName(int fd) {
	string dev_name = "null";
	if (fd == m_fdFSKZ)
		dev_name = FSKZ;
	else if (fd == m_fdWatchDog)
		dev_name = DEV_WATCHDOG;
	else if (fd == m_fdFPGA_RST)
		dev_name = FPGA_RST;
	else if (fd == m_fdEE_WP)
		dev_name = EE_WP;
	else {
		for (int i = 0; i < LEDNUM; i++)
			if (fd == m_fdLedWork[i]) {
				dev_name = LED_CHANNEL[i];
				break;
			}
	}
	return dev_name;
}



inline void OpticalDevice::InitLED(bool stu) {
	for (int i = 0; i < LEDNUM; i++) {
		__SetGpioOn(m_fdLedWork[i], stu);
	}
	m_nLedStatus[0]=2;
}

inline void OpticalDevice::InitWatchDog() {
	int timeout = WATCH_DOG_TIMEOUT;//
	m_fdWatchDog = DevUtils::OpenDev(DEV_WATCHDOG, O_WRONLY);
	printf("WDT is opened!\n");
	ioctl(m_fdWatchDog, WDIOC_SETTIMEOUT, &timeout);
	ioctl(m_fdWatchDog, WDIOC_GETTIMEOUT, &timeout);
	printf("The timeout was is %d seconds\n", timeout);
}



#endif /* DEVICE_OPTICALDEVICE_H_ */
