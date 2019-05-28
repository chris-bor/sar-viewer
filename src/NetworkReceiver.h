/// @file LocalSocketReceiverThread.h
/// @brief class is responsible for receiving data using TCP.
/// @author Krzysztof Borowiec

#ifndef NETWORKRECEIVER_HPP
#define NETWORKRECEIVER_HPP

#include <QAbstractSocket>
#include <qlocalsocket.h>
#include <QWidget>
#include "SarImageDataBuffer.h"

class QTcpSocket;
class QAction;

namespace SarViewer {
    class NetworkReceiver : public QWidget {
        Q_OBJECT
    public :
        NetworkReceiver(QWidget *parent = 0, SarImageDataBuffer *buff = 0);
        ~NetworkReceiver();

    signals :
        void drawSimpleMode();

    private slots :
        void showConnectWindow();
        void connectToServer(QString, int);
        void connectedToServer();
        void disconnectFromServer();
        void readData();
        void displayError(QAbstractSocket::SocketError);

    private:
        QAction *connectAction_;
        QAction *disconnectAction_;
        QAction *quitAction_;
        quint16 dataSize_;

        QTcpSocket *tcpSocket_;

        SarImageDataBuffer *dataBuff_;
    };
} // namespace SarViewer
#endif // NETWORKRECEIVER_HPP
