#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <netdb.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERV_PORT 3000
#define CMD_LEN 1024
#define BUFFER_SIZE 4096

#define GET_CMD "get"
#define PUT_CMD "put"
#define PWD_CMD "pwd"
#define DIR_CMD "dir"
#define CD_CMD  "cwd"
#define HELP_CMD "?"
#define QUIT_CMD "quit"

#endif

