#include "mypushbutton.h"
#include <QDebug>

MyPushButton::~MyPushButton()
{
    qDebug() << "MyPushButton::~MyPushButton()";
}

MyPushButton::MyPushButton(QWidget *parent):QPushButton(parent)
{
}
