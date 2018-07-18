/**
 * File name: SendData.h
 * Author: even
 * Email: 871454583@qq.com
 * Version: 1.0
 * Created on: Sep 06, 2017
 * Description:�ŵ�����
 * bit0		bit1		bit2		bit3		bit4~bitn
 * ��Դ�ŵ�	Ŀ���ŵ�		�Ƿ�ظ�		���ݳ���		����
 */

#ifndef SENDDATA_H_
#define SENDDATA_H_

#include "../common.h"

struct sendCache{
	int front;//ǰָ��
	int rear;//��ָ��
	int current;//��ǰλ��
	int len;//����
	int tag;//��ɱ�־
	uchar SendBuf[BUFFER_SIZE];//f_dsocket > f_duart

};

class SendData {
public:
	SendData();
	~SendData();
	int PushInArray(int cacheNum,int f_dsource,int f_ddirection,int waitflag,uchar* buf,int len);//��������ջ
	int PushInByMsgData(int cacheNum,Msg* msg);
	int PushInByMsgAll(int cacheNum,Msg* msg);//
	int SendAgain(int cacheNum,int iChanl);//�ٴη�������
	int PushOutArray(Msg* msg);//�����ݳ�ջ
	int UpdatePoint(int cacheNum,int iChanl);
private:

	sendCache UartSendCache;
	sendCache SockSendCache;
//	int front;//ǰָ��
//	int rear;//��ָ��
//	int current;//��ǰλ��
//	int len;//����
//	int tag;//��ɱ�־
	const static uchar ChNum = 30;
	int ChannelStu[ChNum];
//	uchar SendBufUart[BUFFER_SIZE];//f_dsocket > f_duart
};

inline SendData::SendData(){

}
inline SendData::~SendData(){

}
//���Ͷ��д洢��ʽ0.��������1��ʾ���� 2 ��ʾ���ڣ� 1.��Դ�ŵ� 2.Ŀ���ŵ� 3.�ȴ���־ 4.���ݳ��� 5.����
inline int SendData::PushInArray(int cacheNum,int f_dsource,int f_ddirection,int waitflag,uchar* buf,int len){
	sendCache* msendCache;
	if(cacheNum == 1){
		msendCache = &UartSendCache;
	}
	else
	{
		msendCache = &SockSendCache;
	}
	int temp = msendCache->rear;//���浱ǰָ��
	msendCache->SendBuf[msendCache->rear] = f_dsource;
	if(ChannelStu[f_dsource] > 0)ChannelStu[f_dsource]=0;
	msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	msendCache->SendBuf[msendCache->rear] = f_ddirection;
	msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	msendCache->SendBuf[msendCache->rear] = waitflag;

	msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	msendCache->SendBuf[msendCache->rear]  = len;
	msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	for(int i=0;i<len;i++){
		msendCache->SendBuf[msendCache->rear] = *(buf+i);
		msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	}
	if(msendCache->rear <= msendCache->front)//memory is full
	{
		if(msendCache->current == msendCache->front){
			//��ʾû����Ҫ��������ݣ�������ո�֡
			msendCache->current = msendCache->rear;
			return SendAgain(cacheNum,f_ddirection);

		}else
		{
			msendCache->rear = temp;
			return 0;//failed!
			//��ʾ��ǰ����������Ҫ�ط��ʹ���
		}
	}
	else{
		msendCache->tag++;
		return 1;//successful
	}
}
//��Msg���ݴ��뷢�ͻ���--���͸���ģ��     cacheNum��������1��ʾ���� 2 ��ʾ���ڣ�
inline int SendData::PushInByMsgData(int cacheNum,Msg* msg){
	sendCache* msendCache;
	if(cacheNum == 1){
		msendCache = &UartSendCache;
	}
	else
	{
		msendCache = &SockSendCache;
	}
	int temp = msendCache->rear;//���浱ǰָ��
	msendCache->SendBuf[msendCache->rear] = msg->f_dsource;
	if(ChannelStu[msg->f_dsource] > 0)ChannelStu[msg->f_dsource]=0;
	msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	msendCache->SendBuf[msendCache->rear] = msg->f_ddircetion;
	msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	msendCache->SendBuf[msendCache->rear] = msg->iswait;
	msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	msendCache->SendBuf[msendCache->rear] = msg->dataLen;
	msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;

	for(int i=0;i<msg->dataLen;i++){
		msendCache->SendBuf[msendCache->rear] = msg->data[i];
		msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	}
	if(msendCache->rear <= msendCache->front)//memory is full
	{
		if(msendCache->current == msendCache->front){
			//��ʾû����Ҫ��������ݣ�������ո�֡
			msendCache->current = msendCache->rear;
			return SendAgain(cacheNum,msg->f_ddircetion);

		}else
		{
			msendCache->rear = temp;
			return 0;//failed!
			//��ʾ��ǰ����������Ҫ�ط��ʹ���
		}
	}
	else{
		msendCache->tag++;
#if DEBUG
		printf("push data by point:\n");
		for(int idebug=0;idebug<(msendCache->rear-msendCache->front);idebug++)
		{
			printf("%x  ",msendCache->SendBuf[msendCache->front+idebug]);
		}
		printf("\n and tag is : %d\n",msendCache->tag	);
		printf("successful!\n");
#endif
		return 1;//successful
	}

}
//��Msg->buf������ȫ�����뷢�ͻ���--���͸����������FPGA   cacheNum��������1��ʾ���� 2 ��ʾ���ڣ�
inline int SendData::PushInByMsgAll(int cacheNum,Msg* msg){
	sendCache* msendCache;
	if(cacheNum == 1){
		msendCache = &UartSendCache;
	}
	else
	{
		msendCache = &SockSendCache;
	}
	int temp = msendCache->rear;//���浱ǰָ��
	msendCache->SendBuf[msendCache->rear] = msg->f_dsource;
	if(ChannelStu[msg->f_dsource] > 0)
		if(msg->len<30)
		{
			ChannelStu[msg->f_dsource]=0;
			UpdatePoint(1,msg->f_dsource);
		}
	msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	msendCache->SendBuf[msendCache->rear] = msg->f_ddircetion;
	msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	msendCache->SendBuf[msendCache->rear] = msg->iswait;
	msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	msendCache->SendBuf[msendCache->rear] = msg->len;
	msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;

	for(int i=0;i<msg->len;i++){
		msendCache->SendBuf[msendCache->rear] = msg->buf[i];
		msendCache->rear = (msendCache->rear + 1) % BUFFER_SIZE;
	}
	if(msendCache->rear <= msendCache->front)//memory is full
	{
		if(msendCache->current == msendCache->front){
			//��ʾû����Ҫ��������ݣ�������ո�֡
			msendCache->current = msendCache->rear;
			return SendAgain(cacheNum,msg->f_ddircetion);

		}else
		{
			msendCache->rear = temp;
			return 0;//failed!
			//��ʾ��ǰ����������Ҫ�ط��ʹ���
		}
	}
	else{
		msendCache->tag++;
#if DEBUG
		printf("push data by MSG:\n");
		for(int idebug=0;idebug<(msendCache->rear-msendCache->front);idebug++)
		{
			printf("%x  ",msendCache->SendBuf[msendCache->front+idebug]);
		}
		printf("\n and tag is : %d\n the front is %d,the rear is %d\n",msendCache->tag	,msendCache->front,msendCache->rear);
		printf("successful!\n");
#endif

		return 1;//successful
	}

}

