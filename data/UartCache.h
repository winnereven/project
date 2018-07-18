/**
 * File name: UartCache.h
 * Author: even
 * Email: 871454583@qq.com
 * Version: 1.0
 * Created on: 2017-10-11
 * Description:
 */

#ifndef UARTCACHE_H_
#define UARTCACHE_H_

#include "../common.h"
#include "../utils/SocketUtils.h"
#include "Msg.h"

class UartCache {
public:

	UartCache();
	virtual ~UartCache();
//	bool MsgPreParse(int fd, uint8_t *buf, int len, void *args, Msg* msg);	// 通信数据解析
	bool UartRevArray(uint8_t iarray,uint8_t *buf,int len,void *args, Msg* msg);
private:
	uint8_t mbuf[5][MSG_CACHE_SIZE];
	int mfront[5];
	int mrear[5];
	int mcurrent[5];
	int mlen[5];
	int mtag[5];
	SearchStrategy strategy[5];
	void *args[5];

	int __MsgCopyIn(uint8_t iarray,uint8_t *buf, int len, Msg* msg);	// 将数据添加入缓存
	int __MsgCopyOut(uint8_t iarray ,uint8_t *buf, int start_index, int len);	//将数据移出缓存
	int __MsgParse(uint8_t iarray ,Msg* msg);		// 通信数据解析
};

inline UartCache::UartCache()
{
//	mfront[5]={0,0,0,0,0};
//	mrear[5]={0,0,0,0,0};
//	mcurrent[5]={0};
//	mlen[5]={0};
//	mtag[5]={0};
//	strategy[5]={0};
//	args[5] = NULL;
	printf("UartCache object created!\n");
}

inline UartCache::~UartCache() {
	printf("UartCache object destroyed!\n");
}

inline int UartCache::__MsgCopyIn(uint8_t iarray,uint8_t* buf, int len, Msg* msg) {
	int left_len;	// 剩余长度
	int copy_len;
	int count;
	uint8_t* src = buf;

	if (this->mtag[iarray] == 1 && this->mfront[iarray] == this->mrear[iarray]) {
#if DEBUG
		printf("uart cache is full! tag is %d \n" , this->mtag[iarray]);
#endif
		this->mtag[iarray] = 0;
		this->strategy[iarray] = SEARCH_HEAD;
		return 0;
	}

	left_len = MSG_CACHE_SIZE - this->mlen[iarray];
	copy_len = len > left_len ? left_len : len;
	count = copy_len;

	if(count == 0) return 0;
	while (count--) {
		*(this->mbuf[iarray] + this->mrear[iarray]) = *src;	// 入队
		src++;
		this->mrear[iarray] = (this->mrear[iarray] + 1) % MSG_CACHE_SIZE;
		this->mlen[iarray]++;//收到的指令的总长度
		this->mtag[iarray] = 1;
	}

	return copy_len;
}

inline int UartCache::__MsgCopyOut(uint8_t iarray ,uint8_t *buf, int start_index, int len) {
	uint8_t *dest;
	int src;
	int count;
	if (this->mbuf[iarray] == NULL || buf == NULL || len == 0) {
		return 0;
	}
	if (this->mfront[iarray] == this->mrear[iarray] && this->mtag[iarray] == 0) {
#if DEBUG
		printf("uart  cache is empty!");
#endif
		return 0;
	}
	if (this->mrear[iarray] > this->mfront[iarray]) {	// 如果循环队列不为空
		if (start_index < this->mfront[iarray] || start_index > this->mrear[iarray]) {
#if DEBUG
			printf("invalid start index!");
#endif
			return 0;
		}
	} else if (start_index > this->mrear[iarray] && start_index < this->mfront[iarray]) {
#if DEBUG
		printf("invalid start index!");
#endif
		return 0;
	}

	src = start_index;
	dest = buf;
	count = len;
	while (count--) {
		*dest = *(this->mbuf[iarray] + src);
		dest++;
		src = (src + 1) % MSG_CACHE_SIZE;
	}
	return len;
}

