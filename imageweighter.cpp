#include "imageweighter.h"

ImageWeighter::ImageWeighter(QVector<float> normalizedImageVector, quint32 widthOfXrayImg,
                             quint32 heightOfXrayImg, QVector<int> speedColumn,
                              QObject *parent)
    : QObject{parent}
{
    m_settings = new QSettings("Geometry.ini", QSettings::IniFormat);
    qDebug() << m_settings->fileName();
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    m_startOfDividingVector = QPoint(m_endOfDividingVector.x(), m_betatronCoordinates.y());
    m_pointsOfAngles = QVector<QPoint>{m_ACoordinates, m_BCoordinates, m_CCoordinates, m_DCoordinates};
    m_numOfDetectors = QVector<int>{m_firstSegmentNumOfDetectors,m_secondSegmentNumOfDetectors,m_thirdSegmentNumOfDetectors};
    m_detectorCoordinates = QVector<QPoint>();

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

    m_normalizedImageVector = normalizedImageVector;
    m_widthOfXrayImg = widthOfXrayImg;
    m_heightOfXrayImg = heightOfXrayImg;
    m_speedOfColumn = speedColumn;

    toCalculateDistanceFromBetatron();
}



float ImageWeighter::measureWeightOfImage(QRect weightRect, QRect I0Rect, int hypotesis, bool& infinityFlag)
{
    m_imageIsDual = false;
    m_evenIsBigger = false;
    m_weightZoneHasInfinityThickness = false;


    m_weightRect = weightRect;
    m_I0Rect = I0Rect;

    m_hypotesis = hypotesis;

    if(m_hypotesis == 1){
        toCalculateDistanceFromDividingLineClose();
    }else if(m_hypotesis == 2){
        toCalculateDistanceFromDividingLineMid();
    }else if (m_hypotesis == 3){
        toCalculateDistanceFromDividingLineFar();
    }else{
        toCalculateDistanceFromDividingLineMid();
    }

    float evenColumn = 0;
    float unevenColumn = 0;

    for(int x = 0; x < m_widthOfXrayImg; x++)
    {
        for(int y = 0; y < m_heightOfXrayImg; y++)
        {
            if(x%2)
            {
                unevenColumn += m_normalizedImageVector[m_widthOfXrayImg*y + x];
            }else
            {
                evenColumn += m_normalizedImageVector[m_widthOfXrayImg*y + x];
            }
        }
    }

    if(evenColumn > unevenColumn)
    {
        qDebug() << "even is 9";
        m_evenIsBigger = true;
        if(evenColumn/unevenColumn > 1.2)
            m_imageIsDual = true;
    }else
    {
        qDebug() << "uneven is 9";
        m_evenIsBigger = false;
        if(unevenColumn/evenColumn > 1.2)
            m_imageIsDual = true;
    }

    QVector<float> thicknessOfPixels(m_normalizedImageVector.size());
    thicknessOfPixels.fill(0);

    for(int x = m_weightRect.x(); x < m_weightRect.x() + m_weightRect.width(); x++)
    {
        for(int y = m_weightRect.y(); y < m_weightRect.y() + m_weightRect.height(); y++ )
        {
            if(!m_imageIsDual)
            {
                thicknessOfPixels[m_widthOfXrayImg*y + x] = (-(logf(m_normalizedImageVector[m_widthOfXrayImg*y + x] / m_normalizedImageVector[m_widthOfXrayImg*y + m_I0Rect.x()])))*m_T0;
            }else if(m_evenIsBigger && !(x%2))
            {
                thicknessOfPixels[m_widthOfXrayImg*y + x] = (-(logf(m_normalizedImageVector[m_widthOfXrayImg*y + x] / m_normalizedImageVector[m_widthOfXrayImg*y + m_I0Rect.x() + (m_I0Rect.x()%2)])))*m_T0;
            }else if(!m_evenIsBigger && x%2)
            {
                thicknessOfPixels[m_widthOfXrayImg*y + x] = (-(logf(m_normalizedImageVector[m_widthOfXrayImg*y + x] / m_normalizedImageVector[m_widthOfXrayImg*y + m_I0Rect.x() + (1 - m_I0Rect.x()%2)])))*m_T0;
            }

            if(thicknessOfPixels[m_widthOfXrayImg*y + x] == qInf())
            {
                thicknessOfPixels[m_widthOfXrayImg*y + x] = 300;
                m_weightZoneHasInfinityThickness = true;
            }

            if(m_imageIsDual)
            {
                thicknessOfPixels[m_widthOfXrayImg*y /+ x] *= 2;
            }
        }
    }



    float volumeOfPixels =0;

    for(int x = m_weightRect.x(); x < m_weightRect.x() + m_weightRect.width(); x++)
    {
        for(int y = m_weightRect.y(); y < m_weightRect.y() + m_weightRect.height(); y++ )
        {
            volumeOfPixels += ( (m_speedOfColumn[x - m_weightRect.x()] * m_period * m_sizeOfDetector) / m_distancesOfDetectorsFromBetatron[y-m_weightRect.y()])*
                    (( ( (m_distancesOfDetectorsFromBetatron[y- m_weightRect.y()] -  m_distancesOfDetectorsFromDividingLine[y - m_weightRect.y()]) + thicknessOfPixels[m_widthOfXrayImg*y + x])*
                    ( (m_distancesOfDetectorsFromBetatron[y- m_weightRect.y()] -  m_distancesOfDetectorsFromDividingLine[y - m_weightRect.y()]) + thicknessOfPixels[m_widthOfXrayImg*y + x])  -
                    ( (m_distancesOfDetectorsFromBetatron[y- m_weightRect.y()] -  m_distancesOfDetectorsFromDividingLine[y - m_weightRect.y()]) - thicknessOfPixels[m_widthOfXrayImg*y + x])*
                    ( (m_distancesOfDetectorsFromBetatron[y- m_weightRect.y()] -  m_distancesOfDetectorsFromDividingLine[y - m_weightRect.y()]) - thicknessOfPixels[m_widthOfXrayImg*y + x]) )/2);
        }
    }


    infinityFlag = m_weightZoneHasInfinityThickness;

    if(volumeOfPixels < 0)
    {
        return -1;
    }

    return volumeOfPixels;
}

