/**
 * File name: SocketCache.h
 * Author: JoshuaWong
 * Email: 1034316376@qq.com
 * Version: 1.0
 * Created on: Jun 27, 2017 11:35:33 PM
 * Description: 
 */

#ifndef DATACACHE_H_
#define DATACACHE_H_

#include "../common.h"
#include "../utils/SocketUtils.h"
#include "Msg.h"
//#include "../socket/SocketServer.h"
//#include "../uart/UartServer.h"

class DataCache {
public:
//	Msg msg;
//	UartServer* muartServer;
//	SocketServer* msocketServer;

	DataCache();
	virtual ~DataCache();
	bool MsgPreParse(int fd, uint8_t *buf, int len, void *args, Msg* msg);	// 通信数据解析

private:
	uint8_t buf[MSG_CACHE_SIZE];
	int front;
	int rear;
	int current;
	int len;
	int tag;
	SearchStrategy strategy;
	void *args;

	int __MsgCopyIn(uint8_t *buf, int len, Msg* msg);	// 将数据添加入缓存
	int __MsgCopyOut(uint8_t *buf, int start_index, int len);	//将数据移出缓存
	int __MsgParse(int fd ,Msg* msg);		// 通信数据解析
};

inline DataCache::DataCache():
		front(0), rear(0), current(0), len(0), tag(0), strategy(SEARCH_HEAD), args(NULL){
	printf("SocketCache object created!\n");
}

inline DataCache::~DataCache() {
	printf("SocketCache object destroyed!\n");
}

inline int DataCache::__MsgCopyIn(uint8_t *buf, int len, Msg* msg) {
	int left_len;	// 剩余长度
	int copy_len;
	int count;
	uint8_t *src = buf;

	if (this->tag == 1 && this->front == this->rear) {
#if DEBUG
		printf("socket cache is full! tag is %d \n" , this->tag);
#endif
		this->tag = 0;
		this->strategy = SEARCH_HEAD;
		return 0;
	}

	left_len = MSG_CACHE_SIZE - this->len;
	copy_len = len > left_len ? left_len : len;
	count = copy_len;

	if(count == 0) return 0;
	while (count--) {
		*(this->buf + this->rear) = *src;	// 入队
		src++;
		this->rear = (this->rear + 1) % MSG_CACHE_SIZE;
		this->len++;//收到的指令的总长度
		this->tag = 1;
	}

	return copy_len;
}

