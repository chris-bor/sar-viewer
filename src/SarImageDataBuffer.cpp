#include <string.h>
#include <qmutex.h>
#include "SarImageDataBuffer.h"
#include "Config.h"
#include "Debugging.h"
#include "Line.h"

namespace SarViewer {
    QMutex sar_img_data_buff_mux[NrStreams]; // mutex to critical section to buffer access

    SarImageDataBuffer::SarImageDataBuffer() :
        QObject(), SIZE_(NR_LINES_PER_SCREEN), mostActualLine_(NR_LINES_PER_SCREEN-2), startPresent_(-1) {
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF SarImageDataBuffer\n";
#endif
        linesBuffer_ = new Line[NR_LINES_PER_SCREEN];

        resetBuffer();
    }


    SarImageDataBuffer::SarImageDataBuffer(int size, int nr_stream) : QObject(), SIZE_(size),
        mostActualLine_(2*NR_LINES_PER_SCREEN-2), startPresent_(-1),nrStream_(nr_stream) {
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF SarImageDataBuffer\n";
#endif
        linesBuffer_ = new Line[SIZE_];
        qDebug() << "Buffer for " << SIZE_ << "lines allocated.";
        resetBuffer();
    }


    SarImageDataBuffer::~SarImageDataBuffer() {
#if DEBUG_DESTR
//        qDebug() << "DESTRUCTOR OF SarImageDataBuffer\n";
#endif
        delete [] linesBuffer_;
        linesBuffer_ = NULL;
    }


    int SarImageDataBuffer::setLine(Line *one_line) {
#if DEBUG_SAR_IMG_DATA_BUFFER
//        qDebug() << __FUNCTION__ << "()";
#endif
        mostActualLine_ = ((mostActualLine_ + 1) % SIZE_);
        startPresent_ = ((startPresent_ + 1)% SIZE_);

        // rewrite header
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.magic_id = one_line->sarImageLineHeader.magic_id;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.lat_plat = one_line->sarImageLineHeader.lat_plat;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.lon_plat = one_line->sarImageLineHeader.lon_plat;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.lat_ref = one_line->sarImageLineHeader.lat_ref;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.lon_ref = one_line->sarImageLineHeader.lon_ref;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.alt_sea = one_line->sarImageLineHeader.alt_sea;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.alt_ground = one_line->sarImageLineHeader.alt_ground;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.cell_size = one_line->sarImageLineHeader.cell_size;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.r_min = one_line->sarImageLineHeader.r_min;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.head = one_line->sarImageLineHeader.head;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.head_ref = one_line->sarImageLineHeader.head_ref;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.pitch = one_line->sarImageLineHeader.pitch;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.roll = one_line->sarImageLineHeader.roll;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.veloc = one_line->sarImageLineHeader.veloc;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.prf = one_line->sarImageLineHeader.prf;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.flags = one_line->sarImageLineHeader.flags;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.nr_range_cells = one_line->sarImageLineHeader.nr_range_cells;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.line_cnt = one_line->sarImageLineHeader.line_cnt;
        this->linesBuffer_[mostActualLine_].sarImageLineHeader.block_cnt = one_line->sarImageLineHeader.block_cnt;

        // rewrite pixels value
        for(uint y = 0; y < one_line->sarImageLineHeader.nr_range_cells; y++) {
            this->linesBuffer_[mostActualLine_].pixels[y] = one_line->pixels[y];
        }

        return mostActualLine_;
    }


    void SarImageDataBuffer::resetBuffer() {
#if DEBUG_SAR_IMG_DATA_BUFFER
        qDebug() << __FUNCTION__ << "()";
#endif
        for(int nr_line = 0; nr_line < SIZE_; nr_line++) {
            for(int c = 0; c < NR_PIX_PER_LINE; c++) {
                linesBuffer_[nr_line].pixels[c] = 0;
            }
            linesBuffer_[nr_line].sarImageLineHeader.cell_size = DEF_CELL_SIZE;
            linesBuffer_[nr_line].sarImageLineHeader.nr_range_cells = SarViewer::NR_PIX_PER_LINE;
            linesBuffer_[nr_line].sarImageLineHeader.r_min = DEF_R_MIN;
            linesBuffer_[nr_line].sarImageLineHeader.prf = DEF_PRF;
        }
    }

    void SarImageDataBuffer::resetBufferPix()
    {
#if DEBUG_SAR_IMG_DATA_BUFFER
        qDebug() << __FUNCTION__ << "()";
#endif
        for(int nr_line = 0; nr_line < SIZE_; nr_line++) {
            for(int c = 0; c < NR_PIX_PER_LINE; c++) {
                linesBuffer_[nr_line].pixels[c] = 0;
            }
        }
    }


