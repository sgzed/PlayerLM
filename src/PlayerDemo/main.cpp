#include "PlayerDemo.h"
#include <QtWidgets/QApplication>

#include "DemuxClass.h"
#include "DecodeClass.h"
#include "ResampleClass.h"
#include "IAudioPlay.h"
#include <iostream>

#include <QThread>
using namespace std;

class TestThread :public QThread
{
public:
	void Init()
	{
		//char* url = "1920_1080.mp4";
		char* url = "DeskPortal_TikTok_1440p_Ep_02.mp4";

		cout << "demux.Open = " << demux.Open(url);
		demux.Read();
		demux.Clear();
		demux.Close();
		
		cout << "demux.Open = " << demux.Open(url);
		cout << "CopyVPara = " << demux.CopyVPara() << endl;
		cout << "CopyAPara = " << demux.CopyAPara() << endl;
		//cout << "seek=" << demux.Seek(0.95) << endl;

		/////////////////////////////

		cout << "vdecode.Open() = " << vdecode.Open(demux.CopyVPara()) << endl;
		//vdecode.Clear();
		//vdecode.Close();
		cout << "adecode.Open() = " << adecode.Open(demux.CopyAPara()) << endl;
		cout << "resample.Open() = " << resample.Open(demux.CopyAPara()) << endl;

		IAudioPlay::Get()->channels = demux.channels;
		IAudioPlay::Get()->sampleRate = demux.sampleRate;
		cout << "XAudioPlay::Get()->Open() = " << IAudioPlay::Get()->Open() << endl;

	}

	unsigned char* pcm = new unsigned char[1024 * 1024];

	void run()
	{
		//初始化显示
		for (;;)
		{
			auto pkt = demux.Read();
			if (!pkt)break;

			if (demux.IsAudio(pkt)) {
				adecode.Send(pkt);
				auto frame = adecode.Recv();
				int len = resample.Resample(frame, pcm);
				//cout << "Resample:" << len << " ";
				while (len > 0)
				{
					if (IAudioPlay::Get()->GetFree() >= len)
					{
						IAudioPlay::Get()->Write(pcm, len);
						break;
					}
					msleep(1);
				}
			}
			else {
				vdecode.Send(pkt);
				auto frame = vdecode.Recv();
				//video->Repaint(frame);
				//msleep(40);
			}
		}
		
	}
	///测试XDemux
	DemuxClass demux;
	///解码测试
	DecodeClass vdecode;
	DecodeClass adecode;
	ResampleClass resample;
	XVideoWidget* video;

};


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PlayerDemo w;
    w.show();

	TestThread tt;
	tt.Init();

    //初始化gl窗口
    w.ui.video->Init(tt.demux.width, tt.demux.height);
	tt.video = w.ui.video;
	tt.start();

    return a.exec();
}
