#ifndef CAMERACONFIG_H
#define CAMERACONFIG_H
//#define PRINTF
//#include "camera_global.h"
//#include <openssl/bio.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>


#define MULTICAST_ADDRESS "soap.udp://239.255.255.250:3702/"

#define CAMERA_OK  0                            //无错误
#define CAMERA_DEVICENOTONLINE   1              //当前设备不再线
#define CAMERA_UNAUTHORZATION  2                //鉴权失败
#define CAMERA_GETCAPABILICITYERROR 3           //获取设备能力失败
#define CAMERA_GETPROFILEERROR 4                //获取设备配置失败
#define CAMERA_GETSTREAMURIERROR 5              //获取stream uri失败
#define CAMERA_GETCONFIGURATIONERROR 6          //获取设备编码参数失败
#define CAMERA_GETCONFIGURATIONOPTIONERROR 7    //获取设备编码参数范围失败
#define CAMERA_SETCONFIGURATIONERROR 8          //设置设备编码参数失败
#define CAMERA_GETSYSTEMDATEANDTIMEERROR 9
#define CAMERA_SETSYSTEMDATEANDTIMEERROR 10
#define CAMERA_GETOSDOPTIONSERROR 11
#define CAMERA_GETOSDSERROR 12
#define CAMERA_GETOSDERROR 13

enum STREAMNUM{
    MAINSTREAM = 0,
    SUBSTREAM
};

//将IP地址封装进结构体，这样做的好处是可以动态产生多个IP
typedef struct
{
    char ip[32];
}IP;
//配置文件代号和编码参数代号
typedef struct
{
    char profileToken[32];
    char configToken[32];
    char videoSourceToken[32];
    char createOSDToken[32];
}TOKEN;
//分辨率
typedef struct
{
    int Width;
    int Height;

}RESOLUTION;
//码率参数
typedef struct
{
    int FrameRateLimit;
    int EncodingInterval;
    int BitRateLimit;

}RATECONTROL;
//码率参数范围
typedef struct
{
    int FrameRateLimit_max;
    int FrameRateLimit_min;

    int EncodingInterval_max;
    int EncodingInterval_min;

    int BitRateLimit_max;
    int BitRateLimit_min;
}RATECONTROL_OPTION;
//摄像机编码参数
typedef struct
{
    char profileName[32];
    char configurationName[32];
    char streamUri[128];
    RESOLUTION resolution;
    RATECONTROL RateControl;
    int GovLength;
}CAMERA_CONFIGURATION;
//摄像机编码参数范围
typedef struct
{
    char profileName[32];
    char configurationName[32];
    int resolutionSize;
    RESOLUTION *resolution;
    RATECONTROL_OPTION ratecontrol_option;
}CAMERA_CONFIGURATIONOPTION;
//摄像机信息集合，与用户交互的所有摄像机参数都集中在该结构体中
typedef struct
{
    int camera_configurationSize;
    CAMERA_CONFIGURATION* camera_configuration;
    CAMERA_CONFIGURATIONOPTION* camera_configurationOption;        
}CAMERA_INFO;

class  CameraConfig
{

public:
    CameraConfig();
    ~CameraConfig();
    /*******************************用户接口*******************************/
    /*初始化摄像机参数，如果没有传入用户名和密码，则认为不需要鉴权*/
    int init(char *IPAddr, char* UserName, char* Password);
    int init(char* IPAddr);

    /* 检查设备是否在线，返回CAMERA_OK表示设备在线，返回CAMERA_DEVICENOTONLINE
    * 表示设备不再线，用户可循环调用该函数来判断摄像机在线状态，但是建议两次调用之间
    * 相隔适当时间（1秒左右最佳）。此外，如果摄像机不再线，则该函数会阻塞1秒来等待
    * 设备的回复，因此，最好不要在实时性要求较高的线程中循环调用该函数
    */
    int checkDevice();

    /* 获取摄像机信息，调用该函数后，可以获取到摄像机的各项参数。取得的参数会保存在
     * camera_info变量中
     */
    int getCameraInfo();

