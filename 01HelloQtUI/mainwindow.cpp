#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_loginBtn_clicked()
{
    qDebug() << "点击登录按钮";
}

void MainWindow::on_registerBtn_clicked()
{
    qDebug() << "点击注册按钮";
}
