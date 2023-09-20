//#include <QCoreApplication>
#include "log/Log4c.h"
#include <config/platform.h>
#include "onvif/CameraConfig.h"
#include <getopt.h>
#include <sstream>
#include <ctime>

#define MAX_IPC 100
//Log4c log4cForApp;
int main(int argc, char*argv[])
{
    if(argc<2) {
        printf("nead params ...\n");
        return;
    }

    static struct option longOptions[] = {
        {"probe", no_argument, nullptr, 'p'},
        {"ipaddr", required_argument, nullptr, 'i'},
        {"osd", required_argument, nullptr, 'o'},
        {"time", required_argument, nullptr, 't'},
        {nullptr, 0, nullptr, 0}  // 结束标记
    };

    int ret = 0;
    int loop_time = 1;
    std::string ipaddr;
    std::string osd;
    std::string time_string;
    bool probe_all = false;
    bool set_time = false;
    bool applicate_to_all = false;
    bool set_osd = false;
    bool set_addr = false;
    const char* optString = "ahpi:o:t:l:";
    int option = 0;
    while ((option = getopt_long(argc, argv, optString, longOptions, nullptr)) != -1) {
        switch (option) {
            case 'i':
                // 通过 optarg 获取参数值
                set_addr = true;
                ipaddr = optarg;
                break;
            case 'o':
                osd = optarg;
                set_osd = true;
                break;
            case 'a':
                applicate_to_all = true;
                probe_all = true;
                set_time = true;
                break;
            case 't':
                time_string = optarg;
                set_time = true;
                break;
            case 'p':
                probe_all = true;
                break;
            case 'l':
                loop_time = std::stoi(optarg);
                break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [--ipaddr <ip>] [--probe] [--osd <text>] [--time <2000-00-00 00:00:00>]" << std::endl;
                return 0;
            case '?':
                // 无法识别的选项或缺少参数时，会返回 '?'，可以根据需要进行处理
                std::cout << "Unknown option or missing argument: " << static_cast<char>(optopt) << std::endl;
                return 1;
        }
    }

    CameraConfig *configCamera[MAX_IPC];

    for(int i=0; i<30; i++)
        configCamera[i] = new CameraConfig(i);

    if(set_addr){
        *configCamera[0] = new CameraConfig(0);
        configCamera[0]->init(ipaddr.data(), "admin", "waycom12345");
    }

    IP *ip_sum;
    int num = 30;

    if(probe_all){
        ret = camera_findAll(&ip_sum, &num);
        printf("find %d camera: \n", ret);
        for(int i=0; i<ret; i++){
            *configCamera[i] = new CameraConfig(i);
            printf("%s\n", ip_sum[i].ip);
        }
    }


    if(set_time){
        std::tm timeStruct = {};

        if(time_string.empty()){
            time_t time_t_now;
            struct tm* tm_now;
            time(&time_t_now);
            time_t_now -= 8*60*60;
            tm_now = localtime(&time_t_now);

            timeStruct.tm_year = tm_now->tm_year + 1900;
            timeStruct.tm_mon = tm_now->tm_mon + 1;
            timeStruct.tm_mday = tm_now->tm_mday;
            timeStruct.tm_hour = tm_now->tm_hour;
            timeStruct.tm_min = tm_now->tm_min;
            timeStruct.tm_sec = tm_now->tm_sec;
        }
        else{
            std::istringstream iss(time_string);
            char delimiter;
            // 解析年份、月份、日期、小时、分钟和秒钟
            iss >> timeStruct.tm_year >> delimiter
                >> timeStruct.tm_mon  >> delimiter
                >> timeStruct.tm_mday >> delimiter
                >> timeStruct.tm_hour >> delimiter
                >> timeStruct.tm_min  >> delimiter
                >> timeStruct.tm_sec;
        }

        printf("set camera time: %04d-%02d-%02d %02d:%02d:%02d\n", timeStruct.tm_year, timeStruct.tm_mon, timeStruct.tm_mday, timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec);


        if(applicate_to_all){
            for(int i=0; i<num; i++){
                configCamera[i]->init(ip_sum[i].ip, "admin", "waycom12345");
                int loop_time_tmp = loop_time;

                while(loop_time_tmp){
                    loop_time_tmp -= 1;
                    ret = configCamera[i]->getCameraInfo();
                    if(ret == CAMERA_OK) break;
                }

                loop_time_tmp = loop_time;
                while(loop_time_tmp){
                    loop_time_tmp -= 1;
                    ret = configCamera[i]->setSystemDateAndTime(timeStruct.tm_year, timeStruct.tm_mon, timeStruct.tm_mday, timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec);
                    if(ret==CAMERA_OK) break;
                }
            }
        }else{
            ret = configCamera[0]->getCameraInfo();
            if(ret != CAMERA_OK) return;

            while(loop_time){
                loop_time -= 1;
                ret = configCamera[0]->setSystemDateAndTime(timeStruct.tm_year, timeStruct.tm_mon, timeStruct.tm_mday, timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec);
                if(ret==CAMERA_OK) break;
            }
        }
    }

    if(set_osd){
        if(applicate_to_all){
            for(int i=0; i<num; i++)
            {
                int loop_time_tmp = loop_time;
                while(loop_time_tmp){
                    loop_time_tmp -= 1;
                    ret = configCamera[i]->setCameraAlarmOsd(osd);
                    if(ret == CAMERA_OK) break;
                }
            }
        }else{
            int loop_time_tmp = loop_time;
            while(loop_time_tmp){
                loop_time_tmp -= 1;
                configCamera[0]->setCameraAlarmOsd(osd);
            }
        }
    }
}


//    Camera myCamera;
//    myCamera.init("172.16.41.1", "admin", "waycom12345");
//    myCamera.init("172.16.41.250", "admin", "admin123");

//    int ret = myCamera.getCameraInfo();

//    myCamera.setMainReslution(1280, 720);
//    myCamera.setSubReslution(640, 480);

//    myCamera.setMainFrameRateLimit(15);
//    myCamera.setSubFrameRateLimit(15);

//    myCamera.setMainBitrateLimit(1024);
//    myCamera.setSubBitrateLimit(1024);

//    myCamera.setMainFrameGovLength(15);
//    myCamera.setSubFrameGovLength(15);

//    myCamera.setVideoConfig(MAINSTREAM);
//    myCamera.setVideoConfig(SUBSTREAM);

/****************find all camera***************/
//    IP *ip_sum;
//    int num = 30;
//    camera_findAll(&ip_sum, &num);
/**********************************************/

//    myCamera.setSystemDateAndTime(2019, 06, 6, 00, 00, 00);


//    ret = myCamera.getOSDOption();
//    ret = myCamera.getOSDs();
//    myCamera.deleteOSD();
//    myCamera.createOSD();

//    myCamera.getSnapshot();

