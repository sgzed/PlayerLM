#pragma once

#include <mutex>

struct AVCodecParameters;
struct AVFrame;
struct SwrContext;

class ResampleClass
{
public:
	//����������������һ�³��˲�����ʽ�����ΪS16 ,���ͷ�para
	virtual bool Open(std::shared_ptr<AVCodecParameters> para);
	virtual void Close();

	//�����ز������С,���ܳɹ�����ͷ�indata�ռ�
	virtual int Resample(std::shared_ptr<AVFrame> indata, unsigned char* data);

	ResampleClass();
	~ResampleClass();

	//AV_SAMPLE_FMT_S16
	int outFormat = 1;
protected:
	std::mutex mMtx;
	SwrContext* mSwrCtx = nullptr;
};

