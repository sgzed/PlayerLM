#pragma once

#include <mutex>
#include <memory>

struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;
struct AVFrame;

class DemuxClass
{
public:
	DemuxClass();
	~DemuxClass();

	virtual bool Open(const char* url);

	//空间需要调用者释放 ，释放AVPacket对象空间，和数据空间 av_packet_free
	virtual std::shared_ptr<AVPacket> Read();

	bool IsAudio(std::shared_ptr<AVPacket> pkt);

	//获取视频参数  返回的空间需要清理  avcodec_parameters_free
	virtual std::shared_ptr<AVCodecParameters> CopyVPara();

	//获取音频参数  返回的空间需要清理 avcodec_parameters_free
	virtual std::shared_ptr<AVCodecParameters> CopyAPara();

	//seek 位置 pos 0.0 ~1.0
	virtual bool Seek(double pos);

	//清空读取缓存
	virtual void Clear();
	virtual void Close();

	//媒体总时长（毫秒）
	int totalMs = 0;

protected:

	std::mutex mMtx;
	//解封装上下文
	AVFormatContext* mFmtCtx = nullptr;
	//音视频索引
	int mVideoStream;
	int mAudioStream;
};

