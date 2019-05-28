TEMPLATE = app

HEADERS += \
    MainWindow.h \
    Config.h \
    View.h \
    SimpleMode.h \
    SarImageReader.h \
    sar_image.h \
    Palettes.h \
    Line.h \
    SarImageDataBuffer.h \
    NetworkReceiver.h \
    ConnectWindow.h \
    TimeMeasure.h \
    LocalSocketReceiver.h \
    LocalSocketConnectWindow.h \
    Debugging.h \
    LocalSocketReceiverThread.h \
    PlotSettings.h \
    DockWidget.h \
    WidgetWithBackground.h \
    Led.h \
    uni_types.h \
    sar_types.h \
    propatria_types.h \
    sar_types.h \
    fftw3.h \
    Scene.h \
    MapWindow.h \
    qcustomplot.h \
    MapViewer.h \
    ConfigParser.h \
    GeoConv.h \
    Plane.h \
    MapTextItem.h \
    MapPointItem.h \
    StripScan.h \
    StripScanImage.h \
    SimpleModeDrawer.h \
    UdpReceiver.h \
    GoogleMaps/GoogleMaps.h \
    GoogleMaps/GoogleMapsItem.h
SOURCES +=  \
    Main.cpp \
    MainWindow.cpp \
    View.cpp \
    SimpleMode.cpp \
    SarImageReader.cpp \
    Palettes.cpp \
    SarImageDataBuffer.cpp \
    NetworkReceiver.cpp \
    ConnectionWindow.cpp \
    TimeMeasure.cpp \
    LocalSocketReceiver.cpp \
    LocalSocketConnectWindow.cpp \
    LocalSocketReceiverThread.cpp \
    PlotSettings.cpp \
    DockWidget.cpp \
    WidgetWithBackground.cpp \
    Led.cpp \
    propatria_types.c \
    Scene.cpp \
    MapWindow.cpp \
    qcustomplot.cpp \
    MapViewer.cpp \
    ConfigParser.cpp \
    GeoConv.cpp \
    Plane.cpp \
    MapTextItem.cpp \
    MapPointItem.cpp \
    StripScan.cpp \
    StripScanImage.cpp \
    SimpleModeDrawer.cpp \
    UdpReceiver.cpp \
    GoogleMaps/GoogleMaps.cpp \
    GoogleMaps/GoogleMapsItem.cpp

# VS2010 Command line: -Zm200 -O3 -w34100 -w34189
LIBS += -lws2_32
#LIBS += -llibfftw3f-3
#LIBS += -lm
#LIBS += -Le:\praca\wskaznik_SAR\src\libfftw3f-3

CONFIG += qt console thread
#CONFIG += static thread x86 rtti warn_on thread
QT += network gui core webkit

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/SAR
#sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS SAR_viewer.pro
#sources.path = $$[QT_INSTALL_EXAMPLES]/SAR
#INSTALLS += target sources

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
#QMAKE_CXXFLAGS_DEBUG += -g

# C++11
#if win32{
#    QMAKE_CXXFLAGS += -std=gnu++0x
#}
#else {
#    QMAKE_CXXFLAGS += -std=c++0x
#}

#QMAKE_CXXFLAGS += -std=c++0x
#QMAKE_CXXFLAGS += -std=c++11

RESOURCES += \
    SarViewer.qrc

RC_FILE = SarViewer.rc
