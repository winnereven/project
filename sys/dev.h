/**
 * File name: dev.h
 * Author: even
 * Email: 871454583@qq.com
 * Version: 1.0
 * Created on: 2017��8��23�� ����
 * Description: 
 */

#ifndef SYS_DEV_H_
#define SYS_DEV_H_

// TODO �޸�����

#define DYK_T		"/dev/gpio-P3.26"	//��Դģ���¶ȱ���
#define FSKZ		"/dev/gpio-P3.21"	//���Ҳ���ȿ���
//��ע�Ͳ���Ӧ����epc-28x-c-l
#define G1BW1		"/dev/gpio-P2.14"	//1#��ģ��1�����޼�� һ��
#define G1BW2		"/dev/gpio-P2.15"	//1#��ģ��2�����޼��
#define FPGA_RST		"/dev/gpio-P2.12"	//FPGA ����
#define EE_WP			"/dev/gpio-P2.13"	//�洢��ʹ��
//�ò�������Ӧ����epc-28x-c-l-t
//#define G1BW1		"/dev/gpio-P3.24"	//1#��ģ��1�����޼�� һ��
//#define G1BW2		"/dev/gpio-P3.25"	//1#��ģ��2�����޼��
//#define FPGA_RST		"/dev/gpio-P2.20"	//FPGA ����
//#define EE_WP			"/dev/gpio-P2.21"	//�洢��ʹ��


#define G2BW1		"/dev/gpio-P2.4"	//2#��ģ��1�����޼��
#define G2BW2		"/dev/gpio-P2.5"	//2#��ģ��2�����޼��

#define KJ1			"/dev/gpio-P2.6"	//1���������ʹ��ؼ��
#define KJ2			"/dev/gpio-P2.7"	//2���������ʹ��ؼ��
#define GMK1_T		"/dev/gpio-P1.17"	//1#��ģ���¶ȱ���
#define GMK2_T		"/dev/gpio-P1.18"	//2#��ģ���¶ȱ���
#define LED1		"/dev/gpio-P3.20"	//1��ָʾ��			0
#define LED4		"/dev/gpio-P3.27"	//2��ָʾ��->4��		1
#define LED3		"/dev/gpio-CLK"		//3��ָʾ��			2
#define LED2		"/dev/gpio-CS"		//4��ָʾ��->2��
#define LED5		"/dev/gpio-MISO"	//5��ָʾ��
#define LED6		"/dev/gpio-MOSI"	//6��ָʾ��


#define UART0_DEBUG		"/dev/ttySP0"		//���Դ���
#define LIGHT1			"/dev/ttySP1"		//��1�Ź�ģ��ͨѶ
#define LIGHT2			"/dev/ttySP2"		//��2�Ź�ģ��ͨѶ
#define FPGA1			"/dev/ttySP3"		//��FPGAͨѶ
#define FPGA2			"/dev/ttySP4"		//��FPGAͨѶ

#define DEV_WATCHDOG			"/dev/watchdog"		//���Ź�

#endif /* SYS_DEV_H_ */
