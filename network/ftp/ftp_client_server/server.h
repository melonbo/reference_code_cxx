#ifndef _SERVER_H_
#define _SERVER_H_
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

#define SERV_PORT 3000  //the port to send data
#define CMD_LEN 1024
#define BUFFER_SIZE 4096

#define ROOT "/home"  //root file
#define USER "user9"  //my user name
#define PASSWD "ftp9"  //my password

char* get_path(char* path, const char* file);
char* get_filename(char* filename, const char* file);
int get_real_path(char* real_path, const char* file);
int is_subdir(const char* perfix, const char* dir);
int convert_path(char* vdir, const char* perfix, const char* rdir);


#endif