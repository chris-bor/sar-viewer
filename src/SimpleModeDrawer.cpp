#include <QtGui>
#include <windef.h>
#include "MainWindow.h"
#include "SimpleModeDrawer.h"
#include "Debugging.h"
#include "Line.h"
#include "Config.h"

extern ViewerConfig viewer_config;
extern QVector<SarViewer::PlotSettings> zoomStack[NrStreams];
extern int currZoom[NrStreams];

namespace SarViewer {
    extern QMutex sar_img_data_buff_mux[NrStreams];
    const QString AXIS_FONT_FAMILY_DEFAULT("Arial"); // Gabriola, LED BOARD REVERSED, French Script MT
    const int AXIS_FONT_SIZE_DEFAULT = 12;
    const QString LABEL_FONT_FAMILY_DEFAULT("Arial");
    const int LABEL_FONT_SIZE_DEFAULT = 10;
}

SarViewer::SimpleModeDrawer::SimpleModeDrawer(SarViewer::SarImageDataBuffer *buff, int nr_stream, QWidget *parent) :
    QObject(parent), quit_(false), bigSarImgDireDataBuf_(NR_LINES_DIRE_BUFFER, nr_stream), nrLines_(0),
    sleepTime_(0), oldNrRangeCells_(SarViewer::NR_PIX_PER_LINE), antDirRight_(false), flags_(0), prf_(0.0), cellSize_(0.0),
    resizeSarImg_(true), resizeBackImg_(true),
    refresh_(true), zoomChanged_(false), noMemmoveShift_(false), currZoom_(0), sizeBackImg_(QSize(SCREEN_X_RESOLUTION,SCREEN_Y_RESOLUTION)),
    imgSar_(NULL), imgBack_(NULL), imgScale_(NULL),
    nrStream_(nr_stream), colorbarImg_(NULL), currLine_(0), rest_(0), shift_(viewer_config.shift_val),
    xAxisFont_(AXIS_FONT_FAMILY_DEFAULT, AXIS_FONT_SIZE_DEFAULT),
    yAxisFont_(AXIS_FONT_FAMILY_DEFAULT, AXIS_FONT_SIZE_DEFAULT),
    labelFont_(LABEL_FONT_FAMILY_DEFAULT, LABEL_FONT_SIZE_DEFAULT) {
//#if DEBUG_CONST
    qDebug() << __FUNCTION__ << "()";
//#endif

//    setStackSize(8*1048576);

    if(!buff) {
        qDebug() << "ERROR passing of SarImageDataBuffer.";
        quit_ = true;
    }
    bigSarImgCircDataBuf_ = buff; // set pointer to circural buffer
    bigSarImgDireDataBuf_.resetBuffer();

    palette_ = Palettes::getPalette(SarViewer::PaletteJet); // PaletteJet PaletteGray

    if(!palette_) {
        qDebug() << "ERROR passing of color palette.";
        quit_ = true;
    }

    QSize img_size(oldNrRangeCells_, WIDGET_HEIGHT_DEF);
    if(imgSar_) {
        delete imgSar_;
    }

    imgSar_ = new QImage(img_size, QImage::Format_ARGB32_Premultiplied);// Format_RGB888  Format_RGB32
    if(imgSar_->isNull()) {
        qDebug() << "Bad allocation SAR image. Application will be closed";
        quit_ = true;
    }
    sourceRect_ = imgSar_->rect();

    if(imgScale_) {
        delete imgScale_;
    }
    imgScale_ = new QImage(this->sizeBackImg_, QImage::Format_ARGB32_Premultiplied);
    imgScale_->fill(0xffffff);

    if(imgBack_) {
        delete imgBack_;
    }
    imgBack_ = new QImage(this->sizeBackImg_, QImage::Format_ARGB32_Premultiplied);
    imgBack_->fill(0xffffff);
    targetRect_ = QRectF(MarginLeft, MarginTop,
                         imgBack_->rect().width() - MarginLeft - colorbarWidth_,
                         imgBack_->rect().height() - MarginTop);

    penBlack_ = QPen(Qt::black);
    xBufStart_ = 0;

    // Create colorbar
    colorbar_ = viewer_config.showColorbar[nrStream_];
    if(colorbar_) {
        colorbarWidth_ = ColorbarWidth;
    }
    else {
        colorbarWidth_ = 0;
    }
    createColorbar();
}


