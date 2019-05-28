#ifndef CONNECTIONWINDOW_H
#define CONNECTIONWINDOW_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

namespace SarViewer {
    class ConnectWindow : public QWidget {
        Q_OBJECT

    public :
        explicit ConnectWindow(QWidget *parent = 0, Qt::WindowFlags f = 0);

    signals :
        void connectToHost(QString, int);

    private slots :
        void connectToHost();

    private :
        QLabel *hostLabel_;
        QLabel *portLabel_;

        QLineEdit *hostEdit_;
        QLineEdit *portEdit_;

        QPushButton *connectButton_;
    };
} // namespace SarViewer
#endif // CONNECTIONWINDOW_H
