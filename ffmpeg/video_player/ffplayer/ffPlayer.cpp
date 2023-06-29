#include "ffPlayer.h"
#include "widget.h"
#include <sys/time.h>
#include <unistd.h>
#include <QFileInfo>
#include <QDebug>
#include <sys/prctl.h>
#define TIME_OUT 2000

int interruptCallBack(void* p)
{
    FFPlayer* param = (FFPlayer*)p;
    gettimeofday(&param->time_end, NULL);
    int64_t time_use = ((param->time_end.tv_sec-param->time_start.tv_sec)*1000 +
            (param->time_end.tv_usec-param->time_start.tv_usec)/1000);

    if(!param->interrupt_enable) return 0;
    if(time_use>TIME_OUT)
    {
        printf("interruptCallBack, read frame use time %lld\n", time_use);
        return 1;
    }
    return 0;
}

FFPlayer::FFPlayer(QWidget *parent, char *file)
{
    m_parent = parent;
    strcpy(m_file, file);
    m_pause = 0;
    m_ratio = 1;

    av_register_all();
    avformat_network_init();
}

int FFPlayer::openInput(char *file)
{
    char buf[1024]={0};
    int ret = 0;
    printf("\nopen file %s\n", file);

    fmt_ctx = avformat_alloc_context();
//    fmt_ctx->interrupt_callback.callback = interruptCallBack;
//    fmt_ctx->interrupt_callback.opaque = this;

    AVDictionary* options = NULL;
//    av_dict_set(&options, "buffer_size", "102400", 0); //设置缓存大小，1080p可将值调大
    av_dict_set(&options, "probesize", "2048", 0);
    av_dict_set(&options, "max_analyze_duration", "10", 0);
    av_dict_set(&options, "rtsp_transport", "tcp", 0); //默认以udp方式打开，改为tcp
//    av_dict_set(&options, "stimeout", "2000000", 0); //设置超时断开连接时间，单位微秒

    interrupt_enable = true;
    ret=avformat_open_input(&fmt_ctx, file, NULL, NULL);    
    interrupt_enable = false;
    if (ret < 0) {
        av_strerror(ret, buf, 1024);
        printf("Couldn't open file %s, return %d, %s\n", file, ret, buf);
        return 1;
    }

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        printf("Could not find stream information\n");
        return 1;
    }

    video_stream_idx=Find_StreamIndex(fmt_ctx,AVMEDIA_TYPE_VIDEO);
    audio_stream_idx=Find_StreamIndex(fmt_ctx,AVMEDIA_TYPE_AUDIO);

    printf("\navformat_find_stream_info, video %d, audio %d\n", video_stream_idx, audio_stream_idx);
    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        video_stream = fmt_ctx->streams[video_stream_idx];
    }

    if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
        audio_stream = fmt_ctx->streams[audio_stream_idx];
    }

    m_video_height_in =fmt_ctx->streams[video_stream_idx]->codec->height;
    m_video_width_in = fmt_ctx->streams[video_stream_idx]->codec->width;
    m_video_framerate_source = fmt_ctx->streams[video_stream_idx]->codec->framerate.num/fmt_ctx->streams[video_stream_idx]->codec->framerate.den;
    m_video_framerate_source = 25;//
    m_video_duration = fmt_ctx->streams[video_stream_idx]->duration;
    m_video_timebase = fmt_ctx->streams[video_stream_idx]->time_base;
    m_video_bitrate = fmt_ctx->bit_rate;
    m_usleep = 1000000/m_video_framerate_source;

    emit sig_duration(m_video_duration/m_video_timebase.den);
    printf("frame rate %d/%d\n", fmt_ctx->streams[video_stream_idx]->codec->framerate.num, fmt_ctx->streams[video_stream_idx]->codec->framerate.den);
    printf("duration %lld\n", m_video_duration/m_video_timebase.den);
    printf("time base %d/%d\n", m_video_timebase.num, m_video_timebase.den);
    printf("video frame rate %f\n", m_video_framerate_source);

    printf("bit rate %lld\n", fmt_ctx->bit_rate);
    printf("first dts %lld\n", fmt_ctx->streams[video_stream_idx]->first_dts);

    printf("usleep %d\n", m_usleep);

    avpicture_alloc(&pAVPicture, AV_PIX_FMT_RGB24, m_video_width_in, m_video_height_in);
    pSwsContext = sws_getContext(m_video_width_in, m_video_height_in, AV_PIX_FMT_YUV420P, m_video_width_in, m_video_height_in, AV_PIX_FMT_RGB24, SWS_BICUBIC, 0, 0, 0);

    av_dump_format(fmt_ctx, 0, file, 0);

    if (!audio_stream) {
        fprintf(stderr, "Could not find audio in the input\n");
    }

    if (!video_stream) {
        fprintf(stderr, "Could not find video stream in the input, aborting\n");
        return 1;
    }

    pAVFrame = av_frame_alloc();
    if (!pAVFrame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        return 1;
    }

    printf("av_frame_alloc success\n");
    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    return 0;
}

