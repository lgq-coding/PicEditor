#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QDebug>

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
    //alpha
    connect(ui->alphaSlider,&QSlider::valueChanged,this,&MainWindow::on_alphaSlider_valueChanged);
    //撤销与重做
    ui->undoaction->setEnabled(false);
    ui->redoaction->setEnabled(false);
    connect(ui->undoaction,&QAction::triggered,this,&MainWindow::undoactionSlot);
    connect(ui->redoaction,&QAction::triggered,this,&MainWindow::redoactionSlot);
    //rotationspinBox
    ui->rotationspinBox->setRange(0,180);
    ui->rotationspinBox->setSingleStep(15);
    ui->rotationspinBox->setValue(90);
    ui->rotationspinBox->setPrefix("angle= ");
    ui->rotationspinBox->setSuffix(" °");
    ui->rotationspinBox->setWrapping(true);
    // //裁剪相关
    // ui_imagelabel=qobject_cast<CropLabel*>(ui->imagelabel);//获取promote后的QLabel
    // m_confirmButton=new QPushButton("✓",this);
    // m_confirmButton->setFixedSize(30,30);
    // m_confirmButton->setStyleSheet("background: #4CAF50; color: white; border-radius: 15px;font-weight:bold;");
    // m_confirmButton->hide();
    // connect(ui_imagelabel,&CropLabel::selectionUpdated,[this]()
    //         {
    //     QRect rect=ui_imagelabel->getselectedArea();
    //     if(!rect.isEmpty())
    //     {
    //         QPoint btnPos=ui_imagelabel->mapToParent(
    //             QPoint(rect.right()+10,rect.bottom()+10));
    //         btnPos.setX(qMin(btnPos.x(),width()-m_confirmButton->width()));
    //         btnPos.setY(qMin(btnPos.y(),height()-m_confirmButton->height()));
    //         m_confirmButton->move(btnPos);
    //         m_confirmButton->show();
    //     }
    //     else
    //     {
    //         m_confirmButton->hide();
    //     }

    // });
    // connect(m_confirmButton,&QPushButton::clicked,this,&MainWindow::onCropConfirmed);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//原版的displayImage()
// void MainWindow::displayImage(const cv::Mat &image)
// {
//     if(image.empty()) return;
//     QImage qImg(image.data,image.cols,image.rows,image.step,QImage::Format_RGB888);
//     QPixmap pixmap=QPixmap::fromImage(qImg);
//     //imagelabel显示图片，且通过scaled函数保持宽高比缩放
//     ui->imagelabel->setPixmap(pixmap.scaled(ui->imagelabel->size(),
//                     Qt::KeepAspectRatio,Qt::SmoothTransformation));
// }

