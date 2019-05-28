#include <QtGui>
#include <ctime>
#include <QDebug>
#include <QKeyEvent>
//#include <QMetaType>
#include "MainWindow.h"
#include "Config.h"
#include "SimpleMode.h"
#include "SimpleModeDrawer.h"
#include <stdio.h>
#include "uni_types.h"
#include "sar_types.h"
#include "propatria_types.h"
#include "MapWindow.h"
#include "StripScanImage.h"

#ifdef __unix__
#include <unistd.h>
#else
#include <windows.h>
#include <winbase.h>
#endif

extern ViewerConfig viewer_config;

namespace SarViewer {
    MainWindow::MainWindow(QMainWindow *parent) :
        QMainWindow(parent), APP_NAME( DEF_APP_NAME ), closeAllWindows_(false), /*locSockRecvThr_(qobject_cast<QObject *>(this)),*/
        onQuit_(false)
        {
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF MainWindow\n";
#endif
        setWindowTitle( APP_NAME );
        englishLocale_ = QLocale(QLocale::English);
        setAttribute(Qt::WA_DeleteOnClose);

        setWindowIcon(QIcon(QPixmap("./images/MainWindowIcon.png")));

//        setBackgroundRole(QPalette::Light); // dark background until paintEvent() appers
//        setAutoFillBackground(true); // allows the above mechanism
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // This tells any layout manager that is responsible for the widget that the widget is especially willing to grow, but can also shrink
        setFocusPolicy(Qt::StrongFocus); // Makes the widget accept focus by clicking or by pressing Tab

        windowSize_.setWidth( WINDOW_WIDTH_DEF );
        windowSize_.setHeight( WINDOW_HEIGHT_DEF );
        resize( windowSize_ );

        removeContextMenuFromAllWidgets();

        createPlatParamDockWidget();
        createDockWidgets();

        simpleMode_ = new SimpleMode(this);

//        setPlotSettings(PlotSettings());

        // TCP receiver
        /*networkReceiver_ = new NetworkReceiver(this, simpleMode_->getSarImgDataBuf(Stream0));
        QObject::connect(networkReceiver_, SIGNAL(drawSimpleMode()), simpleMode_, SLOT(drawSimpleModeNetwork()));

        // Local socked receiver - named pipe
        locSockRecvThr_.setSarImgDataBuf(simpleMode_->getBigSarImgDataBuf()); // set bigger buffer
        QObject::connect(&locSockRecvThr_, SIGNAL(drawSimpleMode(int, int, int)),
                         simpleMode_->getPtrSimpleModeDrawerThr(Stream0), SLOT(startDrawing(int, int, int)));
        QObject::connect(simpleMode_->getPtrSimpleModeDrawerThr(Stream0), SIGNAL(imageReady(QImage)),
                         simpleMode_, SLOT(drawSimpleModeLocSock(QImage)));
        QObject::connect(&locSockRecvThr_, SIGNAL(setPlatParams(double,double,float,float,float,float,float,int)),
                         this, SLOT(setPlatParams(double,double,float,float,float,float,float,int)));
        QObject::connect(streamDockWidget_[Stream0], SIGNAL(resized(QSize)),
                         simpleMode_->getPtrSimpleModeDrawerThr(Stream0), SLOT(setSizeBackImg(QSize)));*/

        // UDP receiver STREAM 0
        udpRecvStream0_ = new UdpReceiver(VIEWER_IP_ADD, VIEWER_PORT_NR_0, Stream0);
//        udpRecvStream0_ = new UdpReceiver(VIEWER_IP_ADD, VIEWER_PORT_NR_0, Stream0, qobject_cast<QObject *>(this));
        udpRecvStream0_->setSarImgDataBuf(simpleMode_->getBigSarImgDataBuf(Stream0));
        QObject::connect(udpRecvStream0_, SIGNAL(drawSimpleMode(int,int,int,int,int)),
                         simpleMode_->simpleModeDrawer(Stream0), SLOT(startDrawing(int,int,int,int,int)), Qt::AutoConnection);
        QObject::connect(simpleMode_->simpleModeDrawer(Stream0), SIGNAL(imageReady(QImage)),
                         simpleMode_, SLOT(drawSimpleModeUdpStream0(QImage)), Qt::AutoConnection);
        QObject::connect(udpRecvStream0_, SIGNAL(setPlatParams(double,double,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,UINT32,UINT32)),
                         this, SLOT(setPlatParams(double,double,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,UINT32,UINT32)), Qt::AutoConnection);
        QObject::connect(streamDockWidget_[Stream0], SIGNAL(resized(QSize)),
                         simpleMode_->simpleModeDrawer(Stream0), SLOT(setSizeBackImg(QSize)));
        QObject::connect(streamDockWidget_[Stream0], SIGNAL(visibilityChanged(bool)),
                         simpleMode_, SLOT(dockWidgVisiChan0(bool)));
        QObject::connect(streamDockWidget_[Stream0], SIGNAL(visibilityChanged(bool)),
                         udpRecvStream0_, SLOT(setIsVisible(bool)));

        // UDP receiver STREAM 1
        udpRecvStream1_ = new UdpReceiver(VIEWER_IP_ADD, VIEWER_PORT_NR_1, Stream1);
//        udpRecvStream1_ = new UdpReceiver(VIEWER_IP_ADD, VIEWER_PORT_NR_1, Stream1, qobject_cast<QObject *>(this));
        udpRecvStream1_->setSarImgDataBuf(simpleMode_->getBigSarImgDataBuf(Stream1));
        QObject::connect(udpRecvStream1_, SIGNAL(drawSimpleMode(int,int,int,int,int)),
                         simpleMode_->simpleModeDrawer(Stream1), SLOT(startDrawing(int,int,int,int,int)), Qt::AutoConnection);
        QObject::connect(simpleMode_->simpleModeDrawer(Stream1), SIGNAL(imageReady(QImage)),
                         simpleMode_, SLOT(drawSimpleModeUdpStream1(QImage)), Qt::AutoConnection);
        QObject::connect(streamDockWidget_[Stream1], SIGNAL(resized(QSize)),
                         simpleMode_->simpleModeDrawer(Stream1), SLOT(setSizeBackImg(QSize)));
        QObject::connect(streamDockWidget_[Stream1], SIGNAL(visibilityChanged(bool)),
                         simpleMode_, SLOT(dockWidgVisiChan1(bool)));
        QObject::connect(streamDockWidget_[Stream1], SIGNAL(visibilityChanged(bool)),
                         udpRecvStream1_, SLOT(setIsVisible(bool)));

        createActions();
        createMenus();
        createStatusBar();
        createView();

        QObject::connect(scene_[Stream0], SIGNAL(zoomChanged()),
                         simpleMode_->simpleModeDrawer(Stream0), SLOT(setZoom()));
        QObject::connect(scene_[Stream1], SIGNAL(zoomChanged()),
                         simpleMode_->simpleModeDrawer(Stream1), SLOT(setZoom()));
        for(int nr = 0; nr < NrStreams; nr++) {
            QObject::connect(showColorbarAction_[nr], SIGNAL(toggled(bool)),
                             scene_[nr], SLOT(showColorbar(bool)));
        }

        setVisible( true );
        setMinimumWidth(minimumHeight());
        showMaximized();

        mapWindow_ = new SarViewer::MapWindow;
        if(mapWindow_ != 0L) {
            mapWindow_->getMapViewer()->getPlane()->stripScanImage()->setCircBuffer(simpleMode_->getBigSarImgDataBuf(Stream0));
        }

        QObject::connect(udpRecvStream0_, SIGNAL(addTrackPoint(sar_image_line_header, int)),
                         mapWindow_->getMapViewer()->getPlane(), SLOT(addTrackPoint(sar_image_line_header, int)));
        QObject::connect(mapWindow_, SIGNAL(mapWindowShowEnable()),
                         this, SLOT(mapWindowShowEnable()));

        // Create treads
        udpRecvThr_[Stream0] = new QThread(this);
        udpRecvThr_[Stream1] = new QThread(this);
        simpleModeDrawerThr_[Stream0] = new QThread(this);
        simpleModeDrawerThr_[Stream1] = new QThread(this);
        planeThr_ = new QThread(this);

        // Connect signal finished with slot
        QObject::connect(udpRecvThr_[Stream0], SIGNAL(finished()), this, SLOT(onUdpRecvThrFinished0()));
        QObject::connect(udpRecvThr_[Stream1], SIGNAL(finished()), this, SLOT(onUdpRecvThrFinished1()));
        QObject::connect(simpleModeDrawerThr_[Stream0], SIGNAL(finished()), this, SLOT(onSimpModDrawThrFinished0()));
        QObject::connect(simpleModeDrawerThr_[Stream1], SIGNAL(finished()), this, SLOT(onSimpModDrawThrFinished1()));
        QObject::connect(planeThr_, SIGNAL(finished()), this, SLOT(onPlaneThrFinished()));

        // Moving to threads
        udpRecvStream0_->moveToThread(udpRecvThr_[Stream0]);
        udpRecvStream1_->moveToThread(udpRecvThr_[Stream1]);
        simpleMode_->simpleModeDrawer(Stream0)->moveToThread(simpleModeDrawerThr_[Stream0]);
        simpleMode_->simpleModeDrawer(Stream1)->moveToThread(simpleModeDrawerThr_[Stream1]);
//        mapWindow_->getMapViewer()->getPlane()->moveToThread(planeThr_);

        // Start threads
        udpRecvThr_[Stream0]->start(QThread::LowPriority);
        udpRecvThr_[Stream1]->start(QThread::LowPriority);
        simpleModeDrawerThr_[Stream0]->start(QThread::LowPriority);
        simpleModeDrawerThr_[Stream1]->start(QThread::LowPriority);
//        planeThr_->start(QThread::LowestPriority);

        // Show MapWindow
//        QTimer::singleShot(100, this, SLOT(showMapWindow()));
    }


