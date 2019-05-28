#include <QtCore>
#include <QtNetwork>
#include <winsock.h>
#include <winsock2.h>
#include "UdpReceiver.h"
#include "Debugging.h"
#include "Config.h"

extern ViewerConfig viewer_config;

namespace SarViewer {
    bool strip_scan_img_mux;
    extern QMutex sar_img_data_buff_mux[NrStreams];
    const int MAX_BUFF_UDP_SOC_SIZE = 20000000;
    const quint64 MAX_DATAGRAM_SIZE = 100000;
}

SarViewer::UdpReceiver::UdpReceiver(QString address, int port, int nr_stream, QObject *parent)  :
    QObject(parent), nrLines_(0), socket_(NULL), address_(QHostAddress(address)), port_(port), /*quit_(false),*/
    nrStream_(nr_stream), nrRangeCells_(0), nrLinesPerBlock_(0), nrLinToRefrStrScan_(0), sleepTime_(0),
    pendingDatagramSize_(0), nr_line(0), isVisible_(true), altitudeType_(AltGround) {
//#if DEBUG_CONST
    qDebug() << __FUNCTION__ << "()";
//#endif
}


SarViewer::UdpReceiver::~UdpReceiver() {
//#if DEBUG_CONST
    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
//#endif

//    mutex_.lock();
//    quit_ = true; // to tell run() to stop running as soon as possible
//    condition_.wakeOne(); // to wake up the thread if it's sleeping ( the thread is put to sleep when it has nothing to do )
//    mutex_.unlock();

//    quit();

//    wait();
    if(socket_) {
        socket_->abort();
        socket_->close();
        delete socket_;
        socket_ = NULL;
    }
}


void SarViewer::UdpReceiver::connectToServer() {
#if DEBUG_UDP_RECEIVER
    qDebug() << __FUNCTION__ << "()" << "[ Stream " << nrStream_ << "]";
#endif

    // Disable option
    emit changeAction(true, UDP);

    // Create socket
    socket_ = new QUdpSocket();
    socket_->bind(QHostAddress(address_), port_);

    // Setting size of buffer of UDP socket
    if (::setsockopt(socket_->socketDescriptor(), SOL_SOCKET,
                     SO_RCVBUF, (char *)&MAX_BUFF_UDP_SOC_SIZE, sizeof(MAX_BUFF_UDP_SOC_SIZE)) == -1) {
        PR_ERROR("Cannot set size of buffer of the socket");// error!
    }

    // Resize the place for new data
    datagram_.resize(SarViewer::MAX_DATAGRAM_SIZE);

//    QObject::connect(socket_, SIGNAL(readyRead()),
//            this, SLOT(readPendingDatagrams()));
    QObject::connect(socket_, SIGNAL(readyRead()),
                     this, SLOT(readPendingDatagrams()), Qt::DirectConnection);
}


void SarViewer::UdpReceiver::setIsVisible(bool val) {
#if DEBUG_UDP_RECEIVER
    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
    isVisible_ = val;
}


