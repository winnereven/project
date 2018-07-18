/**
 * File name: UartServer.h
 * Author: Even
 * Email: 871454583@qq.com
 * Version: 1.0
 * Created on: 2017年8月31日
 * Description: UART数据收发
 */

#ifndef UARTSERVER_H_
#define UARTSERVER_H_

#include "../common.h"
#include "../sys/serial.h"
#include "../utils/DevUtils.h"
//#include "../socket/SocketServer.h"
class UartServer {
public:
	UartServer(int* fdUartList);
	~UartServer();
	int DataSorting(int fd, Msg *msg, void *args);//fd 来源信道，Msg-> f_ddirection 目标信道，Msg->data 数据内容
	void GetUartData(int fd , uchar* buf,int* lenofdata);
	int Uart0Send(uint8_t* buf, int len);
	int UartSendByfd(int fd,uchar* buf,int len);
	void SendMsg(Msg *msg);

	int m_fdDEBUG,m_fdLIGHT1,m_fdLIGHT2,m_fdFPGA1,m_fdFPGA2;
	//	int m_bUartEn;

private:
	uint8_t mRecvBuf[BUFFER_SIZE];
	// 串口初始化
	void __InitUART();
	void __UpstreamProcessor(Msg *msg, void *args);
};

inline UartServer::UartServer(int* fdUartList){

	int ret;
	m_fdDEBUG = DevUtils::OpenDev(UART0_DEBUG, O_RDWR | O_NOCTTY);
	ret = set_port_attr(m_fdDEBUG, UART_BAUD_RATE, UART_DATA_BIT, UART_STOP_BIT, UART_PARITY,
			UART_VTIME, UART_VMIN);
	if (ret < 0) {
		printf("set uart0 attr failed \n");
		exit(-1);
	}
	m_fdLIGHT1 = DevUtils::OpenDev(LIGHT1, O_RDWR | O_NOCTTY);
	ret = set_port_attr(m_fdLIGHT1, UART_BAUD_RATE, UART_DATA_BIT, UART_STOP_BIT, UART_PARITY,
			UART_VTIME, UART_VMIN);
	if (ret < 0) {
		printf("set uart1 attr failed \n");
		exit(-1);
	}
	m_fdLIGHT2 = DevUtils::OpenDev(LIGHT2, O_RDWR | O_NOCTTY);
	ret = set_port_attr(m_fdLIGHT2, UART_BAUD_RATE, UART_DATA_BIT, UART_STOP_BIT, UART_PARITY,
			UART_VTIME, UART_VMIN);
	if (ret < 0) {
		printf("set uart2 attr failed \n");
		exit(-1);
	}
	//TODO 更换FPGA波特率
	m_fdFPGA1 = DevUtils::OpenDev(FPGA1, O_RDWR | O_NOCTTY);
	ret = set_port_attr(m_fdFPGA1, UART_BAUD_RATE, UART_DATA_BIT, UART_STOP_BIT, UART_PARITY_E,
			UART_VTIME, UART_VMIN);
	if (ret < 0) {
		printf("set uart3 attr failed \n");
		exit(-1);
	}
	m_fdFPGA2 = DevUtils::OpenDev(FPGA2, O_RDWR | O_NOCTTY);
	ret = set_port_attr(m_fdFPGA2, UART_BAUD_RATE, UART_DATA_BIT, UART_STOP_BIT, UART_PARITY_E,
			UART_VTIME, UART_VMIN);
	if (ret < 0) {
		printf("set uart4 attr failed \n");
		exit(-1);
	}

	//TODO  配置为非阻塞模式
	if(fcntl(m_fdDEBUG,F_SETFL,FNDELAY) < 0)
		printf("fcntl m_fdDEBUG failed\n");

	if(fcntl(m_fdLIGHT1,F_SETFL,FNDELAY) < 0)
		printf("fcntl m_fdLIGHT1 failed\n");

	if(fcntl(m_fdLIGHT2,F_SETFL,FNDELAY) < 0)
		printf("fcntl m_fdLIGHT2 failed\n");

	if(fcntl(m_fdFPGA1,F_SETFL,FNDELAY) < 0)
		printf("fcntl m_fdFPGA1 failed\n");

	if(fcntl(m_fdFPGA2,F_SETFL,FNDELAY) < 0)
		printf("fcntl m_fdFPGA2 failed\n");

	fdUartList[UADEBUG] = m_fdDEBUG;
	fdUartList[UALIGHT1] = m_fdLIGHT1;
	fdUartList[UALIGHT2] = m_fdLIGHT2;
	fdUartList[UAFPGA1] = m_fdFPGA1;
	fdUartList[UAFPGA2] = m_fdFPGA2;


	printf("UartServer Object created!\n");
#if DEBUG
	printf("fdUartList is :");
	for(int ifduart=0;ifduart<UARTNUM;ifduart++)
	{
		printf("%d",fdUartList[ifduart]);
	}
	printf("\n");
#endif
}

