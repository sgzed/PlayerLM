#pragma once

#include <QThread>
#include <mutex>
#include <list>

struct AVPacket;
class DecodeClass;

class DecodeThread :public QThread
{
public:
	DecodeThread();
	virtual ~DecodeThread();

	//生产者
	virtual void Push(std::shared_ptr<AVPacket> pkt);
	//取出一帧数据，并出栈，如果没有返回NULL
	virtual std::shared_ptr<AVPacket> Pop();

	virtual bool PktsEmpty();

	//设置decode的时间基
	virtual void SetTimeBase(double timeBase);

	//清理队列
	virtual void Clear();
	//清理资源，停止线程
	virtual void Close();

	//最大队列
	int maxList = 500;
	bool isExit = false;

protected:
	DecodeClass* decode = 0;
	std::list<std::shared_ptr<AVPacket>> pkts;
	std::mutex mux;
};

