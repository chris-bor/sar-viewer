#ifndef SAR_IMAGE_H
# define SAR_IMAGE_H

#include "qglobal.h"
#include "uni_types.h"
#include "propatria_types.h"

#define VIEWER_PIPE_NAME_0  "\\\\.\\pipe\\SAR_PIPE_0"
#define VIEWER_PIPE_NAME_1  "\\\\.\\pipe\\SAR_PIPE_1"

#define VIEWER_PORT_NR_0    5000
#define VIEWER_PORT_NR_1    5001

#define VIEWER_IP_ADD       "127.0.0.1"

#define PIPE_OUT_BUF_SIZE 10000000
#define PIPE_IN_BUF_SIZE  10000

#define IMAGE_LINE_MAGIC 0x4E494C53

/* bit manipulation macros */
#define BitSet(arg, val) ((arg) |= (val))           /* set bit using given value */
#define BitClr(arg, val) ((arg) &= ~(val))          /* clear bit using given value */
#define BitFlp(arg, val) ((arg) ^= (val))           /* flip bit using given value */
#define BitMsk(bit) (1 << (bit))                    /* create value with desired bit shift */
#define BitNset(arg, bit) ((arg) |= BitMsk(bit))    /* set bit at position bit */
#define BitNclr(arg, bit) ((arg) &= ~BitMsk(bit))   /* clear bit at position bit */
#define BitNflp(arg, bit) ((arg) ^= BitMsk(bit))    /* flip bit at position bit */
#define BitNtst(arg, bit) BOOL((arg) & BitMsk(bit)) /* test bit at position bit */
#define BitSetVal(arg, mask ,val) (BitSet(BitClr(arg, mask), val)) /* set values of bits specified by the mask */

/* definitions of bits in flags field */
#define FLAG_BIT_AS_MASK         0xFF
#define FLAG_BIT_ANT_DIR_RIGHT   8
#define FLAG_BIT_GPS_AVAILABLE   9
#define FLAG_BIT_GPS_VALID       10

/** structure defining  */
//#ifdef _MSC_VER
//#pragma pack(push, r1, 1)
//#endif
#pragma pack(1)
struct sar_image_line_header_{
    UINT32 magic_id;       /**< identifier (0x4E494C53 = SLIN in ASCII) */
    DOUBLE lat_plat;       /**< latitude of the platform [deg] */
    DOUBLE lon_plat;       /**< longitude of the platform [deg] */
    DOUBLE lat_ref;        /**< latitude of the reference trajectory [deg] */
    DOUBLE lon_ref;        /**< longitude of the reference trajectory [deg] */
    FLOAT alt_sea;         /**< altitude above the sea level [m] */
    FLOAT alt_ground;      /**< altitude above the ground level [m] */
    FLOAT cell_size;       /**< size of the range cell [m] */
    FLOAT r_min;           /**< range to the start of the swath [m] */
    FLOAT head;            /**< heading of the platform [deg] */
    FLOAT head_ref;        /**< heading of the reference trajectory [deg] */
    FLOAT pitch;           /**< pitch of the platform [deg] */
    FLOAT roll;            /**< roll of the platform [deg] */
    FLOAT veloc;           /**< platform velocity [m/s] */
    FLOAT prf;             /**< pulse repetition frequency [Hz] */
    UINT32 flags;          /**< flags:
                              bits: 0-7 - analog settings (AS) byte form ADC unit
                              bit:  8   - 0 antenna poining to the left, 1 - antenna pointing to the right
                              bit:  9   - GPS data available
                              bit:  10  - GPS data valid */
    UINT32 nr_range_cells; /**< number of range cells in the image line */
    UINT32 line_cnt;       /**< counter of  lines since the beginning */
    UINT32 block_cnt;      /**< counter of blocks */
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
//__attribute__ ((packed));

typedef struct sar_image_line_header_ sar_image_line_header;

const uint SIZE_OF_IMG_HEADER = sizeof(sar_image_line_header);

/*

Each line is followed by "sar_image_line_header" structure. Then "nr_range_cells" image values are sent. Each image value is 8-bit. 

sar_image_line_header header;      \
BYTE image_val[0];                 |
BYTE image_val[1];                 |
BYTE image_val[2];                  \  first line
.                                   /
.                                  |
.                                  |
BYTE image_val[nr_range_cells-1];  /
sar_image_line_header header;      \
BYTE image_val[0];                 |
BYTE image_val[1];                 |
BYTE image_val[2];                  \  second line
.                                   /
.                                  |
.                                  |
BYTE image_val[nr_range_cells-1];  /
sar_image_line_header header;      \
BYTE image_val[0];                 |
BYTE image_val[1];                 |
BYTE image_val[2];                  \  third line
.                                   /
.                                  |
.                                  |
BYTE image_val[nr_range_cells-1];  /
and so on...

*/

#ifndef MAX
# define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#ifndef MIN
# define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

void convert_float_to_byte(float *float_tab, BYTE *byte_tab, int len, float scaling_coeff);
int save_sar_image(FILE *file, BYTE *image, radar_pars *r_pars, int nr_range_cells, int block_size, int block_overlap);
int sar_image_saver(char *file_name_root, BYTE *image, radar_pars *r_pars, int nr_range_cells, int block_size, int block_overlap, int chan_nr);
int interpolate_geographic_pos(ins_gps_data *geo_data, int block_size);

int write_sar_image_pipe(BYTE *image, ins_gps_data *geo_data, radar_pars *r_pars, int nr_range_cells, int block_size, int block_overlap, int chan_nr, UINT32 flags);
int send_sar_image_udp(BYTE *image, ins_gps_data *geo_data, radar_pars *r_pars, int nr_range_cells, int block_size, int block_overlap, int chan_nr, UINT32 flags);

#endif

