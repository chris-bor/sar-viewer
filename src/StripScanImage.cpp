#include <windef.h>
#include "Debugging.h"
#include "StripScanImage.h"
#include "Config.h"
#include "GeoConv.h"
#include "MapViewer.h"

extern ViewerConfig viewer_config;

namespace SarViewer {
    extern QMutex sar_img_data_buff_mux[NrStreams];
    extern bool strip_scan_img_mux;
}

SarViewer::StripScanImage::StripScanImage(SpecialPoint *start_point, int nr_stream, SarViewer::StripScan *strip_scan, QCustomPlot *parent_plot) :
    QCPAbstractItem(parent_plot),
    topLeft(createPosition("topLeft")),
    bottomRight(createPosition("bottomRight")),
    bigSarImgCircDataBuf_(0L),
    headersBuffer_(0L),
    lastHeaderPtr_(0L),
    pixelsBuffer_(0L),
    zBuffer_(0L),
    nrStream_(nr_stream),
    antDirRight_(0), // "0" means left "1" means right
    refreshCnt_(0),
    startIdx_(0),
    widthRatio_(1),
    heightRatio_(1),
    startPoint_(start_point),
    stripScan_(strip_scan) {
#if DEBUG_CONST
    qDebug() << "CONSTRUCTOR OF StripScanImage";
#endif
    qDebug() << "startPoint" << start_point->lat << start_point->lon;
    setClipAxes(parent_plot->xAxis, parent_plot->yAxis);
    setScaled(false);

//    setImage(QImage("./images/MainWindowIcon.png"));
    mImage = QImage(QSize(20, 20), QImage::Format_ARGB32_Premultiplied);// Format_RGB888  Format_RGB32
    mImage.load("./images/MainWindowIcon.png");
    mImage = mImage.scaled(QSize(20,20));

    topLeft->setType(QCPItemPosition::ptPlotCoords);
    topLeft->setCoords(15000.0, -20000.0);

    bottomRight->setType(QCPItemPosition::ptPlotCoords);
    bottomRight->setCoords(15100.0, -20100.0);

    // Create data buffer and z buffer
    dataSize_ = viewer_config.nr_lin_to_ref_str_scan * viewer_config.nr_ref_str_scan;
    headersBuffer_ = new sar_image_line_header[dataSize_];
    lastHeaderPtr_ = &headersBuffer_[dataSize_-1];
    pixelsBuffer_ = new quint8[dataSize_*SarViewer::MAX_PIX_PER_LINE];
    zBuffer_ = new float [dataSize_];  
    pos_ = new SarViewer::Pos [dataSize_];

    memset(pixelsBuffer_, 0, sizeof(quint8)*dataSize_*SarViewer::MAX_PIX_PER_LINE);
    memset(zBuffer_, 0, sizeof(float)*dataSize_);
    memset(pos_, 0, sizeof(SarViewer::Pos)*dataSize_);

    palette_ = Palettes::getPalette(SarViewer::PaletteGray); // PaletteJet PaletteGray

    setLayer("image");

    setScaled(true);
}

SarViewer::StripScanImage::~StripScanImage() {
#if DEBUG_CONST
    qDebug() << "DESTRUCTOR OF StripScanImage";
#endif
    if(headersBuffer_) {
        delete [] headersBuffer_;
        headersBuffer_ = 0L;
    }
    if(pixelsBuffer_) {
        delete [] pixelsBuffer_;
        pixelsBuffer_ = 0L;
    }
    if(zBuffer_) {
        delete [] zBuffer_;
        zBuffer_ = 0L;
    }

    if(pos_) {
        delete [] pos_;
        pos_ = 0L;
    }
    if(palette_) {
        delete palette_;
        palette_ = NULL;
    }
}


void SarViewer::StripScanImage::setImage(const QImage &image) {
    //    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    mImage = image;
}


void SarViewer::StripScanImage::setPen(const QPen &pen) {
    //    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    mPen = pen;
}


void SarViewer::StripScanImage::setSelectedPen(const QPen &pen) {
    //    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    mSelectedPen = pen;
}


