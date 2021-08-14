#include "ffmpegs.h"

#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>

#include <QDebug>
#include <QFile>

#include "libavcodec/packet.h"
extern "C" {
#include <libavcodec/avcodec.h>
}

#define IN_DATA_SIZE 2048

FFmpegs::FFmpegs() {}

static int decode(AVCodecContext *ctx, AVPacket *pkt, AVFrame *frame,
                  QFile &outFile) {
  int ret = avcodec_send_packet(ctx, pkt);
  if (ret < 0) {
    printAvError("avcodec_send_packet failed.", ret);
    return ret; //出错了，结束解码
  }

  while (true) {
    ret = avcodec_receive_frame(ctx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      return 0; // retry
    } else if (ret < 0) {
      printAvError("avcodec_receive_frame failed.", ret);
      return ret; //出错了，结束解码
    }

    outFile.write((const char *)frame->data[0], frame->linesize[0]);
  }
}

//解码
void FFmpegs::aac2pcm(const char *aacFileName, const char *pcmFileName) {
  char inDataArray[IN_DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
  char *inData = inDataArray;

  int inLen;

  QFile inputFile(aacFileName);
  QFile outputFile(pcmFileName);

  AVCodec *codec = nullptr;
  AVCodecParserContext *pctx = nullptr;
  AVCodecContext *ctx = nullptr;

  //存放解码前的数据
  AVPacket *pkt = nullptr;
  //存放解码后的数据
  AVFrame *frame = nullptr;

  int result = 0;

  codec = avcodec_find_decoder_by_name("libfdk_aac");
  if (!codec) {
    qDebug() << "decoder not found";
    return;
  }

  ctx = avcodec_alloc_context3(codec);
  if (!ctx) {
    qDebug() << "avcodec_alloc_context3 failed";
    goto end;
  }

  pctx = av_parser_init(codec->id);
  if (!pctx) {
    qDebug() << "av_parser_init failed.";
    goto end;
  }

  pkt = av_packet_alloc();
  if (!pkt) {
    qDebug() << "av_packet_alloc failed.";
    goto end;
  }

  frame = av_frame_alloc();
  if (!frame) {
    qDebug() << "av_frame_alloc failed.";
    goto end;
  }

  result = avcodec_open2(ctx, codec, nullptr);
  if (result < 0) {
    printAvError("avcodec_open2 failed.", result);
    goto end;
  }

  if (!inputFile.open(QFile::ReadOnly)) {
    qDebug() << "input file open failed";
    goto end;
  }

  if (!outputFile.open(QFile::WriteOnly)) {
    qDebug() << "output file open failed";
    goto end;
  }

  while ((inLen = inputFile.read(inDataArray, IN_DATA_SIZE)) > 0) {
    inData = inDataArray;

    while (inLen > 0) {
      int usedSize = av_parser_parse2(pctx, ctx, &pkt->data, &pkt->size,
                                      (const uint8_t *)inData, inLen,
                                      AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

      if (usedSize < 0) {
        printAvError("av_parser_parse2 failed.", usedSize);
        goto end;
      }

      inData += usedSize;
      inLen -= usedSize;

      if (pkt->size > 0 && decode(ctx, pkt, frame, outputFile) < 0) {
        //出错了
        goto end;
      }
    }
  }

end:

  inputFile.close();
  outputFile.close();

  if (ctx) {
    avcodec_free_context(&ctx);
  }

  if (pctx) {
    av_parser_close(pctx);
    pctx = nullptr;
  }

  if (pkt) {
    av_packet_free(&pkt);
  }

  if (frame) {
    av_frame_free(&frame);
  }
}
