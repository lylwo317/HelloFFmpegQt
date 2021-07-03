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


void MainWindow::on_recordAudioBtn_clicked()
{
    if(!audioThread){
        //新建线程
        audioThread = new AudioThread(this);
        connect(audioThread, &AudioThread::finished, this, [this](){
           ui->recordAudioBtn->setText("开始重采样");
           audioThread = nullptr;
        });
        audioThread->start();
        ui->recordAudioBtn->setText("结束重采样");
    }else {
        //结束线程
        audioThread->requestInterruption();
        audioThread = nullptr;
    }

}
