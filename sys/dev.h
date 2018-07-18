/**
 * File name: dev.h
 * Author: even
 * Email: 871454583@qq.com
 * Version: 1.0
 * Created on: 2017年8月23日 下午
 * Description: 
 */

#ifndef SYS_DEV_H_
#define SYS_DEV_H_

// TODO 修改引脚

#define DYK_T		"/dev/gpio-P3.26"	//电源模块温度报警
#define FSKZ		"/dev/gpio-P3.21"	//左右侧风扇控制
//该注释部分应用于epc-28x-c-l
#define G1BW1		"/dev/gpio-P2.14"	//1#光模块1板有无检测 一次
#define G1BW2		"/dev/gpio-P2.15"	//1#光模块2板有无检测
#define FPGA_RST		"/dev/gpio-P2.12"	//FPGA 重启
#define EE_WP			"/dev/gpio-P2.13"	//存储器使能
//该部分描述应用于epc-28x-c-l-t
//#define G1BW1		"/dev/gpio-P3.24"	//1#光模块1板有无检测 一次
//#define G1BW2		"/dev/gpio-P3.25"	//1#光模块2板有无检测
//#define FPGA_RST		"/dev/gpio-P2.20"	//FPGA 重启
//#define EE_WP			"/dev/gpio-P2.21"	//存储器使能


#define G2BW1		"/dev/gpio-P2.4"	//2#光模块1板有无检测
#define G2BW2		"/dev/gpio-P2.5"	//2#光模块2板有无检测

#define KJ1			"/dev/gpio-P2.6"	//1接收器功率过载检测
#define KJ2			"/dev/gpio-P2.7"	//2接收器功率过载检测
#define GMK1_T		"/dev/gpio-P1.17"	//1#光模块温度报警
#define GMK2_T		"/dev/gpio-P1.18"	//2#光模块温度报警
#define LED1		"/dev/gpio-P3.20"	//1号指示灯			0
#define LED4		"/dev/gpio-P3.27"	//2号指示灯->4号		1
#define LED3		"/dev/gpio-CLK"		//3号指示灯			2
#define LED2		"/dev/gpio-CS"		//4号指示灯->2号
#define LED5		"/dev/gpio-MISO"	//5号指示灯
#define LED6		"/dev/gpio-MOSI"	//6号指示灯


#define UART0_DEBUG		"/dev/ttySP0"		//调试串口
#define LIGHT1			"/dev/ttySP1"		//与1号光模块通讯
#define LIGHT2			"/dev/ttySP2"		//与2号光模块通讯
#define FPGA1			"/dev/ttySP3"		//与FPGA通讯
#define FPGA2			"/dev/ttySP4"		//与FPGA通讯

#define DEV_WATCHDOG			"/dev/watchdog"		//看门狗

#endif /* SYS_DEV_H_ */
