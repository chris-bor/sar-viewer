#include <QtNetwork>
#include <QMessageBox>
#include "NetworkReceiver.h"
#include "ConnectWindow.h"
#include "Config.h"

extern ViewerConfig viewer_config;

namespace SarViewer {
    NetworkReceiver::NetworkReceiver(QWidget *parent, SarImageDataBuffer *buff) : QWidget(parent){
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF NetworkReceiver\n";
#endif
        tcpSocket_ = new QTcpSocket(this);
        connect(tcpSocket_, SIGNAL(readyRead()), this, SLOT(readData()));
        connect(tcpSocket_, SIGNAL(connected()), this, SLOT(connectedToServer())); // connection signal emited when connection occured to slot which inform as that this connection ended successfully
        connect(tcpSocket_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));

        if(buff) {
            dataBuff_ = buff;
        }
    }


    NetworkReceiver::~NetworkReceiver() {
#if DEBUG_CONST
        qDebug() << "DESTRUCTOR OF NetworkReceiver\n";
#endif
        if(!tcpSocket_->peerAddress().isNull()) {
            disconnectFromServer();
        }
    }


    void NetworkReceiver::showConnectWindow() {
#if DEBUG_NET
        qDebug() << "showConnectWindow()";
#endif
        ConnectWindow *connectWindow = new ConnectWindow(this, Qt::Window);
        connect(connectWindow, SIGNAL(connectToHost(QString, int)), this, SLOT(connectToServer(QString, int)));
        connectWindow->show();
    }


    void NetworkReceiver::connectToServer(QString ip, int port) {
#if DEBUG_NET
        qDebug() << "connectToServer(QString ip, int port)";
#endif

        dataSize_ = 0;
        tcpSocket_->abort();
        tcpSocket_->connectToHost(ip, port);
    }


    void NetworkReceiver::connectedToServer() {
#if DEBUG_NET
        qDebug() << "connectedToServer()";
#endif
//        QMessageBox::information( this, tr( "Connection status" ), tr( "Connected with server : %1 port : %2 ").arg(tcpSocket_->peerAddress().toString()).arg(tcpSocket_->peerPort()));
    }


    void NetworkReceiver::disconnectFromServer() {
#if DEBUG_NET
        qDebug() << "disconnectFromServer()";
#endif
        tcpSocket_->close();
    }


    void NetworkReceiver::readData() {
#if DEBUG_NET
        qDebug() << "readData()";
#endif

        QDataStream in(tcpSocket_);
        in.setVersion(QDataStream::Qt_4_0);

        if (dataSize_ == 0)  {
            if (tcpSocket_->bytesAvailable() < (int)sizeof(quint16))
                return;
            in >> dataSize_;
//            PR_LINE("");
//            PR(dataSize_);
        }

        if (tcpSocket_->bytesAvailable() < dataSize_ && dataSize_ == 0) {
//            QByteArray ar = tcpSocket_->readAll();
//            PR(ar.size());
            return;
        }

        if (in.atEnd())
            return;

        Line line;
        uint &int_ref = line.sarImageLineHeader.magic_id;
        double &double_ref = line.sarImageLineHeader.lat_plat;
        float &float_ref = line.sarImageLineHeader.alt_sea;


//        PR_LINE("");
//        PR(dataSize_);
//        PR(tcpSocket_->bytesAvailable());

//        quint8 cnt = dataSize_/1342;
//        PR(cnt);

        for(uint k = 0; k < viewer_config.shift_val; k++) {
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

//            PR(line.sarImageLineHeader.line_cnt);
//            PR(line.sarImageLineHeader.nr_range_cells);

            for(uint y = 0; y < line.sarImageLineHeader.nr_range_cells; y++) {
                in >> line.pixels[y];
            }

            dataBuff_->setLine(&line);
        }

        dataSize_ = 0;
//        PR("\n\n");
//        return;

//        PR_LINE("EMIT");
        emit drawSimpleMode();

    }


    void NetworkReceiver::displayError(QAbstractSocket::SocketError socketError) {
        switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            break;

        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, tr("SAR client"),
                                     tr("The host was not found. Please check the "
                                        "host name and port settings."));
            break;

        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, tr("SAR client"),
                                     tr("The connection was refused by the peer. "
                                        "Make sure the server is running, "
                                        "and check that the host name and port "
                                        "settings are correct."));
            break;

        default:
            QMessageBox::information(this, tr("SAR client"),
                                     tr("The following error occurred: %1.")
                                     .arg(tcpSocket_->errorString()));
        }

    }
} // namespace SarViewer
