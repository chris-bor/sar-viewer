#include <QtGui>
#include "ConnectWindow.h"

namespace SarViewer {
    ConnectWindow::ConnectWindow(QWidget *parent, Qt::WindowFlags f) :
        QWidget(parent, f) {
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF ConnectWindow\n";
#endif
        hostEdit_ = new QLineEdit(this);
        portEdit_ = new QLineEdit(this);
        portEdit_->setValidator(new QIntValidator(1024, 65535, this));
        portEdit_->setText("1234");
        hostEdit_->setText("127.0.0.1");

        hostLabel_ = new QLabel(tr("&Server:"), this);
        hostLabel_->setBuddy(hostEdit_);

        portLabel_ = new QLabel(tr("&Port:"), this);
        portLabel_->setBuddy(portEdit_);

        connectButton_ = new QPushButton(tr("&Connect"), this);
        connect(connectButton_, SIGNAL(clicked()), this, SLOT(connectToHost()));

        QGridLayout *mainLayout = new QGridLayout(this);
        mainLayout->addWidget(hostLabel_, 0, 0, 1, 1);
        mainLayout->addWidget(hostEdit_, 0, 1, 1, 2);
        mainLayout->addWidget(portLabel_, 1, 0, 1, 1);
        mainLayout->addWidget(portEdit_, 1, 1, 1, 2);
        mainLayout->addWidget(connectButton_, 3, 2, 1, 1);
    }


    void ConnectWindow::connectToHost() {
        emit connectToHost(hostEdit_->text(), portEdit_->text().toInt()); // connected to connectToServer(ip, port)
        close();
    }

} // namespace SarViewer
