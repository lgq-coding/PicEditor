#ifndef SCREENCAPTUREWIDGET_H
#define SCREENCAPTUREWIDGET_H

#include <QWidget>
#include <QScreen>

class ScreenCaptureWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ScreenCaptureWidget(QWidget *parent=nullptr);

signals:
    void regionSelected(const QPixmap &cropped);
    void canceled();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;


private:
    QPoint m_startPos;
    QPoint m_endPos;
    bool m_isSelecting=false;
    QVector<QRect>m_screenGeometries;

    QRect currentSelection() const;
    QScreen* findScreenForPoint(const QPoint &pos)const;
};

#endif // SCREENCAPTUREWIDGET_H
