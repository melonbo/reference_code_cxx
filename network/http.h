#ifndef HTTP_H
#define HTTP_H
#include "CTcpServerSocket.h"
#define CONN_CNT 5

class HTTP:public CTcpServerSocket{
public:
    HTTP(int iLisPort);
    ~HTTP();
private:
    tagClientSocketInfo m_SocketInfo[CONN_CNT];
    struct epoll_event m_Events[CONN_CNT];
    bool OnDataReady(int iHandle, const unsigned char *pBuffer, int iReadSizes);
};



#endif // HTTP_H
