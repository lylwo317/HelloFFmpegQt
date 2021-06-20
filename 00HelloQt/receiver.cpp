#include "receiver.h"
#include <QDebug>

Receiver::Receiver(QObject *parent) : QObject(parent)
{

}

int Receiver::handleExit(int a, int b)
{
    qDebug() << "handle exit";
    return a + b;
}