    MainWindow::~MainWindow() {
#if DEBUG_DESTR
        qDebug() << "DESTRUCTOR OF MainWindow\n";
#endif
        // Delete udp receivers
        if(udpRecvStream0_ != 0L) {
            delete udpRecvStream0_;
            udpRecvStream0_ = 0L;
        }
        if(udpRecvStream1_ != 0L) {
            delete udpRecvStream1_;
            udpRecvStream1_ = 0L;
        }

        // Delete udp threads
        if(udpRecvThr_[Stream0] != 0L) {
            delete udpRecvThr_[Stream0];
            udpRecvThr_[Stream0] = 0L;
        }
        if(udpRecvThr_[Stream1] != 0L) {
            delete udpRecvThr_[Stream1];
            udpRecvThr_[Stream1] = 0L;
        }

        // Delete plane thread
        if(planeThr_ != 0L) {
            delete planeThr_;
            planeThr_ = 0L;
        }

        // Delete drawer threads
        if(simpleModeDrawerThr_[Stream0] != 0L) {
            delete simpleModeDrawerThr_[Stream0];
            simpleModeDrawerThr_[Stream0] = 0L;
        }
        if(simpleModeDrawerThr_[Stream1] != 0L) {
            delete simpleModeDrawerThr_[Stream1];
            simpleModeDrawerThr_[Stream1] = 0L;
        }


        /*locSockRecvThr_.setQuit();*/
        if(streamDockWidget_[Stream0]->isFloating()) {
            streamDockWidget_[Stream0]->setFloating(false);
        }
        if(streamDockWidget_[Stream1]->isFloating()) {
            streamDockWidget_[Stream1]->setFloating(false);
        }

        if(view_[Stream0])
            delete view_[Stream0];
        if(view_[Stream1])
            delete view_[Stream1];
        if(scene_[Stream0])
            delete scene_[Stream0];
        if(scene_[Stream1])
            delete scene_[Stream1];
        /*if(networkReceiver_)
            delete networkReceiver_;*/
        delete simpleMode_;

        if(mapWindow_) {
            delete mapWindow_;
            mapWindow_ = 0L;
        }
    }


