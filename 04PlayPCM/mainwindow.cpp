#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_playAudioBtn_clicked()
{
    if(!playThread){
        //新建线程
        playThread = new PlayThread(this);
        connect(playThread, &PlayThread::finished, this, [this](){
           ui->playAudioBtn->setText("开始播放");
           playThread = nullptr;
        });
        playThread->start();
        ui->playAudioBtn->setText("结束播放");
    }else {
        //结束线程
        playThread->requestInterruption();
        playThread = nullptr;
    }
}
