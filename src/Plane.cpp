#include <windef.h>
#include "Plane.h"
#include "Config.h"
#include "GeoConv.h"
#include "StripScan.h"
#include "StripScanImage.h"
#include "MapViewer.h"

using namespace SarViewer;

extern ViewerConfig viewer_config;

SarViewer::Plane::Plane(SpecialPoint *start_point, QCustomPlot *parent_plot) :
    QCPAbstractItem(parent_plot),
    position(createPosition("position")),
    mCurve(0),
    startPoint_(start_point),
    parent_(parent_plot) {
#if DEBUG_CONST
    qDebug() << "CONSTRUCTOR OF Plane";
#endif
    position->setCoords(0, 0);

    setBrush(Qt::NoBrush);
    setSelectedBrush(Qt::NoBrush);
    setPen(QPen(PLANE_COLOR));
    setSelectedPen(QPen(Qt::blue, 2));
    setStyle(tsPlane);
    setSize(PLANE_RECT_SIZE);
    setInterpolating(false);
    setCurveKey(0);

    // Create tracCurve_
    trackCurve_ = new QCPCurve(parent_plot->xAxis, parent_plot->yAxis);
    QPen pen = trackCurve_->pen();
    pen.setColor(TRACK_COLOR);
    pen.setWidth(TRACK_WIDTH);
    trackCurve_->setPen(pen);
    trackCurve_->setCurveType(QCPCurve::ctMapCurve);
    trackCurve_->setName(tr("plane's track"));
    parent_plot->addPlottable(trackCurve_);
    setCurve(trackCurve_); // ??? To nie potrzebne

    // Create stripScanBorder_
    stripScanCurve_ = new StripScan(parent_plot->xAxis, parent_plot->yAxis);
    pen = stripScanCurve_->pen();
    pen.setColor(STRIP_SCAN_CURVE_COLOR);
    pen.setWidth(STRIP_SCAN_CURVE_WIDTH);
    stripScanCurve_->setPen(pen);
    stripScanCurve_->setCurveType(QCPCurve::ctMapCurve);
    stripScanCurve_->setName(tr("strip scan border"));
    parent_plot->addPlottable(stripScanCurve_);
//    setCurve(stripScanCurve_);

    // Create path_
    createPath();

    // Create StripScanImage object
    stripScanImage_ = new StripScanImage(start_point, 0, stripScanCurve_, parent_plot);
    parent_plot->addItem(stripScanImage_);
}


SarViewer::Plane::~Plane() {
#if DEBUG_DESTR
    qDebug() << "DESTRUCTOR OF Plane";
#endif
//    if(strScaImg_) {
//        delete strScaImg_;
//        strScaImg_ = NULL;
//    }
}


void SarViewer::Plane::setPen(const QPen &pen) {
    mPen = pen;
}


void SarViewer::Plane::setSelectedPen(const QPen &pen) {
    mSelectedPen = pen;
}


void SarViewer::Plane::setBrush(const QBrush &brush) {
    mBrush = brush;
}


void SarViewer::Plane::setSelectedBrush(const QBrush &brush) {
    mSelectedBrush = brush;
}


void SarViewer::Plane::setSize(double size) {
    mSize = size;
}


void SarViewer::Plane::setStyle(SarViewer::Plane::TracerStyle style) {
    mStyle = style;
}


void SarViewer::Plane::setCurve(QCPCurve *curve) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    if(curve) {
        if (curve->parentPlot() == mParentPlot) {
            position->setType(QCPItemPosition::ptPlotCoords);
            position->setAxes(curve->keyAxis(), curve->valueAxis());
            mCurve = curve;
//            updatePosition();
        } else
            qDebug() << Q_FUNC_INFO << "curve isn't in same QCustomPlot instance as this item";
    } else {
        mCurve = 0;
    }
}


void SarViewer::Plane::setCurveKey(double key) {
    mCurveKey = key;
}


