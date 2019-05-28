#include <time.h>
#include "MainWindow.h"
#include "SimpleMode.h"
#include "View.h"
#include "Config.h"
#include "Palettes.h"
#include "TimeMeasure.h"


namespace SarViewer {
    SimpleMode::SimpleMode( QMainWindow *mainWindow ) :
        QObject(),
        simpleModeSwitch_(false),
        bigSarImgCircDataBufStream0_(NR_LINES_CIRC_BUFFER, Stream0),
        bigSarImgCircDataBufStream1_(NR_LINES_CIRC_BUFFER, Stream1),
        /*bigSarImgDireDataBufStream0_(NR_LINES_DIRE_BUFFER),
        bigSarImgDireDataBufStream1_(NR_LINES_DIRE_BUFFER),*/
        simpleModeDrawerStream0_(&bigSarImgCircDataBufStream0_, Stream0), // qobject_cast<QWidget *>(mainWindow)
        simpleModeDrawerStream1_(&bigSarImgCircDataBufStream1_, Stream1) { // qobject_cast<QWidget *>(mainWindow),
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF SimpleMode\n";
#endif
        if(mainWindow->inherits("QMainWindow")) {
            this->parentWindow_ = (MainWindow *)mainWindow;
        }

        scenePixmapSize_.setWidth(WIDGET_WIDTH_DEF); // WIDGET_WIDTH_DEF 1250 1366
        scenePixmapSize_.setHeight(WIDGET_HEIGHT_DEF); // WIDGET_HEIGHT_DEF 700 768

//        PR_LINE_VAL(WIDGET_HEIGHT_DEF);
        /*imageIdx8Size_.setWidth(WIDGET_WIDTH_DEF);
        imageIdx8Size_.setHeight(WIDGET_HEIGHT_DEF);

        this->sceneImage_ = new QImage(scenePixmapSize_, QImage::Format_RGB32); // Faster. QImage::Format_Indexed8*/
        this->image32_[Stream0] = QImage(scenePixmapSize_, QImage::Format_ARGB32_Premultiplied); // Format_ARGB32 || Format_ARGB32_Premultiplied
        this->image32_[Stream1] = QImage(scenePixmapSize_, QImage::Format_ARGB32_Premultiplied); // Format_ARGB32 || Format_ARGB32_Premultiplied
        this->scenePixmap_[Stream0] = QPixmap(scenePixmapSize_); // Main pixmap
        this->scenePixmap_[Stream1] = QPixmap(scenePixmapSize_); // Main pixmap

        /*this->imageIdx8_ = QImage(imageIdx8Size_, QImage::Format_Indexed8);*/
        palette_ = Palettes::getPalette(SarViewer::PaletteJet);
        /*imageIdx8_.setColorTable(*palette_);*/

        replayTimer_ = new QTimer( this );
        simpleModeTimer_ = new QTimer( this );

        /*QObject::connect(this->replayTimer_, SIGNAL(timeout()), this, SLOT(replayDraw()));*/
        /*QObject::connect(&sarImageReader_, SIGNAL(endOfFile()), this->replayTimer_, SLOT(stop()));*/

        scenePixmapItem_[Stream0] = NULL;
        scenePixmapItem_[Stream1] = NULL;

        dockWidgVisi_[Stream0] = true;
        dockWidgVisi_[Stream1] = true;

//        // Create SimpleModeDrawer objects
//        simpleModeDrawerStream0_ = new SimpleModeDrawer(&bigSarImgCircDataBufStream0_, Stream0);
//        qDebug() << "********";
//        simpleModeDrawerStream1_ = new SimpleModeDrawer(&bigSarImgCircDataBufStream1_, Stream1);
//        qDebug() << "+++++++++";
    }


