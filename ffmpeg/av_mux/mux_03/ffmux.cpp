#include <thread>
#include <unistd.h>
#include "ffmux.h"
#include <stdio.h>

char av_ts_string[AV_TS_MAX_STRING_SIZE] = { 0 };
#define av_ts2str(ts) av_ts_make_string(av_ts_string, ts)

char av_ts_buff[AV_TS_MAX_STRING_SIZE] = { 0 };
#define av_ts2timestr(ts, tb) av_ts_make_time_string(av_ts_buff, ts, tb)

static void log_packet(const AVFormatContext* fmt_ctx, const AVPacket* pkt, const char* tag)
{
    AVRational* time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}

FFMPEG::FFMPEG()
{
    avformat_network_init();
    //av_log_set_level(AV_LOG_DEBUG);
}

int FFMPEG::openInputVideo()
{
    int ret = avformat_open_input(&pAVFormatContext_video, inputVidioFilename, NULL, NULL);
    if (ret != 0)
    {
        printf("Couldn't open input stream %s.\n", inputVidioFilename);
        return -1;
    }
    if (avformat_find_stream_info(pAVFormatContext_video, NULL) < 0)
    {
        printf("Couldn't find stream information.\n");
        return -1;
    }
}

int FFMPEG::openInputAudio_01()
{
    int ret = avformat_open_input(&pAVFormatContext_audio_01, inputAudeoFilename_01, NULL, NULL);
    if (ret != 0)
    {
        printf("Couldn't open input stream.\n");
        return -1;
    }
    if (avformat_find_stream_info(pAVFormatContext_audio_01, NULL) < 0)
    {
        printf("Couldn't find stream information.\n");
        return -1;
    }

}

int FFMPEG::openInputAudio_02()
{
    int ret = avformat_open_input(&pAVFormatContext_audio_02, inputAudeoFilename_02, NULL, NULL);
    if (ret != 0)
    {
        printf("Couldn't open input stream.\n");
        return -1;
    }
    if (avformat_find_stream_info(pAVFormatContext_audio_02, NULL) < 0)
    {
        printf("Couldn't find stream information.\n");
        return -1;
    }

}

