#ifndef FFPLAYER_H
#define FFPLAYER_H
#include <QImage>
#include <QObject>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <thread>
#include <mutex>

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include <libavutil/time.h>
    #include "libavutil/pixfmt.h"
    #include "libswscale/swscale.h"
    #include "libswresample/swresample.h"
    #include "libavutil/imgutils.h"
}

class FFPlayer:public QObject
{
    Q_OBJECT
public:
    FFPlayer(QWidget *parent, char *file);
    int openInput(char *file);
    int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    void run();
    void start();
    void setOutputSize(int w, int h){m_width_out = w; m_height_out = h;}
    uint8_t *AVFrame2Img(AVFrame *pFrame);
    void writeYUV();
    void setPause(bool flag){m_pause = flag;}
    int Find_StreamIndex(AVFormatContext* ic,enum AVMediaType type);
    void printArray(uint8_t* array, int len);

    void *m_parent;
    char m_file[1024];
    pthread_t thread_handle;
    bool m_pause;
    int m_video_height_in, m_video_width_in;
    float m_video_framerate_source;
    int m_height_out, m_width_out;

    AVFormatContext *fmt_ctx = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    AVOutputFormat *ofmt_out = NULL;
    int video_stream_idx = -1, audio_stream_idx = -1;
    AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx;
    AVFrame *pAVFrame = NULL;
    AVPacket pkt;
    struct SwsContext *pSwsContext;
    AVPicture  pAVPicture;
    int m_usleep;
    int64_t m_video_duration;
    int64_t m_position;
    AVRational m_video_timebase;
    float m_ratio;
    int64_t m_video_bitrate;
    struct timeval time_start;
    struct timeval time_end;
    bool interrupt_enable;
    int frame_second = 0;
    bool bSaveVideo = false;
    std::vector<AVPacket> packetList;
    std::mutex mtx_packet_list;


    int trans_file = 0;
    bool isPlayEndFile = false;
    AVStream *video_stream = NULL, *audio_stream = NULL;
    const char *src_filename = NULL;
    int refcount = 0;
    int seek_request = 0;
    int seek_step = 0;


signals:
    void sig_duration(int value);
    void sig_position(int value);
    void sig_send_rgb(QImage image);
    void sig_request_pause(bool flag);

public slots:
    void slot_seek(int value);
    void slot_seek_start();
    void slot_ratio(float value);
    void slot_save_image();
    void slot_save_video(bool flag);
    void slot_seek_step(int value);
};

#endif // FFPLAYER_H
