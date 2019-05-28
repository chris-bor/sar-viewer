#include "MapTextItem.h"


SarViewer::MapTextItem::MapTextItem(QCPAxis *keyAxis, QCPAxis *valueAxis) :
    QCPCurve(keyAxis, valueAxis) {
#if DEBUG_CONST
    qDebug() << "CONSTRUCTOR OF MapTextItem";
#endif
    mName = new QVector<QString>;
}


SarViewer::MapTextItem::~MapTextItem() {
#if DEBUG_DESTR
    qDebug() << "DESTRUCTOR OF MapTextItem";
#endif
}


void SarViewer::MapTextItem::addName(const QString name_str) {
    mName->append(name_str);
}


void SarViewer::MapTextItem::clearName() {
    mName->clear();
}


void SarViewer::MapTextItem::draw(QCPPainter *painter) {
    if (mData->isEmpty()) return;

    // Allocate line vector:
    QVector<QPointF> *points = new QVector<QPointF>;
    QVector<QString> *names = new QVector<QString>;
    // Fill with data and names
    getDataAndName(points, names);

    // Draw text
    applyDefaultAntialiasingHint(painter);
    painter->setPen(mainPen());
    painter->setBrush(Qt::NoBrush);

    for (int i=0; i<points->size(); i++) {
        painter->drawText(points->at(i), names->at(i));
    }

    delete points;
    delete names;
}


void SarViewer::MapTextItem::getDataAndName(QVector<QPointF> *data, QVector<QString> *names) const {
    data->reserve(mData->size());
    names->reserve(mName->size());
    QCPCurveDataMap::const_iterator it;
    int currentRegion = 5;
    double RLeft = mKeyAxis->range().lower;
    double RRight = mKeyAxis->range().upper;
    double RBottom = mValueAxis->range().lower;
    double RTop = mValueAxis->range().upper;
    double x, y; // current key/value
    uint names_cnt = 0;

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
            names->append(mName->at(names_cnt));
        }
            names_cnt++;
    }
}
