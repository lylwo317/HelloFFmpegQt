#ifndef RECEIVER_H
#define RECEIVER_H

#include <QObject>

class Receiver : public QObject
{
    Q_OBJECT
public:
    explicit Receiver(QObject *parent = nullptr);

public slots:
    //槽函数必须实现
    int handleExit(int a, int b);
};

#endif // RECEIVER_H
