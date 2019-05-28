#include <QtGui>
#include "LocalSocketConnectWindow.h"

namespace SarViewer {
    LocalSocketConnectWindow::LocalSocketConnectWindow(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f) {
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF LocalSocketConnectWindow\n";
#endif

        hostEdit_ = new QLineEdit(this);
        hostEdit_->setText("SAR_PIPE");

        hostLabel_ = new QLabel(tr("&Server:"), this);
        hostLabel_->setBuddy(hostEdit_);

        connectButton_ = new QPushButton(tr("&Connect"), this);
        connect(connectButton_, SIGNAL(clicked()), this, SLOT(connectToHost()));

        QGridLayout *mainLayout = new QGridLayout(this);
        mainLayout->addWidget(hostLabel_, 0, 0, 1, 1);
        mainLayout->addWidget(hostEdit_, 0, 1, 1, 2);
        mainLayout->addWidget(connectButton_, 3, 2, 1, 1);
    }


    void LocalSocketConnectWindow::connectToHost() {
        emit connectToHost(hostEdit_->text()); // connected to connectToServer(ip, port)
        close();
    }
} // namespace SarViewer
