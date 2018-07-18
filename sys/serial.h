/**
 * File name: serial.h
 * Author: JoshuaWong
 * Email: 1034316376@qq.com
 * Version: 1.0
 * Created on: 2017��7��27�� ����10:10:29
 * Description: 
 */

#ifndef DEVICE_SERIAL_H_
#define DEVICE_SERIAL_H_

#include "../common.h"

// ����������
void set_baudrate(struct termios *opt, unsigned int baudrate);
// ��������λ������
void set_data_bit(struct termios *opt, unsigned int databit);
// ������żУ������
void set_parity(struct termios *opt, char parity);
// ����ֹͣλ������
void set_stopbit(struct termios *opt, const char *stopbit);
// �������Ե�����
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
	case 'N': /* ��У�� */
	case 'n':
		opt->c_cflag &= ~PARENB;
		break;
	case 'E': /* żУ�� */
	case 'e':
		opt->c_cflag |= PARENB;
		opt->c_cflag &= ~PARODD;
		break;
	case 'O': /* ��У�� */
	case 'o':
		opt->c_cflag |= PARENB;
		opt->c_cflag |= ~PARODD;
		break;
	default: /*����ѡ��Ϊ��У��*/
		opt->c_cflag &= ~PARENB;
		break;
	}
}

void set_stopbit(struct termios *opt, const char *stopbit) {
	if (0 == strcmp(stopbit, "1")) {
		opt->c_cflag &= ~CSTOPB; /* 1λֹͣλt*/
	} else if (0 == strcmp(stopbit, "1.5")) {
		opt->c_cflag &= ~CSTOPB; /*1.5λֹͣλ*/
	} else if (0 == strcmp(stopbit, "2")) {
		opt->c_cflag |= CSTOPB; /*2λֹͣλ*/
	} else {
		opt->c_cflag &= ~CSTOPB; /*1λֹͣλ*/
	}
}

int set_port_attr(int fd, int baudrate, int databit, const char *stopbit, char parity, int vtime,
		int vmin) {
	struct termios opt;
	tcgetattr(fd, &opt);
	if (tcgetattr(fd, &opt) < 0) {
		return -1;
	}

//	opt.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);//��ֹ����	����ԭʼģʽ
	opt.c_lflag &= ~(ECHO | ICANON | ECHOE | ISIG);//��ֹ����	����ԭʼģʽ
	opt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	opt.c_oflag &= ~(OPOST);//ԭʼ���
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
