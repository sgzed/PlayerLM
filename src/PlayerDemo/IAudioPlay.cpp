#include "IAudioPlay.h"
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>

IAudioPlay* IAudioPlay::Get()
{
    static CAudioPlay player;
    return &player;
}

IAudioPlay::IAudioPlay()
{
}

IAudioPlay::~IAudioPlay()
{
}

bool CAudioPlay::Open()
{
    Close();
	QAudioFormat fmt;
	fmt.setSampleRate(sampleRate);
	fmt.setSampleSize(sampleSize);
	fmt.setChannelCount(channels);
	fmt.setCodec("audio/pcm");
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::SignedInt);
	
	QAudioDeviceInfo deviceInfo = QAudioDeviceInfo::defaultOutputDevice();

	if (!deviceInfo.isFormatSupported(fmt)) {
		fmt = deviceInfo.nearestFormat(fmt);
	}

	std::lock_guard<std::mutex> lck(mMtx);

	output = new QAudioOutput(deviceInfo,fmt);

	io = output->start(); //开始播放
	return io != nullptr;
}

void CAudioPlay::Close()
{
	std::lock_guard<std::mutex> lck(mMtx);

	if (io) {
		io->close();
		io = nullptr;
	}

	if (output) {
		output->stop();
		delete output;
		output = nullptr;
	}
}

bool CAudioPlay::Write(const unsigned char* data, int datasize)
{
	if (!data || datasize <= 0) return false;
	mMtx.lock();
	if (!output || !io)
	{
		mMtx.unlock();
		return false;
	}
	int size = io->write((char*)data, datasize);
	mMtx.unlock();
	if (datasize != size)
		return false;
	return true;
}

void CAudioPlay::setVolume(double volume)
{
	std::lock_guard<std::mutex> lck(mMtx);

	if (output) {
		output->setVolume(volume);
	}
}

int CAudioPlay::GetFree()
{
	std::lock_guard<std::mutex> lck(mMtx);

	if (!output) {
		return 0;
	}

	int free = output->bytesFree();
	return free;
}

long long CAudioPlay::GetNoPlayPts()
{
	std::lock_guard<std::mutex> lck(mMtx);
	if (!output) return 0;

	long long pts = 0;

	//未播放的字节数
	int size = output->bufferSize() - output->bytesFree();
	//bytes per second
	int secSize = sampleRate * (sampleSize / 8) * channels;
	if (secSize <= 0) {
		pts = 0;
	}
	else {
		pts = ((double)size / secSize) * 100;
	}
	return pts;
}

void CAudioPlay::SetPause(bool isPause)
{
	std::lock_guard<std::mutex> lck(mMtx);
	if (!output) return;
	
	if (isPause) {
		output->suspend();
	}
	else {
		output->resume();
	}
}

void CAudioPlay::Clear()
{
	std::lock_guard<std::mutex> lck(mMtx);
	if (!io) return;
	io->reset();
}
