/// @file DockWidget.h
/// @brief class necessary for 2 stream data. Needed some reimplemented functions. Bases on QDockWidget.
/// @author Krzysztof Borowiec

#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <qdockwidget.h>
#include <QtGui>

namespace SarViewer {
    const uint MIN_WIDTH_DOCK_WIDGET = 300;

    class DockWidget : public QDockWidget {

        Q_OBJECT

    public :
        DockWidget(const QString & title, QWidget *parent = 0, Qt::WindowFlags flags = 0);
        ~DockWidget();

    signals :
        void resized(QSize size);

    private slots :
        void floatingChanged(bool change);

    protected :
        void resizeEvent(QResizeEvent *event);
    };

} // namespace SarViewer

#endif // DOCKWIDGET_H
