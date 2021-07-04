#ifndef FFMPEGS_H
#define FFMPEGS_H

#include <stdint.h>

#define AUDIO_FORMAT_PCM 1
#define AUDIO_FORMAT_FLOAT 3

struct WAVHeader {
    WAVHeader(uint16_t audioFormat, uint16_t numChannels, uint32_t sampleRate, uint16_t bitsPerSample):
        audioFormat(audioFormat), numChannels(numChannels), sampleRate(sampleRate), bitsPerSample(bitsPerSample)
    {
        byteRate = sampleRate * blockAlign;
        blockAlign = bitsPerSample * numChannels >> 3;
    }
  // RIFF chunkId
  uint8_t chunkId[4] = {'R', 'I', 'F', 'F'}; // 4 bytes
  // 从chunkSize下面开始到文件结束的size = 4 + (8 + SubChunk1Size) + (8 +
  // SubChunk2Size)
  uint32_t chunkSize = 0; // 8 bytes
  uint8_t format[4] = {'W', 'A', 'V', 'E'};

  // subChunk1
  uint8_t fmtSubchunkID[4] = {'f', 'm', 't', ' '};
  uint32_t fmtSubchunkSize = 16;
  // start sub1, size = 16 bytes
  uint16_t audioFormat = AUDIO_FORMAT_PCM;
  uint16_t numChannels = 0;   // Mono = 1, Stereo = 2
  uint32_t sampleRate = 0;    // 44100
  uint32_t byteRate = 0;      // SampleRate * blockAlign
  uint16_t blockAlign = 0;    // NumChannels * BitsPerSample/8
  uint16_t bitsPerSample = 0; // 8 bits = 8, 16 bits = 16, etc
  // end sub1

  // subChunk2
  uint8_t dataSubchunkID[4] = {'d', 'a', 't', 'a'};
  // PCM数据的Size
  uint32_t dataSubchunkSize = 0; // NumSamples * blockAlign
};

class FFmpegs {
    public:
        FFmpegs();

        static void pcm2wav(WAVHeader &header,
                const char *pcmFileName,
                const char *wavFileName);

};

#endif
