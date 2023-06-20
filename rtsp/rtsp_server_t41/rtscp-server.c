/*
 * 作者：_JT_
 * 博客：https://blog.csdn.net/weixin_42462202
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "tcp_rtp.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>

//#include "logodata_100x100_bgra.h"
#include "sample-common.h"

#define H264_FILE_NAME  "test.h264"
#define SERVER_PORT     8554
#define BUF_MAX_SIZE    (1024*1024)
#define TAG "Sample-Encoder-video"
#define FS_CHN_NUM 4
extern struct chn_conf chn[];
void rtpHeaderInit(struct RtpPacket* rtpPacket, uint8_t csrcLen, uint8_t extension,
                    uint8_t padding, uint8_t version, uint8_t payloadType, uint8_t marker,
                   uint16_t seq, uint32_t timestamp, uint32_t ssrc)
{
    rtpPacket->rtpHeader.csrcLen = csrcLen;
    rtpPacket->rtpHeader.extension = extension;
    rtpPacket->rtpHeader.padding = padding;
    rtpPacket->rtpHeader.version = version;
    rtpPacket->rtpHeader.payloadType =  payloadType;
    rtpPacket->rtpHeader.marker = marker;
    rtpPacket->rtpHeader.seq = seq;
    rtpPacket->rtpHeader.timestamp = timestamp;
    rtpPacket->rtpHeader.ssrc = ssrc;
}

int rtpSendPacket(int socket, uint8_t rtpChannel, struct RtpPacket* rtpPacket, uint32_t dataSize)
{
    int ret;

    rtpPacket->header[0] = '$';
    rtpPacket->header[1] = rtpChannel;
    rtpPacket->header[2] = ((dataSize+RTP_HEADER_SIZE) & 0xFF00 ) >> 8;
    rtpPacket->header[3] = (dataSize+RTP_HEADER_SIZE) & 0xFF;

    rtpPacket->rtpHeader.seq = htons(rtpPacket->rtpHeader.seq);
    rtpPacket->rtpHeader.timestamp = htonl(rtpPacket->rtpHeader.timestamp);
    rtpPacket->rtpHeader.ssrc = htonl(rtpPacket->rtpHeader.ssrc);
    ret = send(socket, (void*)rtpPacket, dataSize+RTP_HEADER_SIZE+4, 0);
    if(ret<0)
	{
    		printf("send errror!\n");
	}
    rtpPacket->rtpHeader.seq = ntohs(rtpPacket->rtpHeader.seq);
    rtpPacket->rtpHeader.timestamp = ntohl(rtpPacket->rtpHeader.timestamp);
    rtpPacket->rtpHeader.ssrc = ntohl(rtpPacket->rtpHeader.ssrc);

    return ret;
}

static int createTcpSocket()
{
    int sockfd;
    int on = 1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        return -1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

    return sockfd;
}

static int createUdpSocket()
{
    int sockfd;
    int on = 1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
        return -1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

    return sockfd;
}

static int bindSocketAddr(int sockfd, const char* ip, int port)
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if(bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
        return -1;

    return 0;
}

static int acceptClient(int sockfd, char* ip, int* port)
{
    int clientfd;
    socklen_t len = 0;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    len = sizeof(addr);

    clientfd = accept(sockfd, (struct sockaddr *)&addr, &len);
    if(clientfd < 0)
        return -1;
    
    strcpy(ip, inet_ntoa(addr.sin_addr));
    *port = ntohs(addr.sin_port);

    return clientfd;
}

static inline int startCode3(char* buf)
{
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
        return 1;
    else
        return 0;
}

static inline int startCode4(char* buf)
{
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
        return 1;
    else
        return 0;
}

static char* findNextStartCode(char* buf, int len)
{
    int i;

    if(len < 3)
        return NULL;

    for(i = 0; i < len-3; ++i)
    {
        if(startCode3(buf) || startCode4(buf))
            return buf;
        
        ++buf;
    }

    if(startCode3(buf))
        return buf;

    return NULL;
}

static int getFrameFromH264File(int fd, char* frame, int size)
{
    int rSize, frameSize;
    char* nextStartCode;

    if(fd < 0)
        return fd;

    rSize = read(fd, frame, size);
    if(!startCode3(frame) && !startCode4(frame))
        return -1;
    
    nextStartCode = findNextStartCode(frame+3, rSize-3);
    if(!nextStartCode)
    {
        //lseek(fd, 0, SEEK_SET);
        //frameSize = rSize;
        return -1;
    }
    else
    {
        frameSize = (nextStartCode-frame);
        lseek(fd, frameSize-rSize, SEEK_CUR);
    }

    return frameSize;
}

static int rtpSendH264Frame(int socket, int rtpChannel, struct RtpPacket* rtpPacket, uint8_t* frame, uint32_t frameSize)
{
    uint8_t naluType; // nalu第一个字节
    int sendBytes = 0;
    int ret;

    naluType = frame[0];

    if (frameSize <= RTP_MAX_PKT_SIZE) // nalu长度小于最大包场：单一NALU单元模式
    {
        /*
         *   0 1 2 3 4 5 6 7 8 9
         *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *  |F|NRI|  Type   | a single NAL unit ... |
         *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         */
        memcpy(rtpPacket->payload, frame, frameSize);
        ret = rtpSendPacket(socket, rtpChannel, rtpPacket, frameSize);
        if(ret < 0)
            return -1;

        rtpPacket->rtpHeader.seq++;
        sendBytes += ret;
        if ((naluType & 0x1F) == 7 || (naluType & 0x1F) == 8) // 如果是SPS、PPS就不需要加时间戳
            goto out;
    }
    else // nalu长度小于最大包场：分片模式
    {
        /*
         *  0                   1                   2
         *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         * | FU indicator  |   FU header   |   FU payload   ...  |
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         */

        /*
         *     FU Indicator
         *    0 1 2 3 4 5 6 7
         *   +-+-+-+-+-+-+-+-+
         *   |F|NRI|  Type   |
         *   +---------------+
         */

        /*
         *      FU Header
         *    0 1 2 3 4 5 6 7
         *   +-+-+-+-+-+-+-+-+
         *   |S|E|R|  Type   |
         *   +---------------+
         */

        int pktNum = frameSize / RTP_MAX_PKT_SIZE;       // 有几个完整的包
        int remainPktSize = frameSize % RTP_MAX_PKT_SIZE; // 剩余不完整包的大小
        int i, pos = 1;

        /* 发送完整的包 */
        for (i = 0; i < pktNum; i++)
        {
            rtpPacket->payload[0] = (naluType & 0x60) | 28;
            rtpPacket->payload[1] = naluType & 0x1F;
            
            if (i == 0) //第一包数据
                rtpPacket->payload[1] |= 0x80; // start
            else if (remainPktSize == 0 && i == pktNum - 1) //最后一包数据
                rtpPacket->payload[1] |= 0x40; // end

            memcpy(rtpPacket->payload+2, frame+pos, RTP_MAX_PKT_SIZE);
            ret = rtpSendPacket(socket, rtpChannel, rtpPacket, RTP_MAX_PKT_SIZE+2);
            if(ret < 0)
                return -1;

            rtpPacket->rtpHeader.seq++;
            sendBytes += ret;
            pos += RTP_MAX_PKT_SIZE;
        }

        /* 发送剩余的数据 */
        if (remainPktSize > 0)
        {
            rtpPacket->payload[0] = (naluType & 0x60) | 28;
            rtpPacket->payload[1] = naluType & 0x1F;
            rtpPacket->payload[1] |= 0x40; //end

            memcpy(rtpPacket->payload+2, frame+pos, remainPktSize+2);
            ret = rtpSendPacket(socket, rtpChannel, rtpPacket, remainPktSize+2);
            if(ret < 0)
                return -1;

            rtpPacket->rtpHeader.seq++;
            sendBytes += ret;
        }
    }

