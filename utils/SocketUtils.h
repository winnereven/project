/**
 * File name: SocketUtils.h
 * Author: JoshuaWong
 * Email: 1034316376@qq.com
 * Version: 1.0
 * Created on: Jun 28, 2017 10:39:05 AM
 * Description: 
 */

#ifndef UTILS_SOCKETUTILS_H_
#define UTILS_SOCKETUTILS_H_

#include "../common.h"

namespace SocketUtils {
// 下行指令
inline int GetDownstreamDataLen(uint8_t cmd);
// 上行指令
inline int GetUpstreamDataLen(uint8_t cmd);
}

namespace SocketUtils {
int GetDownstreamDataLen(uint8_t cmd) {
	int len;
	switch (cmd) {
//TODO 修改指令长度
//	case 0x00:		len = 0;		break;
	case 0x01:		len = 4;		break;
	case 0x02:		len = 8;		break;
	case 0x03:		len = 4;		break;
	case 0x04:		len = 8;		break;
	case 0x05:		len = 7;		break;
	case 0x06:		len = 1;		break;
	case 0x07:		len = 14;		break;
	case 0x08:		len = 1;		break;
//	case 0x09:		len = 2;		break;
//	case 0x0a:		len = 1;		break;
//	case 0x0b:		len = 8;		break;
//	case 0x0c:		len = 8;		break;
//	case 0x0d:		len = 8;		break;
	case 0x0e:		len = 1;		break;
	case 0x0f:		len = 1;		break;
//	case 0x10:		len = 8;		break;
//	case 0x11:		len = 1;		break;
	case 0x12:		len = 54;		break;
	case 0x13:		len = 54;		break;
	case 0x14:		len = 54;		break;
	case 0x15:		len = 14;		break;
	case 0x16:		len = 14;		break;
//	case 0x17:		len = 2;		break;

	case 0x1f:		len = 1;		break;

	case 0xfe:		len = 2;		break;

	case 0xaa:		len = 1;		break;
	default:		len = -1;		break;
	}
	return len;
}

int GetUpstreamDataLen(uint8_t cmd) {
	int len;
	switch (cmd) {
	case 0x11:		len = 3;		break;
	case 0x12:		len = 3;		break;
	case 0x13:		len = 3;		break;
	case 0x14:		len = 3;		break;
//	case 0x05:		len = 38;		break;
//	case 0x06:		len = 0;		break;
//	case 0x07:		len = 1;		break;
//	case 0xff:		len = 0;		break;
	default:		len = -1;		break;
	}
	return len;
	}
}

#endif /* UTILS_SOCKETUTILS_H_ */