    void SarImageDataBuffer::copyNewScanStrip(SarImageDataBuffer &strip_buf, int most_actual_line) {
//        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
        // If copied area is dividet to two parts
//                    |  most actual line
//                   \|/
//        *****************************************           *****************************
//        *           *            *              *           *              **           *
//        *   young   *            *     old      *   =>      *     old      **   young   *
//        *           *            *              *           *              **           *
//        *****************************************           *****************************
//        qDebug() << "$$$" << (most_actual_line - strip_buf.SIZE_ < 0);
        if(most_actual_line - strip_buf.SIZE_ < 0) {
            // Young part
            memcpy(&(strip_buf.getBuffer()[strip_buf.SIZE_ - 1 - most_actual_line]),
                   &(this->getBuffer()[0]),
                   ((most_actual_line+1)*SIZE_OF_LINE));
            // Old part
            memcpy(&(strip_buf.getBuffer()[0]),
                   &(this->getBuffer()[this->SIZE_ - 1 - (strip_buf.SIZE_ - 1 - most_actual_line - 1)]),
                   (strip_buf.SIZE_ - 1 - most_actual_line)*SIZE_OF_LINE);
//            qDebug() << "*** RAZEM SKOPIOWANE (PODZIAL)" << (most_actual_line+1) + (strip_buf.SIZE_ - 1 - most_actual_line);
        }
        else {
//                                       |  most actual line
//                                      \|/
//        *****************************************           *****************************
//        *     *                        *        *           *                           *
//        *     *         whole          *        *   =>      *         whole             *
//        *     *                        *        *           *                           *
//        *****************************************           *****************************
            memcpy(&(strip_buf.getBuffer()[0]),
                   &(this->getBuffer()[most_actual_line - (strip_buf.SIZE_ - 1)]),
                   (strip_buf.SIZE_*SIZE_OF_LINE));
//            qDebug() << "*** RAZEM SKOPIOWANE (CALOSC)" << strip_buf.SIZE_;
        }
    }


    void SarImageDataBuffer::copyNewScanStrip(sar_image_line_header *headers, quint8 *pixels, int most_actual_line, int size) {
//        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
        int k;
        // If copied area is dividet to two parts
//                    |  most actual line
//                   \|/
//        *****************************************           *****************************
//        *           *            *              *           *              **           *
//        *   young   *            *     old      *   =>      *     old      **   young   *
//        *           *            *              *           *              **           *
//        *****************************************           *****************************
//        qDebug() << "$$$" << (most_actual_line - strip_buf.SIZE_ < 0);
        if(most_actual_line - size < 0) {
            // Young part
            for(k = 0; k < (most_actual_line+1); k++) {
                // One header
                memcpy(&headers[size - 1 - most_actual_line + k],
                       &(this->linesBuffer_[k]),
                       SIZE_OF_IMG_HEADER);
                // Nr_range_cells pixels
                memcpy(&pixels[size - 1 - most_actual_line + k*MAX_PIX_PER_LINE],
                       (this->linesBuffer_[k].pixels),
                       sizeof(quint8)*(this->linesBuffer_[k].sarImageLineHeader.nr_range_cells));
//                qDebug() << "k" << k << "nr_range_cells" << (this->linesBuffer_[k].sarImageLineHeader.nr_range_cells);
            }
            // Old part
            for(k = 0; k < (size - 1 - most_actual_line); k++) {
                // One header
                memcpy(&headers[k],
                       &(this->linesBuffer_[this->SIZE_ - 1 - (size - 1 - most_actual_line - 1) + k]),
                       SIZE_OF_IMG_HEADER);
                // Nr_range_cells pixels
                memcpy(&pixels[k*MAX_PIX_PER_LINE],
                       this->linesBuffer_[this->SIZE_ - 1 - (size - 1 - most_actual_line - 1) + k].pixels,
                       sizeof(quint8)*(this->linesBuffer_[this->SIZE_ - 1 - (size - 1 - most_actual_line - 1) + k].sarImageLineHeader.nr_range_cells));
            }
            /*
            memcpy(&(strip_buf.getBuffer()[0]),
                   &(this->getBuffer()[this->SIZE_ - 1 - (strip_buf.SIZE_ - 1 - most_actual_line - 1)]),
                   (strip_buf.SIZE_ - 1 - most_actual_line)*SIZE_OF_LINE);
                   */
//            qDebug() << "*** RAZEM SKOPIOWANE (PODZIAL)" << (most_actual_line+1) + (strip_buf.SIZE_ - 1 - most_actual_line);
        }
        else {
//                                       |  most actual line
//                                      \|/
//        *****************************************           *****************************
//        *     *                        *        *           *                           *
//        *     *         whole          *        *   =>      *         whole             *
//        *     *                        *        *           *                           *
//        *****************************************           *****************************
            for(k = 0; k < size; k++) {
                // One header
                memcpy(&headers[k],
                       &(this->linesBuffer_[most_actual_line - (size - 1) + k]),
                       SIZE_OF_IMG_HEADER);
                // Nr_range_cells pixels
                memcpy(&pixels[k*MAX_PIX_PER_LINE],
                       this->linesBuffer_[most_actual_line - (size - 1) + k].pixels,
                       sizeof(quint8)*(this->linesBuffer_[most_actual_line - (size - 1) + k].sarImageLineHeader.nr_range_cells));
            }

            /*memcpy(&(strip_buf.getBuffer()[0]),
                   &(this->getBuffer()[most_actual_line - (strip_buf.SIZE_ - 1)]),
                   (strip_buf.SIZE_*SIZE_OF_LINE));
            */
//            qDebug() << "*** RAZEM SKOPIOWANE (CALOSC)" << strip_buf.SIZE_;
        }
    }


