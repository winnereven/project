/**
 * File name: config.h
 * Author: JoshuaWong
 * Email: 1034316376@qq.com
 * Version: 1.0
 * Created on: 2017��7��28�� ����12:04:08
 * Description: 
 */

#ifndef SYS_CONFIG_H_
#define SYS_CONFIG_H_

#include <termios.h>

#define VERSION "171122"

#define WATCH_DOG_EN 1
#define RESTART_MIDNIGHT 1	// ��ҹ�������ܿ�����־λ
#define CLOSE_FIBER 1//�����رռ�����ʹ��
//#define TEMP_EN 1
//#define UART_EN 1

// UART
//TODO ����������
#define UART_BAUD_RATE		B9600
#define UART_BAUD_RATE_Q	B115200
#define UART_BAUD_RATE_M	B19200
#define UART_DATA_BIT		8
#define UART_STOP_BIT		"1"
#define UART_PARITY			'N'
#define UART_PARITY_E		'E'

#define UART_VTIME			1//��200ms ���û�����ݾͷ���0
#define UART_VMIN			0
//#define UART_VMIN			255

// TODO Watch dog ʱ��
#define WATCH_DOG_TIMEOUT 60	// ���Ź���ʱ����λ��

#endif /* SYS_CONFIG_H_ */