    void MainWindow::paintEvent( QPaintEvent * event ) {
//        QWidget::paintEvent(event);
        QMainWindow::paintEvent(event);
        if( simpleMode_->getSimpleModeSwitch() ) {
//            QTime t;
//            t.start();
//            simpleMode_->getScenePixmap(Stream0)->convertFromImage(*simpleMode_->getImage32(Stream0), Qt::AutoColor);
//            simpleMode_->getScenePixmap(Stream1)->convertFromImage(*simpleMode_->getImage32(Stream1), Qt::AutoColor);
//            *simpleMode_->getScenePixmap(Stream0) = QPixmap::fromImage(*simpleMode_->getImage32(Stream0), Qt::AutoColor);
//            *simpleMode_->getScenePixmap(Stream1) = QPixmap::fromImage(*simpleMode_->getImage32(Stream1), Qt::AutoColor);
//            simpleMode_->getScenePixmap(Stream0)->fromImage(*simpleMode_->getImage32(Stream0));
//            simpleMode_->getScenePixmap(Stream1)->fromImage(*simpleMode_->getImage32(Stream1));

//            this->scenePainter_.begin( simpleMode_->getScenePixmap() );

//            QRectF rect( 0, 0, (simpleMode_->getScenePixmap() )->width(), ( simpleMode_->getScenePixmap() )->height() );

//            scenePainter_.drawImage(rect, *(simpleMode_->getImage32()));
//            this->scenePainter_.end();
//            qDebug("drawSimpleModeLocSock(QImage img) tooks %i ms", t.elapsed());
        }
#if ( DEBUG_QPAINTEVENT )
            PR_LINE( "PAINTEVENT" );
#endif
        return;
    }


    void MainWindow::resizeEvent(QResizeEvent *event) {
        QMainWindow::resizeEvent(event);
    }


    void MainWindow::closeEvent(QCloseEvent *event) {
//        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
        // If both treads are finished
        if(!udpRecvThr_[Stream0]->isRunning() && !udpRecvThr_[Stream1]->isRunning() &&
                !simpleModeDrawerThr_[Stream0]->isRunning() &&
                !simpleModeDrawerThr_[Stream1]->isRunning() &&
                !planeThr_->isRunning()) {
            event->accept();
        }
        else {
            event->ignore();
        }

        // Set closeAllWindows
        closeAllWindows_ = true;

        // Require quit threads
        if(udpRecvThr_[Stream0]->isRunning()) {
            udpRecvThr_[Stream0]->quit();
        }
        if(udpRecvThr_[Stream1]->isRunning()) {
            udpRecvThr_[Stream1]->quit();
        }
        if(simpleModeDrawerThr_[Stream0]->isRunning()) {
            simpleModeDrawerThr_[Stream0]->quit();
        }
        if(simpleModeDrawerThr_[Stream1]->isRunning()) {
            simpleModeDrawerThr_[Stream1]->quit();
        }
        if(planeThr_->isRunning()) {
            planeThr_->quit();
        }
    }


