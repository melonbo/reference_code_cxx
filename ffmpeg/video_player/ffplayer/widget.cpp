#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QtCore/QtCore>
#include <QFileInfo>

Widget::Widget(QWidget *parent, char* url) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint|Qt::Tool|Qt::X11BypassWindowManagerHint);
    ui->setupUi(this);
    m_player_rate = 1;
    strcpy(m_url, url);

    timer_slider = new QTimer();
    connect(timer_slider, SIGNAL(timeout()), this, SLOT(slot_timer_slider()));
    timer_slider->start(100);
    connect(this, SIGNAL(sig_send_rgb(QImage)), this, SLOT(slot_send_rgb(QImage)));

    player = new FFPlayer(this, m_url);
    connect(player, SIGNAL(sig_duration(int)), this, SLOT(slot_duration(int)));
    connect(player, SIGNAL(sig_position(int)), this, SLOT(slot_position(int)));
    connect(this, SIGNAL(sig_slier_release(int)), player, SLOT(slot_seek(int)));
    connect(ui->horizontalSlider, SIGNAL(sliderPressed()), player, SLOT(slot_seek_start()));
    connect(this, SIGNAL(sig_ratio(float)), player, SLOT(slot_ratio(float)));
    connect(player, SIGNAL(sig_send_rgb(QImage)), this, SLOT(slot_send_rgb(QImage)));
    connect(this, SIGNAL(sig_save_image()), player, SLOT(slot_save_image()));
    connect(this, SIGNAL(sig_save_video(bool)), player, SLOT(slot_save_video(bool)));
    connect(this, SIGNAL(sig_seek_step(int)), player, SLOT(slot_seek_step(int)));
    connect(player, SIGNAL(sig_request_pause(bool)), this, SLOT(slot_request_pause(bool)));
    player->start();

    ui->btn_save_image->hide();
    ui->btn_save_video->hide();
    ui->btn_seek_forward->hide();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::sendYUV(uchar *buff, int size, int num)
{

}


void Widget::slot_send_yuv(uchar *buff, int size, int num)
{

}

void Widget::sendRGB(QImage image)
{
    m_image = image;
    emit sig_send_rgb(m_image);
}


void Widget::slot_send_rgb(QImage image)
{

    int labWidth = ui->label_video->geometry().width();
    int labHeight = ui->label_video->geometry().height();

    if (image.height() > 0)
    {
        QPixmap pix = QPixmap::fromImage(image.scaled(labWidth, labHeight));
        ui->label_video->setPixmap(pix);
    }
}

void Widget::setVideoSize(int w, int h)
{
    emit sig_set_video_size(w,h);
}

void Widget::slot_set_video_size(int w, int h)
{

}

void Widget::setVideoDuration(int len)
{
    emit sig_set_video_duration(len);
}

void Widget::slot_set_video_duration(int len)
{

}

void Widget::setVideoPts(int len)
{
    emit sig_set_video_pts(len);
}

void Widget::slot_set_video_pts(int len)
{

}

void Widget::slot_timer_slider()
{
//    m_pts = video_player.m_pts;
//    ui->horizontalSlider->setValue(m_pts/1000000);
}


void Widget::slot_duration(int value)
{
    m_duration = value;
    ui->horizontalSlider->setRange(0, m_duration);
}

void Widget::slot_position(int value)
{
//    if((m_position-value) == 1) return;d
    m_position = value;
    ui->horizontalSlider->setValue(m_position);

    QString min_p = QString::number(m_position/60,10);
    QString sec_p = QString::number(m_position%60,10);
    QString min_d = QString::number(m_duration/60,10);
    QString sec_d = QString::number(m_duration%60,10);
    QString str_progress = min_p + ":" + sec_p + "/" + min_d + ":" + sec_d;
    ui->label_progress->setText(str_progress);
}

void Widget::slot_error(int value)
{

}

void Widget::on_horizontalSlider_sliderReleased()
{
    int value  = ui->horizontalSlider->value();
    emit sig_slier_release(value);
    connect(player, SIGNAL(sig_position(int)), this, SLOT(slot_position(int)));
}

void Widget::on_horizontalSlider_sliderPressed()
{
    disconnect(player, SIGNAL(sig_position(int)), this, SLOT(slot_position(int)));
}

void Widget::on_btn_speed_up_clicked()
{
    if(m_player_rate >= 1)
        m_player_rate += 0.2;
    else
        m_player_rate += 0.2;

    if(m_player_rate >= 2.0)
        m_player_rate = 2.0;

    emit sig_ratio(m_player_rate);
    ui->label_speed->setText(QString("X%1").arg(m_player_rate));
}

void Widget::on_btn_speed_down_clicked()
{
    if(m_player_rate >= 1)
        m_player_rate -= 0.2;
    else
        m_player_rate -= 0.2;

    if(m_player_rate <= 0.2)
        m_player_rate = 0.2;

    emit sig_ratio(m_player_rate);
    ui->label_speed->setText(QString("X%1").arg(m_player_rate));
}

void Widget::on_btn_exit_clicked()
{
    exit(0);
}

void Widget::on_btn_play_clicked()
{
    if(ui->btn_play->text() == "暂停")
    {
        ui->btn_play->setText("播放");
        player->setPause(true);
    }
    else
    {
        ui->btn_play->setText("暂停");
        player->setPause(false);
        player->seek_step = 0;
    }
}

void Widget::on_btn_save_image_clicked()
{
    emit sig_save_image();
}

void Widget::on_btn_save_video_clicked()
{
    if(ui->btn_save_video->text()=="录像")
    {
        ui->btn_save_video->setText("停止录像");
        emit sig_save_video(true);
    }
    else
    {
        ui->btn_save_video->setText("录像");
        emit sig_save_video(false);
    }
}

void Widget::on_btn_seek_forward_clicked()
{
    ui->btn_play->setText("暂停");
    player->setPause(false);
    player->seek_step = 10;
}

void Widget::slot_request_pause(bool flag)
{
    ui->btn_play->setText("播放");
    player->setPause(true);
    printf("receive request pause in widget\n");
}
