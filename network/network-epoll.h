/*
 *  NetWork.h
 *
 *  send or rcv data by net of TRDS Wrapper by C++11
 *
 */

#ifndef __NET_WORK_H
#define __NET_WORK_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>  //struct sockaddr_in
#include <sys/epoll.h> //epoll


class NetWork
{
private:
    int sock_recv; 
    struct sockaddr_in  addr;

    struct epoll_type
    {
        struct epoll_event *ep_events;
        struct epoll_event event;
        int epfd;
    };
    
public:
    NetWork();
    ~NetWork();

public:
    void init_net_server();
    int  epoll_dispatch();

private:
    void epoll_init();
    void init_mcast_server(uint16_t mcastPort, const char* mcastAddr, int *socket_fd, const char* ifname);
    void init_mcast_client(uint16_t mcastPort, const char* mcastAddr, int *socket_fd, struct sockaddr_in* addr_dst);

private:
    struct sockaddr_in addr_dst;

private:

private:
    static constexpr int MAX_EVENTS = 10;

};

#endif