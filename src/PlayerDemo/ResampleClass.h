#pragma once

#include <mutex>

struct AVCodecParameters;
struct AVFrame;
struct SwrContext;

class ResampleClass
{
public:
	//输出参数和输入参数一致除了采样格式，输出为S16 ,会释放para
	virtual bool Open(std::shared_ptr<AVCodecParameters> para);
	virtual void Close();

	//返回重采样后大小,不管成功与否都释放indata空间
	virtual int Resample(std::shared_ptr<AVFrame> indata, unsigned char* data);

	ResampleClass();
	~ResampleClass();

	//AV_SAMPLE_FMT_S16
	int outFormat = 1;
protected:
	std::mutex mMtx;
	SwrContext* mSwrCtx = nullptr;
};

