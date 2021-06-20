QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mypushbutton.cpp \
    receiver.cpp \
    sender.cpp

HEADERS += \
    mainwindow.h \
    mypushbutton.h \
    receiver.h \
    sender.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

macx {
# mac only
message("mac mode")
INCLUDEPATH += /usr/local/Cellar/ffmpeg/4.3.2_4/include
LIBS += -L /usr/local/Cellar/ffmpeg/4.3.2_4/lib
}

unix:!macx{
# linux only
message("linux mode")
}

win32 {
message("win32 mode")
# windows only
}

//FFmpeg相关的库
LIBS += -lavformat \
        -lavcodec \
        -lavdevice \
        -lavfilter \
        -lavutil \
        -lpostproc \
        -lswscale \
        -lswresample

# message() 打印
# $${} 可以用来取值。.pro中的变量中的值
# $$() 可以用来取值。取系统环境变量中的值