    quint8 &SarImageDataBuffer::getPixel(const int &x, const int &y) {
//        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";

        return(linesBuffer_[y].pixels[x]);
    }


    void SarImageDataBuffer::copyToDirectBuffer(SarImageDataBuffer &direct_buff) {
#if DEBUG_SAR_IMG_DATA_BUFFER
        qDebug() << __FUNCTION__ << "()";
#endif
        int most_actual_line = this->mostActualLine_;
        int k;

//        t1 = clock();
        if(direct_buff.linesBuffer_ && (most_actual_line != (NR_LINES_PER_SCREEN-1 ))) {
        // old part
            for(k = 0; k < (NR_LINES_PER_SCREEN-(most_actual_line+1)); k++) {
                memcpy(&(direct_buff.getBuffer()[k]), &(this->getBuffer()[most_actual_line+1+k]), sizeof(Line));
            }
//        memcpy(&(direct_buff.linesBuffer_[0]), &(this->linesBuffer_[most_actual_line+1]), ((MAX_LINES-(most_actual_line+1))*sizeof(Line)));

        // new part
//        memcpy(&(direct_buff.linesBuffer_[(MAX_LINES-(most_actual_line+1))]), &(this->linesBuffer_[0]), (most_actual_line+1));
//            memcpy(&(direct_buff.getBuffer()[0]),&(this->getBuffer()[0]), sizeof(SarImageDataBuffer));
            for(k = 0; k < (most_actual_line+1); k++) {
                memcpy(&(direct_buff.getBuffer()[NR_LINES_PER_SCREEN-(most_actual_line+1)+k]), &(this->getBuffer()[k]), sizeof(Line));
            }
            direct_buff.mostActualLine_ = NR_LINES_PER_SCREEN - 1;
        }
        else if(direct_buff.linesBuffer_ && (most_actual_line == (NR_LINES_PER_SCREEN-1 ))) {
            for(k = 0; k < NR_LINES_PER_SCREEN; k++) {
                memcpy(&(direct_buff.getBuffer()[k]), &(this->getBuffer()[k]), sizeof(Line));
            }
//            memcpy(&(direct_buff.linesBuffer_[0]),&(this->linesBuffer_[0]), sizeof(SarImageDataBuffer));
        }
        else {
            qDebug("Error: bad allocation of direct buffer");
        }

//        for(int z = 0; z < MAX_LINES; z++) {
//            PR(z);
//            PR(this->linesBuffer_[z].sarImageLineHeader.line_cnt);
//            PR(direct_buff.linesBuffer_[z].sarImageLineHeader.line_cnt);
//        }
//        t2 = clock();
//        qDebug( "copyToDirectBuffer(): %.3f s\n", (float)(t2-t1)/(float)CLOCKS_PER_SEC);
    }


