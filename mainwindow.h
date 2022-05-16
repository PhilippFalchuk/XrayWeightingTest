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

    void loadImage();
};
#endif // MAINWINDOW_H
