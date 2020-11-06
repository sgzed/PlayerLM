#include "PlayerDemo.h"
#include <QtWidgets/QApplication>

#include <qdialog.h>
#include "D3DVideoWidget.h"
#include <Windows.h>
#include "PlayThread.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
     PlayerDemo w;
     w.show();

     // PlayThread player;

     // D3DVideoWidget widget;
      //widget.Init()

     // EnableWindow((HWND)widget.winId(), false);

      //player.Start();
      //player.Open("1920_1080.mp4", &widget);
     // player.Open("luming.mp4", &widget);
      //player.Open("jiami2.ndf", &widget);

      // player.Open("1920_1080.mp4", w.ui.video);
      //player.Open("DeskPortal_TikTok_1440p_Ep_02.mp4", w.ui.video);
    
    return a.exec();
}
