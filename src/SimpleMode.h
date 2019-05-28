/// @file SimpleMode.h
/// @brief class managed simple mode viewing
/// @author Krzysztof Borowiec

#ifndef SIMPLEMODE_H
#define SIMPLEMODE_H

#include <QtGui>
#include "View.h"
#include "SarImageReader.h"
#include "Line.h"
#include "SarImageDataBuffer.h"
#include "Palettes.h"
#include "SimpleModeDrawer.h"
#include "Config.h"

namespace SarViewer {
    class MainWindow;
}

namespace SarViewer {
    class SimpleMode : public QObject {
        Q_OBJECT
    public:
        SimpleMode( QMainWindow *mainWindow );
        virtual ~SimpleMode();
        /*inline  QImage *getSceneImage() {
            return(sceneImage_);
        }*/
        QImage *getImage32(int nr_stream) {
            return(&image32_[nr_stream]);
        }
        QPixmap *getScenePixmap(int nr_stream) {
            return(&scenePixmap_[nr_stream]);
        }
        /*inline QImage *getImageIdx8() {
            return(&imageIdx8_);
        }*/
        void setSimpleModeSwitch(const bool val) {
            this->simpleModeSwitch_ = val;
            return;
        }
        bool getSimpleModeSwitch() const {
            return(this->simpleModeSwitch_);
        }
        /*inline SarImageDataBuffer *getSarImgDataBuf(int nr_stream) {
            if(nr_stream == Stream0)
                return(&sarImgCircDataBufStream0_);
            else
                return(&sarImgCircDataBufStream1_);
        }*/
        SarImageDataBuffer *getBigSarImgDataBuf(int nr_stream) {
            if(nr_stream == Stream0)
                return(&bigSarImgCircDataBufStream0_);
            else
                return(&bigSarImgCircDataBufStream1_);
        }
        SimpleModeDrawer *simpleModeDrawer(int nr_stream) {
//            return(simpleModeDrawer_[nr_stream]);
            if(nr_stream == Stream0)
                return(&simpleModeDrawerStream0_);
            else
                return(&simpleModeDrawerStream1_);
        }
        /*int drawLines(); // draws lines of pixels from list buffer*/

    public slots :
        void startSimpleModeNetwork();
        void dockWidgVisiChan0(bool changed);
        void dockWidgVisiChan1(bool changed);

    private slots :
        /*void replay();
        void replayDraw();
        void startSimpleModeLocSock();
        void drawSimpleModeNetwork(); // to draw simple mode for data received from network
        void drawSimpleModeLocSock(); // to draw simple mode for data received from local socket
        void drawSimpleModeLocSock(QImage img); // to draw simple mode for data received from local socket*/
        void drawSimpleModeUdpStream0(QImage img); // to draw simple mode for data received by UDP
        void drawSimpleModeUdpStream1(QImage img); // to draw simple mode for data received by UDP

    private:
        SarViewer::MainWindow *parentWindow_;

        /*QImage *sceneImage_; // Image which id holded by scene. Main Paint Device to draw on it
        QImage imageIdx8_; // Image to setting pixels after reading from file*/
        QImage image32_[NrStreams];
        QPixmap scenePixmap_[NrStreams]; // Pixmap which is holded by scene
        QGraphicsPixmapItem *scenePixmapItem_[NrStreams]; // Pixmap item whose holds scene pixmap and enable painting on it
        QSize scenePixmapSize_; // Size of pixmap on the scene
        /*QSize imageIdx8Size_;*/
        QTimer *replayTimer_; // Timer for counting time at direct mode viewing
        QTimer *simpleModeTimer_; // Timer for counting time at direct mode viewing
        bool simpleModeSwitch_; // On/Off direct mode
        QString imageFileName_; // Name of image file

        QVector<QRgb> *palette_;

        /*SarImageReader sarImageReader_;
        SarImageDataBuffer sarImgCircDataBufStream0_;
        SarImageDataBuffer sarImgCircDataBufStream1_;
        SarImageDataBuffer sarImgDireDataBufStream0_;
        SarImageDataBuffer sarImgDireDataBufStream1_;*/
        SarImageDataBuffer bigSarImgCircDataBufStream0_;
        SarImageDataBuffer bigSarImgCircDataBufStream1_;
        /*SarImageDataBuffer bigSarImgDireDataBufStream0_;
        SarImageDataBuffer bigSarImgDireDataBufStream1_;*/
//        SimpleModeDrawer simpleModeDrawerStream0_;
        SimpleModeDrawer simpleModeDrawerStream0_;
        SimpleModeDrawer simpleModeDrawerStream1_;

        bool dockWidgVisi_[NrStreams];
    };

} // namespace SarViwer
#endif // SIMPLEMODE_H
