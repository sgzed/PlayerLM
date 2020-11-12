#include "transparentwidget.h"

#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
#include <qtimer.h>
#include <qlabel.h>
#include <qt_windows.h>


TransparentWidget::TransparentWidget(QWidget* parent) :
	QWidget(parent) 
{
    setWindowTitle(QString::fromLocal8Bit("TransparentWidget"));
    //setFixedSize(w, h);

    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
 
    label = new QLabel(this);
    label->setAttribute(Qt::WA_TranslucentBackground, false);

    QString str("QLabel{background-color:rgba(255,0,0,0.0);}");   
    label->setStyleSheet(str);
}

TransparentWidget::~TransparentWidget()
{
}

void TransparentWidget::Init()
{
}

void TransparentWidget::SetTransparentGeometry(QRect rect)
{
    setGeometry(rect);
    label->setGeometry(rect);
}

void TransparentWidget::ChangeAlpha(double alpha)
{
    QString str = QString("QLabel{background-color:rgba(0,0,0,%1);}").arg(alpha);
    label->setStyleSheet("");
    label->setStyleSheet(str);
    label->update();
    label->repaint();
    UpdateWindow((HWND)label->winId());

}
void TransparentWidget::onTimeOut()
{

}

void TransparentWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(255, 0, 0, 0));  //QColor最后一个参数代表背景的透明度
    event->accept();
}

//void TransparentWidget::mousePressEvent(QMouseEvent* event)
//{
//    if (event->button() == Qt::LeftButton) {
//        m_startPoint = frameGeometry().topLeft() - event->globalPos();
//    }
//}
//
//void TransparentWidget::mouseMoveEvent(QMouseEvent* event)
//{
//    this->move(event->globalPos() + m_startPoint);
//}
