#include "audiothread.h"
#include "ffmpegs.h"
#include <QDebug>
extern "C" {
//引用C语言的库，必须要阻止name mangling
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
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
    ReSampleSpec intSpec;
    intSpec.fileName = "../44100_s16_2_src.pcm";
    intSpec.sampleRate = 44100;
    intSpec.samplefmt = AV_SAMPLE_FMT_S16;
    intSpec.chLayout = AV_CH_LAYOUT_STEREO;

    ReSampleSpec outSpec1;
    outSpec1.fileName = "../48000_s16_1_dest.pcm";
    outSpec1.sampleRate = 48000;
    outSpec1.samplefmt = AV_SAMPLE_FMT_S16;
    outSpec1.chLayout = AV_CH_LAYOUT_MONO;

    ReSampleSpec outSpec2;
    outSpec2.fileName = "../48000_f32_1_dest.pcm";
    outSpec2.sampleRate = 48000;
    outSpec2.samplefmt = AV_SAMPLE_FMT_FLT;
    outSpec2.chLayout = AV_CH_LAYOUT_MONO;

    FFmpegs::audioResample(intSpec, outSpec1);
    FFmpegs::audioResample(intSpec, outSpec2);
}
