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

    this->mOffscreenDiagram = new QImage(DEFAULT_WIDTH, DEFAULT_HEIGHT, QImage::Format_RGB32);
    this->mOffscreenDiagram->fill(COLOR_BACKGROUND);

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

