#include "GoogleMapsItem.h"
#include "../Debugging.h"

using namespace SarViewer;

SarViewer::GoogleMapsItem::GoogleMapsItem(QCustomPlot *parent_plot) :
    QCPAbstractItem(parent_plot),
    position_(createPosition("GoogleMapsItemPosition")),
    parent_(parent_plot) {
#if DEBUG_CONST
    qDebug() << "CONSTRUCTOR OF StripScanImage";
#endif
    position_->setCoords(0, 0);
    position_->setType(QCPItemPosition::ptPlotCoords);
    position_->setAxes(parent_plot->xAxis, parent_plot->yAxis);

    googleMaps_ = new GoogleMaps(parent_plot);
//    googleMaps_->setHidden(true);
//    googleMaps_->setVisible(false);
    googleMaps_->setUpdatesEnabled(false); // Cause that QWeb disapperas!!!
//    googleMaps_->setUpdatesEnabled(true);

//    QPalette palette = googleMaps_->palette();
//    palette.setBrush(QPalette::Base, Qt::transparent);
//    googleMaps_->page()->setPalette(palette);
//    googleMaps_->setAttribute(Qt::WA_OpaquePaintEvent, false);


//    QObject::connect(googleMaps_, SIGNAL(drawGoogleMaps(QImage)),
//                     this, SLOT(drawGoogleMaps(QImage)));

//    if(!setLayer("googlemaps")) {
//        qDebug() << "Error during setting \"googlemaps\" layer";
//    }
//    qDebug() << "setLayer"<< setLayer("googlemaps");
    qDebug() <<"setLayer"<< setLayer("image") << googleMaps_->size();
}

SarViewer::GoogleMapsItem::~GoogleMapsItem() {
#if DEBUG_DESTR
    qDebug() << "DESTRUCTOR OF StripScanImage";
#endif
}


double SarViewer::GoogleMapsItem::selectTest(const QPointF &pos) const {
    return 0;
}


void SarViewer::GoogleMapsItem::draw(QCPPainter *painter) {
//#if DEBUG_DESTR
    qDebug() << __FILE__ << __FUNCTION__ << "()";
//#endif
//    painter->setPen(mainPen());
//    painter->setBrush(mainBrush());
//    QPointF center(position->pixelPoint());
//    QRect clip = clipRect();
//    double w = mSize/2.0;

//    if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)).toRect())) {

//        painter->save();

//        painter->translate(center.x(), center.y());
//        painter->rotate(headerVect_.last().head);

//        painter->drawPath(path_);
//        painter->restore();
    if(parent_->isVisible()) {
        googleMapsImg_ = QPixmap::grabWidget(this->googleMaps()).toImage();
        QPoint p = parent_->xAxis->axisRect().topLeft();
//        p = parent_->yAxis->axisRect().topLeft();
//        qDebug() << parent_->axisRect();
//        qDebug() << parent_->xAxis->axisRect() << parent_->yAxis->axisRect()
//                 << parent_->xAxis2->axisRect() << parent_->yAxis2->axisRect();
        painter->drawImage(p.x(), p.y(), googleMapsImg_);
        //painter->drawImage(position_->pixelPoint().toPoint(), googleMapsImg_);
        //painter->drawImage(topLeft->pixelPoint().toPoint(), mImage);
    }
//    }
}
