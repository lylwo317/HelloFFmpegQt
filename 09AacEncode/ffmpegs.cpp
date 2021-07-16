#include "ffmpegs.h"

#include <libavutil/channel_layout.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>

#include <QDebug>
#include <QFile>
extern "C" {
#include <libavcodec/avcodec.h>
}

FFmpegs::FFmpegs() {}

static int checkSampleFmt(const AVCodec& codec, enum AVSampleFormat sampleFmt) {
    const enum AVSampleFormat* p = codec.sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sampleFmt) {
            return 1;
        }
        p++;
    }

    return 0;
}

static int encode(AVCodecContext* ctx, AVFrame* frame, AVPacket* pkt,
                  QFile* aacFile) {
    int result = avcodec_send_frame(ctx, frame);
    if (result < 0) {
        printAvError("avcodec_send_frame error.", result);
        return result;
    }

    while (true) {
        result = avcodec_receive_packet(ctx, pkt);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            // EAGAIN 说明数据不够编码，需要等待用户继续填充数据到frame
            // AVERROR_EOF 说明编码缓冲区已经刷新完了，没有需要receive的数据了
            //结束这一轮编码
            return 0;
        } else if (result < 0) {
            //出错了，返回错误码
            return result;
        }

        //将编码生成的数据写入到文件
        aacFile->write((const char*)pkt->data, pkt->size);

        /* av_packet_unref(pkt); */
    }
}

/*
 * 1. 获取编码器(AVCodec)
 * 2. 检查pcm的格式是否符合编码器的要求，如果不符合就无法编码
 * 3.
 */
void FFmpegs::pcm2aac(const char* pcmFileName, PCMSpec& spec,
                      const char* aacFileName) {
    QFile pcmFile(pcmFileName);
    QFile aacFile(aacFileName);

    AVCodec* codec = nullptr;

    codec = avcodec_find_encoder_by_name("libfdk_aac");
    if (!codec) {
        qDebug() << "encoder not found";
        return;
    }

    //检测pcm的参数是否符合要求，不符合就需要重新采样之后再编码

    if (!checkSampleFmt(*codec, spec.sampleFmt)) {
        qDebug() << "unspport sample format"
                 << av_get_sample_fmt_name(spec.sampleFmt);
        return;
    }

    AVCodecContext *ctx = nullptr;
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        qDebug() << "avcodec_alloc_context3 error.";
        return;
    }

    //由于编码器无法知道PCM的参数，需要我们手动赋值告诉编码器上下文
    ctx->sample_fmt = spec.sampleFmt;
    ctx->channel_layout = spec.chLayout;
    ctx->sample_rate = spec.sampleRate;

    ctx->bit_rate = 32000;
    //规格
    ctx->profile = FF_PROFILE_AAC_HE_V2;

    AVFrame* frame = nullptr;
    AVPacket* packet = nullptr;

    int result = avcodec_open2(ctx, codec, nullptr);
    if (result < 0) {
        printAvError("avcodec_open2 error.", result);
        goto fail;
    }

    //存放pcm，传送给编码器。编码后用AVPacket获取编码后的数据
    frame = av_frame_alloc();
    if (!frame) {
        qDebug() << "av_frame_alloc error.";
        goto fail;
    }

    frame->nb_samples = ctx->frame_size;
    frame->format = spec.sampleFmt;
    frame->channel_layout = spec.chLayout;

    //创建缓冲区frame->buf frame->data
    result = av_frame_get_buffer(frame, 0);
    if (result < 0) {
        printAvError("av_frame_get_buffer error.", result);
        goto fail;
    }

    packet = av_packet_alloc();
    if (!packet) {
        qDebug() << "av_packet_alloc error.";
        goto fail;
    }

    if (!pcmFile.open(QFile::ReadOnly)) {
        qDebug() << "pcmFile open error";
        goto fail;
    }

    if (!aacFile.open(QFile::WriteOnly)) {
        qDebug() << "aacFile open error";
        goto fail;
    }

    while ((result = pcmFile.read((char*)frame->data[0], frame->linesize[0])) >
           0) {
        //不足
        if (result < frame->linesize[0]) {
            //设置真正有效的样本数量
            //防止编码器将无效的数据也编码
            int bytesPerSample =
                av_get_bytes_per_sample((AVSampleFormat)frame->format);
            int ch = av_get_channel_layout_nb_channels(frame->channel_layout);
            frame->nb_samples = result / (bytesPerSample * ch);
        } else {
            frame->nb_samples = ctx->frame_size;
        }

        if (encode(ctx, frame, packet, &aacFile)) {
            goto fail;
        }
    }

    encode(ctx, nullptr, packet, &aacFile);

fail:
    if (ctx) {
        avcodec_free_context(&ctx);
    }
    if (frame) {
        av_frame_free(&frame);
    }
}
