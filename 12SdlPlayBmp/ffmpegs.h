#ifndef FFMPEGS_H
#define FFMPEGS_H

extern "C" {
#include <libavformat/avformat.h>
}
#include <stdint.h>

#define printAvError(msg, ret)                \
    char errbuf[1024];                        \
    av_strerror(ret, errbuf, sizeof(errbuf)); \
    qDebug() << msg << errbuf;

struct PCMSpec {
    int sampleRate;
    AVSampleFormat sampleFmt;
    int chLayout;
};

class FFmpegs {
   public:
    FFmpegs();

    static void aac2pcm(const char *aacFileName, const char *pcmFileName);
};

#endif
