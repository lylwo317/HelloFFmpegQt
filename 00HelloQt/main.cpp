#include "mainwindow.h"

#include <QApplication>
extern "C" {
//FFmpeg是由C语言编写的,然后Qt使用的是C++,所以这里必须要用extern来以C语言的方式调用函数
//否则就会出现undefined错误
#include <libavcodec/avcodec.h>
}
#include <QtDebug>

int main(int argc, char *argv[])
{
    qDebug() << av_version_info();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
