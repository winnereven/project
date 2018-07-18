//============================================================================
// Name        : optical_fiber_project.cpp
// Author      : even
// Version     : 3.0
// Copyright   : all rights reserved
// Description :
//============================================================================

#define DEBUG 0

#include "../common.h"
#include "../device/OpticalDevice.h"
#include "../socket/SocketServer.h"
#include "../uart/UartServer.h"
#include "../data/DataCache.h"
#include "../data/UartCache.h"
#include "../data/Msg.h"
#include "../data/SendData.h"
using namespace std;

uchar RecvBuf[BUFFER_SIZE];
//bit7 bit6 bit5	 bit4 	bit3 			bit2 			bit1 		bit0
//X		X	过载2	过载1	2#光模块2通道	2#光模块1通道	1#2通道		1#1通道
//1表示使能    	0表示无使能
uchar HEART=0x0f;//^^

int datalen;
//int DEBUG=0;
int f_dUart[UARTNUM] = {0,0,0,0,0};//五个串口地址   0:调试串口  1:1#光模块   2:2#光模块   3:1#FPGA  4:2#FPGA

OpticalDevice* mOpticalDevice;//设备类
UartServer* mUartServer;//串口服务类
DataCache* mDataCache;//数据处理类
UartCache* mUartCache;//数据处理类
SocketServer* mSocketServer;//网口服务类
Msg* mSendMsg;//发送数据的结构
Msg* mUartRcvMsg;//串口接受到数据的结构
Msg* mSocketRcvMsg;//网口接受到数据的结构
SendData* mSendData;//发送入缓存


void *SendDataThread(void *arg) {
	while(true){
		int idsend=mSendData->PushOutArray(mSendMsg);
		if(idsend==1)//返回1说明有数据需要发送
		{
			int id;
			for(id=0;id<UARTNUM;id++){
				if(mSendMsg->f_ddircetion == f_dUart[id])
					break;
			}
			switch(id)//判断目标信道
			{
				case UADEBUG:
					//mSendMsg->Update();
					mSocketServer->SendMsg(mSendMsg);
#if DEBUG
					mUartServer->Uart0Send(mSendMsg->data,mSendMsg->dataLen);

#endif
					break;
				case UALIGHT1:
				case UALIGHT2:mOpticalDevice->SetFPGARst(false);//开启激光器的时候复位FPGA
				case UAFPGA1:
				case UAFPGA2:
					mUartServer->UartSendByfd(mSendMsg->f_ddircetion,mSendMsg->data,mSendMsg->dataLen);break;

				default:break;
			}
		}
		else if(idsend > 0){
			int id;
			for(id=0;id<UARTNUM;id++){
				if(idsend == f_dUart[id])//表示该通道故障
				{
					uchar buf[8]={0x24,0x24,0xfe,0x02,0x00,0x00,0x00,0x0d};
					buf[5] = 0x01<<(id-1);
					mSocketServer->Sendbuf(buf,8);
#if DEBUG
					printf("channel %d is error \n",id	);
#endif
				}

			}

		}else
		{
			//usleep(DATA_PARSE_DELAY_US);
		}

		//*mSendMsg=NULL;//清空
		//usleep(DATA_PARSE_DELAY_US);
	}
	pthread_exit(NULL);
}