inline UartServer::~UartServer(){
	DevUtils::CloseDev(m_fdFPGA2);
	DevUtils::CloseDev(m_fdFPGA1);
	DevUtils::CloseDev(m_fdLIGHT2);
	DevUtils::CloseDev(m_fdLIGHT1);
	DevUtils::CloseDev(m_fdDEBUG);
	printf("UartServer Object destroyed!\n");
}
inline int UartServer::Uart0Send(uint8_t* buf, int len) {
	if(m_fdDEBUG>0)
		return write(m_fdDEBUG, buf, len);
	else
		return -1;
}
inline int UartServer::UartSendByfd(int fd,uchar* buf,int len){
	if(fd>0)
		return write(fd, buf, len);
	else
		return -1;
}
inline void UartServer::GetUartData(int fd, uchar* buf,int* lenofdata) {

		*lenofdata = read(fd,buf, BUFFER_SIZE);
		if(*lenofdata > 0)
		{/*
			if(*buf==0x24&&*(buf+1)==0x24)
			{
				if(buf[(*lenofdata)-1]==0x0d){
					printf("read successful\n");
				}
				else{
					for(int idebug=0;idebug<*lenofdata;idebug++)
					{
						printf("%x  ",*(buf+idebug));
					}
					printf("\n");

					int lenofother = read(fd,buf+(*lenofdata),BUFFER_SIZE-(*lenofdata));
					printf("lenofother is %d\n",lenofother);
					*lenofdata += lenofother;
					for(int idebug=0;idebug<*lenofdata;idebug++)
					{
						printf("%x  ",*(buf+idebug));
					}
					printf("\n and buf[(*lenofdata)-1] is %x",buf[(*lenofdata)-1]);
						if(buf[(*lenofdata)-1]==0x0d){
							printf("read successful\n");
						}
						else{
							*lenofdata = 0;
							printf("read date end is empty!\n");
						}
				}
			}
			else{
				*lenofdata = 0;
				printf("read date head is empty!\n");
				printf("buf[0] is %x,buf[1] is %x \n",*buf,*(buf+1));
			}*/
		}

}
//数据处理函数，返回为0表示失败，返回为1表示发送给光模块，返回为2表示发送给单个FPGA，返回3表示发给多个FPGA，返回4表示发给计算机 返回5表示心跳
//fd表示来源信道
inline int UartServer::DataSorting(int fd, Msg *msg, void *args) {
#if DEBUG
	printf("Parse 1 instruction.cmd:%x dataLen:%d isRecv:%d", msg->cmd, msg->dataLen, msg->isRecv);
#endif
	msg->f_dsource = fd;
	msg->iswait = 0;
	switch (msg->cmd) {

		case 0xaa://心跳
			msg->f_ddircetion=m_fdDEBUG;return 5;
		case 0x01:msg->iswait = 1;
		case 0x02:msg->f_ddircetion=m_fdLIGHT1 ;return 1;
		case 0x03:msg->iswait = 1;
		case 0x04:msg->f_ddircetion=m_fdLIGHT2 ;return 1;

		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
//		case 0x09:
//		case 0x0a:
//TODO 修改指令码功能
			switch(msg->data[0]){
				case 0x01:msg->f_ddircetion=m_fdFPGA1;msg->iswait=1;return 2;
				case 0x02:msg->f_ddircetion=m_fdFPGA2;msg->iswait=1;return 2;
				case 0x05:msg->f_ddircetion=m_fdFPGA1;msg->iswait=1;return 3;
				default:return 0;
			}
			break;

//		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
			if(fd==m_fdFPGA1)//根据来源信道填充FPGA号
				msg->buf[4]=0x01;
			if(fd==m_fdFPGA2)
				msg->buf[4]=0x02;

		case 0x15:
		case 0x16:
//		case 0x17:
			msg->f_ddircetion=m_fdDEBUG ;return 4;
		case 0x0f:
//		case 0x1f:
			msg->f_ddircetion=m_fdDEBUG;return 6;

		case 0x0e:
			msg->f_ddircetion=m_fdDEBUG;return 7;

		default :msg->f_ddircetion = 0;return 0;
	}

}

inline void UartServer::SendMsg(Msg *msg) {
	__UpstreamProcessor(msg, NULL);
}



inline void UartServer::__UpstreamProcessor(Msg *msg, void *args) {
	uint8_t sdbuf[SOCKET_MSG_DATA_SIZE];
	int len = msg->CopyToBuf(sdbuf, 0);
	int ret;
	printf("upstream:");
	for (int i = 0; i < len; i++) {
		printf("%x ", sdbuf[i]);
	}
	printf("\n");
	if ((ret = Uart0Send(sdbuf,len)) < 0) {
		printf("ERROR: Failed  to sent string.\n");
	}
//	if (m_bUartEn)
//		__UartSend(sdbuf, len);
//	printf("OK: Sent %d bytes successful, please enter again.\n", ret);
}

#endif
