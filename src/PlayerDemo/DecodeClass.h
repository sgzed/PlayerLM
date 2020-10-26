#pragma once

struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
class D3DVideoWidget;

#include <mutex>
#include <memory>

class DecodeClass
{
public:

	bool isAudio = false;

	//是否启用了硬解码
	bool bAccel = false;

	//当前解码到的Pts
	long long pts = 0;

	//打开解码器,不管成功与否都释放para空间
	virtual bool Open(std::shared_ptr<AVCodecParameters> para, D3DVideoWidget* widget = nullptr,bool bAccel = false);

	//发送到解码线程
	virtual bool Send(std::shared_ptr<AVPacket> pkt);

	//获取解码数据，一次send可能需要多次Recv，获取缓冲中的数据Send NULL在Recv多次
	//每次复制一份，由调用者释放 av_frame_free
	virtual std::shared_ptr<AVFrame> Recv();

	virtual void Close();
	virtual void Clear();

	DecodeClass();
	virtual ~DecodeClass();

protected:
	AVCodecContext* mCodecCtx = 0;
	std::mutex mMtx;
};