    /* 设置摄像机的各项参数，调用该函数之前必须至少调用过一次camera_getCameraInfo()
     * 函数，以保证camera_info变量非空。该函数会根据当前camera_info的内容来配置摄像机
     * 各参数。因此，如需修改摄像机某一项参数，应线首先修改camera_info中的对应参数，然后
     * 调用该函数即可。此处应注意：使用该函数修改摄像机分辨率时，该函数会阻塞5秒钟左右，
     * 产生阻塞的原因在于摄像机，而非本程序。
     */
    int setVideoConfig(int num);

    int  setSystemDateAndTime(int sysYear, int sysMonth, int sysDay, int sysHour, int sysMinute, int sysSecond);
    void setMainReslution(int width, int height);
    void setSubReslution(int width, int height);
    void setMainBitrateLimit(int rate);
    void setSubBitrateLimit(int rate);
    void setMainFrameRateLimit(int rate);
    void setSubFrameRateLimit(int rate);
    void setMainFrameGovLength(int len);
    void setSubFrameGovLength(int len);
    void setOSDString(char *str);

    int getOSDOption();
    int getOSDs();
    int getOSD();
    int createOSD();
    int deleteOSD();
    int getSnapshot();
    int make_uri_withauth(char *src_uri, char *username, char *password, char *dest_uri, unsigned int size_dest_uri);

    int find_event(struct _tev__PullMessagesResponse *rep, const char *topic, const char *name, const char *value);
    int isMotion(struct _tev__PullMessagesResponse *rep);
    int ONVIF_CreatePullPointSubscription(const char *EventXAddr);
    int ONVIF_GetEventProperties(const char *EventXAddr);
    int ONVIF_GetServiceCapabilities(const char *EventXAddr);
    void cb_discovery(char *DeviceXAddr);

    void startSetCameraTime();
    void setCameraTime();
    void setAlarm(bool flag, int type);
    void startSetCameraAlarmOsd();
    void setCameraAlarmOsd();

    int isAlarm;
    int isAlarmVideoList;
    int alarm_type;//bit0:emergency alarm, bit1:pre fire, bit2:fire, bit3:door unlock bit4:door close block bit5:door open block
    int alarm_type_old;

    /*错误代码*/
    int soap_error_number;

    /*摄像机各项参数集合*/
    CAMERA_INFO camera_info;

    bool camera_isOnline;
    /*******************************************************************************/
    char camera_mediaAddr[128];
    char camera_eventAddr[128];
private:
    bool needAuthorization;
    char camera_IP[32];
    char camera_userName[64];
    char camera_password[64];
    char camera_serviceAddr[128];
    unsigned char camera_channel;


    TOKEN *camera_token;
    char stringAddToOsd[128];
    char snapshotUri[128];
    char osdToken[128];


    int getCapabilities();
    int getProfiles();
    int getStreamUri();
    int getVideoConfigOption();
    const char* soap_wsa_rand_uuid(struct soap* soap);
    void getIPFromXAddr(const char* XAddr, char* DeviceIP, unsigned short* DevicePort);
    int getSystemDateAndTime();

};
/*该函数的作用为获取所有当前在线的摄像机的IP
 * 该函数第二个参数为用户传入的摄像机数量值，如果实际发现的摄像机数量小于这一值
 * 则函数会阻塞5秒钟，以确保已经发现所有摄像机
 * 如果实际发现的摄像机数量等于这一值，则函数立刻返回，
 * 如果实际发现的摄像机数量大于这一值，用户只能得到等于deviceCount
 * 个摄像机的IP，其余的将会被丢弃。
 * 获取到的所有摄像机的IP地址将会通过all_ip指针返回给用户。
 * 注意：该函数会为all_ip指针分配内存空间来保存IP地址，因此用户
 * 只需要传递一个空指针的地址即可，否则会造成内存泄漏
 */
int camera_findAll(IP** all_ip, int* deviceCount);
#endif // CAMERACONFIG_H
