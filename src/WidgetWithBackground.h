/// @file WidgetWithBackground.h
/// @brief basic class to build widget such as class Led. Class bases on QWidget.
/// @author Krzysztof Borowiec

#ifndef WIDGETWITHBACKGROUND_H
#define WIDGETWITHBACKGROUND_H

#include <qwidget.h>

class QPixmap;

namespace SarViewer {
    class WidgetWithBackground : public QWidget {
        Q_OBJECT

    public :
        WidgetWithBackground(QWidget *parent = 0);
        virtual ~WidgetWithBackground();
        void  drawBackground(); // Draws if modified or size changed
        void  updateWithBackground(); // Calls update()
        bool getModified() const { return(modified_); }

    protected :
        void repaintBackground(); // Calls paintBackground()
        virtual void  paintBackground(QPainter &painer) = 0;

    private :
        QPixmap *pixmap_; // Buffer for background
        bool modified_;
    };
}
#endif // WIDGETWITHBACKGROUND_H
