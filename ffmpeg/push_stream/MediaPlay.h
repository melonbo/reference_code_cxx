/*
 * MediaPlay.h
 *
 *  Created on: Sep 16, 2022
 *      Author: root
 */

#ifndef SRC_MEDIA_MEDIAPLAY_H_
#define SRC_MEDIA_MEDIAPLAY_H_
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include "string.h"
#include <string>

using namespace std;
extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/log.h>
	#include <libavutil/avutil.h>
	//引入时间
	#include "libavutil/time.h"
}
#include "BaseThread.h"

typedef void (*BS_MediaCallback)(unsigned char *data , uint8_t size);
class MediaPlay: public BaseThread {
public:
	MediaPlay();
	virtual ~MediaPlay();
	void PlayModel(unsigned char model);
	int PlayMeida(const char *name);
    void findMediaFiles(string path);
	bool InitService(string pathName_,string destName_,BS_MediaCallback pDataCallBack);

private:
	static bool playFlag;
	static bool stopFlag;
	static unsigned char playModel;
	void ThreadProc(void);

    std::vector<std::string> mVeterFile;
	bool localPlayState ;
	bool netPlayState ;
	string mPathName;
	string mDestName;
};

#endif /* SRC_MEDIA_MEDIAPLAY_H_ */