    void SarImageDataBuffer::copyToDirectBufferWithLap(SarImageDataBuffer &direct_buff) {
#if DEBUG_SAR_IMG_DATA_BUFFER
//        qDebug() << __FUNCTION__ << "()";
#endif
        int most_actual_line = this->mostActualLine_;
        int start_present = this->startPresent_;

//        if(!direct_buff.linesBuffer_) { // Check if direct buffer exists
//            qDebug("Error: bad allocation of direct buffer");
//            exit(-1);
//        }

        if(most_actual_line > start_present && start_present >= 0) { // normal copy two last screens
            memcpy(&(direct_buff.getBuffer()[0]),
                   &(this->getBuffer()[start_present]),
                   ((most_actual_line - start_present + 1)*SIZE_OF_LINE));
            qDebug() << "\n STREAM" <<nrStream_;
            qDebug() << "SOURCE" << start_present
                     << "RAZEM" << (most_actual_line - start_present + 1);
        }

        if(most_actual_line < start_present && start_present >= 0) { // buffer is written from the beginning
            memcpy(&(direct_buff.getBuffer()[0]),
                   &(this->getBuffer()[start_present]),
                   ((SIZE_ - start_present)*SIZE_OF_LINE)); // older part
            qDebug() << "\n STREAM" << nrStream_;
            qDebug() << "SOURCE" << start_present
                     << "NUMBER" << (SIZE_ - start_present);
            memcpy(&(direct_buff.getBuffer()[SIZE_ - start_present]),
                   &(this->getBuffer()[0]),
                   ((most_actual_line + 1)*SIZE_OF_LINE)); // younger part
            qDebug() << "DESTINATION" << SIZE_ - start_present
                     << "NUMBER" << (most_actual_line + 1)
                     << "RAZEM" << (SIZE_ - start_present)+(most_actual_line + 1);
        }


/*
        if(direct_buff.linesBuffer_ && (most_actual_line != (SIZE_-1))) {
        // old part
            for(k = 0; k < (SIZE_-(most_actual_line+1)); k++) {
                memcpy(&(direct_buff.getBuffer()[k]), &(this->getBuffer()[most_actual_line+1+k]), sizeof(Line));
            }
//        memcpy(&(direct_buff.linesBuffer_[0]), &(this->linesBuffer_[most_actual_line+1]), ((MAX_LINES-(most_actual_line+1))*sizeof(Line)));

        // new part
//        memcpy(&(direct_buff.linesBuffer_[(MAX_LINES-(most_actual_line+1))]), &(this->linesBuffer_[0]), (most_actual_line+1));
//            memcpy(&(direct_buff.getBuffer()[0]),&(this->getBuffer()[0]), sizeof(SarImageDataBuffer));
            for(k = 0; k < (most_actual_line+1); k++) {
                memcpy(&(direct_buff.getBuffer()[SIZE_-(most_actual_line+1)+k]), &(this->getBuffer()[k]), sizeof(Line));
            }
            direct_buff.mostActualLine_ = SIZE_ - 1;
        }
        else if(direct_buff.linesBuffer_ && (most_actual_line == (SIZE_-1 ))) {
            for(k = 0; k < SIZE_; k++) {
                memcpy(&(direct_buff.getBuffer()[k]), &(this->getBuffer()[k]), sizeof(Line));
            }
//            memcpy(&(direct_buff.linesBuffer_[0]),&(this->linesBuffer_[0]), sizeof(SarImageDataBuffer));
        }
        */
    }

