#ifndef MULTICASTSOCKET_H
#define MULTICASTSOCKET_H
#include <sys/socket.h>
#include "CUdpSocket.h"
#include "sys/epoll.h"
#include <stdio.h>

#define MaxCount 10

class CMulticastSocket:private ICommonThread
{
public:
    CMulticastSocket();
    virtual ~CMulticastSocket();

    bool SendTo(const void*, int);
    bool CreateMulticastSocket(char* strGroupIP, unsigned int nGroupPort);
    void StartSocket();
    void initSendFrame();
    void dealRecvData(unsigned char* buf, int size);
    int  CreateSendingSocket(char* strGroupIP, unsigned int nGroupPort);
    int  CreateReceivingSocket(char* strGroupIP, unsigned int nGroupPort);
   
public:
    int m_socketHandle;
    int m_portReceiv;
    int fd_recv, fd_send;
    pthread_t pth_recv, pth_send,;

    struct sockaddr_in multicast_addr;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
    struct sockaddr_in addr_src;
    struct sockaddr_in addr_dst;
    struct epoll_event m_Events;
    pthread_mutex_t lock;


private:
    void ThreadProc();
    bool IsRun();
    struct epoll_event *m_pEvents;
    int m_epfd;
    CCommonThread m_commonThread;
};

#endif // MULTICASTSOCKET_H
