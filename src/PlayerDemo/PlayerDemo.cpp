#include "PlayerDemo.h"
#include <qfiledialog.h>
#include <qdebug.h>
#include <qmessagebox.h>
#include "D3DVideoWidget.h"
#include "PlayThread.h"
#include <qaudio.h>

static PlayThread player;

static D3DVideoWidget widget;

PlayerDemo::PlayerDemo(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    player.Start();

    QVector<QRect> rects;
    rects.append(QRect(0, 0, 1920, 1080));
    rects.append(QRect(1920, 0, 1920, 1080));
    player.SetViewportRect(rects);
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
    if (!player.Open(fileName.toLocal8Bit(), &widget))
    {
        QMessageBox::information(0, "error", "open file failed!");
        return;
    }
    SetPause(player.IsPause());

    player.SetNextMedia(fileName);
    //qDebug() << fileName;
}

PlayerDemo::~PlayerDemo()
{
    player.Close();
}

void PlayerDemo::timerEvent(QTimerEvent* e)
{
    if (isSliderPressed) return;

    static bool first = true;

    long long total = player.GetDuration();
    if (total > 0) {
        double progress = (double)player.GetCurrentTime() / (double)total;
        int v = ui.progress->maximum() * progress;
        ui.progress->setValue(v);
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
    bool isPause = !player.IsPause();
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

void PlayerDemo::VolumeChanged(int value)
{
    qreal linearVolume = QAudio::convertVolume(value / qreal(100),
        QAudio::LogarithmicVolumeScale,
        QAudio::LinearVolumeScale);

     player.SetVolume(linearVolume);
}

