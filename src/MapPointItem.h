#ifndef MAPPOINTITEM_H
#define MAPPOINTITEM_H

#include "qcustomplot.h"

const uint CIRCLE_RADIUS = 3; // Radius of the point

namespace SarViewer {

    class MapPointItem : public QCPCurve {
        Q_OBJECT

    public :
        MapPointItem(QCPAxis *keyAxis, QCPAxis *valueAxis);
        virtual ~MapPointItem();

    protected :
        virtual void draw(QCPPainter *painter);
        void getData(QVector<QPointF> *data) const;
    };
} // namespace SarViewer


#endif // MAPPOINTITEM_H
