#include <QtGui>
#include "Led.h"

SarViewer::Led::Led(QWidget *parent) {
    checked_ = false;
    color_ = Qt::red;
    resize(SarViewer::LED_SIZE.width(), SarViewer::LED_SIZE.height());
    setMinimumSize(20,20);
}


SarViewer::Led::~Led() {
}


void SarViewer::Led::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    initCoord(painter);
    // Draw cicrcle
    QColor c, back = getColor();
    c = back;

    // Led's color
    if(!checked_) { // off
        back = Qt::red;
    }
    else // on
        back = Qt::green;
    painter.setBrush(back);

    // Led's border
    QPen pen;

    pen.setColor(c);
    pen.setWidthF(1.0);

    painter.drawEllipse(-9, -9, 9, 9);
    painter.end();

    // Draw led's shine
    drawBackground();
}

void SarViewer::Led::setColor(QColor val) {
    color_ = val;
    updateWithBackground();
}

QSize SarViewer::Led::minimumSizeHint() const {
    return(QSize(SarViewer::LED_SIZE.width(), SarViewer::LED_SIZE.height()));
}

QSize SarViewer::Led::sizeHint() const {
    return(QSize(SarViewer::LED_SIZE.width(), SarViewer::LED_SIZE.height()));
}


void SarViewer::Led::setChecked(bool val)  {
    checked_ = val;
    updateWithBackground();
    checkChanged(checked_);
}

void SarViewer::Led::initCoord(QPainter &painter) {
    int side = qMin(width(), height());

    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 20.0, side / 20.0);
}

void SarViewer::Led::paintBackground(QPainter &painter) {
    initCoord(painter);
    painter.setPen(Qt::NoPen);
    QRadialGradient shine(QPointF(0.0, 0.0), 120.0, QPointF(0.0, 0.0));
    QColor white1(255, 255, 255, 150);
    QColor white0(255, 255, 255, 0);

    shine.setColorAt(0.0, white1);
    shine.setColorAt(1.0, white0);

    painter.setBrush(shine);
//    painter.drawEllipse(-7, -7, 5, 5);
}
