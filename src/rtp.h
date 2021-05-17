/*
 * rtp.h  --  RTP header file (RFC 1890)
 */

#include <sys/types.h>
#include <boost/predef/other/endian.h>

#include "sysdep.h"

#define RTP_SVM_RAW 40
/*
 * The type definitions below are valid for 32-bit architectures and
 * may have to be adjusted for 16- or 64-bit architectures.
 */
typedef unsigned char  u_int8;
typedef unsigned short u_int16;
typedef unsigned int   u_int32;
typedef          short int16;

/*
 * Current protocol version.
 */
#define RTP_VERSION    2

#define RTP_SEQ_MOD (1<<16)
#define RTP_MAX_SDES 255      /* maximum text length for SDES */


typedef enum {
  RTCP_SR   = 200,
  RTCP_RR   = 201,
  RTCP_SDES = 202,
  RTCP_BYE  = 203,
  RTCP_APP  = 204
} rtcp_type_t;

typedef enum {
  RTCP_SDES_END   = 0,
  RTCP_SDES_CNAME = 1,
  RTCP_SDES_NAME  = 2,
  RTCP_SDES_EMAIL = 3,
  RTCP_SDES_PHONE = 4,
  RTCP_SDES_LOC   = 5,
  RTCP_SDES_TOOL  = 6,
  RTCP_SDES_NOTE  = 7,
  RTCP_SDES_PRIV  = 8
} rtcp_sdes_type_t;


/*
 * Largest (user-level) packet size generated by our rtp applications.
 * Individual video formats may use smaller mtu's.
 */
#define PACKET_SIZE 1024
#define RTP_MTU 1024
#define MAXHDR 24
#define RTP_HDR_SZ 12
#define TOTHDR 20

/*
 *
 *
 *    The RTP header has the following format:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|X|  CC   |M|     PT      |       sequence number         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                           timestamp                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           synchronization source (SSRC) identifier            |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |            contributing source (CSRC) identifiers             |
 * |                             ....                              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * RTP data header
 */


typedef struct {
#if WORDS_BIGENDIAN
    unsigned int version:2;   /* protocol version */
    unsigned int p:1;         /* padding flag */
    unsigned int x:1;         /* header extension flag */
    unsigned int cc:4;        /* CSRC count */
    unsigned int m:1;         /* marker bit */
    unsigned int pt:7;        /* payload type */
#else
    unsigned int cc:4;        /* CSRC count */
    unsigned int x:1;         /* header extension flag */
    unsigned int p:1;         /* padding flag */
    unsigned int version:2;   /* protocol version */
    unsigned int pt:7;        /* payload type */
    unsigned int m:1;         /* marker bit */
#endif

    unsigned int seq:16;      /* sequence number */
    u_int32 ts;               /* timestamp */
    u_int32 ssrc;             /* synchronization source */
} rtp_hdr_t;

/*
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |V=2|P|    RC   |   PT=SR=200   |             length            | header
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 * RTCP common header word
 */


typedef struct {
#if WORDS_BIGENDIAN
  unsigned int version:2;   /* protocol version */
  unsigned int p:1;         /* padding flag */
  unsigned int count:5;     /* varies by packet type */
  unsigned int pt:8;        /* RTCP packet type */
#else
  unsigned int count:5;  
  unsigned int p:1;   
  unsigned int version:2; 
  unsigned int pt:8;   
#endif
  u_int16 length;           /* pkt len in words, w/o this word */
} rtcp_common_t;



/*
* Reception report block
*/


typedef struct {
  u_int32 ssrc;             /* data source being reported */
  unsigned int fraction:8;  /* fraction lost since last SR/RR */
  int lost:24;              /* cumul. no. pkts lost (signed!) */
  u_int32 last_seq;         /* extended last seq. no. received */
  u_int32 jitter;           /* interarrival jitter */
  u_int32 lsr;              /* last SR packet from this source */
  u_int32 dlsr;             /* delay since last SR packet */
} rtcp_rr_t;


/*
 * SDES item
 */
typedef struct {
  u_int8 type;              /* type of item (rtcp_sdes_type_t) */
  u_int8 length;            /* length of item (in octets) */
  char data[1];             /* text, not null-terminated */
} rtcp_sdes_item_t;


/*
 * One RTCP packet
 */
typedef struct {
  rtcp_common_t common;     /* common header */
  union {
    /* sender report (SR) */
    struct {
      u_int32 ssrc;     /* sender generating this report */
      u_int32 ntp_sec;  /* NTP timestamp */
      u_int32 ntp_frac;
      u_int32 rtp_ts;   /* RTP timestamp */
      u_int32 psent;    /* packets sent */
      u_int32 osent;    /* octets sent */
      rtcp_rr_t rr[1];  /* variable-length list */
    } sr;

    /* reception report (RR) */
    struct {
      u_int32 ssrc;     /* receiver generating this report */
      rtcp_rr_t rr[1];  /* variable-length list */
    } rr;

    /* source description (SDES) */
    struct rtcp_sdes {
      u_int32 src;      /* first SSRC/CSRC */
      rtcp_sdes_item_t item[1]; /* list of SDES items */
    } sdes;

    /* BYE */
    struct {
      u_int32 src[1];   /* list of sources */
      /* can't express trailing text for reason */
    } bye;
  } r;
} rtcp_t;


typedef struct rtcp_sdes rtcp_sdes_t;


/*
 * Per-source state information
 */
typedef struct {
  u_int16 max_seq;        /* highest seq. number seen */
  u_int32 cycles;         /* shifted count of seq. number cycles */
  u_int32 base_seq;       /* base seq number */
  u_int32 bad_seq;        /* last 'bad' seq number + 1 */
  u_int32 probation;      /* sequ. packets till source is valid */
  u_int32 received;       /* packets received */
  u_int32 expected_prior; /* packet expected at last interval */
  u_int32 received_prior; /* packet received at last interval */
  u_int32 transit;        /* relative trans time for prev pkt */
  u_int32 jitter;         /* estimated jitter */
  /* ... */
} source;

/*
 * 
 *
 * Motion JPEG encapsulation.
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                frag offset                    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      x        |       y       |     Width     |     Height    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * Type = Index into a table of predefined JPEG parameters
 * Width = Width of input in 8-pixel blocks
 * Height = Height of input in 8-pixel blocks
 * Q = Quality factor (0-100 = std, >100 = custom)
 * Frag offset = The byte offset into the frame for the data in 
 * this packet
 */
struct jpeghdr {
  //        unsigned int type:8;  
  //    int off:24;              
  u_int32 off;
        u_int8 x;
  u_int8 y;
  u_int8 width;
  u_int8 height;
  /* cells */
};