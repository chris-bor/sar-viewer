#ifndef SIMPLEMODEDRAWERTHREAD_H
#define SIMPLEMODEDRAWERTHREAD_H

#include <QImage>
#include <QSize>
#include <QMainWindow>
#include <QPainter>
#include "qglobal.h"
#include "SarImageDataBuffer.h"
#include "Palettes.h"
#include "PlotSettings.h"

namespace SarViewer {
    class MainWindow;
}

namespace SarViewer {

    class SimpleModeDrawer : public QObject {
        Q_OBJECT

    public :
        SimpleModeDrawer(SarImageDataBuffer *buff = 0, int nr_stream = -1, QWidget *parent = 0);
        virtual ~SimpleModeDrawer();

    public slots :
        void setPalette(int palette_type);
        void setSizeBackImg(QSize new_size);
        void setZoom();
        void showColorbar(bool toggled);
        void checkQuit(); // Called firstly to check if images are allocated

    signals :
        void imageReady(QImage img);
        void quitApp();

    private slots :
        void startDrawing(int nr_lines, int sleep_time, int nr_range_cells, int curr_line, int curr_start); // (arg. inform how many lines at new data block)

    private :
        void run();
        void createColorbar();
        void plotColorbar();
        void copyData();
        void setStartDrawing();
        void resizeSarImage();
        void resizeBackImage();
        void drawBackImage();

        volatile bool quit_; // variable necessary to stop thread
        SarImageDataBuffer *bigSarImgCircDataBuf_;
        SarImageDataBuffer bigSarImgDireDataBuf_;
        int nrLines_;
        int sleepTime_;
        QVector<QRgb> *palette_; // Palette
        int oldNrRangeCells_; // Old number of range cells
        float oldRMin_; // Old number of minimum radius
        bool antDirRight_;
        UINT32 flags_;
        float prf_;
        float cellSize_;
        bool resizeSarImg_; // If old number of range cells differs from new then rescale image to new size
        bool resizeBackImg_; // True if main's window size changed
        bool refresh_; // If true all images are redraw if there is no data
        bool zoomChanged_; // If zoom changed its true
        bool noMemmoveShift_; // If zoom is big (smal size of the SAR img) then draw with NO memmove() function
        int currZoom_; // Current zoom
        QSize sizeBackImg_;
        QImage *imgSar_; // Image where is drawing new image
        QImage *imgBack_; // Image with SAR image and scale
        QImage *imgScale_; // Image with scaled axies
        QRectF sourceRect_;
        QRectF targetRect_;
        int nrStream_; // Number of managed stream
        PaletteType paletteType_; // Type of palette
        SarViewer::PlotSettings plotSett_;
        QPainter painter_;
        QPen penBlack_;
        int xBufStart_; // Pointer where start drawing at buffer
        int xBuf_; // Points line during shift
        int colorbarWidth_; // Width of the color bar
        bool colorbar_;
        QImage *colorbarImg_;
        int currLine_;
        int currStart_;
        int rest_;
        /*int heightShiftDiff_; // difference between height of SAR image and shift (!= 0 if height mod shift != 0)*/
        uint shift_;
        QFont xAxisFont_;
        QFont yAxisFont_;
        QFont labelFont_;
    };
} // namespace SarViewer
#endif // SIMPLEMODEDRAWERTHREAD_H