    SimpleMode::~SimpleMode() {
#if DEBUG_DESTR
        qDebug() << "DESTRUCTOR OF SimpleMode\n";
#endif
        if(palette_)
            delete palette_;
    }

/*
    void SimpleMode::replay() {

        QString fileName = parentWindow_->getSARDataFileName();

        sarImgCircDataBufStream0_.resetBuffer();
        sarImgCircDataBufStream1_.resetBuffer();
        sarImgDireDataBufStream0_.resetBuffer();
        sarImgDireDataBufStream1_.resetBuffer();

        sarImageReader_.openSARFile(fileName);

        QPoint start_point(0, 0);

        scenePixmapItem_[Stream0] = (parentWindow_->getScene(Stream0))->addPixmap(scenePixmap_[Stream0]);

        scenePixmapItem_[Stream0]->setPos((start_point));
        scenePixmapItem_[Stream0]->setPixmap(scenePixmap_[Stream0]); // ???po co to????

        simpleModeSwitch_ = true;

        replayTimer_->start(REPLAY_TIMER_VAL);

        return;
    }


    void SimpleMode::replayDraw() {
        QTime t;
        t.start();

//        scenePixmap_.convertFromImage( image32_, Qt::AutoColor );
        scenePixmapItem_[Stream0]->setPixmap(scenePixmap_[Stream0]);

        sarImageReader_.readLines(&sarImgCircDataBufStream0_);

        drawLines();

            ( parentWindow_->getView(Stream0) )->repaint(parentWindow_->geometry().x(),
                                                         parentWindow_->geometry().y(),
                                                         parentWindow_->geometry().width(),
                                                         parentWindow_->geometry().height());

        qDebug("replayDraw() tooks %i ms", t.elapsed());
    }
*/

    void SimpleMode::startSimpleModeNetwork() {
        qDebug() << __FUNCTION__ << "()";
        bigSarImgCircDataBufStream0_.resetBuffer();
        bigSarImgCircDataBufStream1_.resetBuffer();

        QPoint start_point(0, 0);

        scenePixmapItem_[Stream0] = (parentWindow_->getScene(Stream0))->addPixmap(scenePixmap_[Stream0]);
        scenePixmapItem_[Stream1] = (parentWindow_->getScene(Stream1))->addPixmap(scenePixmap_[Stream1]);

        scenePixmapItem_[Stream0]->setPos(start_point);
        scenePixmapItem_[Stream1]->setPos(start_point);

        simpleModeSwitch_ = true;
    }

    void SimpleMode::dockWidgVisiChan0(bool changed) {
//        qDebug() << __FUNCTION__ << "()";
        dockWidgVisi_[Stream0] = changed;
    }

    void SimpleMode::dockWidgVisiChan1(bool changed) {
//        qDebug() << __FUNCTION__ << "()";
        dockWidgVisi_[Stream1] = changed;
    }


