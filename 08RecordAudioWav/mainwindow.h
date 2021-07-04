#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "audiothread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_recordAudioBtn_clicked();
    void on_time_changed(qint64 ms);

private:
    Ui::MainWindow *ui;
    AudioThread *audioThread = nullptr;
};
#endif // MAINWINDOW_H
