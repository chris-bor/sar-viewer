#ifndef STRIPSCANIMAGE_H
#define STRIPSCANIMAGE_H

#include "qcustomplot.h"
#include "SarImageDataBuffer.h"
#include "Palettes.h"
#include "StripScan.h"
#include "Config.h"

typedef struct SpecialPoint_ SpecialPoint;

namespace SarViewer {
    #pragma pack(1)
    typedef struct Pos_ {
        float xstart;
        float ystart;
        float xstop;
        float ystop;
    } Pos;
    #pragma pack()

    class StripScanImage : public QCPAbstractItem {
        Q_OBJECT
    public :
        StripScanImage(SpecialPoint *start_point, int nr_stream = -1, StripScan *strip_scan = 0, QCustomPlot *parent_plot = 0);
        virtual ~StripScanImage();

        // Getters :
        QImage image() const { return mImage; }
        Qt::AspectRatioMode aspectRatioMode() const { return mAspectRatioMode; }
        QPen pen() const { return mPen; }
        QPen selectedPen() const { return mSelectedPen; }

        // Setters :
        void setImage(const QImage &image);
        void setPen(const QPen &pen);
        void setSelectedPen(const QPen &pen);
        void setRedrawImage(bool val);
        void setScaled(bool val);
        void setCircBuffer(SarImageDataBuffer *circ_buff);

        // Non-property methods:
        virtual double selectTest(const QPointF &pos) const;

        QCPItemPosition * const topLeft;
        QCPItemPosition * const bottomRight;

    public slots :
        void drawStripScanImage(int most_actual_line);
        void setNewSarSize(int width, int height);

    protected :
        virtual void draw(QCPPainter *painter);
        void imageCorrection();
        void resamplTrajectory();
        void calcAntennaDirection();
        void calcMinMaxPositions();
        void calcImageSize();
        void calcSarImage();
        void calcMean();

        // Helper functions :
        QRect getFinalRect(bool *flippedHorz=0, bool *flippedVert=0) const;
        QPen mainPen() const;
        void setImageUnvisible(); // Initialize alpha channel to 0 (unvisible)

        QImage mImage;
        QImage mScaledImage;
        bool mScaled;//????
        Qt::AspectRatioMode mAspectRatioMode;
        QPen mPen, mSelectedPen;
        bool mRedrawImage;
        SarImageDataBuffer *bigSarImgCircDataBuf_; // Pointer to the main buffer of data
        int dataSize_;
        sar_image_line_header *headersBuffer_;
        sar_image_line_header *lastHeaderPtr_;
        quint8 *pixelsBuffer_; // Raw sar image
        float *zBuffer_; // Flat eart position X and Y are stored at dataBuffer_
        int nrStream_; // Number of managed stream
        float r_min_; // todo: rMin_
        float r_max_; // todo: rMax_
        float cell_size_; // todo: cellSize_
        int nrRangeCells_;
        int antDirRight_;
        Pos *pos_;
        Pos minPos_; // tu s¹ przecie¿ bnd ale wg danych headerów
        Pos maxPos_;
        int oryginalWidth_, oryginalHeight_;
        int newWidth_, newHeight_;
        UINT32 refreshCnt_; // Counter of refreshments
        int startIdx_; // Index where data for image are real, didn't derived from initialization process
        float minX_, minY_, maxX_, maxY_;
        float widthRatio_, heightRatio_; // ratio between new sar size and old
        int *vals; // Array of values of the image
        int *cnts; // Array of counters
        QVector<QRgb> *palette_; // Palette
        SpecialPoint *startPoint_; // Reference geodetic coordinates to point (0,0)
        SarViewer::StripScan *stripScan_;
        StripScanBndType imgBnd_; // to s¹ bnd ale wg StripScan
    };
} // namespace SarViewer
#endif // STRIPSCANIMAGE_H
