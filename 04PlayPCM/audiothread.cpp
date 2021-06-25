#include "audiothread.h"
#include <QDebug>
extern "C" {
//引用C语言的库，必须要阻止name mangling
#include <libavformat/avformat.h>
}
#include <QFile>

AudioThread::AudioThread(QObject* parent): QThread(parent)
{
    //线程结束后，自动销毁堆中的对象
    connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread(){
    requestInterruption();//中断线程
    quit();
    wait();//等待线程结束后，再析构
    qDebug() << "析构 AudioThread";
}

void AudioThread::run(){
    qDebug() << "启动线程AudioThread";

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

    AVPacket pkt;

    bool isBegin = false;
    while (!isInterruptionRequested() && av_read_frame(ctx, &pkt) == 0) {
        if(isBegin){
            file.write((const char *)pkt.data,pkt.size);
        }
        int16_t *p = (int16_t *)pkt.data;
        if(!isBegin && *p != 32767 && *p != -32768){
            qDebug() << "正在录制" << *p;
            isBegin = true;
        }
    }
    qDebug() << "录制结束";

    file.close();

    avformat_close_input(&ctx);


    qDebug() << "结束线程AudioThread";
}
