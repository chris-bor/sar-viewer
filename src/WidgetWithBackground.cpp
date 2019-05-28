#include <QtGui>
#include "WidgetWithBackground.h"

SarViewer::WidgetWithBackground::WidgetWithBackground(QWidget *parent) : QWidget(parent) {
    pixmap_ = new QPixmap(size());
    modified_ = false;
}

SarViewer::WidgetWithBackground::~WidgetWithBackground() {
    if(pixmap_) {
        delete pixmap_;
        pixmap_ = NULL;
    }
}

void SarViewer::WidgetWithBackground::drawBackground() {
    if(modified_ || pixmap_->size()!= size()) {
        delete pixmap_;
        pixmap_ = new QPixmap(size());
        modified_ = true;
        repaintBackground();
        modified_ = false;
    }

    QPainter painter(this);
    painter.drawPixmap(0, 0, *pixmap_);
}

void SarViewer::WidgetWithBackground::updateWithBackground() {
    modified_ = true;
    update();
}

void SarViewer::WidgetWithBackground::repaintBackground() {
    pixmap_->fill(QColor(0, 0, 0, 0));
    QPainter painter(pixmap_);
    paintBackground(painter);
}
