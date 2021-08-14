#include "playthread.h"

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
    //窗口
    SDL_Window *window = nullptr;

    //渲染上下文
    SDL_Renderer *renderer = nullptr;

    //纹理(跟特定驱动程序相关的像素数据，驱动可以用来直接显示到屏幕)
    SDL_Texture *texture = nullptr;

    //初始化子系统
    END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    //创建窗口
    window = SDL_CreateWindow("SDL显示绘制的纹理", 
            SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED,
            300, 300, 
            SDL_WINDOW_SHOWN);

    END(!window, SDL_CreateWindow);

    //创建渲染上下文
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if(!renderer){
        //如果不支持硬件加速，就创建普通的
        renderer = SDL_CreateRenderer(window, -1, 0);
        END(!renderer, SDL_CreateRenderer);
    }

    //创建纹理
    /* texture = SDL_CreateTextureFromSurface(renderer, surface); */
    texture = createTexture(renderer);
    END(!texture, SDL_CreateTextureFromSurface);


    //设置画笔颜色
    END(SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE),//红色
            SDL_SetRenderDrawColor);

    showClick(150,150, renderer, texture);

    while (!isInterruptionRequested()) {
        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
            case SDL_QUIT:
                goto end;
                break;
            case SDL_MOUSEBUTTONUP://鼠标点击后抬起
                SDL_MouseButtonEvent btn = event.button;
                showClick(btn.x, btn.y, renderer, texture);
                break;
        }
    }

end:
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