void SarViewer::Plane::setInterpolating(bool enabled) {
    mInterpolating = enabled;
}

void SarViewer::Plane::setHeading(double heading) {
    mHeading = heading;
}


/* inherits documentation from base class */
double SarViewer::Plane::selectTest(const QPointF &pos) const {
    qDebug() << __FILE__ << __FUNCTION__ << "()";
    if (!mVisible || mStyle == tsNone)
        return -1;

    QPointF center(position->pixelPoint());
    double w = mSize/2.0;
    QRect clip = clipRect();

    switch (mStyle) {
    case tsNone: return -1;
    case tsPlus: {
        if (clipRect().intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)).toRect()))
            return qSqrt(qMin(distSqrToLine(center+QPointF(-w, 0), center+QPointF(w, 0), pos),
                              distSqrToLine(center+QPointF(0, -w), center+QPointF(0, w), pos)));
        break;
    }
    case tsCrosshair: {
        return qSqrt(qMin(distSqrToLine(QPointF(clip.left(), center.y()), QPointF(clip.right(), center.y()), pos),
                          distSqrToLine(QPointF(center.x(), clip.top()), QPointF(center.x(), clip.bottom()), pos)));
        break;
    }
    case tsCircle: {
        if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)).toRect())) {
            // distance to border:
            double centerDist = QVector2D(center-pos).length();
            double circleLine = w;
            double result = qAbs(centerDist-circleLine);
            // filled ellipse, allow click inside to count as hit:
            if (result > mParentPlot->selectionTolerance()*0.99 && mBrush.style() != Qt::NoBrush && mBrush.color().alpha() != 0) {
                if (centerDist <= circleLine)
                    result = mParentPlot->selectionTolerance()*0.99;
            }
            return result;
        }
        break;
    }
    case tsSquare: {
        if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)).toRect())) {
            QRectF rect = QRectF(center-QPointF(w, w), center+QPointF(w, w));
            bool filledRect = mBrush.style() != Qt::NoBrush && mBrush.color().alpha() != 0;
            return rectSelectTest(rect, pos, filledRect);
        }
        break;
    }
    case tsPlane: {
        // as tsSquare:
        if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)).toRect())) {
            QRectF rect = QRectF(center-QPointF(w, w), center+QPointF(w, w));
            bool filledRect = mBrush.style() != Qt::NoBrush && mBrush.color().alpha() != 0;
            return rectSelectTest(rect, pos, filledRect);
        }
        break;
    }
    }
    return -1;
}


void SarViewer::Plane::updatePosition() {
    qDebug() << __FILE__ << __FUNCTION__ << Q_FUNC_INFO <<  "() *** to nie powinno byc wywolywane" ;
    if (mCurve)
    {
        if (mParentPlot->hasPlottable(mCurve))
        {
            if (mCurve->data()->size() > 1)
            {
                QCPCurveDataMap::const_iterator first = mCurve->data()->constBegin();
                QCPCurveDataMap::const_iterator last = mCurve->data()->constEnd()-1;
                if (mCurveKey < first.key())
                    position->setCoords(first.key(), first.value().value);
                else if (mCurveKey > last.key())
                    position->setCoords(last.key(), last.value().value);
                else
                {
                    QCPCurveDataMap::const_iterator it = first;
                    it = mCurve->data()->lowerBound(mCurveKey);
                    if (it != first) // mGraphKey is somewhere between iterators
                    {
                        QCPCurveDataMap::const_iterator prevIt = it-1;
                        if (mInterpolating)
                        {
                            // interpolate between iterators around mGraphKey:
                            double slope = (it.value().value-prevIt.value().value)/(it.key()-prevIt.key());
                            position->setCoords(mCurveKey, (mCurveKey-prevIt.key())*slope+prevIt.value().value);
                        } else
                        {
                            // find iterator with key closest to mGraphKey:
                            if (mCurveKey < (prevIt.key()+it.key())*0.5)
                                it = prevIt;
                            position->setCoords(it.key(), it.value().value);
                        }
                    } else // mGraphKey is exactly on first iterator
                        position->setCoords(it.key(), it.value().value);
                }
                QCPCurveDataMap::const_iterator it = mCurve->data()->constEnd()-1;
            } else if (mCurve->data()->size() == 1)
            {
                QCPCurveDataMap::const_iterator it = mCurve->data()->constBegin();
                position->setCoords(it.key(), it.value().value);
            } else
                qDebug() << Q_FUNC_INFO << "curve has no data";
        } else
            qDebug() << Q_FUNC_INFO << "curve not contained in QCustomPlot instance (anymore)";
    }
}