void SarViewer::UdpReceiver::readPendingDatagrams() {
#if DEBUG_UDP_RECEIVER
    qDebug() << __FUNCTION__ << "()";
#endif

        Line line;

        int ile = 0;
        while (socket_->hasPendingDatagrams()) {
            pendingDatagramSize_ = socket_->pendingDatagramSize();

            socket_->readDatagram(datagram_.data(), pendingDatagramSize_, &sender_, &senderPort_);

            QDataStream data_stream(&datagram_, QIODevice::ReadOnly);
            data_stream.setVersion(QDataStream::Qt_4_0);
            data_stream.setByteOrder(QDataStream::LittleEndian);

            int int_val;
            float float_val, alt_sea;
            double double_val;

            data_stream >> int_val;

            if(int_val != IMAGE_LINE_MAGIC) { // If magic id mismatch search new id at stream
                qDebug() << "ERROR : MAGIC ID MISMATCH. RECEIVED ID:" << int_val ;
                continue;
            }

            line.sarImageLineHeader.magic_id = int_val;
            data_stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
            data_stream >> double_val;
            line.sarImageLineHeader.lat_plat = double_val;
            data_stream >> double_val;
            line.sarImageLineHeader.lon_plat = double_val;
            data_stream >> double_val;
            line.sarImageLineHeader.lat_ref = double_val;
            data_stream >> double_val;
            line.sarImageLineHeader.lon_ref = double_val;
            data_stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            data_stream >> alt_sea;
            line.sarImageLineHeader.alt_sea = alt_sea;
            data_stream >> float_val;
            switch(altitudeType_){
            case Zero :
                line.sarImageLineHeader.alt_ground = 0;
                break;
            case AltGround :
                line.sarImageLineHeader.alt_ground = float_val;
                break;
            case AltSea :
                line.sarImageLineHeader.alt_ground = alt_sea;
                break;
            }
            data_stream >> float_val;
            line.sarImageLineHeader.cell_size = float_val;
            data_stream >> float_val;
            line.sarImageLineHeader.r_min = float_val;
            data_stream >> float_val;
            line.sarImageLineHeader.head = float_val;
            data_stream >> float_val;
            line.sarImageLineHeader.head_ref = float_val;
            data_stream >> float_val;
            line.sarImageLineHeader.pitch = float_val;
            data_stream >> float_val;
            line.sarImageLineHeader.roll = float_val;
            data_stream >> float_val;
            line.sarImageLineHeader.veloc = float_val;
            data_stream >> float_val;
            line.sarImageLineHeader.prf = float_val;
            data_stream >> int_val;
            line.sarImageLineHeader.flags = int_val;
            data_stream >> int_val;
            line.sarImageLineHeader.nr_range_cells = int_val;
            nrRangeCells_ = int_val;
            data_stream >> int_val;
            if(int_val - 1 != nr_line) {
                qDebug() << "[ Stream" << nrStream_ << "]" <<"Lost line" << int_val - 1 - nr_line << "while"
                         << ile << "lines" << "line_cnt" << int_val;
            }
            nr_line = int_val;
            line.sarImageLineHeader.line_cnt = int_val;
            nrLines_++;
            data_stream >> int_val;
            line.sarImageLineHeader.block_cnt = int_val;

            for(uint y = 0; y < nrRangeCells_; y++) {
                data_stream >> line.pixels[y];
            }

            SarViewer::sar_img_data_buff_mux[this->nrStream_].lock();
            if(dataBuff_->getBuffer()/* && !quit_*/) //
                dataBuff_->setLine(&line); // Set line to circural buffer
            SarViewer::sar_img_data_buff_mux[this->nrStream_].unlock();

            nrLinesPerBlock_++;
            nrLinToRefrStrScan_++;

            ile++;

            if(nrLinesPerBlock_ >= viewer_config.min_nr_lines_to_draw) {
//                qDebug("[ Stream %d ] Number of received lines : %d", nrStream_, nrLinesPerBlock_);
                sleepTime_ = nrLinesPerBlock_/line.sarImageLineHeader.prf * 1000; // [ms]
//                qDebug() << "TIME" << sleepTime_;
                if(isVisible_) {
                    emit drawSimpleMode(nrLinesPerBlock_, sleepTime_, nrRangeCells_,
                                        dataBuff_->getMostActualLine(), dataBuff_->getStartPresentLine()); // received new data so emit signal
                }
                emit setPlatParams(line.sarImageLineHeader.lat_plat, line.sarImageLineHeader.lon_plat,
                                   line.sarImageLineHeader.alt_ground, line.sarImageLineHeader.head,
                                   line.sarImageLineHeader.pitch, line.sarImageLineHeader.roll,
                                   line.sarImageLineHeader.veloc, line.sarImageLineHeader.flags,
                                   nrRangeCells_);
                nrLinesPerBlock_ = 0;
            }

            // Refresh strip scan frame
            if(nrLinToRefrStrScan_ == viewer_config.nr_lin_to_ref_str_scan && nrStream_ == 0) {
                emit addTrackPoint(line.sarImageLineHeader, dataBuff_->getMostActualLine());
//                if(SarViewer::strip_scan_img_mux == 0) {
//                }
                nrLinToRefrStrScan_ = 0;
            }
        }
//        qDebug("Strumien %d readPendingdata() tooks %i ms", nrStream_, t.elapsed());
//        qDebug() << "Ile" << ile;
}