inline int UartCache::__MsgParse(uint8_t iarray, Msg* msg) {
	int currLen;
	int p, q;
	int i;
	bool find;
	int recvDataLen;
	//Msg* msg = pmsg;

	if (this->mfront[iarray] == this->mrear[iarray] && this->mtag[iarray] == 0) { // Cache为空
		return 0;
	}

	if (this->mcurrent[iarray] >= this->mfront[iarray]) {
		currLen = this->mlen[iarray] - (this->mcurrent[iarray] - this->mfront[iarray]);
	} else {
		currLen = this->mrear[iarray] - this->mcurrent[iarray];
	}

#if DEBUG
	int ioffset;
	printf("strategy is : %x. data is \n",this->strategy[iarray]);
	for(ioffset=0;ioffset<currLen;ioffset++)
	{
		printf("%x  ",this->mbuf[iarray][this->mcurrent[iarray] + ioffset]);
	}
#endif
	switch (this->strategy[iarray]) {
	case SEARCH_HEAD:
		if (currLen < MSG_HEAD_SIZE) {
//			this->mcurrent[iarray] = (this->mcurrent[iarray] + currLen) % MSG_CACHE_SIZE;//更新到最新位置
			this->mcurrent[iarray] = this->mfront[iarray];
			return 0;
		}
		find = false;
		for (i = 0; i < currLen - 1; i++) {
			p = this->mcurrent[iarray];
			q = (this->mcurrent[iarray] + 1) % MSG_CACHE_SIZE;
			if ((this->mbuf[iarray][p] == (MSG_HEAD >> 8)) && (this->mbuf[iarray][q] == (MSG_HEAD & 0xff))) {
				msg->isRecv = true;
				find = true;
				break; // exit for loop
			} else {
				// current pointer move to next
				this->mcurrent[iarray] = q;
				// delete one item
				this->mfront[iarray] = this->mcurrent[iarray];
				this->mlen[iarray]--;
				this->mtag[iarray] = 0;
			}
		}

		if (find == true) {
			//move 2 items towards next
			this->mcurrent[iarray] = (this->mcurrent[iarray] + 2) % MSG_CACHE_SIZE;
			//we found the head format, go on to find Type byte
			this->strategy[iarray] = SEARCH_CMD;
		} else {
			//if there is no head format ,delete previous items
			printf("uart message without head: %x!\n", MSG_HEAD);
			//go on to find Head format
			this->mcurrent[iarray] = (this->mcurrent[iarray] + 1) % MSG_CACHE_SIZE;//更新到最新位置
			this->mfront[iarray] = this->mcurrent[iarray];
			this->strategy[iarray] = SEARCH_HEAD;
		}
		break;

	case SEARCH_CMD: //to find the type direction in cache
		if (currLen < MSG_CMD_SIZE) {
//			this->mcurrent[iarray] = (this->mcurrent[iarray] + currLen) % MSG_CACHE_SIZE;//更新到最新位置
			this->mcurrent[iarray] = this->mfront[iarray];
			this->strategy[iarray] = SEARCH_HEAD;
			return 0;
		}
		//get the value of type
		msg->cmd = this->mbuf[iarray][this->mcurrent[iarray]];
		if (msg->isRecv)
		{
			recvDataLen = SocketUtils::GetDownstreamDataLen(msg->cmd);
			msg->dataLen = recvDataLen;
		}

		else
		{
			//if there is no head format ,delete previous items
			printf("socket message direction is error: %x!\n", msg->cmd);
			//go on to find Head format
			this->strategy[iarray] = SEARCH_HEAD;
		}
		if (this->strategy[iarray] == SEARCH_HEAD) {
			//delete the first item 'a5'
			//move back 1 items
			this->mcurrent[iarray] =
					this->mcurrent[iarray] >= 1 ? (this->mcurrent[iarray] - 1) : (MSG_CACHE_SIZE - 1 + this->mcurrent[iarray]);
			this->mfront[iarray] = this->mcurrent[iarray];
			this->mlen[iarray] -= 1;
			this->mtag[iarray] = 0;
//			this->strategy = SEARCH_HEAD;
		} else {
			this->mcurrent[iarray] = (this->mcurrent[iarray] + 1) % MSG_CACHE_SIZE;
			this->strategy[iarray] = SEARCH_LEN;
		}
		break;

	case SEARCH_LEN: //to find the type len in cache
		if (currLen < MSG_CMD_LEN) {
//			this->mcurrent[iarray] = (this->mcurrent[iarray] + currLen) % MSG_CACHE_SIZE;//更新到最新位置
			this->mcurrent[iarray] = this->mfront[iarray];
			this->strategy[iarray] = SEARCH_HEAD;
			return 0;
		}
		//get the value of type
		recvDataLen = this->mbuf[iarray][this->mcurrent[iarray]];
		if(msg->dataLen != recvDataLen){
			this->mcurrent[iarray] = (this->mcurrent[iarray] + currLen) % MSG_CACHE_SIZE;//更新到最新位置
			this->mfront[iarray] = this->mcurrent[iarray];
			return 0;
		}
		if (recvDataLen < 0) {
			//delete the first item 'a5'
			//move back 1 items
			this->mcurrent[iarray] =
					this->mcurrent[iarray] >= 1 ? (this->mcurrent[iarray] - 1) : (MSG_CACHE_SIZE - 1 + this->mcurrent[iarray]);
			this->mfront[iarray] = this->mcurrent[iarray];
			this->mlen[iarray] -= 1;
			this->mtag[iarray] = 0;
			this->strategy[iarray] = SEARCH_HEAD;
		} else {
			msg->dataLen = recvDataLen;
			this->mcurrent[iarray] = (this->mcurrent[iarray] + 1) % MSG_CACHE_SIZE;
			this->strategy[iarray] = SEARCH_END;
		}
		break;

	case SEARCH_END:
		if (currLen < (msg->dataLen + MSG_CRC_SIZE + MSG_END_SIZE))
		{
//			this->mcurrent[iarray] = (this->mcurrent[iarray] + currLen) % MSG_CACHE_SIZE;//更新回去
			this->mcurrent[iarray] = this->mfront[iarray];
			this->strategy[iarray] = SEARCH_HEAD;
			return 0;
		}
		// Because we have known the data bytes' length, so we move the very
		// distance of data_len to see if there is End format.
		p = (this->mcurrent[iarray] + msg->dataLen + MSG_CRC_SIZE) % MSG_CACHE_SIZE;
		if (this->mbuf[iarray][p] == MSG_END) {		// 匹配到完整指令
			__MsgCopyOut(iarray,msg->data, this->mcurrent[iarray], msg->dataLen);
			// Delete all previous items.
			this->mcurrent[iarray] = (p + 1) % MSG_CACHE_SIZE;
			this->mfront[iarray] = this->mcurrent[iarray];
			this->mlen[iarray] -= (msg->dataLen + SOCKET_MSG_FORMAT_SIZE);
			this->mtag[iarray] = 0;

			msg->Update();
			printf("success!\n");
			this->strategy[iarray] = SEARCH_HEAD;
			return 1;
			//muartServer->DataSorting(fd, this->msg.Update(), this->args);
			// this->msg.Update() 更新SocketMsg参数，buf、len等
			//mFindMsgListener->onFindInstruction(fd, this->msg.Update(), this->args);
		} else {
			printf("socket message without end: %x! IS %x\n", MSG_END ,this->mbuf[iarray][p]);
			// Delete the first item 'a5'
			// Move back 2 items
			this->mcurrent[iarray] =
					this->mcurrent[iarray] >= 3 ? (this->mcurrent[iarray] - 3) : (MSG_CACHE_SIZE - 3 + this->mcurrent[iarray]);
			this->mfront[iarray] = this->mcurrent[iarray];
			// Length sub 2
			this->mlen[iarray] -= 1;
			this->mtag[iarray] = 0;
		}
		this->strategy[iarray] = SEARCH_HEAD;
		break;

	default:
		break;
	}
	// Parse new socket message.
	return __MsgParse(iarray,msg);
}
/*
//来源指针，数据起始地址，数据长度，空
inline bool UartCache::MsgPreParse(int fd, uint8_t *buf, int len, void *args,Msg* msg) {
	int n = 0;
	uint8_t *p = buf;
	// When reading buffer's length is greater than cache's left length,
	// We should copy many times.
	msg->f_dsource = fd;//配置来源信道
	this->args = args;
	while (1) {
		n = __MsgCopyIn(p, len,msg);
		if (n == 0) {
			return false;		// Cache is full
		}
		// Parse and handle socket message from cache
		__MsgParse(fd,msg);
#if DEBUG
		printf("datalen n: %d &  len: %d this->mlen[iarray]: %d\n",n,len,this->mlen[iarray]);
#endif
		if ((n == len)&&(this->mlen==0)) {
#if DEBUG
			printf("return successful!\n");
#endif
			break; // Copy completed
		}
		// Move the pointer

		p = p + n;
		len = len - n;
		this->mlen = 0;
	}

	return true;
}*/
//参数说明：iarray 串口标号 buf 数据起始指针 len 数据长度 args NULL msg 信息载体
inline bool UartCache::UartRevArray(uint8_t iarray,uint8_t* buf,int len,void *args, Msg* msg){
	if(len > 0)
		if(__MsgCopyIn(iarray,buf,len,msg)==0)
			return false;
	if(__MsgParse(iarray,msg))
		return true;
	return false;
}
#endif /* UartCache */
