#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#define SERVER_PORT 6666

#define MAXBUFLEN 1024

time_t start;
time_t end;
unsigned int count = 0;
#define THREAD_LENGTH 4
pthread_t tid[THREAD_LENGTH];

static void signal_handler(int s)
{
	if (s == SIGINT)
	{
		end = time(NULL);
		time_t diff = end - start;
		double dt = (double)diff;
		double rs = count / dt;
		printf("\nuse time=%0.2fs, total request=%d, avarge is %0.2f r/s\n",dt,count,rs);
		for (int i=0;i<THREAD_LENGTH;++i)
		{
			pthread_cancel(tid[i]);
		}
		exit(0);
	}
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * run(void * arg)
{
	int sockfd;
	struct sockaddr_in server_addr;
	char buf[MAXBUFLEN];
	int m_count;
	while (true) {
		m_count = ++count;
		sockfd = socket(AF_INET,SOCK_STREAM,0);
		if (-1 == sockfd)
		{
			perror("socket failed..");
			_exit(0);
		}

		memset(&server_addr,0,sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(SERVER_PORT);
		server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

		if (-1 == connect(sockfd,(struct sockaddr*)&server_addr, sizeof(struct sockaddr)))
		{
			perror("connect error");
			_exit(0);
		}
		printf("[%u] connect successfully\n", m_count);
		recv(sockfd,buf,MAXBUFLEN,0);
		printf("[%u] Server Message:%s\n",m_count,buf);
		close(sockfd);
		printf("[%u] client connect close\n", m_count);
	}
}

int main()
{
	signal(SIGINT,signal_handler);

    int sockfd;
    struct sockaddr_in server_addr;
    char buf[MAXBUFLEN];

    start = time(NULL);

    for (int i=0;i<THREAD_LENGTH;++i)
    {
    	pthread_create(&tid[i],NULL,run,NULL);
    }

    for (int i=0;i<THREAD_LENGTH;++i)
    {
    	pthread_join(tid[i],NULL);
    }

//    while (true) {
//    	++count;
//    	sockfd = socket(AF_INET,SOCK_STREAM,0);
//		if (-1 == sockfd)
//		{
//			perror("socket failed..");
//			_exit(0);
//		}
//
//		memset(&server_addr,0,sizeof(server_addr));
//		server_addr.sin_family = AF_INET;
//		server_addr.sin_port = htons(SERVER_PORT);
//		server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
//
//		if (-1 == connect(sockfd,(struct sockaddr*)&server_addr, sizeof(struct sockaddr)))
//		{
//			perror("connect error");
//			_exit(0);
//		}
//		printf("[%u] connect successfully\n", count);
//		recv(sockfd,buf,MAXBUFLEN,0);
//		printf("[%u] Server Message:%s\n",count,buf);
//		close(sockfd);
//		printf("[%u] client connect close\n", count);
//    }

    return 0;
}
