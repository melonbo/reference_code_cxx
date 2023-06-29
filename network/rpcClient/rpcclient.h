#ifndef RPCCLIENT_H
#define RPCCLIENT_H
#include <QObject>
#include "QDebug"
#include "QSemaphore"
#include "QTime"
#include "littlerpc/little_rpc.h"
using namespace little_rpc;

struct Window{
    int x;
    int y;
    int width;
    int height;
    Window(){

    }
    Window(int x, int y, int width, int height){
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
    }
};

class rpcclient : public RpcClientCallback
{
public:
    rpcclient();
    void Play(QString str_url, int loop = 0);

    /**
    * @brief StartCameraStream
    *
    * @param dst_ipaddress dst_ipaddress
    *            172.16.7.230
    * @param argc  setting parameters it could be NULL
    *
    */
        void StartCameraStream();

    /**
    * @brief StopCameraStream
    *
    * @param dev_name v4l2 device name
    *            /dev/video0
    * @param argc  setting parameters it could be NULL
    *
    */
        void StopCameraStream();

        void Pause(bool clear_flag);

        void Resume();

    /**
    * @brief PlayCamera 播放CAMERA
    *
    * @param url camera ip:port
    *            172.16.7.230:5000
    *
    */
        void PlayCamera(const char*ipaddr);

    /**
    * @brief Stop 停止播放
    *
    *
    */
        void Stop();

        /*
         * 设置播放窗口位置，注意窗口大小不能大于视频本身分辨率。
    */
    /**
    * @brief SetWindow 设置视频播放窗口
    *
    * @param w Window类型的窗口
    */
        void SetWindow(Window* w);

    /**
    * @brief SetEnableDisplay 设置是否使能显示
    *
    * @param en false : 不显示
                true :  显示
    */
        void SetEnableDisplay(bool en);

    /**
    * @brief SetVolume 设置音量
    *
    * @param vol 范围0-100
    */
        void SetVolume(int vol);


    /**
    * @brief SetMute 设置静音
    *
    * @param mute  true : 静音
                   false : 关闭静音
    */
        void SetMute(bool mute);

        /**
        * @brief GetStatus 获取当前状态
        *
        * @return int	\n
        * #define VIDEO_STATUS_CREATE  0          ///< @brief 库建立完成	\n
        * #define VIDEO_STATUS_READY  1           ///< @brief 已经准备好播放	\n
        * #define VIDEO_STATUS_RUNNING    2       ///< @brief 正在播放		\n
        * #define VIDEO_STATUS_STALL  3           ///< @brief 线程阻塞		\n
        * #define VIDEO_STATUS_END    4           ///< @brief 播放结束		\n
        * #define VIDEO_STATUS_ERR_PATH   5       ///< @brief 路径错误，找不到文件\n
        * #define VIDEO_STATUS_RESUME    6        ///< @brief 播放恢复 		\n
        */
            int GetStatus();


            /**
            * @brief GetStreamTime
            *
            * @param out_duration duration time in second
            * @param out_elapse elapse time in econd
            *
            */
            void GetStreamTime(int* out_duration, int* out_elapse);


        /**
        * @brief GetWindow 获取当前显示窗口
        *
        * @return 返回Window类型
        */
            Window* GetWindow();


        /**
        * @brief GetURL 获取当前播放的URL地址
        *
        * @return 返回字符串
        */
            const char* GetURL();


        /**
        * @brief GetVolume 获取音量
        *
        * @return 返回0-100
        */
            int GetVolume();

private:
     void on_connected();
     void on_disconnected();
     void on_msg_result(const char *msg_data);


private:
    RpcClient m_client;

    QSemaphore sem;
};

#endif // RPCCLIENT_H