void *UartRcvThread(void *arg) {

	int iuart;
	while (true) {
		for(iuart=0;iuart<UARTNUM;iuart++){

			mUartServer->GetUartData(f_dUart[iuart],RecvBuf,&datalen);
			if(datalen > 0)
			{
#if DEBUG
				printf("receive data:");
				for (int i = 0; i < datalen; i++) {
					printf("%x ", RecvBuf[i]);
				}
				printf("\n");
#endif
				if(iuart == UALIGHT1 || iuart == UALIGHT2)//表示1、2号光模块收到数据
				{
					//printf("clean waiting\n");
					//需要打包 再调用socket发送函数
					if(((RecvBuf[1]&0x0b)==0x0a)&&(RecvBuf[2]==0x00 ))//表示该光模块已经开启
					{
						HEART = HEART | (0x08<<iuart);
					}
					if(((RecvBuf[1]&0x0b)==0x08)&&(RecvBuf[2]==0x00 ))//表示该光模块已经关闭
					{
						HEART = HEART & (HEART ^ (0x08<<iuart));
					}
					if(datalen<6)
						break;
					Msg* backMsg= new Msg(f_dUart[iuart],f_dUart[0],iuart==1?0x01:0x03,RecvBuf,datalen,0,false);
					mSendData->PushInByMsgAll(2,backMsg);
				}
//				else if(iuart == UAFPGA1 || iuart == UAFPGA1)//来自fpga串口
//				{
//					mSendData->PushInArray(2,f_dUart[iuart],f_dUart[0],0,RecvBuf,datalen);
//				}
				else//来自调试串口0或者 FPGA
				{
					while(mUartCache->UartRevArray(iuart, RecvBuf, datalen, NULL, mUartRcvMsg))
					{

						int ret = mUartServer->DataSorting(f_dUart[iuart],mUartRcvMsg,NULL);
						//mUartRcvMsg=mUartRcvMsg->Update();
						switch(ret){
							case 0:printf("DataSorting failed \n");break;//失败
							case 1:mSendData->PushInByMsgData(1,mUartRcvMsg);break;//发给光模块
							case 2:mSendData->PushInByMsgAll(1,mUartRcvMsg);break;//发给单路FPGA
							case 3:mSendData->PushInByMsgAll(1,mUartRcvMsg);
									mSocketRcvMsg->f_ddircetion=f_dUart[UAFPGA2];
									mSendData->PushInByMsgAll(1,mUartRcvMsg);
									break;//发给多路FPGA
							case 4:mSendData->PushInByMsgAll(2,mUartRcvMsg);break;//发给计算机
							case 5:
								switch(mUartRcvMsg->buf[4])
								{
									case 0x55:mOpticalDevice->SetFPGARst(true);//复位FPGA
									case 0xaa:
									default:
										mUartRcvMsg->buf[4]=HEART;
										mSocketServer->Sendbuf(mUartRcvMsg->buf,mUartRcvMsg->len);
										break;
								}break;
							case 6://风扇控制
								mUartRcvMsg->buf[2]=0x1f;
								switch(mUartRcvMsg->buf[4])
								{
									case 0x00:
									case 0x01:
										mOpticalDevice->FengShanCtrl(mUartRcvMsg->buf[4]);
										mSocketServer->Sendbuf(mUartRcvMsg->buf,mUartRcvMsg->len);
										break;
									default:
										mUartRcvMsg->buf[4]=mOpticalDevice->FengShanRead();
										mSocketServer->Sendbuf(mUartRcvMsg->buf,mUartRcvMsg->len);
										break;
								}break;
							case 7:
								mUartRcvMsg->buf[2]=0x1e;
								mUartRcvMsg->buf[4]=0x01;
								mSocketServer->Sendbuf(mUartRcvMsg->buf,mUartRcvMsg->len);
								exit(EXIT_SUCCESS);
								break;
							default:break;
						}
						datalen = 0;
					}
				}

				memset(RecvBuf, 0, sizeof(RecvBuf));
			}
			else
			{
				//printf("read date is empty!\n");
			}
		}



		//usleep(DATA_PARSE_DELAY_US);
	}
	pthread_exit(NULL);
}

void *SocketRcvThread(void *arg) {

	while(true){
		mSocketServer->GetNetAccept();
#if WATCH_DOG_EN
		mOpticalDevice->SetFeedDogInSocket(true);

#endif
		while(mSocketServer->CheckStatu()){
			try{
				int f_dsource;//来源信道

				f_dsource = mSocketServer->GetNetData(RecvBuf,&datalen);
				if (datalen  == 0) {
		//				m_bUartEn = true;
		//				mOpticalDevice->SetFeedDogInSocket(false);
					mOpticalDevice->SetFeedDogInSocket(false);
					mSocketServer->DisConnect();
					printf("client disconnect\n");
					break ;
				}
				if(f_dsource > 30){
					//连接次数过多，重新启动
				}
#if WATCH_DOG_EN
				mOpticalDevice->FeedDog();
#endif
#if DEBUG
				printf("receive data:");
				for (int i = 0; i < datalen ; i++) {
					printf("%x ", RecvBuf[i]);
				}
				printf("\n");
				mUartServer->Uart0Send(RecvBuf,datalen);
#endif
				while(mDataCache->MsgPreParse(f_dsource, RecvBuf, datalen, NULL, mSocketRcvMsg))
				{
					int ret = mUartServer->DataSorting(f_dsource,mSocketRcvMsg,NULL);
					switch(ret){
						case 0:printf("DataSorting failed \n");break;//失败
						case 1:mSendData->PushInByMsgData(1,mSocketRcvMsg);break;//发给光模块
						case 2:mSendData->PushInByMsgAll(1,mSocketRcvMsg);break;//发给单路FPGA
						case 3:mSendData->PushInByMsgAll(1,mSocketRcvMsg);
								mSocketRcvMsg->f_ddircetion=f_dUart[UAFPGA2];
								mSendData->PushInByMsgAll(1,mSocketRcvMsg);
								break;//发给多路FPGA
						case 4:mSendData->PushInByMsgAll(2,mSocketRcvMsg);break;//发给计算机
						case 5:
							switch(mSocketRcvMsg->buf[4])
							{
								case 0x55:mOpticalDevice->SetFPGARst(true);//复位FPGA
								case 0xaa:
								default:
									mSocketRcvMsg->buf[4]=HEART;
									mSocketServer->Sendbuf(mSocketRcvMsg->buf,mSocketRcvMsg->len);
									printf("heart send successful!\n");
									break;
							}break;
						case 6://风扇控制
							mSocketRcvMsg->buf[2]=0x1f;
							switch(mSocketRcvMsg->buf[4])
							{
								case 0x00:
								case 0x01:
									mOpticalDevice->FengShanCtrl(mSocketRcvMsg->buf[4]);
									mSocketServer->Sendbuf(mSocketRcvMsg->buf,mSocketRcvMsg->len);
									break;
								default:
									mSocketRcvMsg->buf[4]=mOpticalDevice->FengShanRead();
									mSocketServer->Sendbuf(mSocketRcvMsg->buf,mSocketRcvMsg->len);
									break;
							}break;
						case 7:
							mSocketRcvMsg->buf[2]=0x1e;
							mSocketRcvMsg->buf[4]=0x01;
							mSocketServer->Sendbuf(mSocketRcvMsg->buf,mSocketRcvMsg->len);
							exit(EXIT_SUCCESS);
							break;
						default:break;
					}
					datalen = 0;
				}

				memset(RecvBuf, 0, sizeof(RecvBuf));
			}
			catch(exception e)
			{
				printf("the exception is %s\n",e.what());
				pthread_exit(NULL);
			}
		}
		//
		if(mSocketServer->CheckStatu()==0){
//			mSocketServer->~SocketServer();
			mOpticalDevice->SetFeedDogInSocket(false);
//			mOpticalDevice->SetFPGARst();
//			mSocketServer = new SocketServer();

		}
	}
	pthread_exit(NULL);

}

