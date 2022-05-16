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

    exit(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadImage()
{
    QFile intFile("i_6PET.int");
    QByteArray intBa;

    if(!intFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "cant open file";
    }else
    {
        intBa = intFile.readAll();
    }

    QDataStream baStream(&intBa, QIODevice::ReadWrite);

    baStream.setByteOrder(QDataStream::LittleEndian);

    baStream >> m_widthOfXrayImg;
    baStream >> m_heightOfXrayImg;

    baStream.skipRawData(56);

    m_imageVector = QVector<quint32>(m_widthOfXrayImg*m_heightOfXrayImg);
    for(int i = 0; i < m_imageVector.size(); i++)
    {
        baStream >> m_imageVector[i];
    }


    m_weightRect = QRect(503, 180, 5, 5);
    m_calibRect = QRect(0,480, m_widthOfXrayImg, 20);
    m_I0Rect = QRect(m_widthOfXrayImg - 90,0,80,m_heightOfXrayImg);


    m_speedOfColumn.resize(m_widthOfXrayImg);
    m_speedOfColumn.fill(300);

    //updateNormalizedImage();


}


