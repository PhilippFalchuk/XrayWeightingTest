#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "imageloader.h"
#include <QFile>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onImageSizeRecieved(quint32 width, quint32 height);

private:
    Ui::MainWindow *ui;

    quint32 m_imageWidth;
    quint32 m_imageHeight;

};
#endif // MAINWINDOW_H
