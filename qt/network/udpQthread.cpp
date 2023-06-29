#include "QDataThread.h"
#include <QDebug>
#include <QProcess>

QDataThread *QDataThread::instance_dataThread = NULL;


QDataThread::QDataThread()
{
    qDebug()<<"QDataThread"<<endl;

    QProcess *add_route1 = new QProcess(this);
    QProcess *add_route2 = new QProcess(this);
    QProcess *add_route3 = new QProcess(this);

    add_route1->start("sudo route add -net 239.192.0.1 netmask 255.255.255.255 eth0");
    add_route2->start("sudo route add -net 239.192.0.2 netmask 255.255.255.255 eth0");

    sendSocket = 0;
    recvSocket = 0;
    memset(&recvAddr,0,sizeof(recvAddr));
    memset(&sendAddr,0,sizeof(sendAddr));
    memset(&localAddr,0,sizeof(localAddr));
    addr_len = 0;



}

void QDataThread::initRecvSocket(const char *multicastIP, unsigned short port)
{
    int ret = 0;
    /* 创建接收 socket 用于UDP通讯 */
    recvSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(recvSocket < 0)
    {
        qDebug()<<" socket creating error"<<endl;
        perror("socket()");
    }

    /*填充接收套接字地址结构体*/
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_addr.s_addr= htonl(INADDR_ANY);
    recvAddr.sin_port = htons(port);

    /*绑定socket*/
    ret = bind(recvSocket,(struct sockaddr*)&recvAddr, sizeof(recvAddr));
    if(ret < 0)
    {
        perror("bind()");
    }

    /*设置回环许可*/

    int loop = 0;

    ret = setsockopt(recvSocket,IPPROTO_IP, IP_MULTICAST_LOOP,&loop, sizeof(loop));
    if(ret < 0)
    {
        perror("setsockopt():IP_MULTICAST_LOOP");
    }

    struct ip_mreq mreq; /*加入广播组*/

    mreq.imr_multiaddr.s_addr    = inet_addr(multicastIP); /*广播地址*/

    mreq.imr_interface.s_addr    = htonl(INADDR_ANY); /*网络接口为默认*/


    /*将本机加入广播组*/

    ret = setsockopt(recvSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,&mreq, sizeof(mreq));
    if (ret < 0)
    {
        perror("setsockopt():IP_ADD_MEMBERSHIP");
    }
}
void QDataThread::initSendSocket(const char *multicastIP, unsigned short port)
{
    sendSocket = socket(AF_INET, SOCK_DGRAM, 0);/*建立发送套接字*/
    if(sendSocket < 0)
    {
        perror("send socket error");
    }
    sendAddr.sin_family = AF_INET;                /*设置协议族类行为AF*/
    sendAddr.sin_addr.s_addr = inet_addr(multicastIP);/*设置多播IP地址*/
    sendAddr.sin_port = htons(port);        /*设置多播端口*/

}

int QDataThread::sendToClient(unsigned char *sendBuffer, int len)
{
    int sendCnt = sendto(sendSocket,sendBuffer,len,0,(struct sockaddr*)&sendAddr,sizeof(sendAddr));
    return sendCnt;
}

int QDataThread::recvFromClient(unsigned char *recvBuffer, int len)
{
    addr_len = sizeof(recvAddr);
    int recvCnt = recvfrom(recvSocket, recvBuffer, len, 0,(struct sockaddr*)&recvAddr, &addr_len);
    return recvCnt;
}

void QDataThread::run()
{
    int recvCnt = 0;
    unsigned char dataCheck = 0;

    initSendSocket(MCAST_ADDR_SEND,SEND_PORT);
    initRecvSocket(MCAST_ADDR_RECV,RECV_PORT);

    while(1)
    {

        //sendToClient();
        recvCnt = recvFromClient(buffer,BYTE_SIZE);
        if(recvCnt == BYTE_SIZE)
        {

        }
        else
        {
            
        }

    }

}



