#include <QtGui>
#include "MapWindow.h"
#include "Debugging.h"
#include "View.h"
#include "Plane.h"

extern ViewerConfig viewer_config;

SarViewer::MapWindow::MapWindow(QMainWindow *parent) :
    QMainWindow(parent) {
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF MapWindow\n";
#endif
        setWindowTitle(tr("Map"));

        setWindowIcon(QIcon(QPixmap("./images/MapWindowIcon.jpg")));

        resize(700, 500);
        setMinimumSize(500, 300);

        // Create actions
        closeAction_ = new QAction(tr("&Close"), this);
        closeAction_->setStatusTip(tr("Close map window"));
        closeAction_->setShortcut(Qt::CTRL + Qt::Key_C);
        QObject::connect(closeAction_, SIGNAL(triggered()), this, SLOT(close()));

        exitAction_ = new QAction(tr("&Quit"), this);
        exitAction_->setShortcut(Qt::CTRL + Qt::Key_Q);
        exitAction_->setStatusTip(tr("Exit the application"));
        QObject::connect(exitAction_, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

        trackRestartAction_ = new QAction(tr("&Restart"), this);
        trackRestartAction_->setStatusTip(tr("Restart drawing the track"));
        trackRestartAction_->setShortcut(Qt::CTRL + Qt::Key_R);
        // Connect at end

        legendShowAction_ = new QAction(tr("S&how"), this);
        legendShowAction_->setStatusTip(tr("Show legend"));
        legendShowAction_->setShortcut(Qt::CTRL + Qt::Key_H);
        legendShowAction_->setCheckable(true);
        legendShowAction_->setChecked(false);
        // Connect at end

        // Create menus
        fileMenu_ = menuBar()->addMenu(tr("&File"));
        fileMenu_->addAction(closeAction_);
        fileMenu_->addAction(exitAction_);

        optionsMenu_ = menuBar()->addMenu(tr("&Options"));
        mapDetailsMenu_ = optionsMenu_->addMenu(tr("&Map details"));

        mapDetailSigMap_ = new QSignalMapper(this);

        QActionGroup *group = new QActionGroup(this);
        group->setExclusive(false);

        QAction *action = mapDetailsMenu_->addAction(tr("&Coast"));
        action->setCheckable(true);
        action->setChecked(viewer_config.showCoast);
        action->setStatusTip(tr("Show cost"));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), mapDetailSigMap_, SLOT(map()));
        mapDetailSigMap_->setMapping(action, COAST);

        action = mapDetailsMenu_->addAction(tr("Primary boundary"));
        action->setCheckable(true);
        action->setChecked(viewer_config.showPrimaryBnd);
        action->setStatusTip(tr("Show primary boundary"));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), mapDetailSigMap_, SLOT(map()));
        mapDetailSigMap_->setMapping(action, PRIMARY_BND);

        action = mapDetailsMenu_->addAction(tr("Internal boundary"));
        action->setCheckable(true);
        action->setChecked(viewer_config.showInternalBnd);
        action->setStatusTip(tr("Show internal boundary"));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), mapDetailSigMap_, SLOT(map()));
        mapDetailSigMap_->setMapping(action, INTERNAL_BND);

        action = mapDetailsMenu_->addAction(tr("Lakes"));
        action->setCheckable(true);
        action->setChecked(viewer_config.showLakes);
        action->setStatusTip(tr("Show lakes"));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), mapDetailSigMap_, SLOT(map()));
        mapDetailSigMap_->setMapping(action, LAKES);

        action = mapDetailsMenu_->addAction(tr("Rivers"));
        action->setCheckable(true);
        action->setChecked(viewer_config.showRivers);
        action->setStatusTip(tr("Show rivers"));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), mapDetailSigMap_, SLOT(map()));
        mapDetailSigMap_->setMapping(action, RIVERS);

        action = mapDetailsMenu_->addAction(tr("Roads"));
        action->setCheckable(true);
        action->setChecked(viewer_config.showRoad);
        action->setStatusTip(tr("Show roads"));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), mapDetailSigMap_, SLOT(map()));
        mapDetailSigMap_->setMapping(action, ROAD);

        action = mapDetailsMenu_->addAction(tr("Airports"));
        action->setCheckable(true);
        action->setChecked(viewer_config.showAirport);
        action->setStatusTip(tr("Show airports"));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), mapDetailSigMap_, SLOT(map()));
        mapDetailSigMap_->setMapping(action, AIRPORT);

        action = mapDetailsMenu_->addAction(tr("Airport names"));
        action->setCheckable(true);
        action->setChecked(viewer_config.showAirportNamed);
        action->setStatusTip(tr("Show airport names"));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), mapDetailSigMap_, SLOT(map()));
        mapDetailSigMap_->setMapping(action, AIRPORT_NAMED_LOCATION);

        action = mapDetailsMenu_->addAction(tr("Cities"));
        action->setCheckable(true);
        action->setChecked(viewer_config.showCity);
        action->setStatusTip(tr("Show cities"));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), mapDetailSigMap_, SLOT(map()));
        mapDetailSigMap_->setMapping(action, CITY);

        action = mapDetailsMenu_->addAction(tr("City names"));
        action->setCheckable(true);
        action->setChecked(viewer_config.showCityNamed);
        action->setStatusTip(tr("Show city name"));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), mapDetailSigMap_, SLOT(map()));
        mapDetailSigMap_->setMapping(action, CITY_NAMED_LOCATION);

        optionsMenu_->addSeparator();
        keepAspeRatAction_ =  optionsMenu_->addAction(tr("Keep aspect ratio"));
        keepAspeRatAction_->setCheckable(true);
        keepAspeRatAction_->setChecked(true);
        keepAspeRatAction_->setStatusTip(tr("Keep aspect ratio"));
        keepAspeRatAction_->setShortcut(Qt::CTRL + Qt::Key_K);

        // View menu
        viewMenu_ = menuBar()->addMenu(tr("&View"));

        trackMenu_ = viewMenu_->addMenu(tr("&Track"));
        trackMenu_->addAction(trackRestartAction_);

        legendMenu_ = viewMenu_->addMenu(tr("&Legend"));
        legendMenu_->addAction(legendShowAction_);

        statusBar()->showMessage(tr("Ready"));

