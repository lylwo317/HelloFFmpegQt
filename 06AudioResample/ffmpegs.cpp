#include "ffmpegs.h"
#include <QFile>
#include <QDebug>
#include <stdint.h>

extern "C" {
#include "libswresample/swresample.h"
#include "libavutil/mathematics.h"
#include "libavutil/mem.h"
#include "libavutil/samplefmt.h"
}

FFmpegs::FFmpegs(){

}

#define printAvError(msg, ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf)); \
    qDebug() << msg;

/*
 * 创建重采样上下文
 * 创建输入缓冲区
 * 创建输出缓冲区
 * 执行重采样
 * 写入到新的文件
 */
void FFmpegs::audioResample(ReSampleSpec &in, ReSampleSpec &out){
    SwrContext *swr = nullptr;
    /* swr = swr_alloc_set_opts(swr, out.format, enum AVSampleFormat out_sample_fmt, int out_sample_rate, int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate, int log_offset, void *log_ctx); */
    swr = swr_alloc_set_opts(nullptr,
            out.chLayout, out.samplefmt, out.sampleRate,
            in.chLayout, in.samplefmt, in.sampleRate, 0, nullptr);

    int result = 0;

    QFile inFile(in.fileName);
    QFile outFile(out.fileName);

    uint8_t **inData = nullptr;
    int inLineSize = 0;
    int inChannels = av_get_channel_layout_nb_channels(in.chLayout);
    int inPerSampleFramesBytes = inChannels * av_get_bytes_per_sample(in.samplefmt);
    //输入缓冲区样本数
    int inSamplesNumber = 1024;
    int inNum = 0;

    uint8_t **outData = nullptr;
    int outLineSize = 0;
    int outChannels = av_get_channel_layout_nb_channels(out.chLayout);
    int outPerSampleFramesBytes = outChannels * av_get_bytes_per_sample(out.samplefmt);
    //输入缓冲区样本数，这个要计算。因为比特率或者声道数变化的时候，输出缓冲区的样本数跟输入缓冲区的不一样
    // 总时长 / 输出采样率
    // 输入样本数 / 输入采样率 = 输出样本数 / 输出采样率
    /* int outSamplesNumber = inSamplesNumber * out.sampleRate / in.sampleRate ; */
    int outSamplesNumber = av_rescale_rnd(inSamplesNumber, out.sampleRate, in.sampleRate, AV_ROUND_UP);

    int len = 0;

    if (!swr) {
        qDebug() << "swr_alloc_set_opts failed";
        goto fail;
    }

    //初始化SwrContext
    result = swr_init(swr);
    if (result < 0) {
        printAvError("swr init error: ", result);
        goto fail;
    }

    //创建输入缓冲区

    result = av_samples_alloc_array_and_samples(&inData, &inLineSize, inChannels, inSamplesNumber, in.samplefmt, 0);

    if (result < 0) {
        printAvError("av_samples_alloc_array_and_samples failed ", result);
        goto fail;
    }

    //创建输出缓冲区

    av_samples_alloc_array_and_samples(&outData, &outLineSize, outChannels,
                                       outSamplesNumber, out.samplefmt, 0);

    if (result < 0) {
        printAvError("av_samples_alloc_array_and_samples failed ", result);
        goto fail;
    }

    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "open file fail. " ;
        goto fail;
    }

    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "open file fail. " ;
        goto fail;
    }

    while ((len = inFile.read((char *)inData[0], inLineSize)) > 0) {
        inNum = len / inPerSampleFramesBytes;
        result = swr_convert(swr, outData, outSamplesNumber, (const uint8_t **)inData, inNum);
        if (result < 0) {
            printAvError("swr_convert error. ", result);
            goto fail;
        }
        outFile.write((const char *)outData[0], result * outPerSampleFramesBytes);
    }

    while ((result = swr_convert(swr, outData, outSamplesNumber, nullptr, 0)) > 0) {
        outFile.write((const char *)outData[0], result * outPerSampleFramesBytes);
    }


fail:

    inFile.close();
    outFile.close();

    if (inData[0]) {
        av_freep(&inData[0]);
    }

    av_freep(&inData);

    if (outData[0]) {
        av_freep(&outData[0]);
    }

    av_freep(&outData);

    if (!swr) {
        swr_free(&swr);
    }
    return;
}
