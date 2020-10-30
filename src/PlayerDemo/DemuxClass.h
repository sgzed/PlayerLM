#pragma once

#include <mutex>
#include <memory>

struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;
struct AVFrame;

struct AVStream;

class DemuxClass
{
public:
	DemuxClass();
	~DemuxClass();

	virtual bool Open(const char* url);

	//空间需要调用者释放 ，释放AVPacket对象空间，和数据空间 av_packet_free
	virtual std::shared_ptr<AVPacket> Read();

	virtual std::shared_ptr<AVPacket> ReadVideo();

	bool IsAudioExist();

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

	//AVPixelFormat GetHwFormat(AVCodecContext* ctx, const AVPixelFormat* pix_fmts);

	//打开Cuda硬解码器
	//bool GetCudaDecoder(AVStream* stream);

	//媒体总时长（毫秒）
	int totalMs = 0;

	//读取packet结束
	bool bFinished = false;

	int width = 0;
	int height = 0;

	int sampleRate = 44100;
	int channels = 2;

	double frameRate = 0;

	double mVideoTimeBase = 0;
	double mAudioTimeBase = 0;
	//AVCodec* find_codec = nullptr;

protected:

	std::mutex mMtx;
	//音视频索引
	int mVideoStream;
	int mAudioStream;
	//解封装上下文
	AVFormatContext* mFmtCtx = nullptr;
};

