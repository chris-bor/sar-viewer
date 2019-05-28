#include "../Config.h"
#include "GoogleMaps.h"
#include "../Debugging.h"
#include "../qcustomplot.h"

extern ViewerConfig viewer_config;

SarViewer::GoogleMaps::GoogleMaps(QCustomPlot *parent)
    : QWebView(parent),
      parent_(parent) {
//#if DEBUG_CONST
    qDebug() << "CONSTRUCTOR OF GoogleMaps\n";
//#endif

    readContentHtml();

    QWebFrame *frame = page()->mainFrame();
    frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    //        content.replace("zoom: 15", "zoom: 6");
    frame->setHtml(content_);

    this->setAttribute(Qt::WA_OpaquePaintEvent, false);
//    this->setStyleSheet("background-color:transparent;");

//    this->setAttribute(Qt::WA_TransparentForMouseEvents);

    QTimer::singleShot(1000, this, SLOT(triggerLoading()));
}

SarViewer::GoogleMaps::~GoogleMaps() {
//#if DEBUG_DESTR
    qDebug() << "DESTRUCTOR OF GoogleMaps\n";
//#endif
}

void SarViewer::GoogleMaps::setCenter(qreal latitude, qreal longitude) {
#if DEBUG_GOOGLEMAPS
    qDebug() << __FILE__ << __FUNCTION__ << "()";
#endif
    QString code = "map.setCenter(new google.maps.LatLng(%1, %2))";
    QWebFrame *frame = page()->mainFrame();
    qDebug() << code.arg(latitude).arg(longitude);
    frame->evaluateJavaScript(code.arg(latitude).arg(longitude));
//    update();
    //    emit updateMapViewer();
}


void SarViewer::GoogleMaps::rangeChanged(const QCPRange &newRange) {
//#if DEBUG_GOOGLEMAPS
    qDebug() << __FILE__ << __FUNCTION__ << "()";
//#endif

    if(parent_->axisRect() != this->geometry()) {
        this->setGeometry(parent_->axisRect());
    }
}


void SarViewer::GoogleMaps::triggerLoading() {
//#if DEBUG_GOOGLEMAPS
    qDebug() << __FILE__ << __FUNCTION__ << "()";
//#endif
    if (parent_->isVisible()){
//    QString code = "init(52.2296756, 21.0122287)"; // Warszawa
    QString code = QString("init(%1, %2)").arg(viewer_config.start_pos_map.lat).arg(viewer_config.start_pos_map.lon);

    QWebFrame *frame = page()->mainFrame();
    frame->evaluateJavaScript(code);

    repaint();
//    qreal lat = frame->evaluateJavaScript("map.get_center().lat()").toDouble();
//    qreal lng = frame->evaluateJavaScript("map.get_center().lng()").toDouble();
//    setCenter(52.2296756, 21.0122287);

    startTimer(1000);
    }
    else {
        QTimer::singleShot(1000, this, SLOT(triggerLoading()));
    }
}


void SarViewer::GoogleMaps::timerEvent(QTimerEvent *event)  { // to nie potrzebne
//#if DEBUG_GOOGLEMAPS
    qDebug() << __FILE__ << __FUNCTION__ << "()";
//#endif
    QWebView::timerEvent(event);

//    QWebFrame *frame = page()->mainFrame();
//    qreal lat = frame->evaluateJavaScript("map.get_center().lat()").toDouble();
//    qreal lng = frame->evaluateJavaScript("map.get_center().lng()").toDouble();
//    setCenter(lat, lng);


    killTimer(event->timerId());

    replotAfterEvent_ = false;
    emit updateMapViewer();
}

void SarViewer::GoogleMaps::mousePressEvent(QMouseEvent *event) {
#if DEBUG_GOOGLEMAPS
    qDebug() << __FILE__ << __FUNCTION__ << "()";
#endif
    pressPos_ = dragPos_ = event->pos();

    QWebView::mousePressEvent(event);

    replotAfterEvent_ = true;

    event->ignore();
}

void SarViewer::GoogleMaps::mouseReleaseEvent(QMouseEvent *event) {
//#if DEBUG_GOOGLEMAPS
    qDebug() << __FILE__ << __FUNCTION__ << "()";
//#endif
    QWebView::mouseReleaseEvent(event);

    replotAfterEvent_ = true;

    event->ignore();
}

void SarViewer::GoogleMaps::mouseMoveEvent(QMouseEvent *event) {
#if DEBUG_GOOGLEMAPS
    qDebug() << __FILE__ << __FUNCTION__ << "()";
#endif
    QWebView::mouseMoveEvent(event);

    dragPos_ = event->pos();

    replotAfterEvent_ = true;

    event->ignore();
}

void SarViewer::GoogleMaps::wheelEvent(QWheelEvent *event) {
#if DEBUG_GOOGLEMAPS
    qDebug() << __FILE__ << __FUNCTION__ << "()";
#endif
    QWebView::wheelEvent(event);

    replotAfterEvent_ = true;

    event->ignore();
}

void SarViewer::GoogleMaps::paintEvent(QPaintEvent *event) {
//#if DEBUG_GOOGLEMAPS
    qDebug() << __FILE__ << __FUNCTION__ << "()";
//#endif
//    QWebView::paintEvent(event);

    QWebFrame *frame = this->page()->mainFrame();
    QPainter p(this);
    p.setRenderHints(this->renderHints());
    frame->render(&p, event->region());


    if(replotAfterEvent_ && parent_->isVisible())
        startTimer(1000);

}

void SarViewer::GoogleMaps::resizeEvent(QResizeEvent *event) {
//#if DEBUG_GOOGLEMAPS
    qDebug() << __FILE__ << __FUNCTION__ << "()";
//#endif
    QWebView::resizeEvent(event);

    replotAfterEvent_ = true;

    event->ignore();
}

void SarViewer::GoogleMaps::readContentHtml() {
#if DEBUG_GOOGLEMAPS
    qDebug() << __FILE__ << __FUNCTION__ << "()";
#endif
//    content_ = MAP_HTML;
    QFile file("GoogleMaps/map.html");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    content_ = in.readAll();
}
