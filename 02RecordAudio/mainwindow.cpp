#include "mainwindow.h"
#include "ui_mainwindow.h"
extern "C"{
#include <libavformat/avformat.h>
}
#include <QDebug>
#include <QFile>

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


void MainWindow::on_recordBtn_clicked()
{
    AVInputFormat *fmt = av_find_input_format("pulse");
    if(!fmt){
        qDebug() << "找到不这个 AVInputFormat";
        return;
    }

    AVFormatContext *ctx = nullptr;
    int result = avformat_open_input(&ctx,"alsa_input.pci-0000_00_1f.3.analog-stereo",fmt, nullptr);
    if(result < 0){
        char buf[100];
        av_make_error_string(buf,sizeof (buf), result);
        qDebug() << "打开设备失败" << buf;
        avformat_close_input(&ctx);
        return;
    }

    QFile file("out.pcm");
    if(!file.open(QFile::WriteOnly)){
        qDebug() << "文件打开失败";
        avformat_close_input(&ctx);
        return;
    }

    int count = 50;
    AVPacket pkt;

    qDebug() << "正在录制";
    while (count-- > 0 && av_read_frame(ctx, &pkt) == 0) {
        file.write((const char *)pkt.data,pkt.size);
    }
    qDebug() << "录制结束";

    file.close();

    avformat_close_input(&ctx);

}
