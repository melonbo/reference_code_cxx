#ifndef LOCALCLIENT_H
#define LOCALCLIENT_H
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "config/util.h"

class LocalClient
{
public:
    LocalClient();
    void setDomainSend(char* domain){send_domain = domain;}
    void setDomainRecv(char* domain){recv_domain = domain;}
    int initSendSocket();
    int initReceiveSocket();
    int sendData(char* data, int len);
    int receiveData();
    void dealReceiveData(char*data, int size);
    char *recv_domain;
    char *send_domain;
    char recv_buf[1024];
    int listen_fd, server_fd, client_fd;
    struct sockaddr_un server_addr;
    struct sockaddr_un local_addr;
    struct sockaddr_un client_addr;
    DataDealCallBack pDataDealCallBack;
    void setCallBackFunc(DataDealCallBack callback);
};

#endif // LOCALCLIENT_H
