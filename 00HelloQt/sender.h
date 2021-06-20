#ifndef SENDER_H
#define SENDER_H

#include <QObject>

class Sender : public QObject
{
    Q_OBJECT
public:
    explicit Sender(QObject *parent = nullptr);

signals:
    //只需要函数声明，不需要实现
    int exit(int a, int b);
};

#endif // SENDER_H