    void MainWindow::createMenus() {
        // menu FILE
        fileMenu_ = menuBar()->addMenu(tr("&File"));
        /*fileMenu_->addAction(replayAction_);*/
        fileMenu_->addAction(simpleModeFromNetworkAction_);
        /*fileMenu_->addAction(simpleModeFromLocSockThrAction_);*/

        fileMenu_->addSeparator();

        fileMenu_->addAction(exitAction_);

        // menu VIEW
        viewMenu_ = menuBar()->addMenu(tr("&View"));
        QAction *action = streamDockWidget_[Stream0]->toggleViewAction();
        viewMenu_->addAction(action);
        action = streamDockWidget_[Stream1]->toggleViewAction();
        viewMenu_->addAction(action);
        viewMenu_->addAction(paramDockWidget_->toggleViewAction());

        viewMenu_->addSeparator();

        viewMenu_->addAction(mapWindowShowAction_);

        // menu OPTIONS
        optionsMenu_ = menuBar()->addMenu(tr("&Options"));
        paletteMenu_ = optionsMenu_->addMenu(tr("&Palette"));

        paletteSigMap_ = new QSignalMapper(this);
        QObject::connect(paletteSigMap_, SIGNAL(mapped(int)),
                this->simpleMode_->simpleModeDrawer(Stream0), SLOT(setPalette(int)));
        QObject::connect(paletteSigMap_, SIGNAL(mapped(int)),
                this->simpleMode_->simpleModeDrawer(Stream1), SLOT(setPalette(int)));

        QActionGroup *group = new QActionGroup(this);
        group->setExclusive(true);

        action = paletteMenu_->addAction(tr("&Jet"));
        action->setCheckable(true);
        action->setChecked(true);
        action->setStatusTip((tr("Jet palette")));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), paletteSigMap_, SLOT(map()));
        paletteSigMap_->setMapping(action, PaletteJet);

        action = paletteMenu_->addAction(tr("&Gray"));
        action->setCheckable(true);
        action->setStatusTip((tr("Gray palette")));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), paletteSigMap_, SLOT(map()));
        paletteSigMap_->setMapping(action, PaletteGray);

        action = paletteMenu_->addAction(tr("&Inverse gray"));
        action->setCheckable(true);
        action->setStatusTip((tr("Inverse gray palette")));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), paletteSigMap_, SLOT(map()));
        paletteSigMap_->setMapping(action, PaletteGrayInverse);

        action = paletteMenu_->addAction(tr("G&reen"));
        action->setCheckable(true);
        action->setStatusTip(tr("Green palette"));
        group->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), paletteSigMap_, SLOT(map()));
        paletteSigMap_->setMapping(action, PaletteGreen);

        optionsMenu_->addSeparator();

        showColorbarMenu_ = optionsMenu_->addMenu(tr("Show colorbar"));
        showColorbarMenu_->addAction(showColorbarAction_[Stream0]);
        showColorbarMenu_->addAction(showColorbarAction_[Stream1]);

        // menu HELP
        helpMenu_ = menuBar()->addMenu(tr("&Help"));
        helpMenu_->addAction(aboutAction_);

        return;
    }


    void MainWindow::createActions() {
        aboutAction_ = new QAction(tr("&About"), this);
        aboutAction_->setStatusTip(tr("Show the application's About box"));
        QObject::connect(aboutAction_, SIGNAL(triggered()), this, SLOT(about()));

        exitAction_ = new QAction(tr("&Quit"), this);
        exitAction_->setShortcut(Qt::CTRL + Qt::Key_Q);
        exitAction_->setStatusTip(tr("Exit the application"));
//        QObject::connect(exitAction_, SIGNAL(triggered()), this, SLOT(close()));
        QObject::connect(exitAction_, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

        /*replayAction_ = new QAction(tr("Si&mple mode from binary file"), this);
        replayAction_->setShortcut(Qt::CTRL + Qt::Key_B);
        replayAction_->setStatusTip(tr("Replay simulation using image file reading data from binary file"));
        QObject::connect(replayAction_, SIGNAL(triggered()), simpleMode_, SLOT(replay()));
        replayAction_->setDisabled(true);*/

        // UDP
        simpleModeFromNetworkAction_ = new QAction(tr("Simple mode using &network"), this);
        simpleModeFromNetworkAction_->setShortcut(Qt::CTRL + Qt::Key_N);
        simpleModeFromNetworkAction_->setStatusTip(tr("Receive data using network"));
        QObject::connect(simpleModeFromNetworkAction_, SIGNAL(triggered()),
                         udpRecvStream0_, SLOT(connectToServer()));
        QObject::connect(simpleModeFromNetworkAction_, SIGNAL(triggered()),
                         udpRecvStream1_, SLOT(connectToServer()));
        QObject::connect(simpleModeFromNetworkAction_, SIGNAL(triggered()),
                         simpleMode_, SLOT(startSimpleModeNetwork()));
        QObject::connect(udpRecvStream0_, SIGNAL(changeAction(bool,int)), this, SLOT(changeAction(bool,int)));
        QObject::connect(simpleModeFromNetworkAction_, SIGNAL(triggered()),
                         simpleMode_->simpleModeDrawer(Stream0), SLOT(checkQuit()));
        QObject::connect(simpleModeFromNetworkAction_, SIGNAL(triggered()),
                         simpleMode_->simpleModeDrawer(Stream1), SLOT(checkQuit()));
        QObject::connect(simpleMode_->simpleModeDrawer(Stream0), SIGNAL(quitApp()),
                         this, SLOT(onQuitApp()));
        QObject::connect(simpleMode_->simpleModeDrawer(Stream1), SIGNAL(quitApp()),
                         this, SLOT(onQuitApp()));

        // Local socket - named pipe
        /*simpleModeFromLocSockThrAction_ = new QAction(tr("Simple mode using &local socket"), this);
        simpleModeFromLocSockThrAction_->setShortcut(Qt::CTRL + Qt::Key_L);
        simpleModeFromLocSockThrAction_->setStatusTip(tr("Receive data using local socket"));
        QObject::connect(simpleModeFromLocSockThrAction_, SIGNAL(triggered()),
                         &locSockRecvThr_, SLOT(connectToServer()));
        QObject::connect(simpleModeFromLocSockThrAction_, SIGNAL(triggered()),
                         simpleMode_, SLOT(startSimpleModeLocSock()));
        simpleModeFromLocSockThrAction_->setDisabled(true);*/

        mapWindowShowAction_ = new QAction(tr("Show the &map window"), this);
        mapWindowShowAction_->setShortcut(Qt::CTRL + Qt::Key_M);
        mapWindowShowAction_->setStatusTip(tr("Show the map window"));
        mapWindowShowAction_->setCheckable(false);
        mapWindowShowAction_->setEnabled(true);
        QObject::connect(mapWindowShowAction_, SIGNAL(triggered()),
                         this, SLOT(mapWindowShow()));

        for(int nr = 0; nr < NrStreams; nr++) {
            showColorbarAction_[nr] = new QAction(tr("Show color&bar for stream %1").arg(nr), this);
            showColorbarAction_[nr]->setStatusTip(tr("Show the colorbar for stream %1").arg(nr));
            showColorbarAction_[nr]->setCheckable(true);
            showColorbarAction_[nr]->setChecked(viewer_config.showColorbar[nr]);
            QObject::connect(showColorbarAction_[nr], SIGNAL(toggled(bool)),
                             simpleMode_->simpleModeDrawer(nr), SLOT(showColorbar(bool)));
        }
        return;
    }


    void MainWindow::createStatusBar() {
        statusBar()->showMessage(tr("Ready"));

        imgXPosLabel_ = new QLabel(" W999 ");
        imgXPosLabel_->setAlignment(Qt::AlignHCenter);
        imgXPosLabel_->setMinimumSize(50, imgXPosLabel_->sizeHint().height());
        imgXPosLabel_->setText("123");

        imgYPosLabel_ = new QLabel(" W999 ");
        imgYPosLabel_->setAlignment(Qt::AlignHCenter);
        imgYPosLabel_->setMinimumSize(50, imgXPosLabel_->sizeHint().height());
        imgYPosLabel_->setText("456");

        statusBar()->addPermanentWidget(imgXPosLabel_, 0);
        statusBar()->addPermanentWidget(imgYPosLabel_, 0);

        return;
    }


    void MainWindow::createView() {
        scene_[Stream0] = new Scene(0, 0, WIDGET_WIDTH_DEF, WIDGET_HEIGHT_DEF, Stream0);
        scene_[Stream1] = new Scene(0, 0, WIDGET_WIDTH_DEF, WIDGET_HEIGHT_DEF, Stream1);
        view_[Stream0] = new View(scene_[Stream0]);
        view_[Stream1] = new View(scene_[Stream1]);
        view_[Stream0]->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate); // SmartViewportUpdate or MinimalViewportUpdate or BoundingRectViewportUpdate FullViewportUpdate
        view_[Stream1]->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate); // SmartViewportUpdate or MinimalViewportUpdate or BoundingRectViewportUpdate FullViewportUpdate