SarViewer::SimpleModeDrawer::~SimpleModeDrawer() {
//#if DEBUG_DESTR
    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
//#endif

    quit_ = true; // to tell run() to stop running as soon as possible

    if(palette_) {
        delete palette_;
        palette_ = NULL;
    }

    if(colorbarImg_) {
        delete colorbarImg_;
        colorbarImg_ = NULL;
    }

    if(imgSar_) {
        delete imgSar_;
        imgSar_ = NULL;
    }

    if(imgBack_) {
        delete imgBack_;
        imgBack_ = NULL;
    }

    if(imgScale_) {
        delete imgScale_;
        imgScale_ = NULL;
    }
}

void SarViewer::SimpleModeDrawer::setPalette(int palette_type) {
#if DEBUG_SIM_MOD_DRAW
    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
    if(palette_) {
        delete palette_;
        palette_ = Palettes::getPalette(palette_type);
    }
    else {
        palette_ = Palettes::getPalette(palette_type);
    }
    this->refresh_ = true;
    this->resizeBackImg_ = true;
    createColorbar();

    startDrawing(this->nrLines_, this->sleepTime_, this->oldNrRangeCells_, this->currLine_, this->currStart_);
}

void SarViewer::SimpleModeDrawer::setSizeBackImg(QSize new_size) {
#if DEBUG_SIM_MOD_DRAW
    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
    if(this->sizeBackImg_ != new_size) {
        this->sizeBackImg_ = new_size;
        this->resizeBackImg_ = true;
    }
    this->refresh_ = true;
    startDrawing(this->nrLines_, this->sleepTime_, this->oldNrRangeCells_, this->currLine_, this->currStart_);
}

void SarViewer::SimpleModeDrawer::setZoom() {
#if DEBUG_SIM_MOD_DRAW
    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
    currZoom_ = currZoom[nrStream_];
    resizeSarImg_ = true;
    resizeBackImg_ = true;
    refresh_ = true;
    zoomChanged_ = true;
    startDrawing(this->nrLines_, this->sleepTime_, this->oldNrRangeCells_, this->currLine_, this->currStart_);
}


void SarViewer::SimpleModeDrawer::showColorbar(bool toggled) {
#if DEBUG_SIM_MOD_DRAW
    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
    colorbar_ = toggled;
    if(colorbar_){
        colorbarWidth_ = ColorbarWidth;
    }
    else {
        colorbarWidth_ = 0;
    }

    this->refresh_ = true;
    this->resizeBackImg_ = true;

    startDrawing(this->nrLines_, this->sleepTime_, this->oldNrRangeCells_, this->currLine_, this->currStart_);
}


void SarViewer::SimpleModeDrawer::checkQuit() {
#if DEBUG_SIM_MOD_DRAW
    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
    if(quit_) {
        emit quitApp();
        this->disconnect();
    }
}


