#include "localClient.h"
#include <sys/prctl.h>

LocalClient::LocalClient()
{

}

int LocalClient::initSendSocket()
{
    server_fd=socket(AF_UNIX,SOCK_STREAM,0);
    if(server_fd<0)
    {
        perror("cannot create communication socket");
        return -1;
    }

    //connect server
    server_addr.sun_family=AF_UNIX;
    strcpy(server_addr.sun_path,send_domain);
    return 0;
}

int LocalClient::sendData(char* data, int len)
{
    initSendSocket();
    printf("send data to %s\n", send_domain);
    int ret=connect(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr));
    if(ret==-1)
    {
        perror("cannot connect to the server");
        close(server_fd);
        return -1;
    }

    write(server_fd,data,len);
    close(server_fd);
    return 0;
}

int LocalClient::initReceiveSocket()
{
    int ret;
    listen_fd=socket(AF_UNIX,SOCK_STREAM,0);
    if(listen_fd<0)
    {
        perror("cannot create communication socket");
        return -1;
    }
    //set server addr_param
    local_addr.sun_family=AF_UNIX;
    strncpy(local_addr.sun_path,recv_domain,sizeof(local_addr.sun_path)-1);
    unlink(recv_domain);
    //bind sockfd & addr
    ret=bind(listen_fd,(struct sockaddr*)&local_addr,sizeof(local_addr));
    if(ret==-1)
    {
        perror("cannot bind server socket");
        close(listen_fd);
        unlink(recv_domain);
        return -1;
    }
    //listen sockfd
    ret=listen(listen_fd,1);
    if(ret==-1)
    {
        perror("cannot listen the client connect request");
        close(listen_fd);
        unlink(recv_domain);
        return -1;
    }

    return listen_fd;
}

int LocalClient::receiveData()
{
    printf("start listen ... ...\n");
    int len=sizeof(client_addr);
    prctl(PR_SET_NAME, "previewReceiveData");
    while(1)
    {
        //have connect request use accept
        client_fd=accept(listen_fd,(struct sockaddr*)&client_addr,(socklen_t*)&len);
        printf("client fd is %d\n", client_fd);
        if(client_fd<0)
        {
            printf("cannot accept client connect request, %d\n", client_fd);
            usleep(1000);
            continue;
        }
        //read and printf sent client info
        memset(recv_buf,0,1024);
        int num=read(client_fd,recv_buf,sizeof(recv_buf));
        printf("read %d bytes from client\n",num);
        if(num>0)
            pDataDealCallBack(recv_buf, num);
        close(client_fd);
    }
    return 0;
}

void LocalClient::dealReceiveData(char *data, int size)
{
    if(size >=8)
    {

    }

//    tagFrame *recv_frame = (tagFrame*)recv_buf;
//    for(int i=0; i<sizeof(tagFrame); i++)
//    {
//        printf("%02x ", recv_buf[i]);
//    }
//    printf("\n");

//    if(recv_frame->head_01 == 0xff && recv_frame->tail == 0xfe)
//    {
//        preview.setPlayChannel(recv_frame->mode, recv_frame->side, recv_frame->ipc_01, recv_frame->ipc_02, recv_frame->ipc_03, recv_frame->ipc_04);
//    }
}

void LocalClient::setCallBackFunc(DataDealCallBack callback)
{
    pDataDealCallBack = callback;
}