int FFPlayer::open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
     AVStream *st;
     AVCodec *dec = NULL;
     AVDictionary *opts = NULL;

     ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
     if (ret < 0) {
         fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                 av_get_media_type_string(type), src_filename);
         return ret;
     } else {
         stream_index = ret;
         st = fmt_ctx->streams[stream_index];

         /* find decoder for the stream */
         dec = avcodec_find_decoder(st->codecpar->codec_id);
         if (!dec) {
             fprintf(stderr, "Failed to find %s codec\n",
                     av_get_media_type_string(type));
             return AVERROR(EINVAL);
         }

         /* Allocate a codec context for the decoder */
         *dec_ctx = avcodec_alloc_context3(dec);
         if (!*dec_ctx) {
             fprintf(stderr, "Failed to allocate the %s codec context\n",
                     av_get_media_type_string(type));
             return AVERROR(ENOMEM);
         }

         /* Copy codec parameters from input stream to output codec context */
         if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
             fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                     av_get_media_type_string(type));
             return ret;
         }

         /* Init the decoders, with or without reference counting */
         av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
         if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
             fprintf(stderr, "Failed to open %s codec\n",
                     av_get_media_type_string(type));
             return ret;
         }
         *stream_idx = stream_index;
     }

     return 0;
}

void* thread_proc_player(void *p)
{
    ((FFPlayer*)p)->run();
}

void FFPlayer::start()
{
    thread_handle = pthread_create(&thread_handle, NULL, thread_proc_player, this);
}

