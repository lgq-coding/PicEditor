#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QFileDialog>
using namespace cv;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->imagelabel->setStyleSheet("QLabel {border:3px solid rgb(0,0,139);}");//imagelabel边框设置
    //导入
    connect(ui->insertaction,&QAction::triggered,this,&MainWindow::insertactionSlot);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::insertactionSlot()
{
    QString filename=QFileDialog::getOpenFileName(this,"选择一张图片",
                    "D:\\qtcode","Image Files(*.png *.jpg *.bmp");
    if(!filename.isEmpty())
    {

    }
}

void MainWindow::saveactionSlot()
{

}
