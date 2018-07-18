/**
 * File name: DevUtils.h
 * Author: JoshuaWong
 * Email: 1034316376@qq.com
 * Version: 1.0
 * Created on: 2017年7月28日 下午4:56:44
 * Description: 
 */

#ifndef UTILS_DEVUTILS_H_
#define UTILS_DEVUTILS_H_

#include "../common.h"

using namespace std;

namespace DevUtils {
inline int OpenDev(string dev, int flag);
inline int OpenDev(string dev);
inline void CloseDev(int fd);
}

namespace DevUtils {
int OpenDev(string dev, int flag) {
	int fd = open(dev.c_str(), flag);
	if (fd < 0) {
		char str[1024];
		sprintf(str, "open %s failed!", dev.c_str());
		//handle_error_en(fd, str);
	}
	return fd;
}

int OpenDev(string dev) {
	return OpenDev(dev, O_RDWR);
}

void CloseDev(int fd) {
	int ret = close(fd);
	if (ret)
		//handle_error_en(ret, G1BW1);
		printf("close %d failed!", fd);
}
}

#endif /* UTILS_DEVUTILS_H_ */
