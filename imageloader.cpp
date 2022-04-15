#include "imageloader.h"
#include <QImage>
#include <QQueue>
#include <QPainter>
#include <math.h>

ImageLoader::ImageLoader(QObject *parent)
    : QObject{parent}
{
    m_settings = new QSettings("Geometry.ini", QSettings::IniFormat);
    qDebug() << m_settings->fileName();


    m_settings->beginGroup("SettGroup");

    m_betatronCoordinates = QPoint(m_settings->value("betatronX").toInt(), m_settings->value("betatronY").toInt());

    m_ACoordinates = QPoint(m_settings->value("AX").toInt(), m_settings->value("AY").toInt());
    m_BCoordinates = QPoint(m_settings->value("BX").toInt(), m_settings->value("BY").toInt());
    m_CCoordinates = QPoint(m_settings->value("CX").toInt(), m_settings->value("CY").toInt());
    m_DCoordinates = QPoint(m_settings->value("DX").toInt(), m_settings->value("DY").toInt());

    m_firstSegmentNumOfDetectors = m_settings->value("ABn").toInt();
    m_secondSegmentNumOfDetectors = m_settings->value("BCn").toInt();
    m_thirdSegmentNumOfDetectors = m_settings->value("CDn").toInt();

    m_numOfSegments = m_settings->value("numberOfSegments").toInt();

    m_endOfDividingVector = QPoint(m_settings->value("upperPointOfDividingVectorX").toInt(), m_settings->value("upperPointOfDividingVectorY").toInt() );

    m_settings->endGroup();


    m_startOfDividingVector = QPoint(m_endOfDividingVector.x(), m_betatronCoordinates.y());

    m_pointsOfAngles = QVector<QPoint>{m_ACoordinates, m_BCoordinates, m_CCoordinates, m_DCoordinates};

    m_numOfDetectors = QVector<int>{m_firstSegmentNumOfDetectors,m_secondSegmentNumOfDetectors,m_thirdSegmentNumOfDetectors};


    m_detectorCoordinates = QVector<QPoint>();

   // qDebug() << m_numOfDetectors;
    //qDebug() << m_numOfSegments;

    //qDebug() << m_pointsOfAngles;

    for(int i=0;i< m_numOfSegments;i++)
        {
            QPoint startPoint = m_pointsOfAngles[i];
            QPoint endPoint = m_pointsOfAngles[i+1];

            float lenghtOfDetectorX = (static_cast<float>(endPoint.x() - startPoint.x()))/ static_cast<float>(m_numOfDetectors[i]);
            float lenghtOfDetectorY = (static_cast<float>(endPoint.y() - startPoint.y()))/ static_cast<float>(m_numOfDetectors[i]);
            for(int j=0;j< m_numOfDetectors[i];j++)
            {

                int xCoordinate = startPoint.x() + lenghtOfDetectorX*j;
                int yCoordinate = startPoint.y() + lenghtOfDetectorY*j;

                QPoint coordinatesOfDetector = QPoint(xCoordinate,yCoordinate);

                m_detectorCoordinates.push_back(coordinatesOfDetector);

            }
        }

//    qDebug() << m_detectorCoordinates;

    toCalculateDistanceFromDividingLine();
    toCalculateDistanceFromBetatron();







}

