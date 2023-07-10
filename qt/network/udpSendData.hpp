void sendDataToDebugPort(const char *fmt, ...)
{
    char buf[1024];
    va_list args;
    int n;

    va_start(args, fmt);
    n = vsnprintf(buf, BUFSIZE, fmt, args);
    va_end(args);

    if(buf[strlen(buf)-1] == '\n')
        buf[strlen(buf)-1] = '\0';

    printf("%s\n", buf);

    QUdpSocket *mSocket = new QUdpSocket();
    int ret = mSocket->writeDatagram(buf, strlen(buf), QHostAddress("172.16.42.229"), 6677);
    printf("send data %d\n", ret);
}

void sendDataToDebugPort(int len, char *data)
{
    QUdpSocket *mSocket = new QUdpSocket();
    int ret = mSocket->writeDatagram(data, len, QHostAddress("172.16.42.229"), 6677);
    delete mSocket;
    printf("send data %d\n", ret);
}