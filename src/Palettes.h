/// @file Palettes.h
/// @brief class managing palettes
/// @author Krzysztof Borowiec

#ifndef PALETTE_H
#define PALETTE_H

#include <QVector>
#include <QRgb>

namespace SarViewer {

    enum PaletteType{
        PaletteJet = 0,
        PaletteGray,
        PaletteGrayInverse,
        PaletteGreen
    };

    const int ALPHA_IMAGES = 255;

    class Palettes {
    public:
        static QVector<QRgb> *getGray();
        static QVector<QRgb> *getGrayInverse();
        static QVector<QRgb> *getJet();
        static QVector<QRgb> *getGreen();
        static QVector<QRgb> *getPalette(int paletteType);
    };

} // namespace SarViewer

#endif // PALETTE_H
