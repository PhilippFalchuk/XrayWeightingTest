#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "imageweighter.h"

#include <QMainWindow>
#include <QFile>
#include <QDebug>
#include <QVideoProbe>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QVector<quint32> m_imageVector;
    QVector<float> m_normalizedImageVector;

    quint32 m_widthOfXrayImg;
    quint32 m_heightOfXrayImg;
    QRect m_calibRect;
    QRect m_I0Rect;
    QRect m_weightRect;
    QVector<int> m_speedOfColumn;

    void loadImage();
};
#endif // MAINWINDOW_H
