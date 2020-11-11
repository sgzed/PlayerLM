#include "transparentwidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
#include <qtimer.h>

static int w = 1280;
static int h = 720;

TransparentWidget::TransparentWidget(QWidget* parent) :
	QWidget(parent) 
{
    setWindowTitle(QString::fromLocal8Bit("TransparentWidget"));
    //setFixedSize(w, h);
    setAutoFillBackground(false); 
    setGeometry(0, 0, w, h);
   // setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlags(Qt::FramelessWindowHint | Qt::SubWindow | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AlwaysStackOnTop);

    QPushButton* button = new QPushButton("Hello world!", this);
   // button->setGeometry(5, 5, 80, 40);
}

TransparentWidget::~TransparentWidget()
{
}

void TransparentWidget::Init()
{
    t = new QTimer(this);
    t->connect(t, SIGNAL(timnout()), this, SLOT(onTimeOut()));
    t->start(100);
}

void TransparentWidget::onTimeOut()
{
    if (alpha < 0xff) {
        alpha += 20;
        update();
    }
}

void TransparentWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(255, 0, 0, 200));  //QColor最后一个参数80代表背景的透明度
    event->accept();
}

void TransparentWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_startPoint = frameGeometry().topLeft() - event->globalPos();
    }
}

void TransparentWidget::mouseMoveEvent(QMouseEvent* event)
{
    this->move(event->globalPos() + m_startPoint);
}
