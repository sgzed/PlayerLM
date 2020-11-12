#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include "D3DVideoWidget.h"
#include "PlayThread.h"

class Player : public QObject
{
    Q_OBJECT
public:
    explicit Player(QObject *parent = nullptr);
    ~Player();

    //设置视口Rect
    void SetViewportRect(QVector<QRect>& rects);

    //打开并播放
    bool SetCurrentMedia(QString url,double progess = 0);

    //下一个播放的文件
    void SetNextMedia(QString url);

    void SetPause(bool isPause);

    bool IsPause();

    void Seek(double progress);

    long long GetDuration();

    long long GetCurrentTime();

    void SetVolume(double volume);

signals:

protected:
    PlayThread m_player;
    D3DVideoWidget m_d3dWidget;
};

#endif // PLAYER_H
