#pragma once

#include <QThread>
#include <mutex>
#include "IVideoCall.h"

class DemuxClass;
class VideoThread;
class AudioThread;

class PlayThread : public QThread
{
public:
	virtual bool Open(const char* url,IVideoCall* call);

	//启动所有线程
	virtual void Start();

	//关闭线程，清理资源
	virtual void Close();

	//清理资源，Fix Seek时需要等待的Bug
	virtual void Clear();

	virtual void Seek(double progress);

	void run();

	PlayThread();
	~PlayThread();

	bool isExit = false;
	long long pts = 0;
	long long totalMs = 0;

	void SetPause(bool isPause);
	bool isPause = false;

protected:
	std::mutex mux;

	DemuxClass* demux = 0;
	VideoThread* vt = 0;
	AudioThread* at = 0;
};