    /*void SimpleMode::startSimpleModeLocSock() {
        bigSarImgCircDataBufStream0_.resetBuffer();
        bigSarImgDireDataBufStream0_.resetBuffer();
        QPoint start_point( 0, 0 );

        scenePixmapItem_[Stream0] = (parentWindow_->getScene(Stream0))->addPixmap(scenePixmap_[Stream0]);

        scenePixmapItem_[Stream0]->setPos((start_point));

        simpleModeSwitch_ = true;
    }


    void SimpleMode::drawSimpleModeNetwork() {
            (parentWindow_->getView(Stream0))->repaint(parentWindow_->geometry().x(),
                                                       parentWindow_->geometry().y(),
                                                       parentWindow_->geometry().width(),
                                                       parentWindow_->geometry().height());

        scenePixmapItem_[Stream0]->setPixmap(scenePixmap_[Stream0]);

        drawLines();

    }


    void SimpleMode::drawSimpleModeLocSock() {
//        qDebug() << __FUNCTION__ << "()";

        QTime t;
        t.start();

        scenePixmapItem_[Stream0]->setPixmap(scenePixmap_[Stream0]);

        drawLines();

            (parentWindow_->getView(Stream0))->repaint(parentWindow_->geometry().x(),
                                                       parentWindow_->geometry().y(),
                                                       parentWindow_->geometry().width(),
                                                       parentWindow_->geometry().height() );

        qDebug("drawSimpleModeLocSock() tooks %i ms", t.elapsed());

    }


    void SimpleMode::drawSimpleModeLocSock(QImage img) {
//        QTime t;
//        t.start();

        QSize size = img.size();

        if(size != scenePixmap_[Stream0].size()) {
            qDebug() << "New size of scene pixmap :" << size.width() << "X" << size.height();
            parentWindow_->getScene(Stream0)->setSceneRect(0, 0, size.width(), size.height());
            parentWindow_->getView(Stream0)->fitInView(parentWindow_->getScene(Stream0)->sceneRect(),
                                                       Qt::IgnoreAspectRatio);
            this->scenePixmap_[Stream0] = this->scenePixmap_[Stream0].scaled(size.width(), size.height(),
                                                                             Qt::IgnoreAspectRatio,
                                                                             Qt::FastTransformation);
            this->image32_[Stream0] = this->image32_[Stream0].scaled(size.width(), size.height(),
                                                                     Qt::IgnoreAspectRatio,
                                                                     Qt::FastTransformation);
        }
        scenePixmapItem_[Stream0]->setPixmap(scenePixmap_[Stream0]);

        image32_[Stream0] = img;

            (parentWindow_->getView(Stream0))->repaint(parentWindow_->geometry().x(),
                                                       parentWindow_->geometry().y(),
                                                       parentWindow_->geometry().width(),
                                                       parentWindow_->geometry().height());

//        qDebug("drawSimpleModeLocSock(QImage img) tooks %i ms", t.elapsed());

    }*/


    void SimpleMode::drawSimpleModeUdpStream0(QImage img) {
//        qDebug() << __FUNCTION__ << "()";
        if(dockWidgVisi_[Stream0]) {
            QSize size = img.size();

            if(size != scenePixmap_[Stream0].size() && !scenePixmap_[Stream0].isNull() && !image32_[Stream0].isNull()) {
                qDebug() << "[stream 0] New size of scene pixmap :" << size.width() << "X" << size.height();
                parentWindow_->getScene(Stream0)->setSceneRect(0, 0, size.width(), size.height());
                parentWindow_->getView(Stream0)->fitInView(parentWindow_->getScene(Stream0)->sceneRect(),
                                                           Qt::IgnoreAspectRatio);
                scenePixmap_[Stream0] = scenePixmap_[Stream0].scaled(size.width(), size.height(),
                                                                     Qt::IgnoreAspectRatio,
                                                                     Qt::FastTransformation);
                image32_[Stream0] = image32_[Stream0].scaled(size.width(), size.height(),
                                                             Qt::IgnoreAspectRatio,
                                                             Qt::FastTransformation);
            }

            if(scenePixmapItem_[Stream0]) {
                scenePixmapItem_[Stream0]->setPixmap(scenePixmap_[Stream0]);
            }

            image32_[Stream0] = img; // shallow copy
            //        image32_[Stream0] = img.copy(); // TODO nizej od img a nie image32_...

            scenePixmap_[Stream0].convertFromImage(image32_[Stream0]);

//            parentWindow_->update();
            (parentWindow_->getView(Stream0))->repaint(parentWindow_->geometry().x(),
                                                       parentWindow_->geometry().y(),
                                                       parentWindow_->geometry().width(),
                                                       parentWindow_->geometry().height());
            //            parentWindow_->getPtrStreamDockWidget(Stream0)->repaint();
            //            parentWindow_->getPtrStreamDockWidget(Stream0)->widget()->repaint();
        }
    }


