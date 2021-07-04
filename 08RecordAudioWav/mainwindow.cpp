#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QTime>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    on_time_changed(0);
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
        connect(audioThread, &AudioThread::onTimeChanged,
                this, &MainWindow::on_time_changed);
        connect(audioThread, &AudioThread::finished, this, [this](){
           ui->recordAudioBtn->setText("开始录音");
           audioThread = nullptr;
        });
        audioThread->start();
        ui->recordAudioBtn->setText("结束录音");
    }else {
        //结束线程
        audioThread->requestInterruption();
        audioThread = nullptr;
    }

}

void MainWindow::on_time_changed(qint64 ms)
{
       QTime time(0,0);
       QString str =  time.addMSecs(ms).toString("mm:ss.z");
       ui->timeLabel->setText(str.left(7));
}
