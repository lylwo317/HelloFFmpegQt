#include "audiothread.h"
#include <QDebug>
#include "ffmpegs.h"
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
    int result = avformat_open_input(&ctx,"alsa_output.pci-0000_28_00.4.analog-stereo.monitor",fmt, nullptr);
    if(result < 0){
        char buf[100];
        av_make_error_string(buf,sizeof (buf), result);
        qDebug() << "打开设备失败" << buf;
        avformat_close_input(&ctx);
        return;
    }

    QFile file("out.wav");
    if(!file.open(QFile::WriteOnly)){
        qDebug() << "文件打开失败";
        avformat_close_input(&ctx);
        return;
    }

    AVStream *stream = ctx->streams[0];
    AVCodecParameters *avPara =  stream->codecpar;

    //wirte header
    int audioFormat = AUDIO_FORMAT_PCM;
    if(avPara->codec_id >= AV_CODEC_ID_PCM_F32BE){
        audioFormat = AUDIO_FORMAT_FLOAT;
    }
    WAVHeader header(audioFormat,
                     avPara->channels,
                     avPara->sample_rate,
                     av_get_bits_per_sample(avPara->codec_id));

    file.write((const char *)&header, sizeof (WAVHeader));

    AVPacket pkt;

    bool isBegin = false;
    uint64_t pcmSize = 0;
    int perSampleFramsBytes = header.bitsPerSample * header.numChannels >> 3;
    while (!isInterruptionRequested() && av_read_frame(ctx, &pkt) == 0) {
        if(isBegin){
            file.write((const char *)pkt.data,pkt.size);
            pcmSize += pkt.size;
            qint64 ms = 1000 * pcmSize / perSampleFramsBytes / header.sampleRate;
            emit onTimeChanged(ms);
        }
        int16_t *dataCur = (int16_t *)pkt.data;
        if(!isBegin && *dataCur != 32767 && *dataCur != -32768){
            qDebug() << "正在录制" << *dataCur;
            isBegin = true;
        }
    }
    qDebug() << "录制结束";
    file.seek(offsetof(WAVHeader, chunkSize));
    header.chunkSize = pcmSize + sizeof (WAVHeader)
            - sizeof (header.chunkId)
            - sizeof (header.chunkSize);
    file.write((const char *)&header.chunkSize, sizeof(header.chunkSize));

    file.seek(offsetof(WAVHeader, dataSubchunkSize));
    header.dataSubchunkSize = pcmSize;
    file.write((const char *)&header.dataSubchunkSize, sizeof(header.dataSubchunkSize));


    file.close();

    avformat_close_input(&ctx);


    qDebug() << "结束线程AudioThread";
}
