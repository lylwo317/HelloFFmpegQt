#include "playthread.h"
#include <QDebug>
#include <QFile>
#include <SDL2/SDL.h>

#define FILENAME "/home/kevin/QtProjects/HelloFFmpegQt/04PlayPCM/out.pcm"
//PCM解析所需要的信息
#define SAMPLE_RATE 44100
#define SAMPLE_SIZE 16
#define CHANNELS 2

//SDL 音频缓冲区样本数
#define SDL_BUFFER_SAMPLES 4096
//BufferData缓冲样本数
#define BUFFER_SAMPLES 4096

//一个样本的字节数
#define ONE_SAMPLE_BYTES ((SAMPLE_SIZE * CHANNELS)/8)
#define BUFFER_SIZE ONE_SAMPLE_BYTES * BUFFER_SAMPLES

PlayThread::PlayThread(QObject *parent) : QThread(parent)
{
    //线程结束后，释放对象
    connect(this, &PlayThread::finished, this, &PlayThread::deleteLater);
}

char * bufferData;
int bufferLen;

PlayThread::~PlayThread(){
    disconnect();
    requestInterruption();
    quit();
    wait();
    qDebug() << this << "被析构了";
}

void PlayThread::run()
{
    int result = SDL_Init(SDL_INIT_AUDIO);
    if(result < 0){
        qDebug() << "sdl_init failed";
        return;
    }

    SDL_AudioSpec spec;
    spec.freq = SAMPLE_RATE;

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

    char data[BUFFER_SIZE];
    while (!isInterruptionRequested()) {
        if (bufferLen > 0) { //sdl没有读取完
           msleep(10);
           continue;
        }

        bufferLen = file.read(data, BUFFER_SIZE);

        if (bufferLen <= 0) {
           break; 
        }

        bufferData = data;
    }


    file.close();


    SDL_Quit();
}
