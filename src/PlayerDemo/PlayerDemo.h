#pragma once

#include <QtWidgets/QWidget>
#include "ui_PlayerDemo.h"

class PlayerDemo : public QWidget
{
    Q_OBJECT

public:
    PlayerDemo(QWidget *parent = Q_NULLPTR);

private:
    Ui::PlayerDemoClass ui;
};
