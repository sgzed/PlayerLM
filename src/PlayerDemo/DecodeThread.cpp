#include "DecodeThread.h"
#include "DecodeClass.h"

DecodeThread::DecodeThread()
{
    //打开解码器
    if (!decode) decode = new DecodeClass();
}

DecodeThread::~DecodeThread()
{
	//等待线程退出
	isExit = true;
	wait();
}

void DecodeThread::Push(std::shared_ptr<AVPacket> pkt)
{
	if (!pkt)	return;

	//阻塞直到队列可用
	while (!isExit) {

		mux.lock();

		if (pkts.size() < maxList) {
			pkts.push_back(pkt);
			mux.unlock();
			break;
		}
		mux.unlock();
		msleep(1);
	}
}

std::shared_ptr<AVPacket> DecodeThread::Pop()
{
	std::lock_guard<std::mutex> lck(mux);
	if (pkts.empty()) {
		return nullptr;
	}
	auto pkt = pkts.front();
	pkts.pop_front();
	return pkt;
}

void DecodeThread::Clear()
{
	decode->Clear();
	std::lock_guard<std::mutex> lck(mux);
	if (!pkts.empty()){
		pkts.clear();
	}
}

void DecodeThread::Close()
{
	Clear();
	//等待线程退出
	isExit = true;
	wait();
	decode->Close();

	std::lock_guard<std::mutex> lck(mux);
	delete decode;
	decode = nullptr;
}
