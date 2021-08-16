#include "playthread.h"
#include "SDL_pixels.h"
#include "SDL_render.h"

#include <QDebug>

#include <SDL2/SDL.h>
extern "C" {
//引用C语言的库，必须要阻止name mangling
}
#include <QFile>

#define END(judge, func)\
    if(judge){ \
        qDebug() << #func << "error"  << SDL_GetError();\
        goto end; \
    }

static SDL_Texture * createTexture(SDL_Renderer *renderer){
    SDL_Texture *texture = SDL_CreateTexture(renderer, 
            SDL_PIXELFORMAT_ARGB32, 
            SDL_TEXTUREACCESS_TARGET, //可以作为渲染目标
            50, 50);
    if (!texture) {
        return nullptr;
    }

    //设置纹理作为渲染目标
    if (SDL_SetRenderTarget(renderer, texture)) {
        return nullptr;
    }

    //设置画笔颜色(黄色)
    if (SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE)) {
        return nullptr;
    }

    //绘制图形

    SDL_Rect rect = {0,0,50,50};
    if(SDL_RenderDrawRect(renderer, &rect)) return nullptr;
    if(SDL_RenderDrawLine(renderer, 0, 0, 50, 50)) return nullptr;
    if(SDL_RenderDrawLine(renderer, 50, 0, 0, 50)) return nullptr;

    //重新设置渲染目标为window
    SDL_SetRenderTarget(renderer, nullptr);
    
    return texture;
}

static void showClick(int btnX, int btnY, SDL_Renderer *renderer, SDL_Texture *texture){

    //获取纹理的宽高
    int w = 0;
    int h = 0;
    if(SDL_QueryTexture(texture, nullptr, nullptr, &w, &h)) return;

    //计算绘制的坐标点（使得绘制后的纹理中心在鼠标点击的位置）
    int x = btnX - (w>>1);
    int y = btnY - (h>>1);

    SDL_Rect destRest = {x,y,w,h};

    //清屏
    if(SDL_RenderClear(renderer))return;


    //拷贝纹理数据到渲染目标（默认是window）
    if(SDL_RenderCopy(renderer, texture, nullptr, &destRest)) return;


    //更新上面所有的渲染操作到屏幕上
    SDL_RenderPresent(renderer);
}

PlayThread::PlayThread(void *winId, QObject *parent) : QThread(parent) {
    //线程结束后，自动销毁堆中的对象
    _winId = winId;
    connect(this, &PlayThread::finished, this, &PlayThread::deleteLater);
}

PlayThread::~PlayThread() {
    requestInterruption();  //中断线程
    quit();
    wait();  //等待线程结束后，再析构
    qDebug() << "析构 PlayThread";
}

#define IMG_W 512
#define IMG_H 512
#define FILE_NAME "/home/kevin/QtProjects/HelloFFmpegQt/yuv420p.yuv"

void PlayThread::run() {
    //窗口
    SDL_Window *window = nullptr;

    //渲染上下文
    SDL_Renderer *renderer = nullptr;

    //纹理(跟特定驱动程序相关的像素数据，驱动可以用来直接显示到屏幕)
    SDL_Texture *texture = nullptr;

    QFile file(FILE_NAME);

    //初始化子系统
    END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    //创建窗口
//    window = SDL_CreateWindow("SDL显示YUV图片",
//            SDL_WINDOWPOS_UNDEFINED,
//            SDL_WINDOWPOS_UNDEFINED,
//            IMG_W, IMG_H,
//            SDL_WINDOW_SHOWN);
    window = SDL_CreateWindowFrom(_winId);

    END(!window, SDL_CreateWindow);

    //创建渲染上下文
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if(!renderer){
        //如果不支持硬件加速，就创建普通的
        renderer = SDL_CreateRenderer(window, -1, 0);
        END(!renderer, SDL_CreateRenderer);
    }

    //创建纹理
    texture = SDL_CreateTexture(
            renderer, 
            SDL_PIXELFORMAT_IYUV, 
            SDL_TEXTUREACCESS_STREAMING, 
            IMG_W, IMG_H);
    END(!texture, SDL_CreateTextureFromSurface);

    //将YUV数据填充到纹理上
    if(!file.open(QFile::ReadOnly)){
        qDebug() << "file open error";
        goto end;
    }

    END(SDL_UpdateTexture(texture, nullptr, file.readAll().data(), IMG_W),
            SDL_UpdateTexture);

    END(SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);

    END(SDL_RenderClear(renderer), SDL_RenderClear);

    END(SDL_RenderCopy(renderer, texture, nullptr, nullptr), SDL_RenderCopy);

    SDL_RenderPresent(renderer);
    

    while (!isInterruptionRequested()) {
        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
            case SDL_QUIT:
                goto end;
                break;
        }
    }

end:
    file.close();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
