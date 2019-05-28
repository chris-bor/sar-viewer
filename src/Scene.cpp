#include <QtGui>
#include "Scene.h"
#include "Config.h"

using namespace SarViewer;

extern QVector<PlotSettings> zoomStack[NrStreams];
extern int currZoom[NrStreams];
extern ViewerConfig viewer_config;

// TODO w konstruktorze wskaŸnik do parenta ¿eby ustawiaæ kursor
SarViewer::Scene::Scene(qreal x, qreal y, qreal width, qreal height, int nr_stream, QObject *parent) :
    QGraphicsScene(x, y, width, height, parent), nrStream_(nr_stream) {
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF Scene\n";
#endif

        // Init zoom stack
        setPlotSettings(PlotSettings());
        rectItem_ = NULL;
        rubberBandPen_.setWidth(1);
        rubberBandPen_.setStyle(Qt::DashLine);
        rubberBandPen_.setColor(Qt::white);

        colorbar_ = viewer_config.showColorbar[nrStream_];
        if(colorbar_) {
            colorbarWidth_ = ColorbarWidth;
        }
        else {
            colorbarWidth_ = 0;
        }
}

SarViewer::Scene::~Scene() {
#if DEBUG_DESTR
        qDebug() << "DESTRUCTOR OF Scene\n";
#endif
}

void Scene::setPlotSettings(const PlotSettings &settings) {
    zoomStack[nrStream_].clear();
    zoomStack[nrStream_].append(settings);
    currZoom[nrStream_] = 0;
}

void Scene::zoomIn() {
    if (currZoom[nrStream_] < zoomStack[nrStream_].count() - 1) {
        qDebug() << "Zoom in";
        ++currZoom[nrStream_];
        emit zoomChanged();
    }
}

void Scene::zoomOut() {
    if (currZoom[nrStream_] > 0) {
        qDebug() << "Zoom out";
        --currZoom[nrStream_];
        emit zoomChanged();
    }
}


void Scene::showColorbar(bool toggled) {
    qDebug() << __FUNCTION__ << "()";
    colorbar_ = toggled;
    if(colorbar_) {
        colorbarWidth_ = ColorbarWidth;
    }
    else {
        colorbarWidth_ = 0;
    }
}

void SarViewer::Scene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
#if DEBUG_SAR_SCENE
//        qDebug() << __FUNCTION__ << "()";
#endif

    QRectF rect(MarginLeft, MarginTop, width() - MarginLeft - MarginRight - colorbarWidth_, height() - MarginTop - MarginBottom);

    if(event->button() == Qt::LeftButton) { // If left button
        if(rect.contains(event->scenePos())) { // If event on the image
            rubberBandIsShown_ = true;
            rubberBandRect_.setTopLeft(event->scenePos());
            rubberBandRect_.setBottomRight(event->scenePos());
            if(!rectItem_)
                rectItem_ = addRect(rubberBandRect_, rubberBandPen_, rubberBandBrush_);
            rectItem_->show();
        }
    }
}

void SarViewer::Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
#if DEBUG_SAR_SCENE
//        qDebug() << __FUNCTION__ << "()";
#endif
    if (rubberBandIsShown_) {
        rubberBandRect_.setBottomRight(event->scenePos());
        rectItem_->setRect(rubberBandRect_.intersected(QRectF(MarginLeft, MarginTop, width() - MarginLeft - MarginRight - colorbarWidth_, height() - MarginTop - MarginBottom)));
    }
}

void SarViewer::Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
#if DEBUG_SAR_SCENE
//        qDebug() << __FUNCTION__ << "()";
#endif
    if((event->button() == Qt::LeftButton) && rubberBandIsShown_) {
        rubberBandIsShown_ = false;
        rectItem_->setRect(0,0,0,0);
        rectItem_->hide();

        QRectF rect = rubberBandRect_.normalized();
        QRectF scene_rect(MarginLeft, MarginTop, width() - MarginLeft - MarginRight - colorbarWidth_, height() - MarginTop - MarginBottom);
        rect = scene_rect.intersected(rect);
        if (rect.width() < MIN_ZOOM_X || rect.height() < MIN_ZOOM_Y) // If small rubberBandRect
            return;

        // Conversion from scene to viewer coordinates :
        rect.translate(-MarginLeft, -MarginTop);

        PlotSettings prevSettings = zoomStack[nrStream_][currZoom[nrStream_]];
        PlotSettings settings;
        double dx = prevSettings.spanX() / (width() - MarginLeft - MarginRight - colorbarWidth_);
        double dy = prevSettings.spanY() / (height() - MarginTop - MarginBottom);

        settings.minX_ = prevSettings.minX_ + dx * rect.left();
        settings.maxX_ = prevSettings.minX_ + dx * rect.right();
        settings.minY_ = prevSettings.minY_ + dy * rect.top();
        settings.maxY_ = prevSettings.minY_ + dy * rect.bottom();

        settings.minXRound_ = qRound(settings.minX_);
        settings.maxXRound_ = qRound(settings.maxX_);
        settings.minYRound_ = qRound(settings.minY_);
        settings.maxYRound_ = qRound(settings.maxY_);

        zoomStack[nrStream_].resize(currZoom[nrStream_] + 1);
        if(settings.spanX() >= 1 && settings.spanY() >= 1)
            zoomStack[nrStream_].append(settings);
        else
            qDebug() << "[ Stream" << nrStream_ << "] Zoom not available : selected area too little.";
        zoomIn();
    }

    if((event->button() == Qt::RightButton)) {
        zoomOut();
    }
}

