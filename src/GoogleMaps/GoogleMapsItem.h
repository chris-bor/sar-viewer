/// @file GoogleMaps.h
/// @brief class providing
/// @author Krzysztof Borowiec

#ifndef GOOGLEMAPSITEM_H
#define GOOGLEMAPSITEM_H

#include "../qcustomplot.h"
#include "GoogleMaps.h"

namespace SarViewer {

    class GoogleMapsItem : public QCPAbstractItem {
        Q_OBJECT
    public :
        GoogleMapsItem(QCustomPlot *parent_plot = 0);
        virtual ~GoogleMapsItem();
        // Non-property methods:
        virtual double selectTest(const QPointF &pos) const;
        GoogleMaps *googleMaps() const { return googleMaps_; }
        QCustomPlot *parent() const { return parent_; }

        QCPItemPosition * const position_;

    public slots :
        //void drawGoogleMaps(QImage image);

    protected :
        virtual void draw(QCPPainter *painter);

        QCustomPlot *parent_;
        GoogleMaps *googleMaps_;
        QImage googleMapsImg_;
    };

}

#endif // GOOGLEMAPSITEM_H
