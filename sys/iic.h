/**
 * File name: iic.h
 * Author: even
 * Email: 871454583@qq.com
 * Version: 1.0
 * Created on: 2017��9��11��
 * Description:
 * 20�� 1400
 * 25�� 1900
 * 30�� 1E00
 * 35�� 2300
 * 45�� 2D00
 */
#ifndef __IIC_H
#define __IIC_H
#include <sys/ioctl.h>
#define LM75 		0x90
#define I2C_SLAVE	0x0703
#define I2C_TENBIT	0x0704

unsigned char Tos[3]={0x03,0x2D,0x00};//����
unsigned char Thyst[3]={0x02,0x23,0x00};//����
unsigned char Temp[3]={0x00,0x00,0x00};//�¶Ȼض�

int  InitIIC(){
	int GiFd = open("/dev/i2c-1", O_RDWR);
		if (GiFd == -1){
			perror("open serial 0\n");
			close(GiFd);
			return -1;
		}
  	int uiRet = ioctl(GiFd, I2C_SLAVE, LM75 >> 1);//���ôӻ���ַ
    	if (uiRet < 0) {
		printf("setenv address faile ret: %x \n", uiRet);
		close(GiFd);
		return -1;
     	}
	uiRet = ioctl(GiFd, I2C_TENBIT, 0);//���õ�ַΪ8bit
		if (uiRet < 0) {
		printf("setenv 8bit address faile ret: %x \n", uiRet);
		close(GiFd);
		return -1;
		}
	uiRet = write(GiFd, &Tos[0], 3);//���ø���
		if (uiRet < 3) {
		printf("setenv high temp faile ret: %x \n", uiRet);
		close(GiFd);
		return -1;
		}
	uiRet = write(GiFd, &Thyst[0], 3);//���õ���
		if (uiRet < 3) {
		printf("setenv high temp faile ret: %x \n", uiRet);
		close(GiFd);
		return -1;
		}
		close(GiFd);
	return 1;

}
float IICReadTemp(){
	int GiFd = open("/dev/i2c-1", O_RDWR);
		if (GiFd == -1){
			perror("open serial 0\n");
			close(GiFd);
			return -1;
		}
  	int uiRet = ioctl(GiFd, I2C_SLAVE, LM75 >> 1);//���ôӻ���ַ
    	if (uiRet < 0) {
		printf("setenv address faile ret: %x \n", uiRet);
		close(GiFd);
		return -1;
     	}
	uiRet = ioctl(GiFd, I2C_TENBIT, 0);//���õ�ַΪ8bit
		if (uiRet < 0) {
		printf("setenv 8bit address faile ret: %x \n", uiRet);
		close(GiFd);
		return -1;
		}
	uiRet = write(GiFd, &Temp[0], 1);//д���¶ȵ�ַ
		if (uiRet < 1) {
		printf("setenv temp addre faile ret: %x \n", uiRet);
		close(GiFd);
		return -1;
		}
	uiRet = read(GiFd, &Temp[1], 2);
		if (uiRet < 2) {
		printf("read temp faile ret: %x \n", uiRet);
		close(GiFd);
		return -1;
		}
		close(GiFd);
	float temp = Temp[2]>>5;
	temp = temp + Temp[1]*8;
	temp = temp * 0.125;
	return temp;


}
#endif



