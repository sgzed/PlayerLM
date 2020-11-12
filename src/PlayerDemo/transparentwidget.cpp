#include "transparentwidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
#include <qtimer.h>
#include <qlabel.h>

static int w = 1280;
static int h = 720;

TransparentWidget::TransparentWidget(QWidget* parent) :
	QWidget(parent) 
{
    setWindowTitle(QString::fromLocal8Bit("TransparentWidget"));
    //setFixedSize(w, h);

    setGeometry(0, 0, w, h);
    setWindowFlag(Qt::FramelessWindowHint);
   // setWindowFlags(Qt::FramelessWindowHint | Qt::SubWindow | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
   
    QPushButton* button = new QPushButton("Hello world!", this);
    button->setGeometry(5, 5, 80, 40);

    label = new QLabel(this);
    label->setAttribute(Qt::WA_TranslucentBackground, false);
    label->setGeometry(0, 0, 1900, 1000);

     QString str("QLabel{background-color:rgba(255,0,0,0.0);}");   //0.6代表透明度，根据需要自行调整
     label->setStyleSheet(str);
  //  QString str("QLabel{background-color:rgba(255,0,0,0.2);}");
   // setStyleSheet(str);
}

TransparentWidget::~TransparentWidget()
{
}

void TransparentWidget::Init()
{
    t = new QTimer(this);
    t->connect(t, &QTimer::timeout, this, &TransparentWidget::onTimeOut);
    t->start(100);
}

void TransparentWidget::SetTransparentGeometry(QRect rect)
{
    setGeometry(rect);
    label->setGeometry(rect);
}

#include <qt_windows.h>
void TransparentWidget::ChangeAlpha(double alpha)
{
    QString str = QString("QLabel{background-color:rgba(0,0,0,%1);}").arg(alpha);
    label->setStyleSheet("");
    label->setStyleSheet(str);
    label->update();
    label->repaint();
    UpdateWindow((HWND)label->winId());

  /*  QString str = QString("QWidget{background-color:rgba(255,0,0,%1);}").arg(alpha);
    setStyleSheet(str);*/
}
void TransparentWidget::onTimeOut()
{
    static int a = 0;

}

void TransparentWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(255, 0, 0, 1));  //QColor最后一个参数80代表背景的透明度
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
