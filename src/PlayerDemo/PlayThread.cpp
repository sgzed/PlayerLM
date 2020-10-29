#include "PlayThread.h"
#include "DemuxClass.h"
#include "AudioThread.h"
#include "VideoThread.h"
#include <iostream>

using std::cout;
using std::endl;

bool PlayThread::Open(const char* url, IVideoCall* call)
{
	if (url == 0 || url[0] == '\0')
		return false;

	std::lock_guard<std::mutex> lck(mux);

	if (!demux) demux = new DemuxClass();
	if (!vt) vt = new VideoThread();
	if (!at) at = new AudioThread();

	//打开解封装
	bool re = demux->Open(url);
	if (!re) {
		cout << "demux->Open(url) failed!" << endl;
		return false;
	}
	totalMs = demux->totalMs;
	vt->timeBase = demux->mVideoTimeBase;
	vt->timeBase = demux->mAudioTimeBase;

	//打开视频解码器和处理线程
	if (!vt->Open(demux->CopyVPara(), call)) {
		re = false;
		cout << "vt->Open failed!" << endl;
	}
	//打开音频解码器和处理线程
	if (!at->Open(demux->CopyAPara(), demux->sampleRate, demux->channels)) {
		re = false;
		cout << "at->Open failed!" << endl;
	}
	
	cout << "PlayThread::Open " << re << endl;
	return re;
}

void PlayThread::Start()
{
	std::lock_guard<std::mutex> lck(mux);

	//启动当前线程
	QThread::start();
	if (vt)vt->start();
	if (at)at->start();

}

void PlayThread::Close()
{
	isExit = true;
	wait();
	if (vt) vt->Close();
	if (at) at->Close();
	std::lock_guard<std::mutex> lck(mux);
	delete vt;
	delete at;
	vt = nullptr;
	at = nullptr;
}

void PlayThread::Clear()
{
	std::lock_guard<std::mutex> lck(mux);
	if (demux) demux->Clear();
	if (vt) vt->Clear();
	if (at) at->Clear();
}

void PlayThread::Seek(double progress)
{
	//清理缓冲
	Clear();

	std::lock_guard<std::mutex> lck(mux);
	//暂停
	bool status = isPause;
	if(!isPause)	SetPause(true);
	
	long long seekPts = 0;

	if (!demux) {
		return;
	}
	demux->Seek(progress);
	//实际要显示的pts
	seekPts = demux->totalMs;

	while (!isExit) {
		auto pkt = demux->ReadVideo();
		if (!pkt) break;

		if (vt->RepaintPts(pkt, seekPts)) {
			this->pts = seekPts;
			break;
		}
	}

	if(!status)	SetPause(false);

}

void PlayThread::run()
{
	while (!isExit)  {
		
		if (isPause) {
			msleep(5);
			continue;
		}

		mux.lock();
		if (!demux) {
			mux.unlock();
			msleep(5);
			continue;
		}
		
		//同步
		if (vt && at) {
			vt->synPts = at->pts;
		}
		
		auto pkt = demux->Read();
		if (!pkt) {
			mux.unlock();
			msleep(5);
			continue;
		}
		
		//判断数据是音频
		if (demux->IsAudio(pkt)) {
			if (at) at->Push(pkt);
			
		}
		else {    //视频
			if (vt) vt->Push(pkt);
			//msleep(10);
		}

		mux.unlock();
		msleep(1);
	}
}

PlayThread::PlayThread()
{
}

PlayThread::~PlayThread()
{
	isExit = true;
	wait();
}

void PlayThread::SetPause(bool isPause)
{
	//std::lock_guard<std::mutex> lck(mux);
	this->isPause = isPause;

	if (at) at->SetPause(isPause);
	if (vt) vt->SetPause(isPause);

}
