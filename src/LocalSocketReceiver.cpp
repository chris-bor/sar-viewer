#include <QtNetwork>
#include <QMessageBox>
#include "LocalSocketReceiver.h"
#include "LocalSocketConnectWindow.h"
#include "Config.h"
#include "Debugging.h"

namespace SarViewer {
    LocalSocketReceiver::LocalSocketReceiver(QWidget *parent, SarImageDataBuffer *buff) : QWidget(parent){
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF LocalSocketReceiver\n";
#endif
        socket_ = new QLocalSocket(this);

        connect(socket_, SIGNAL(readyRead()), this, SLOT(readData()));
        connect(socket_, SIGNAL(connected()), this, SLOT(connectedToServer())); // connection signal emited when connection occured to slot which inform as that this connection ended successfully
        connect(socket_, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(displayError(QLocalSocket::LocalSocketError)));

        if(buff) {
            dataBuff_ = buff;
        }
    }


    LocalSocketReceiver::~LocalSocketReceiver() {
#if DEBUG_DESTR
        qDebug() << "DESTRUCTOR OF LocalSocketReceiver\n";
#endif
    }


    void LocalSocketReceiver::showConnectWindow() {
#if DEBUG_NET
        qDebug() << "showConnectWindow()";
#endif
        LocalSocketConnectWindow *connectWindow = new LocalSocketConnectWindow(this, Qt::Window);
        connect(connectWindow, SIGNAL(connectToHost(QString)), this, SLOT(connectToServer(QString)));
        connectWindow->show();
    }


    void LocalSocketReceiver::connectToServer(QString name) {
#if DEBUG_NET
        qDebug() << "connectToServer(QString name)";
#endif

        dataSize_ = 0;
        socket_->abort();
        socket_->connectToServer(name);
    }


    void LocalSocketReceiver::connectedToServer() {
#if DEBUG_NET
        qDebug() << "connectedToServer()";
#endif
#if DEBUG_LOC_SERV_NAME
        qDebug() << socket_->fullServerName();
#endif

//        QMessageBox::information( this, tr( "Connection status" ), tr( "Connected with server name : %1").arg(socket_->serverName())));
    }


    void LocalSocketReceiver::disconnectFromServer() {
#if DEBUG_NET
        qDebug() << "disconnectFromServer()";
#endif
        socket_->close();
    }

    void LocalSocketReceiver::readData() {
#if DEBUG_NET
        qDebug() << "readData()";
#endif

        QDataStream in(socket_);
        in.setVersion(QDataStream::Qt_4_0);
        in.setByteOrder(QDataStream::LittleEndian);

        if (dataSize_ == 0)  {
            if (socket_->bytesAvailable() < (int)sizeof(quint16))
                return;
            in >> dataSize_;
            if(dataSize_ == 0)
                return;
        }

        if (in.atEnd()) {
            return;
        }

        Line line;
        uint &int_ref = line.sarImageLineHeader.magic_id;
        double &double_ref = line.sarImageLineHeader.lat_plat;
        float &float_ref = line.sarImageLineHeader.alt_sea;

//        for(int k = 0; k < SHIFT_VAL; k++) {
//            PR(k);
/*
            in.setFloatingPointPrecision(QDataStream::DoublePrecision);
            in >> line.sarImageLineHeader.magic_id;
            in >> line.sarImageLineHeader.lat_plat;
            in >> line.sarImageLineHeader.lon_plat;
            in >> line.sarImageLineHeader.lat_ref;
            in >> line.sarImageLineHeader.lon_ref;
            in.setFloatingPointPrecision(QDataStream::SinglePrecision);
            in >> line.sarImageLineHeader.alt_sea;
            in >> line.sarImageLineHeader.alt_ground;
            in >> line.sarImageLineHeader.cell_size;
            in >> line.sarImageLineHeader.r_min;
            in >> line.sarImageLineHeader.head;
            in >> line.sarImageLineHeader.head_ref;
            in >> line.sarImageLineHeader.pitch;
            in >> line.sarImageLineHeader.roll;
            in >> line.sarImageLineHeader.veloc;
            in >> line.sarImageLineHeader.prf;
            in >> line.sarImageLineHeader.flags;
            in >> line.sarImageLineHeader.nr_range_cells;
            in >> line.sarImageLineHeader.line_cnt;
            in >> line.sarImageLineHeader.block_cnt;
*/
            int_ref = line.sarImageLineHeader.magic_id;
            in >> int_ref;
            in.setFloatingPointPrecision(QDataStream::DoublePrecision);
            double_ref = line.sarImageLineHeader.lat_plat;
            in >> double_ref;
            double_ref = line.sarImageLineHeader.lon_plat;
            in >> double_ref;
            double_ref = line.sarImageLineHeader.lat_ref;
            in >> double_ref;
            double_ref = line.sarImageLineHeader.lon_ref;
            in >> double_ref;
            in.setFloatingPointPrecision(QDataStream::SinglePrecision);
            float_ref = line.sarImageLineHeader.alt_sea;
            in >> float_ref;
            float_ref = line.sarImageLineHeader.alt_ground;
            in >> float_ref;
            float_ref = line.sarImageLineHeader.cell_size;
            in >> float_ref;
            float_ref = line.sarImageLineHeader.r_min;
            in >> float_ref;
            float_ref = line.sarImageLineHeader.head;
            in >> float_ref;
            float_ref = line.sarImageLineHeader.head_ref;
            in >> float_ref;
            float_ref = line.sarImageLineHeader.pitch;
            in >> float_ref;
            float_ref = line.sarImageLineHeader.roll;
            in >> float_ref;
            float_ref = line.sarImageLineHeader.veloc;
            in >> float_ref;
            float_ref = line.sarImageLineHeader.prf;
            in >> float_ref;
            int_ref = line.sarImageLineHeader.flags;
            in >> int_ref;
            int_ref = line.sarImageLineHeader.nr_range_cells;
            in >> int_ref;
            int_ref = line.sarImageLineHeader.line_cnt;
            in >> int_ref;
            int_ref = line.sarImageLineHeader.block_cnt;
            in >> int_ref;

            for(uint y = 0; y < line.sarImageLineHeader.nr_range_cells; y++) {
                in >> line.pixels[y];
            }

            dataBuff_->setLine(&line);
//        }

        dataSize_ = 0;

        static int draw_cnt = 0;
        if (draw_cnt == 10) {
            emit drawSimpleMode();
            draw_cnt = 0;
        }
        else
            draw_cnt++;
    }


    void LocalSocketReceiver::displayError(QLocalSocket::LocalSocketError socketError) {
        switch (socketError)  {
        case QLocalSocket::ServerNotFoundError:
            QMessageBox::information(this, tr("SAR client"),
                                     tr("The host was not found. Please check "
                                        "settings."));
            break;
        case QLocalSocket::ConnectionRefusedError:
            QMessageBox::information(this, tr("SAR client"),
                                     tr("The connection was refused by the peer. "
                                        "Make sure the server is running, "
                                        "and check that "
                                        "settings are correct."));
            break;
        case QLocalSocket::PeerClosedError:
            break;
        default:
            QMessageBox::information(this, tr("SAR client"),
                                     tr("The following error occurred: %1.")
                                     .arg(socket_->errorString()));
        }
    }
} // namespace SarViewer