void SarViewer::SimpleModeDrawer::run() {
#if DEBUG_SIM_MOD_DRAW
    qDebug() << "SarViewer::SimpleModeDrawer::run() [ Stream"  << nrStream_ << "]";
#endif

//    int sleep_time = this->sleepTime_*SHIFT_VAL/nr_lines;

    if(!refresh_) { // If refresh do not copy new data
        copyData();
    }

//    qDebug() << "STACK ::::::::::::::: " << stackSize();

//    QTime t;

//    int licznik = 0;

    setStartDrawing();

    prf_ = bigSarImgDireDataBuf_.getBuffer()[xBufStart_].sarImageLineHeader.prf;
    cellSize_ = bigSarImgDireDataBuf_.getBuffer()[xBufStart_].sarImageLineHeader.cell_size;
    flags_ = bigSarImgDireDataBuf_.getBuffer()[xBufStart_].sarImageLineHeader.flags;
    if(antDirRight_ != (bool)BitNtst(flags_, FLAG_BIT_ANT_DIR_RIGHT)) {
        antDirRight_ = (bool)BitNtst(flags_, FLAG_BIT_ANT_DIR_RIGHT);
        resizeBackImg_ = true;
    }

    //        licznik = 0;

    // Drawing with shift
    while(xBufStart_ <= (NR_LINES_DIRE_BUFFER - 1)) {
//            t.start();

//            licznik++;

        // Resize image with SAR image
        if(resizeSarImg_) {
            resizeSarImage();
        }

        // Resize back image with SCALE
        if(resizeBackImg_) {
            resizeBackImage();
        }

        // Draw back image
        drawBackImage();

        emit imageReady(*imgBack_);

        if(zoomChanged_) {
            zoomChanged_ = false;
            break;
        }

        xBufStart_ += shift_;
        xBuf_ = xBufStart_;

//            qDebug("Strumien %d run() tooks %i ms", nrStream_, t.elapsed());
//            qDebug() << "licznik" << licznik;

//            msleep(sleep_time - ((int)t.elapsed() - 10));
//            PR_LINE_VAL(qMax(sleep_time*1551/nr_lines + 20, 100));
//            msleep(qMax(sleep_time*(int)MIN_NR_LINES_TO_DRAW/nr_lines, 50)); // 47
//            msleep(150);
//            msleep(sleep_time + 20);
    }

//        heightShiftDiff_ = NR_LINES_DIRE_BUFFER - 1 - (xBufStart_ - shift_);
//        qDebug() << "***diff" << nrStream_ << heightShiftDiff_;
}


void SarViewer::SimpleModeDrawer::startDrawing(int nr_lines, int sleep_time, int nr_range_cells,
                                                     int curr_line, int curr_start) {
#if DEBUG_SIM_MOD_DRAW
//    qDebug() << __FUNCTION__ << "() [ Stream"  << nrStream_ << "]";
#endif
//    QTime t;
//    t.start();

    // Copy local args to class memebers
    this->nrLines_ = nr_lines;
    this->sleepTime_  = sleep_time;

    // Check size of SAR image
    if(nr_range_cells != this->oldNrRangeCells_) {
        this->oldNrRangeCells_ = nr_range_cells;
        this->resizeSarImg_ = true;
    }

    this->currLine_ = curr_line;
    this->currStart_ = curr_start;

    if(oldRMin_ != bigSarImgDireDataBuf_.getBuffer()[NR_LINES_DIRE_BUFFER - 1].sarImageLineHeader.r_min) {
        qDebug("[stream %d] New number of range cells", nrStream_);
        this->resizeBackImg_ = true;
    }

    // Run drawing
    run();

//    qDebug("Strumien %d startDrawing() tooks %i ms", nrStream_, t.elapsed());
}


void SarViewer::SimpleModeDrawer::createColorbar() {
    if(colorbarImg_) {
        delete colorbarImg_;
    }

    colorbarImg_ = new QImage(1, 256, QImage::Format_ARGB32_Premultiplied);

    uchar* p_bits = colorbarImg_->bits();
    int n_bytes_per_line = colorbarImg_->bytesPerLine();

    for (int y_img = 0; y_img < 256; y_img++) {
        uchar *scan_line = p_bits+y_img*n_bytes_per_line;
        ((uint *)scan_line)[0] = palette_->at(255-y_img);
    }
}


void SarViewer::SimpleModeDrawer::plotColorbar() {
    if(colorbar_) {
        QRectF target(targetRect_.right()+ColorbarMarginLeft, targetRect_.top(), colorbarWidth_-ColorbarMarginLeft-ColorbarMarginRight, targetRect_.height());
        QRectF source(colorbarImg_->rect());

        painter_.drawImage(target, *colorbarImg_, source);
        painter_.drawRect(target);

        for(int j = 0; j <= NrYTicksColorbar; j++) {
            int y = target.top() + (j * (target.height() - 1)) / NrYTicksColorbar;
            double label = (double)((NrYTicksColorbar-j) * 255 / NrYTicksColorbar);
            painter_.drawLine(target.right() - 5, y, target.right(), y);
            painter_.drawLine(target.left() + 5, y, target.left(), y);
            painter_.drawText(target.right()+5, y - 10,
                             MarginLeft - 5, 20,
                             Qt::AlignLeft | Qt::AlignVCenter, QString("%1").arg(label, 0, 'f', 1));
        }
    }
}


