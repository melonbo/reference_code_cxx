#include "rpcclient.h"
#include <unistd.h>
rpcclient::rpcclient()
{
    const char* addr = "tcp://127.0.0.1:5000";
 //   const char* addr = "ipc://bxtmm_rpc";
    m_client.set_callback(this);
    m_client.connect(addr);
//QTime dieTime = QTime::currentTime().addMSecs(100);

//while( QTime::currentTime() < dieTime )
//	QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	usleep(10000);
    m_client.send("new,0");

}

void rpcclient::Play(QString str_url, int loop)
{
    QString cmd("play,");
    cmd.append(str_url);
    cmd.append(",");
    cmd.append(QString("%1").arg(loop));
    qDebug()<<cmd;
    m_client.send(cmd.toLatin1().data());

}

void rpcclient::StartCameraStream()
{
    m_client.send("StartCameraStream");
}

void rpcclient::StopCameraStream()
{
    m_client.send("StopCameraStream");
}

void rpcclient::Pause(bool clear_flag)
{
    QString cmd("pause");
    cmd.append(",");
    cmd.append(QString("%1").arg(clear_flag));
    m_client.send(cmd.toLatin1().data());
}

void rpcclient::Resume()
{
    m_client.send("resume");
}


void rpcclient::PlayCamera(const char *ipaddr)
{
    QString cmd("playcamera");
    cmd.append(",");
    cmd.append(QString(ipaddr));
    m_client.send(cmd.toLatin1().data());
}

void rpcclient::Stop()
{
    m_client.send("stop");
}

void rpcclient::SetWindow(Window *w)
{
    QString cmd("setwindow,");
    cmd.append(QString("%1,%2,%3,%4").arg(w->x).arg(w->y).arg(w->width).arg(w->height));
    qDebug()<<cmd;
    m_client.send(cmd.toLatin1().data());
}

void rpcclient::SetEnableDisplay(bool en)
{
    QString cmd("SetEnableDisplay");
    if(en)
        cmd.append(",1");
    else
        cmd.append(",0");

    m_client.send(cmd.toLatin1().data());
}

void rpcclient::SetVolume(int vol)
{
    QString cmd("SetVolume");
    cmd.append(",");
    cmd.append(QString("%1").arg(vol));

    m_client.send(cmd.toLatin1().data());
}

void rpcclient::SetMute(bool mute)
{
    QString cmd("SetMute");
    if(mute)
        cmd.append(",1");
    else
        cmd.append(",0");

    m_client.send(cmd.toLatin1().data());
}

int rpcclient::GetStatus()
{
    return 0;
}

void rpcclient::GetStreamTime(int *out_duration, int *out_elapse)
{

}

Window *rpcclient::GetWindow()
{
    return NULL;
}

const char *rpcclient::GetURL()
{
    return NULL;
}

int rpcclient::GetVolume()
{

}




void rpcclient::on_msg_result(const char *msg_data)
{
    QString str_dat(msg_data);
    if(str_dat == QString("heartbeat"))
    {
        m_client.send("heartbeat");
    }
}

void rpcclient::on_disconnected()
{
    qDebug()<<"on_disconnected";
}

void rpcclient::on_connected()
{
    qDebug()<<"on_connected";
}
