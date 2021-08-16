#include "mainwindow.h"

#include <QString>
#include <QTime>

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    _widget = new QWidget(this);

    _widget->setGeometry(180, 30,512,512);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_displayBtn_clicked()
{

    if (!playThread) {
        //新建线程
//        playThread = new PlayThread((void *)ui->label->winId(), this);
        playThread = new PlayThread((void *)_widget->winId(), this);
        connect(playThread, &PlayThread::finished, this, [this]() {
            ui->displayBtn->setText("显示YUV");
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