out:

    return sendBytes;
}

static char* getLineFromBuf(char* buf, char* line)
{
    while(*buf != '\n')
    {
        *line = *buf;
        line++;
        buf++;
    }

    *line = '\n';
    ++line;
    *line = '\0';

    ++buf;
    return buf; 
}

static int handleCmd_OPTIONS(char* result, int cseq)
{
    sprintf(result, "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Public: OPTIONS, DESCRIBE, SETUP, PLAY\r\n"
                    "\r\n",
                    cseq);
                
    return 0;
}

static int handleCmd_DESCRIBE(char* result, int cseq, char* url)
{
    char sdp[500];
    char localIp[100];

    sscanf(url, "rtsp://%[^:]:", localIp);

    sprintf(sdp, "v=0\r\n"
                 "o=- 9%ld 1 IN IP4 %s\r\n"
                 "t=0 0\r\n"
                 "a=control:*\r\n"
                 "m=video 0 RTP/AVP 96\r\n"
                 "a=rtpmap:96 H264/90000\r\n"
                 "a=control:track0\r\n",
                 time(NULL), localIp);
    
    sprintf(result, "RTSP/1.0 200 OK\r\nCSeq: %d\r\n"
                    "Content-Base: %s\r\n"
                    "Content-type: application/sdp\r\n"
                    "Content-length: %d\r\n\r\n"
                    "%s",
                    cseq,
                    url,
                    strlen(sdp),
                    sdp);
    
    return 0;
}

