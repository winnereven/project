/**
 * File name: SendData.h
 * Author: even
 * Email: 871454583@qq.com
 * Version: 1.0
 * Created on: Sep 06, 2017
 * Description:信道定义
 * bit0		bit1		bit2		bit3		bit4~bitn
 * 来源信道	目标信道		是否回复		数据长度		数据
 */

#ifndef SENDDATA_H_
#define SENDDATA_H_

#include "../common.h"

struct sendCache{
	int front;//前指针
	int rear;//后指针
	int current;//当前位置
	int len;//长度
	int tag;//完成标志
	uchar SendBuf[BUFFER_SIZE];//f_dsocket > f_duart

};

class SendData {
public:
	SendData();
	~SendData();
	int PushInArray(int cacheNum,int f_dsource,int f_ddirection,int waitflag,uchar* buf,int len);//将数据入栈
	int PushInByMsgData(int cacheNum,Msg* msg);
	int PushInByMsgAll(int cacheNum,Msg* msg);//
	int SendAgain(int cacheNum,int iChanl);//再次发送数据
	int PushOutArray(Msg* msg);//将数据出栈
	int UpdatePoint(int cacheNum,int iChanl);
private:

	sendCache UartSendCache;
	sendCache SockSendCache;
//	int front;//前指针
//	int rear;//后指针
//	int current;//当前位置
//	int len;//长度
//	int tag;//完成标志
	const static uchar ChNum = 30;
	int ChannelStu[ChNum];
//	uchar SendBufUart[BUFFER_SIZE];//f_dsocket > f_duart
};

inline SendData::SendData(){

}
inline SendData::~SendData(){

}
//发送队列存储方式0.缓存区（1表示串口 2 表示网口） 1.来源信道 2.目标信道 3.等待标志 4.数据长度 5.内容
inline int SendData::PushInArray(int cacheNum,int f_dsource,int f_ddirection,int waitflag,uchar* buf,int len){
	sendCache* msendCache;
	if(cacheNum == 1){
		msendCache = &UartSendCache;
	}
	else
	{
		msendCache = &SockSendCache;
	}
	int temp = msendCache->rear;//保存当前指针
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
			//表示没有需要处理的数据，可以清空改帧
			msendCache->current = msendCache->rear;
			return SendAgain(cacheNum,f_ddirection);

		}else
		{
			msendCache->rear = temp;
			return 0;//failed!
			//表示当前还有数据需要重发送处理
		}
	}
	else{
		msendCache->tag++;
		return 1;//successful
	}
}
//将Msg数据传入发送缓存--发送给光模块     cacheNum缓存区（1表示串口 2 表示网口）
inline int SendData::PushInByMsgData(int cacheNum,Msg* msg){
	sendCache* msendCache;
	if(cacheNum == 1){
		msendCache = &UartSendCache;
	}
	else
	{
		msendCache = &SockSendCache;
	}
	int temp = msendCache->rear;//保存当前指针
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
			//表示没有需要处理的数据，可以清空改帧
			msendCache->current = msendCache->rear;
			return SendAgain(cacheNum,msg->f_ddircetion);

		}else
		{
			msendCache->rear = temp;
			return 0;//failed!
			//表示当前还有数据需要重发送处理
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
//将Msg->buf的数据全部传入发送缓存--发送给计算机或者FPGA   cacheNum缓存区（1表示串口 2 表示网口）
inline int SendData::PushInByMsgAll(int cacheNum,Msg* msg){
	sendCache* msendCache;
	if(cacheNum == 1){
		msendCache = &UartSendCache;
	}
	else
	{
		msendCache = &SockSendCache;
	}
	int temp = msendCache->rear;//保存当前指针
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
			//表示没有需要处理的数据，可以清空改帧
			msendCache->current = msendCache->rear;
			return SendAgain(cacheNum,msg->f_ddircetion);

		}else
		{
			msendCache->rear = temp;
			return 0;//failed!
			//表示当前还有数据需要重发送处理
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

//将当前没发送成功的数据再次放入缓存的尾巴
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

	if(msendCache->SendBuf[(msendCache->front)+1] == iChanl)//表示当前为所发数据
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
		return 0;//返回失败
	}


}
//重发失败，更新当前front指针位置到current位置	iChanl 表示当前等待信道即目标信道
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
	if(tempsendCache->SendBuf[(tempsendCache->front)+1] == iChanl)//表示当前为所发数据
	{
		tempsendCache->front += tempsendCache->SendBuf[(tempsendCache->front)+3]+4;
	}else{
		//ChannelStu[iChanl]--;//否则等待
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
		if(msendCache->current < msendCache->rear)//当前有数据需要发送
		{
			msg->f_dsource = msendCache->SendBuf[msendCache->current];
			msendCache->current = (msendCache->current + 1) % BUFFER_SIZE;
			msg->f_ddircetion = msendCache->SendBuf[msendCache->current];
			msendCache->current = (msendCache->current + 1) % BUFFER_SIZE;
			if(msendCache->SendBuf[msendCache->current]>0)ChannelStu[msg->f_ddircetion]++;//发出的数据需要等待
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

			return 1;//有数据 发送

		}
		else if(msendCache->current >= msendCache->rear)//当前无发送数据
		{
			msendCache->current = msendCache->rear;
			msendCache->tag=0;
			return 0;//没有数据可以法送
		}
		return 0;
	}
	else//没有数据需要法送
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
				case 0x0000:ChannelStu[0]++;break;//信道不需要等待
				case 0x2a00:
				case 0x5400:
					if(SendAgain(1,i))
						if(UartSendCache.rear-UartSendCache.front-UartSendCache.SendBuf[3]>8)
							UpdatePoint(1,i);
//					usleep(100);
//					SendAgain(1,i);
					break;
				case 0x7f00:printf("channel %d is error\n",i);UpdatePoint(1,i);ChannelStu[i] = 0;errornum=i;break;//信道故障
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
