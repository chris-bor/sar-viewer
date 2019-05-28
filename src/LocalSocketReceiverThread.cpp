#include <QtNetwork>
#include <QMessageBox>
#include <QtCore>
#include <math.h>
#include "LocalSocketConnectWindow.h"
#include "Config.h"
#include "Debugging.h"
#include "LocalSocketReceiverThread.h"

namespace SarViewer {
    extern QMutex sar_img_data_buff_mux[];

    LocalSocketReceiverThread::LocalSocketReceiverThread(QObject *parent) :
        QThread(parent), bytesAvailable_(0), nrLines_(0), quit_(false) {
//#if DEBUG_CONST
        qDebug() << __FUNCTION__ << "()";
//#endif

        socket_.setParent(this);
        socket_.setObjectName("SAR_SOCKET");
        socket_.setReadBufferSize(0); // infinite

        connect(&socket_, SIGNAL(connected()), this, SLOT(connectedToServer())); // connection signal emited when connection occured to slot which inform as that this connection ended successfully
        connect(&socket_, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(displayError(QLocalSocket::LocalSocketError)));
    }


    LocalSocketReceiverThread::~LocalSocketReceiverThread() {
//#if DEBUG_DESTR
        qDebug() << __FUNCTION__ << "()";
//#endif
        mutex_.lock();
        quit_ = true; // to tell run() to stop running as soon as possible
        condition_.wakeOne(); // to wake up the thread if it's sleeping ( the thread is put to sleep when it has nothing to do )
        mutex_.unlock();

        wait();
    }


    void LocalSocketReceiverThread::run() {
#if DEBUG_LOC_SOC_REC_THR
//        qDebug() << __FUNCTION__ << "()";
#endif
        const int timeout = 1 * 1000; // [ms]

        while (!quit_) { // infinite loop
            mutex_.lock();
            static int liczba_while = 0;
            Line line;
            static uint nr_range_cells = 0;
            static uint bytes_available = 0;
            static uint nr_lines_per_block = 0;
            static int sleep_time = 0;

            // Check if socket connected
//            if(socket_.state() == QLocalSocket::UnconnectedState) {
//                sleep(1);
//                this->connectToServer();
//            }

            // HEADER
            while (!quit_ && (bytes_available = socket_.bytesAvailable()) <= SIZE_OF_IMG_HEADER + nr_range_cells * sizeof(quint8)) { // 1342+2  SIZE_OF_IMG_HEADER
                if (!socket_.waitForReadyRead(timeout)) {
                    PR_LINE_MSG("ERROR : Socket do not ready to read data. Pleace try to reconnect.");
                    this->disconnectFromServer();
                    quit_ = true;
                    continue;
                }
                else {
//                    PR_LINE_MSG("Waiting for data...");
                    msleep(50);
                }
            }

            msleep(500); // min 500
            liczba_while = 0;

            nr_lines_per_block = 0;

            while(!quit_ && bytes_available > SIZE_OF_IMG_HEADER + nr_range_cells * sizeof(quint8)) {
                liczba_while++;
                if(bytes_available > SIZE_OF_IMG_HEADER) {
                    QByteArray header_arr  = socket_.read((qint64)SIZE_OF_IMG_HEADER);
                    bytes_available -= SIZE_OF_IMG_HEADER;

                    QDataStream head_str(&header_arr, QIODevice::ReadOnly); // Setting setream of data
                    head_str.setVersion(QDataStream::Qt_4_0);
                    head_str.setByteOrder(QDataStream::LittleEndian);

                    // Read header from data stream
                    int int_val;

                    head_str >> int_val;
                    if(int_val != IMAGE_LINE_MAGIC) { // If magic id mismatch search new id at stream
                        qDebug() << "ERROR : MAGIC ID MISMATCH. RECEIVED ID:" << int_val ;
                        // TO DO : if(nr_range_clls) bo else powinien poszukac mimo bad id ile jest pixeli czyli odczytywac dalej header i wywalic pixeli ile znalazl
                        QByteArray loose_data = socket_.read((qint64)(nr_range_cells * sizeof(quint8)));
                        bytes_available -= nr_range_cells * sizeof(quint8);
                        continue;
                    }

                    float floatVal_;
                    double doubleVal_;

                    line.sarImageLineHeader.magic_id = int_val;
                    head_str.setFloatingPointPrecision(QDataStream::DoublePrecision);
                    head_str >> doubleVal_;
                    line.sarImageLineHeader.lat_plat = doubleVal_;
                    head_str >> doubleVal_;
                    line.sarImageLineHeader.lon_plat = doubleVal_;
                    head_str >> doubleVal_;
                    line.sarImageLineHeader.lat_ref = doubleVal_;
                    head_str >> doubleVal_;
                    line.sarImageLineHeader.lon_ref = doubleVal_;
                    head_str.setFloatingPointPrecision(QDataStream::SinglePrecision);
                    head_str >> floatVal_;
                    line.sarImageLineHeader.alt_sea = floatVal_;
                    head_str >> floatVal_;
                    line.sarImageLineHeader.alt_ground = floatVal_;
                    head_str >> floatVal_;
                    line.sarImageLineHeader.cell_size = floatVal_;
                    head_str >> floatVal_;
                    line.sarImageLineHeader.r_min = floatVal_;
                    head_str >> floatVal_;
                    line.sarImageLineHeader.head = floatVal_;
                    head_str >> floatVal_;
                    line.sarImageLineHeader.head_ref = floatVal_;
                    head_str >> floatVal_;
                    line.sarImageLineHeader.pitch = floatVal_;
                    head_str >> floatVal_;
                    line.sarImageLineHeader.roll = floatVal_;
                    head_str >> floatVal_;
                    line.sarImageLineHeader.veloc = floatVal_;
                    head_str >> floatVal_;
                    line.sarImageLineHeader.prf = floatVal_;
                    head_str >> int_val;
                    line.sarImageLineHeader.flags = int_val;
                    head_str >> int_val;
                    line.sarImageLineHeader.nr_range_cells = int_val;
                    nr_range_cells = int_val;
                    head_str >> int_val;
                    line.sarImageLineHeader.line_cnt = nrLines_;
                    nrLines_++;
                    head_str >> int_val;
                    line.sarImageLineHeader.block_cnt = int_val;

                }

                // PIXELS
                if(bytes_available > nr_range_cells * sizeof(quint8)) {
                    QByteArray pixels_arr  = socket_.read((qint64)(nr_range_cells * sizeof(quint8)));
                    bytes_available -= nr_range_cells * sizeof(quint8);

                    QDataStream pixels_str(&pixels_arr, QIODevice::ReadOnly); // Setting setream of data
                    pixels_str.setVersion(QDataStream::Qt_4_0);
                    pixels_str.setByteOrder(QDataStream::LittleEndian);

                    for(uint y = 0; y < nr_range_cells; y++) {
                        pixels_str >> line.pixels[y];
                    }

                    sar_img_data_buff_mux[Stream0].lock();
                    dataBuff_->setLine(&line); // Set line to circural buffer
                    sar_img_data_buff_mux[Stream0].unlock();
                }
                nr_lines_per_block++;
            }

            if(!quit_ && nr_lines_per_block) {
                PR_LINE_VAL(nr_lines_per_block); // liczba_while sleep_time line.sarImageLineHeader.prf
                sleep_time = nr_lines_per_block/line.sarImageLineHeader.prf * 1000;
                emit drawSimpleMode(nr_lines_per_block, sleep_time,
                                    line.sarImageLineHeader.nr_range_cells); // received new data so emit signal
                emit setPlatParams(line.sarImageLineHeader.lat_plat, line.sarImageLineHeader.lon_plat,
                                   line.sarImageLineHeader.head, line.sarImageLineHeader.pitch,
                                   line.sarImageLineHeader.roll, line.sarImageLineHeader.alt_sea,
                                   line.sarImageLineHeader.veloc, line.sarImageLineHeader.flags);
            }
            mutex_.unlock();
//            PR_LINE_VAL(bytes_available);
        } // end of infinite loop
    } // function


