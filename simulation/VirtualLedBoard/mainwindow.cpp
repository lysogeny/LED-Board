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

    this->mOffscreenPanel = new QImage(DEFAULT_WIDTH + LED_DISTANCE, DEFAULT_HEIGHT + LED_DISTANCE, QImage::Format_RGB32);
    for(int x=0; x < (PANEL_WIDTH * MAXIMUM_PANELSIZE); x++) {
        for(int y=0; y < PANEL_HEIGHT; y++) {
            setLED(x, y);
        }
    }
    updatePanel();
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


void MainWindow::setLED(uint8_t x, uint8_t y) {
    /* draw inital screen */
    QPainter painter(this->mOffscreenPanel);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(COLOR_FOREGROUND, 1));
    painter.setBrush(COLOR_FOREGROUND);
    painter.drawEllipse(LED_DIAMETER/2 + (x* LED_DISTANCE), LED_DIAMETER/2 + (y * LED_DISTANCE), LED_DIAMETER, LED_DIAMETER);
}

void MainWindow::updatePanel(void) {
    this->drawImage(this->mOffscreenPanel);
    this->renderPanel();
}

void MainWindow::renderPanel(void) {
    this->mOffscreenPanel->fill(COLOR_BACKGROUND);
}
