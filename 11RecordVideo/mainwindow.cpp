#include "mainwindow.h"

#include <QString>
#include <QTime>

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() { delete ui; }
void MainWindow::on_recordVideoBtn_clicked()
{
    if (!audioThread) {
        //新建线程
        audioThread = new AudioThread(this);
        connect(audioThread, &AudioThread::finished, this, [this]() {
            ui->recordVideoBtn->setText("开始录制视频");
            audioThread = nullptr;
        });
        audioThread->start();
        ui->recordVideoBtn->setText("结束录制");
    } else {
        //结束线程
        audioThread->requestInterruption();
        audioThread = nullptr;
    }
}