void ImageWeighter::toCalculateDistanceFromDividingLineMid()
{
    m_pointsOfCrossing = QVector<QPoint>();
    for(int count = 0; count < m_detectorCoordinates.size(); count++)
    {
        float n; // coefficient
        if(count <= m_numOfDetectors[0] + m_numOfDetectors[1] + m_numOfDetectors[2]/2)
        {
            n = toCalculateCoefficientofCrossing(count,0);
        }else
        {
            n = toCalculateCoefficientofCrossing(count,-150);
        }

        m_pointsOfCrossing.push_back(QPoint(m_detectorCoordinates[count].x() + (m_betatronCoordinates.x() - m_detectorCoordinates[count].x())*n, m_detectorCoordinates[count].y() + (m_betatronCoordinates.y() - m_detectorCoordinates[count].y())*n));
    }

    for(int count = 0; count/8 < m_pointsOfCrossing.size(); count++)
    {
        QPoint distancePoint = m_detectorCoordinates[count/8] - m_pointsOfCrossing[count/8];
        m_distancesOfDetectorsFromDividingLine.push_back(distancePoint.manhattanLength());
    }
}

void ImageWeighter::toCalculateDistanceFromDividingLineClose()
{
    m_pointsOfCrossing = QVector<QPoint>();
    for(int count = 0; count < m_detectorCoordinates.size(); count++)
    {
        float n = toCalculateCoefficientofCrossing(count, -150);
        m_pointsOfCrossing.push_back(QPoint(m_detectorCoordinates[count].x() + (m_betatronCoordinates.x() - m_detectorCoordinates[count].x())*n, m_detectorCoordinates[count].y() + (m_betatronCoordinates.y() - m_detectorCoordinates[count].y())*n));
    }

    for(int count = 0; count/8 < m_pointsOfCrossing.size(); count++)
    {
        QPoint distancePoint = m_detectorCoordinates[count/8] - m_pointsOfCrossing[count/8];
        m_distancesOfDetectorsFromDividingLine.push_back(distancePoint.manhattanLength());
    }
}

void ImageWeighter::toCalculateDistanceFromDividingLineFar()
{
    m_pointsOfCrossing = QVector<QPoint>();
    for(int count = 0; count < m_detectorCoordinates.size(); count++)
    {
        float n; // coefficient
        if(count <= m_numOfDetectors[0] + m_numOfDetectors[1])
        {
            n = toCalculateCoefficientofCrossing(count,150);
        }else if(count <= m_numOfDetectors[0] + m_numOfDetectors[1] + m_numOfDetectors[2]/2)
        {
            n = toCalculateCoefficientofCrossing(count,0);
        }else
        {
            n = toCalculateCoefficientofCrossing(count,-150);
        }

        m_pointsOfCrossing.push_back(QPoint(m_detectorCoordinates[count].x() + (m_betatronCoordinates.x() - m_detectorCoordinates[count].x())*n, m_detectorCoordinates[count].y() + (m_betatronCoordinates.y() - m_detectorCoordinates[count].y())*n));
    }

    for(int count = 0; count/8 < m_pointsOfCrossing.size(); count++)
    {
        QPoint distancePoint = m_detectorCoordinates[count/8] - m_pointsOfCrossing[count/8];
        m_distancesOfDetectorsFromDividingLine.push_back(distancePoint.manhattanLength());
    }
}

void ImageWeighter::toCalculateDistanceFromBetatron()
{
    for(int count = 0; count/8 < m_detectorCoordinates.size(); count++)
    {
        QPoint distancePoint = m_detectorCoordinates[count/8] - m_betatronCoordinates;
        m_distancesOfDetectorsFromBetatron.push_back(distancePoint.manhattanLength());
    }
}

float ImageWeighter::toCalculateCoefficientofCrossing(int numOfDetector, int shiftOfDividingLine)
{
    int detNum = numOfDetector;
    if((m_endOfDividingVector.y() - m_startOfDividingVector.y()) != 0)
    {
        float q = ((m_endOfDividingVector.x()) - (m_startOfDividingVector.x())) / (m_startOfDividingVector.y() - m_endOfDividingVector.y());
        float sn = (m_detectorCoordinates[detNum].x() - m_betatronCoordinates.x()) + (m_detectorCoordinates[detNum].y() - m_betatronCoordinates.y())*q;
        float fn = (m_detectorCoordinates[detNum].x() - (m_startOfDividingVector.x() + shiftOfDividingLine)) + (m_detectorCoordinates[detNum].y() - m_startOfDividingVector.y())*q;
        return  fn / sn;
    }else
    {
        if(!(m_detectorCoordinates[detNum].y() - m_betatronCoordinates.y())){ return -500000000.; }
        return (m_detectorCoordinates[detNum].y() - m_startOfDividingVector.y()) / (m_detectorCoordinates[detNum].y() - m_betatronCoordinates.y());
    }
}
