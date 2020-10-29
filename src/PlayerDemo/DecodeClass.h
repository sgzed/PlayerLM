#pragma once

struct AVCodec;
struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

#include <mutex>
#include <memory>
#include <Windows.h>

class DecodeClass
{
public:

	bool isAudio = false;

	//当前解码到的Pts
	long long pts = 0;

	//打开解码器,不管成功与否都释放para空间
	virtual bool Open(std::shared_ptr<AVCodecParameters> para);

	bool bAccel = false;
	//硬解加速
	virtual bool HwAcceleration(HWND hwnd);

	//发送到解码线程
	virtual bool Send(std::shared_ptr<AVPacket> pkt);

	//获取解码数据，一次send可能需要多次Recv，获取缓冲中的数据Send NULL在Recv多次
	//每次复制一份，由调用者释放 av_frame_free
	virtual std::shared_ptr<AVFrame> Recv();

	//渲染硬解解码出来的frame
	virtual void RetrieveFrame(std::shared_ptr<AVFrame> frame);

	virtual void Close();
	virtual void Clear();

	DecodeClass();
	virtual ~DecodeClass();

protected:
	AVCodec* codec = 0;
	AVCodecContext* mCodecCtx = 0;
	std::mutex mMtx;

	std::shared_ptr<AVCodecParameters> mPara;
};

