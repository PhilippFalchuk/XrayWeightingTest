#ifndef IMAGEWEIGHTER_H
#define IMAGEWEIGHTER_H

#include <QObject>
#include <QFile>
#include <QDebug>
#include <QDataStream>
#include <QRect>
#include <QSettings>
#include <QImage>
#include <QQueue>
#include <QPainter>
#include <math.h>

class ImageWeighter : public QObject
{
    Q_OBJECT
public:
    explicit ImageWeighter(QObject *parent = nullptr);

    QVector<quint32> loadImage();
    float measureWeightOfImage();
    void updateNormalizedImage();

private:
    QSettings* m_settings = nullptr;

    QVector<quint32> m_imageVector;
    QVector<float> m_normalizedImageVector;

    quint32 m_widthOfXrayImg;
    quint32 m_heightOfXrayImg;
    QRect m_calibRect;
    QRect m_I0Rect;
    QRect m_weightRect;
    quint32 m_maxInt;

    const float m_T0 = 24;
    const float m_period = 0.0025;
    const float m_sizeOfDetector = 1.2;

    QVector<QPoint> m_detectorCoordinates;

    QVector<QPoint> m_pointsOfAngles;
    QVector<QPoint> m_pointsOfCrossing;
    QVector<int> m_distancesOfDetectorsFromDividingLine;
    QVector<int> m_distancesOfDetectorsFromBetatron;
    QVector<int> m_speedOfColumn;
    QPoint m_betatronCoordinates;
    QPoint m_ACoordinates;
    QPoint m_BCoordinates;
    QPoint m_CCoordinates;
    QPoint m_DCoordinates;
    QPoint m_endOfDividingVector;
    QPoint m_startOfDividingVector;
    QVector<int> m_numOfDetectors;
    int m_firstSegmentNumOfDetectors;
    int m_secondSegmentNumOfDetectors;
    int m_thirdSegmentNumOfDetectors;
    int m_numOfSegments;

    void toCalculateDistanceFromDividingLineMid();
    void toCalculateDistanceFromDividingLineClose();
    void toCalculateDistanceFromDividingLineFar();
    void toCalculateDistanceFromBetatron();
    float toCalculateCoefficientofCrossing(int numOfDetector, int shiftOfDividingLine = 0);
};

#endif // IMAGEWEIGHTER_H
