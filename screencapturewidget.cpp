#include "screencapturewidget.h"

#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>

ScreenCaptureWidget::ScreenCaptureWidget(QWidget *parent):QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowState(Qt::WindowFullScreen);

    const auto screens=QGuiApplication::screens();
    for(const QScreen *screen :screens)
    {
        m_screenGeometries.append(screen->geometry());
    }
}

QScreen* ScreenCaptureWidget::findScreenForPoint(const QPoint &pos) const
{
    for(const QRect &geo:m_screenGeometries)
    {
        if(geo.contains(pos))
        {
            return QGuiApplication::screenAt(pos);
        }
    }
    return nullptr;
}

void ScreenCaptureWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        m_startPos=event->globalPos();
        m_endPos=m_startPos;
        m_isSelecting=true;
        update();
    }
}

void ScreenCaptureWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isSelecting)
    {
        m_endPos=event->globalPos();
        update();
    }
}

void ScreenCaptureWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton&&m_isSelecting)
    {
        m_isSelecting=false;

        QScreen *targetScreen=findScreenForPoint(m_startPos);
        if(!targetScreen)
        {
            emit canceled();
            return;
        }

        QPixmap screenShot=targetScreen->grabWindow(
            0,
            targetScreen->geometry().x(),
            targetScreen->geometry().y(),
            targetScreen->size().width(),
            targetScreen->size().height()
            );
        QRect selection=currentSelection();
        QPoint screenOrigin=targetScreen->geometry().topLeft();
        QRect localSelection(
            selection.x()-screenOrigin.x(),
            selection.y()-screenOrigin.y(),
            selection.width(),
            selection.height()
            );

        QPixmap cropped=screenShot.copy(localSelection);
        emit regionSelected(cropped);
        close();
    }
}

void ScreenCaptureWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(),QColor(0,0,0,5));
    if(m_isSelecting)
    {
        painter.setPen(QPen(Qt::green, 2));
        painter.drawRect(currentSelection());

        // 显示选区尺寸
        QString sizeText = QString("%1 x %2")
                               .arg(currentSelection().width())
                               .arg(currentSelection().height());
        painter.setFont(QFont("Arial", 12));
        painter.drawText(currentSelection().bottomRight() + QPoint(10, 20), sizeText);
    }
}

QRect ScreenCaptureWidget::currentSelection() const
{
    return QRect(m_startPos,m_endPos).normalized();
}
