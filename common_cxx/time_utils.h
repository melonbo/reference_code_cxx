#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <iostream>
#include <chrono>
#include <QThread>
#include <time.h>
#include <ctime>

using namespace std;
using namespace std::chrono;

void wrapper_time_count(void(*fun)());//计算函数运行时间
void wrapper_time_count(void(*fun)(char*, char*), char* str1, char* str2);//计算函数运行时间
void getSysTime(char* year, char* mon, char* day, char* hour, char* min, char* sec);//获取系统时间
unsigned int getSystemSeconds();//获取系统时间,1970年到现在的秒数
unsigned int getFormatSeconds(char year, char mon, char day, char hour, char min, char sec);//格式时间转换为秒数
void printTime();//打印系统时间,yyyy-mm-dd hh:mm:ss
#endif // TIME_UTILS_H
