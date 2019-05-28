#ifndef LOCALSOCKETCONNECTWINDOW_H
#define LOCALSOCKETCONNECTWINDOW_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

namespace SarViewer {
    class LocalSocketConnectWindow : public QWidget {
        Q_OBJECT

    public :
        explicit LocalSocketConnectWindow(QWidget *parent = 0, Qt::WindowFlags f = 0);

    signals :
        void connectToHost(QString);

    private slots :
        void connectToHost();

    private :
        QLabel *hostLabel_;
        QLineEdit *hostEdit_;
        QPushButton *connectButton_;
    };
} // namespace SarViewer
#endif // LOCALSOCKETCONNECTWINDOW_H