    void SarImageDataBuffer::copyToDirectBufferWithLap(SarImageDataBuffer &direct_buff, int curr_line, int curr_start) {
//        qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";

//        if(!direct_buff.linesBuffer_) { // Check if direct buffer exists
//            qDebug("Error: bad allocation of direct buffer");
//            exit(-1);
//        }

        // If copied area is dividet to two parts
//                    |  most actual line
//                   \|/
//        *****************************************           *****************************
//        *           *            *              *           *              **           *
//        *   young   *            *     old      *   =>      *     young    **    new    *
//        *           *            *              *           *              **           *
//        *****************************************           *****************************
        if(curr_line > curr_start && curr_start >= 0) { // normal copy two last screens
            memcpy(&(direct_buff.linesBuffer_[0]),
                   &(linesBuffer_[curr_start]),
                   ((curr_line - curr_start + 1)*SIZE_OF_LINE));
//            qDebug() << "\n STREAM"<<nrStream_<<"SOURCE"<<curr_start << "RAZEM" << (curr_line - curr_start + 1);
        }
        else if(curr_start >= 0) {
//            if(curr_line < curr_start && curr_start >= 0) { // buffer is written from the beginning
            // If copied area is dividet to two parts
//                    |  most actual line
//                   \|/
//        *****************************************           *****************************
//        *           *            *              *           *              **           *
//        *   young   *            *     old      *   =>      *     young    **    new    *
//        *           *            *              *           *              **           *
//        *****************************************           *****************************
            memcpy(&(direct_buff.linesBuffer_[0]),
                   &(linesBuffer_[curr_start]),
                   ((SIZE_ - curr_start)*SIZE_OF_LINE)); // older part
//            qDebug() << "\n STREAM" << nrStream_ << "SOURCE" << curr_start << "NUMBER" << (SIZE_ - curr_start);
            memcpy(&(direct_buff.linesBuffer_[SIZE_ - curr_start]),
                   &(linesBuffer_[0]),
                   ((curr_line + 1)*SIZE_OF_LINE)); // younger part
//            qDebug() << "DESTINATION" << SIZE_ - curr_start << "NUMBER" << (curr_line + 1) << "RAZEM" << (SIZE_ - curr_start)+(curr_line + 1);
        }

//////////////////
        // If copied area is dividet to two parts
//                    |  most actual line
//                   \|/
//        *****************************************           *****************************
//        *           *            *              *           *              **           *
//        *   young   *            *     old      *   =>      *     young    **    new    *
//        *           *            *              *           *              **           *
//        *****************************************           *****************************
//        if(curr_line - direct_buff.SIZE_ < 0 && this->startPresent_ >= 0) {
//            // Young part
//            qDebug() << "\ncurr_line" << curr_line
//                     << "\ndirect_buff.SIZE_" << direct_buff.SIZE_
//                     << "\nthis->SIZE_" << this->SIZE_
//                     << "\ndirect_buff.SIZE_ - 1 - curr_line" << direct_buff.SIZE_ - 1 - curr_line;
//            memcpy(&(direct_buff.getBuffer()[direct_buff.SIZE_ - 1 - curr_line]),
//                   &(this->getBuffer()[0]),
//                   ((curr_line+1)*SIZE_OF_LINE));
//            // Old part
//            memcpy(&(direct_buff.getBuffer()[0]),
//                   &(this->getBuffer()[this->SIZE_ - 1 - (direct_buff.SIZE_ - 1 - curr_line - 1)]),
//                   ((direct_buff.SIZE_ - 1 - curr_line)*SIZE_OF_LINE));
//            qDebug() << "\nthis->SIZE_ - 1 - (direct_buff.SIZE_ - 1 - curr_line - 1)" << this->SIZE_ - 1 - (direct_buff.SIZE_ - 1 - curr_line - 1)
//                     << "\ndirect_buff.SIZE_ - 1 - curr_line" << direct_buff.SIZE_ - 1 - curr_line;
//            qDebug() << "KOPIOWANIE CZESCIOWE";
//        }
//        else if(this->startPresent_ >= 0){
//                                           |  most actual line
//                                          \|/
//        *****************************************           *****************************
//        *     *                            *    *           *                           *
//        *     *         whole              *    *   =>      *         whole             *
//        *     *                            *    *           *                           *
//        *****************************************           *****************************

//            qDebug() << "KOPIOWANIE CALE";
//            memcpy(&(direct_buff.getBuffer()[0]),
//                   &(this->getBuffer()[curr_line - (direct_buff.SIZE_ - 1)]),
//                   (direct_buff.SIZE_*SIZE_OF_LINE));
//        }


//        // normal copy two last screens
//        if(most_actual_line > start_present && start_present >= 0) {
//            memcpy(&(direct_buff.getBuffer()[0]),
//                   &(this->getBuffer()[start_present]),
//                   ((most_actual_line - start_present + 1)*SIZE_OF_LINE));
//            qDebug() << "\n STREAM" <<nrStream_;
//            qDebug() << "SOURCE" << start_present
//                     << "RAZEM" << (most_actual_line - start_present + 1);
//        }

//        // buffer is written from the beginning
//        if(most_actual_line < start_present && start_present >= 0) {
//            memcpy(&(direct_buff.getBuffer()[0]),
//                   &(this->getBuffer()[start_present]),
//                   ((SIZE_ - start_present)*SIZE_OF_LINE)); // older part
//            qDebug() << "\n STREAM" << nrStream_;
//            qDebug() << "SOURCE" << start_present
//                     << "NUMBER" << (SIZE_ - start_present);
//            memcpy(&(direct_buff.getBuffer()[SIZE_ - start_present]),
//                   &(this->getBuffer()[0]),
//                   ((most_actual_line + 1)*SIZE_OF_LINE)); // younger part
//            qDebug() << "DESTINATION" << SIZE_ - start_present
//                     << "NUMBER" << (most_actual_line + 1)
//                     << "RAZEM" << (SIZE_ - start_present)+(most_actual_line + 1);
//        }
    }
} // namespace SarViewer
