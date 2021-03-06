#include <QWidget>
#include <QTimer>
class QLabel;

class TransparentWidget : public QWidget
{
    Q_OBJECT

public:
    TransparentWidget(QWidget* parent = nullptr);
    ~TransparentWidget();

    void Init();

    void SetTransparentGeometry(QRect);

    void ChangeAlpha(double alpha);

    int alpha = 0;

protected:

    void paintEvent(QPaintEvent*);

    //void mousePressEvent(QMouseEvent* event);

    //void mouseMoveEvent(QMouseEvent* event);

public slots:
    void onTimeOut();

private:
    QPoint m_startPoint;
    QTimer* t;
    QLabel* label;
};