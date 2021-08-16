#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H

#include <QThread>

class PlayThread : public QThread
{
    Q_OBJECT

private:
    void run() override;

public:
    PlayThread(QObject *parent = nullptr);

    ~PlayThread();
};

#endif // AUDIOTHREAD_H
