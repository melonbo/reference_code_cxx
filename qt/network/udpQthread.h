#ifndef QDATATHREAD_H
#define QDATATHREAD_H

#include <QObject>
#include <QThread>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>


class QDataThread : public QThread
{
    Q_OBJECT
private:
    QDataThread();//

    //add
public:
    void run();

    static QDataThread *getInstance()
    {
        if(instance_dataThread == NULL)
        {
            instance_dataThread = new QDataThread();

        }
        return instance_dataThread;
    }
private:
    static QDataThread* instance_dataThread;

    QDataLgoic *pDataLogic;
    int sendSocket;
    int recvSocket;

     struct sockaddr_in recvAddr, sendAddr,localAddr; /*发送 接收  地址结构体*/
      socklen_t addr_len;
    //add by test

    int  test;

private:
    void initRecvSocket(const char *multicastIP, unsigned short port);
    void initSendSocket(const char *multicastIP, unsigned short port);
    int recvFromClient(unsigned char *recvBuffer, int len);
    int sendToClient(unsigned char *sendBuffer, int len);
signals:
    void getdata( unsigned char *_data);

};

#endif // QDATATHREAD_H
