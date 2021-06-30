#include "audiothread.h"
#include "ffmpegs.h"
#include <QDebug>
extern "C" {
//引用C语言的库，必须要阻止name mangling
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include "libavcodec/codec_par.h"
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

    QString filename = "out";
    QString wavFilename = filename;
    filename += ".pcm";
    wavFilename += ".wav";

    QFile file(filename);
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

    //pcm转wav
    AVStream *stream = ctx->streams[0];
    AVCodecParameters *p = stream->codecpar;

    //获取输入流参数信息，然后填写到wavheader中
    WAVHeader header;
    header.sampleRate = p->sample_rate;
    header.bitsPerSample = av_get_bits_per_sample(p->codec_id);
    header.numChannels = p->channels;
    if (p->codec_id >= AV_CODEC_ID_PCM_F32BE) {
        header.audioFormat = AUDIO_FORMAT_FLOAT;
    }

    FFmpegs::pcm2wav(header,
            filename.toUtf8().data(),
            wavFilename.toUtf8().data());


    avformat_close_input(&ctx);


    qDebug() << "结束线程AudioThread";
}