void SarViewer::Plane::draw(QCPPainter *painter) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
//    updatePosition();

    if (mStyle == tsNone)
        return;

    painter->setPen(mainPen());
    painter->setBrush(mainBrush());
    QPointF center(position->pixelPoint());
    QRect clip = clipRect();
    double w = mSize/2.0;

    if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)).toRect())) {

        painter->save();

        painter->translate(center.x(), center.y());
        painter->rotate(headerVect_.last().head);

        painter->drawPath(path_);
        painter->restore();
    }
}


QPen SarViewer::Plane::mainPen() const {
    return mSelected ? mSelectedPen : mPen;
}


QBrush SarViewer::Plane::mainBrush() const {
    return mSelected ? mSelectedBrush : mBrush;
}


void SarViewer::Plane::addTrackPoint(sar_image_line_header header, int most_actual_line) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    TwoCartCoordType tmpCartCoord;
//    PR_LINE_VAL(header.head);
    if(viewer_config.nr_track_points) {
        if(headerVect_.size() > viewer_config.nr_track_points){
            // Remove data from headerVect_
            headerVect_.pop_front();
            // Remove data from trackCurve_
            trackCurve_->removeFirst();
        }
    }

    // Add header
    headerVect_.append(header); // This operation is relatively fast, because QVector typically allocates more memory than necessary, so it can grow without reallocating the entire vector each time.

    // Add track point to the curve
    GeoConv::mapGeoPos2xy(startPoint_->lat, startPoint_->lon,
                            header.lat_plat, header.lon_plat,
                            &tmpCartCoord.y, &tmpCartCoord.x );
    header.alt_ground = header.alt_ground - H_REFF;
    trackCurve_->addData(tmpCartCoord.x, tmpCartCoord.y);

    position->setCoords(tmpCartCoord.x, tmpCartCoord.y);

    // Add strip scan bounding points
    addStripScanPoint(header, tmpCartCoord);

    // Draw strip scan image
    stripScanImage_->drawStripScanImage(most_actual_line);
//    qDebug() << "pos x" << position->pixelPoint().x() << "pos y" << position->pixelPoint().y() << position->type();

    mParentPlot->replot();
}


void SarViewer::Plane::trackRestart() {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    this->trackCurve_->clearData();
    this->stripScanCurve_->clearData();
    qDebug() << "Reset drawing the track and the strip scan";
    parent_->replot();
}


