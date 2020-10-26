#pragma once

#include "DecodeThread.h"
#include <mutex>
#include "IVideoCall.h"

struct AVCodecParameters;
struct AVPacket;
class D3DVideoWidget;

class VideoThread : public DecodeThread
{
public:
	//打开，不管成功与否都清理
	virtual bool Open(std::shared_ptr<AVCodecParameters> para, IVideoCall* call, int width, int height);

	virtual bool Open(std::shared_ptr<AVCodecParameters> para, D3DVideoWidget* call);
	
	//解码pkt到指定pts，并repaint
	virtual bool RepaintPts(std::shared_ptr<AVPacket>,long long pts);

	void run();

	VideoThread();
	~VideoThread();

	void SetPause(bool isPause);
	bool isPause = false;

	long long synPts = 0;
	long long pts = 0;

protected:
	IVideoCall* call = 0;
	D3DVideoWidget* d3dCall = 0;
	std::mutex vmux;
};

