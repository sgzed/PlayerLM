#include "ResampleClass.h"

extern "C" {
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}
#pragma comment(lib,"swresample.lib")
#include <iostream>
using namespace std;

bool ResampleClass::Open(std::shared_ptr<AVCodecParameters> para)
{
    if (!para) {
        return false;
    }

	int ret = 0;

	{
		lock_guard<mutex> lck(mMtx);

		//���actxΪNULL�����ռ�
		mSwrCtx = swr_alloc_set_opts(mSwrCtx,
			av_get_default_channel_layout(2),	//�����ʽ
			(AVSampleFormat)outFormat,			//���������ʽ 1 AV_SAMPLE_FMT_S16
			para->sample_rate,					//���������
			av_get_default_channel_layout(para->channels),//�����ʽ
			(AVSampleFormat)para->format,
			para->sample_rate,
			0, 0
		);

		ret = swr_init(mSwrCtx);
	}
	
	if (ret != 0) {
		char buf[1024] = { 0 };
		av_strerror(ret, buf, sizeof(buf) - 1);
		cout << "swr_init  failed! :" << buf << endl;
		return false;
	}

    return true;
}

void ResampleClass::Close()
{
	lock_guard<mutex> lck(mMtx);
	if (mSwrCtx) {
		swr_free(&mSwrCtx);
	}
}

int ResampleClass::Resample(std::shared_ptr<AVFrame> indata, unsigned char* d)
{
	if (!indata || !d) {
		return 0;
	}

	uint8_t* data[2] = { 0 };
	data[0] = d;
	int re = swr_convert(mSwrCtx, data, indata->nb_samples,		//���
		(const uint8_t**)indata->data, indata->nb_samples	//����
	);
	if (re <= 0) {
		return re;
	}

	int outSize = re * indata->channels * av_get_bytes_per_sample((AVSampleFormat)outFormat);
	return outSize;
}

ResampleClass::ResampleClass()
{
}

ResampleClass::~ResampleClass()
{
}
