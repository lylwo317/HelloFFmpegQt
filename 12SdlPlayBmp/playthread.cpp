#include "playthread.h"

#include <QDebug>

#include "SDL_pixels.h"
#include "SDL_render.h"
#include "ffmpegs.h"
extern "C" {
//引用C语言的库，必须要阻止name mangling
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}
#include <SDL2/SDL.h>
#include <QFile>

#define printAvError(msg, ret)                \
    char errbuf[1024];                        \
    av_strerror(ret, errbuf, sizeof(errbuf)); \
    qDebug() << msg << errbuf;

#define END(judge, func)\
    if(judge){ \
        qDebug() << #func << "error"  << SDL_GetError();\
        goto end; \
    }

#define FILE_NAME "out.yuv"
#define DEVICE_NAME "0"
#define FORMAT_NAME "avfoundation"

PlayThread::PlayThread(QObject *parent) : QThread(parent) {
    //线程结束后，自动销毁堆中的对象
    connect(this, &PlayThread::finished, this, &PlayThread::deleteLater);
}

PlayThread::~PlayThread() {
    requestInterruption();  //中断线程
    quit();
    wait();  //等待线程结束后，再析构
    qDebug() << "析构 PlayThread";
}

void PlayThread::run() {
    //像素数据
    SDL_Surface *surface = nullptr;

    //窗口
    SDL_Window *window = nullptr;

    //渲染上下文
    SDL_Renderer *renderer = nullptr;

    //纹理(跟特定驱动程序相关的像素数据，驱动可以用来直接显示到屏幕)
    SDL_Texture *texture = nullptr;

    //矩形框
    SDL_Rect srcRect = {0,0,512,512};
    SDL_Rect destRect;
    SDL_Rect rect;

    //初始化子系统
    END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    //加载BMP
    surface = SDL_LoadBMP("/home/kevin/Pictures/out.bmp");
    END(!surface, SDL_LoadBMP);

    //创建窗口
    window = SDL_CreateWindow("SDL显示BMP图片", 
            SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED,
            surface->w, //根据BMP宽高设置窗口的宽高
            surface->h, 
            SDL_WINDOW_SHOWN);

    END(!window, SDL_CreateWindow);

    //创建渲染上下文
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if(!renderer){
        //如果不支持硬件加速，就创建普通的
        renderer = SDL_CreateRenderer(window, -1, 0);
        END(!renderer, SDL_CreateRenderer);
    }

    //创建纹理(surface转成纹理)
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    END(!texture, SDL_CreateTextureFromSurface);


    //设置画笔颜色
    END(SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE),
            SDL_SetRenderDrawColor);
    //1. 画矩形框
    rect = {0,0,50,50};
    END(SDL_RenderFillRect(renderer, &rect),SDL_RenderFillRect);

    //设置画笔颜色
    END(SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE),
            SDL_SetRenderDrawColor);
    //使用画笔清屏(涂满全屏)
    //2. 涂黑这块屏幕
    /* END(SDL_RenderClear(renderer), SDL_RenderClear); */
    //执行清屏操作的话，上面的绘制的矩形框就被清屏色覆盖了

    srcRect = {100,100,100,100};
    //           x   y   w   h
    destRect = {200,200,100,100};
    // 拷贝纹理数据到渲染目标（默认是window）
    //3. 将BMP图片中srcRect区域绘制到window的destRect区域
    END(SDL_RenderCopy(renderer, texture, &srcRect, &destRect),
        SDL_RenderCopy);

    // 更新上面所有的渲染操作到屏幕上
    SDL_RenderPresent(renderer);

    SDL_Delay(2000);

end:
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