    void SimpleMode::drawSimpleModeUdpStream1(QImage img) {
//        qDebug() << __FUNCTION__ << "()";
        if(dockWidgVisi_[Stream1]) {
            QSize size = img.size();
            if(size != scenePixmap_[Stream1].size() && !scenePixmap_[Stream1].isNull()  && !image32_[Stream1].isNull()) {
                qDebug() << "[stream 1] New size of scene pixmap :" << size.width() << "X" << size.height();
                parentWindow_->getScene(Stream1)->setSceneRect(0, 0, size.width(), size.height());
                parentWindow_->getView(Stream1)->fitInView(parentWindow_->getScene(Stream1)->sceneRect(),
                                                           Qt::IgnoreAspectRatio);
                scenePixmap_[Stream1] = scenePixmap_[Stream1].scaled(size.width(), size.height(),
                                                                     Qt::IgnoreAspectRatio,
                                                                     Qt::FastTransformation);
                image32_[Stream1] = image32_[Stream1].scaled(size.width(), size.height(),
                                                             Qt::IgnoreAspectRatio,
                                                             Qt::FastTransformation);
            }

            if(scenePixmapItem_[Stream1]) {
                scenePixmapItem_[Stream1]->setPixmap(scenePixmap_[Stream1]);
            }

            image32_[Stream1] = img; // shallow copy
            //        image32_[Stream1] = img.copy();

            scenePixmap_[Stream1].convertFromImage(image32_[Stream1]);

//            parentWindow_->update();
            (parentWindow_->getView(Stream1))->repaint(parentWindow_->geometry().x(),
                                                       parentWindow_->geometry().y(),
                                                       parentWindow_->geometry().width(),
                                                       parentWindow_->geometry().height());
            //            parentWindow_->getPtrStreamDockWidget(Stream1)->repaint();
            //            parentWindow_->getPtrStreamDockWidget(Stream0)->widget()->repaint();
        }
    }


    /*int SimpleMode::drawLines() {
        qDebug() << __FUNCTION__ << "()";
//        t1 = clock();
        // direct buffer from circural
//        sarImgCircDataBuf_.copyToDirectBuffer(sarImgDireDataBuf_);
        bigSarImgCircDataBufStream0_.copyToDirectBufferWithLap(bigSarImgDireDataBufStream0_);

        const int most_actual_line = (NR_LINES_PER_SCREEN - 1);
        int x_buf = (NR_LINES_PER_SCREEN - 1);


        // setting pixels value
        uchar* p_bits = image32_[Stream0].bits(); // returns a pointer to the first pixel data
        int n_bytes_per_line = image32_[Stream0].bytesPerLine(); // returns the number of bytes per image scanline

        for(int y_img = 0; y_img < NR_LINES_PER_SCREEN; y_img++){
            for(int x_img = 0; x_img < NR_PIX_PER_LINE; x_img++) {
                uchar * scan_line = p_bits+y_img*n_bytes_per_line;
                ((uint *)scan_line)[x_img] = palette_->at(bigSarImgDireDataBufStream0_.getBuffer()[x_buf].pixels[x_img]);
            }
            x_buf--;
        }
        parentWindow_->setPlatParams(bigSarImgDireDataBufStream0_.getBuffer()[most_actual_line].sarImageLineHeader.lat_plat,
                                     bigSarImgDireDataBufStream0_.getBuffer()[most_actual_line].sarImageLineHeader.lon_plat,
                                     bigSarImgDireDataBufStream0_.getBuffer()[most_actual_line].sarImageLineHeader.alt_sea,
                                     bigSarImgDireDataBufStream0_.getBuffer()[most_actual_line].sarImageLineHeader.head,
                                     bigSarImgDireDataBufStream0_.getBuffer()[most_actual_line].sarImageLineHeader.pitch,
                                     bigSarImgDireDataBufStream0_.getBuffer()[most_actual_line].sarImageLineHeader.roll,
                                     bigSarImgDireDataBufStream0_.getBuffer()[most_actual_line].sarImageLineHeader.veloc,
                                     bigSarImgDireDataBufStream0_.getBuffer()[most_actual_line].sarImageLineHeader.flags,
                                     bigSarImgDireDataBufStream0_.getBuffer()[most_actual_line].sarImageLineHeader.nr_range_cells);

//        t2 = clock();
//        qDebug( "setPixel: %.3f s\n", (float)(t2-t1)/(float)CLOCKS_PER_SEC);

        return 0;
    }*/
} // namespace SarViewer