double SarViewer::StripScanImage::selectTest(const QPointF &pos) const {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";

    if (!mVisible)
        return -1;

    return rectSelectTest(getFinalRect(), pos, true);
}


/**<
  \brief Main function creating scan strip image.
*/
void SarViewer::StripScanImage::drawStripScanImage(int most_actual_line) {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
// Jak bedzie zawolno: r_min i cell_size dac wskaznik do lla2fe i tam podmianiac a petle for nizej delete.

    QTime t;
    t.start();
//    SarViewer::strip_scan_img_mux = 1;
    // Copy data from circular buffer
    SarViewer::sar_img_data_buff_mux[nrStream_].lock();
    bigSarImgCircDataBuf_->copyNewScanStrip(headersBuffer_, pixelsBuffer_, most_actual_line, dataSize_);
    SarViewer::sar_img_data_buff_mux[nrStream_].unlock();
    refreshCnt_++;

    if(dataSize_ > (int)(refreshCnt_*viewer_config.nr_lin_to_ref_str_scan)) {
        startIdx_ = dataSize_-refreshCnt_*viewer_config.nr_lin_to_ref_str_scan;
    }
    else {
        startIdx_ = 0;
    }

    // Latitude, longitude, attitude 2 flat earth
//    SarViewer::GeoConv::lla2fe(headersBuffer_, zBuffer_, &dataSize_,
//                               startPoint_->lat, startPoint_->lon, 90, H_REFF);
    SarViewer::GeoConv::mapGeoPos2xy(headersBuffer_, zBuffer_, &dataSize_, startPoint_->lat, startPoint_->lon, H_REFF);

    // Initialize required parameters
    cell_size_ = lastHeaderPtr_->cell_size;
    nrRangeCells_ = lastHeaderPtr_->nr_range_cells;
    r_min_ = lastHeaderPtr_->r_min;
    r_max_ = r_min_ + cell_size_*nrRangeCells_;

    // Image correction
    imageCorrection();

//    SarViewer::strip_scan_img_mux = 0;
    qDebug("Strumien draw() tooks %i ms", t.elapsed());
}


void SarViewer::StripScanImage::setNewSarSize(int width, int height) {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    newWidth_ = width;
    newHeight_ = height;
}


/**<
  \brief This function sets scan strip image at map window.
*/
void SarViewer::StripScanImage::draw(QCPPainter *painter) {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";

    // todo : rect wg bnd mo¿e jest dobry???

    bool flipHorz = false;
    bool flipVert = false;
    QRect rect = getFinalRect(&flipHorz, &flipVert);
    double clipPad = mainPen().style() == Qt::NoPen ? 0 : mainPen().widthF();
    QRect boundingRect = rect.adjusted(-clipPad, -clipPad, clipPad, clipPad);

    if (boundingRect.intersects(clipRect())) {
//    updateScaledPixmap(rect, flipHorz, flipVert);
//    painter->drawPixmap(rect.topLeft(), mScaled ? mScaledPixmap : mPixmap);

        painter->drawImage(topLeft->pixelPoint().toPoint(), mImage);
//        painter->drawImage(rect.topLeft(), mImage);

//    QPen pen = mainPen();
    //    if (pen.style() != Qt::NoPen)
    //    {
    //      painter->setPen(pen);
    //      painter->setBrush(Qt::NoBrush);
    //      painter->drawRect(rect);
    //    }
    }


//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ] end";
}


