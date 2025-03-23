#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <stack>
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




    void on_grayscaleButton_clicked();

private:
    Ui::MainWindow *ui;
    cv::Mat m_currentImage;
    double m_brightness=1.0;
    double m_contrast=1.0;
    std::stack<cv::Mat> m_undoStack;
    std::stack<cv::Mat> m_redoStack;
};
#endif // MAINWINDOW_H
