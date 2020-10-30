#include "PlayerDemo.h"
#include <qfiledialog.h>
#include <qdebug.h>
#include <qmessagebox.h>
#include "PlayThread.h"

static PlayThread player;

PlayerDemo::PlayerDemo(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    player.Start();
    startTimer(40);
}

void PlayerDemo::OpenFile()
{
    //选择文件
    QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择视频文件"));
    if (fileName.isEmpty()) {
        return;
    }
    this->setWindowTitle(fileName);
    if (!player.Open(fileName.toLocal8Bit(), this->ui.widget))
    {
        QMessageBox::information(0, "error", "open file failed!");
        return;
    }
    SetPause(player.isPause);
    //qDebug() << fileName;
}

PlayerDemo::~PlayerDemo()
{
    player.Close();
}

void PlayerDemo::timerEvent(QTimerEvent* e)
{
    if (isSliderPressed) return;

    long long total = player.totalMs;
    if (total > 0) {
        double progress = (double)player.pts / (double)total;
        int v = ui.progress->maximum() * progress;
        ui.progress->setValue(v);
       // qDebug() << "v = " << v;
    }
}

void PlayerDemo::SetPause(bool isPause)
{
    if (isPause)
        ui.isPlay->setText("播放");
    else 
        ui.isPlay->setText("暂停");
}

void PlayerDemo::PlayOrPause()
{
    bool isPause = !player.isPause;
    SetPause(isPause);
    player.SetPause(isPause);
}

void PlayerDemo::SliderPressed()
{
    isSliderPressed = true;
}

void PlayerDemo::SliderReleased()
{
    isSliderPressed = false;
    double progress = 0;
    progress = (double)ui.progress->value() / (double)ui.progress->maximum();

    player.Seek(progress);
}

