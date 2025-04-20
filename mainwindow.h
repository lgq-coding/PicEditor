#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QPushButton>
#include <QMainWindow>
#include <stack>
// #include <croplabel.h>
#include <screencapturewidget.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void displayImage(const cv::Mat &image);
    bool checkImageValid();
    void pushUndoState();   //保存操作状态
    void applyAdjustments();//亮度与对比度调整

private slots:
    //插入与保存
    void insertactionSlot();
    void saveactionSlot();
    //撤销与重做
    void undoactionSlot();
    void redoactionSlot();
    //旋转
    void on_rotationButton_clicked();
    //亮度与对比度
    void on_brightnessSlider_valueChanged();
    void on_contrastSlider_valueChanged();
    //透明度
    cv::Mat addAlphaChannel(const cv::Mat &src);
    void on_alphaSlider_valueChanged();
    //灰度化
    // void on_grayscaleButton_clicked();
    //canny
    void on_cannyButton_clicked();
    //滤镜
    void on_filterButton_clicked();
    //裁剪
    // void onCropConfirmed();
    void on_cropButton_clicked();

private:
    Ui::MainWindow *ui;
    cv::Mat m_currentImage;
    double m_brightness=1.0;
    double m_contrast=1.0;
    std::stack<cv::Mat> m_undoStack;
    std::stack<cv::Mat> m_redoStack;
    //裁剪相关
    // CropLabel *ui_imagelabel;
    // QPushButton *m_confirmButton;
};
#endif // MAINWINDOW_H
