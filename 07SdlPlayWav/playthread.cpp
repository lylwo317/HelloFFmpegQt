#include "playthread.h"
#include "SDL2/SDL_audio.h"
#include "SDL2/SDL_stdinc.h"
#include "SDL2/SDL_timer.h"
#include <QDebug>
#include <QFile>
#include <SDL2/SDL.h>
#include <cstddef>

#define FILENAME "../44100_s16_2_src.wav"
//PCM解析所需要的信息
#define SAMPLE_RATE 44100
#define SAMPLE_BIT_SIZE 16
#define CHANNELS 2

//SDL 音频缓冲区样本数
#define SDL_BUFFER_SAMPLES 1024
//BufferData缓冲样本数
#define BUFFER_SAMPLES 1024

//一个样本的字节数
#define ONE_SAMPLE_BYTES ((SAMPLE_BIT_SIZE / 8)  * CHANNELS)
#define BUFFER_SIZE ONE_SAMPLE_BYTES * BUFFER_SAMPLES

struct AudioBuffer{
    int len = 0;
    int fill_Len = 0;//填充到sdl的大小，为了计算线程结束的时间
    Uint8 *data = nullptr;
};


PlayThread::PlayThread(QObject *parent) : QThread(parent)
{
    //线程结束后，释放对象
    connect(this, &PlayThread::finished, this, &PlayThread::deleteLater);
}

//char * bufferData;
//int bufferLen;

PlayThread::~PlayThread(){
    disconnect();
    requestInterruption();
    quit();
    wait();
    qDebug() << this << "被析构了";
}

void fill_audio(void *userdata, 
        Uint8 * stream, //将数据填充到stream中
        int len){//希望填充的大小，其实就是SDL_AudioSpec 中的（sample * channels * (bit_size / 8)）
    AudioBuffer* audiobuffer = (AudioBuffer *)userdata;

    SDL_memset(stream, 0, len);
    
    //没有数据可以读取，提前结束
    if (audiobuffer->len <= 0) {
        return;
    }

    audiobuffer->fill_Len = audiobuffer->len < len ? audiobuffer->len : len;

    //填充数据
    SDL_MixAudio(stream, audiobuffer->data, audiobuffer->fill_Len, SDL_MIX_MAXVOLUME);

    audiobuffer->data += audiobuffer->fill_Len;
    audiobuffer->len -= audiobuffer->fill_Len;
}

void PlayThread::run()
{
    int result = SDL_Init(SDL_INIT_AUDIO);
    if(result < 0){
        qDebug() << "sdl_init failed";
        return;
    }

    SDL_AudioSpec spec{};

    Uint8 *data = nullptr;

    Uint32 len = 0;

    if (!SDL_LoadWAV(FILENAME, &spec, &data, &len)) {
       qDebug()  << "SDL_LoadWAV error." << SDL_GetError();
       SDL_Quit();
       return;
    }

    //音频缓冲区的样本数量（这个值必须是2的幂）
    spec.samples = SDL_BUFFER_SAMPLES;
    //设置回调
    spec.callback = fill_audio;

    AudioBuffer audiobuffer;
    audiobuffer.data = data;
    audiobuffer.len = len;
    spec.userdata = &audiobuffer;

    //打开音频设备
    result = SDL_OpenAudio(&spec, nullptr);
    if (result < 0) {
       qDebug() << "open audio device failed";
       SDL_Quit();
       return ;
    }

    QFile file(FILENAME);
    if (!file.open(QFile::ReadOnly)) {
       qDebug() << "open " << FILENAME << " file failed.";
       SDL_Quit();
       return ;
    }

    //开始播放
    SDL_PauseAudio(0);

    while (!isInterruptionRequested()) {
        if (audiobuffer.len > 0) { //sdl没有读取完
           continue;
        }

        /* audiobuffer.len = file.read(data, BUFFER_SIZE); */

        if (audiobuffer.len <= 0) {
            //文件已经读取完毕，这里要计算已经填充了多少数据
           int sampleCount = audiobuffer.fill_Len / ONE_SAMPLE_BYTES; 
           int ms = sampleCount * 1000 / SAMPLE_RATE; //ms
           qDebug() << "等待" << ms << "ms";
           SDL_Delay(ms);
           break; 
        }

        audiobuffer.data = (Uint8 *)data;
    }

    msleep(1000);


    file.close();


    SDL_Quit();
}
