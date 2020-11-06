#pragma once

#include <QThread>
#include <mutex>
#include "IVideoCall.h"
#include <qvector.h>
#include <qrect.h>

class DemuxClass;
class VideoThread;
class AudioThread;
class D3DVideoWidget;

class PlayThread : public QThread
{
	Q_OBJECT

public:
	//启动所有线程
	virtual void Start();

	//打开文件开始播放
	virtual bool Open(const char* url,IVideoCall* call = nullptr,bool init = true);

	virtual void Seek(double progress);

	void SetPause(bool isPause);

	bool IsPause() {
		return isPause;
	}

	long long GetDuration() {
		return totalMs;
	}

	long long GetCurrentTime() {
		return pts;
	}

	//下一个播放的url
	void SetNextMedia(QString url);

	void SetVolume(double volume);

	//设置视口Rect
	static void SetViewportRect(QVector<QRect>& rects);

	static QVector<QRect> GetViewportRect() {
		return m_rtViewports;
	}

	//关闭线程，清理资源
	virtual void Close();

	PlayThread();
	~PlayThread();

protected:
	void run();

	//清理资源，Fix Seek时需要等待的Bug
	virtual void Clear();

Q_SIGNALS:
	void playFinished();

public slots:
	void onPlayFinisned();

protected:

	bool isExit = false;
	long long pts = 0;
	long long totalMs = 0;

	bool isPause = false;

	std::mutex mux;

	DemuxClass* demux = 0;
	VideoThread* vt = 0;
	AudioThread* at = 0;

	QList<QString> playList;

	static QVector<QRect> m_rtViewports;
};

