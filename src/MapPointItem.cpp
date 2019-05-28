#include <QtGui>
#include "MapPointItem.h"

SarViewer::MapPointItem::MapPointItem(QCPAxis *keyAxis, QCPAxis *valueAxis) :
    QCPCurve(keyAxis, valueAxis) {
#if DEBUG_CONST
    qDebug() << "CONSTRUCTOR OF MapPointItem";
#endif
}


SarViewer::MapPointItem::~MapPointItem() {
#if DEBUG_DESTR
    qDebug() << "DESTRUCTOR OF MapPointItem";
#endif
}


void SarViewer::MapPointItem::draw(QCPPainter *painter) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";

    if (mData->isEmpty()) return;

    // Allocate vector for data:
    QVector<QPointF> *points = new QVector<QPointF>;

    // Fill with data and names
    getData(points);

    // Draw point
    applyDefaultAntialiasingHint(painter);
    painter->setPen(mainPen());
    painter->setBrush(Qt::NoBrush);
//    QBrush brush = painter->brush();
//    brush.setColor(mainPen().color());
//    brush.setStyle(Qt::SolidPattern);
//    painter->setBrush(brush);

    for (int i=0; i<points->size(); i++) {
        painter->drawEllipse(points->at(i), CIRCLE_RADIUS, CIRCLE_RADIUS);
    }

    delete points;
}


void SarViewer::MapPointItem::getData(QVector<QPointF> *data) const {
    data->reserve(mData->size());
    QCPCurveDataMap::const_iterator it;
    int currentRegion = 5;
    double RLeft = mKeyAxis->range().lower;
    double RRight = mKeyAxis->range().upper;
    double RBottom = mValueAxis->range().lower;
    double RTop = mValueAxis->range().upper;
    double x, y; // current key/value

    for (it = mData->constBegin(); it != mData->constEnd(); ++it)
    {
        x = it.value().key;
        y = it.value().value;
        // determine current region:
        if (x < RLeft) // region 123
        {
            if (y > RTop)
                currentRegion = 1;
            else if (y < RBottom)
                currentRegion = 3;
            else
                currentRegion = 2;
        } else if (x > RRight) // region 789
        {
            if (y > RTop)
                currentRegion = 7;
            else if (y < RBottom)
                currentRegion = 9;
            else
                currentRegion = 8;
        } else // region 456
        {
            if (y > RTop)
                currentRegion = 4;
            else if (y < RBottom)
                currentRegion = 6;
            else
                currentRegion = 5;
        }

        if( currentRegion == 5 ) {
            data->append(coordsToPixels(it.value().key, it.value().value)); // add current point to vector
        }
    }
}