//����ǰû���ͳɹ��������ٴη��뻺���β��
inline int SendData::SendAgain(int cacheNum,int iChanl){

	sendCache* msendCache;
	if(cacheNum == 1){
		msendCache = &UartSendCache;
	}else{
		msendCache = &SockSendCache;
	}

	int f_ds;
	int f_dd;
	int againwait;
	uchar againbuf[SOCKET_MSG_DATA_SIZE];
	int againlen;

	if(msendCache->SendBuf[(msendCache->front)+1] == iChanl)//��ʾ��ǰΪ��������
	{
		f_ds = msendCache->SendBuf[msendCache->front];
		msendCache->front = (msendCache->front + 1) % BUFFER_SIZE;
		f_dd = msendCache->SendBuf[msendCache->front];
		msendCache->front = (msendCache->front + 1) % BUFFER_SIZE;
		if(cacheNum == 1)
			againwait = msendCache->SendBuf[msendCache->front]+1;
		else
			againwait = 0;
		msendCache->front = (msendCache->front + 1) % BUFFER_SIZE;
		againlen = msendCache->SendBuf[msendCache->front];
		msendCache->front = (msendCache->front + 1) % BUFFER_SIZE;
		for(int i=0;i<againlen;i++){
			*(againbuf+i) = msendCache->SendBuf[msendCache->front];
			msendCache->front = (msendCache->front + 1) % BUFFER_SIZE;
		}
		return PushInArray(cacheNum,f_ds,f_dd,againwait,againbuf,againlen);

	}
	else{
		ChannelStu[iChanl]--;
		return 0;//����ʧ��
	}


}
//�ط�ʧ�ܣ����µ�ǰfrontָ��λ�õ�currentλ��	iChanl ��ʾ��ǰ�ȴ��ŵ���Ŀ���ŵ�
inline int SendData::UpdatePoint(int cacheNum,int iChanl){

	sendCache* tempsendCache;
	switch(cacheNum)
	{
		case 1:tempsendCache=&UartSendCache;break;

		case 2:tempsendCache=&SockSendCache;break;
		case 3:
			UartSendCache.front = UartSendCache.current;
			SockSendCache.front = SockSendCache.current;
			return 1;
		default:return 0;
	}
	if(tempsendCache->SendBuf[(tempsendCache->front)+1] == iChanl)//��ʾ��ǰΪ��������
	{
		tempsendCache->front += tempsendCache->SendBuf[(tempsendCache->front)+3]+4;
	}else{
		//ChannelStu[iChanl]--;//����ȴ�
	}

	return 1;
}

