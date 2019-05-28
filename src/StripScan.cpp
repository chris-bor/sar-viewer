#include <QtGui>
#include "StripScan.h"
#include "Debugging.h"
#include "Config.h"
#include "MapViewer.h"

extern ViewerConfig viewer_config;

SarViewer::StripScan::StripScan(QCPAxis *keyAxis, QCPAxis *valueAxis) :
    QCPCurve(keyAxis, valueAxis), nrPoinDash_(0) {
#if DEBUG_CONST
    qDebug() << "CONSTRUCTOR OF StripScan";
#endif
    nrPoinDash_ = viewer_config.nr_lines_per_screen / viewer_config.nr_lin_to_ref_str_scan;
}

SarViewer::StripScan::~StripScan() {
#if DEBUG_CONST
    qDebug() << "DESTRUCTOR OF StripScan";
#endif
}


void SarViewer::StripScan::draw(QCPPainter *painter) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";

    if (mData->isEmpty()) {
        qDebug() << "StripScn mData is empty";
        return;
    }

    // Allocate vector for data:
    QVector<QPointF> *pointsN = new QVector<QPointF>; // Near to plane points
    QVector<QPointF> *pointsF = new QVector<QPointF>; // Far from plane points

    // Fill with data and names
    getData(pointsN, pointsF);

    uint size = pointsN->size(); // Both have the same size

    // Draw StripScan
    applyDefaultAntialiasingHint(painter);
    QPen pen = mainPen();
    pen.setColor(Qt::green);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    uint i = 0;

    // Front line
    painter->drawLine(pointsF->at(0), pointsN->at(0));
    if(nrPoinDash_<size) { // With history
        // Side line
        for(i = 0; i < nrPoinDash_-1; i++) { // {-1 because draw from i to i+1}
            painter->drawLine(pointsN->at(i), pointsN->at(i+1));
            painter->drawLine(pointsF->at(i), pointsF->at(i+1));
        }
        // Detached line
        painter->drawLine(pointsF->at(nrPoinDash_-1), pointsN->at(nrPoinDash_-1));

        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);

        for(i = nrPoinDash_-1; i < size-1; i++) {
            painter->drawLine(pointsN->at(i), pointsN->at(i+1));
            painter->drawLine(pointsF->at(i), pointsF->at(i+1));
        }
    }
    else { // No history
        // Side line
        for(i = 0; i < size-1; i++) {
            painter->drawLine(pointsN->at(i), pointsN->at(i+1));
            painter->drawLine(pointsF->at(i), pointsF->at(i+1));
        }
    }
    // End line
    painter->drawLine(pointsF->at(size-1), pointsN->at(size-1));

//    painter->drawEllipse(pointsN->at(1),5,5);
//    painter->drawText(pointsF->at(0),"F0");
//    painter->drawText(pointsN->at(0),"N0");
//    painter->drawText(pointsF->at(1),"F1");
//    painter->drawText(pointsN->at(1),"N1");

//    painter->drawEllipse(coordsToPixels(bnd_.x_min, bnd_.y_min),5,5);
//    painter->drawEllipse(coordsToPixels(bnd_.x_min, bnd_.y_max),7,7);
//    painter->drawEllipse(bnd_.x_max, bnd_.y_min,3,3);
//    painter->drawEllipse(bnd_.x_max, bnd_.y_max,4,4);
//    qDebug() << "^^^^^^^^" << pointsN->at(1).x() << pointsN->at(1).y();
//    qDebug() << "============" << bnd_.x_min << bnd_.x_max << bnd_.y_min << bnd_.y_max;

    delete pointsN;
    delete pointsF;
}


void SarViewer::StripScan::getData(QVector<QPointF> *near, QVector<QPointF> *far) const {
//    qDebug() << __FUNCTION__ << "()";

    int size = mData->size()/2;
    near->reserve(size);
    far->reserve(size);

    QCPCurveDataMap::const_iterator it = mData->constEnd(); // it points to imaginary element
    it--;

    for (int i = 0; i < size; i++) {
        // Even points = near
        near->append(coordsToPixels(it.value().key, it.value().value)); // add current point to vector
        // Odd points = far
        far->append(coordsToPixels((it-1).value().key, (it-1).value().value)); // add current point to vector
        it-=2;
    }
}

void SarViewer::StripScan::getBounds(StripScanBndType &bnd) {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";

    // If no data
    if(mData->size() < 6) {
        bnd.x_min = 0;
        bnd.y_max = 0;
        bnd.x_max = 5;
        bnd.y_min = 5;

        bnd.x_pix_min = 0;
        bnd.y_pix_max = 0;

        bnd.x_pix_max = 5;
        bnd.y_pix_min = 5;

        // Calculate width and height
        bnd.width_pix = bnd.x_pix_max - bnd.x_pix_min;
        bnd.height_pix = bnd.y_pix_min - bnd.y_pix_max;

        return;
    }

    QCPCurveDataMap::const_iterator it = mData->constEnd()-1; // it points to imaginary element

//    float minX = it.value().key, minY = it.value().value, maxX = it.value().key, maxY = it.value().value;
    bnd.x_min = it.value().key; // minX
    bnd.y_min = it.value().value; // minY
    bnd.x_max = it.value().key; // maxX
    bnd.y_max = it.value().value; // maxY

    for(int i = 0; i < 4; i++) {
        if(it.value().key < bnd.x_min)
            bnd.x_min = it.value().key;
        if(it.value().key > bnd.x_max)
            bnd.x_max = it.value().key;
        if(it.value().value < bnd.y_min)
            bnd.y_min = it.value().value;
        if(it.value().value > bnd.y_max)
            bnd.y_max = it.value().value;
        it--;
    }

    // Calculate positions at pixels
    QPoint p = coordsToPixels(bnd.x_min, bnd.y_max).toPoint(); // top left
    bnd.x_pix_min = p.x();
    bnd.y_pix_max = p.y();

    p = coordsToPixels(bnd.x_max, bnd.y_min).toPoint(); // bottom right
    bnd.x_pix_max = p.x();
    bnd.y_pix_min = p.y();

    // Calculate width and height
    bnd.width_pix = qMin(qMax((int)(bnd.x_pix_max - bnd.x_pix_min), 5), 1920);
    bnd.height_pix = qMin(qMax((int)(bnd.y_pix_min - bnd.y_pix_max), 5), 1080);

    bnd_ = bnd;
}