void FFPlayer::run()
{
    int ret, got_picture;
    uint time_diff;
    struct timeval time01, time02, time03;
    time01.tv_sec = 0;
    time01.tv_usec = 0;
    gettimeofday(&time01, NULL);
    openInput(m_file);
    int firstFrame = 1;
    int64_t time_stamp_offset, time_stamp_frame;
    int64_t time_stamp_tmp = 0;
    int frame_num = 1;
    int frame_index = 0;

    while(1)
    {
        if(m_pause)
        {
            usleep(100);
            continue;
        }

        interrupt_enable = true;
        ret = av_read_frame(fmt_ctx, &pkt);

        interrupt_enable = false;
        if(ret < 0)
        {
            printf("################# av_read_frame error, ret=%d\n", ret);
            if(-541478725 == ret)
            {

            }
            break;
        }

        if(bSaveVideo)
        {
            mtx_packet_list.lock();
            if(pkt.stream_index == video_stream_idx)
                packetList.push_back(pkt);
            mtx_packet_list.unlock();
        }

        if(firstFrame==1)
        {
            firstFrame = 0;
            time_stamp_offset = pkt.pts*av_q2d(fmt_ctx->streams[video_stream_idx]->time_base);
            printf("first package time stamp %lld\n", time_stamp_offset);
        }

        if(pkt.stream_index == video_stream_idx)
        {
            m_position = pkt.pts;
//            printf("pts time stamp %lld, %lld\n", pkt.pts, AV_TIME_BASE);
            avcodec_send_packet(video_dec_ctx, &pkt);
            ret = avcodec_receive_frame(video_dec_ctx, pAVFrame);

            if(ret == 0)
            {
                if(time_stamp_tmp==0)
                    time_stamp_tmp = pkt.pts;
                frame_index++;
                gettimeofday(&time03, NULL);
                //printf("get a picture %d, at time %d.%d, pkt stampe %lld\n", frame_index, time03.tv_sec, time03.tv_usec, pkt.pts);
                do
                {
                    gettimeofday(&time02, NULL);
                    time_diff = (time02.tv_sec-time01.tv_sec)*1000000+(time02.tv_usec - time01.tv_usec);
                    //printf("========= %d %d %f, frame rate %f\n", time_diff, m_usleep, m_usleep/m_ratio, m_video_framerate_source);
                    usleep(100);
                }while(time_diff < m_usleep/m_ratio);

                //printf("wait packet time %d, usleep %d, ratio %f, frame num %d\n", time_diff, m_usleep, m_ratio, frame_num);
                //printf("stream %d pakcet dts %lld pts %lld\n", pkt.stream_index, pkt.dts, pkt.pts);

                while(seek_request)
                {
                    usleep(100);
                }

                frame_num += 1;
                if(m_ratio > 1.5)
                {
                    if(1 == frame_num%2)
                        continue;
                }


                time_stamp_frame = pkt.pts;
                frame_second = time_stamp_frame/m_video_timebase.den - time_stamp_offset;

                emit sig_position(frame_second);
                time01.tv_sec = time02.tv_sec;
                time01.tv_usec = time02.tv_usec;


                //((Widget*)m_parent)->sendYUV(AVFrame2Img(frame), frame->width*frame->height*3, m_channel);
                sws_scale(pSwsContext, (const uint8_t* const *)pAVFrame->data, pAVFrame->linesize, 0, m_video_height_in, pAVPicture.data, pAVPicture.linesize);
                //发送获取一帧图像信号
                QImage image(pAVPicture.data[0], m_video_width_in, m_video_height_in, QImage::Format_RGB888);

//                ((Widget*)m_parent)->sendRGB(image);
                emit sig_send_rgb(image);

                seek_step--;
                if(seek_step==1)
                {
                    m_pause = true;
                    emit sig_request_pause(true);
                    printf("request pause in FFPlayer\n");
                }
                if(seek_step>1)
                    printf("seek step least %d\n", seek_step);
            }
            else
            {
                printf("avcodec_receive_frame error, ret=%d, errno=%d\n", ret, errno);
            }
        }

    }
    exit(0);
}

void FFPlayer::slot_save_image()
{
    QFileInfo fileinfo = QFileInfo(m_file);
    QString base_name = fileinfo.baseName();
    QString image_name = base_name + "." + QString::number(frame_second) + ".png";
    QImage image(pAVPicture.data[0], m_video_width_in, m_video_height_in, QImage::Format_RGB888);
    bool ret = image.save(image_name);
    qDebug() << "save image" << image_name << ret;
}