void SarViewer::StripScanImage::imageCorrection() {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    resamplTrajectory();

    // Check antenna direction
    calcAntennaDirection();

    // Calculate minimum and maximum positions of the strip scan
    calcMinMaxPositions();

    // Calculate size of the creating image
    calcImageSize();

    stripScan_->getBounds(imgBnd_);
//    qDebug() << "WIDTH" << imgBnd_.width_pix << imgBnd_.height_pix;
//    qDebug() << "minXmax"<<imgBnd_.x_min<<imgBnd_.y_max<<imgBnd_.x_max<<imgBnd_.y_min;
//    qDebug() << "minX_" << minX_ << "maxY_" << maxY_;

    setNewSarSize(imgBnd_.width_pix, imgBnd_.height_pix);
//    setNewSarSize(1920, 1080);

    // Calculate rations between new and old sar size
    widthRatio_ = (float)(newWidth_)/(float)oryginalWidth_;
    heightRatio_ = (float)(newHeight_)/(float)oryginalHeight_;

    // Create sar image
    mImage = mImage.scaled(QSize(newWidth_, newHeight_));
//    mImage = QImage(newWidth_, newHeight_, QImage::Format_ARGB32_Premultiplied);
    vals = new int[(newWidth_)*(newHeight_)];
    cnts = new int[(newWidth_)*(newHeight_)];

    memset(vals, 0, sizeof(int)*(newWidth_)*(newHeight_));
    memset(cnts, 0, sizeof(int)*(newWidth_)*(newHeight_));

    calcSarImage();

    topLeft->setCoords(imgBnd_.x_min, imgBnd_.y_max);
//    topLeft->setCoords(minX_, maxY_);

    calcMean();

    delete [] vals;
    delete [] cnts;
    //    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ] end";
}


void SarViewer::StripScanImage::resamplTrajectory() {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    float dx1, dx2, dy1, dy2;
    // Linear resampling of positions
    bool traj_resampl = 1;
    if(traj_resampl == 1) {
        dx1 = headersBuffer_[startIdx_].lat_plat;
        dx2 = lastHeaderPtr_->lat_plat;
        dy1 = headersBuffer_[startIdx_].lon_plat;
        dy2 = lastHeaderPtr_->lon_plat;
        float deltax = dx2 - dx1;
        float deltay = dy2 - dy1;

        for(int cnt = startIdx_; cnt < dataSize_; cnt++) {
            headersBuffer_[cnt].lat_plat = dx1+cnt*deltax/dataSize_;
            headersBuffer_[cnt].lon_plat = dy1+cnt*deltay/dataSize_;
        }
    }
}


void SarViewer::StripScanImage::calcAntennaDirection() {
    if((bool)BitNtst(lastHeaderPtr_->flags, FLAG_BIT_ANT_DIR_RIGHT)) {
        antDirRight_ = 1;
    }
    else {
        antDirRight_ = 0;
    }
}


void SarViewer::StripScanImage::calcMinMaxPositions() {
    float dx1, dx2, dy1, dy2, dl1, dl2;
    double alpha, beta;

    for(int cnt = startIdx_; cnt < dataSize_; cnt++) {
//        r_min_ = headersBuffer_[cnt].r_min;
        // Calculate angle
        if(zBuffer_[cnt] < r_min_) {
            alpha = qAcos(zBuffer_[cnt]/r_min_);
        }
        else {
            alpha = M_PI/2.0;
        }

        dl1 = r_min_*qSin(alpha);

//        r_max_ = r_min_ + headersBuffer_[cnt].nr_range_cells*headersBuffer_[cnt].cell_size;
        dl2 = r_max_*qSin(alpha);

        // % heading + w prawo - w lewo
        beta = M_PI-headersBuffer_[cnt].head-M_PI*antDirRight_;
        dy1 = qSin(beta)*dl1;
        dx1 = qCos(beta)*dl1;
        dy2 = qSin(beta)*dl2;
        dx2 = qCos(beta)*dl2;
        pos_[cnt].xstart = headersBuffer_[cnt].lat_plat+dx1;
        pos_[cnt].ystart = headersBuffer_[cnt].lon_plat+dy1;
        pos_[cnt].xstop = headersBuffer_[cnt].lat_plat+dx2;
        pos_[cnt].ystop = headersBuffer_[cnt].lon_plat+dy2;
//        qDebug() << pos_[cnt].xstart << pos_[cnt].xstop << pos_[cnt].ystart << pos_[cnt].ystop;
    }
}


