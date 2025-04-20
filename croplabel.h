#ifndef CROPLABEL_H
#define CROPLABEL_H

#include <QLabel>
#include <QPainter>
#include <QMouseEvent>

class CropLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CropLabel(QWidget *parent=nullptr);
    QRect getselectedArea() const;//最终选区

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void selectionUpdated(QRect screenRect);

private:
    QPoint m_startPos;
    QPoint m_endPos;
    bool m_isDragging=false;
    QRect m_screenRect;//屏幕坐标系下
    // QPen m_pen;

    //void updateSelectionRect();
    QRect convertToImageRect(const QRect &screenRect) const;//坐标转换
};
#endif // CROPLABEL_H
