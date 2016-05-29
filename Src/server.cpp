#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <mysql/mysql.h>
#include <list>
#include <errno.h>

#include "envirment.h"
#include "exit.h"
#include "global.h"
#include "handler.h"
#include "logger.h"

static int lisetn_fd = -1;
extern bool g_exit_flag;
extern MYSQL * g_mysql_conn;

static void s_clear_server(void * arg)
{
	if (-1 != lisetn_fd)
	{
		fl_exit_log("close listen fd\n");
		close(lisetn_fd);
	}
	lisetn_fd = -1;
	mysql_library_end();
	kill(0, SIGTERM);
}

void fl_server_start()
{
	fl_add_main_exit_handle(s_clear_server);

	int listenfd = -1;
	struct sockaddr_in server_addr;
	listenfd = socket(AF_INET,SOCK_STREAM,0);
	if (-1 == listenfd)
	{
		fprintf(stderr,"socket failed..\n");
		exit(0);
	}

	//set server information
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((uint16_t)fl_getenv("server_port",6666));
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //any ip is ok

	//bind server
	if (-1 == (bind(listenfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))))
	{
		fprintf(stderr,"bind error..\n");
		exit(0);
	}

	lisetn_fd = listenfd;

	//listen server
	if (-1 == listen(listenfd,1024))
	{
		fprintf(stderr,"listen error...\n");
		exit(0);
	}

	char * addr_tmp;
	uint16_t port_tmp;
	addr_tmp = inet_ntoa(server_addr.sin_addr);
	port_tmp = ntohs(server_addr.sin_port);
	fprintf(stdout,"listen on %s : %d\n", addr_tmp, port_tmp);

}

static void * s_moniter_zombie_child(void * arg)
{
	while (true)
	{
		if (true == g_exit_flag)
		{
			break;
		}
		wait(NULL);
		usleep(500000);  //500ms
	}
	pthread_exit(0);
}

static void * s_wait_thread_run(void * arg)
{
	pid_t pid = *(pid_t *)arg;
	waitpid(pid, NULL, 0);
	pthread_exit(0);
}

void fl_server_main_loop()
{
	int clientfd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(struct sockaddr);

	fprintf(stdout,"starting main loop\n");
	pid_t pid = -1;
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, s_moniter_zombie_child, NULL);

	mysql_library_init(0,NULL,NULL);
	g_mysql_conn = mysql_init(NULL);

	//accept client
	while (true)
	{
		clientfd = accept(lisetn_fd,(struct sockaddr*)&client_addr,&client_len);
		if (-1 == clientfd)
		{
			fl_log(2,"Accept client error,errno = %d\n",errno);
			if (errno == EAGAIN)
			{
				continue;
			}
			else
			{
				break;
			}
		}

		fl_debug_log("trying fork..");
		pid = fork();

		if (0 == pid)
		{
			fl_debug_log("child fork back..");
			//child process
			close(lisetn_fd);

			pid = getpid();
			struct handle_info client_info;
			client_info.pid = pid;
			client_info.socketfd = clientfd;
			client_info.client_addr = client_addr;
			strcpy(client_info.ip,inet_ntoa(client_addr.sin_addr));
			client_info.port = ntohs(client_addr.sin_port);

			fl_debug_log("child begin handle..");

			fl_start_child_handle(client_info);
			fl_child_exit();
		}
		else
		{
			fl_debug_log("parent fork back..");
			//parent process
			close(clientfd);
			pthread_create(&tid, &attr, s_wait_thread_run, &pid);
		}
	}
}