//结合透明度调整后的displayImage()
void MainWindow::displayImage(const cv::Mat &image)
{
    if(image.empty()) return;
    QImage qImg;
    if(image.channels()==4)
    {
        //addAlphaChannel中已转化为bgra格式
        cv::Mat rgbaImage;
        cv::cvtColor(m_currentImage,rgbaImage,cv::COLOR_BGRA2RGBA);
        qImg=QImage(image.data,image.cols,image.rows,image.step,QImage::Format_ARGB32);
    }
    else if(image.channels()==3)
    {
        //在insert环节已转化为rgb格式
        qImg=QImage(image.data,image.cols,image.rows,image.step,QImage::Format_RGB888);
    }
    else
    {
        QMessageBox::warning(this,"Error","Unsupported image format!");
        return;
    }
    if(image.channels()==4)
    {
        //棋盘格背景
        QPixmap canvas=QPixmap(ui->imagelabel->size());
        canvas.fill(Qt::transparent);
        QPainter painter(&canvas);
        QBrush checkerBrush(Qt::lightGray,Qt::Dense5Pattern);
        painter.fillRect(canvas.rect(),checkerBrush);
        //绘制透明图像
        QPixmap pix=QPixmap::fromImage(qImg);
        painter.drawPixmap(0,0,pix.scaled(ui->imagelabel->size(),
                        Qt::KeepAspectRatio,Qt::SmoothTransformation));
        ui->imagelabel->setPixmap(canvas);
    }
    else
    {
        QPixmap pixmap=QPixmap::fromImage(qImg);
        //imagelabel显示图片，且通过scaled函数保持宽高比缩放
        ui->imagelabel->setPixmap(pixmap.scaled(ui->imagelabel->size(),
                        Qt::KeepAspectRatio,Qt::SmoothTransformation));
     }
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
            //转换成RGB/RGBA格式并保存到当前图像
            if(newImage.channels()==4)
            {
                cv::cvtColor(newImage,m_currentImage,cv::COLOR_BGRA2RGBA);
            }
            else if(newImage.channels()==3)
            {
                cv::cvtColor(newImage,m_currentImage,cv::COLOR_BGR2RGB);
            }
            else
            {
                QMessageBox::warning(this,"Error","Unsupported image format!");
                return;
            }
            pushUndoState();
            displayImage(m_currentImage);
            //重置调整参数
            ui->brightnessSlider->setRange(0,100);
            ui->brightnessSlider->setValue(0);
            ui->contrastSlider->setRange(0,200);
            ui->contrastSlider->setValue(100);
            ui->alphaSlider->setRange(0,100);
            ui->alphaSlider->setValue(50);
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
                    "","PNG(*.png);;JPEG(*.jpg);;BMP(*.bmp)");
    if(!savePath.isEmpty())
    {
        cv::Mat saveImage;
        if(m_currentImage.channels()==4)
        {
            cv::cvtColor(m_currentImage,saveImage,cv::COLOR_RGBA2BGRA);
        }
        else if(m_currentImage.channels()==3)
        {
            cv::cvtColor(m_currentImage,saveImage,cv::COLOR_RGB2BGR);
        }
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
    int angle=ui->rotationspinBox->value();
    cv::Mat rotMat=cv::getRotationMatrix2D(center,angle,1.0);//变换矩阵

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

//之前的灰度化
// void MainWindow::on_grayscaleButton_clicked()
// {
//     if(!checkImageValid()) return;
//     pushUndoState();
//     cv::Mat gray;
//     cv::cvtColor(m_currentImage,gray,cv::COLOR_RGB2GRAY);
//     cv::cvtColor(gray,m_currentImage,cv::COLOR_GRAY2RGB);//保持三通道格式
//     displayImage(m_currentImage);
// }

cv::Mat MainWindow::addAlphaChannel(const cv::Mat &src)
{
    cv::Mat dst;
    if(src.channels()==3)
    {
        cv::cvtColor(src,dst,cv::COLOR_RGB2BGRA);
        dst.forEach<cv::Vec4b>([](cv::Vec4b &pixel,const int*){
            pixel[3]=128;
        });
    }
    else
    {
        dst=src.clone();
    }
    return dst;
}

void MainWindow::on_alphaSlider_valueChanged()
{
    if(!checkImageValid()) return;
    pushUndoState();
    if(ui->alphaCheckBox->isChecked())
    {
        if(m_currentImage.channels()==3)
        {
            m_currentImage=addAlphaChannel(m_currentImage);
        }
        float alpha=(ui->alphaSlider->value())/100.0f;
        m_currentImage.forEach<cv::Vec4b>(
            [alpha](cv::Vec4b &pixel,const int*)
            {
                pixel[3]=cv::saturate_cast<uchar>(alpha*255);
            }
            );
    }
    displayImage(m_currentImage);
}

void MainWindow::on_cannyButton_clicked()
{
    if(!checkImageValid()) return;
    pushUndoState();
    cv::Mat gray,edge;
    cv::cvtColor(m_currentImage,gray,cv::COLOR_RGB2GRAY);
    cv::GaussianBlur(gray,gray,cv::Size(3,3),0);
    cv::Canny(gray,edge,50,150);
    cv::cvtColor(edge,m_currentImage,cv::COLOR_GRAY2RGB);
    displayImage(m_currentImage);
}


void MainWindow::on_filterButton_clicked()
{
    if(!checkImageValid()) return;
    pushUndoState();
    switch(ui->comboBox->currentIndex())
    {
    case 0://反色
    {
        if(m_currentImage.channels()==4)
        {
            std::vector<cv::Mat> channels;
            cv::split(m_currentImage,channels);
            cv::bitwise_not(channels[0],channels[0]);
            cv::bitwise_not(channels[1],channels[1]);
            cv::bitwise_not(channels[2],channels[2]);
            cv::merge(channels,m_currentImage);
        }
        else
        {
            cv::bitwise_not(m_currentImage,m_currentImage);
        }
        displayImage(m_currentImage);
        break;
    }
    case 1://灰度化
    {
        cv::Mat gray;
        if(m_currentImage.channels()==4)
        {
            cv::cvtColor(m_currentImage,gray,cv::COLOR_RGBA2GRAY);
            cv::cvtColor(gray,m_currentImage,cv::COLOR_GRAY2RGBA);
        }
        else
        {
            cv::cvtColor(m_currentImage,gray,cv::COLOR_RGB2GRAY);
            cv::cvtColor(gray,m_currentImage,cv::COLOR_GRAY2RGB);//保持三通道格式
        }
        displayImage(m_currentImage);
        break;
    }
    case 2://马赛克
    {
        cv::Mat dst=m_currentImage.clone();
        int blocksize=15;
        for(int y=0;y<m_currentImage.rows;y+=blocksize)
        {
            for(int x=0;x<m_currentImage.cols;x+=blocksize)
            {
                cv::Rect roi(x,y,blocksize,blocksize);
                roi&=cv::Rect(0,0,m_currentImage.cols,m_currentImage.rows);
                cv::Scalar meanColor=cv::mean(m_currentImage(roi));
                dst(roi).setTo(meanColor);
            }
        }
        m_currentImage=dst;
        displayImage(m_currentImage);
        break;
    }
    case 3://复古棕褐
    {
        cv::Mat dst;
        cv::Mat kernel=(cv::Mat_<float>(3,3)<<
                                0.393,0.769,0.189,
                                0.349,0.686,0.168,
                                0.272,0.534,0.131);
        cv::transform(m_currentImage,dst,kernel);
        cv::normalize(dst,dst,0,255,cv::NORM_MINMAX,CV_8UC3);
        m_currentImage=dst;
        displayImage(m_currentImage);
        break;
    }
    case 4://浮雕
    {
        cv::Mat dst;
        cv::Mat kernel=(cv::Mat_<float>(3,3)<<
                                -2,-1,0,
                                -1,1,1,
                                0,1,2);
        cv::filter2D(m_currentImage,dst,-1,kernel,cv::Point(-1,-1),128,cv::BORDER_DEFAULT);
        m_currentImage=dst;
        displayImage(m_currentImage);
        break;
    }
    default:
        break;
    }
}

// void MainWindow::onCropConfirmed()
// {
//     QRect selected=ui_imagelabel->getselectedArea();
//     //opencv中Rect
//     cv::Rect rc(selected.x(),selected.y(),selected.width(),selected.height());

//     if(rc.x<0||rc.y<0)
//     {
//         QMessageBox::warning(this,"Error","Invalid selection area!");
//         return;
//     }
//     cv::Mat cropped=m_currentImage(rc).clone();
//     m_currentImage=cropped;
//     m_confirmButton->hide();
//     displayImage(m_currentImage);
//     pushUndoState();
// }

void MainWindow::on_cropButton_clicked()
{
    ScreenCaptureWidget *captureWidget=new ScreenCaptureWidget();
    connect(captureWidget,&ScreenCaptureWidget::regionSelected,
    [this](const QPixmap &pixmap)
    {
        QImage image=pixmap.toImage().convertToFormat(QImage::Format_RGB888);
        cv::Mat cvImage(image.height(),image.width(),CV_8UC3,
                        (void*)image.constBits(),image.bytesPerLine());
        displayImage(cvImage);
        m_currentImage=cvImage.clone();
        pushUndoState();
    });
    connect(captureWidget,&ScreenCaptureWidget::canceled,
            [](){qDebug()<<"Capture canceled";});
    captureWidget->show();
}

