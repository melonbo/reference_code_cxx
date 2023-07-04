/*
 *  NetWork.cpp
 *
 *  send or rcv data by net of TRDS Wrapper by C++11
 *
 */

#include "NetWork.h"

#include <string.h>
#include <net/if.h>  //struct ifreq

#include <arpa/inet.h> //inet_addr
#include <sys/ioctl.h> //ioctl

#include <unistd.h> //close

#include <sys/time.h> //gettimeofday
#include <ifaddrs.h> //getifaddrs
#include <vector>
#include <errno.h>

using namespace std;


NetWork::NetWork()
{
}

NetWork::~NetWork() {}

void NetWork::init_net_server()
{
    init_mcast_server(MCAST_PORT_RCV, MCAST_ADDR_RCV, &sock_recv, if_name);   
    init_mcast_client(MCAST_PORT_SND, MCAST_ADDR_SND, &sock_send, &addr_dst);
    epoll_init();    
}

void NetWork::init_net_client_snd_communicate()
{
    init_mcast_client(MCAST_PORT_SND_COMMUNICATE, MCAST_ADDR_SND_COMMUNICATE, &m_sock.sock_snd_mcast_communicate, &addr_dst_communicate);
    pack_and_snd_mcast_data(m_sock.sock_snd_mcast_communicate, 1);
}

void NetWork::epoll_init()
{
    m_epoll.epfd = epoll_create1(0);
    if(m_epoll.epfd == -1)
    {
        LOG_ERROR_FORMAT("epoll create failed,%d,%s.",errno,strerror(errno));
    }
    m_epoll.ep_events = (struct epoll_event *)calloc(MAX_EVENTS,sizeof(struct epoll_event));
    if(m_epoll.ep_events == NULL)
    {
        LOG_ERROR_FORMAT("ep_events calloc failed,%d,%s.",errno,strerror(errno));
    }

    m_epoll.event.events = EPOLLIN;
    m_epoll.event.data.fd = sock_recv;
    if(-1 == epoll_ctl(m_epoll.epfd,EPOLL_CTL_ADD, sock_recv,&m_epoll.event))
    {
        LOG_ERROR_FORMAT("epoll_ctl failed,%d,%s.",errno,strerror(errno));
    }

    m_epoll.event.events = EPOLLIN;
    m_epoll.event.data.fd = sock_senc;
    if (-1 == epoll_ctl(m_epoll.epfd, EPOLL_CTL_ADD, sock_send, &m_epoll.event))
    {
        LOG_ERROR_FORMAT("epoll_ctl failed,%d,%s.", errno, strerror(errno));
    }
}

int NetWork::epoll_dispatch()
{
    int event_cnt;

    for(;;)
    {
        event_cnt = epoll_wait(m_epoll.epfd,m_epoll.ep_events,MAX_EVENTS,-1);
        if(event_cnt == -1)
        {
            LOG_ERROR_FORMAT("epoll_wait error,%d,%s.",errno,strerror(errno));
        }

        for(int i=0;i<event_cnt;++i)
        {
            printf("event %d type is %d, fd %d\n", i, m_epoll.ep_events[i].events, m_epoll.ep_events[i].data.fd);
            if((m_epoll.ep_events[i].events & EPOLLERR) || (m_epoll.ep_events[i].events & EPOLLHUP))
            {
                LOG_WARNING_FORMAT("epoll events error,%d.",m_epoll.ep_events[i].events);
                close(m_epoll.ep_events->data.fd);
                continue;
            }

            if(m_epoll.ep_events[i].data.fd == sock_recv)
            {
                struct sockaddr_in client_addr;
                socklen_t ucast_adr_sz = sizeof(client_addr);
                sock_clnt = accept(sock_rcv_ucast,(struct sockaddr*)&client_addr,&ucast_adr_sz);
                if(-1 == sock_clnt)
                {
                    LOG_ERROR_FORMAT("ucast accept error,%d,%s.",errno,strerror(errno));
                }
                m_epoll.event.events  = EPOLLIN;
                m_epoll.event.data.fd = sock_clnt;

                for(int i=0; i<MAX_UCAST_CLINETS; i++)
                {
                    if(sock_ucast_clients[i] == 0)
                        sock_ucast_clients[i] = sock_clnt;
                }

                char* ip = inet_ntoa(client_addr.sin_addr);

                printf("add socket client %d, %s:%d to epoll\n", sock_clnt, ip, client_addr.sin_port);
                if(-1 == epoll_ctl(m_epoll.epfd,EPOLL_CTL_ADD,m_sock.sock_clnt,&m_epoll.event))
                {
                    LOG_ERROR_FORMAT("epoll_ctl ucast accept error,%d,%s.",errno,strerror(errno));
                }
                continue;
            }

            if(m_epoll.ep_events[i].data.fd == m_sock.sock_rcv_ptu)
            {
                socklen_t ptu_adr_sz = sizeof(m_addr.rmt_local_addr);
                m_sock.sock_ptu_clnt = accept(m_sock.sock_rcv_ptu,(struct sockaddr*)&m_addr.rmt_local_addr,&ptu_adr_sz);
                if(-1 == m_sock.sock_ptu_clnt)
                {
                    LOG_ERROR_FORMAT("ptu accept error,%d,%s.",errno,strerror(errno));
                }
                m_epoll.event.events  = EPOLLIN;
                m_epoll.event.data.fd = m_sock.sock_ptu_clnt;
                if(-1 == epoll_ctl(m_epoll.epfd,EPOLL_CTL_ADD,m_sock.sock_ptu_clnt,&m_epoll.event))
                {
                    LOG_ERROR_FORMAT("epoll_ctl ptu accept error,%d,%s.",errno,strerror(errno));
                }
                continue;
            }

            if(m_epoll.ep_events[i].events & EPOLLIN)
            {
                if(m_epoll.ep_events[i].data.fd == sock_rcv_02) 
                {
                    //ssize_t mcast_rcv_len = recvfrom();         
                    continue;
                }
            }
            
            if(m_epoll.ep_events->events & EPOLLOUT)
            {
                printf("event no matched, type is %d, fd %d\n", m_epoll.ep_events[i].events, m_epoll.ep_events[i].data.fd);
            }
        }
    }
}

