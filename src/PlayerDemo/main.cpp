#include "PlayerDemo.h"
#include <QtWidgets/QApplication>

#include "DemuxClass.h"
#include "DecodeClass.h"
#include "ResampleClass.h"
#include "IAudioPlay.h"
#include "AudioThread.h"
#include "VideoThread.h"

#include "PlayThread.h"



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PlayerDemo w;
    w.show();

    PlayThread player;
    player.Open("1920_1080.mp4", w.ui.video);
    //player.Open("DeskPortal_TikTok_1440p_Ep_02.mp4", w.ui.video);
    
    player.Start();

    return a.exec();
}