static int handleCmd_SETUP(char* result, int cseq, uint8_t rtpChannel)
{
   sprintf(result, "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Transport: RTP/AVP/TCP;unicast;interleaved=%hhu-%hhu\r\n"
                    "Session: 66334873\r\n"
                    "\r\n",
                    cseq,
                    rtpChannel,
                    rtpChannel+1
                    );
    
    return 0;
}

static int handleCmd_PLAY(char* result, int cseq)
{
    sprintf(result, "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Range: npt=0.000-\r\n"
                    "Session: 66334873; timeout=60\r\n\r\n",
                    cseq);
    
    return 0;
}

static void doClient(int clientSockfd, const char* clientIP, int clientPort)
{
    char method[40];
    char url[100];
    char version[40];
    int cseq,ret;
    char *bufPtr;
    char* rBuf = malloc(BUF_MAX_SIZE);
    char* sBuf = malloc(BUF_MAX_SIZE);
    char line[400];
    uint8_t rtpChannel;
    uint8_t rtcpChannel;
    IMPEncoderStream stream;
    unsigned int chnNum=0;
    int i, nr_pack;
   // unsigned int total_send[1000]={0},send_counter=0,total_send_counter=0;
    while(1)
    {
        int recvLen;

        recvLen = recv(clientSockfd, rBuf, BUF_MAX_SIZE, 0);
        if(recvLen <= 0)
            goto out;

        rBuf[recvLen] = '\0';
        printf("---------------C->S--------------\n");
        printf("%s", rBuf);

        /* 解析方法 */
        bufPtr = getLineFromBuf(rBuf, line);
        if(sscanf(line, "%s %s %s\r\n", method, url, version) != 3)
        {
            printf("parse err\n");
            goto out;
        }

        /* 解析序列号 */
        bufPtr = getLineFromBuf(bufPtr, line);
        if(sscanf(line, "CSeq: %d\r\n", &cseq) != 1)
        {
            printf("parse err\n");
            goto out;
        }

        /* 如果是SETUP，那么就再解析channel */
        if(!strcmp(method, "SETUP"))
        {
            while(1)
            {
                bufPtr = getLineFromBuf(bufPtr, line);
                if(!strncmp(line, "Transport:", strlen("Transport:")))
                {
                    sscanf(line, "Transport: RTP/AVP/TCP;unicast;interleaved=%hhu-%hhu\r\n",
                                    &rtpChannel, &rtcpChannel);
                    break;
                }
            }
        }

        if(!strcmp(method, "OPTIONS"))
        {
            if(handleCmd_OPTIONS(sBuf, cseq))
            {
                printf("failed to handle options\n");
                goto out;
            }
        }
        else if(!strcmp(method, "DESCRIBE"))
        {
            if(handleCmd_DESCRIBE(sBuf, cseq, url))
            {
                printf("failed to handle describe\n");
                goto out;
            }
        }
        else if(!strcmp(method, "SETUP"))
        {
            if(handleCmd_SETUP(sBuf, cseq, rtpChannel))
            {
                printf("failed to handle setup\n");
                goto out;
            }
        }
        else if(!strcmp(method, "PLAY"))
        {
            if(handleCmd_PLAY(sBuf, cseq))
            {
                printf("failed to handle play\n");
                goto out;
            }
        }
        else
        {
            goto out;
        }

        printf("---------------S->C--------------\n");
        printf("%s", sBuf);
        send(clientSockfd, sBuf, strlen(sBuf), 0);

        /* 开始播放，发送RTP包 */
        if(!strcmp(method, "PLAY"))
        {
            int frameSize, startCode;
           // char* frame = malloc(500000);
            struct RtpPacket* rtpPacket = (struct RtpPacket*)malloc(500000);
            int fd = open("/tmp/wl.h264", O_RDWR|O_CREAT);
            assert(fd > 0);
            rtpHeaderInit(rtpPacket, 0, 0, 0, RTP_VESION, RTP_PAYLOAD_TYPE_H264, 0,
                            0, 0, 0x88923423);

            printf("start play\n");

            while (1)
            {
		 ret = IMP_Encoder_StartRecvPic(chnNum);
		 if (ret < 0) {
			    printf("IMP_Encoder_StartRecvPic(%d) failed\n", chnNum);
			    return ((void *)-1);
 		 }

  	 	 ret = IMP_Encoder_PollingStream(0, 1000);//chnNum对于0端口，参数传递进来是0
	 	 if (ret < 0) {
     			    printf("IMP_Encoder_PollingStream(%d) timeout\n", chnNum);
     			     continue;
   	   	 }
   	    /* Get H264 or H265 Stream */
	    ret = IMP_Encoder_GetStream(0, &stream, 1);//对于0端口，参数传递进来是0
	    if (ret < 0) {
		      printf("IMP_Encoder_GetStream(%d) failed\n", chnNum);
		      return ((void *)-1);
	    }

	    nr_pack = stream.packCount;
	   // printf("nr_pack=%d\n",nr_pack);
	     for (i = 0; i < nr_pack; i++) 
	     {
	      	IMPEncoderPack *pack = &stream.pack[i];
	    	frameSize = pack->length;
		if(frameSize>0)
		{
		    uint32_t remSize = stream.streamSize - pack->offset;
	            if(remSize < pack->length)
	            {
		      printf("remSize < pack->length\n");	
	            }
	            else
	            {
			   ret = write(fd, (void *)(stream.virAddr + pack->offset), pack->length);
	 		  if (ret != pack->length) {
             			 IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].length(%d) error:%s\n", ret, i, pack->length, strerror(errno));
             			 return -1;
             			 }
	
	 		 if(startCode3((uint8_t*)(stream.virAddr + pack->offset)))
      		 		startCode = 3;
 			 else
       			 	startCode = 4;
 			 frameSize -= startCode;
       			 // printf("frameSize=%d\n",frameSize);
		//	 total_send[send_counter]=
			rtpSendH264Frame(clientSockfd, rtpChannel, rtpPacket, (uint8_t*)(stream.virAddr + pack->offset+startCode), frameSize);
//			printf("total_send[%d]=%d\n",send_counter,total_send[send_counter]);
		//	total_send_counter+=total_send[send_counter];
		//	 printf("total_send[%d]=%d,total_send_counter=%d\n",send_counter,total_send[send_counter],total_send_counter);
		//	send_counter++;
 			 rtpPacket->rtpHeader.timestamp += 90000/SENSOR_FRAME_RATE_NUM;
 			 usleep(1000*1000/SENSOR_FRAME_RATE_NUM);
	           }
	     }//if(frameSize>0)
	   }//for (i = 0; i < nr_pack; i++) 
 	   IMP_Encoder_ReleaseStream(0, &stream);//每次都要release

            }//while(1)

            //free(frame);
	    close(fd);
	    ret = IMP_Encoder_StopRecvPic(chnNum);
	  if (ret < 0) {
   		 printf("IMP_Encoder_StopRecvPic(%d) failed\n", chnNum);
   		 return ((void *)-1);
  		}

            free(rtpPacket);
            goto out;
        }//if(!strcmp(method, "PLAY"))

}//first while(1)
    
