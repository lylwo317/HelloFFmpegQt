cmake_minimum_required(VERSION 3.5)

project(04PlayPCM LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_LIST_DIR}
)

set(DEBUG_SDL FALSE)
set(DEBUG_GLIBC FALSE)
set(DEBUG_FFMPEG FALSE)

#set(SDL2_INCLUDE_DIR "/home/kevin/CLionProjects/SDL/out/include/SDL")
IF (WIN32)
        MESSAGE(STATUS "Now is windows")
        #set(CMAKE_CXX_FLAGS_RELEASE "-Od ${CMAKE_CXX_FLAGS}")
        #STRING(REPLACE "-O2" "-Od" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
ELSEIF (APPLE)
        MESSAGE(STATUS "Now is OSX systems.")
        set(FFMPEG_HOME "/usr/local/Cellar/ffmpeg/4.4_2")
        include_directories(${FFMPEG_HOME}/include)
        link_directories(${FFMPEG_HOME}/lib)
        find_package(SDL2 REQUIRED)
        include_directories(${SDL2_INCLUDE_DIRS}/../)
        set(CMAKE_PREFIX_PATH "/Users/kevinxie/Qt/6.1.1/clang_64/lib/cmake")
ELSEIF (UNIX)
        MESSAGE(STATUS "Now is UNIX-like OS's. Including Linux  and CygWin")
        if (DEBUG_GLIBC)
        #添加debug版glibc链接路径
                set(GLIBC_LIB_DIR /home/kevin/glibc-2.31/out/lib)
                link_directories("${GLIBC_LIB_DIR}")
        endif ()

        if (DEBUG_SDL)
                set(SDL_INCLUDE_DIR /home/kevin/CLionProjects/SDL/out/include)
                set(SDL_LIB_DIR /home/kevin/CLionProjects/SDL/out/lib)
                include_directories("${SDL_INCLUDE_DIR}/SDL" "${SDL_INCLUDE_DIR}")
                link_directories("${SDL_LIB_DIR}")
                set(SDL2_LIBRARIES SDL2d)
        else()
                find_package(SDL2 REQUIRED)
                include_directories(${SDL2_INCLUDE_DIRS})
        endif ()

        if (DEBUG_FFMEPG)
            include_directories("/home/kevin/CLionProjects/FFmpeg/out/include")
            link_directories("/home/kevin/CLionProjects/FFmpeg/out/lib")
        else()
        endif()

        link_libraries(c)

ENDIF ()

MESSAGE(STATUS "SDL2_INCLUDE_DIRS = " ${SDL2_INCLUDE_DIRS})

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        playthread.cpp
        playthread.h
         mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(04PlayPCM
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(04PlayPCM SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(04PlayPCM
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(04PlayPCM PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    avcodec
    avdevice
    avfilter
    avformat
    avutil
    swresample
    swscale
    ${SDL2_LIBRARIES}
    )