void SarViewer::SimpleModeDrawer::copyData() {
        SarViewer::sar_img_data_buff_mux[this->nrStream_].lock();
        bigSarImgCircDataBuf_->copyToDirectBufferWithLap(bigSarImgDireDataBuf_, currLine_, currStart_);
        SarViewer::sar_img_data_buff_mux[this->nrStream_].unlock();
}


/**<
  \brief Sets place at the direct buffer where start drawing.
  \param refresh boolean value indicate if copy new data from buffer or only redraw SAR image
  \param nr_lines how many new lines
  \param rest rest lines to draw between two new blocks of data
*/
void SarViewer::SimpleModeDrawer::setStartDrawing() {
    plotSett_ = zoomStack[nrStream_][currZoom[nrStream_]];

    // Set start drawing
    if(!refresh_) { // No refresh, normal received new data
        xBufStart_ = (NR_LINES_DIRE_BUFFER - 1) - nrLines_ + shift_ - rest_; // Set begin of new block at buffer
    rest_ = 0;
    }
    else { // Refresh (if palette, size, zoom was changed)
//            qDebug() << "REFRESH";
        if(zoomChanged_) { // if zoom was changed
            xBufStart_ = (NR_LINES_DIRE_BUFFER - 1) - plotSett_.minYRound(); // Set begin of new block at buffer
            rest_ = plotSett_.minYRound();
        }
        else // f.g. palette, size
            xBufStart_ = (NR_LINES_DIRE_BUFFER - 1); // Set begin of new block at buffer
        //this->refresh_ = false;
    }
    xBuf_ = xBufStart_;
}


/**<
  \brief Function called if resize SAR image is necessary.
  \param resize_sar_img reference which define if resize SAR image
*/
void SarViewer::SimpleModeDrawer::resizeSarImage() {
#if DEBUG_SIM_MOD_DRAW
    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
    if(!currZoom_) { // jak nie ma zoom to wg old nrRange
        zoomStack[nrStream_][currZoom[nrStream_]].maxX_ = bigSarImgDireDataBuf_.getBuffer()[NR_LINES_DIRE_BUFFER - 1].sarImageLineHeader.nr_range_cells;
        plotSett_ = zoomStack[nrStream_][currZoom[nrStream_]];
        if(imgSar_) {
            delete imgSar_;
        }
        imgSar_ = new QImage(oldNrRangeCells_, WIDGET_HEIGHT_DEF, QImage::Format_ARGB32_Premultiplied);
        if(imgSar_->isNull()) {
            qDebug() << "Bad allocation SAR image. Application will be closed";
            exit(-1);
        }
//        bigSarImgCircDataBuf_->resetBufferPix();
    }
    else {// jak zoom to z zakresu zoomu
        if(imgSar_) {
            delete imgSar_;
        }
        imgSar_ = new QImage((int)(plotSett_.maxXRound() - plotSett_.minXRound()), (int)(plotSett_.maxYRound() - plotSett_.minYRound()), QImage::Format_ARGB32_Premultiplied);
        if(imgSar_->isNull()) {
            qDebug() << "Bad allocation SAR image. Application will be closed";
            exit(-1);
        }
    }
    sourceRect_ = imgSar_->rect();
    this->resizeSarImg_ = false;

    // Set if draw using memmove() or draw image from raw data
    if((uint)imgSar_->height() < 2*viewer_config.shift_val) {
        noMemmoveShift_ = true;
    }
    else {
        noMemmoveShift_ = false;
    }
}


