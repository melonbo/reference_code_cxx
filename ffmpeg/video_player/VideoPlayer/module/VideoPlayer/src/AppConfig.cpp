#include "AppConfig.h"

#if defined(WIN32)
#include <windows.h>
#include <direct.h>
#include <io.h> //C (Windows)    access
#else
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

void Sleep(long mSeconds)
{
    usleep(mSeconds * 1000);
}

#endif

int AppConfig::VERSION = 1;
char AppConfig::VERSION_NAME[32] = "2.1.1";

LogWriter* AppConfig::gLogWriter = new LogWriter;

AppConfig::AppConfig()
{

}

void AppConfig::mkdir(char *dirName)
{
#if defined(WIN32)
    ///���Ŀ¼������ �򴴽�
    if (access(dirName, 0)!=0)
    {
        _mkdir(dirName);
    }
#else
    ///���Ŀ¼������ �򴴽�
    if (access(dirName, R_OK)!=0)
    {
        char cmd[128] = {0};
        sprintf(cmd,"mkdir %s", dirName);
        system(cmd);
    }
#endif
}

void AppConfig::mkpath(char *path)
{
#if defined(WIN32)
        ///windows�����ļ������� ·�����Ƿ�б�� ���������Ҫ�滻һ��
        char dirPath[128] = {0};
        strcpy(dirPath, path);

        AppConfig::replaceChar(dirPath, '/', '\\');

        ///���Ŀ¼������ �򴴽���
        if (access(dirPath, 0)!=0)
        {
    //        _mkdir(dirPath);
            char cmd[128];
            sprintf(cmd,"mkdir %s", dirPath);
            system(cmd);
        }

#else
    ///���Ŀ¼������ �򴴽���
    if (access(path,R_OK)!=0)
    {
        char cmd[128];
        sprintf(cmd,"mkdir %s -p",path);
        system(cmd);
    }
#endif
}

void AppConfig::removeDir(char *dirName)
{
    if (strlen(dirName) <= 0) return;

    if (access(dirName, 0) != 0 ) //�ļ��в�����
    {
        return;
    }

#if defined(WIN32)

    ///ɾ�������ļ�
    char cmd[128];
    sprintf(cmd,"rd /s /q \"%s\"", dirName);
    system(cmd);

#else

    char cmd[128];
    sprintf(cmd,"rm -rf \"%s\"",dirName);
    system(cmd);

#endif
}

void AppConfig::removeFile(const char *filePath)
{
    if (filePath == NULL || strlen(filePath) <= 0) return;

#if defined(WIN32)

        ///ɾ�������ļ�
        remove(filePath);

#else
        ///ɾ�������ļ�
        char cmd[128] = {0};
        sprintf(cmd,"rm -rf \"%s\"",filePath);
        system(cmd);
#endif
}

void AppConfig::copyFile(const char *srcFile, const char *destFile)
{

#if defined(WIN32)
        CopyFileA(srcFile, destFile, FALSE);
#else

        ///���ļ�������Զ�˷�����
        char copyfilecmd[512];
        sprintf(copyfilecmd,"cp \"%s\" \"%s\"", srcFile, destFile);
        system(copyfilecmd);

#endif
}

void AppConfig::replaceChar(char *string, char oldChar, char newChar)
{
    int len = strlen(string);
    int i;
    for (i = 0; i < len; i++){
        if (string[i] == oldChar){
            string[i] = newChar;
        }
    }
}


std::string AppConfig::removeFirstAndLastSpace(std::string &s)
{
    if (s.empty())
    {
        return s;
    }
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

void AppConfig::mSleep(int mSecond)
{
#if defined(WIN32)
    Sleep(mSecond);
#else
    usleep(mSecond * 1000);
#endif
}

int64_t AppConfig::getTimeStamp_MilliSecond()
{

    int mSecond = 0; //��ǰ������

#if defined(WIN32)

    SYSTEMTIME sys;
    GetLocalTime( &sys );

    mSecond = sys.wMilliseconds;

#else

    struct timeval    tv;
    struct timezone tz;

    struct tm         *p;

    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    mSecond = tv.tv_usec / 1000;


#endif

    int64_t timeStamp = ((int64_t)time(NULL)) * 1000 + mSecond;

    return timeStamp;

}