uint8_t *FFPlayer::AVFrame2Img(AVFrame *pFrame) {
    int frameHeight = pFrame->height;
     int frameWidth = pFrame->width;
     int channels = 3;

     //创建保存yuv数据的buffer
     uint8_t *pDecodedBuffer = (uint8_t *) malloc(
             frameHeight * frameWidth * sizeof(uint8_t) * channels);

     //从AVFrame中获取yuv420p数据，并保存到buffer
     int i, j, k;
     //拷贝y分量
     for (i = 0; i < frameHeight; i++) {
         memcpy(pDecodedBuffer + frameWidth * i,
                pFrame->data[0] + pFrame->linesize[0] * i,
                frameWidth);
     }
     //拷贝u分量
     for (j = 0; j < frameHeight / 2; j++) {
         memcpy(pDecodedBuffer + frameWidth * i + frameWidth / 2 * j,
                pFrame->data[1] + pFrame->linesize[1] * j,
                frameWidth / 2);
     }
     //拷贝v分量
     for (k = 0; k < frameHeight / 2; k++) {
         memcpy(pDecodedBuffer + frameWidth * i + frameWidth / 2 * j + frameWidth / 2 * k,
                pFrame->data[2] + pFrame->linesize[2] * k,
                frameWidth / 2);
     }

     return pDecodedBuffer;
}

//查找流ID 索引号
int FFPlayer::Find_StreamIndex(AVFormatContext* ic,enum AVMediaType type)
{
    int Index=-1;
    for(unsigned int i=0; i< ic->nb_streams; i++)
        if(ic->streams[i]->codecpar->codec_type==type)
        {
            Index=i;
            break;
        }
        if(Index==-1)
            return -1;
        return Index;
}

void FFPlayer::slot_seek_start()
{
    seek_request = 1;
}

void FFPlayer::slot_seek(int value)
{
    int64_t seek_pos = value * m_video_bitrate / 8;
    int ret = avformat_seek_file(fmt_ctx, video_stream_idx, seek_pos*0.7, seek_pos, seek_pos*1.3, AVSEEK_FLAG_BYTE);
    printf("stream seek time %d, seek pos %lld, ret = %d\n", value, seek_pos, ret);
    seek_request = 0;
}

void FFPlayer::slot_ratio(float value)
{
    m_ratio = value;
}

