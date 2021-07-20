#include "audiothread.h"

#include <QDebug>

#include "ffmpegs.h"
#include "libavutil/channel_layout.h"
#include "libavutil/samplefmt.h"
extern "C" {
//引用C语言的库，必须要阻止name mangling
#include <libavformat/avformat.h>
}
#include <QFile>

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
    const char *aacFileName = "../44100_s16_2_src.aac";
    const char *pcmFileName = "../out.pcm";
    PCMSpec spec;
    spec.chLayout = AV_CH_LAYOUT_STEREO;
    spec.sampleFmt = AV_SAMPLE_FMT_S16;
    spec.sampleRate = 44100;

    FFmpegs::aac2pcm(aacFileName, pcmFileName);
}
