/**
 * File name: serial.h
 * Author: JoshuaWong
 * Email: 1034316376@qq.com
 * Version: 1.0
 * Created on: 2017年7月27日 下午10:10:29
 * Description: 
 */

#ifndef DEVICE_SERIAL_H_
#define DEVICE_SERIAL_H_

#include "../common.h"

// 波特率设置
void set_baudrate(struct termios *opt, unsigned int baudrate);
// 串口数据位的设置
void set_data_bit(struct termios *opt, unsigned int databit);
// 串口奇偶校验设置
void set_parity(struct termios *opt, char parity);
// 串口停止位的设置
void set_stopbit(struct termios *opt, const char *stopbit);
// 串口属性的设置
int set_port_attr(int fd, int baudrate, int databit, const char *stopbit, char parity, int vtime,
		int vmin);
void set_baudrate(struct termios *opt, unsigned int baudrate) {
	cfsetispeed(opt, baudrate);
	cfsetospeed(opt, baudrate);
}

void set_data_bit(struct termios *opt, unsigned int databit) {
	opt->c_cflag &= ~CSIZE;
	switch (databit) {
	case 8:
		opt->c_cflag |= CS8;
		break;
	case 7:
		opt->c_cflag |= CS7;
		break;
	case 6:
		opt->c_cflag |= CS6;
		break;
	case 5:
		opt->c_cflag |= CS5;
		break;
	default:
		opt->c_cflag |= CS8;
		break;
	}
}

void set_parity(struct termios *opt, char parity) {
	switch (parity) {
	case 'N': /* 无校验 */
	case 'n':
		opt->c_cflag &= ~PARENB;
		break;
	case 'E': /* 偶校验 */
	case 'e':
		opt->c_cflag |= PARENB;
		opt->c_cflag &= ~PARODD;
		break;
	case 'O': /* 奇校验 */
	case 'o':
		opt->c_cflag |= PARENB;
		opt->c_cflag |= ~PARODD;
		break;
	default: /*其它选择为无校验*/
		opt->c_cflag &= ~PARENB;
		break;
	}
}

void set_stopbit(struct termios *opt, const char *stopbit) {
	if (0 == strcmp(stopbit, "1")) {
		opt->c_cflag &= ~CSTOPB; /* 1位停止位t*/
	} else if (0 == strcmp(stopbit, "1.5")) {
		opt->c_cflag &= ~CSTOPB; /*1.5位停止位*/
	} else if (0 == strcmp(stopbit, "2")) {
		opt->c_cflag |= CSTOPB; /*2位停止位*/
	} else {
		opt->c_cflag &= ~CSTOPB; /*1位停止位*/
	}
}

int set_port_attr(int fd, int baudrate, int databit, const char *stopbit, char parity, int vtime,
		int vmin) {
	struct termios opt;
	tcgetattr(fd, &opt);
	if (tcgetattr(fd, &opt) < 0) {
		return -1;
	}

//	opt.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);//禁止回送	启用原始模式
	opt.c_lflag &= ~(ECHO | ICANON | ECHOE | ISIG);//禁止回送	启用原始模式
	opt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	opt.c_oflag &= ~(OPOST);//原始输出
	opt.c_cc[VTIME] = vtime;
	opt.c_cc[VMIN] = vmin;

	set_baudrate(&opt, baudrate);
	set_data_bit(&opt, databit);
	set_parity(&opt, parity);
	set_stopbit(&opt, stopbit);

	tcflush(fd, TCIFLUSH);
	return (tcsetattr(fd, TCSANOW, &opt));

//
//	opt.c_cflag |= CLOCAL | CREAD; /* | CRTSCTS */
//	opt.c_oflag = 0;
//	opt.c_lflag |= 0;
//	opt.c_oflag &= ~OPOST;
}

#endif /* DEVICE_SERIAL_H_ */
