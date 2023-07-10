#include "time_utils.h"


void wrapper_time_count(void(*fun)())
{
    auto start = high_resolution_clock::now();

    fun();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Time taken by function: " << duration.count() << " microseconds" << endl;
}

void wrapper_time_count(void(*fun)(char*, char*), char* str1, char* str2)
{
    auto start = high_resolution_clock::now();

    fun(str1, str2);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Time taken by function: " << duration.count() << " microseconds" << endl;
}

void getSysTime(char* year, char* mon, char* day, char* hour, char* min, char* sec)
{
    time_t timer;
    struct tm* t_tm;
    time(&timer);
    t_tm = localtime(&timer);
    *year = t_tm->tm_year+1900-2000;
    *mon  = t_tm->tm_mon+1;
    *day  = t_tm->tm_mday;
    *hour = t_tm->tm_hour;
    *min  = t_tm->tm_min;
    *sec  = t_tm->tm_sec;
}

unsigned int getSystemSeconds()
{
    time_t timer;
    time(&timer);
    return timer;
}

unsigned int getFormatSeconds(char year, char mon, char day, char hour, char min, char sec)
{
    struct tm formatTime;
    formatTime.tm_year = year + 2000 - 1900;
    formatTime.tm_mon = mon - 1;
    formatTime.tm_mday = day;
    formatTime.tm_hour = hour;
    formatTime.tm_min = min;
    formatTime.tm_sec = sec;

    return mktime(&formatTime);
}

void printTime()
{
    time_t timer;
    struct tm* t_tm;
    time(&timer);
    t_tm = localtime(&timer);
    printf("%4d-%02d-%02d %02d:%02d:%02d\n",
           t_tm->tm_year+1900, t_tm->tm_mon+1, t_tm->tm_mday,
           t_tm->tm_hour, t_tm->tm_min, t_tm->tm_sec);
}
