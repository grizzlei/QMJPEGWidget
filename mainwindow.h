#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mjpegstreamer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    MJPEGStreamer * m_stream_0;
    MJPEGStreamer * m_stream_1;
    MJPEGStreamer * m_stream_2;
    MJPEGStreamer * m_stream_3;
};
#endif // MAINWINDOW_H
