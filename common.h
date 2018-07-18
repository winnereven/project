/**
 * File name: common.h
 * Author: JoshuaWong
 * Email: 1034316376@qq.com
 * Version: 1.0
 * Created on: 2016��11��29�� ����5:22:11
 * Description: 
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <exception>
#include <fstream>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <vector>
#include <pthread.h>
#include <limits.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
//#include <sstream>
#include "sys/config.h"
#include "sys/dev.h"
#include "sys/iic.h"


using namespace std;
#define uchar unsigned char

/******************************************************************************
 *                                Socket                                      *
 ******************************************************************************/
#define MSG_HEAD		0x2424
#define MSG_END			0x0D
#define MSG_HEAD_SIZE	2
#define MSG_CMD_SIZE	1
#define MSG_CMD_LEN		1
//�м仹���ϱ߳��ȸ��ֽ�
#define MSG_CRC_SIZE	1	//У��λ����ʱΪ��
#define MSG_END_SIZE	1

#define SOCKET_MSG_FORMAT_SIZE (MSG_HEAD_SIZE + MSG_CMD_SIZE + MSG_CMD_LEN + MSG_CRC_SIZE + MSG_END_SIZE)

#define BUFFER_SIZE 			1024
#define MSG_CACHE_SIZE			512
#define SOCKET_MSG_DATA_SIZE	128
#define SOCKET_MSG_SIZE		(SOCKET_MSG_DATA_SIZE + SOCKET_MSG_FORMAT_SIZE)

#define SOCKET_PORT 		5000       		// The port which is communicate with server
#define BACKLOG 			10
#define SOCKET_BUFFER_LEN 	512              		// Buffer length
//ʱ������
time_t now;
struct tm *timenow;
int count = 0;

// ���Ҳ���ö�٣����ڲ���ʱ�жϲ���֡�ṹ���ĸ���λ
typedef enum {
	SEARCH_HEAD, SEARCH_CMD, SEARCH_LEN, SEARCH_END
} SearchStrategy;
//TODO �޸Ĵ���˳��
#define UARTNUM 5
typedef enum{
	UADEBUG, UALIGHT1, UALIGHT2, UAFPGA1, UAFPGA2
//	UADEBUG, UAFPGA1, UAFPGA2, UALIGHT1, UALIGHT2
  //���Դ���  ��ģ��1		��ģ��2		FPGA1	FPGA2
} UartNum;
/* ������깩���ش�����ĺ���ʹ��*/
#define handle_error_en(en, msg) do{errno=en; perror(msg); exit(EXIT_FAILURE);} while(0)
/* �������*/
#define handle_error(msg) do{perror(msg); exit(EXIT_FAILURE);} while(0)
#define handle_err_log(x, args...) do{fprintf(stderr, x"\n", ##args); exit(EXIT_FAILURE);}while(0)
#define log(x, args...) do{printf("file:%s function:%s line:%d "x"\n", __FILE__, __FUNCTION__, __LINE__, args);}while(0)

#define TICK_DELAY_US 50*1000	// ���̼߳��
// DELAY
#define DATA_PARSE_DELAY_US TICK_DELAY_US		// Socket Server���ݽ������



#endif /* COMMON_H_ */
