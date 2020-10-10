#include "DecodeClass.h"

extern "C"
{
#include<libavcodec/avcodec.h>
}
#include <iostream>
using namespace std;


static void freeFrame(AVFrame* frame) {
	av_frame_free(&frame);
}

bool DecodeClass::Open(std::shared_ptr<AVCodecParameters> para)
{
    if (!para) {
        return false;
    }
    
    Close();

	///找到解码器
	AVCodec* codec = avcodec_find_decoder(para->codec_id);
	if (!codec) {
		cout << "can't find the codec id " << para->codec_id << endl;
		return false;
	}
	cout << "find the AVCodec " << para->codec_id << endl;

	lock_guard<mutex> lck(mMtx);

	mCodecCtx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(mCodecCtx, para.get());

	//8线程解码
	mCodecCtx->thread_count = 8;

	///打开解码器上下文
	int ret = avcodec_open2(mCodecCtx, 0, 0);
	
	if (ret != 0) {
		avcodec_free_context(&mCodecCtx);
		char buf[1024] = { 0 };
		av_strerror(ret, buf, sizeof(buf) - 1);
		cout << "avcodec_open2  failed! :" << buf << endl;
		return false;
	}

	cout << " avcodec_open2 success!" << endl;
	return true;
}

//发送到解码线程
bool DecodeClass::Send(std::shared_ptr<AVPacket> pkt)
{
	//容错处理
	if (!pkt || pkt->size <= 0 || !pkt->data) {
		return false;
	}

	lock_guard<mutex> lck(mMtx);

	if (!mCodecCtx) {
		return false;
	}

	int ret = avcodec_send_packet(mCodecCtx, pkt.get());
	if (ret != 0) {
		return false;
	}
	return true;
}

shared_ptr<AVFrame> DecodeClass::Recv()
{
	
	lock_guard<mutex> lck(mMtx);
	if (!mCodecCtx) {
		return nullptr;
	}

	shared_ptr<AVFrame> frame(av_frame_alloc(), freeFrame);
	int ret = avcodec_receive_frame(mCodecCtx, frame.get());
	
	if (ret != 0) {
		return nullptr;
	}

	cout << "[" << frame->linesize[0] << "] " << flush;
	return frame;
}

void DecodeClass::Close()
{
	lock_guard<mutex> lck(mMtx);
	if (mCodecCtx) {
		avcodec_close(mCodecCtx);
		avcodec_free_context(&mCodecCtx);
	}
}

void DecodeClass::Clear()
{
	lock_guard<mutex> lck(mMtx);
	if (mCodecCtx) {
		avcodec_flush_buffers(mCodecCtx);
	}
}

DecodeClass::DecodeClass()
{
}

DecodeClass::~DecodeClass()
{
}
