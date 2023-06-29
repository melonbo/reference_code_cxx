#include "http.h"
#include "stdio.h"
#include "Parser.h"
#include "config/util.h"
#include <string>
using namespace std;

HTTP::HTTP(int iLisPort)
{
    ConfigSocketParam(CONN_CNT, m_SocketInfo, m_Events);
    StartSocket(iLisPort, 1000);
}
HTTP::~HTTP()
{
}

bool HTTP::OnDataReady(int iHandle, const unsigned char *pBuffer,int iReadSizes)
{
    if(pBuffer == NULL) return false;
    string request((char *)pBuffer);
    Parser p(request);
    HTTPRequest res=p.getParseResult();
    printf("method:%s\n", res.method.data());
    printf("uri:%s\n", res.uri.data());
    printf("host:%s\n", res.host.data());
    printf("connection:%s\n", res.connection.data());
    printf("version:%s\n", res.version.data());

    if(res.uri==string("/"))
    {
        sendHomePage(iHandle);
    }
    else
    {
        int num = count(res.uri.begin(), res.uri.end(), '=');
        char *p=strtok((char*)res.uri.data(), "&");

        for(int i=0; i<num;i++)
        {
            snprintf(NVRServer.IPC[i].addr, 20, strchr(p,'=')+1);
            printf("ip-%02d:%s\n", i+1, NVRServer.IPC[i].addr);
            p=strtok(NULL, "&");
        }
        writeConfigFile("config.ini");
    }
    return true;
}

