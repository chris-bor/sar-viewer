#ifndef STRIPSCAN_H
#define STRIPSCAN_H

#include "qcustomplot.h"
#include "Config.h"

namespace SarViewer {

    class StripScan : public QCPCurve {
        Q_OBJECT
    public :
        StripScan(QCPAxis *keyAxis, QCPAxis *valueAxis);
        virtual ~StripScan();
        void getBounds(StripScanBndType &bnd);

    protected :
        virtual void draw(QCPPainter *painter);
        void getData(QVector<QPointF> *near, QVector<QPointF> *far) const;

    private :
        uint nrPoinDash_;
        QRectF imageBnd_;

        StripScanBndType bnd_;
    };
} // namespace SarViewer
#endif // STRIPSCAN_H
