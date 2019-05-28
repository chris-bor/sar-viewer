#ifndef LINE_H
#define LINE_H

#include "sar_image.h"
#include "qglobal.h"

#define DEF_CELL_SIZE 0.3 // [m]
#define DEF_R_MIN 0.0 // [m]
#define DEF_PRF 100.0 // [Hz]

#define MIN_NR_LINES_PER_SCREEN     1000
#define MAX_NR_LINES_PER_SCREEN     100000

//#define NR_LINES_OLD_DATA           2 * NR_LINES_PER_SCREEN // size for old data which was shown at past stored at the buffer
#define NR_LINES_OLD_DATA           NR_LINES_PER_SCREEN // size for old data which was shown at past stored at the buffer
#define NR_LINES_CIRC_BUFFER        NR_LINES_OLD_DATA + 2 * NR_LINES_PER_SCREEN // size of direct buffer multiple of data blocks
#define NR_LINES_DIRE_BUFFER        2 * NR_LINES_PER_SCREEN // size of direct buffer

namespace SarViewer {
    const int NR_PIX_PER_LINE = 2500; // number pixels per one line 1250 || 2500
    const int MAX_PIX_PER_LINE = 2500; // Maximum number of pixels per line

//#ifdef _MSC_VER
//#pragma pack(push, r1, 1)
//#endif
    #pragma pack(1)
    struct Line_{
        sar_image_line_header sarImageLineHeader;
        quint8 pixels[MAX_PIX_PER_LINE]; // Line with maximal size
    }
//#ifdef __GNUC__
//    __attribute__((__packed__))
//#elif defined(_MSC_VER)
//#pragma pack(pop, r1)
//#else
//    #error "Cannot eliminate structure padding"
//#endif
;
    #pragma pack()

//    __attribute__ ((packed));
    typedef Line_ Line;

    const uint SIZE_OF_LINE = sizeof(Line); // Size of line
    const uint MAX_SIZE_OF_LINE = SIZE_OF_IMG_HEADER + MAX_PIX_PER_LINE; // Maximum size of line
} // namespace SarViewer
#endif // LINE_H
