#ifndef SRC_NET_H_
#define SRC_NET_H_

#include <unistd.h>

struct connection_info
{
	int session;
	int clientfd;

};

int send_fd(int recvfd, int sendfd);
int recv_fd(int recvfd, int sendfd);

#endif /* SRC_NET_H_ */