QVector<quint32> ImageLoader::loadImage()
{
    QFile intFile("i_3.int");
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

//    quint32 m_widthOfXrayImg;
//    quint32 m_heightOfXrayImg;
    baStream >> m_widthOfXrayImg;
    baStream >> m_heightOfXrayImg;


    emit gotImageSize(m_widthOfXrayImg, m_heightOfXrayImg);

    //qDebug() << widthOfXrayImg << heightOfXrayImg;

    baStream.skipRawData(56);

    m_imageVector = QVector<quint32>(m_widthOfXrayImg*m_heightOfXrayImg);
    for(int i = 0; i < m_imageVector.size(); i++)
    {
        baStream >> m_imageVector[i];
    }

    m_maxInt = 0;

    for(int i =0; i < m_imageVector.size(); i++)
    {
        if(m_imageVector[i] > m_maxInt)
            m_maxInt = m_imageVector[i];
    }

    QImage imgX(m_widthOfXrayImg, m_heightOfXrayImg, QImage::Format_RGB888);

    //qDebug() << m_maxInt;


    //QRgb color = qRgb(127,127,127);


    for(int y = 0; y < m_heightOfXrayImg; y++)
    {
        for(int x = 0; x < m_widthOfXrayImg; x++)
        {
            quint16 brightness = static_cast<quint16>(((static_cast<double>(m_imageVector[m_widthOfXrayImg*y +x]))/m_maxInt) * 255);

            imgX.setPixel(x,y, qRgb(brightness,brightness,brightness) );
        }
    }


    m_weightRect = QRect(576, 210, 49, 65);
    m_calibRect = QRect(0,m_heightOfXrayImg - 45, m_widthOfXrayImg, 45);
    m_I0Rect = QRect(m_widthOfXrayImg - 90,0,80,m_heightOfXrayImg);





//    QPainter p(&imgX);
//    p.setBrush(Qt::red);

//    p.drawRect(m_weightRect);

//    p.setBrush(Qt::blue);

//    p.drawRect(m_calibRect);

//    p.setBrush(Qt::green);
//    p.drawRect(m_I0Rect);






    imgX.save("657.bmp");


    m_speedOfColumn.resize(m_widthOfXrayImg);
    m_speedOfColumn.fill(300);


    updateNormalizedImage();

    measureWeightOfImage();

return QVector<quint32>();


}

int ImageLoader::measureWeightOfImage()
{

    QVector<float> thicknessOfPixels(m_normalizedImageVector.size());
    thicknessOfPixels.fill(0);

    for(int x = m_weightRect.x(); x < m_weightRect.x() + m_weightRect.width(); x++)
    {
        for(int y = m_weightRect.y(); y < m_weightRect.y() + m_weightRect.height(); y++ )
        {
            thicknessOfPixels[m_widthOfXrayImg*y + x] = (-(logf(m_normalizedImageVector[m_widthOfXrayImg*y + x] / m_normalizedImageVector[m_widthOfXrayImg*y + m_I0Rect.x()])))*m_T0;
        }
    }


    float volumeOfPixels =0;


    QVector<float> thicknessDebug;

    for(int i = 0; i < thicknessOfPixels.size(); i++)
    {
        if(thicknessOfPixels[i] != 0)
            thicknessDebug.append(thicknessOfPixels[i]);
    }

    qDebug() << thicknessDebug;

    for(int x = m_weightRect.x(); x < m_weightRect.x() + m_weightRect.width(); x++)
    {
        for(int y = m_weightRect.y(); y < m_weightRect.y() + m_weightRect.height(); y++ )
        {
            volumeOfPixels += ( (m_speedOfColumn[x - m_weightRect.x()] * m_period * m_sizeOfDetector) / m_distancesOfDetectorsFromBetatron[y-m_weightRect.y()])*
                    (( ( (m_distancesOfDetectorsFromBetatron[y- m_weightRect.y()] -  m_distancesOfDetectorsFromMiddle[y - m_weightRect.y()]) + thicknessOfPixels[m_widthOfXrayImg*y + x])*
                    ( (m_distancesOfDetectorsFromBetatron[y- m_weightRect.y()] -  m_distancesOfDetectorsFromMiddle[y - m_weightRect.y()]) + thicknessOfPixels[m_widthOfXrayImg*y + x])  -
                    ( (m_distancesOfDetectorsFromBetatron[y- m_weightRect.y()] -  m_distancesOfDetectorsFromMiddle[y - m_weightRect.y()]) - thicknessOfPixels[m_widthOfXrayImg*y + x])*
                    ( (m_distancesOfDetectorsFromBetatron[y- m_weightRect.y()] -  m_distancesOfDetectorsFromMiddle[y - m_weightRect.y()]) - thicknessOfPixels[m_widthOfXrayImg*y + x]) )/2);
        }
    }

    qDebug() << volumeOfPixels;
}

