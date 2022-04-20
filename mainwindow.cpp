#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    ImageLoader imageLoader;

    //connect(&imageLoader, &ImageLoader::gotImageSize, this, &MainWindow::onImageSizeRecieved);

    imageLoader.loadImage();

    imageLoader.measureWeightOfImage();

//    QVector<quint32> imageVector = imageLoader.loadImage();


//    quint32 maxInt = 0;

//    for(int i =0; i < imageVector.size(); i++)
//    {
//        if(imageVector[i] > maxInt)
//            maxInt = imageVector[i];
//    }

//    QImage imgX(m_imageWidth, m_imageHeight, QImage::Format_RGB888);


//    QRgb color = qRgb(127,127,127);

//    for(int x = 0; x < m_imageWidth; x++)
//    {
//        for(int y = 0; y < m_imageHeight; y++)
//        {
//            quint16 brightness = static_cast<quint16>(((static_cast<double>(imageVector[x*y]))/maxInt) * 255);

//            imgX.setPixel(x,y, qRgb(brightness,brightness,brightness) );
//            //qDebug() << brightness;
//        }
//    }





}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onImageSizeRecieved(quint32 width, quint32 height)
{
    m_imageWidth = width;
    m_imageHeight = height;
}