void SarViewer::Plane::addStripScanPoint(sar_image_line_header &header, TwoCartCoordType &p_c) {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    TwoCartCoordType p1, p2, p11, p22; // p_c - plane coordinate; p1 - point 1; p2 - point2
    double angle = 0.0, alpha;
    float r_max;

    // Calculate angle to radians because header is in degrees
    angle = M_PI * header.head / 180.0;

    // Calculate points depending on ant dir
    if(header.alt_ground < header.r_min) {
        alpha = qAcos(header.alt_ground/header.r_min);
    }
    else {
        alpha = M_PI/2.0;
    }

    r_max = (header.r_min + header.cell_size * header.nr_range_cells); // r max
    if((bool)BitNtst(header.flags, FLAG_BIT_ANT_DIR_RIGHT)) { // ant dir right
        p1.x = r_max*qSin(alpha);
        p1.y = 0.0;
        p2.x = header.r_min*qSin(alpha);
        p2.y = 0.0;
    }
    else { // ant dir left
        p1.x = (-1.0)*r_max*qSin(alpha);
        p1.y = 0.0;
        p2.x = (-1.0)*header.r_min*qSin(alpha);
        p2.y = 0.0;
    }

    // Rotation
    p11.x = p1.x * qCos(angle) + p1.y * qSin(angle);
    p11.y = (-1)*p1.x * qSin(angle) + p1.y * qCos(angle);

    p22.x = p2.x * qCos(angle) + p2.y * qSin(angle);
    p22.y = (-1)*p2.x * qSin(angle) + p2.y * qCos(angle);

    // Calculate points depedning on plane coordinates
    p1.x = p_c.x + p11.x;
    p1.y = p_c.y + p11.y;

    p2.x = p_c.x + p22.x;
    p2.y = p_c.y + p22.y;

    // Check if do not to delete data from mData
    if(viewer_config.nr_ref_str_scan){
        while((uint)stripScanCurve_->data()->size() > (viewer_config.nr_ref_str_scan-2)) {
            stripScanCurve_->removeFirst();
        }
    }

    // Check boundings


    // Add data at end
    stripScanCurve_->addData(p1.x, p1.y);
    stripScanCurve_->addData(p2.x, p2.y);

//    parent_->replot();
}


void SarViewer::Plane::createPath() {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    double w = mSize/2.0;
    double s = 6.0; // side
    double ws = w/s;
    // Dziób
    path_.moveTo(2.0*ws, -1.0*ws);
    path_.cubicTo(2.0*ws, -w, 0, -6.0*ws, 0, -w);
    path_.moveTo(-2.0*ws, -1.0*ws);
    path_.cubicTo(-2.0*ws, -w, 0, -6.0*ws, 0, -w);

    // Lewe skrzydlo
    path_.moveTo(-2.0*ws, -1.0*ws);
    path_.lineTo(-4.0*ws, -1.0*ws);
    path_.cubicTo(-w, -1.0*ws, -w, 0.5*ws, -w, 0.5*ws);
    path_.moveTo(-2.0*ws, 2.0*ws);
    path_.lineTo(-4.0*ws, 2.0*ws);
    path_.cubicTo(-w, 2.0*ws, -w, 0.5*ws, -w, 0.5*ws);

    // Lewy ogon
    path_.moveTo(-2.0*ws, 2.0*ws);
    path_.lineTo(-1.0*ws, 4.0*ws);

    // Lewy statecznik
    path_.cubicTo(-3.0*ws, 4.0*ws, -3.0*ws, 5.0*ws, -3.0*ws, 5.0*ws);
    path_.moveTo(-2.0*ws, w);
    path_.cubicTo(-3.0*ws, w, -3.0*ws, 5.0*ws, -3.0*ws, 5.0*ws);
    path_.moveTo(-2.0*ws, w);
    path_.lineTo(1.0*ws, w);

    // Prawy statecznik
    path_.cubicTo(3.0*ws, w, 3.0*ws, 5.0*ws, 3.0*ws, 5.0*ws);
    path_.moveTo(1.0*ws, 4.0*ws);
    path_.cubicTo(3.0*ws, 4.0*ws, 3.0*ws, 5.0*ws, 3.0*ws, 5.0*ws);

    // Prawy ogon
    path_.moveTo(1.0*ws, 4.0*ws);
    path_.lineTo(2.0*ws, 2.0*ws);

    // Pawe skrzydo
    path_.lineTo(4.0*ws, 2.0*ws);
    path_.cubicTo(w, 2.0*ws, w, 0.5*ws, w, 0.5*ws);
    path_.moveTo(2.0*ws, -1.0*ws);
    path_.lineTo(4.0*ws, -1.0*ws);
    path_.cubicTo(w, -1.0*ws, w, 0.5*ws, w, 0.5*ws);
}
