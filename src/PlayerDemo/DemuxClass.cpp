#include "DemuxClass.h"

#include <iostream>
using namespace std;

extern "C" {
	#include "libavformat/avformat.h"
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")

static double r2d(AVRational r) {
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

static void freePkt(AVPacket* pkt) {
	av_packet_free(&pkt);
}

static void freePara(AVCodecParameters* para) {
	avcodec_parameters_free(&para);
}

DemuxClass::DemuxClass()
{
}

DemuxClass::~DemuxClass()
{
}

bool DemuxClass::Open(const char* url)
{
	Close();

	lock_guard<mutex> lck(mMtx);

	//参数设置
	AVDictionary* opts = NULL;

	int re = avformat_open_input(&mFmtCtx, url,
		nullptr,  // 0表示自动选择解封器
		&opts //参数设置，比如rtsp的延时时间
	);

	if (re != 0){
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "open " << url << " failed! :" << buf << endl;
		return false;
	}
	cout << "open " << url << " success! " << endl;

	//获取流信息 
	re = avformat_find_stream_info(mFmtCtx, 0);

	//总时长 毫秒
	totalMs = mFmtCtx->duration / (AV_TIME_BASE / 1000);
	cout << "totalMs = " << totalMs << endl;

	//打印视频流详细信息
	av_dump_format(mFmtCtx, 0, url, 0);


	//获取视频流
	mVideoStream = av_find_best_stream(mFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	AVStream* as = mFmtCtx->streams[mVideoStream];

	cout << "=======================================================" << endl;
	cout << "视频信息" << endl;
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << "width=" << as->codecpar->width << endl;
	cout << "height=" << as->codecpar->height << endl;
	//帧率 fps 分数转换
	cout << "video fps = " << r2d(as->avg_frame_rate) << endl;

	cout << "=======================================================" << endl;
	cout << "音频信息" << endl;
	//获取音频流
	mAudioStream = av_find_best_stream(mFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	as = mFmtCtx->streams[mAudioStream];
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << "sample_rate = " << as->codecpar->sample_rate << endl;
	//AVSampleFormat;
	cout << "channels = " << as->codecpar->channels << endl;
	//一帧数据？？ 单通道样本数 
	cout << "frame_size = " << as->codecpar->frame_size << endl;

	return true;
}

shared_ptr<AVPacket> DemuxClass::Read()
{
	lock_guard<mutex> lck(mMtx);

	if (!mFmtCtx) {
		return nullptr;
	}

	shared_ptr<AVPacket> pkt(av_packet_alloc(), freePkt);

	//读取一帧，并分配空间
	int re = av_read_frame(mFmtCtx, pkt.get());
	if (re != 0) {
		return nullptr;
	}

	//pts转换为毫秒
	pkt->pts = pkt->pts * (1000 * (r2d(mFmtCtx->streams[pkt->stream_index]->time_base)));
	pkt->dts = pkt->dts * (1000 * (r2d(mFmtCtx->streams[pkt->stream_index]->time_base)));
	cout << pkt->pts << " " << flush;
	return pkt;
}

bool DemuxClass::IsAudio(std::shared_ptr<AVPacket> pkt)
{
	if (!pkt || pkt->stream_index == mVideoStream) {
		return false;
	}
	return true;
}

//获取视频参数  返回的空间需要清理  avcodec_parameters_free
std::shared_ptr<AVCodecParameters> DemuxClass::CopyVPara()
{
	lock_guard<mutex> lck(mMtx);

	if (!mFmtCtx) {
		return nullptr;
	}

	shared_ptr<AVCodecParameters> pa(avcodec_parameters_alloc(), freePara);
	avcodec_parameters_copy(pa.get(), mFmtCtx->streams[mVideoStream]->codecpar);
	return pa;
}

//获取音频参数  返回的空间需要清理  avcodec_parameters_free
std::shared_ptr<AVCodecParameters> DemuxClass::CopyAPara()
{
	lock_guard<mutex> lck(mMtx);

	if (!mFmtCtx) {
		return nullptr;
	}

	shared_ptr<AVCodecParameters> pa(avcodec_parameters_alloc(), freePara);
	avcodec_parameters_copy(pa.get(), mFmtCtx->streams[mAudioStream]->codecpar);
	return pa;
}

bool DemuxClass::Seek(double pos)
{
	lock_guard<mutex> lck(mMtx);
	if (!mFmtCtx) {
		return false;
	}

	avformat_flush(mFmtCtx);
	long long seekPos = 0;
	seekPos = mFmtCtx->streams[mVideoStream]->duration * pos;
	int ret = av_seek_frame(mFmtCtx, mVideoStream, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	if (ret < 0) {
		return false;
	}
	return true;
}

void DemuxClass::Clear()
{
	lock_guard<mutex> lck(mMtx);

	if (!mFmtCtx) {
		return;
	}
	avformat_flush(mFmtCtx);
}

void DemuxClass::Close()
{
	lock_guard<mutex> lck(mMtx);

	if (!mFmtCtx) {
		return;
	}

	avformat_close_input(&mFmtCtx);
	totalMs = 0;
}