int FFMPEG::findVideoStream()
{
    video_index_in = av_find_best_stream(pAVFormatContext_video, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
}

int FFMPEG::findAudioStream_01()
{
    audio_index_in_01 = av_find_best_stream(pAVFormatContext_audio_01, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
}

int FFMPEG::findAudioStream_02()
{
    audio_index_in_02 = av_find_best_stream(pAVFormatContext_audio_02, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
}


int FFMPEG::createOutputStreamVideo()
{
    int ret;
    AVStream* stream = pAVFormatContext_video->streams[video_index_in];
    const AVCodec* codec2 = NULL;
    codec2 = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec2)
    {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    AVCodecContext* codec_ctx2 = NULL;
    codec_ctx2 = avcodec_alloc_context3(codec2);
    if (!codec_ctx2)
    {
        exit(1);
    }
    avcodec_parameters_to_context(codec_ctx2, pAVFormatContext_video->streams[video_index_in]->codecpar);
    if ((ret = avcodec_open2(codec_ctx2, codec2, NULL) < 0))
    {
        return -1;
    }
}

int FFMPEG::createOutputStreamAudio_01()
{
    int ret;
    AVStream* stream = pAVFormatContext_audio_01->streams[audio_index_in_01];
    const AVCodec* codec = NULL;
    codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec)
    {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    AVCodecContext* codec_ctx = NULL;
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        exit(1);
    }
    avcodec_parameters_to_context(codec_ctx, pAVFormatContext_audio_01->streams[audio_index_in_01]->codecpar);
    if ((ret = avcodec_open2(codec_ctx, codec, NULL) < 0))
    {
        return -1;
    }
}

int FFMPEG::createOutputStreamAudio_02()
{
    int ret;
    AVStream* stream = pAVFormatContext_audio_02->streams[audio_index_in_02];
    const AVCodec* codec = NULL;
    codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec)
    {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    AVCodecContext* codec_ctx = NULL;
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        exit(1);
    }
    avcodec_parameters_to_context(codec_ctx, pAVFormatContext_audio_02->streams[audio_index_in_02]->codecpar);
    if ((ret = avcodec_open2(codec_ctx, codec, NULL) < 0))
    {
        return -1;
    }
}

int FFMPEG::initOutputContext()
{
    avformat_alloc_output_context2(&pAVFormatContext_output, NULL, "mpegts", outputFilename);
    if (!pAVFormatContext_output) {
        printf("Could not create output context\n");
        return -1;
    }

    pAVOutputFormat = pAVFormatContext_output->oformat;


    if(pAVFormatContext_audio_01)//创建输出音频流 01
    {
        audio_index_out_01 = pAVFormatContext_output->nb_streams;
        AVStream* out = avformat_new_stream(pAVFormatContext_output, NULL);
        if (!out)
        {
            return -1;
        }

        AVCodecParameters* codecpar = pAVFormatContext_audio_01->streams[audio_index_in_01]->codecpar;
        avcodec_parameters_copy(out->codecpar, codecpar);
        out->codecpar->codec_tag = 0;
    }

    if(pAVFormatContext_video)//创建输出视频流
    {
        video_index_out = pAVFormatContext_output->nb_streams;
        AVStream* out = avformat_new_stream(pAVFormatContext_output, NULL);
        if (!out)
        {
            return -1;
        }

        AVCodecParameters* codecpar = pAVFormatContext_video->streams[video_index_in]->codecpar;
        avcodec_parameters_copy(out->codecpar, codecpar);
        out->codecpar->codec_tag = 0;
    }

    if(pAVFormatContext_audio_02)//创建输出音频流 02
    {
        audio_index_out_02 = pAVFormatContext_output->nb_streams;
        AVStream* out = avformat_new_stream(pAVFormatContext_output, NULL);
        if (!out)
        {
            return -1;
        }

        AVCodecParameters* codecpar = pAVFormatContext_audio_02->streams[audio_index_in_02]->codecpar;
        avcodec_parameters_copy(out->codecpar, codecpar);
        out->codecpar->codec_tag = 0;
    }

    av_dump_format(pAVFormatContext_output, 0, outputFilename, 1);

    packet_video_recv = av_packet_alloc();
    packet_audio_recv_01 = av_packet_alloc();
    packet_audio_recv_02 = av_packet_alloc();
}

int FFMPEG::openOutputfile()
{
    int ret;
    if (!(pAVOutputFormat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&pAVFormatContext_output->pb, outputFilename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf("Could not open output URL '%s'", outputFilename);
            return -1;
        }
    }
    //Write file header
    ret = avformat_write_header(pAVFormatContext_output, NULL);
    if (ret < 0) {
        printf("Error occurred when opening output URL\n");
        return -1;
    }
}

int FFMPEG::packetConvertPts(AVFormatContext *pAVFormatContext_in, AVFormatContext *pAVFormatContext_out, AVPacket *pAVPacket, int stream_index_output)
{
    AVStream* in_stream = pAVFormatContext_in->streams[pAVPacket->stream_index];
    AVStream* out_stream = pAVFormatContext_out->streams[stream_index_output];

    log_packet(pAVFormatContext_in, pAVPacket, "in");
    av_packet_rescale_ts(pAVPacket, in_stream->time_base, out_stream->time_base);
    pAVPacket->pos = -1;
    pAVPacket->stream_index = stream_index_output;
    log_packet(pAVFormatContext_out, pAVPacket, "out");
}



int FFMPEG::writeOutputfile(AVPacket* pkt)
{
    int ret = av_interleaved_write_frame(pAVFormatContext_output, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error muxing packet\n");
    }
}

int FFMPEG::readPacketVideo()
{
    static int index_video = 0;
    for(;;){
        if(queue_input_video_paket.size()>10){
            usleep(100);
            continue;
        }

        if(av_read_frame(pAVFormatContext_video, packet_video_recv) < 0){
            readVideoPacketFailed = 1;
            break;
        }else{
            printf("av_read_frame queue_input_video_paket %p, %d\n", packet_video_recv, packet_video_recv->size);
            //packet_video_recv->stream_index = 0;
            mutex_queue_video_packet.lock();
            queue_input_video_paket.push_back(*packet_video_recv);
            mutex_queue_video_packet.unlock();
        }
        //printf("read packet video 01, index %d, size %d, pts/dts %lld %lld, flag %d\n", ++index_video, packet_video.size, packet_video.pts, packet_video.dts, packet_video.flags);
    }
    return 0;
}

int FFMPEG::readPacketAudio_01()
{
    static int index_audio_01 = 0;
    for(;;){
        if(queue_input_audio_paket_01.size()>100){
            usleep(100);
            continue;
        }
        if(av_read_frame(pAVFormatContext_audio_01, packet_audio_recv_01) < 0){
            break;
        }else{
            printf("av_read_frame queue_input_audio_01_paket %p, %d\n", packet_audio_recv_01, packet_audio_recv_01->size);
            mutex_queue_audio_packet_01.lock();
            queue_input_audio_paket_01.push_back(*packet_audio_recv_01);
            mutex_queue_audio_packet_01.unlock();
        }

        //printf("read packet audio 01, %d, size %d, pts/dts %lld %lld\n", ++index_audio_01, packet_audio_01.size, packet_audio_01.pts, packet_audio_01.dts);
    }
    return 0;
}

int FFMPEG::readPacketAudio_02()
{
    static int index_audio_02 = 0;
    for(;;){
        if(queue_input_audio_paket_02.size()>100){
            usleep(100);
            continue;
        }
        if(av_read_frame(pAVFormatContext_audio_02, packet_audio_recv_02) < 0){
            break;
        }else{
            mutex_queue_audio_packet_02.lock();
            queue_input_audio_paket_02.push_back(*packet_audio_recv_02);
            mutex_queue_audio_packet_02.unlock();
        }

        //printf("read packet audio 01, %d, size %d, pts/dts %lld %lld\n", ++index_audio_01, packet_audio_01.size, packet_audio_01.pts, packet_audio_01.dts);
    }
    return 0;
}

int FFMPEG::writeOutputfile()
{
    int ret = 0;
    AVPacket *out_packet_video = nullptr;
    AVPacket *out_packet_audio_01 = nullptr;
    AVPacket *out_packet_audio_02 = nullptr;


    for(;;){
        if(!queue_input_video_paket.empty())
        {
            //printf("queue video size %d\n", queue_input_video_paket.size());
            if(out_packet_video == nullptr)
            {
                mutex_queue_video_packet.lock();
                AVPacket pkt_v = queue_input_video_paket.front();
                out_packet_video = &pkt_v;
                queue_input_video_paket.pop_front();
                mutex_queue_video_packet.unlock();
                packetConvertPts(pAVFormatContext_video, pAVFormatContext_output, out_packet_video, video_index_out);
            }
        }
        else {
            if(readVideoPacketFailed)
            {
                printf("readVideoPacketFailed \n");
                break;
            }
        }


        if(!queue_input_audio_paket_01.empty())
        {
            if(out_packet_audio_01 == nullptr)
            {
                mutex_queue_audio_packet_01.lock();
                AVPacket pkt_a1 = queue_input_audio_paket_01.front();
                out_packet_audio_01 = &pkt_a1;
                queue_input_audio_paket_01.pop_front();
                mutex_queue_audio_packet_01.unlock();
                packetConvertPts(pAVFormatContext_audio_01, pAVFormatContext_output, out_packet_audio_01, audio_index_out_01);
            }
        }

        if(!queue_input_audio_paket_02.empty())
        {
            if(out_packet_audio_02 == nullptr)
            {
                mutex_queue_audio_packet_02.lock();
                AVPacket pkt_a2 = queue_input_audio_paket_02.front();
                out_packet_audio_02 = &pkt_a2;
                queue_input_audio_paket_02.pop_front();
                mutex_queue_audio_packet_02.unlock();
                packetConvertPts(pAVFormatContext_audio_02, pAVFormatContext_output, out_packet_audio_02, audio_index_out_02);
            }
        }

        if(out_packet_audio_01){
            if(out_packet_video){
                if(out_packet_audio_01->pts <= out_packet_video->pts)
                {
                    writePacketToOutputfile(out_packet_audio_01);
                    out_packet_audio_01 = nullptr;
                }
            }else{
                writePacketToOutputfile(out_packet_audio_01);
                out_packet_audio_01 = nullptr;
            }

            continue;
        }

        if(out_packet_audio_02){
            if(out_packet_video)
            {
                if(out_packet_audio_02->pts <= out_packet_video->pts)
                {
                    writePacketToOutputfile(out_packet_audio_02);
                    out_packet_audio_02 = nullptr;
                }

            }else{
                writePacketToOutputfile(out_packet_audio_02);
                out_packet_audio_02 = nullptr;
            }
            continue;
        }

        if(out_packet_video){
            //write out_packet_video
            writePacketToOutputfile(out_packet_video);
            out_packet_video = nullptr;
        }


    }

    printf("av_write_trailer() start %p\n", pAVFormatContext_output);
    if((ret = av_write_trailer(pAVFormatContext_output))<0)
    {
        printf("write trailer to file error, ret = %d, errno=%d\n", ret, errno);
    }
    printf("av_write_trailer()\n");
}

int FFMPEG::writePacketToOutputfile(AVPacket* packet)
{
    if(packet->pts == AV_NOPTS_VALUE) return -1;
    //printf("111111111111111111 %p\n", pAVFormatContext_output);
    int ret = av_interleaved_write_frame(pAVFormatContext_output, packet);
    //printf("222222222222222222 %p\n", packet);
    //av_packet_unref(packet);
    return ret;
}

int FFMPEG::closeOutputfile()
{
    if(pAVFormatContext_video) avformat_close_input(&pAVFormatContext_video);
    if(pAVFormatContext_audio_01) avformat_close_input(&pAVFormatContext_audio_01);
    if(pAVFormatContext_audio_02) avformat_close_input(&pAVFormatContext_audio_02);
    if(pAVFormatContext_output) avformat_free_context(pAVFormatContext_output);
    printf("closeOutputfile()\n");
    exit(0);
    return 0;
}

int FFMPEG::start()
{
    int ret = 0;

    // 打开视频流
    openInputVideo();
    findVideoStream();
    createOutputStreamVideo();

    // 打开音频流01
    openInputAudio_01();
    findAudioStream_01();
    createOutputStreamAudio_01();

    // 打开音频流02
    openInputAudio_02();
    findAudioStream_02();
    createOutputStreamAudio_02();

    // 打开输出流
    initOutputContext();
    openOutputfile();

    std::thread([=]{this->readPacketVideo();}).detach();
    std::thread([=]{this->readPacketAudio_01();}).detach();
    std::thread([=]{this->readPacketAudio_02();}).detach();
    std::thread([=]{writeOutputfile();closeOutputfile();}).detach();

    return 0;

    // 音视频合成
    while (1)
    {
        {

            ret = av_read_frame(pAVFormatContext_audio_01, packet_video_recv);
            if (ret < 0)
                break;

            if (packet_video_recv->stream_index == audio_index_in_01)
            {
                packetConvertPts(pAVFormatContext_audio_01, pAVFormatContext_output, packet_video_recv, audio_index_out_01);
                writeOutputfile(packet_video_recv);
            }
            else
            {
                av_packet_unref(packet_video_recv);
            }
        }
        //else
        {
            ret = av_read_frame(pAVFormatContext_video, packet_audio_recv_01);
            if (ret < 0)
                break;

            if (packet_audio_recv_01->stream_index == video_index_in)
            {
                packetConvertPts(pAVFormatContext_video, pAVFormatContext_output, packet_audio_recv_01, video_index_out);
                writeOutputfile(packet_audio_recv_01);
            }
            else
            {
                av_packet_unref(packet_audio_recv_01);
            }

        }

    }


    av_write_trailer(pAVFormatContext_output);


    return 0;
}