inline 	int SendData::PushOutArray(Msg* msg){
	sendCache* msendCache;
	if(UartSendCache.tag  > 0){
		msendCache = &UartSendCache;
#if DEBUG
		printf("the uartsend tag is : %d\n",msendCache->tag);
#endif
	}
	else if(SockSendCache.tag > 0)
	{
		msendCache = &SockSendCache;
#if DEBUG
		printf("the SockSend tag is : %d\n",msendCache->tag);
#endif
	}
	else
	{
		msendCache = &UartSendCache;
	}

	if(msendCache->tag > 0){
		if(msendCache->current < msendCache->rear)//��ǰ��������Ҫ����
		{
			msg->f_dsource = msendCache->SendBuf[msendCache->current];
			msendCache->current = (msendCache->current + 1) % BUFFER_SIZE;
			msg->f_ddircetion = msendCache->SendBuf[msendCache->current];
			msendCache->current = (msendCache->current + 1) % BUFFER_SIZE;
			if(msendCache->SendBuf[msendCache->current]>0)ChannelStu[msg->f_ddircetion]++;//������������Ҫ�ȴ�
			else ChannelStu[msg->f_ddircetion] = 0;
			msendCache->current = (msendCache->current + 1) % BUFFER_SIZE;
			msg->dataLen = msendCache->SendBuf[msendCache->current];
			msendCache->current = (msendCache->current + 1) % BUFFER_SIZE;
			for(int i=0;i<msg->dataLen;i++){
				msg->data[i] = msendCache->SendBuf[msendCache->current];
				msendCache->current = (msendCache->current + 1) % BUFFER_SIZE;
			}
			msendCache->tag--;
#if DEBUG
			printf("push out data:\n");
			printf("%x %x %x ",msg->f_dsource,msg->f_ddircetion,msg->dataLen);
			for(int idebug=0;idebug<msg->dataLen;idebug++)
			{
				printf("%x ",msg->data[idebug]);
			}
			printf("\n and tag is : %d\n",msendCache->tag	);
			printf("successful!\n");
#endif

			return 1;//������ ����

		}
		else if(msendCache->current >= msendCache->rear)//��ǰ�޷�������
		{
			msendCache->current = msendCache->rear;
			msendCache->tag=0;
			return 0;//û�����ݿ��Է���
		}
		return 0;
	}
	else//û��������Ҫ����
	{

		ChannelStu[0] = 0;
		int errornum = 0;
		for(int i = 0;i<ChNum;i++)
		{
			int istu = ChannelStu[i];
//#ifdef DEBUG
//			if(istu > 0x00)
//				printf("\n the Channel %d stu is %x\n",i,istu);
//#endif
			switch(istu)
			{
				case 0x0000:ChannelStu[0]++;break;//�ŵ�����Ҫ�ȴ�
				case 0x2a00:
				case 0x5400:
					if(SendAgain(1,i))
						if(UartSendCache.rear-UartSendCache.front-UartSendCache.SendBuf[3]>8)
							UpdatePoint(1,i);
//					usleep(100);
//					SendAgain(1,i);
					break;
				case 0x7f00:printf("channel %d is error\n",i);UpdatePoint(1,i);ChannelStu[i] = 0;errornum=i;break;//�ŵ�����
//#if DEBUG
//				case 0x2000:
//				case 0x2b00:
//
//				case 0x5000:
//					printf("buf contant is \n");
//					for(int bufoffset = UartSendCache.front;bufoffset<=UartSendCache.rear;bufoffset++)
//					{
//						printf(" %x",UartSendCache.SendBuf[bufoffset]);
//					}
//					printf("\n the front is %d,the rear is %d \n",UartSendCache.front,UartSendCache.rear);
//					ChannelStu[i]++;
//					break;
//#endif
				default :ChannelStu[i]++;usleep(10);break;
			}
			if(ChannelStu[0] == ChNum)
			{
				UpdatePoint(3,0);

			}
			//usleep(100);
		}
//		if(ChannelStu[25]!=0)
//			printf("the 25 channel is %d\n",ChannelStu[25]);
		return errornum;
	}
}
#endif /* SENDDATA_H_ */
