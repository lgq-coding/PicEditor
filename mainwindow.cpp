#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //样式相关
    ui->imagelabel->setStyleSheet("QLabel {border:3px solid rgb(0,0,139);}");//imagelabel边框设置
    //导入
    connect(ui->insertaction,&QAction::triggered,this,&MainWindow::insertactionSlot);
    //保存
    connect(ui->saveaction,&QAction::triggered,this,&MainWindow::saveactionSlot);
    //brightness
    connect(ui->brightnessSlider,&QSlider::valueChanged,this,&MainWindow::on_brightnessSlider_valueChanged);
    //contrast
    connect(ui->contrastSlider,&QSlider::valueChanged,this,&MainWindow::on_contrastSlider_valueChanged);
    //撤销与重做
    ui->undoaction->setEnabled(false);
    ui->redoaction->setEnabled(false);
    connect(ui->undoaction,&QAction::triggered,this,&MainWindow::undoactionSlot);
    connect(ui->redoaction,&QAction::triggered,this,&MainWindow::redoactionSlot);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::displayImage(const cv::Mat &image)
{
    if(image.empty()) return;
    QImage qImg(image.data,image.cols,image.rows,image.step,QImage::Format_RGB888);
    QPixmap pixmap=QPixmap::fromImage(qImg);
    //imagelabel显示图片，且通过scaled函数保持宽高比缩放
    ui->imagelabel->setPixmap(pixmap.scaled(ui->imagelabel->size(),
                    Qt::KeepAspectRatio,Qt::SmoothTransformation));
}

bool MainWindow::checkImageValid()
{
    if(m_currentImage.empty())
    {
        QMessageBox::warning(this,"Error","No image loaded!");
        return false;
    }
    return true;
}

void MainWindow::pushUndoState()
{
    if(!m_currentImage.empty())
    {
        m_undoStack.push(m_currentImage.clone());
        m_redoStack=std::stack<cv::Mat>();
        ui->undoaction->setEnabled(true);
        ui->redoaction->setEnabled(false);
    }
}

void MainWindow::insertactionSlot()
{
    QString filename=QFileDialog::getOpenFileName(this,"选择一张图片",
                    "","Image Files(*.png *.jpg *.bmp");
    if(!filename.isEmpty())
    {
        cv::Mat newImage=cv::imread(filename.toStdString(),cv::IMREAD_COLOR);
        if(!newImage.empty())
        {
            //转换成RGB格式并保存到当前图像
            cv::cvtColor(newImage,m_currentImage,cv::COLOR_BGR2RGB);
            pushUndoState();
            displayImage(m_currentImage);
            //重置调整参数
            ui->brightnessSlider->setValue(0);
            ui->contrastSlider->setValue(100);
        }
        else
        {
            QMessageBox::warning(this,"Error","Failed to open image!");
        }
    }
}

void MainWindow::saveactionSlot()
{
    if(!checkImageValid()) return;
    QString savePath=QFileDialog::getSaveFileName(this,"选择保存路径",
                    "","PNG(*.png);;JPEG(*.jpg)");
    if(!savePath.isEmpty())
    {
        cv::Mat saveImage;
        cv::cvtColor(m_currentImage,saveImage,cv::COLOR_RGB2BGR);
        if(!cv::imwrite(savePath.toStdString(),saveImage))
        {
            QMessageBox::critical(this,"Error","Save operation failed!");
        }
    }
}

void MainWindow::on_rotationButton_clicked()
{
    if(!checkImageValid()) return;
    pushUndoState();
    cv::Mat baseImage=m_currentImage.clone();
    cv::Point2f center(baseImage.cols/2.0,baseImage.rows/2.0);
    cv::Mat rotMat=cv::getRotationMatrix2D(center,45,1.0);//变换矩阵

    //仿射变换
    cv::warpAffine(baseImage,m_currentImage,rotMat,baseImage.size());
    //cv::imshow("img",m_currentImage);
    displayImage(m_currentImage);

}

void MainWindow::applyAdjustments()//output = contrast * input + brightness
{
    if(!checkImageValid()) return;
    if(!m_undoStack.empty())
    {
        cv::Mat baseImage=m_undoStack.top().clone();
        baseImage.convertTo(m_currentImage,-1,m_contrast,m_brightness*255-128*m_contrast+128);
        displayImage(m_currentImage);
    }

}

void MainWindow::on_brightnessSlider_valueChanged()
{
    if(ui->brightnessCheckBox->isChecked())
    {
        m_brightness=ui->brightnessSlider->value()/100.0;
        applyAdjustments();
    }
}

void MainWindow::on_contrastSlider_valueChanged()
{
    if(ui->contrastCheckBox->isChecked())
    {
        m_contrast=ui->contrastSlider->value()/100.0;
        applyAdjustments();
    }
}

void MainWindow::undoactionSlot()
{
    if(!m_undoStack.empty())
    {
        m_redoStack.push(m_currentImage.clone());
        m_currentImage=m_undoStack.top().clone();
        m_undoStack.pop();
        displayImage(m_currentImage);

        ui->redoaction->setEnabled(true);
        ui->undoaction->setEnabled(!m_undoStack.empty());
    }
}

void MainWindow::redoactionSlot()
{
    if(!m_redoStack.empty())
    {
        m_undoStack.push(m_currentImage.clone());
        m_currentImage=m_redoStack.top().clone();
        m_redoStack.pop();
        displayImage(m_currentImage);

        ui->undoaction->setEnabled(true);
        ui->redoaction->setEnabled(!m_redoStack.empty());

    }
}

void MainWindow::on_grayscaleButton_clicked()
{
    if(!checkImageValid()) return;
    pushUndoState();
    cv::Mat gray;
    cv::cvtColor(m_currentImage,gray,cv::COLOR_RGB2GRAY);
    cv::cvtColor(gray,m_currentImage,cv::COLOR_GRAY2RGB);//保持三通道格式
    displayImage(m_currentImage);
}

