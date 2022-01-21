#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_stream_0 = new MJPEGStreamer(this);
    m_stream_0->set_url("http://88.53.197.250/axis-cgi/mjpg/video.cgi?resolution=320x240");
    m_stream_1 = new MJPEGStreamer(this);
    m_stream_1->set_url("http://88.53.197.250/axis-cgi/mjpg/video.cgi?resolution=320x240");
    m_stream_2 = new MJPEGStreamer(this);
    m_stream_2->set_url("http://88.53.197.250/axis-cgi/mjpg/video.cgi?resolution=320x240");
    m_stream_3 = new MJPEGStreamer(this);
    m_stream_3->set_url("http://88.53.197.250/axis-cgi/mjpg/video.cgi?resolution=320x240");
    ui->glo_main->addWidget(m_stream_0, 0, 0);
    ui->glo_main->addWidget(m_stream_1, 1, 0);
    ui->glo_main->addWidget(m_stream_2, 0, 1);
    ui->glo_main->addWidget(m_stream_3, 1, 1);
}

MainWindow::~MainWindow()
{
    delete m_stream_0;
    delete m_stream_1;
    delete m_stream_2;
    delete m_stream_3;
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    m_stream_0->start();
    m_stream_1->start();
    m_stream_2->start();
    m_stream_3->start();
}

void MainWindow::on_pushButton_2_clicked()
{
    m_stream_0->stop();
    m_stream_1->stop();
    m_stream_2->stop();
    m_stream_3->stop();
}
