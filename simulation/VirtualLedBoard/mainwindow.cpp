#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->server = new UdpLedServer ();

    this->mOffscreenPanel = new QImage(DEFAULT_WIDTH, DEFAULT_HEIGHT, QImage::Format_RGB32);
    this->mOffscreenPanel->fill(COLOR_BACKGROUND);

    /* draw inital screen */
    QPainter painter(this->mOffscreenPanel);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(COLOR_FOREGROUND, 1));
    painter.setBrush(COLOR_FOREGROUND);
    for(int x=0; x < (PANEL_WIDTH * MAXIMUM_PANELSIZE); x++) {
        for(int y=0; y < PANEL_HEIGHT; y++) {
            painter.drawEllipse(LED_DIAMETER/2 + (x* LED_DISTANCE), LED_DIAMETER/2 + (y * LED_DISTANCE), LED_DIAMETER, LED_DIAMETER);
        }
    }

    this->drawImage(this->mOffscreenPanel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::drawImage(QImage *target) {
    QGraphicsView *graphicsView = new QGraphicsView();
    graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QGraphicsScene* scene=new QGraphicsScene() ;
    graphicsView->setScene(scene);
    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(*(target)));
    scene->addItem(item);
    graphicsView->show();
    this->ui->ledPanel->addWidget(graphicsView);
}

