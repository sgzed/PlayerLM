#pragma once

#include "DecodeThread.h"
#include <mutex>

struct AVCodecParameters;
class ResampleClass;
class IAudioPlay;

class AudioThread :public DecodeThread
{
public:
	//当前音频的pts
	long long pts = 0;

	//打开，不管成功与否都清理
	virtual bool Open(std::shared_ptr<AVCodecParameters> para, int sampleRate, int channels);
	
	//停止线程，清理资源
	virtual void Close();

	virtual void Clear();

	void SetPause(bool isPause);
	bool isPause = false;

	void run();
	AudioThread();
	virtual ~AudioThread();
	
protected:

	std::mutex amux;
	IAudioPlay* ap = 0;
	ResampleClass* res = 0;
};

