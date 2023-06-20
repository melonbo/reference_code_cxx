#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <stdio.h>
#include <string>

#include "LogWriter/LogWriter.h"

#if defined(WIN32)

#define PRINT_INT64_FORMAT "%I64d"

#else
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

void Sleep(long mSeconds);

#define PRINT_INT64_FORMAT "%lld"

#endif

#include <QDebug>
#define OUTPUT qDebug

typedef unsigned char uchar;

class AppConfig
{
public:
    AppConfig();

    static int VERSION;
    static char VERSION_NAME[32];

    static LogWriter* gLogWriter;

    static void mkdir(char *dirName); //�����ļ���
    static void mkpath(char *path);   //�����༶�ļ���

    static void removeDir(char *dirName);
    static void removeFile(const char *filePath);

    static void copyFile(const char *srcFile, const char *destFile);

    static void replaceChar(char *string, char oldChar, char newChar); //�ַ����滻�ַ�
    static std::string removeFirstAndLastSpace(std::string &s); //�Ƴ���ʼ�ͽ����Ŀո�

    static std::string getIpFromRtspUrl(std::string rtspUrl); //��rtsp��ַ�л�ȡip��ַ

    static void mSleep(int mSecond);

    static int64_t getTimeStamp_MilliSecond(); //��ȡʱ��������룩

};

#endif // APPCONFIG_H