/**<
  \brief Function called if resize back image is necessary.
  \param resize_back_img reference which define if resize back image
  \param r_min reference to r_min value
  \param ant_dir_right reference to variable which define if antenna looks at right direction
*/
void SarViewer::SimpleModeDrawer::resizeBackImage() {
#if DEBUG_SIM_MOD_DRAW
    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
    oldRMin_ = bigSarImgDireDataBuf_.getBuffer()[NR_LINES_DIRE_BUFFER - 1].sarImageLineHeader.r_min;
    resizeBackImg_ = false;
    if(imgBack_) {
        delete imgBack_;
    }
    imgBack_ = new QImage(this->sizeBackImg_, QImage::Format_ARGB32_Premultiplied);
    imgBack_->fill(0xffffff);
    if(imgScale_) {
        delete imgScale_;
    }
    imgScale_ = new QImage(this->sizeBackImg_, QImage::Format_ARGB32_Premultiplied);
    imgScale_->fill(0xffffff);
    targetRect_ = QRectF(MarginLeft, MarginTop,
                         sizeBackImg_.width() - MarginLeft - MarginRight - colorbarWidth_,
                         sizeBackImg_.height()- MarginTop - MarginBottom);

    painter_.begin(imgScale_);
    painter_.setRenderHint(QPainter::Antialiasing);
    painter_.setPen(penBlack_);
//    QFont font = painter_.font();
//    font.setPointSize(12);
    painter_.setFont(xAxisFont_);

    // Draw X title
    painter_.drawText(imgScale_->width()/2 - 100, 0, 200, 25,
                      Qt::AlignHCenter | Qt::AlignVCenter, tr("SLANT RANGE [m]"));

    // Draw Y title
    painter_.save();

    // 1 way
//    painter_.rotate(-90);
//    font.setPointSizeF(12.4);
//    font.setBold(true);
//    painter_.setFont(yAxisFont_);
//    painter_.drawText(-imgScale_->height()/2, 18, tr("TIME [s]"));

    // 2 way
    int x = 3;
    int dy = 15;
    QString str(tr("TIME "));
    int y = imgScale_->height()/2 - str.size()/2*dy;
    painter_.setFont(yAxisFont_);
    for(int i = 0; i<str.size();i++){
        painter_.drawText(x, y, 20, 18, Qt::AlignHCenter | Qt::AlignVCenter, str.at(i));
        y+=dy;
    }
    painter_.drawText(x, y, 20, 18, Qt::AlignHCenter | Qt::AlignVCenter, tr("[s]"));

    painter_.restore();

    // Draw X scale
//    font.setBold(false);
//    font.setPointSize(10);
    painter_.setFont(labelFont_);

    if(antDirRight_) { // Antenna direction right
        for(int i = 0; i <= NrXTicks; i++) {
            int x = targetRect_.left() + (i * (targetRect_.width() - 1)) / NrXTicks;
            painter_.drawLine(x, targetRect_.top(), x, targetRect_.top() - 5);
            double label = oldRMin_ + plotSett_.minX_*cellSize_ + (i * plotSett_.spanX() / plotSett_.numXTicks_)*cellSize_;
            painter_.drawText(x - 50, targetRect_.top() - 25, 100, 20, Qt::AlignHCenter | Qt::AlignTop,
                              QString("%1").arg(label, 0, 'f', 1));
        }
    }
    else { // Antenna direction left
        for(int i = NrXTicks; i >= 0; i--) {
            int x = targetRect_.left() + ((NrXTicks - i) * (targetRect_.width() - 1)) / NrXTicks;
            painter_.drawLine(x, targetRect_.top(), x, targetRect_.top() - 5);
            double label = oldRMin_ + ((double)(oldNrRangeCells_ - plotSett_.maxXRound()))*cellSize_ + (i * plotSett_.spanX() / plotSett_.numXTicks_)*cellSize_;
            painter_.drawText(x - 50, targetRect_.top() - 25, 100, 20, Qt::AlignHCenter | Qt::AlignTop,
                              QString("%1").arg(label, 0, 'f', 1));
        }
    }

    // Plot colorbar
    plotColorbar();

    painter_.end();
}


