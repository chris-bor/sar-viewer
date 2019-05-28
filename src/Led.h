#ifndef LED_H
#define LED_H

#include <qcolor.h>
#include "WidgetWithBackground.h"

namespace SarViewer {
    const QSize LED_SIZE(20, 20);

    class Led : public WidgetWithBackground {
        Q_OBJECT
    public :
        Led(QWidget *parent = 0);
        virtual ~Led();
        void paintEvent(QPaintEvent *event);
        bool getChecked() const { return(checked_); }
        QColor getColor() const { return(color_); }
        void setColor(QColor val);
        QSize minimumSizeHint() const;
        QSize sizeHint() const;

    public slots :
        void setChecked(bool val);

    signals :
        void checkChanged(bool val);

    protected :
        void initCoord(QPainter &painter);
        void paintBackground(QPainter &painter);

    private :
        bool checked_;
        QColor color_;
    };
} // namespace SarViewer

#endif // LED_H
