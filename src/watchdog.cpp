#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "watchdog.h"
#include "logger.h"
#include "exit.h"

static int s_watch_dog_pid;
static int s_pipe_fd[2];

void fl_start_watch_dog()
{
	int result = pipe(s_pipe_fd);
	if (result < 0)
	{
		fl_log(2, "create watch pipe failed..\n");
		fl_main_exit();
		return;
	}
	s_watch_dog_pid = fork();
	if (s_watch_dog_pid < 0)
	{
		fl_log(2, "watch dog fork failed..\n");
		fl_main_exit();
		return;
	}
	if (s_watch_dog_pid > 0)
	{
		close(s_pipe_fd[0]);
		fl_log(0, "watch dog start successfully,pid = %d\n", s_watch_dog_pid);
	}
	else
	{
		close(s_pipe_fd[1]);
	}
}

void fl_destory_watch_dog()
{

}
