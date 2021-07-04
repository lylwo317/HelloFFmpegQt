#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H

#include <QThread>

class AudioThread : public QThread
{
    Q_OBJECT

private:
    void run() override;
Q_SIGNALS:
    void onTimeChanged(qint64 ms);

public:
    AudioThread(QObject *parent = nullptr);

    ~AudioThread();
};

#endif // AUDIOTHREAD_H