inline int DataCache::__MsgCopyOut(uint8_t *buf, int start_index, int len) {
	uint8_t *dest;
	int src;
	int count;
	if (this->buf == NULL || buf == NULL || len == 0) {
		return 0;
	}
	if (this->front == this->rear && this->tag == 0) {
#if DEBUG
		printf("socket cache is empty!");
#endif
		return 0;
	}
	if (this->rear > this->front) {	// 如果循环队列不为空
		if (start_index < this->front || start_index > this->rear) {
#if DEBUG
			printf("invalid start index!");
#endif
			return 0;
		}
	} else if (start_index > this->rear && start_index < this->front) {
#if DEBUG
		printf("invalid start index!");
#endif
		return 0;
	}

	src = start_index;
	dest = buf;
	count = len;
	while (count--) {
		*dest = *(this->buf + src);
		dest++;
		src = (src + 1) % MSG_CACHE_SIZE;
	}
	return len;
}
//参数说明：　fd 句柄　msg 载体
//返回值：1表示成功 0或者其他表示失败
inline int DataCache::__MsgParse(int fd, Msg* msg) {
	int currLen;
	int p, q;
	int i;
	bool find;
	int recvDataLen;
	//Msg* msg = pmsg;

	if (this->front == this->rear && this->tag == 0) { // Cache为空
		return 0;
	}

	if (this->current >= this->front) {
		currLen = this->len - (this->current - this->front);
	} else {
		currLen = this->rear - this->current;
	}

#if DEBUG
	int ioffset;
	printf("strategy is : %x. data is \n",this->strategy);
	for(ioffset=0;ioffset<currLen;ioffset++)
	{
		printf("%x  ",this->buf[this->current + ioffset]);
	}
#endif
	switch (this->strategy) {
	case SEARCH_HEAD:
		if (currLen < MSG_HEAD_SIZE) {
//			this->current = (this->current + currLen) % MSG_CACHE_SIZE;//更新到最新位置
			this->current=this->front ;//回到初始位置
			return 0;
		}
		find = false;
		for (i = 0; i < currLen - 1; i++) {
			p = this->current;
			q = (this->current + 1) % MSG_CACHE_SIZE;
			if ((this->buf[p] == (MSG_HEAD >> 8)) && (this->buf[q] == (MSG_HEAD & 0xff))) {
				msg->isRecv = true;
				find = true;
				break; // exit for loop
			} else {
				// current pointer move to next
				this->current = q;
				// delete one item
				this->front = this->current;
				this->len--;
				this->tag = 0;
			}
		}

		if (find == true) {
			//move 2 items towards next
			this->current = (this->current + 2) % MSG_CACHE_SIZE;
			//we found the head format, go on to find Type byte
			this->strategy = SEARCH_CMD;
		} else {
			//if there is no head format ,delete previous items
			printf("socket message without head: %x!\n", MSG_HEAD);
			//go on to find Head format
			this->current = (this->current + 1) % MSG_CACHE_SIZE;//更新到最新位置
			this->front = this->current;
			this->strategy = SEARCH_HEAD;
		}
		break;

	case SEARCH_CMD: //to find the type direction in cache
		if (currLen < MSG_CMD_SIZE) {
//			this->current = (this->current + currLen) % MSG_CACHE_SIZE;//更新到最新位置
			this->current=this->front;//回到初始位置
			this->strategy = SEARCH_HEAD;
			return 0;
		}
		//get the value of type
		msg->cmd = this->buf[this->current];
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
			this->strategy = SEARCH_HEAD;
		}
		if (this->strategy == SEARCH_HEAD) {
			//delete the first item 'a5'
			//move back 1 items
			this->current =
					this->current >= 1 ? (this->current - 1) : (MSG_CACHE_SIZE - 1 + this->current);
			this->front = this->current;
			this->len -= 1;
			this->tag = 0;
//			this->strategy = SEARCH_HEAD;
		} else {
			this->current = (this->current + 1) % MSG_CACHE_SIZE;
			this->strategy = SEARCH_LEN;
		}
		break;

	case SEARCH_LEN: //to find the type len in cache
		if (currLen < MSG_CMD_LEN) {
//			this->current = (this->current + currLen) % MSG_CACHE_SIZE;//更新到最新位置
			this->current= this->front;//回到初始位置
			this->strategy = SEARCH_HEAD;
			return 0;
		}
		//get the value of type
		recvDataLen = this->buf[this->current];
		if(msg->dataLen != recvDataLen){
			this->current = (this->current + currLen) % MSG_CACHE_SIZE;//更新到最新位置
			this->front = this->current;
			return 0;
		}
		if (recvDataLen < 0) {
			//delete the first item 'a5'
			//move back 1 items
			this->current =
					this->current >= 1 ? (this->current - 1) : (MSG_CACHE_SIZE - 1 + this->current);
			this->front = this->current;
			this->len -= 1;
			this->tag = 0;
			this->strategy = SEARCH_HEAD;
		} else {
			msg->dataLen = recvDataLen;
			this->current = (this->current + 1) % MSG_CACHE_SIZE;
			this->strategy = SEARCH_END;
		}
		break;

	case SEARCH_END:
		if (currLen < (msg->dataLen + MSG_CRC_SIZE + MSG_END_SIZE))
		{
			//this->current = (this->current + currLen) % MSG_CACHE_SIZE;//更新到最初位置
			this->current = this->front ;
			this->strategy = SEARCH_HEAD;
			return 0;
		}
		// Because we have known the data bytes' length, so we move the very
		// distance of data_len to see if there is End format.
		p = (this->current + msg->dataLen + MSG_CRC_SIZE) % MSG_CACHE_SIZE;
		if (this->buf[p] == MSG_END) {		// 匹配到完整指令
			__MsgCopyOut(msg->data, this->current, msg->dataLen);
			// Delete all previous items.
			this->current = (p + 1) % MSG_CACHE_SIZE;
			this->front = this->current;
			this->len -= (msg->dataLen + SOCKET_MSG_FORMAT_SIZE);
			this->tag = 0;

			msg->Update();
			printf("success!\n");
			this->strategy = SEARCH_HEAD;
			return 1;
			//muartServer->DataSorting(fd, this->msg.Update(), this->args);
			// this->msg.Update() 更新SocketMsg参数，buf、len等
			//mFindMsgListener->onFindInstruction(fd, this->msg.Update(), this->args);
		} else {
			printf("socket message without end: %x! IS %x\n", MSG_END ,this->buf[p]);
			// Delete the first item 'a5'
			// Move back 2 items
			this->current =
					this->current >= 3 ? (this->current - 3) : (MSG_CACHE_SIZE - 3 + this->current);
			this->front = this->current;
			// Length sub 2
			this->len -= 1;
			this->tag = 0;
		}
		this->strategy = SEARCH_HEAD;
		break;

	default:
		break;
	}
	// Parse new socket message.
	return __MsgParse(fd,msg);

}
//来源指针，数据起始地址，数据长度，空
inline bool DataCache::MsgPreParse(int fd, uint8_t *buf, int len, void *args,Msg* msg) {
	int n = 0;
	uint8_t *p = buf;
	// When reading buffer's length is greater than cache's left length,
	// We should copy many times.
	msg->f_dsource = fd;//配置来源信道
	this->args = args;
	if(len > 0){
		n = __MsgCopyIn(p, len,msg);
		if (n == 0) {
			return false;		// Cache is full
		}
	}
		// Parse and handle socket message from cache
		if(__MsgParse(fd,msg))
		{
			return true;
		}
//#if DEBUG
//		printf("datalen n: %d &  len: %d this->len: %d\n",n,len,this->len);
//#endif
//		if ((n == len)&&(this->len==0)) {
//#if DEBUG
//			printf("return successful!\n");
//#endif
//			return true; // Copy completed
//		}
		// Move the pointer

		//p = p + n;
		//len = len - n;
		//this->len = 0;


	return false;
}

#endif /* SOCKET_DATACACHE_H_ */
