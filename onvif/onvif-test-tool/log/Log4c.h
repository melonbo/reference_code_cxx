/*
 * Log4c.h
 *
 *  Created on: 2013-10-24
 *      Author: root
 */

#ifndef LOG4C_H_
#define LOG4C_H_

#include "log/log4cplus/logger.h"
#include "log/log4cplus/fileappender.h"
#include "log/log4cplus/layout.h"

#include "log/log4cplus/loggingmacros.h"

#include "log/log4cplus/loglevel.h"
#include "log/log4cplus/ndc.h"
#include "log/log4cplus/configurator.h"

using namespace log4cplus;
using namespace log4cplus::helpers;


class Log4c {
public:
	Log4c();
	virtual ~Log4c();

public:
    void StartLog(char* path);
    void StartNewLog();
    void getLogName();
    bool isNewDay();

   void StartSystem(const char* properties_filename);
   void StopSystem();

   tm tm_log;
   int mday;
   char logPath[128];
   char logName[1024];

public:

   void WriteLog_Debug(const char *debug);
   void WriteLog_Info(const char *info);
   void Debug(const char* filename, const int fileline, const char* pFormat,... );

    void Error(const char* filename, const int fileline, const char* pFormat,... );

    void Fatal(const char* filename, const int fileline, const char* pFormat,... );

    void Info(const char* filename, const int fileline, const char* pFormat,... );

    void Warn(const char* filename, const int fileline, const char* pFormat,... );

    void Trace(const char* filename, const int fileline, const char* pFormat,...);

};

#endif /* LOG4C_H_ */