void ImageLoader::updateNormalizedImage()
{
    m_normalizedImageVector = QVector<float>(m_imageVector.size());

    QVector<float> columnSumVector(m_widthOfXrayImg);
    columnSumVector.fill(0);

    for(int x = 0; x < m_widthOfXrayImg; x++)
    {
        for(int y = m_calibRect.y(); y < m_calibRect.y() + m_calibRect.height(); y++)
        {
            columnSumVector[x] += static_cast<float>(m_imageVector[y*m_widthOfXrayImg + x]);
        }
        columnSumVector[x] /= static_cast<float>(m_calibRect.height());
    }

    float sumOfColumns = 0;

    for(int i = 0; i< columnSumVector.size(); i++)
    {
        sumOfColumns += columnSumVector[i];
    }

    float avgOfColumns = sumOfColumns / columnSumVector.size();

    for(int x = 0; x < columnSumVector.size(); x++)
    {
        columnSumVector[x] = columnSumVector[x] / avgOfColumns;
    }

//    qDebug() << columnSumVector;

        for(int y = 0; y < m_heightOfXrayImg; y++)
        {
            for(int x = 0; x < m_widthOfXrayImg; x++)
            {
                m_normalizedImageVector[m_widthOfXrayImg*y+x] = m_imageVector[m_widthOfXrayImg*y +x] / columnSumVector[x];
            }
        }


    QImage imgY(m_widthOfXrayImg, m_heightOfXrayImg, QImage::Format_RGB888);



    for(int y = 0; y < m_heightOfXrayImg; y++)
    {
        for(int x = 0; x < m_widthOfXrayImg; x++)
        {
            quint16 brightness = static_cast<quint16>(((static_cast<double>(m_normalizedImageVector[m_widthOfXrayImg*y +x]))/m_maxInt) * 255);

            imgY.setPixel(x,y, qRgb(brightness,brightness,brightness) );
        }
    }



    imgY.save("777.bmp");

}

void ImageLoader::toCalculateDistanceFromDividingLine()
{
    for(int count = 0; count < m_detectorCoordinates.size(); count++)
    {
        float n; // coefficient
        if((m_endOfDividingVector.y() - m_startOfDividingVector.y()) != 0)
        {
            float q = (m_endOfDividingVector.x() - m_startOfDividingVector.x()) / (m_startOfDividingVector.y() - m_endOfDividingVector.y());
            float sn = (m_detectorCoordinates[count].x() - m_betatronCoordinates.x()) + (m_detectorCoordinates[count].y() - m_betatronCoordinates.y())*q;
            float fn = (m_detectorCoordinates[count].x() - m_startOfDividingVector.x()) + (m_detectorCoordinates[count].y() - m_startOfDividingVector.y())*q;
            n = fn / sn;
        }else
        {
            if(!(m_detectorCoordinates[count].y() - m_betatronCoordinates.y())){ return; }
            n = (m_detectorCoordinates[count].y() - m_startOfDividingVector.y()) / (m_detectorCoordinates[count].y() - m_betatronCoordinates.y());
        }

        m_pointsOfCrossing.push_back(QPoint(m_detectorCoordinates[count].x() + (m_betatronCoordinates.x() - m_detectorCoordinates[count].x())*n, m_detectorCoordinates[count].y() + (m_betatronCoordinates.y() - m_detectorCoordinates[count].y())*n));
    }

    for(int count = 0; count/8 < m_pointsOfCrossing.size(); count++)
    {
        if(m_pointsOfCrossing[count/8].y() <= m_endOfDividingVector.y())
        {
            QPoint distancePoint = m_detectorCoordinates[count/8] - m_pointsOfCrossing[count/8];

            m_distancesOfDetectorsFromMiddle.push_back(distancePoint.manhattanLength());
        }else
        {
            m_distancesOfDetectorsFromMiddle.push_back(-1);
        }
    }

    //qDebug() << m_distancesOfDetectorsFromMiddle.size();
}

void ImageLoader::toCalculateDistanceFromBetatron()
{
    for(int count = 0; count/8 < m_detectorCoordinates.size(); count++)
    {
        QPoint distancePoint = m_detectorCoordinates[count/8] - m_betatronCoordinates;

        m_distancesOfDetectorsFromBetatron.push_back(distancePoint.manhattanLength());
    }

    //qDebug() << m_distancesOfDetectorsFromBetatron;
}
