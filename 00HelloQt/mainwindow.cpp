#include "mainwindow.h"
#include <QPushButton>
#include <QDebug>
#include "mypushbutton.h"
#include "receiver.h"
#include "sender.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    qDebug() << "MainWindow Create";

    setWindowTitle("主窗口");

    setFixedSize(600,600);

    //使用默认提供的信号与槽
    QPushButton *btn = new QPushButton();//QObject不需要主动回收
    btn->setText("关闭");
    btn->setFixedSize(100,30);
    btn->move(100, 200);
    btn->setParent(this);//设置了父对象,所以不需要自己管理回收对象
    //connect(信号发送者，信号，信号接收者，槽函数);
            //sender  //signal                          //slot
    connect(btn, &QAbstractButton::clicked , this, &MainWindow::close);


    //使用自定义的信号与槽
    Sender* sender = new Sender;
    Receiver* receiver = new Receiver;
    QPushButton *btn2 = new MyPushButton(this);
    btn2->setText("发送给Receiver");
    connect(sender, &Sender::exit, receiver, &Receiver::handleExit);

    qDebug() << emit sender->exit(13,15);//发送信号，并打印槽函数返回值

    //没有设置父对象，需要自己管理对象的回收
    delete sender;
    delete receiver;

    //整体看下来，信号与槽类似于rpc远程调用，只是这里是进程内的调用
}

MainWindow::~MainWindow()
{
}

