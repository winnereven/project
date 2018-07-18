/**
 * File name: Socket.h
 * Author: Even
 * Email: 871454583@qq.com
 * Version: 1.0
 * Created on: 2017年8月28日
 * Description: 负责Socket数据收发 */

#ifndef SOCKETSERVER_H_
#define SOCKETSERVER_H_

#include "../common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "../data/Msg.h"
#include "../utils/DevUtils.h"


class SocketServer {
public:
	SocketServer();
	virtual ~SocketServer();

	int GetNetData(uchar* buf,int* lenofdata);
	void GetNetAccept();
	void DisConnect();
	void StartUartTask();
	int CheckStatu();
	void SendMsg(Msg *msg);
	void Sendbuf(uchar* buf,int lenofdata);

private:

	int m_nSocketServerFd, m_nSocketClientFd;
//	int m_fdUART1;
//	int m_bUartEn;
	socklen_t sin_size;                      	// to store struct size


	// 上行指令处理
	void __UpstreamProcessor(int fd, Msg *msg, void *args);



};

inline SocketServer::SocketServer(): m_nSocketClientFd(0){
	struct sockaddr_in addr_local;
	sin_size = sizeof(struct sockaddr_in);

	/* Get the Socket file descriptor */
	if ((m_nSocketServerFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("ERROR: Failed to obtain Socket Descriptor.\n");
		return;
	} else {
		printf("OK: Obtain Socket Descriptor successfully.\n");
	}

	/* Fill the local socket address struct */
	addr_local.sin_family = AF_INET;				// Protocol Family
	addr_local.sin_port = htons(SOCKET_PORT);         		// Port number
	addr_local.sin_addr.s_addr = htonl(INADDR_ANY);	// AutoFill local address
	memset(addr_local.sin_zero, 0, 8);				// Flush the rest of struct

	/* Bind a special Port */
	if (bind(m_nSocketServerFd, (struct sockaddr*) &addr_local, sizeof(struct sockaddr)) == -1) {
		printf("ERROR: Failed to bind Port %d.\n", SOCKET_PORT);
		exit(EXIT_SUCCESS);
		//重新启动
	} else {
		printf("OK: Bind the Port %d successfully.\n", SOCKET_PORT);
	}

	/* Listen remote connect/calling */
	if (listen(m_nSocketServerFd, BACKLOG) == -1) {
		printf("ERROR: Failed to listen Port %d.\n", SOCKET_PORT);
		return;
	} else {
		printf("OK: Listening the Port %d successfully.\n", SOCKET_PORT);
	}

}

inline SocketServer::~SocketServer() {
	close(m_nSocketServerFd);
	m_nSocketServerFd = -1;
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
	printf("SocketServer Object destroyed!\n");
}
inline void SocketServer::DisConnect()
{
	close(m_nSocketClientFd);
	m_nSocketClientFd = -1;

}
inline int SocketServer::CheckStatu(){
	if(m_nSocketServerFd > 0 && m_nSocketClientFd > 0)
		return 1;
	else
		return 0;
}
inline void SocketServer::GetNetAccept(){
	struct sockaddr_in addr_remote;
	while(1){
		usleep(LED_DELAY_US);
		/*  Wait a connection, and obtain a new socket file descriptor for single connection */
		if ((m_nSocketClientFd = accept(m_nSocketServerFd, (struct sockaddr *) &addr_remote,
				&sin_size)) == -1) {
			printf("ERROR: Obtain new Socket Descriptor error.\n");
			continue;
		}
		else {
	//			m_bUartEn = false;
	//			mOpticalDevice->SetFeedDogInSocket(true);
			printf("OK: Server has got connect from %s.\n", inet_ntoa(addr_remote.sin_addr));
			printf("SocketServer Object created!\n");
			break;
		}

	}

}
inline int SocketServer::GetNetData(uchar* buf,int* lenofdata) {

		/* Child process */

			*lenofdata = recv(m_nSocketClientFd, buf, BUFFER_SIZE, 0); // 接收
			return m_nSocketClientFd;
//			if (*lenofdata  == 0) {
//				m_bUartEn = true;
//				mOpticalDevice->SetFeedDogInSocket(false);
//				printf("client disconnect\n");
//				break;
//			}
//			printf("receive data:");
//			for (int i = 0; i < *lenofdata ; i++) {
//				printf("%x ", buf[i]);
//			}
//			printf("\n");

			//__UartSend(mRecvBuf, recvCount);

//			mdataCache->MsgPreParse(m_nSocketClientFd, mRecvBuf, recvCount, NULL);

//			memset(mRecvBuf, 0, sizeof(mRecvBuf));
			//usleep(DATA_PARSE_DELAY_US);


}



inline void SocketServer::SendMsg(Msg *msg) {
	if(m_nSocketClientFd > 0)
		__UpstreamProcessor(m_nSocketClientFd, msg, NULL);
	else
		printf("socket is not connected!\n");
}

inline void SocketServer::Sendbuf(uchar* buf,int lenofdata){
	int ret;
	if(m_nSocketClientFd > 0){
		if ((ret = send(m_nSocketClientFd, buf, lenofdata, 0)) == -1) {
			printf("ERROR: Failed  to sent string.\n");
		}
	}
	else
		printf("socket is not connected!\n");

}

inline void SocketServer::__UpstreamProcessor(int fd, Msg *msg, void *args) {
	uint8_t sdbuf[SOCKET_MSG_DATA_SIZE];
	int len = msg->CopyToBuf(sdbuf, 0);
	int ret;
	printf("upstream:");
	for (int i = 0; i < len; i++) {
		printf("%x ", sdbuf[i]);
	}
	printf("\n");
	if ((ret = send(fd, sdbuf, len, 0)) == -1) {
		printf("ERROR: Failed  to sent string.\n");
//		close(fd);
//		exit(1);
	}
//	if (m_bUartEn)
//		__UartSend(sdbuf, len);
//	printf("OK: Sent %d bytes successful, please enter again.\n", ret);
}





#endif /* SOCKETSERVER_H_ */
