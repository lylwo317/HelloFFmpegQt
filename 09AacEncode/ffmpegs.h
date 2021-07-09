#ifndef FFMPEGS_H
#define FFMPEGS_H

extern "C"{
#include <libavformat/avformat.h>
}
#include <stdint.h>

#define printAvError(msg, ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf)); \
    qDebug() << msg;

struct PCMSpec{
    int sampleRate;
    AVSampleFormat sampleFmt;
    int chLayout;
};

class FFmpegs {
    public:
        FFmpegs();

        static void pcm2aac(const char *pcmFileName, PCMSpec &spec,
                const char *wavFileName);

};

#endif
