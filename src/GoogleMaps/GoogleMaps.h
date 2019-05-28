/// @file GoogleMaps.h
/// @brief class providing google maps
/// @author Krzysztof Borowiec

#ifndef GOOGLEMAPS_H
#define GOOGLEMAPS_H

#include <QtCore>
#include <QtGui>
#include <QtWebKit>
//#include "../qcustomplot.h"

class QCPRange;
class QCustomPlot;
//namespace SarViewer {
//    class GoogleMapsItem;
//}

#define MAP_HTML \
    "<html>" \
    "<head>" \
    "<script type=\"text/javascript\" " \
    "src=\"http://maps.google.com/maps/api/js?sensor=false\">" \
    "</script>" \
    "<script type=\"text/javascript\">" \
    "var map; " \
    "var mapOptions; " \
    "function init(lat, lng) { "\
    "mapOptions = {" \
    "center: new google.maps.LatLng(lat, lng), " \
    " zoom: 13," \
    "disableDefaultUI: false," \
    "disableDoubleClickZoom: true," \
    "mapTypeId: google.maps.MapTypeId.ROADMAP " \
    "};" \
    "map = new google.maps.Map(document.getElementById(\"map_canvas\"), mapOptions );" \
    "} </script>" \
    "</head>" \
    "<body style=\"margin:0px; padding:0px;\">" \
    "<div id=\"map_canvas\" style=\"width:100%; height:100%\"/>" \
    "</body>" \
    "</html>"

//key=AIzaSyB6yWKP__zJPY1ep35FEtO-fPDoCfdEpLk&

namespace SarViewer {
    class GoogleMaps: public QWebView
    {
        Q_OBJECT

    public :
        GoogleMaps(QCustomPlot *parent = 0);
        virtual ~GoogleMaps();
        void setCenter(qreal latitude, qreal longitude);
        QCustomPlot *parent() const { return parent_; }

    public slots :
        void rangeChanged(const QCPRange &newRange);

    signals :
        void updateMapViewer();

    private slots :
        void triggerLoading();

    protected :
        void timerEvent(QTimerEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void wheelEvent(QWheelEvent *event);
        void paintEvent(QPaintEvent *event);
        void resizeEvent (QResizeEvent *event);
        void readContentHtml();

    private :
        QCustomPlot *parent_;
        QImage googleMapsImg_;
        QPoint pressPos_;
        QPoint dragPos_;
        QString content_;
        bool replotAfterEvent_;
        QRect geometry_;
    };

} // namespace SarViewer

#endif // GOOGLEMAPS_H
