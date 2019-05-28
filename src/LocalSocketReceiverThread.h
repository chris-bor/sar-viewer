/// @file LocalSocketReceiverThread.h
/// @brief class is responsible for receiving data using named pipes. Class bases on QThread.
/// @author Krzysztof Borowiec

#ifndef LOCALSOCKETRECEIVERTHREAD_H
#define LOCALSOCKETRECEIVERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <qlocalsocket.h>
#include "SarImageDataBuffer.h"
#include "Debugging.h"

class QAction;

namespace SarViewer {
//    const int SIZE_DATA_BLOCK = 1551; // size of block at lines
//    const int MIN_BYTES_AVAILABLE = SIZE_DATA_BLOCK * SIZE_OF_LINE; // Block lines to receive at one time
    const int RECEIVER_SLEEP_TIME = 1000; // sleeping time of thread which receive data from server
    const int MAGIC_ID_DEF = 5; // Magic id necessary to check data correction

    class LocalSocketReceiverThread : public QThread {
        Q_OBJECT

    public :
        LocalSocketReceiverThread(QObject *parent = 0);
        virtual ~LocalSocketReceiverThread();
        inline QLocalSocket const *getSocket() const {
            return(&socket_);
        }
        inline void setSarImgDataBuf(SarImageDataBuffer *buff) {
            if(buff) {
                dataBuff_ = buff;
            }
            else {
                PR_ERROR("Error passing allocation buffer at file:");
            }
        }
        inline void setQuit() {
            this->quit_ = true;
        }

    protected :
        void run();

    signals :
        void drawSimpleMode(); // Keep SimpleModeDrawerThread inform that new data are received and raedy to draw
        void drawSimpleMode(int,    // Number of lines per one block
                            int,    // Time to show one block of data
                            int);   // nrumber of range cells to scale image if changed
        void setPlatParams(double lat, double lon, float head, float pitch, float roll,
                           float alt, float veloc, int anal_sett); // Sets parameters at main window

    private slots :
        void connectToServer();
        void connectedToServer();
        void disconnectFromServer();
        void readData(); // is called by LocalSocketReceiverThread whenever it needs to copy data from socket
        void displayError(QLocalSocket::LocalSocketError socketError);

    private :
        QAction *connectAction_; // connect action
        QAction *disconnectAction_; // disconnect action
        QAction *quitAction_; // quit action

        uint bytesAvailable_; // bytes available from socket_
        int nrLines_; // number lines to read calculatedo from bytesAvailable_

        QLocalSocket socket_; // socket for named pipe

        SarImageDataBuffer *dataBuff_; // circural SAR image data buffer

        mutable QMutex mutex_; // for blocking conditional wait
        QWaitCondition condition_; // we need a mutex to protect accesses to the abort and condition variables, which might be accessed at any time by run()
        volatile bool quit_; // variable necessary to stop thread
    };

} // namespace SarViewer

#endif // LOCALSOCKETRECEIVERTHREAD_H
