#ifndef LOCALSOCKETRECEIVER_H
#define LOCALSOCKETRECEIVER_H

#include <QWidget>
#include <qlocalsocket.h>
#include "SarImageDataBuffer.h"

class QLocalSocket;
class QAction;

namespace SarViewer {
    class LocalSocketReceiver : public QWidget {
        Q_OBJECT
    public :
        LocalSocketReceiver(QWidget *parent = 0, SarImageDataBuffer *buff = 0);
        ~LocalSocketReceiver();

    signals :
        void drawSimpleMode();

    private slots :
        void showConnectWindow();
        void connectToServer(QString);
        void connectedToServer();
        void disconnectFromServer();
        void readData();
        void displayError(QLocalSocket::LocalSocketError socketError);

    private :
        QAction *connectAction_;
        QAction *disconnectAction_;
        QAction *quitAction_;
        quint16 dataSize_;

        QLocalSocket *socket_;

        SarImageDataBuffer *dataBuff_;
    };

} // namespace SarViewer
#endif // LOCALSOCKETRECEIVER_H
