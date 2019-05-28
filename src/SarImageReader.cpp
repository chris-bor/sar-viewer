/*
#include <QDebug>
#include <QMessageBox>
#include <QObject>
#include "SarImageReader.h"
#include "Config.h"
#include "sar_image.h"
#include "Debugging.h"

extern ViewerConfig viewer_config;

namespace SarViewer
{
    SarImageReader::SarImageReader() :
        QObject(), file_(0) {
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF SarImageReader\n";
#endif
    }

    SarImageReader::~SarImageReader() {
#if DEBUG_DESTR
        qDebug() << "DESTRUCTOR OF SarImageReader\n";
#endif
        if(file_)
            file_->close();
    }


    void SarImageReader::openSARFile(QString &file_name) {
        file_ = new QFile(file_name);
        if ( file_->open(QIODevice::ReadOnly)) {
            streamPos_ = 0;
        }
        else {
            streamPos_ = -1;
            QMessageBox::information( 0, QObject::tr( "Error reading file" ), QObject::tr( "Could not open file with SAR imege") );
        }
    }


    int SarImageReader::readLines(SarImageDataBuffer *buff) { // 0.063
        Line one_line;
        int int_data;
        double double_data;
        float float_data;

        // reading header
        inputFileStream_.setDevice(file_);
        inputFileStream_.device()->seek(streamPos_);
        inputFileStream_.setByteOrder(QDataStream::LittleEndian);

        for(uint k = 0; k < viewer_config.shift_val; k++) {

        if(inputFileStream_.atEnd()) {
            qDebug() << "EOF\n";
            streamPos_ = 0;
            return 1;
        }


            // reading header
            inputFileStream_.setFloatingPointPrecision(QDataStream::DoublePrecision);
            inputFileStream_ >> int_data;
            one_line.sarImageLineHeader.magic_id = int_data;
            inputFileStream_ >> double_data;
            one_line.sarImageLineHeader.lat_plat = double_data;
            inputFileStream_ >> double_data;
            one_line.sarImageLineHeader.lon_plat = double_data;
            inputFileStream_ >> double_data;
            one_line.sarImageLineHeader.lat_ref = double_data;
            inputFileStream_ >> double_data;
            one_line.sarImageLineHeader.lon_ref = double_data;
            inputFileStream_.setFloatingPointPrecision(QDataStream::SinglePrecision);
            inputFileStream_ >> float_data;
            one_line.sarImageLineHeader.alt_sea = float_data;
            inputFileStream_ >> float_data;
            one_line.sarImageLineHeader.alt_ground = float_data;
            inputFileStream_ >>float_data;
            one_line.sarImageLineHeader.cell_size = float_data;
            inputFileStream_ >> float_data;
            one_line.sarImageLineHeader.r_min = float_data;
            inputFileStream_ >> float_data;
            one_line.sarImageLineHeader.head = float_data;
            inputFileStream_ >> float_data;
            one_line.sarImageLineHeader.head_ref = float_data;
            inputFileStream_ >> float_data;
            one_line.sarImageLineHeader.pitch = float_data;
            inputFileStream_ >> float_data;
            one_line.sarImageLineHeader.roll = float_data;
            inputFileStream_ >> float_data;
            one_line.sarImageLineHeader.veloc = float_data;
            inputFileStream_ >> float_data;
            one_line.sarImageLineHeader.prf = float_data;

            inputFileStream_ >> int_data;
            one_line.sarImageLineHeader.flags = int_data;
            inputFileStream_ >> int_data;
            one_line.sarImageLineHeader.nr_range_cells = int_data;
            inputFileStream_ >> int_data;
            one_line.sarImageLineHeader.line_cnt = int_data;
            inputFileStream_ >> int_data;
            one_line.sarImageLineHeader.block_cnt = int_data;

            // reading pixels
            for(uint y = 0; y < one_line.sarImageLineHeader.nr_range_cells; y++) {
                inputFileStream_>>one_line.pixels[y];
            }

            buff->setLine(&one_line);
        }
        streamPos_ = inputFileStream_.device()->pos();
        return 0;
    }
} // namespace SarViewer
*/
