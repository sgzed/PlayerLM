#include "VideoThread.h"
#include "DXVADecode.h"
#include "D3DVideoWidget.h"

bool VideoThread::Open(std::shared_ptr<AVCodecParameters> para)
{
    if (!para) return false;
	Clear();

    std::lock_guard<std::mutex> lck(vmux);
	synPts = 0;

	if (decode->Open(para)) {
		if (call) call->Init(decode);
		return true;
	}
	return false;
}

bool VideoThread::Open(std::shared_ptr<AVCodecParameters> para, IVideoCall* call)
{
	if (!para) return false;
	Clear();

	std::lock_guard<std::mutex> lck(vmux);
	this->call = call;
	synPts = 0;

	if (decode->Open(para)) {
		if (call) {
			call->Init(decode);
		}
		return true;
	}
	return false;
}

bool VideoThread::RepaintPts(std::shared_ptr<AVPacket> pkt, long long seekPts)
{
	std::lock_guard<std::mutex> lck(vmux);

	bool ret = decode->Send(pkt);
	if (!ret) return true;
	
	auto frame = decode->Recv();
	if (!frame) return false;

	if (decode->pts >= seekPts) {
		if(call)	call->Repaint(frame);
		return true;
	}
	return false;
}

void VideoThread::Clear()
{
	DecodeThread::Clear();
	lastPts = 0;
}

void VideoThread::run()
{
	bool ret = false;
	int size = 0;

	while (!isExit) {

		vmux.lock();

		if (this->isPause) {
			vmux.unlock();
			msleep(5);
			continue;
		}

		if (synPts >0 && synPts < decode->pts) {
			vmux.unlock();
			msleep(1);
			continue;
		}

		auto pkt = Pop();
		ret = decode->Send(pkt);

		if (!ret) {
			vmux.unlock();
			msleep(1);
			continue;
		}

		while (!isExit) {
			auto frame = decode->Recv();
			if (!frame)	break;

			if (synPts == 0) {
				vmux.unlock();
				msleep(decode->pts - lastPts);
				lastPts = decode->pts;
				vmux.lock();
			}
				
			if (call) {
				call->Repaint(frame);
			}
		}

		pts = decode->pts;
		vmux.unlock();
	}
}

VideoThread::VideoThread()
{
	if (decode) {
		delete decode;
		decode = new DXVADecode();
	}
}

VideoThread::~VideoThread()
{
}

void VideoThread::SetPause(bool isPause)
{
	std::lock_guard<std::mutex> lck(vmux);
	this->isPause = isPause;
}
