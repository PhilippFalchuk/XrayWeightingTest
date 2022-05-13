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


    QCamera* camera;

    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cameraInfo : cameras) {
            camera = new QCamera(cameraInfo);
    }

    qDebug() << camera;

    QVideoProbe *probe = new QVideoProbe;

    connect(probe, SIGNAL(videoFrameProbed(QVideoFrame)), this, SLOT(processFrame(QVideoFrame)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processFrame(QVideoFrame vf)
{
    qDebug() << vf;
}
