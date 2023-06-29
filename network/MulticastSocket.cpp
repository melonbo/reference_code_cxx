#include "MulticastSocket.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/prctl.h>

char test_data[] = {0x7e, 0x81, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0c, 0x02, 0x00, 0x01, 0xac, 0x10, 0x07, 0x01, 0x15, 0x05, 0x0d, 0x0a, 0x21, 0x1e, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x14, 0x01, 0x01, 0x41, 0x60, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbb, 0x00, 0x00, 0xbc, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe};

CMulticastSocket::CMulticastSocket() : m_commonThread(this)
{
    time_done = 0;
    pthread_mutex_init(&lock, NULL);
}

CMulticastSocket::~CMulticastSocket()
{

}

void *readSocket(void *p)
{
    CMulticastSocket* pthis = (CMulticastSocket*)p;
    unsigned char line[1024+1] = {0};
    int addrlen_src = sizeof(pthis->addr_src);
    char IPdotdec[20];
    char thread_name[128];
    sprintf(thread_name, "readSocket_%d", pthis->m_portReceiv);
    prctl(PR_SET_NAME, thread_name);
    while(1)
    {
        int ret = recvfrom(pthis->fd_recv, line, sizeof(line),0, (struct sockaddr *) &pthis->addr_src,(socklen_t*)&addrlen_src);
        inet_ntop(AF_INET, (void *)&(pthis->addr_src.sin_addr), IPdotdec, 16);
        printf("\n************************************* \n");
        mprintf("read %d bytes from %s \n", ret, IPdotdec);
        if(ret > 0)
        {
            line[ret] = '\0';
            pthis->dealRecvData(line, ret);
            mprintf("deal data %s end", IPdotdec);
            dprintf(line, ret);
        }
        else
        {
            mprintf("receive multicast error, errno = %d\n", errno);
        }
    }
}

void *sendSocket(void *p)
{
    CMulticastSocket* pthis = (CMulticastSocket*)p;
    static int sendNum = 0;
    int addrlen_dst = sizeof(pthis->addr_dst);    
    prctl(PR_SET_NAME, "sendSocket");
    while(1)
    {
        sleep(1);
    
        /****************************发送数据*******************************/
        printf("send data to %s:%d\n", MULTICAST_SEND_IP_ADDR_NVR, MULTICAST_SEND_IP_PORT_NVR);
        int ret = sendto(pthis->fd_send, test_data, sizeof(test_data), 0, (struct sockaddr *) &pthis->addr_dst, addrlen_dst);
    }
}

int CMulticastSocket::CreateReceivingSocket(char* strGroupIP, unsigned int nGroupPort)
{
    struct ip_mreq mreq;
    m_portReceiv = nGroupPort;
    u_int yes=1; /*** MODIFICATION TO ORIGINAL */

    /* create what looks like an ordinary UDP socket */
    if ((fd_recv=socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
            perror("socket");
            exit(1);
    }

    /**** MODIFICATION TO ORIGINAL */
    /* allow multiple sockets to use the same PORT number */
    if (setsockopt(fd_recv,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0)
    {
            perror("Reusing ADDR failed");
            exit(1);
    }
    /*** END OF MODIFICATION TO ORIGINAL */

    /* set up destination address */
    memset(&addr_src,0,sizeof(addr_src));
    addr_src.sin_family=AF_INET;
    addr_src.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
    addr_src.sin_port=htons(nGroupPort);

    /* bind to receive address */
    if (bind(fd_recv,(struct sockaddr *) &addr_src,sizeof(addr_src)) < 0)
    {
            perror("bind");
            exit(1);
    }

    /* use setsockopt() to request that the kernel join a multicast group */
    mreq.imr_multiaddr.s_addr=inet_addr(strGroupIP);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (setsockopt(fd_recv,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0)
    {
            perror("setsockopt");
            exit(1);
    }

    return pthread_create(&pth_recv, NULL, readSocket, this);

}

int CMulticastSocket::CreateReceivingSocket(char* strGroupIP, unsigned int nGroupPort, int *fd_recv, pthread_t *pth_recv , void *callback)
{
    struct ip_mreq mreq;

    u_int yes=1; /*** MODIFICATION TO ORIGINAL */

    /* create what looks like an ordinary UDP socket */
    if ((*fd_recv=socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
            perror("socket");
            exit(1);
    }

    /**** MODIFICATION TO ORIGINAL */
    /* allow multiple sockets to use the same PORT number */
    if (setsockopt(*fd_recv,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0)
    {
            perror("Reusing ADDR failed");
            exit(1);
    }
    /*** END OF MODIFICATION TO ORIGINAL */

    /* set up destination address */
    memset(&addr_src,0,sizeof(addr_src));
    addr_src.sin_family=AF_INET;
    addr_src.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
    addr_src.sin_port=htons(nGroupPort);

    /* bind to receive address */
    if (bind(*fd_recv,(struct sockaddr *) &addr_src,sizeof(addr_src)) < 0)
    {
            perror("bind");
            exit(1);
    }

    /* use setsockopt() to request that the kernel join a multicast group */
    mreq.imr_multiaddr.s_addr=inet_addr(strGroupIP);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);

    if (setsockopt(*fd_recv,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0)
    {
            perror("setsockopt");
            exit(1);
    }

    return pthread_create(pth_recv, NULL, callback, this);
}

int CMulticastSocket::CreateSendingSocket(char* strGroupIP, unsigned int nGroupPort)
{
    /* create what looks like an ordinary UDP socket */
    if ((fd_send=socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
        mprintf("send udp socket error %d\n", errno);
    }

    /* set up destination address */
    memset(&addr_dst,0,sizeof(addr_dst));
    addr_dst.sin_family=AF_INET;
    addr_dst.sin_addr.s_addr=inet_addr(strGroupIP);
    addr_dst.sin_port=htons(nGroupPort);

    return pthread_create(&pth_send, NULL, sendSocket, this);

}

void CMulticastSocket::ThreadProc()
{

}

void CMulticastSocket::dealRecvData(unsigned char* buf, int size)
{
    printf("receive data device type is %d\n", buf[8]);
}

void CMulticastSocket::StartSocket()
{
    m_commonThread.start(256*1024);
}

bool CMulticastSocket::IsRun() {
        return m_commonThread.isRun();
}

