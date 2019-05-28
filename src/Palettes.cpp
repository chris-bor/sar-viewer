#include <QColor>
#include <QDebug>
#include "Palettes.h"

namespace SarViewer {

    QVector<QRgb> *Palettes::getGray() {
        QVector<QRgb> *palette;
        palette = new QVector<QRgb>(256);
        for(int i = 0; i < 256; i++) {
//            (*palette)[i] = qRgba(i,i,i, SarViewer::ALPHA_IMAGES);
            (*palette)[i] = (0xff << 24) | ((i & 0xff) << 16) | ((i & 0xff) << 8) | (i & 0xff);
//            (*palette)[i] = qRgb(i,i,i);
        }
        return palette;
    }


    QVector<QRgb> *Palettes::getGrayInverse() {
        QVector<QRgb> *palette;
        palette = new QVector<QRgb>(256);
//        unsigned char r[256];
//        unsigned char g[256];
//        unsigned char b[256];
//        QVector<QRgb> *paletteNew;
//        paletteNew = new QVector<QRgb>(256);
        int val;

        for (int k = 0; k < 256; k++) {
//            r[k] = 255-k;
//            g[k] = 255-k;
//            b[k] = 255-k;
//            (*palette)[k] = qRgba(r[k],g[k],b[k], SarViewer::ALPHA_IMAGES);
//            (*palette)[k] = qRgb(r[k],g[k],b[k]);

            val = 255 - k;
            (*palette)[k] = (0xff << 24) | ((val & 0xff) << 16) | ((val & 0xff) << 8) | (val & 0xff);
//            qDebug() << "k" << k << (*palette)[k];
//            qDebug("k = %d : 0x%x",k,(*palette)[k]);
//            (*paletteNew)[k] = (0xffu << 24) | ((val & 0xff) << 16) | ((val & 0xff) << 8) | (val & 0xff);
        }
//        qDebug("0x%x",(*paletteNew)[0]);

//        Q_GUI_EXPORT_INLINE QRgb qRgb(int r, int g, int b)// set RGB value
//        { return (0xffu << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

//        Q_GUI_EXPORT_INLINE QRgb qRgba(int r, int g, int b, int a)// set RGBA value
//        { return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }
//        qDebug() << "SSSSS" << (0xffu << 24);

//        for (int k = 0; k < 256; k++) {
//            (*palette)[k] = qRgb(r[k],g[k],b[k]);
//        }

        return palette;
    }


    QVector<QRgb> *Palettes::getJet() {
        QVector<QRgb> *palette;
        unsigned char r[256];
        unsigned char g[256];
        unsigned char b[256];
        int k;

        for (k = 0; k < 93; k++)
            r[k] = 0;
        for (k = 93; k < 158; k++)
            r[k] = (unsigned char)(255.0*(k-93.0)/(158.0-93.0));
        for (k = 158; k < 222; k++)
            r[k] = 255;
        for (k = 222; k < 256; k++)
            r[k] = (unsigned char)((222.0-k)/(255.0-222.0)*127.0+255.0);

        for (k = 0; k < 27; k++)
            g[k] = 0;
        for (k = 27; k < 93; k++)
            g[k] = (unsigned char)(255.0*(k-27.0)/(93.0-27.0));
        for (k = 93; k < 158; k++)
            g[k] = 255;
        for (k = 158; k < 222; k++)
            g[k] = (unsigned char)((158.0-k)/(222.0-158.0)*255.0+255.0);
        for (k = 222; k < 256; k++)
            g[k] = 0;

        for (k = 0; k < 27; k++)
            b[k] = (unsigned char)(127.0*((float)k)/(27.0)+127.0);
        for (k = 27; k < 93; k++)
            b[k] = 255;
        for (k = 93; k < 158; k++)
            b[k] = (unsigned char)((93.0-k)/(158.0-93.0)*255.0+255.0);
        for (k = 158; k < 256; k++)
            b[k] = 0;

        palette = new QVector<QRgb>(256);

        for(k = 0; k < 256; k++) {
//            (*palette)[k] = qRgba(r[k],g[k],b[k], SarViewer::ALPHA_IMAGES);
//            (*palette)[k] = qRgb(r[k],g[k],b[k]);
            (*palette)[k] = (0xff << 24) | ((r[k] & 0xff) << 16) | ((g[k] & 0xff) << 8) | (b[k] & 0xff);
        }

        return palette;
    }


    QVector<QRgb> *Palettes::getGreen() {
        QVector<QRgb> *palette;
        palette = new QVector<QRgb>(256);
        for(int i = 0; i < 256; i++) {
//            (*palette)[i] = qRgba(0,i,0, SarViewer::ALPHA_IMAGES);
//            (*palette)[i] = qRgb(0,i,0);
            (*palette)[i] = (0xff << 24) | ((0 & 0xff) << 16) | ((i & 0xff) << 8) | (0 & 0xff);
        }
        return palette;
    }


    QVector<QRgb> *Palettes::getPalette(int paletteType) {
        switch (paletteType) {
        case SarViewer::PaletteJet:
            return Palettes::getJet();
        case SarViewer::PaletteGray:
            return Palettes::getGray();
        case SarViewer::PaletteGrayInverse:
            return getGrayInverse();
        case SarViewer::PaletteGreen:
            return getGreen();
        default:
            return Palettes::getJet();
        }
    }
}
