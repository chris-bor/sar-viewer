/// @file SarImageReader.h
/// @brief header class responsible for reading sar image from file
/// @author Krzysztof Borowiec
/*
#ifndef SARIMAGEREADER_H
#define SARIMAGEREADER_H

#include <QFile>
#include <QDataStream>
#include <QImage>
#include "sar_image.h"
#include "Line.h"
#include "SarImageDataBuffer.h"

namespace SarViewer
{
    const char FILE_NAME[] = "sar_image.bin";

    class SarImageReader : public QObject {
        Q_OBJECT
    public :
        SarImageReader();
        ~SarImageReader();
        void openSARFile(QString &file_name); // opens SAR data from file
//        int readOneLine(QList<Line> &line_data); // reads pixel values from file
        int readLines(SarImageDataBuffer *buff); // reads lines pixels from binary file depending on value of translation (SHIFT_VAL)

    signals :
        void endOfFile();

    private :
        QFile *file_;
        QDataStream inputFileStream_;
        qint64 streamPos_;
        sar_image_line_header sarImageLineHeader_;
    };

} // namespace SarViewer

#endif // SARIMAGEREADER_H
*/
