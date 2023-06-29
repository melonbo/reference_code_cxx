#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QImage>
#include "ffPlayer.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent, char* url);
    ~Widget();
    void sendYUV(uchar *buff, int size, int num);
    void sendRGB(QImage image);
    QTimer *timer_slider;
    QImage m_image;
    int m_duration;
    int m_position;
    FFPlayer *player;
    char m_url[1024];
    float m_player_rate;

    void setVideoDuration(int len);
    void setVideoPts(int len);
    void setVideoSize(int w, int h);
    void setUrl(char* url){strcpy(m_url, url);}
private:
    Ui::Widget *ui;

public slots:
    void slot_send_yuv(uchar *buff, int size, int num);
    void slot_send_rgb(QImage image);
    void slot_timer_slider();
    void slot_set_video_size(int w, int h);
    void slot_set_video_duration(int len);
    void slot_set_video_pts(int len);
    void slot_duration(int value);
    void slot_position(int value);
    void slot_error(int value);
    void slot_request_pause(bool flag);

signals:
    void sig_send_yuv(uchar *buff, int size, int num);
    void sig_send_rgb(QImage image);
    void sig_set_video_size(int w, int h);
    void sig_set_video_duration(int len);
    void sig_set_video_pts(int len);
    void sig_slier_release(int value);
    void sig_ratio(float value);
    void sig_save_image();
    void sig_save_video(bool flag);
    void sig_seek_step(int value);//true=right, false=left;

private slots:
    void on_horizontalSlider_sliderReleased();
    void on_horizontalSlider_sliderPressed();
    void on_btn_speed_up_clicked();
    void on_btn_speed_down_clicked();
    void on_btn_exit_clicked();
    void on_btn_play_clicked();
    void on_btn_save_image_clicked();
    void on_btn_save_video_clicked();
    void on_btn_seek_forward_clicked();
};

#endif // WIDGET_H
