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

	//�ռ���Ҫ�������ͷ� ���ͷ�AVPacket����ռ䣬�����ݿռ� av_packet_free
	virtual std::shared_ptr<AVPacket> Read();

	bool IsAudio(std::shared_ptr<AVPacket> pkt);

	//��ȡ��Ƶ����  ���صĿռ���Ҫ����  avcodec_parameters_free
	virtual std::shared_ptr<AVCodecParameters> CopyVPara();

	//��ȡ��Ƶ����  ���صĿռ���Ҫ���� avcodec_parameters_free
	virtual std::shared_ptr<AVCodecParameters> CopyAPara();

	//seek λ�� pos 0.0 ~1.0
	virtual bool Seek(double pos);

	//��ն�ȡ����
	virtual void Clear();
	virtual void Close();

	//ý����ʱ�������룩
	int totalMs = 0;

protected:

	std::mutex mMtx;
	//���װ������
	AVFormatContext* mFmtCtx = nullptr;
	//����Ƶ����
	int mVideoStream;
	int mAudioStream;
};

