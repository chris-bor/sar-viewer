#ifndef SARIMAGEDIRECTDATABUFFER_H
#define SARIMAGEDIRECTDATABUFFER_H

#include <QObject>
#include "Line.h"

namespace SarViewer {
    class SarImageDirectDataBuffer : public QObject {
        Q_OBJECT
    public:
        SarImageDirectDataBuffer();
        ~SarImageDirectDataBuffer();
        int setLine(Line *one_line);
        inline Line *getBuffer() const {
            return(this->linesBuffer_);
        }
        inline int getMostActualLine() const {
            return(this->mostActualLine_);
        }

    private:
        void resetBuffer();

        Line *linesBuffer_;
        int mostActualLine_;

    };
} // namespace SarViewer
#endif // SARIMAGEDIRECTDATABUFFER_H
