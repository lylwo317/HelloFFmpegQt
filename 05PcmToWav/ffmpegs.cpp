#include "ffmpegs.h"
#include <QFile>
#include <QDebug>

FFmpegs::FFmpegs(){

}

void FFmpegs::pcm2wav(WAVHeader &header, const char *pcmFileName, const char *wavFileName){
    header.blockAlign = header.bitsPerSample * header.numChannels >> 3;
    header.byteRate = header.sampleRate * header.blockAlign;

    QFile pcmFile(pcmFileName);
    if (!pcmFile.open(QFile::ReadOnly)) {
        qDebug() << "文件打开失败" << pcmFileName;
        return;
    }

    header.dataSubchunkSize = pcmFile.size();
    header.chunkSize = header.dataSubchunkSize
        + sizeof(WAVHeader)
        - sizeof(header.chunkId)
        - sizeof(header.chunkSize);

    QFile wavFile(wavFileName);
    if (!wavFile.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << wavFileName;
        pcmFile.close();
        return;
    }

    wavFile.write((const char *)&header, sizeof(WAVHeader));

    //写入头部
    char buf[1024];
    int size;//实际写入的size
    //循环写入
    while ((size = pcmFile.read(buf,sizeof(buf))) > 0) {
        wavFile.write(buf, size);
    }

    //关闭文件
    pcmFile.close();
    wavFile.close();
}