//            view_->setCacheMode( QGraphicsView::CacheBackground );
        view_[Stream0]->setCacheMode(QGraphicsView::CacheNone);
        view_[Stream1]->setCacheMode(QGraphicsView::CacheNone);

//        setCentralWidget(view_);
        view_[Stream0]->setRenderHints(QPainter::Antialiasing); // view_->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        view_[Stream1]->setRenderHints(QPainter::Antialiasing); // view_->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        setUpdatesEnabled(true);
        streamDockWidget_[Stream0]->setWidget(view_[Stream0]);
        streamDockWidget_[Stream1]->setWidget(view_[Stream1]);
//        view_->setViewport(new QGLWidget); // More quick without it
        view_[Stream0]->show();
        view_[Stream1]->show();

        return;
    }


    void MainWindow::createPlatParamDockWidget() {
        paramDockWidget_ = new QDockWidget(tr("Parameters"), this);
        paramDockWidget_->setAllowedAreas(Qt::TopDockWidgetArea);
        paramDockWidget_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        QWidget *param_widget = new QWidget;

        // Current time
        QGroupBox *curr_time_group_box = new QGroupBox(param_widget);
        curr_time_group_box->setTitle(tr("Current time"));
        curr_time_group_box->setFixedHeight(72);

        currTimeTextEdit_ = new QTextEdit(curr_time_group_box);
        currTimeTextEdit_->setReadOnly(true);

        currTimer_ = new QTimer(this);
        QObject::connect(currTimer_, SIGNAL(timeout()), this, SLOT(setTime()));
        currTimer_->start(100);

        QVBoxLayout *curr_time_v_box_layout = new QVBoxLayout(curr_time_group_box);
        curr_time_v_box_layout->addWidget(currTimeTextEdit_);
        curr_time_v_box_layout->setContentsMargins(4,4,4,4);

        // Platform parameters
        QGroupBox *plat_param_group_box = new QGroupBox(param_widget);
        plat_param_group_box->setTitle(tr("Platform parameters"));
        plat_param_group_box->setFixedHeight(137);

        platTextEdit_ = new QTextEdit(plat_param_group_box);
        platTextEdit_->setReadOnly(true);

        QVBoxLayout *plat_param_v_box_layout = new QVBoxLayout(plat_param_group_box);
        plat_param_v_box_layout->addWidget(platTextEdit_);
        plat_param_v_box_layout->setContentsMargins(4,4,4,4);

        // Radar parameters
        QGroupBox *radar_param_group_box = new QGroupBox(param_widget);
        radar_param_group_box->setTitle(tr("Radar parameters"));
        radar_param_group_box->setFixedHeight(170);

        QLabel *anal_sett_label = new QLabel(radar_param_group_box);
        anal_sett_label->setText(tr("Analog settings"));

        for(int cnt = 0; cnt < NR_FLAG_BITS; cnt++) {
            flagsLed_[cnt] = new Led(this);
        }

        QGridLayout *grid_layout = new QGridLayout(radar_param_group_box);
        grid_layout->addWidget(anal_sett_label, 0, 0, 1, -1, Qt::AlignCenter);

        for(int cnt = 0; cnt < NR_FLAG_BITS; cnt++) {
            grid_layout->addWidget(flagsLed_[cnt], 2, cnt);
        }

        radarTextEdit_ = new QTextEdit(radar_param_group_box);
        radarTextEdit_->setReadOnly(true);
        grid_layout->addWidget(radarTextEdit_, 3, 0, 4, -1);
        grid_layout->setContentsMargins(4,4,4,4);

        // GPS parameters
        QGroupBox *gps_param_group_box = new QGroupBox(param_widget);
        gpsAvailLed_ = new Led(this);
        gpsValidLed_ = new Led(this);

        QGridLayout *gps_layout = new QGridLayout(gps_param_group_box);
        QLabel *gps_avail_label = new QLabel(gps_param_group_box);
        gps_avail_label->setText(tr("    GPS\nAvailable"));
        QLabel *gps_valid_label = new QLabel(gps_param_group_box);
        gps_valid_label->setText(tr("GPS\nValid"));

        // diody pod napisami
        gps_layout->addWidget(gps_avail_label, 0, 0, Qt::AlignCenter);
        gps_layout->addWidget(gps_valid_label, 0, 1, Qt::AlignCenter);
        gps_layout->addWidget(gpsAvailLed_, 1, 0, Qt::AlignCenter);
        gps_layout->addWidget(gpsValidLed_, 1, 1, Qt::AlignCenter);
        gps_layout->setContentsMargins(4,4,4,4);

        // All together
        QVBoxLayout *param_layout = new QVBoxLayout;
        param_layout->addWidget(curr_time_group_box);
        param_layout->addWidget(plat_param_group_box);
        param_layout->addWidget(radar_param_group_box);
        param_layout->addWidget(gps_param_group_box);
        param_layout->addStretch();
        param_layout->setContentsMargins(0,0,0,4);

        param_widget->setLayout(param_layout);
        paramDockWidget_->setWidget(param_widget);

#ifdef Q_WS_WIN
        paramDockWidget_->setFixedWidth(150);
#endif
//        paramDockWidget_->setFixedHeight(310);
#ifdef Q_WS_X11
        paramDockWidget_->setFixedHeight(370);
#endif

        paramDockWidget_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);

