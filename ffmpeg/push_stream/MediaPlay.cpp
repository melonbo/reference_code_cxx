/*
 * MediaPlay.cpp
 *
 *  Created on: Sep 16, 2022
 *      Author: root
 */

#include "MediaPlay.h"
#include <dirent.h>

bool MediaPlay::playFlag = false;
bool MediaPlay::stopFlag = false;
unsigned char MediaPlay::playModel = false;

MediaPlay::MediaPlay() {
	localPlayState = false;
	netPlayState = false;
}

MediaPlay::~MediaPlay() {
	// TODO Auto-generated destructor stub
}

void MediaPlay::PlayModel(unsigned char model)
{
	playModel = model;
}

int MediaPlay::PlayMeida(const char *name)
{
	int ret = 0;
	int re = -1;
	AVRational rate;
	AVFormatContext *ifmt_ctx = NULL;
	int audio_stream_index = -1;
    int video_stream_index = -1;
	AVPacket *packet = NULL;
	AVFrame *frame = NULL;
	AVStream *in_stream = NULL;
	AVStream *out_stream =NULL;
	AVCodec *codec = NULL;
	AVFormatContext *ofmt_ctx =NULL;
	int64_t start_time_relative = 0;


	av_register_all();
	avformat_network_init();
	//申请avformatcontext
	ifmt_ctx = avformat_alloc_context();
	if(!ifmt_ctx)
	{
		ret = -1;
		goto end;
	}
	printf("file name is %s \n",name);
    if (ret = avformat_open_input(&ifmt_ctx,name,NULL,NULL) != 0)
	{
        printf("fail to open input stream, ret %d, errno %d\n", ret, errno);
		ret = -2;
		goto end;
	}
	//读取一些流的信息
	if (avformat_find_stream_info(ifmt_ctx,NULL) < 0)
	{
		printf("fail to find the stream info\n");
		ret = -3;
		goto end;
	}
	av_dump_format(ifmt_ctx,0,name,0);

    printf("ifmt_ctx codec id = %d, streams number = %d\n",ifmt_ctx->streams[0]->codecpar->codec_id,ifmt_ctx->nb_streams);
	//找到最好的音频流的索引

	audio_stream_index = av_find_best_stream(ifmt_ctx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
	if (audio_stream_index == -1)
	{
		printf("fail to find the audio stream\n");
		ret = -4;
		goto end;
	}
	printf("audio_stream_index = %d\n",audio_stream_index);


    video_stream_index = av_find_best_stream(ifmt_ctx,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
    if (video_stream_index == -1)
	{
        printf("fail to find the video stream\n");
		ret = -5;
		goto end;
	}
    printf("video_stream_index = %d\n",video_stream_index);

	//分配AVPacket和AVFrame内存，用于接收音频数据，解码数据
	packet = av_packet_alloc();
	if(!packet)
	{
		ret = -6;
		goto end;
	}
	frame = av_frame_alloc();
	if(!frame)
	{
		ret = -6;
		goto end;
	}

	//输出（Output）
	ofmt_ctx = avformat_alloc_context();
	if(!ofmt_ctx)
	{
		ret = -7;
		goto end;
	}

	if(avformat_alloc_output_context2(&ofmt_ctx, NULL, "mpegts", mDestName.c_str()) < 0)
	{
		goto end;
	}

	if (!ofmt_ctx) {
		printf("Could not create output context\n");
		ret = -8;
		goto end;
	}

	rate.num = 1;
	rate.den = 25;

	for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
		//根据输入流创建输出流（Create output AVStream according to input AVStream）
		in_stream = ifmt_ctx->streams[i];
		codec = avcodec_find_decoder(ifmt_ctx->streams[i]->codecpar->codec_id);
		if (!codec) {
			printf("Could not create output context\n");
			ret = -9;
			goto end;
		}
		out_stream = avformat_new_stream(ofmt_ctx, codec);
		if (!out_stream) {
			printf("Failed allocating output stream\n");
			ret = -10;
			goto end;

		}
		out_stream->time_base = rate;
		//复制AVCodecContext的设置（Copy the settings of AVCodecContext）
		if(avcodec_copy_context(out_stream->codec, in_stream->codec) < 0)
		{
			ret = -11;
			goto end;
		}

		if(avcodec_parameters_copy(out_stream->codecpar,ifmt_ctx->streams[i]->codecpar)< 0)
		{
			ret = -12;
			goto end;
		}
	}
	if (avio_open(&ofmt_ctx->pb, mDestName.c_str(), AVIO_FLAG_WRITE) < 0) {
		printf("Could not open output URL '%s'", mDestName.c_str());
		ret = -13;
		goto end;
	}
	re = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0) {
		printf("Error occurred when opening output URL\n");
		ret = -14;
		goto end;
	}

	av_dump_format(ofmt_ctx,0,mDestName.c_str(),1);
	start_time_relative=av_gettime_relative();
    while (av_read_frame(ifmt_ctx, packet) == 0) {//将音频数据读入packet
        if (packet->stream_index == video_stream_index) {
			//printf("*****************************************\n");
            AVRational time_base = ifmt_ctx->streams[video_stream_index]->time_base;
			AVRational time_base_q = { 1,AV_TIME_BASE };
			int64_t pts_time = av_rescale_q(packet->dts, time_base, time_base_q);

			//int64_t now_time = av_gettime() - start_time;
			int64_t now_time = av_gettime_relative() - start_time_relative;
			if (pts_time > now_time)
			{
				av_usleep(pts_time - now_time);
				//printf("time=%d\n",(pts_time - now_time));
			}
		}
		in_stream = ifmt_ctx->streams[packet->stream_index];
		out_stream = ofmt_ctx->streams[packet->stream_index];
		packet->pts = av_rescale_q_rnd(packet->pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		packet->dts = av_rescale_q_rnd(packet->dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		packet->duration = av_rescale_q(packet->duration, in_stream->time_base, out_stream->time_base);
		packet->pos = -1;
//		printf("--*********---packet = %d ptk-time = %d  in_stream->time_base = %d out_stream->time_base = %d packet->stream_index = %d,%s\n",
//									packet->dts,ifmt_ctx->streams[packet->stream_index]->time_base.den,in_stream->time_base,out_stream->time_base,packet->stream_index,__TIME__);
		re = av_interleaved_write_frame(ofmt_ctx, packet);
		if (re < 0) {
            printf("Error muxing packet, ret %d, errno %d\n", re, errno);
            printf("packet stream_index %d, pts %lld, dts %lld\n", packet->stream_index, packet->pts, packet->dts);
			ret = -14;
			goto end;
		}
		av_free_packet(packet);

	}

	end:
	if(packet) av_free_packet(packet);
	if(frame) av_frame_free(&frame);
	if(ifmt_ctx) avformat_close_input(&ifmt_ctx);
	if(ofmt_ctx) avformat_close_input(&ofmt_ctx);
	return ret;
}


bool MediaPlay::InitService(string pathName_,string destName_,BS_MediaCallback pDataCallBack)
{
    mPathName = pathName_;
    mDestName = destName_;

	if(!this->IsRun())
	{
		if(!this->Start())
		{
			return false;
		}
	}
	localPlayState = true;
	return true;
}


void MediaPlay::ThreadProc(void)
{
    while(this->IsRun())//循环播放模式
	{
        PlayMeida(mPathName.c_str());
        usleep(100*1000);
	}
}
