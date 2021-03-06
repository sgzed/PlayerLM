#pragma once

#include <QtWidgets/QWidget>
#include "ui_PlayerDemo.h"
#include "VideoWidget.h" 
#include "Player.h"

class PlayerDemo : public QWidget
{
    Q_OBJECT

public:
    PlayerDemo(QWidget *parent = Q_NULLPTR);
    ~PlayerDemo();

    void timerEvent(QTimerEvent* e);

    void SetPause(bool isPause);

    //void resizeEvent(QResizeEvent* e);

public slots:
    void OpenFile();
    void PlayOrPause();

    void SliderPressed();
    void SliderReleased();
    void VolumeChanged(int value);

   
private:
    bool isSliderPressed;
    
    Ui::PlayerDemoClass ui;

    //VideoWidget vid;
    Player player;

};