out:
    printf("finish\n");
    close(clientSockfd);
    free(rBuf);
    free(sBuf);
    /* Step.a Stream Off */
        ret = sample_framesource_streamoff();
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
                return -1;
        }

        /* Step.b UnBind */
        for (i = 0; i < FS_CHN_NUM; i++) {
                if (chn[i].enable) {
                        ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
                        if (ret < 0) {
                                IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
                                return -1;
                        }
                }
        }

        /* Step.c Encoder exit */
        ret = sample_encoder_exit();
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "Encoder exit failed\n");
                return -1;
        }

        /* Step.d FrameSource exit */
        ret = sample_framesource_exit();
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
                return -1;
        }

        /* Step.e System exit */
        ret = sample_system_exit();
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
                return -1;
        }

}

int main(int argc, char* argv[])
{
    int serverSockfd;
    int ret;
    int i;
    serverSockfd = createTcpSocket();
    if(serverSockfd < 0)
    {
        printf("failed to create tcp socket\n");
        return -1;
    }

    ret = bindSocketAddr(serverSockfd, "0.0.0.0", SERVER_PORT);
    if(ret < 0)
    {
        printf("failed to bind addr\n");
        return -1;
    }

    ret = listen(serverSockfd, 10);
    if(ret < 0)
    {
        printf("failed to listen\n");
        return -1;
    }

    printf("rtsp://127.0.0.1:%d\n", SERVER_PORT);

    while(1)
    {
        int clientSockfd;
        char clientIp[40];
        int clientPort;
	/* Step.1 System init */
        ret = sample_system_init();
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
                return -1;
        }
       printf("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n");
        /* Step.2 FrameSource init */
        ret = sample_framesource_init();
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "FrameSource init failed\n");
                return -1;
        }

        /* Step.3 Encoder init */
        for (i = 0; i < FS_CHN_NUM; i++) {
                if (chn[i].enable) {
                        ret = IMP_Encoder_CreateGroup(chn[i].index);
                        if (ret < 0) {
                                IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", chn[i].index);
                                return -1;
                        }
                }
        }

        ret = sample_encoder_init();
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "Encoder init failed\n");
                return -1;
        }

        /* Step.4 Bind */
        for (i = 0; i < FS_CHN_NUM; i++) {
                if (chn[i].enable) {
                        ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
                        if (ret < 0) {
                                IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
                                return -1;
                        }
                }
	}
 	/* Step.5 Stream On */
        ret = sample_framesource_streamon();
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
                return -1;
        }

        /* Step.6 Get stream */
       /* ret = sample_get_video_stream();
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "Get video stream failed\n");
            return -1;
        }
*/


        clientSockfd = acceptClient(serverSockfd, clientIp, &clientPort);
        if(clientSockfd < 0)
        {
            printf("failed to accept client\n");
            return -1;
        }

        printf("accept client;client ip:%s,client port:%d\n", clientIp, clientPort);

        doClient(clientSockfd, clientIp, clientPort);
    
	}
    return 0;
}
