#ifndef FFMPEG_H
#define FFMPEG_H

#include <time.h>
#include <sys/time.h>
#include <list>
#include <queue>
#include <mutex>

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavfilter/avfilter.h>
    #include <libswscale/swscale.h>
    #include <libavutil/frame.h>
    #include <libavutil/timestamp.h>
}

class FFMPEG
{
public:
    FFMPEG();
    static void initFFMPEG();
    int start();

    char addr_input_video[100];
    char inputVidioFilename[128] = "/mnt/hgfs/share/video/10-anticlockwise.mp4";
    char inputAudeoFilename[128] = "/mnt/hgfs/share/video/music/tonghuazhen.mp3";
    char outputFilename[128] = "output.mp4";


    //common
    std::list <AVPacket> queue_input_video_paket;
    std::list <AVPacket> queue_input_audio_paket;
    std::mutex mutex_queue_video_packet;
    std::mutex mutex_queue_audio_packet;

    int packetConvertPts(AVFormatContext* pAVFormatContext_in, AVFormatContext* pAVFormatContext_out, AVPacket *pAVPacket, int stream_index_output);

    int findStream(AVFormatContext* ifmt_ctx, AVMediaType type, int* stream_index);
    int openCodec(AVFormatContext* ifmt_ctx, int stream_index);

    //video stream
    AVFormatContext *pAVFormatContext_video = NULL;    
    int video_index_in = 0;
    int video_index_out = 0;
    int openInputVideo();
    int findVideoStream();
    int createOutputStreamVideo();
    int readPacketVideo();
    int readVideoPacketFailed = 0;

    //audio stream
    AVFormatContext* pAVFormatContext_audio = NULL;
    int audio_index_in = 0;
    int audio_index_out = 0;
    int openInputAudio();
    int findAudioStream();
    int createOutputStreamAudio();
    int readPacketAudio();

    //output stream
    AVFormatContext* pAVFormatContext_output = nullptr;
    AVOutputFormat* pAVOutputFormat = nullptr;
    AVStream *outputStream_video, *outputStream_audio;
    AVPacket *packet_video, *packet_audio;
    AVPacket *packet_video_recv, *packet_audio_recv;
    int initOutputContext();
    int createOutputStream(AVFormatContext* ifmt_ctx, AVFormatContext* ofmt_ctx, int stream_index);
    int openOutputfile();
    int writePacketToOutputfile(AVPacket* packet);
    int writeOutputfile();
    int closeOutputfile();


};

#endif // FFMPEG_H
