#include "audiothread.h"

#include <QDebug>

#include "ffmpegs.h"
extern "C" {
//引用C语言的库，必须要阻止name mangling
#include "libavutil/channel_layout.h"
#include "libavutil/samplefmt.h"
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}
#include <QFile>

#define printAvError(msg, ret)                \
    char errbuf[1024];                        \
    av_strerror(ret, errbuf, sizeof(errbuf)); \
    qDebug() << msg << errbuf;

#define FILE_NAME "out.yuv"
#define DEVICE_NAME "0"
#define FORMAT_NAME "avfoundation"

AudioThread::AudioThread(QObject *parent) : QThread(parent) {
    //线程结束后，自动销毁堆中的对象
    connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
    requestInterruption();  //中断线程
    quit();
    wait();  //等待线程结束后，再析构
    qDebug() << "析构 AudioThread";
}

void AudioThread::run() {

    //获取输入格式对象（mac：avfoundation）

    //由于是从硬件设备获取。这里得手动指定inputformat
    //否则如果是打开mp4这种文件，就直接传nullptr调用avformat_open_input就可以了。内部会
    //根据文件扩展名生成AVInputFormat
    AVInputFormat* inputFormat =  av_find_input_format(FORMAT_NAME);
    if(!inputFormat){
        qDebug() << "av_find_input_format failed.";
        return;
    }

    //设备参数
    AVDictionary* avDic = nullptr;
    av_dict_set(&avDic, "video_size", "640x480", 0);
    av_dict_set(&avDic, "pixel_format", "yuyv422", 0);
    av_dict_set(&avDic, "framerate", "30", 0);

    AVFormatContext* avfCtx = nullptr;

    //打开设备
    int ret = avformat_open_input(&avfCtx, DEVICE_NAME, inputFormat, &avDic);

    //打开mp4之类的文件，不需要指定inputFormat
    //avformat_open_input(&pFormatCtx, filepath, NULL, NULL);
    if(ret < 0){
        printAvError("avformat_open_input failed.",ret);
        return;
    }

    //打开输出文件
    QFile outFile(FILE_NAME);
    if(!outFile.open(QFile::WriteOnly)){
        qDebug() << "file open failed.";
        avformat_close_input(&avfCtx);
        return;
    }

    //获取一帧yuv的大小
    AVCodecParameters* paramter = avfCtx->streams[0]->codecpar;
    AVPixelFormat pxf =  (AVPixelFormat)paramter->format;
    int imageSize = av_image_get_buffer_size(pxf,paramter->width,paramter->height,1);


    //循环读取YUV并输出到文件
    AVPacket *pkt = av_packet_alloc();
    while (!isInterruptionRequested()) {
        ret = av_read_frame(avfCtx, pkt);

        if(ret == 0){
//            outFile.write((const char *)pkt->data, pkt->size);
            outFile.write((const char *)pkt->data, imageSize);
            av_packet_unref(pkt);
        }else if (ret == AVERROR(EAGAIN)){
            continue;
        }else{
            printAvError("av_read_frame error", ret);
            break;
        }
    }

    av_packet_free(&pkt);

    outFile.close();

    avformat_close_input(&avfCtx);

    qDebug() << "录制结束";

}
