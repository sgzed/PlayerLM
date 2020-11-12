#include "AudioThread.h"
#include "DecodeClass.h"
#include "IAudioPlay.h"
#include "ResampleClass.h"

#include <iostream>
using namespace std;

bool AudioThread::Open(std::shared_ptr<AVCodecParameters> para, int sampleRate, int channels)
{
	if (!para) return false;
	Clear();
	
	std::lock_guard<std::mutex> lck(amux);
	pts = 0;
	
	bool re = true;

	if (!res->Open(para)) {
		cout << "XResample open failed!" << endl;
		re = false;
	}
	ap->sampleRate = sampleRate;
	ap->channels = channels;
	if (!ap->Open()) {
		re = false;
		cout << "IAudioPlay open failed!" << endl;
	}
	if (!decode->Open(para)) {
		cout << "audio XDecode open failed!" << endl;
		re = false;
	}
	
	cout << "XAudioThread::Open :" << re << endl;
	return re;
}

void AudioThread::Close()
{
	DecodeThread::Close();
	if (res) {
		res->Close();
		amux.lock();
		delete res;
		res = NULL;
		amux.unlock();
	}

	if (ap) {
		ap->Close();
		amux.lock();
		ap = NULL;
		amux.unlock();
	}
}

void AudioThread::Clear()
{
	DecodeThread::Clear();
	//ap一定存在,不需要锁
	//std::lock_guard<std::mutex> lck(amux);
	if (ap)	ap->Clear();
}

void AudioThread::SetVolume(double volume)
{
	if (ap) {
		ap->setVolume(volume);
	}
}

double AudioThread::GetVolume()
{
	if (ap) {
		return ap->GetVolume();
	}
	return 0.0;
}

void AudioThread::SetPause(bool isPause)
{
	//std::lock_guard<std::mutex> lck(amux);
	this->isPause = isPause;
	if (ap)	ap->SetPause(isPause);
}

void AudioThread::run()
{
	bool ret = false;
	int size = 0;
	unsigned char* pcm = new unsigned char[1024 * 1024];

	while (!isExit) {

		amux.lock();

		if (this->isPause) {
			amux.unlock();
			msleep(5);
			continue;
		}

		auto pkt = Pop();
		ret = decode->Send(pkt);
		if (!ret) {
			amux.unlock();
			msleep(1);
			continue;
		}

		while (!isExit) {
			auto frame = decode->Recv();
			if (!frame)	break;

			pts = decode->pts - ap->GetNoPlayPts();
			
			//重采样
			size = res->Resample(frame, pcm);
			//播放
			while (!isExit) {
				if (size <= 0) break;

				if (ap->GetFree() < size || isPause) {
					msleep(1);
					continue;
				}
				
				ap->Write(pcm, size);
				break;
			}
		}
		amux.unlock();
	}
	delete[] pcm;
}

AudioThread::AudioThread()
{
	if (!res) res = new ResampleClass();
	if (!ap) ap = IAudioPlay::Get();
}

AudioThread::~AudioThread()
{
	isExit = true;
	//等待线程退出
	wait();
}
