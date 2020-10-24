#include "VideoThread.h"
#include "DecodeClass.h"

bool VideoThread::Open(std::shared_ptr<AVCodecParameters> para, IVideoCall* call, int width, int height)
{
    if (!para) return false;
	Clear();

    std::lock_guard<std::mutex> lck(vmux);
    this->call = call;
    if (this->call) {
        this->call->Init(width, height);
    }

    return decode->Open(para);
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
	
		if (synPts >0 &&  synPts < decode->pts) {
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
			
			if (call) {
				call->Repaint(frame);
			}
		}
		vmux.unlock();
	}
}

VideoThread::VideoThread()
{
}

VideoThread::~VideoThread()
{
}

void VideoThread::SetPause(bool isPause)
{
	std::lock_guard<std::mutex> lck(vmux);
	this->isPause = isPause;
}