void writeOutputFile(FFPlayer* pthis)
{
    int ret;
    prctl(PR_SET_NAME, "writeOutputFile");
    avformat_alloc_output_context2(&pthis->ofmt_ctx, NULL, "mp4", NULL);
    if (!pthis->ofmt_ctx) {
        qDebug() << "avformat_alloc_output_context2 failed";
        return;
    }else{
        qDebug() << "avformat_alloc_output_context2 success";
    }
    pthis->ofmt_out = pthis->ofmt_ctx->oformat;

    AVStream* in_stream;
    AVStream* out_stream;
    if(pthis->video_stream_idx >= 0)
    {
        in_stream = pthis->fmt_ctx->streams[pthis->video_stream_idx];
        out_stream = avformat_new_stream(pthis->ofmt_ctx, in_stream->codec->codec);
        if (!out_stream) {
            qDebug() << "Failed allocating output stream";
            return;
        }
        //Copy the settings of AVCodecContext
        if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
            qDebug() << "Failed to copy context from video input to output stream codec context";
            return;
        }

        out_stream->codec->codec_tag = 0;
        if (pthis->ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    else
    {
        return;
    }

    if(pthis->audio_stream_idx >= 0)
    {
        in_stream = pthis->fmt_ctx->streams[pthis->audio_stream_idx];
        out_stream = avformat_new_stream(pthis->ofmt_ctx, in_stream->codec->codec);
        if (!out_stream) {
            qDebug() << "Failed allocating output stream";
            return;
        }
        //Copy the settings of AVCodecContext
        if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
            qDebug() << "Failed to copy context from audio input to output stream codec context";
            return;
        }
        out_stream->codec->codec_tag = 0;
        if (pthis->ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }


    QFileInfo fileinfo = QFileInfo(pthis->m_file);
    QString base_name = fileinfo.baseName();
    QString image_name = base_name + "." + QString::number(pthis->frame_second) + ".mp4";
    if (ret = avio_open(&pthis->ofmt_ctx->pb, image_name.toLatin1().data(), AVIO_FLAG_WRITE) < 0)
    {
        printf( "Could not open output file %s, ret = %d\n", image_name.toLatin1().data(), ret);
        return;
    }
    else
    {
        printf("open output file success, %s\n", image_name.toLatin1().data());
    }

    if (ret=avformat_write_header(pthis->ofmt_ctx, NULL) < 0)
    {
        printf( "Error occurred when opening output file, ret=%d\n", ret);
        return ;
    }

    int find_start_sps = 0;

    while(1)
    {
        sleep(1);
        pthis->mtx_packet_list.lock();
        if(pthis->packetList.size()<10)
        {
            pthis->mtx_packet_list.unlock();
            continue;
        }

        //+6是因为每帧前面有一个AUD，即00 00 00 01 09 f0
        while(pthis->packetList.size())
        {
            std::vector<AVPacket>::iterator pAVPacket = pthis->packetList.begin();
            if(find_start_sps == 0)
            {
                //pthis->printArray((*pAVPacket).data, 30);

                if((*pAVPacket).data[0+6] == 0x00 && (*pAVPacket).data[1+6] == 0x00 && (*pAVPacket).data[2+6] == 0x01)
                {
                    if((*pAVPacket).data[3+6] == 0x67 || (*pAVPacket).data[3+6] == 0x68)
                    {
                        find_start_sps = 1;
                    }
                }
                if((*pAVPacket).data[0+6] == 0x00 && (*pAVPacket).data[1+6] == 0x00 && (*pAVPacket).data[2+6] == 0x00 && (*pAVPacket).data[3+6] == 0x01)
                {
                    if((*pAVPacket).data[4+6] == 0x67 || (*pAVPacket).data[4+6] == 0x68)
                    {
                        find_start_sps = 1;
                    }
                }
            }
            if(find_start_sps == 0)
            {
                av_packet_unref(&(*pAVPacket));
                pthis->packetList.erase(pAVPacket);
                continue;
            }


            int ret = av_write_frame(pthis->ofmt_ctx, &(*pAVPacket));
            if (ret < 0) {
                printf( "Error write frame to file, ret=%d, errno=%d\n", ret, errno);
            }
            av_packet_unref(&(*pAVPacket));
            pthis->packetList.erase(pAVPacket);

        }
        pthis->mtx_packet_list.unlock();
        if(!pthis->bSaveVideo)
            break;
    }
    if((ret = av_write_trailer(pthis->ofmt_ctx))<0)
    {
        printf("write trailer to file error, ret = %d, errno=%d\n", ret, errno);
    }

    printf("close output stream start\n");
    if (pthis->ofmt_ctx && !(pthis->ofmt_out->flags & AVFMT_NOFILE))
            avio_close(pthis->ofmt_ctx->pb);

    if(pthis->ofmt_ctx)
    {
        avformat_free_context(pthis->ofmt_ctx);
        pthis->ofmt_ctx = NULL;
    }
    printf("close output stream end\n");
}

void FFPlayer::slot_save_video(bool flag)
{
    bSaveVideo = flag;

    if(bSaveVideo)
    {
        std::thread([&](FFPlayer*pthis){writeOutputFile(this);},this).detach();
    }
}

void FFPlayer::printArray(uint8_t *array, int len)
{
    printf("packet header");
    for(int i=0; i<len; i++)
    {
        printf(" %02x", array[i]);
    }
    printf("\n");
}

void FFPlayer::slot_seek_step(int value)
{
    printf("seek forwark in FFPlayer\n");
//    seek_step = 10;
//    int64_t seek_pos = value * m_video_bitrate / 8;
//    int ret = avformat_seek_file(fmt_ctx, video_stream_idx, seek_pos*0.9, seek_pos, seek_pos*1.1, AVSEEK_FLAG_BYTE);
//    if(ret<0)
//        printf("seek forward error, ret=%d, errno=%d\n", ret, errno);

}
