#include "DemuxClass.h"
#include <qfile.h>

#include <iostream>
using namespace std;

extern "C" {
	#include "libavformat/avformat.h"
	#include "libavformat/avio.h"
	#include <libavutil/file.h>
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")

#define BUFFE_OFFECT 2

static double r2d(AVRational r) {
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

static void freePkt(AVPacket* pkt) {
	av_packet_free(&pkt);
}

static void freePara(AVCodecParameters* para) {
	avcodec_parameters_free(&para);
}

int read_buffer(void* opaque, uint8_t* buf, int buf_size) {
	FILE* fp_open = (FILE*)opaque;
	if (!feof(fp_open)) {
		int true_size = fread(buf, 1, buf_size, fp_open);
		return true_size;
	}
	else {
		return -1;
	}
}

static int64_t seek(void* opaque, int64_t offset, int whence) {
	FILE* fp_open = (FILE*)opaque;
	if (whence == SEEK_SET && fseek(fp_open, offset + BUFFE_OFFECT, SEEK_SET) == 0) {
		return offset;
	}
	// handling AVSEEK_SIZE doesn't seem mandatory
	return -1;
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

	QString file(url);
	int re = 0;
	lock_guard<mutex> lck(mMtx);

	if (file.endsWith(".ndf")) {
		m_pFile = fopen(url, "rb+");
		if (!m_pFile) {
			return false;
		}
		//解密:偏移BUFFE_OFFECT字节
		fseek(m_pFile, BUFFE_OFFECT, 0);

		if (!(mFmtCtx = avformat_alloc_context())) {
			return false;
		}

		avio_ctx_buffer = (uint8_t*)av_malloc(avio_ctx_buffer_size);
		if (!avio_ctx_buffer) {
			return false;
		}

		avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
			0, m_pFile, read_buffer, NULL, &seek);
		
		if (!avio_ctx) {
			return false;
		}

		mFmtCtx->pb = avio_ctx;
	}

	//参数设置
	AVDictionary* opts = NULL;

	re = avformat_open_input(&mFmtCtx, url,
		nullptr,  // 0表示自动选择解封器
		&opts //参数设置，比如rtsp的延时时间
	);

	if (re != 0) {
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
	mVideoTimeBase = av_q2d(as->time_base);

	width = as->codecpar->width;
	height = as->codecpar->height;

	cout << "=======================================================" << endl;
	cout << "视频信息" << endl;
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << "width=" << as->codecpar->width << endl;
	cout << "height=" << as->codecpar->height << endl;
	//帧率 fps 分数转换
	cout << "video fps = " << (frameRate = r2d(as->avg_frame_rate)) << endl;

	cout << "=======================================================" << endl;
	cout << "音频信息" << endl;

	//获取音频流
	mAudioStream = av_find_best_stream(mFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

	//不存在音频,视频怎么同步
	if (mAudioStream >= 0) {

		as = mFmtCtx->streams[mAudioStream];

		mAudioTimeBase = av_q2d(as->time_base);
		sampleRate = as->codecpar->sample_rate;
		channels = as->codecpar->channels;

		cout << "codec_id = " << as->codecpar->codec_id << endl;
		cout << "format = " << as->codecpar->format << endl;
		cout << "sample_rate = " << as->codecpar->sample_rate << endl;
		//AVSampleFormat;
		cout << "channels = " << as->codecpar->channels << endl;
		//一帧数据？？ 单通道样本数 
		cout << "frame_size = " << as->codecpar->frame_size << endl;
	}

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
		if (re == AVERROR_EOF) {
			bFinished = true;
		}
		return nullptr;
	}

	//pts转换为毫秒,后面AVFrame的时间就不需要做转换，已验证
	pkt->pts = pkt->pts * (1000 * (r2d(mFmtCtx->streams[pkt->stream_index]->time_base)));
	pkt->dts = pkt->dts * (1000 * (r2d(mFmtCtx->streams[pkt->stream_index]->time_base)));
	return pkt;
}

std::shared_ptr<AVPacket> DemuxClass::ReadVideo()
{
	//lock_guard<mutex> lck(mMtx);

	if (!mFmtCtx) {
		return nullptr;
	}

	shared_ptr<AVPacket> pkt =nullptr;

	for (int i = 0; i < 20; ++i) {
		pkt = Read();
		if (!pkt || pkt->stream_index == mVideoStream) {
			break;
		}
		else {
			pkt.reset();
			continue;
		}
	}
	return pkt;
}

bool DemuxClass::IsAudioExist()
{
	return mAudioStream >= 0;
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
	if (!mFmtCtx)	return;
	
	avformat_flush(mFmtCtx);
	bFinished = false;
}

void DemuxClass::Close()
{
	lock_guard<mutex> lck(mMtx);
	if (!mFmtCtx)	return;
	
	avformat_close_input(&mFmtCtx);
	totalMs = 0;
	bFinished = false;
}

//AVPixelFormat DemuxClass::GetHwFormat(AVCodecContext* ctx, const AVPixelFormat* pix_fmts)
//{
//	const enum AVPixelFormat* p;
//
//	for (p = pix_fmts; *p != -1; p++) {
//		if (*p == mFmtCtx->streams[mVideoStream].hw_pix_fmt) {
//			return *p;
//		}
//	}
//
//	fprintf(stderr, "Failed to get HW surface format.\n");
//	return AV_PIX_FMT_NONE;
//
//}

//bool DemuxClass::GetCudaDecoder(AVStream* stream)
//{
//	if (!mFmtCtx) {
//		return false;
//	}
//
//	int ret = av_find_best_stream(mFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &find_codec, 0);
//	if (ret < 0) {
//		return false;
//	}
//
//	for (int i = 0;; i++) {
//		const AVCodecHWConfig* config = avcodec_get_hw_config(find_codec, i);
//		if (!config) {
//			// 没找到cuda解码器，不能使用;
//			return false;
//		}
//		if (config->device_type == AV_HWDEVICE_TYPE_CUDA) {
//			// 找到了cuda解码器，记录对应的AVPixelFormat,后面get_format需要使用;
//			m_AVStreamInfo.hw_pix_fmt = config->pix_fmt;
//			m_AVStreamInfo.device_type = AV_HWDEVICE_TYPE_CUDA;
//			break;
//		}
//	}
//	AVCodecContext* decoder_ctx = avcodec_alloc_context3(find_codec);
//
//
//	return true;
//}






