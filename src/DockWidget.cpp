#include "DockWidget.h"
#include "Debugging.h"

SarViewer::DockWidget::DockWidget(const QString & title, QWidget *parent, Qt::WindowFlags flags) :
    QDockWidget(title, parent, flags){
    QObject::connect(this, SIGNAL(topLevelChanged(bool)), this, SLOT(floatingChanged(bool)));
}


SarViewer::DockWidget::~DockWidget() {
}


void SarViewer::DockWidget::resizeEvent(QResizeEvent *event) {
//    qDebug() << __FUNCTION__ << "()";
    QDockWidget::resizeEvent(event);
    if(!isFloating()) { // For docked - no floating
        emit resized(QSize(this->geometry().width() - 8,
                           this->geometry().height() - 28));
    }
    else {// For undocked - floating
        emit resized(QSize(this->geometry().width() - 2,
                           this->geometry().height() - 2));
    }
}


void SarViewer::DockWidget::floatingChanged(bool change) {
//    qDebug() << __FUNCTION__ << "()";
    if(!isFloating()) { // For docked - no floating
        emit resized(QSize(this->geometry().width() - 8,
                           this->geometry().height() - 28));
    }
    else {// For undocked - floating
        emit resized(QSize(this->geometry().width() - 2,
                           this->geometry().height() - 2));
    }
}