//        addDockWidget(Qt::TopDockWidgetArea, paramDockWidget_); // Moved to createDockWidgets() function.

        return;
    }


    void MainWindow::createDockWidgets() {
        streamDockWidget_[Stream0] = new DockWidget(tr("Stream 0"), this);
        streamDockWidget_[Stream0]->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
        streamDockWidget_[Stream0]->setMinimumWidth(MIN_WIDTH_DOCK_WIDGET);
        streamDockWidget_[Stream1] = new DockWidget(tr("Stream 1"), this);
        streamDockWidget_[Stream1]->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
        streamDockWidget_[Stream1]->setMinimumWidth(MIN_WIDTH_DOCK_WIDGET);

        addDockWidget(Qt::TopDockWidgetArea, streamDockWidget_[Stream0]);
        addDockWidget(Qt::TopDockWidgetArea, paramDockWidget_);
        addDockWidget(Qt::TopDockWidgetArea, streamDockWidget_[Stream1]);

        return;
    }

    void MainWindow::removeContextMenuFromAllWidgets() {
        QWidgetList widgets = QApplication::allWidgets();
        QWidget* w = 0;
        foreach(w, widgets) {
            w->setContextMenuPolicy(Qt::NoContextMenu);
        }

        return;
    }

    void MainWindow::showMapWindow() {
        mapWindow_->setVisible(true);
    }


    void MainWindow::setPlatParams(DOUBLE lat, DOUBLE lon, FLOAT alt, FLOAT head, FLOAT pitch,
                                   FLOAT roll, FLOAT veloc, UINT32 anal_sett, UINT32 nr_range_cells) {

        platTextEdit_->clear();
        QTextCursor cursor(platTextEdit_->textCursor());
        cursor.movePosition(QTextCursor::Start);

        // Platform parameters
        cursor.insertText(QString("Lat:%1 deg").arg(lat, 0, 'f', 6));
        cursor.insertBlock();
        cursor.insertText(QString("Long:%1 deg").arg(lon, 0, 'f', 6));
        cursor.insertBlock();
        cursor.insertText(QString("Alt:%1 m").arg(alt, 0, 'f', 2));
        cursor.insertBlock();
        cursor.insertText(QString("Head:%1 deg").arg(head, 0, 'f', 2));
        cursor.insertBlock();
        cursor.insertText(QString("Pitch:%1 deg").arg(pitch, 0, 'f', 2));
        cursor.insertBlock();
        cursor.insertText(QString("Roll:%1 deg").arg(roll, 0, 'f', 2));
        cursor.insertBlock();
        cursor.insertText(QString("Velocity:%1 m/s").arg(veloc, 0, 'f', 2));
        cursor.insertBlock();

        // Analog settings
        for(uint cnt = 0; cnt < NR_FLAG_BITS; cnt++){
            flagsLed_[NR_FLAG_BITS - 1 - cnt]->setChecked(BitNtst(anal_sett,cnt));
        }

        // Radar parameters
        radar_pars_fmcw r_pars;
        decode_settings_swap((BYTE)(anal_sett & 0xFF), &r_pars);

        radarTextEdit_->clear();
        QTextCursor r_cursor(radarTextEdit_->textCursor());
        r_cursor.movePosition(QTextCursor::Start);
        r_cursor.insertText(QString("fc : %1 GHz").arg(r_pars.fc_RF/1e9, 0, 'f', 2));
        r_cursor.insertBlock();
        r_cursor.insertText(QString("fc_IF : %1 MHz").arg(r_pars.fc_IF/1e6, 0, 'f', 2));
        r_cursor.insertBlock();
        r_cursor.insertText(QString("B : %1 MHz").arg(r_pars.B/1e6, 0, 'f', 2));
        r_cursor.insertBlock();
        r_cursor.insertText(QString("PRF : %1 Hz").arg((1/(r_pars.Tc + r_pars.Tp)), 0, 'f', 2));
        r_cursor.insertBlock();
        r_cursor.insertText(QString("Dec : %1").arg(r_pars.dec_fact));
        r_cursor.insertBlock();
        if(BitNtst(anal_sett, FLAG_BIT_ANT_DIR_RIGHT))
            r_cursor.insertText(QString("Ant dir : right"));
        else
            r_cursor.insertText(QString("Ant dir : left"));
        r_cursor.insertBlock();
        r_cursor.insertText(QString("Nr range cells:%1").arg(nr_range_cells));

        // GPS settings
        gpsAvailLed_->setChecked(BitNtst(anal_sett,FLAG_BIT_GPS_AVAILABLE));
        gpsValidLed_->setChecked(BitNtst(anal_sett,FLAG_BIT_GPS_VALID));

        return;
    }


    void MainWindow::changeAction(bool val, int kind) {
//        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
        switch(kind) {
        /*case BinaryFile :
            replayAction_->setDisabled(val);
            break;*/
        case UDP :
            simpleModeFromNetworkAction_->setDisabled(val);
            /*simpleModeFromLocSockThrAction_->setDisabled(val);*/
            break;
        /*case NamedPipe :
            simpleModeFromLocSockThrAction_->setDisabled(val);
            simpleModeFromNetworkAction_->setDisabled(val);
            break;*/
        default :
            qDebug() << "There is no such kind of reciving data.";
            break;
        }
    }


    void MainWindow::mapWindowShowEnable() {
//        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
       mapWindowShowAction_->setEnabled(true);
    }


    void MainWindow::about() {
        QMessageBox::information( this, tr( "About Application " ), tr( "<b>SAR VIEWER</b> <br><br> Version: %1.%2 <br><br> The program visualizes SAR's images.").arg( (QString) SAR_VIEWER_VERION_MAJOR, SAR_VIEWER_VERION_MINOR ) );
        return;
    }

    void MainWindow::setTime() {
//        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";

        currTimeTextEdit_->clear();
        QTextCursor cursor(currTimeTextEdit_->textCursor());
        cursor.movePosition(QTextCursor::Start);

        cursor.insertText(englishLocale_.toString(QDateTime::currentDateTime(),"ddd MMMM d yy\nhh:mm:ss"));

        cursor.insertBlock();
    }


    void MainWindow::mapWindowShow() {
//        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
        if(mapWindow_) {
            mapWindow_->setVisible(true);
        }
        else {
            PR_ERROR("Cannot view the map window");
        }
        mapWindowShowAction_->setEnabled(false);
    }


    void MainWindow::onUdpRecvThrFinished0() {
#if DEBUG_MAIN_WINDOW_UDP_FINISHED
        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
        if(udpRecvThr_[Stream0]->isRunning()) {
            QTimer::singleShot(10, this, SLOT(onUdpRecvFinished0()));
        }

        if(closeAllWindows_) {
            if(udpRecvThr_[Stream0]->isRunning()) {
                return;
            }
            else {
                close();
            }
        }
    }


    void MainWindow::onUdpRecvThrFinished1() {
#if DEBUG_MAIN_WINDOW_UDP_FINISHED
        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
        if(udpRecvThr_[Stream1]->isRunning()) {
            QTimer::singleShot(10, this, SLOT(onUdpRecvFinished1()));
        }

        if(closeAllWindows_) {
            if(udpRecvThr_[Stream1]->isRunning()) {
                return;
            }
            else {
                close();
            }
        }
    }


    void MainWindow::onSimpModDrawThrFinished0() {
#if DEBUG_MAIN_WINDOW_DRAW_FINISHED
        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
        if(simpleModeDrawerThr_[Stream0]->isRunning()) {
            QTimer::singleShot(10, this, SLOT(onSimpModDrawThrFinished0()));
        }

        if(closeAllWindows_) {
            if(simpleModeDrawerThr_[Stream0]->isRunning()) {
                return;
            }
            else {
                close();
            }
        }
    }


    void MainWindow::onSimpModDrawThrFinished1() {
#if DEBUG_MAIN_WINDOW_DRAW_FINISHED
        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
        if(simpleModeDrawerThr_[Stream1]->isRunning()) {
            QTimer::singleShot(10, this, SLOT(onSimpModDrawThrFinished1()));
        }

        if(closeAllWindows_) {
            if(simpleModeDrawerThr_[Stream1]->isRunning()) {
                return;
            }
            else {
                close();
            }
        }
    }


    void MainWindow::onPlaneThrFinished() {
#if DEBUG_MAIN_WINDOW_PLANE_FINISHED
        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
        if(planeThr_->isRunning()) {
            QTimer::singleShot(10, this, SLOT(onPlaneThrFinished()));
        }

        if(closeAllWindows_) {
            if(planeThr_->isRunning()) {
                return;
            }
            else {
                close();
            }
        }
    }


    void MainWindow::onQuitApp() {
#if DEBUG_MAIN_WINDOW_DRAW_FINISHED
        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
#endif
        if(!onQuit_) {
            onQuit_ = true;
            QMessageBox msgBox;
            msgBox.setWindowTitle("Error");
            msgBox.setText("Bad allocation. The application has to be closed.");
            msgBox.setInformativeText("Try run the SAR viewer again.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Critical);
            int ret = msgBox.exec();

            switch (ret) {
            case QMessageBox::Ok :
                // Ok was clicked
                close();
                break;
            default :
                break;
            }
        }
    }


    QString MainWindow::getSARDataFileName() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open file with SAR data"), QDir::currentPath());
        return fileName;
    }

} // namespace SarViewer
