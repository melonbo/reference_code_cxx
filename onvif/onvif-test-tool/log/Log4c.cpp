/*
 * Log4c.cpp
 *
 *  Created on: 2013-10-24
 *      Author: root
 */

#include "Log4c.h"
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <thread>

using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace std;


#define LOG4CPLUS_HAVE_RVALUE_REFS

Log4c::Log4c()
{
	//StartLog();
}

Log4c::~Log4c()
{
	// TODO Auto-generated destructor stub
}


void Log4c::StartLog(char* path)
{
    if(!path) return;
    if(access(path, F_OK) < 0)
        mkdir(path, S_IRWXU|S_IRWXG);
    strcpy(logPath, path);

    getLogName();

    SharedAppenderPtr system(new RollingFileAppender(logName, 1024*1024*1024,1, true));
	system->setName("file system");
    std::string systemFormat = "[%D] [%p] [%m] %n";
	auto_ptr<Layout> systemLayout(new PatternLayout(systemFormat));
	system->setLayout(systemLayout);

    Logger::getRoot().addAppender(system);

    std::thread([&](Log4c *pthis){
        while(1)
        {
            if(pthis->isNewDay())
                pthis->StartNewLog();
            sleep(1);
        }
    }, this).detach();
}

void Log4c::StartNewLog()
{
    char lastLogName[1024];
    char buff[1024];

    strcpy(lastLogName, logName);
    getLogName();

    sprintf(buff, "start a new log, next is %s", logName);
    LOG4CPLUS_INFO(Logger::getRoot(), buff);

    Logger::getRoot().removeAppender("file system");


    SharedAppenderPtr system(new RollingFileAppender(logName, 500*1024*1024,1, true));
    system->setName("file system");
    std::string systemFormat = "[%D] [%p] [%m] %n";
    auto_ptr<Layout> systemLayout(new PatternLayout(systemFormat));
    system->setLayout(systemLayout);

    Logger::getRoot().addAppender(system);
    printf("start a new log\n");
    sprintf(buff, "start a new log, last is %s", lastLogName);
    LOG4CPLUS_INFO(Logger::getRoot(), buff);
}

void Log4c::getLogName()
{
    time_t time_now;
    time(&time_now);
    tm *tm_now = localtime(&time_now);
    memcpy(&tm_log, tm_now, sizeof(struct tm));
    sprintf(logName, "%s/%04d%02d%02d", logPath, tm_log.tm_year+1900, tm_log.tm_mon+1, tm_log.tm_mday);

    if(access(logName, F_OK) < 0)
        mkdir(logName, S_IRWXU|S_IRWXG);

    sprintf(logName, "%s/%04d%02d%02d/%04d%02d%02d.log", logPath,
            tm_log.tm_year+1900, tm_log.tm_mon+1, tm_log.tm_mday,
            tm_log.tm_year+1900, tm_log.tm_mon+1, tm_log.tm_mday);
}

bool Log4c::isNewDay()
{
    time_t time_now;
    time(&time_now);
    tm *tm_now = localtime(&time_now);
    return !(tm_log.tm_mday == tm_now->tm_mday);
}
