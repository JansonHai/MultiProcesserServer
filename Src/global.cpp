//all global variables declare here
#include "global.h"

#include <mysql/mysql.h>

//int g_lisetn_fd = -1;  //socket listen fd
int g_exit_flag = false;
MYSQL * g_mysql_conn;
