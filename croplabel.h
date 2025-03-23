#ifndef CROPLABEL_H
#define CROPLABEL_H

#include <QLabel>
#include <QPainter>

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
    void selectionChanged(QRect rect);

private:
    QPoint m_startPos;
    QPoint m_endPos;
    bool m_isSelecting=false;
    QRect m_currentRect;
    QPen m_pen;

    void updateSelectionRect();
};

#endif // CROPLABEL_H
