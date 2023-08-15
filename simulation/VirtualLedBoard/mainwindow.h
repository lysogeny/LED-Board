#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include "udpserver.h"

#include "settings.h"

#define COLOR_BACKGROUND    Qt::black
#define COLOR_FOREGROUND    QColor(255, 127, 0, 255)

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    UdpLedServer *server;
    QImage       *mOffscreenPanel;
    void drawImage(QImage *image);
    void renderPanel(void);
};
#endif // MAINWINDOW_H
