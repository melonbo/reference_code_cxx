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
#include <sys/prctl.h>

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

    std::string dataFormat = "%m%n";
    std::string systemFormat = "[%D] [%p] [%m] %n";
    auto_ptr<Layout> systemLayout_general(new PatternLayout(systemFormat));
    auto_ptr<Layout> systemLayout_operate(new PatternLayout(systemFormat));
    auto_ptr<Layout> systemLayout_data(new PatternLayout(dataFormat));


    SharedAppenderPtr general_appender(new RollingFileAppender(logName_general, 1024*1024*1024,1, true));
    general_appender->setName("general");
    general_appender->setLayout(systemLayout_general);

    SharedAppenderPtr operate_appender(new RollingFileAppender(logName_operate, 1024*1024*1024,1, true));
    operate_appender->setName("operate");
    operate_appender->setLayout(systemLayout_operate);

    SharedAppenderPtr data_appender(new log4cplus::RollingFileAppender(logName_data, 1024*1024*1024, 1, true));
    data_appender->setName("data");
    data_appender->setLayout(systemLayout_data);


    logger_general = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("general"));
    logger_general.addAppender(general_appender);

    logger_operate = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("operate"));
    logger_operate.addAppender(operate_appender);

    logger_data = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("data"));
    logger_data.addAppender(data_appender);


    std::thread([&](Log4c *pthis){
        prctl(PR_SET_NAME, "logTimeCheck");
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

    strcpy(lastLogName, logName_general);
    getLogName();

    sprintf(buff, "start a new log, next is %s", logName_general);
    LOG4CPLUS_INFO(logger_general, buff);

    logger_data.removeAppender("data");
    logger_general.removeAppender("general");
    logger_operate.removeAppender("operate");


    std::string dataFormat = "%m%n";
    std::string systemFormat = "[%D] [%p] [%m] %n";
    auto_ptr<Layout> systemLayout_general(new PatternLayout(systemFormat));
    auto_ptr<Layout> systemLayout_operate(new PatternLayout(systemFormat));
    auto_ptr<Layout> systemLayout_data(new PatternLayout(dataFormat));

    SharedAppenderPtr general_appender(new RollingFileAppender(logName_general, 1024*1024*1024,1, true));
    general_appender->setName("general");
    general_appender->setLayout(systemLayout_general);

    SharedAppenderPtr operate_appender(new RollingFileAppender(logName_operate, 1024*1024*1024,1, true));
    operate_appender->setName("operate");
    operate_appender->setLayout(systemLayout_operate);

    SharedAppenderPtr data_appender(new log4cplus::RollingFileAppender(logName_data, 1024*1024*1024, 1, true));
    data_appender->setName("data");
    data_appender->setLayout(systemLayout_data);


    logger_general = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("general"));
    logger_general.addAppender(general_appender);

    logger_operate = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("operate"));
    logger_operate.addAppender(operate_appender);

    logger_data = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("data"));
    logger_data.addAppender(data_appender);

    printf("start a new log\n");
    sprintf(buff, "start a new log, last is %s", lastLogName);
    LOG4CPLUS_INFO(logger_general, buff);
}

void Log4c::getLogName()
{
    time_t time_now;
    time(&time_now);
    tm *tm_now = localtime(&time_now);
    memcpy(&tm_log, tm_now, sizeof(struct tm));
    sprintf(logName_general, "%s/%04d%02d%02d", logPath, tm_log.tm_year+1900, tm_log.tm_mon+1, tm_log.tm_mday);

    if(access(logName_general, F_OK) < 0)
        mkdir(logName_general, S_IRWXU|S_IRWXG);

    sprintf(logName_general, "%s/%04d%02d%02d/%04d%02d%02d.log", logPath,
            tm_log.tm_year+1900, tm_log.tm_mon+1, tm_log.tm_mday,
            tm_log.tm_year+1900, tm_log.tm_mon+1, tm_log.tm_mday);

    sprintf(logName_operate, "%s/%04d%02d%02d/%04d%02d%02d.operate", logPath,
            tm_log.tm_year+1900, tm_log.tm_mon+1, tm_log.tm_mday,
            tm_log.tm_year+1900, tm_log.tm_mon+1, tm_log.tm_mday);

    sprintf(logName_data, "%s/%04d%02d%02d/%04d%02d%02d.data", logPath,
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

log4cplus::Logger Log4c::getLog(char* name)
{
    if(strcmp(name,"general")==0)
        return logger_general;
    if(strcmp(name,"operate")==0)
        return logger_operate;
}