/**<
  \brief Function draws back image with SAR image and scale.
  \param prf pulse repetition frequency
  \param ant_dir_right reference to variable which define if antenna looks at right direction
  \param old_nr_range_cells old number of range cells
*/
void SarViewer::SimpleModeDrawer::drawBackImage() {
#if DEBUG_SIM_MOD_DRAW
    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
    (*imgBack_) = imgScale_->copy(0, 0, imgScale_->width(), imgScale_->height());

    painter_.begin(imgBack_);
    painter_.setRenderHint(QPainter::Antialiasing, true);
    painter_.setPen(penBlack_);
//    QFont font = painter_.font();
//    font.setPointSize(10);
    painter_.setFont(labelFont_);

    // Draw Y scale
    for(int j = 0; j <= plotSett_.numYTicks_; j++) {
        int y = targetRect_.top() + (j * (targetRect_.height() - 1)) / plotSett_.numYTicks_;
        double label = (double)bigSarImgDireDataBuf_.getBuffer()[xBufStart_].sarImageLineHeader.line_cnt/(double)prf_
                        + (double)(j * plotSett_.spanY() / plotSett_.numYTicks_)/(double)prf_;
        painter_.drawLine(targetRect_.left() - 5, y, targetRect_.left(), y);
        painter_.drawText(targetRect_.left() - MarginLeft - 10, y - 10,
                         MarginLeft - 5, 20,
                         Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(label, 0, 'f', 1));
    }

//    QTime t;
//    t.start();

    // Draw sar image
    uchar* p_bits = imgSar_->bits(); // returns a pointer to the first pixel data
    int n_bytes_per_line = imgSar_->bytesPerLine(); // returns the number of bytes per image scanline
    plotSett_.spanXRound_ = plotSett_.spanXRound();
    plotSett_.spanYRound_ = plotSett_.spanYRound();
    plotSett_.minXRound_ = plotSett_.minXRound();
    plotSett_.maxXRound_ = plotSett_.maxXRound();

    // Copy old part of the SAR image

    if(!zoomChanged_ && !refresh_ && !noMemmoveShift_) { // If no zoom and refresh, only normal drawing with shift
        // Copy SAR image
        memmove(p_bits+n_bytes_per_line*shift_,
                p_bits,
                imgSar_->numBytes() - n_bytes_per_line*shift_);

        // Draw the new part
        SarViewer::Line *line = bigSarImgDireDataBuf_.getBuffer();
        if(antDirRight_) { // antenna direction right
            for(uint y_img = 0; y_img < shift_; y_img++) { // Vertical
                SarViewer::Line *subline = &line[xBuf_];
                for(int x_img = 0; x_img < plotSett_.spanXRound_; x_img++) { // Horizontal
                    uchar *scan_line = p_bits+y_img*n_bytes_per_line;
                    ((uint *)scan_line)[x_img] = palette_->at(subline->pixels[x_img+plotSett_.minXRound_]);
                }
                xBuf_--;
            }
        }
        else { // Antenna direction left
            for(uint y_img = 0; y_img < shift_; y_img++) { // Vertical
                SarViewer::Line *subline = &line[xBuf_];
                for(int x_img = plotSett_.spanXRound_-1, x_back = 0; x_img >=0 ; x_img--, x_back++) { // Horizontal
                    uchar *scan_line = p_bits+y_img*n_bytes_per_line;
                    ((uint *)scan_line)[x_back] = palette_->at(subline->pixels[oldNrRangeCells_-plotSett_.maxXRound_+x_img]);
                }
                xBuf_--;
            }
        }
    }
    else { // Draw the image afresh
        this->refresh_ = false;
        SarViewer::Line *line = bigSarImgDireDataBuf_.getBuffer();
        if(antDirRight_) { // antenna direction right
            for(int y_img = 0; y_img < plotSett_.spanYRound_; y_img++) { // Vertical
                SarViewer::Line *subline = &line[xBuf_];
                for(int x_img = 0; x_img < plotSett_.spanXRound_; x_img++) { // Horizontal
                    uchar *scan_line = p_bits+y_img*n_bytes_per_line;
                    ((uint *)scan_line)[x_img] = palette_->at(subline->pixels[x_img+plotSett_.minXRound_]);
                }
                xBuf_--;
            }
        }
        else { // Antenna direction left
            for(int y_img = 0; y_img < plotSett_.spanYRound_; y_img++) { // Vertical
                SarViewer::Line *subline = &line[xBuf_];
                for(int x_img = plotSett_.spanXRound_-1, x_back = 0; x_img >=0 ; x_img--, x_back++) { // Horizontal
                    uchar *scan_line = p_bits+y_img*n_bytes_per_line;
                    ((uint *)scan_line)[x_back] = palette_->at(subline->pixels[oldNrRangeCells_-plotSett_.maxXRound_+x_img]);
                }
                xBuf_--;
            }
        }
    }

    painter_.drawImage(targetRect_, (*imgSar_), sourceRect_);
    painter_.end();
//    qDebug("Strumien %d run() tooks %i ms", nrStream_, t.elapsed());
}
