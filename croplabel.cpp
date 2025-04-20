#include "croplabel.h"
#include <QStyle>

CropLabel::CropLabel(QWidget *parent):QLabel(parent)
{
    setMouseTracking(true);
}

// QRect CropLabel::convertToImageRect(const QRect &screenRect) const
// {
//     QPixmap pm=pixmap(Qt::ReturnByValue);
//     if(pm.isNull())   return QRect();

//     // qreal devicePixelRatio=pm.devicePixelRatio();//高DPI适配
//     // QSize actualPixmapSize=pm.size()/devicePixelRatio;
//     QRect imgRect=pm.rect();//实际显示区域
//     imgRect.moveCenter(rect().center());
//     imgRect = imgRect.intersected(rect());

//     qreal scaleX=qRound((qreal)pm.width()/width()*10000)/10000.0;//缩放比例
//     qreal scaleY=qRound((qreal)pm.height()/height()*10000)/10000.0;

//     int imgx=qRound((screenRect.x()-imgRect.x())*scaleX);
//     int imgy=qRound((screenRect.y()-imgRect.y())*scaleY);
//     int imgwidth=qRound(screenRect.width()*scaleX);
//     int imgheight=qRound(screenRect.height()*scaleY);
//     return QRect(imgx,imgy,imgwidth,imgheight);//确保在图像显示范围内
// }

//改进后的convertToImageRect
QRect CropLabel::convertToImageRect(const QRect &screenRect) const
{
    QPixmap pm=pixmap(Qt::ReturnByValue);
    if(pm.isNull())   return QRect();

    qreal devicePixelRatio=pm.devicePixelRatio();
    QSize actualPixmapSize=pm.size()/devicePixelRatio;

    QRect displayRect=QStyle::alignedRect(Qt::LeftToRight,
                                        Qt::AlignCenter,
                                        actualPixmapSize,
                                        this->rect());

    QRect validDisplayRect=displayRect.intersected(this->rect());

    qreal scaleX=static_cast<qreal>(pm.width())/validDisplayRect.height();
    qreal scaleY=static_cast<qreal>(pm.height())/validDisplayRect.height();

    int imgX=qRound((screenRect.x()-validDisplayRect.x())*scaleX);
    int imgY=qRound((screenRect.y()-validDisplayRect.y())*scaleY);
    int imgWidth=qRound(screenRect.width()*scaleX);
    int imgHeight=qRound(screenRect.height()*scaleY);

    return QRect(imgX,imgY,imgWidth,imgHeight)&pm.rect();
}

void CropLabel::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        m_startPos=event->pos();
        m_endPos=event->pos();
        m_isDragging=true;
        update();
    }
}

void CropLabel::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isDragging)
    {
        m_endPos=event->pos();
        m_screenRect=QRect(m_startPos,m_endPos).normalized();
        emit selectionUpdated(m_screenRect);
        update();
    }
}

void CropLabel::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDragging=false;
}

void CropLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    if(m_isDragging&&!m_screenRect.isNull())
    {
        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        //暗色遮罩
        painter.setBrush(QColor(0,0,0,50));
        painter.drawRect(rect());
        //清除选中区域
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.drawRect(m_screenRect);
        //边框
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setPen(QPen(Qt::green,2,Qt::DashLine));
        painter.drawRect(m_screenRect);
    }
}

QRect CropLabel::getselectedArea() const
{
    return convertToImageRect(m_screenRect);
}