void SarViewer::StripScanImage::calcImageSize() {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";

    minPos_.xstart = pos_[startIdx_].xstart;
    minPos_.xstop = pos_[startIdx_].xstop;
    minPos_.ystart = pos_[startIdx_].ystart;
    minPos_.ystop = pos_[startIdx_].ystop;
    maxPos_.xstart = pos_[startIdx_].xstart;
    maxPos_.xstop = pos_[startIdx_].xstop;
    maxPos_.ystart = pos_[startIdx_].ystart;
    maxPos_.ystop = pos_[startIdx_].ystop;

    for(int cnt = startIdx_; cnt < dataSize_; cnt++) {
        if(pos_[cnt].xstart < minPos_.xstart) // min xstart
            minPos_.xstart = pos_[cnt].xstart;
        if(pos_[cnt].xstart > maxPos_.xstart) // max xstart
            maxPos_.xstart = pos_[cnt].xstart;

        if(pos_[cnt].xstop < minPos_.xstop) //min xstop
            minPos_.xstop = pos_[cnt].xstop;
        if(pos_[cnt].xstop > maxPos_.xstop) // max xstop
            maxPos_.xstop = pos_[cnt].xstop;

        if(pos_[cnt].ystart < minPos_.ystart) // min ystart
            minPos_.ystart = pos_[cnt].ystart;
        if(pos_[cnt].ystart > maxPos_.ystart) // max ystart
            maxPos_.ystart = pos_[cnt].ystart;

        if(pos_[cnt].ystop < minPos_.ystop) // min ystop
            minPos_.ystop = pos_[cnt].ystop;
        if(pos_[cnt].ystop > maxPos_.ystop) // max ystop
            maxPos_.ystop = pos_[cnt].ystop;
    }

    minX_ = qFloor(qMin(minPos_.xstart, minPos_.xstop));
    maxX_ = qCeil(qMax(maxPos_.xstart, maxPos_.xstop));

    minY_ = qFloor(qMin(minPos_.ystart, minPos_.ystop));
    maxY_ = qCeil(qMax(maxPos_.ystart, maxPos_.ystop));

    oryginalWidth_ = (qCeil((maxX_-minX_+1)/cell_size_));
    oryginalHeight_ = (qCeil((maxY_-minY_+1)/cell_size_));

//    qDebug() << "width" << oryginalWidth_;
//    qDebug() << "height" << oryginalHeight_;
//    qDebug() << "minX_" << minX_ << "maxX_" << maxX_ << "minY_" << minY_ << "maxY_" << maxY_;
//    qDebug() << "minPos_.xstart" << minPos_.xstart << "maxPos_.xstart" << maxPos_.xstart
//             << "minPos_.ystart" << minPos_.ystart << "maxPos_.ystart" << maxPos_.ystart
//             << "minPos_.xstop" << minPos_.xstop << "maxPos_.xstop" << maxPos_.xstop
//             << "minPos_.ystart" << minPos_.ystop << "maxPos_.ystop" << maxPos_.ystop;
}


void SarViewer::StripScanImage::calcSarImage() {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    double alpha, beta;
    float dl_min, dl, dx, dy, pos_x, pos_y;
    int x_idx, y_idx, idx;

    for(int ww = startIdx_; ww < dataSize_; ww++) {
//        r_min_ = headersBuffer_[ww].r_min;

        if(zBuffer_[ww] < r_min_) {
            alpha = qAcos(zBuffer_[ww]/r_min_);
        }
        else {
            alpha = M_PI/2.0;
        }

        dl_min = r_min_*qSin(alpha);

        for(int kk = 0; kk < nrRangeCells_; kk++) {
            dl = dl_min+cell_size_*kk; // Odleglosc do kazdej komorki odleglosciowej // todo tu mo¿e siê nie zgadzaæ
//            beta =
            dy = qSin(M_PI-headersBuffer_[ww].head-M_PI*antDirRight_)*dl; // skladowa pionowa Y
            dx = qCos(M_PI-headersBuffer_[ww].head-M_PI*antDirRight_)*dl; // skladowa poxioma X
            pos_x = headersBuffer_[ww].lat_plat+dx; // wspolrzedna komorki w m
            pos_y = headersBuffer_[ww].lon_plat+dy;

            x_idx = qRound((pos_x-minX_)/cell_size_*widthRatio_);
            y_idx = qRound((pos_y-minY_)/cell_size_*heightRatio_);
            idx = (newHeight_-y_idx)*(newWidth_)+x_idx;
//            idx = (y_idx)*newWidth_+x_idx;
            idx = qMin(qMax(idx,0),newWidth_*newHeight_-1);

//            if(idx<0 || idx>(newWidth_)*(newHeight_)) {
//                qDebug() << "idx" << idx << ">" << (newWidth_)*(newHeight_) << "ww" << ww << "kk" << kk;
//                idx = newWidth_*(newHeight_)-1;
//            }
            // Accumulate value and counter
            vals[idx] += pixelsBuffer_[ww*SarViewer::MAX_PIX_PER_LINE+kk];
            cnts[idx]++;
        }
//        qDebug() << "idx" << idx << "w" << widthRatio_ << "h" << heightRatio_ << "x_idx" << x_idx << "y_idx" << y_idx;
    }
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ] end";
}