void NetWork::init_client(uint16_t mcastPort, const char* mcastAddr, int *socket_fd, struct sockaddr_in* addr_dst)
{
    //创建组播接收套接字
    *socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (*socket_fd == -1)
    {
        LOG_ERROR_FORMAT("socket creation error,%d,%s, on %s:%d", errno, strerror(errno), mcastAddr, mcastPort);
    }
    printf("create client socket %d on %s:%d\n", *socket_fd, mcastAddr, mcastPort);
    memset(addr_dst,0,sizeof(struct sockaddr_in));
    addr_dst->sin_family=AF_INET;
    addr_dst->sin_addr.s_addr=inet_addr(mcastAddr);
    addr_dst->sin_port=htons(mcastPort);    
}

void NetWork::init_mcast_server(uint16_t mcastPort, const char* mcastAddr, int* socket_fd, const char* ifname)
{
    //创建组播接收套接字
    *socket_fd = socket(AF_INET,SOCK_DGRAM,0);
    if(*socket_fd == -1)
    {
        LOG_ERROR_FORMAT("mcast socket creation error,%d,%s.",errno,strerror(errno));
    }
    printf("create mcast server socket %d on %s:%d\n", *socket_fd, mcastAddr, mcastPort);
    //设置接收套接字IP地址重用
    int opt = 1;
    if(setsockopt(*socket_fd,SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        LOG_ERROR_FORMAT("mcast set reuseaddr error,%d,%s.",errno,strerror(errno));
    }

    //设置默认绑定网卡ifname作为UDP
    struct ifreq ifr;
    memset(&ifr,0x00,sizeof(ifr));
    strncpy(ifr.ifr_name,ifname,sizeof(ifr.ifr_name));
    if(setsockopt(*socket_fd,SOL_SOCKET,SO_BINDTODEVICE,(char*)&ifr,sizeof(ifr)) == -1)
    {
        LOG_ERROR_FORMAT("mcast set %s error,%d,%s.",ifname,errno,strerror(errno));
    }

    //初始化组播IP接收地址
    memset(&m_addr.mcast_addr,0,sizeof(m_addr.mcast_addr));
    m_addr.mcast_addr.sin_family = AF_INET;
    m_addr.mcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_addr.mcast_addr.sin_port = htons(mcastPort);

    //接收套接字绑定本地IP地址
    if(bind(*socket_fd,(struct sockaddr *)&m_addr.mcast_addr,sizeof(struct sockaddr_in)) == -1)
    {
        LOG_ERROR_FORMAT("mcast bind error,%d,%s.",errno,strerror(errno));
    }

    //接收套接字加入多播组
    struct ip_mreq  mrep;
    mrep.imr_multiaddr.s_addr = inet_addr(mcastAddr);//设置组播地址
    mrep.imr_interface.s_addr = htonl(INADDR_ANY);//设置发送组播消息的源主机地址
    if(setsockopt(*socket_fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mrep,sizeof(struct ip_mreq)) == -1)//本机加入作为组播成员
    {
        LOG_ERROR_FORMAT("mcast add group error,%d,%s.",errno,strerror(errno));
    }
}

void NetWork::init_ucast_server(uint16_t ucastPort)
{
    m_sock.sock_rcv_ucast = socket(AF_INET,SOCK_STREAM,0);
    if(m_sock.sock_rcv_ucast == -1)
    {
        // perror("ucast socket creation error.");
        LOG_ERROR_FORMAT("ucast socket creation error,%d,%s.",errno,strerror(errno));
    }
    printf("create ucast server socket %d on %s:%d\n", m_sock.sock_rcv_ucast, "172.16.10.5", ucastPort);
    //设置接收套接字IP地址重用
    int opt = 1;
    if(setsockopt(m_sock.sock_rcv_ucast,SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
       LOG_ERROR_FORMAT("ucast set reuseaddr error,%d,%s.",errno,strerror(errno));
    }

    memset(&m_addr.ucast_addr,0,sizeof(m_addr.ucast_addr));
    m_addr.ucast_addr.sin_family = AF_INET;
    m_addr.ucast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_addr.ucast_addr.sin_port = htons(ucastPort);

    if(bind(m_sock.sock_rcv_ucast,(struct sockaddr*)&m_addr.ucast_addr,sizeof(m_addr.ucast_addr)) == -1)
    {
        LOG_ERROR_FORMAT("ucast bind error,%d,%s.",errno,strerror(errno));
    }

    if(listen(m_sock.sock_rcv_ucast,32) == -1)
    {
        LOG_ERROR_FORMAT("ucast listen error,%d,%s.",errno,strerror(errno));
    }
}