int main(int argc, char* argv[]) {
	// 版本查询
	if(argc == 2){
		if(strcmp(argv[1], "--version") == 0)
			printf("version：%s\n", VERSION);
		else if(strcmp(argv[1], "--debug") == 0){
//#define DEBUG
			//DEBUG = 1;
			;
		}
		else
			printf("command error,  --version\n");
		exit(EXIT_SUCCESS);
	}
	mOpticalDevice = new OpticalDevice();

	mOpticalDevice->InitLED(LEDLIGHT);	// 开机所有led点亮2s
	HEART=mOpticalDevice->TestLightMode();//心跳填充


	sleep(2);
	mOpticalDevice->InitLED(!LEDLIGHT);	// 开机所有led点亮2s
	sleep(2);

	mUartServer = new UartServer(f_dUart);
	mDataCache = new DataCache();
	mUartCache = new UartCache();



	pthread_t tdSocket, tdUart, tdSendData;

#if CLOSE_FIBER
	sleep(15);
	uchar read[4]={0x8f,0x01,0x00,0x01};
	uchar close[4]={0x8f,0x71,0x00,0x71};

	if(HEART&0x03)
	{
		mUartServer->UartSendByfd(f_dUart[UALIGHT1],read,4);
		mUartServer->UartSendByfd(f_dUart[UALIGHT1],close,4);

	}
	if(HEART&0x0c)
	{
		mUartServer->UartSendByfd(f_dUart[UALIGHT2],read,4);
		mUartServer->UartSendByfd(f_dUart[UALIGHT2],close,4);
	}
#endif
	mSocketServer = new SocketServer();
	mSendMsg = new Msg;
	mUartRcvMsg = new Msg;
	mSocketRcvMsg = new Msg;
	mSendData = new SendData;

	int ret = pthread_create(&tdSocket, NULL, SocketRcvThread, NULL);
	if (ret != 0)
		handle_error_en(ret, "SocketServerThread create failed!");

	ret = pthread_create(&tdUart, NULL, UartRcvThread, NULL);
	if (ret != 0)
		handle_error_en(ret, "UartServerThread create failed!");

	ret = pthread_create(&tdSendData, NULL, SendDataThread, NULL);
	if (ret != 0)
		handle_error_en(ret, "SendDataThread create failed!");

	while (true) {

#if RESTART_MIDNIGHT
			//count++;
			if(!(mSocketServer->CheckStatu()) && count++ >= 300){	// 每30秒检测一次系统时间
				count = 0;
				time(&now);
				timenow = localtime(&now);
				if(timenow->tm_hour == 1 && timenow->tm_min==0){	// 当系统时间为01:00时重启
					exit(EXIT_SUCCESS);
				}
			}
#endif
			mOpticalDevice->InputStatus(HEART);//填充心跳
			if(mSocketServer->CheckStatu())
				mOpticalDevice->SetLedStatus(0,1);
			else
				mOpticalDevice->SetLedStatus(0,2);
			mOpticalDevice->LedRefresh();
#if WATCH_DOG_EN
				mOpticalDevice->FeedDogInDevice();
#endif
			if(pthread_kill(tdSocket,0)!=0)//发送信号0，当返回为0时，表示线程正常，不为0，则重新启动线程
			{
				pthread_join(tdSocket,NULL);//线程回收
				pthread_create(&tdSocket, NULL, SocketRcvThread, NULL);//线程重新创建
			}
			if(pthread_kill(tdUart,0)!=0)
			{
				pthread_join(tdUart,NULL);
				pthread_create(&tdUart, NULL, UartRcvThread, NULL);
			}
			if(pthread_kill(tdSendData,0)!=0)
			{
				pthread_join(tdSendData,NULL);
				pthread_create(&tdSendData, NULL, SendDataThread, NULL);
			}

			usleep(LED_DELAY_US);
	}

	pthread_exit(NULL);
	return 0;
}
