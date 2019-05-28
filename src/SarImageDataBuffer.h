/// @file SarImageDataBuffer.h
/// @brief class managing circural buffer
/// @author Krzysztof Borowiec

#ifndef SARIMAGEDATABUFFER_H
#define SARIMAGEDATABUFFER_H

#include <qobject.h>
#include "Line.h"

namespace SarViewer {
    class SarImageDataBuffer : public QObject {
        Q_OBJECT
    public:
        SarImageDataBuffer(); // size equal to MAX_LINES
        SarImageDataBuffer(int, int nr_stream = -1); // the ability to change the size
        ~SarImageDataBuffer();
        int setLine(Line *one_line);
        Line *getBuffer() const { return(this->linesBuffer_); }
        int size() const { return(this->SIZE_); }
        int getMostActualLine() const { return(this->mostActualLine_); }
        int getStartPresentLine() const { return(this->startPresent_); }
        int getSize() const { return(this->SIZE_); }
        void copyToDirectBuffer(SarImageDataBuffer &direct_buff); // copies fromn this circural buffer to direct buffer
        void copyToDirectBufferWithLap(SarImageDataBuffer &direct_buff); // copies from this big circular buffer to direct buffer with lap for shifting image during showing on the screen
        void copyToDirectBufferWithLap(SarImageDataBuffer &direct_buff, int curr_line, int curr_start);
        void resetBuffer(); // sets zeros to pixel values and some line values
        void resetBufferPix(); // only sets zeros to pixel values
        void copyNewScanStrip(SarImageDataBuffer &strip_buf, int most_actual_line);
        void copyNewScanStrip(sar_image_line_header *headers, quint8 *pixels, int most_actual_line, int size);
        quint8 &getPixel(const int &x, const int &y);

    private:
        const int SIZE_;
        Line *linesBuffer_;
        int mostActualLine_;
        int startPresent_; // index to line at buffer where is start of presence at screen part
        int nrStream_; // Number of managed stream
    };
} // namespace SarViewer
#endif // SARIMAGEDATABUFFER_H