//        scene_ = new QGraphicsScene;

        mapViewer_ = new SarViewer::MapViewer(this);
        QObject::connect(mapDetailSigMap_, SIGNAL(mapped(int)), this->mapViewer_, SLOT(setMapDetailsVisib(int)));
        QObject::connect(trackRestartAction_, SIGNAL(triggered()),
                         getMapViewer()->getPlane(), SLOT(trackRestart()));
        QObject::connect(legendShowAction_, SIGNAL(triggered()),
                         getMapViewer(), SLOT(legendVisible()));
        QObject::connect(keepAspeRatAction_, SIGNAL(toggled(bool)),
                         mapViewer_, SLOT(keepAspeRatToggled(bool)));
//        SarViewer::MapViewer *mv = new SarViewer::MapViewer();
//        mapViewer_ = mv;

//        this->setCentralWidget(mapViewer_); // albo to albo addWidget do sceny!!!

        QFrame *frame = new QFrame;
//        QGroupBox *group_Box = new QGroupBox("Contact Details");
//        group_Box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QVBoxLayout *frame_layout = new QVBoxLayout;
        frame_layout->addWidget(mapViewer_);
        frame->setLayout(frame_layout);
//        QGraphicsProxyWidget * proxy = scene_->addWidget(mapViewer_);
//        QGraphicsProxyWidget *proxy = scene_->addWidget(mv);
//        QGraphicsProxyWidget *proxy = scene_->addWidget(group_Box);

//        mapViewer_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

//        view_ = new View(scene_);
//        view_->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
//        view_->setCacheMode(QGraphicsView::CacheNone);
//        view_->setRenderHints(QPainter::Antialiasing);
        this->setCentralWidget(frame);

//        view_->show();
}


SarViewer::MapWindow::~MapWindow() {
#if DEBUG_DESTR
        qDebug() << "DESTRUCTOR OF MapWindow\n";
#endif
        delete mapViewer_;
}


void SarViewer::MapWindow::resizeEvent(QResizeEvent *event) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    QMainWindow::resizeEvent(event);
    update();
}


void SarViewer::MapWindow::closeEvent(QCloseEvent *event) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    emit mapWindowShowEnable();
    QMainWindow::closeEvent(event);
}


void SarViewer::MapWindow::close() {
//    QWidget::close();
//    this->setVisible(false);
    this->hide();
    emit mapWindowShowEnable();
}
