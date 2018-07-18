/**
 * File name: config.h
 * Author: JoshuaWong
 * Email: 1034316376@qq.com
 * Version: 1.0
 * Created on: 2017年7月28日 下午12:04:08
 * Description: 
 */

#ifndef SYS_CONFIG_H_
#define SYS_CONFIG_H_

#include <termios.h>

#define VERSION "171122"

#define WATCH_DOG_EN 1
#define RESTART_MIDNIGHT 1	// 半夜重启功能开启标志位
#define CLOSE_FIBER 1//开机关闭激光器使能
//#define TEMP_EN 1
//#define UART_EN 1

// UART
//TODO 更换波特率
#define UART_BAUD_RATE		B9600
#define UART_BAUD_RATE_Q	B115200
#define UART_BAUD_RATE_M	B19200
#define UART_DATA_BIT		8
#define UART_STOP_BIT		"1"
#define UART_PARITY			'N'
#define UART_PARITY_E		'E'

#define UART_VTIME			1//等200ms 如果没有数据就返回0
#define UART_VMIN			0
//#define UART_VMIN			255

// TODO Watch dog 时间
#define WATCH_DOG_TIMEOUT 60	// 看门狗超时，单位秒

#endif /* SYS_CONFIG_H_ */
