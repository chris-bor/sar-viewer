#include <qdebug.h>
#include <qmutex.h>
#include "PlotSettings.h"
#include "Debugging.h"
#include "Line.h"
#include "Config.h"

using namespace SarViewer;

QVector<PlotSettings> zoomStack[NrStreams]; // For storing zooms
int currZoom[NrStreams] = {0, 0}; // For managing the current zoom
QMutex currZoomMux; // Form blocking currZoom during change it's value // TODO  block zoomIn i Out

SarViewer::PlotSettings::PlotSettings() {
#if DEBUG_CONST
    qDebug() << __FUNCTION__ << "()";
#endif

    minXRound_ = 0;
    maxXRound_ = SarViewer::NR_PIX_PER_LINE;
    minX_ = 0.0;
    maxX_ = (double)SarViewer::MAX_PIX_PER_LINE; // nr_range_cells
    numXTicks_ = 10;
    minY_ = 0.0;
    maxY_ = (double)NR_LINES_PER_SCREEN;
    numYTicks_ = 10;
}

void SarViewer::PlotSettings::scroll(int dx, int dy) {
}


void SarViewer::PlotSettings::adjust() {
}
