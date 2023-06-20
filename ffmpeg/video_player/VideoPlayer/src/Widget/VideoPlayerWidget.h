﻿/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef VIDEOPLAYERWIDGET_H
#define VIDEOPLAYERWIDGET_H

#include <QWidget>

#include <QImage>
#include <QPaintEvent>
#include <QTimer>
#include <QPushButton>
#include <QPropertyAnimation>

#include "VideoPlayer/VideoPlayer.h"


namespace Ui {
class VideoPlayerWidget;
}

///这个是播放器的主界面 包括那些按钮和进度条之类的
class VideoPlayerWidget : public QWidget, public VideoPlayerCallBack
{
    Q_OBJECT

public:
    explicit VideoPlayerWidget(QWidget *parent = 0);
    ~VideoPlayerWidget();

    ///捕获按键事件
    void keyPressEvent(QKeyEvent *event);

protected:
    bool eventFilter(QObject *target, QEvent *event);

private:
    Ui::VideoPlayerWidget *ui;

    VideoPlayer *mPlayer; //播放线程
    QTimer *mTimer; //定时器-获取当前视频时间
    float mVolume;

    QTimer *mTimer_CheckControlWidget; //用于控制控制界面的出现和隐藏
    QPropertyAnimation *mAnimation_ControlWidget;   //控制底部控制控件的出现和隐藏
    void showOutControlWidget(); //显示底部控制控件
    void hideControlWidget();    //隐藏底部控制控件

private slots:
    ///播放器相关的槽函数
    void slotSliderMoved(int value);
    void slotTimerTimeOut();
    void slotBtnClick(bool isChecked);


    ///以下函数，用于输出信息给界面
protected:
    ///打开文件失败
    void onOpenVideoFileFailed(const int &code);

    ///打开sdl失败的时候回调此函数
    void onOpenSdlFailed(const int &code);

    ///获取到视频时长的时候调用此函数
    void onTotalTimeChanged(const int64_t &uSec);

    ///播放器状态改变的时候回调此函数
    void onPlayerStateChanged(const VideoPlayerState &state, const bool &hasVideo, const bool &hasAudio);

    ///显示rgb数据，此函数不宜做耗时操作，否则会影响播放的流畅性，传入的brgb32Buffer，在函数返回后既失效。
    void onDisplayVideo(const uint8_t *brgb32Buffer, const int &width, const int &height);

    ///由于不能在子线程中操作界面，因此需要通过信号槽的方式，将上面函数转移到主线程



signals:
    void sig_OpenVideoFileFailed(const int &code);
    void sig_OpenSdlFailed(const int &code);
    void sig_TotalTimeChanged(const qint64 &Usec);
    void sig_PlayerStateChanged(const VideoPlayerState &state, const bool &hasVideo, const bool &hasAudio);
    void sig_DisplayVideo(const QImage &image);

private slots:
    void slotOpenVideoFileFailed(const int &code);
    void slotOpenSdlFailed(const int &code);
    void slotTotalTimeChanged(const qint64 &uSec);
    void slotStateChanged(const VideoPlayerState &state, const bool &hasVideo, const bool &hasAudio);
    void slotDisplayVideo(const QImage &image);

};

#endif // VIDEOPLAYERWIDGET_H
