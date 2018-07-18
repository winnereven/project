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
//X		X	����2	����1	2#��ģ��2ͨ��	2#��ģ��1ͨ��	1#2ͨ��		1#1ͨ��
//1��ʾʹ��    	0��ʾ��ʹ��
uchar HEART=0x0f;//^^

int datalen;
//int DEBUG=0;
int f_dUart[UARTNUM] = {0,0,0,0,0};//������ڵ�ַ   0:���Դ���  1:1#��ģ��   2:2#��ģ��   3:1#FPGA  4:2#FPGA

OpticalDevice* mOpticalDevice;//�豸��
UartServer* mUartServer;//���ڷ�����
DataCache* mDataCache;//���ݴ�����
UartCache* mUartCache;//���ݴ�����
SocketServer* mSocketServer;//���ڷ�����
Msg* mSendMsg;//�������ݵĽṹ
Msg* mUartRcvMsg;//���ڽ��ܵ����ݵĽṹ
Msg* mSocketRcvMsg;//���ڽ��ܵ����ݵĽṹ
SendData* mSendData;//�����뻺��


void *SendDataThread(void *arg) {
	while(true){
		int idsend=mSendData->PushOutArray(mSendMsg);
		if(idsend==1)//����1˵����������Ҫ����
		{
			int id;
			for(id=0;id<UARTNUM;id++){
				if(mSendMsg->f_ddircetion == f_dUart[id])
					break;
			}
			switch(id)//�ж�Ŀ���ŵ�
			{
				case UADEBUG:
					//mSendMsg->Update();
					mSocketServer->SendMsg(mSendMsg);
#if DEBUG
					mUartServer->Uart0Send(mSendMsg->data,mSendMsg->dataLen);

#endif
					break;
				case UALIGHT1:
				case UALIGHT2:mOpticalDevice->SetFPGARst(false);//������������ʱ��λFPGA
				case UAFPGA1:
				case UAFPGA2:
					mUartServer->UartSendByfd(mSendMsg->f_ddircetion,mSendMsg->data,mSendMsg->dataLen);break;

				default:break;
			}
		}
		else if(idsend > 0){
			int id;
			for(id=0;id<UARTNUM;id++){
				if(idsend == f_dUart[id])//��ʾ��ͨ������
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

		//*mSendMsg=NULL;//���
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
				if(iuart == UALIGHT1 || iuart == UALIGHT2)//��ʾ1��2�Ź�ģ���յ�����
				{
					//printf("clean waiting\n");
					//��Ҫ��� �ٵ���socket���ͺ���
					if(((RecvBuf[1]&0x0b)==0x0a)&&(RecvBuf[2]==0x00 ))//��ʾ�ù�ģ���Ѿ�����
					{
						HEART = HEART | (0x08<<iuart);
					}
					if(((RecvBuf[1]&0x0b)==0x08)&&(RecvBuf[2]==0x00 ))//��ʾ�ù�ģ���Ѿ��ر�
					{
						HEART = HEART & (HEART ^ (0x08<<iuart));
					}
					if(datalen<6)
						break;
					Msg* backMsg= new Msg(f_dUart[iuart],f_dUart[0],iuart==1?0x01:0x03,RecvBuf,datalen,0,false);
					mSendData->PushInByMsgAll(2,backMsg);
				}
//				else if(iuart == UAFPGA1 || iuart == UAFPGA1)//����fpga����
//				{
//					mSendData->PushInArray(2,f_dUart[iuart],f_dUart[0],0,RecvBuf,datalen);
//				}
				else//���Ե��Դ���0���� FPGA
				{
					while(mUartCache->UartRevArray(iuart, RecvBuf, datalen, NULL, mUartRcvMsg))
					{

						int ret = mUartServer->DataSorting(f_dUart[iuart],mUartRcvMsg,NULL);
						//mUartRcvMsg=mUartRcvMsg->Update();
						switch(ret){
							case 0:printf("DataSorting failed \n");break;//ʧ��
							case 1:mSendData->PushInByMsgData(1,mUartRcvMsg);break;//������ģ��
							case 2:mSendData->PushInByMsgAll(1,mUartRcvMsg);break;//������·FPGA
							case 3:mSendData->PushInByMsgAll(1,mUartRcvMsg);
									mSocketRcvMsg->f_ddircetion=f_dUart[UAFPGA2];
									mSendData->PushInByMsgAll(1,mUartRcvMsg);
									break;//������·FPGA
							case 4:mSendData->PushInByMsgAll(2,mUartRcvMsg);break;//���������
							case 5:
								switch(mUartRcvMsg->buf[4])
								{
									case 0x55:mOpticalDevice->SetFPGARst(true);//��λFPGA
									case 0xaa:
									default:
										mUartRcvMsg->buf[4]=HEART;
										mSocketServer->Sendbuf(mUartRcvMsg->buf,mUartRcvMsg->len);
										break;
								}break;
							case 6://���ȿ���
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
				int f_dsource;//��Դ�ŵ�

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
					//���Ӵ������࣬��������
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
						case 0:printf("DataSorting failed \n");break;//ʧ��
						case 1:mSendData->PushInByMsgData(1,mSocketRcvMsg);break;//������ģ��
						case 2:mSendData->PushInByMsgAll(1,mSocketRcvMsg);break;//������·FPGA
						case 3:mSendData->PushInByMsgAll(1,mSocketRcvMsg);
								mSocketRcvMsg->f_ddircetion=f_dUart[UAFPGA2];
								mSendData->PushInByMsgAll(1,mSocketRcvMsg);
								break;//������·FPGA
						case 4:mSendData->PushInByMsgAll(2,mSocketRcvMsg);break;//���������
						case 5:
							switch(mSocketRcvMsg->buf[4])
							{
								case 0x55:mOpticalDevice->SetFPGARst(true);//��λFPGA
								case 0xaa:
								default:
									mSocketRcvMsg->buf[4]=HEART;
									mSocketServer->Sendbuf(mSocketRcvMsg->buf,mSocketRcvMsg->len);
									printf("heart send successful!\n");
									break;
							}break;
						case 6://���ȿ���
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
	// �汾��ѯ
	if(argc == 2){
		if(strcmp(argv[1], "--version") == 0)
			printf("version��%s\n", VERSION);
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

	mOpticalDevice->InitLED(LEDLIGHT);	// ��������led����2s
	HEART=mOpticalDevice->TestLightMode();//�������


	sleep(2);
	mOpticalDevice->InitLED(!LEDLIGHT);	// ��������led����2s
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
			if(!(mSocketServer->CheckStatu()) && count++ >= 300){	// ÿ30����һ��ϵͳʱ��
				count = 0;
				time(&now);
				timenow = localtime(&now);
				if(timenow->tm_hour == 1 && timenow->tm_min==0){	// ��ϵͳʱ��Ϊ01:00ʱ����
					exit(EXIT_SUCCESS);
				}
			}
#endif
			mOpticalDevice->InputStatus(HEART);//�������
			if(mSocketServer->CheckStatu())
				mOpticalDevice->SetLedStatus(0,1);
			else
				mOpticalDevice->SetLedStatus(0,2);
			mOpticalDevice->LedRefresh();
#if WATCH_DOG_EN
				mOpticalDevice->FeedDogInDevice();
#endif
			if(pthread_kill(tdSocket,0)!=0)//�����ź�0��������Ϊ0ʱ����ʾ�߳���������Ϊ0�������������߳�
			{
				pthread_join(tdSocket,NULL);//�̻߳���
				pthread_create(&tdSocket, NULL, SocketRcvThread, NULL);//�߳����´���
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
