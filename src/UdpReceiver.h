/// @file UdpReceiver.h
/// @brief class is responsible for receiving data using UDP by network. Class bases on QObject. Will be worked at thread
/// @author Krzysztof Borowiec

#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include <QWidget>
#include <qudpsocket.h>
#include <qhostaddress.h>
#include "SarImageDataBuffer.h"
#include "Debugging.h"
#include "sar_image.h"

class QAction;

namespace SarViewer {
    class UdpReceiver : public QObject {
        Q_OBJECT

    public :
        UdpReceiver(QString address = 0 , int port = 0, int nr_stream = -1, QObject *parent = 0);
        virtual ~UdpReceiver();
        inline QUdpSocket const *getSocket() const {
            return(socket_);
        }
        inline void setSarImgDataBuf(SarImageDataBuffer *buff) {
            if(buff) {
                dataBuff_ = buff;
            }
            else {
                PR_ERROR("Error passing allocation buffer at file:");
            }
        }
        void drawStripScanImage(int most_actual_line);

    signals :
        void drawSimpleMode(int,    // Number of lines per one block
                            int,    // Time to show one block of data
                            int,    // Number of range cells to scale image if changed
                            int,    // Current most actual line where thread should start draw
                            int);   // Current start present
        void setPlatParams(double lat, double lon, FLOAT alt, FLOAT head, FLOAT pitch, FLOAT roll,
                           FLOAT veloc, UINT32 anal_sett, UINT32 nr_range_cells); // Sets parameters at main window
        void changeAction(bool val, int kind);
        void addTrackPoint(sar_image_line_header header, int most_actual_line);

    public slots :
        void connectToServer();
        void setIsVisible(bool val);

    private slots :
        void readPendingDatagrams(); // is called by LocalSocketReceiverThread whenever it needs to copy data from socket

    private :
        QAction *connectAction_; // connect action
        QAction *disconnectAction_; // disconnect action
        QAction *quitAction_; // quit action

        int nrLines_; // number lines to read calculatedo from bytesAvailable_

        QUdpSocket *socket_; // socket for named pipe
        QHostAddress address_;
        int port_;

        SarImageDataBuffer *dataBuff_; // circural SAR image data buffer

        int nrStream_; // Number of managed stream

        uint nrRangeCells_;
        uint nrLinesPerBlock_;
        uint nrLinToRefrStrScan_;
        uint nrLineToRefreshStripScaneImg_;
        int sleepTime_;

        QByteArray datagram_;
        quint64 pendingDatagramSize_;
        QHostAddress sender_;
        quint16 senderPort_;
        int nr_line;
        bool isVisible_;
        quint8 altitudeType_; // Determine type of altitude (0, ground or sea)
    };
}
#endif // UDPRECEIVER_H
