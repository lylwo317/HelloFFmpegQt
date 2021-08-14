#include "mainwindow.h"

#include <QString>
#include <QTime>

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_displayBtn_clicked()
{

    if (!playThread) {
        //新建线程
        playThread = new PlayThread(this);
        connect(playThread, &PlayThread::finished, this, [this]() {
            ui->displayBtn->setText("显示BMP");
            playThread = nullptr;
        });
        playThread->start();
        ui->displayBtn->setText("结束");
    } else {
        //结束线程
        playThread->requestInterruption();
        playThread = nullptr;
    }
}
