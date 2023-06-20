#ifndef VIDEOPLAYEREVENTHANDLE_H
#define VIDEOPLAYEREVENTHANDLE_H

#include "types.h"

class VideoPlayerCallBack
{
public:
    ///���ļ�ʧ��
    virtual void onOpenVideoFileFailed(const int &code = 0) = 0;

    ///��sdlʧ�ܵ�ʱ��ص��˺���
    virtual void onOpenSdlFailed(const int &code) = 0;

    ///��ȡ����Ƶʱ����ʱ����ô˺���
    virtual void onTotalTimeChanged(const int64_t &uSec) = 0;

    ///������״̬�ı��ʱ��ص��˺���
    virtual void onPlayerStateChanged(const VideoPlayerState &state, const bool &hasVideo, const bool &hasAudio) = 0;

    ///��ʾrgb���ݣ��˺�����������ʱ�����������Ӱ�첥�ŵ������ԣ������brgb32Buffer���ں������غ��ʧЧ��
    virtual void onDisplayVideo(const uint8_t *brgb32Buffer, const int &width, const int &height) = 0;

};

#endif // VIDEOPLAERYEVENTHANDLE_H
