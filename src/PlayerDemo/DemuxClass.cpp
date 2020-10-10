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

	//��������
	AVDictionary* opts = NULL;

	int re = avformat_open_input(&mFmtCtx, url,
		nullptr,  // 0��ʾ�Զ�ѡ������
		&opts //�������ã�����rtsp����ʱʱ��
	);

	if (re != 0){
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "open " << url << " failed! :" << buf << endl;
		return false;
	}
	cout << "open " << url << " success! " << endl;

	//��ȡ����Ϣ 
	re = avformat_find_stream_info(mFmtCtx, 0);

	//��ʱ�� ����
	totalMs = mFmtCtx->duration / (AV_TIME_BASE / 1000);
	cout << "totalMs = " << totalMs << endl;

	//��ӡ��Ƶ����ϸ��Ϣ
	av_dump_format(mFmtCtx, 0, url, 0);


	//��ȡ��Ƶ��
	mVideoStream = av_find_best_stream(mFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	AVStream* as = mFmtCtx->streams[mVideoStream];

	cout << "=======================================================" << endl;
	cout << "��Ƶ��Ϣ" << endl;
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << "width=" << as->codecpar->width << endl;
	cout << "height=" << as->codecpar->height << endl;
	//֡�� fps ����ת��
	cout << "video fps = " << r2d(as->avg_frame_rate) << endl;

	cout << "=======================================================" << endl;
	cout << "��Ƶ��Ϣ" << endl;
	//��ȡ��Ƶ��
	mAudioStream = av_find_best_stream(mFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	as = mFmtCtx->streams[mAudioStream];
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << "sample_rate = " << as->codecpar->sample_rate << endl;
	//AVSampleFormat;
	cout << "channels = " << as->codecpar->channels << endl;
	//һ֡���ݣ��� ��ͨ�������� 
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

	//��ȡһ֡��������ռ�
	int re = av_read_frame(mFmtCtx, pkt.get());
	if (re != 0) {
		return nullptr;
	}

	//ptsת��Ϊ����
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

//��ȡ��Ƶ����  ���صĿռ���Ҫ����  avcodec_parameters_free
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

//��ȡ��Ƶ����  ���صĿռ���Ҫ����  avcodec_parameters_free
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






