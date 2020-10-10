#pragma once

struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

#include <mutex>
#include <memory>

class DecodeClass
{
public:

	bool isAudio = false;

	//�򿪽�����,���ܳɹ�����ͷ�para�ռ�
	virtual bool Open(std::shared_ptr<AVCodecParameters> para);

	//���͵������߳�
	virtual bool Send(std::shared_ptr<AVPacket> pkt);

	//��ȡ�������ݣ�һ��send������Ҫ���Recv����ȡ�����е�����Send NULL��Recv���
	//ÿ�θ���һ�ݣ��ɵ������ͷ� av_frame_free
	virtual std::shared_ptr<AVFrame> Recv();

	virtual void Close();
	virtual void Clear();

	DecodeClass();
	virtual ~DecodeClass();

protected:
	AVCodecContext* mCodecCtx = 0;
	std::mutex mMtx;
};

