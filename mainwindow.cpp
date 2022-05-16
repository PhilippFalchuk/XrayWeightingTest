#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCamera>
#include <QCameraInfo>
#include <QVideoProbe>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ImageWeighter imageLoader;
    imageLoader.loadImage();
    float weightOfImage = imageLoader.measureWeightOfImage();

    qDebug() << weightOfImage;



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processFrame(QVideoFrame vf)
{
    qDebug() << vf;
}
