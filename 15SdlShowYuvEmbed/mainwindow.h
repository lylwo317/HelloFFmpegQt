#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "playthread.h"

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

    void on_displayBtn_clicked();

private:
    QWidget *_widget;
    Ui::MainWindow *ui;
    PlayThread *playThread = nullptr;
};
#endif // MAINWINDOW_H