#pragma once
#include <mutex>

class IAudioPlay
{
public:
	int sampleRate = 44100;
	int sampleSize = 16;
	int channels = 2;

	//打开音频播放
	virtual bool Open() = 0;
	virtual void Close() = 0;
	virtual void Clear() = 0;

	//播放音频
	virtual bool Write(const unsigned char* data, int datasize) = 0;
	virtual int GetFree() = 0;
	//设置音量
	virtual void setVolume(double volume) = 0;

	virtual void SetPause(bool isPause) = 0;
	virtual long long GetNoPlayPts() = 0;

	static IAudioPlay* Get();
	IAudioPlay();
	virtual ~IAudioPlay();
};

class QAudioOutput;
class QIODevice;

class CAudioPlay : public IAudioPlay
{
private:

	QAudioOutput* output = nullptr;
	QIODevice* io = nullptr;

public:

	virtual bool Open() override;
	virtual void Close() override;
	virtual bool Write(const unsigned char* data, int datasize) override;
	virtual void setVolume(double volume) override;
	virtual int GetFree() override;
	virtual long long GetNoPlayPts() override;
	virtual void SetPause(bool isPause) override;
	virtual void Clear() override;

private:
	std::mutex mMtx;
};
