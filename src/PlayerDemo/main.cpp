#include "PlayerDemo.h"
#include <QtWidgets/QApplication>

#include "D3DVideoWidget.h"
#include "PlayThread.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PlayerDemo w;
    w.show();

    /*  D3DVideoWidget widget;
      widget.resize(1280,720);
      widget.show();
      player.Open("luming.mp4", &widget); */

      // player.Open("1920_1080.mp4", w.ui.video);
    //player.Open("DeskPortal_TikTok_1440p_Ep_02.mp4", w.ui.video);
    
    return a.exec();
}