    void LocalSocketReceiverThread::connectToServer() {
#if DEBUG_LOC_SOC_REC_THR
        qDebug() << __FUNCTION__ << "()";
#endif
        socket_.abort();
        socket_.connectToServer(QString("\\\\.\\pipe\\SAR_PIPE_0"), QIODevice::ReadOnly | QIODevice::Unbuffered);
    }


    void LocalSocketReceiverThread::connectedToServer() {
#if DEBUG_LOC_SOC_REC_THR
        qDebug() << __FUNCTION__ << "()";
#endif
//#if DEBUG_LOC_SERV_NAME
        qDebug() << "Named pipe:" << socket_.fullServerName() << "connected.";
//#endif
        this->start(InheritPriority);

        //        QMessageBox::information( this, tr( "Connection status" ), tr( "Connected with server name : %1").arg(socket_->serverName())));
    }


    void LocalSocketReceiverThread::disconnectFromServer() {
#if DEBUG_LOC_SOC_REC_THR
        qDebug() << __FUNCTION__ << "()";
#endif
        socket_.close();
    }


    void LocalSocketReceiverThread::readData() {
#if DEBUG_LOC_SOC_REC_THR
        qDebug() << __FUNCTION__ << "()";
#endif
    }


    void LocalSocketReceiverThread::displayError(QLocalSocket::LocalSocketError socketError) {
#if DEBUG_LOC_SOC_REC_THR
        qDebug() << __FUNCTION__ << "()";
#endif
        switch (socketError)  {
        case QLocalSocket::ServerNotFoundError:
            QMessageBox::information(qobject_cast<QWidget *>(this), tr("SAR client"),
                                     tr("The host was not found. Please check "
                                        "settings."));
            break;
        case QLocalSocket::ConnectionRefusedError:
            QMessageBox::information(qobject_cast<QWidget *>(this), tr("SAR client"),
                                     tr("The connection was refused by the peer. "
                                        "Make sure the server is running, "
                                        "and check that "
                                        "settings are correct."));
            break;
        case QLocalSocket::PeerClosedError:
            QMessageBox::information(qobject_cast<QWidget *>(this), tr("SAR client"),
                                     tr("Peer closed."));
            break;
        default:
            this->disconnectFromServer();
            QMessageBox::information(qobject_cast<QWidget *>(this), tr("SAR client"),
                                     tr("The following error occurred: %1.")
                                     .arg(socket_.errorString()));
        }
    }
}// namespace SarViewer