void SarViewer::StripScanImage::calcMean() {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    uchar* p_bits = mImage.bits(); // returns a pointer to the first pixel data
    int n_bytes_per_line = mImage.bytesPerLine(); // returns the number of bytes per image scanline
    uint idx = 0;
    quint8 val = 0;

    for(int h = 0; h < newHeight_; h++) {
        for(int w = 0; w < newWidth_; w++) {
//qDebug() << "val" << val << "idx" <<  idx << "vals[idx]" << vals[idx] << "cnts[idx]" << cnts[idx];
            uchar *scan_line = p_bits+h*n_bytes_per_line;
            idx = h*newWidth_+w;
            if(cnts[idx] > 0){
                val = vals[idx]/cnts[idx];
                ((uint *)scan_line)[w] = palette_->at(val);
            }
            else {
                val = 0;
                ((uint *)scan_line)[w] = (0x00 << 24) | ((0xff) << 16) | (( 0xff) << 8) | ( 0xff);
            }
        }
    }
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ] end";
}


QRect SarViewer::StripScanImage::getFinalRect(bool *flippedHorz, bool *flippedVert) const {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";

    QRect result;

    bool flipHorz = false;
    bool flipVert = false;
    QPoint p1 = topLeft->pixelPoint().toPoint();
    QPoint p2 = bottomRight->pixelPoint().toPoint();

//    if (p1 == p2)
//        return QRect(p1, QSize(0, 0));
    if (mScaled) // jesli ma skalowac, to skaluje wg prostokata od topLeft do bottomRight
    {
        QSize newSize = QSize(p2.x()-p1.x(), p2.y()-p1.y());
        QPoint topLeft = p1;
        if (newSize.width() < 0)
        {
            flipHorz = true;
            newSize.rwidth() *= -1;
            topLeft.setX(p2.x());
        }
        if (newSize.height() < 0)
        {
            flipVert = true;
            newSize.rheight() *= -1;
            topLeft.setY(p2.y());
        }
        QSize scaledSize = mImage.size();
        scaledSize.scale(newSize, mAspectRatioMode);
        result = QRect(topLeft, scaledSize);
    } else
    {
        result = QRect(p1, mImage.size());
    }
    if (flippedHorz)
        *flippedHorz = flipHorz;
    if (flippedVert)
        *flippedVert = flipVert;

    return result;

}


QPen SarViewer::StripScanImage::mainPen() const {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    return mSelected ? mSelectedPen : mPen;
}


void SarViewer::StripScanImage::setImageUnvisible() {
    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";

    // tu init bits i scanline
    for(int y = 0; y < mImage.size().height(); y++) {
        for(int x = 0; x < mImage.size().width(); x++) {
            // ustawiac na niewidzialne
        }
    }
}


void SarViewer::StripScanImage::setScaled(bool val) {
    mScaled = val;
}


void SarViewer::StripScanImage::setCircBuffer(SarViewer::SarImageDataBuffer *circ_buff) {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    if(circ_buff != 0L) {
        bigSarImgCircDataBuf_ = circ_buff;
    }
    else {
        qDebug() << "ERROR Bad allocation of the circural buffer";
    }
}
