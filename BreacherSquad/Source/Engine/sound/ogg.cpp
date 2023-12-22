#include "dxstdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>

#pragma warning(disable:4995)

// This file comprises code from the xiph.org foundation with
// some minor changes (to turn it all into a single file, and to
// make it compile with C++). The original copyright text is
// as follows:
//
//Copyright (c) 2002, Xiph.org Foundation
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//- Redistributions of source code must retain the above copyright
//notice, this list of conditions and the following disclaimer.
//
//- Redistributions in binary form must reproduce the above copyright
//notice, this list of conditions and the following disclaimer in the
//documentation and/or other materials provided with the distribution.
//
//- Neither the name of the Xiph.org Foundation nor the names of its
//contributors may be used to endorse or promote products derived from
//this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
//OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//


/* Some of thee routines (autocorrelator, LPC coefficient estimator)
   are derived from code written by Jutta Degener and Carsten Bormann;
   thus we include their copyright below.  The entirety of this file
   is freely redistributable on the condition that both of these
   copyright notices are preserved without modification.  */

/* Preserved Copyright: *********************************************/

/* Copyright 1992, 1993, 1994 by Jutta Degener and Carsten Bormann,
Technische Universita"t Berlin

Any use of this software is permitted provided that this notice is not
removed and that neither the authors nor the Technische Universita"t
Berlin are deemed to have made any representations as to the
suitability of this software for any purpose nor are held responsible
for any defects of this software. THERE IS ABSOLUTELY NO WARRANTY FOR
THIS SOFTWARE.

As a matter of courtesy, the authors request to be informed about uses
this software has found, about bugs in this software, and about any
improvements that may be of general interest.

Berlin, 28.11.1994
Jutta Degener
Carsten Bormann

*********************************************************************/







// ==========================================================================
// ==========================================================================
// os_types.h
// ==========================================================================
// ==========================================================================
// function: #ifdef jail to whip a few platforms into the UNIX ideal.

/* make it easy on the folks that want to compile the libs with a
   different malloc than stdlib */
#define _ogg_malloc  malloc
#define _ogg_calloc  calloc
#define _ogg_realloc realloc
#define _ogg_free    free

#ifdef _WIN32

#  ifndef __GNUC__
   /* MSVC/Borland */
   typedef __int64 ogg_int64_t;
   typedef __int32 ogg_int32_t;
   typedef unsigned __int32 ogg_uint32_t;
   typedef __int16 ogg_int16_t;
   typedef unsigned __int16 ogg_uint16_t;
#  else
   /* Cygwin */
   #include <_G_config.h>
   typedef _G_int64_t ogg_int64_t;
   typedef _G_int32_t ogg_int32_t;
   typedef _G_uint32_t ogg_uint32_t;
   typedef _G_int16_t ogg_int16_t;
   typedef _G_uint16_t ogg_uint16_t;
#  endif

#elif defined(__MACOS__)

#  include <sys/types.h>
   typedef SInt16 ogg_int16_t;
   typedef UInt16 ogg_uint16_t;
   typedef SInt32 ogg_int32_t;
   typedef UInt32 ogg_uint32_t;
   typedef SInt64 ogg_int64_t;

#elif defined(__MACOSX__) /* MacOS X Framework build */

#  include <sys/types.h>
   typedef int16_t ogg_int16_t;
   typedef u_int16_t ogg_uint16_t;
   typedef int32_t ogg_int32_t;
   typedef u_int32_t ogg_uint32_t;
   typedef int64_t ogg_int64_t;

#elif defined(__BEOS__)

   /* Be */
#  include <inttypes.h>
   typedef int16_t ogg_int16_t;
   typedef u_int16_t ogg_uint16_t;
   typedef int32_t ogg_int32_t;
   typedef u_int32_t ogg_uint32_t;
   typedef int64_t ogg_int64_t;

#elif defined (__EMX__)

   /* OS/2 GCC */
   typedef short ogg_int16_t;
   typedef unsigned short ogg_uint16_t;
   typedef int ogg_int32_t;
   typedef unsigned int ogg_uint32_t;
   typedef long long ogg_int64_t;

#else

#  include <sys/types.h>
#  include <ogg/config_types.h>

#endif



// ==========================================================================
// ==========================================================================
// os.h
// ==========================================================================
// ==========================================================================
// function: #ifdef jail to whip a few platforms into the UNIX ideal.


#ifndef _V_IFDEFJAIL_H_
#  define _V_IFDEFJAIL_H_

// nb. borland refuses to inline functions that contain loops
#  ifdef __GNUC__
#    define STIN static __inline__
#    define STLOOPIN static __inline__
#  elif _WIN32
#    define STIN inline
#    ifdef __BORLANDC__
#      define STLOOPIN
#    else
#      define STLOOPIN inline
#    endif
#  else
#    define STIN static
#    define STIN static
#  endif

#  ifndef M_PI
#    define M_PI (3.1415926536f)
#  endif

#  ifdef _WIN32
#    include <malloc.h>
#    define rint(x)   (floor((x)+0.5f))
#    define NO_FLOAT_MATH_LIB
#    define FAST_HYPOT(a, b) sqrt((a)*(a) + (b)*(b))
#  endif

#  ifndef FAST_HYPOT
#    define FAST_HYPOT hypot
#  endif

#endif

#ifdef HAVE_ALLOCA_H
#  include <alloca.h>
#endif

#ifdef USE_MEMORY_H
#  include <memory.h>
#endif

#ifndef min
#  define min(x,y)  ((x)>(y)?(y):(x))
#endif

#ifndef max
#  define max(x,y)  ((x)<(y)?(y):(x))
#endif

#if defined(__i386__) && defined(__GNUC__) && !defined(__BEOS__)
#  define VORBIS_FPU_CONTROL
/* both GCC and MSVC are kinda stupid about rounding/casting to int.
   Because of encapsulation constraints (GCC can't see inside the asm
   block and so we end up doing stupid things like a store/load that
   is collectively a noop), we do it this way */

/* we must set up the fpu before this works!! */

typedef ogg_int16_t vorbis_fpu_control;

inline void vorbis_fpu_setround(vorbis_fpu_control *fpu){
  ogg_int16_t ret;
  ogg_int16_t temp;
  __asm__ __volatile__("fnstcw %0\n\t"
      "movw %0,%%dx\n\t"
      "orw $62463,%%dx\n\t"
      "movw %%dx,%1\n\t"
      "fldcw %1\n\t":"=m"(ret):"m"(temp): "dx");
  *fpu=ret;
}

inline void vorbis_fpu_restore(vorbis_fpu_control fpu){
  __asm__ __volatile__("fldcw %0":: "m"(fpu));
}

/* assumes the FPU is in round mode! */
inline int vorbis_ftoi(double f){  /* yes, double!  Otherwise,
                                             we get extra fst/fld to
                                             truncate precision */
  int i;
  __asm__("fistl %0": "=m"(i) : "t"(f));
  return(i);
}
#endif


#if defined(_WIN32) && !defined(__GNUC__) && !defined(__BORLANDC__)
#  define VORBIS_FPU_CONTROL

typedef ogg_int16_t vorbis_fpu_control;

__inline int vorbis_ftoi(double f){
    int i;
    __asm{
        fld f
        fistp i
    }
    return i;
}

__inline void vorbis_fpu_setround(vorbis_fpu_control * /*fpu unused*/){
}

__inline void vorbis_fpu_restore(vorbis_fpu_control /*fpu unused*/){
}

#endif


#ifndef VORBIS_FPU_CONTROL

typedef int vorbis_fpu_control;

int vorbis_ftoi(double f){
  return (int)(f+.5);
}

/* We don't have special code for this compiler/arch, so do it the slow way */
#  define vorbis_fpu_setround(vorbis_fpu_control) {}
#  define vorbis_fpu_restore(vorbis_fpu_control) {}

#endif




// ==========================================================================
// ==========================================================================
// highlevel.h
// ==========================================================================
// ==========================================================================
// function: highlevel encoder setup struct seperated out for vorbisenc clarity

typedef struct highlevel_byblocktype {
  double tone_mask_setting;
  double tone_peaklimit_setting;
  double noise_bias_setting;
  double noise_compand_setting;
} highlevel_byblocktype;

typedef struct highlevel_encode_setup {
  void *setup;
  int   set_in_stone;

  double base_setting;
  double long_setting;
  double short_setting;
  double impulse_noisetune;

  int    managed;
  long   bitrate_min;
  long   bitrate_av_lo;
  long   bitrate_av_hi;
  long   bitrate_max;
  double bitrate_limit_window;
  double bitrate_av_window;
  double bitrate_av_window_center;

  int impulse_block_p;
  int noise_normalize_p;

  double stereo_point_setting;
  double lowpass_kHz;

  double ath_floating_dB;
  double ath_absolute_dB;

  double amplitude_track_dBpersec;
  double trigger_setting;

  highlevel_byblocktype block[4]; /* padding, impulse, transition, long */

} highlevel_encode_setup;



// ==========================================================================
// ==========================================================================
// ogg.h
// ==========================================================================
// ==========================================================================
// function: toplevel libogg


typedef struct {
  long endbyte;
  int  endbit;

  unsigned char *buffer;
  unsigned char *ptr;
  long storage;
} oggpack_buffer;

/* ogg_page is used to encapsulate the data in one Ogg bitstream page *****/

typedef struct {
  unsigned char *header;
  long header_len;
  unsigned char *body;
  long body_len;
} ogg_page;

/* ogg_stream_state contains the current encode/decode state of a logical
   Ogg bitstream **********************************************************/

typedef struct {
  unsigned char   *body_data;    /* bytes from packet bodies */
  long    body_storage;          /* storage elements allocated */
  long    body_fill;             /* elements stored; fill mark */
  long    body_returned;         /* elements of fill returned */


  int     *lacing_vals;      /* The values that will go to the segment table */
  ogg_int64_t *granule_vals; /* granulepos values for headers. Not compact
                this way, but it is simple coupled to the
                lacing fifo */
  long    lacing_storage;
  long    lacing_fill;
  long    lacing_packet;
  long    lacing_returned;

  unsigned char    header[282];      /* working space for header encode */
  int              header_fill;

  int     e_o_s;          /* set when we have buffered the last packet in the
                             logical bitstream */
  int     b_o_s;          /* set after we've written the initial page
                             of a logical bitstream */
  long    serialno;
  long    pageno;
  ogg_int64_t  packetno;      /* sequence number for decode; the framing
                             knows where there's a hole in the data,
                             but we need coupling so that the codec
                             (which is in a seperate abstraction
                             layer) also knows about the gap */
  ogg_int64_t   granulepos;

} ogg_stream_state;

/* ogg_packet is used to encapsulate the data and metadata belonging
   to a single raw Ogg/Vorbis packet *************************************/

typedef struct {
  unsigned char *packet;
  long  bytes;
  long  b_o_s;
  long  e_o_s;

  ogg_int64_t  granulepos;

  ogg_int64_t  packetno;     /* sequence number for decode; the framing
                knows where there's a hole in the data,
                but we need coupling so that the codec
                (which is in a seperate abstraction
                layer) also knows about the gap */
} ogg_packet;

typedef struct {
  unsigned char *data;
  int storage;
  int fill;
  int returned;

  int unsynced;
  int headerbytes;
  int bodybytes;
} ogg_sync_state;

/* Ogg BITSTREAM PRIMITIVES: bitstream ************************/

extern void  oggpack_writeinit(oggpack_buffer *b);
extern void  oggpack_writetrunc(oggpack_buffer *b,long bits);
extern void  oggpack_writealign(oggpack_buffer *b);
extern void  oggpack_writecopy(oggpack_buffer *b,void *source,long bits);
extern void  oggpack_reset(oggpack_buffer *b);
extern void  oggpack_writeclear(oggpack_buffer *b);
extern void  oggpack_readinit(oggpack_buffer *b,unsigned char *buf,int bytes);
extern void  oggpack_write(oggpack_buffer *b,unsigned long value,int bits);
extern long  oggpack_look(oggpack_buffer *b,int bits);
extern long  oggpack_look1(oggpack_buffer *b);
extern void  oggpack_adv(oggpack_buffer *b,int bits);
extern void  oggpack_adv1(oggpack_buffer *b);
extern long  oggpack_read(oggpack_buffer *b,int bits);
extern long  oggpack_read1(oggpack_buffer *b);
extern long  oggpack_bytes(oggpack_buffer *b);
extern long  oggpack_bits(oggpack_buffer *b);
extern unsigned char *oggpack_get_buffer(oggpack_buffer *b);

extern void  oggpackB_writeinit(oggpack_buffer *b);
extern void  oggpackB_writetrunc(oggpack_buffer *b,long bits);
extern void  oggpackB_writealign(oggpack_buffer *b);
extern void  oggpackB_writecopy(oggpack_buffer *b,void *source,long bits);
extern void  oggpackB_reset(oggpack_buffer *b);
extern void  oggpackB_writeclear(oggpack_buffer *b);
extern void  oggpackB_readinit(oggpack_buffer *b,unsigned char *buf,int bytes);
extern void  oggpackB_write(oggpack_buffer *b,unsigned long value,int bits);
extern long  oggpackB_look(oggpack_buffer *b,int bits);
extern long  oggpackB_look1(oggpack_buffer *b);
extern void  oggpackB_adv(oggpack_buffer *b,int bits);
extern void  oggpackB_adv1(oggpack_buffer *b);
extern long  oggpackB_read(oggpack_buffer *b,int bits);
extern long  oggpackB_read1(oggpack_buffer *b);
extern long  oggpackB_bytes(oggpack_buffer *b);
extern long  oggpackB_bits(oggpack_buffer *b);
extern unsigned char *oggpackB_get_buffer(oggpack_buffer *b);

/* Ogg BITSTREAM PRIMITIVES: encoding **************************/

extern int      ogg_stream_packetin(ogg_stream_state *os, ogg_packet *op);
extern int      ogg_stream_pageout(ogg_stream_state *os, ogg_page *og);
extern int      ogg_stream_flush(ogg_stream_state *os, ogg_page *og);

/* Ogg BITSTREAM PRIMITIVES: decoding **************************/

extern int      ogg_sync_init(ogg_sync_state *oy);
extern int      ogg_sync_clear(ogg_sync_state *oy);
extern int      ogg_sync_reset(ogg_sync_state *oy);
extern int  ogg_sync_destroy(ogg_sync_state *oy);

extern char    *ogg_sync_buffer(ogg_sync_state *oy, long size);
extern int      ogg_sync_wrote(ogg_sync_state *oy, long bytes);
extern long     ogg_sync_pageseek(ogg_sync_state *oy,ogg_page *og);
extern int      ogg_sync_pageout(ogg_sync_state *oy, ogg_page *og);
extern int      ogg_stream_pagein(ogg_stream_state *os, ogg_page *og);
extern int      ogg_stream_packetout(ogg_stream_state *os,ogg_packet *op);
extern int      ogg_stream_packetpeek(ogg_stream_state *os,ogg_packet *op);

/* Ogg BITSTREAM PRIMITIVES: general ***************************/

extern int      ogg_stream_init(ogg_stream_state *os,int serialno);
extern int      ogg_stream_clear(ogg_stream_state *os);
extern int      ogg_stream_reset(ogg_stream_state *os);
extern int      ogg_stream_reset_serialno(ogg_stream_state *os,int serialno);
extern int      ogg_stream_destroy(ogg_stream_state *os);
extern int      ogg_stream_eos(ogg_stream_state *os);

extern void     ogg_page_checksum_set(ogg_page *og);

extern int      ogg_page_version(ogg_page *og);
extern int      ogg_page_continued(ogg_page *og);
extern int      ogg_page_bos(ogg_page *og);
extern int      ogg_page_eos(ogg_page *og);
extern ogg_int64_t  ogg_page_granulepos(ogg_page *og);
extern int      ogg_page_serialno(ogg_page *og);
extern long     ogg_page_pageno(ogg_page *og);
extern int      ogg_page_packets(ogg_page *og);

extern void     ogg_packet_clear(ogg_packet *op);





// ==========================================================================
// ==========================================================================
// codebook.h
// ==========================================================================
// ==========================================================================
// function: basic shared codebook operations

/* This structure encapsulates huffman and VQ style encoding books; it
   doesn't do anything specific to either.

   valuelist/quantlist are nonNULL (and q_* significant) only if
   there's entry->value mapping to be done.

   If encode-side mapping must be done (and thus the entry needs to be
   hunted), the auxiliary encode pointer will point to a decision
   tree.  This is true of both VQ and huffman, but is mostly useful
   with VQ.

*/

typedef struct static_codebook{
  long   dim;            /* codebook dimensions (elements per vector) */
  long   entries;        /* codebook entries */
  long  *lengthlist;     /* codeword lengths in bits */

  /* mapping ***************************************************************/
  int    maptype;        /* 0=none
                1=implicitly populated values from map column
                2=listed arbitrary values */

  /* The below does a linear, single monotonic sequence mapping. */
  long     q_min;       /* packed 32 bit float; quant value 0 maps to minval */
  long     q_delta;     /* packed 32 bit float; val 1 - val 0 == delta */
  int      q_quant;     /* bits: 0 < quant <= 16 */
  int      q_sequencep; /* bitflag */

  long     *quantlist;  /* map == 1: (int)(entries^(1/dim)) element column map
               map == 2: list of dim*entries quantized entry vals
            */

  /* encode helpers ********************************************************/
  struct encode_aux_nearestmatch *nearest_tree;
  struct encode_aux_threshmatch  *thresh_tree;
  struct encode_aux_pigeonhole  *pigeon_tree;

  int allocedp;
} static_codebook;

/* this structures an arbitrary trained book to quickly find the
   nearest cell match */
typedef struct encode_aux_nearestmatch{
  /* pre-calculated partitioning tree */
  long   *ptr0;
  long   *ptr1;

  long   *p;         /* decision points (each is an entry) */
  long   *q;         /* decision points (each is an entry) */
  long   aux;        /* number of tree entries */
  long   alloc;
} encode_aux_nearestmatch;

/* assumes a maptype of 1; encode side only, so that's OK */
typedef struct encode_aux_threshmatch{
  float *quantthresh;
  long   *quantmap;
  int     quantvals;
  int     threshvals;
} encode_aux_threshmatch;

typedef struct encode_aux_pigeonhole{
  float min;
  float del;

  int  mapentries;
  int  quantvals;
  long *pigeonmap;

  long fittotal;
  long *fitlist;
  long *fitmap;
  long *fitlength;
} encode_aux_pigeonhole;

typedef struct codebook{
  long dim;           /* codebook dimensions (elements per vector) */
  long entries;       /* codebook entries */
  long used_entries;  /* populated codebook entries */
  const static_codebook *c;

  /* for encode, the below are entry-ordered, fully populated */
  /* for decode, the below are ordered by bitreversed codeword and only
     used entries are populated */
  float        *valuelist;  /* list of dim*entries actual entry values */
  ogg_uint32_t *codelist;   /* list of bitstream codewords for each entry */

  int          *dec_index;  /* only used if sparseness collapsed */
  char         *dec_codelengths;
  ogg_uint32_t *dec_firsttable;
  int           dec_firsttablen;
  int           dec_maxlength;

} codebook;


extern void vorbis_staticbook_clear(static_codebook *b);
extern void vorbis_staticbook_destroy(static_codebook *b);
extern int vorbis_book_init_encode(codebook *dest,const static_codebook *source);
extern int vorbis_book_init_decode(codebook *dest,const static_codebook *source);
extern void vorbis_book_clear(codebook *b);

extern float *_book_unquantize(const static_codebook *b,int n,int *map);
extern float *_book_logdist(const static_codebook *b,float *vals);
extern float _float32_unpack(long val);
extern long   _float32_pack(float val);
extern int  _best(codebook *book, float *a, int step);
extern int _ilog(unsigned int v);
extern long _book_maptype1_quantvals(const static_codebook *b);

extern int vorbis_book_besterror(codebook *book,float *a,int step,int addmul);
extern long vorbis_book_codeword(codebook *book,int entry);
extern long vorbis_book_codelen(codebook *book,int entry);



extern int vorbis_staticbook_pack(const static_codebook *c,oggpack_buffer *b);
extern int vorbis_staticbook_unpack(oggpack_buffer *b,static_codebook *c);

extern int vorbis_book_encode(codebook *book, int a, oggpack_buffer *b);
extern int vorbis_book_errorv(codebook *book, float *a);
extern int vorbis_book_encodev(codebook *book, int best,float *a,
                   oggpack_buffer *b);

extern long vorbis_book_decode(codebook *book, oggpack_buffer *b);
extern long vorbis_book_decodevs_add(codebook *book, float *a,
                     oggpack_buffer *b,int n);
extern long vorbis_book_decodev_set(codebook *book, float *a,
                    oggpack_buffer *b,int n);
extern long vorbis_book_decodev_add(codebook *book, float *a,
                    oggpack_buffer *b,int n);
extern long vorbis_book_decodevv_add(codebook *book, float **a,
                     long off,int ch,
                    oggpack_buffer *b,int n);






// ==========================================================================
// ==========================================================================
// codec.h
// ==========================================================================
// ==========================================================================
// function: libvorbis codec headers


typedef struct vorbis_info{
  int version;
  int channels;
  long rate;

  /* The below bitrate declarations are *hints*.
     Combinations of the three values carry the following implications:

     all three set to the same value:
       implies a fixed rate bitstream
     only nominal set:
       implies a VBR stream that averages the nominal bitrate.  No hard
       upper/lower limit
     upper and or lower set:
       implies a VBR bitstream that obeys the bitrate limits. nominal
       may also be set to give a nominal rate.
     none set:
       the coder does not care to speculate.
  */

  long bitrate_upper;
  long bitrate_nominal;
  long bitrate_lower;
  long bitrate_window;

  void *codec_setup;
} vorbis_info;

/* vorbis_dsp_state buffers the current vorbis audio
   analysis/synthesis state.  The DSP state belongs to a specific
   logical bitstream ****************************************************/
typedef struct vorbis_dsp_state{
  int analysisp;
  vorbis_info *vi;

  float **pcm;
  float **pcmret;
  int      pcm_storage;
  int      pcm_current;
  int      pcm_returned;

  int  preextrapolate;
  int  eofflag;

  long lW;
  long W;
  long nW;
  long centerW;

  ogg_int64_t granulepos;
  ogg_int64_t sequence;

  ogg_int64_t glue_bits;
  ogg_int64_t time_bits;
  ogg_int64_t floor_bits;
  ogg_int64_t res_bits;

  void       *backend_state;
} vorbis_dsp_state;

typedef struct vorbis_block{
  /* necessary stream state for linking to the framing abstraction */
  float  **pcm;       /* this is a pointer into local storage */
  oggpack_buffer opb;

  long  lW;
  long  W;
  long  nW;
  int   pcmend;
  int   mode;

  int         eofflag;
  ogg_int64_t granulepos;
  ogg_int64_t sequence;
  vorbis_dsp_state *vd; /* For read-only access of configuration */

  /* local storage to avoid remallocing; it's up to the mapping to
     structure it */
  void               *localstore;
  long                localtop;
  long                localalloc;
  long                totaluse;
  struct alloc_chain *reap;

  /* bitmetrics for the frame */
  long glue_bits;
  long time_bits;
  long floor_bits;
  long res_bits;

  void *internal;

} vorbis_block;

/* vorbis_block is a single block of data to be processed as part of
the analysis/synthesis stream; it belongs to a specific logical
bitstream, but is independant from other vorbis_blocks belonging to
that logical bitstream. *************************************************/

struct alloc_chain{
  void *ptr;
  struct alloc_chain *next;
};

/* vorbis_info contains all the setup information specific to the
   specific compression/decompression mode in progress (eg,
   psychoacoustic settings, channel setup, options, codebook
   etc). vorbis_info and substructures are in backends.h.
*********************************************************************/

/* the comments are not part of vorbis_info so that vorbis_info can be
   storage */
typedef struct vorbis_comment{
  /* unlimited user comment fields.  libvorbis writes 'libvorbis'
     whatever vendor is set to in encode */
  char **user_comments;
  int   *comment_lengths;
  int    comments;
  char  *vendor;

} vorbis_comment;


/* libvorbis encodes in two abstraction layers; first we perform DSP
   and produce a packet (see docs/analysis.txt).  The packet is then
   coded into a framed OggSquish bitstream by the second layer (see
   docs/framing.txt).  Decode is the reverse process; we sync/frame
   the bitstream and extract individual packets, then decode the
   packet back into PCM audio.

   The extra framing/packetizing is used in streaming formats, such as
   files.  Over the net (such as with UDP), the framing and
   packetization aren't necessary as they're provided by the transport
   and the streaming layer is not used */

/* Vorbis PRIMITIVES: general ***************************************/

extern void     vorbis_info_init(vorbis_info *vi);
extern void     vorbis_info_clear(vorbis_info *vi);
extern int      vorbis_info_blocksize(vorbis_info *vi,int zo);
extern void     vorbis_comment_init(vorbis_comment *vc);
extern void     vorbis_comment_add(vorbis_comment *vc, char *comment);
extern void     vorbis_comment_add_tag(vorbis_comment *vc,
                       char *tag, char *contents);
extern char    *vorbis_comment_query(vorbis_comment *vc, char *tag, int count);
extern int      vorbis_comment_query_count(vorbis_comment *vc, char *tag);
extern void     vorbis_comment_clear(vorbis_comment *vc);

extern int      vorbis_block_init(vorbis_dsp_state *v, vorbis_block *vb);
extern int      vorbis_block_clear(vorbis_block *vb);
extern void     vorbis_dsp_clear(vorbis_dsp_state *v);
extern double   vorbis_granule_time(vorbis_dsp_state *v,
                    ogg_int64_t granulepos);

/* Vorbis PRIMITIVES: analysis/DSP layer ****************************/

extern int      vorbis_analysis_init(vorbis_dsp_state *v,vorbis_info *vi);
extern int      vorbis_commentheader_out(vorbis_comment *vc, ogg_packet *op);
extern int      vorbis_analysis_headerout(vorbis_dsp_state *v,
                      vorbis_comment *vc,
                      ogg_packet *op,
                      ogg_packet *op_comm,
                      ogg_packet *op_code);
extern float  **vorbis_analysis_buffer(vorbis_dsp_state *v,int vals);
extern int      vorbis_analysis_wrote(vorbis_dsp_state *v,int vals);
extern int      vorbis_analysis_blockout(vorbis_dsp_state *v,vorbis_block *vb);
extern int      vorbis_analysis(vorbis_block *vb,ogg_packet *op);

extern int      vorbis_bitrate_addblock(vorbis_block *vb);
extern int      vorbis_bitrate_flushpacket(vorbis_dsp_state *vd,
                       ogg_packet *op);

/* Vorbis PRIMITIVES: synthesis layer *******************************/
extern int      vorbis_synthesis_headerin(vorbis_info *vi,vorbis_comment *vc,
                      ogg_packet *op);

extern int      vorbis_synthesis_init(vorbis_dsp_state *v,vorbis_info *vi);
extern int      vorbis_synthesis(vorbis_block *vb,ogg_packet *op);
extern int      vorbis_synthesis_trackonly(vorbis_block *vb,ogg_packet *op);
extern int      vorbis_synthesis_blockin(vorbis_dsp_state *v,vorbis_block *vb);
extern int      vorbis_synthesis_pcmout(vorbis_dsp_state *v,float ***pcm);
extern int      vorbis_synthesis_read(vorbis_dsp_state *v,int samples);
extern long     vorbis_packet_blocksize(vorbis_info *vi,ogg_packet *op);

/* Vorbis ERRORS and return codes ***********************************/

#define OV_FALSE      -1
#define OV_EOF        -2
#define OV_HOLE       -3

#define OV_EREAD      -128
#define OV_EFAULT     -129
#define OV_EIMPL      -130
#define OV_EINVAL     -131
#define OV_ENOTVORBIS -132
#define OV_EBADHEADER -133
#define OV_EVERSION   -134
#define OV_ENOTAUDIO  -135
#define OV_EBADPACKET -136
#define OV_EBADLINK   -137
#define OV_ENOSEEK    -138



// ==========================================================================
// ==========================================================================
// mdct.h
// ==========================================================================
// ==========================================================================
// function: modified discrete cosine transform prototypes



/*#define MDCT_INTEGERIZED  <- be warned there could be some hurt left here*/
#ifdef MDCT_INTEGERIZED

#define DATA_TYPE int
#define REG_TYPE  register int
#define TRIGBITS 14
#define cPI3_8 6270
#define cPI2_8 11585
#define cPI1_8 15137

#define FLOAT_CONV(x) ((int)((x)*(1<<TRIGBITS)+.5))
#define MULT_NORM(x) ((x)>>TRIGBITS)
#define HALVE(x) ((x)>>1)

#else

#define DATA_TYPE float
#define REG_TYPE  float
#define cPI3_8 .38268343236508977175F
#define cPI2_8 .70710678118654752441F
#define cPI1_8 .92387953251128675613F

#define FLOAT_CONV(x) (x)
#define MULT_NORM(x) (x)
#define HALVE(x) ((x)*.5f)

#endif


typedef struct {
  int n;
  int log2n;

  DATA_TYPE *trig;
  int       *bitrev;

  DATA_TYPE scale;
} mdct_lookup;


extern void mdct_init(mdct_lookup *lookup,int n);
extern void mdct_clear(mdct_lookup *l);
extern void mdct_forward(mdct_lookup *init, DATA_TYPE *in, DATA_TYPE *out);
extern void mdct_backward(mdct_lookup *init, DATA_TYPE *in, DATA_TYPE *out);




// ==========================================================================
// ==========================================================================
// envelope.h
// ==========================================================================
// ==========================================================================
// function: PCM data envelope analysis and manipulation


#define VE_PRE    16
#define VE_WIN    4
#define VE_POST   2
#define VE_AMP    (VE_PRE+VE_POST-1)

#define VE_BANDS  7
#define VE_NEARDC 15

#define VE_MINSTRETCH 2   /* a bit less than short block */
#define VE_MAXSTRETCH 12  /* one-third full block */

typedef struct {
  float ampbuf[VE_AMP];
  int   ampptr;

  float nearDC[VE_NEARDC];
  float nearDC_acc;
  float nearDC_partialacc;
  int   nearptr;

} envelope_filter_state;

typedef struct {
  int begin;
  int end;
  float *window;
  float total;
} envelope_band;

typedef struct {
  int ch;
  int winlength;
  int searchstep;
  float minenergy;

  mdct_lookup  mdct;
  float       *mdct_win;

  envelope_band          band[VE_BANDS];
  envelope_filter_state *filter;
  int   stretch;

  int                   *mark;

  long storage;
  long current;
  long curmark;
  long cursor;
} envelope_lookup;


extern void _ve_envelope_init(envelope_lookup *e,vorbis_info *vi);
extern void _ve_envelope_clear(envelope_lookup *e);
extern long _ve_envelope_search(vorbis_dsp_state *v);
extern void _ve_envelope_shift(envelope_lookup *e,long shift);
extern int  _ve_envelope_mark(vorbis_dsp_state *v);



// ==========================================================================
// ==========================================================================
// smallft.h
// ==========================================================================
// ==========================================================================
// function: fft transform

typedef struct {
  int n;
  float *trigcache;
  int *splitcache;
} drft_lookup;


extern void drft_forward(drft_lookup *l,float *data);
extern void drft_backward(drft_lookup *l,float *data);
extern void drft_init(drft_lookup *l,int n);
extern void drft_clear(drft_lookup *l);




// ==========================================================================
// ==========================================================================
// bitrate.h
// ==========================================================================
// ==========================================================================
// function: bitrate tracking and management


/* encode side bitrate tracking */
typedef struct bitrate_manager_state {
  ogg_uint32_t  *queue_binned;
  ogg_uint32_t  *queue_actual;
  int            queue_size;

  int            queue_head;
  int            queue_bins;

  long          *avg_binacc;
  int            avg_center;
  int            avg_tail;
  ogg_uint32_t   avg_centeracc;
  ogg_uint32_t   avg_sampleacc;
  ogg_uint32_t   avg_sampledesired;
  ogg_uint32_t   avg_centerdesired;

  long          *minmax_binstack;
  long          *minmax_posstack;
  long          *minmax_limitstack;
  long           minmax_stackptr;

  long           minmax_acctotal;
  int            minmax_tail;
  ogg_uint32_t   minmax_sampleacc;
  ogg_uint32_t   minmax_sampledesired;

  int            next_to_flush;
  int            last_to_flush;

  double         avgfloat;

  /* unfortunately, we need to hold queued packet data somewhere */
  oggpack_buffer *packetbuffers;
  ogg_packet     *packets;

} bitrate_manager_state;

typedef struct bitrate_manager_info{
  /* detailed bitrate management setup */
  double queue_avg_time;
  double queue_avg_center;
  double queue_minmax_time;
  double queue_hardmin;
  double queue_hardmax;
  double queue_avgmin;
  double queue_avgmax;

  double avgfloat_downslew_max;
  double avgfloat_upslew_max;

} bitrate_manager_info;

extern void vorbis_bitrate_init(vorbis_info *vi,bitrate_manager_state *bs);
extern void vorbis_bitrate_clear(bitrate_manager_state *bs);
extern int vorbis_bitrate_managed(vorbis_block *vb);
extern int vorbis_bitrate_addblock(vorbis_block *vb);
extern int vorbis_bitrate_flushpacket(vorbis_dsp_state *vd, ogg_packet *op);




// ==========================================================================
// ==========================================================================
// codec_internal.h
// ==========================================================================
// ==========================================================================
// function: libvorbis codec headers


#define BLOCKTYPE_IMPULSE    0
#define BLOCKTYPE_PADDING    1
#define BLOCKTYPE_TRANSITION 0
#define BLOCKTYPE_LONG       1

#define PACKETBLOBS 15

typedef struct vorbis_block_internal{
  float  **pcmdelay;  /* this is a pointer into local storage */
  float  ampmax;
  int    blocktype;

  ogg_uint32_t   packetblob_markers[PACKETBLOBS];
} vorbis_block_internal;

typedef void vorbis_look_floor;
typedef void vorbis_look_residue;
typedef void vorbis_look_transform;

/* mode ************************************************************/
typedef struct {
  int blockflag;
  int windowtype;
  int transformtype;
  int mapping;
} vorbis_info_mode;

typedef void vorbis_info_floor;
typedef void vorbis_info_residue;
typedef void vorbis_info_mapping;





#ifndef EHMER_MAX
#define EHMER_MAX 56
#endif

/* psychoacoustic setup ********************************************/
#define P_BANDS 17      /* 62Hz to 16kHz */
#define P_LEVELS 8      /* 30dB to 100dB */
#define P_LEVEL_0 30.    /* 30 dB */
#define P_NOISECURVES 3

#define NOISE_COMPAND_LEVELS 40
typedef struct vorbis_info_psy{
  int   blockflag;

  float ath_adjatt;
  float ath_maxatt;

  float tone_masteratt[P_NOISECURVES];
  float tone_centerboost;
  float tone_decay;
  float tone_abs_limit;
  float toneatt[P_BANDS];

  int noisemaskp;
  float noisemaxsupp;
  float noisewindowlo;
  float noisewindowhi;
  int   noisewindowlomin;
  int   noisewindowhimin;
  int   noisewindowfixed;
  float noiseoff[P_NOISECURVES][P_BANDS];
  float noisecompand[NOISE_COMPAND_LEVELS];

  float max_curve_dB;

  int normal_channel_p;
  int normal_point_p;
  int normal_start;
  int normal_partition;
  double normal_thresh;
} vorbis_info_psy;

typedef struct{
  int   eighth_octave_lines;

  /* for block long/short tuning; encode only */
  float preecho_thresh[VE_BANDS];
  float postecho_thresh[VE_BANDS];
  float stretch_penalty;
  float preecho_minenergy;

  float ampmax_att_per_sec;

  /* channel coupling config */
  int   coupling_pkHz[PACKETBLOBS];
  int   coupling_pointlimit[2][PACKETBLOBS];
  int   coupling_prepointamp[PACKETBLOBS];
  int   coupling_postpointamp[PACKETBLOBS];
  int   sliding_lowpass[2][PACKETBLOBS];

} vorbis_info_psy_global;

typedef struct {
  float ampmax;
  int   channels;

  vorbis_info_psy_global *gi;
  int   coupling_pointlimit[2][P_NOISECURVES];
} vorbis_look_psy_global;


typedef struct {
  int n;
  struct vorbis_info_psy *vi;

  float ***tonecurves;
  float **noiseoffset;

  float *ath;
  long  *octave;             /* in n.ocshift format */
  long  *bark;

  long  firstoc;
  long  shiftoc;
  int   eighth_octave_lines; /* power of two, please */
  int   total_octave_lines;
  long  rate; /* cache it */
} vorbis_look_psy;


extern void   _vp_psy_init(vorbis_look_psy *p,vorbis_info_psy *vi,
               vorbis_info_psy_global *gi,int n,long rate);
extern void   _vp_psy_clear(vorbis_look_psy *p);
extern void  *_vi_psy_dup(void *source);

extern void   _vi_psy_free(vorbis_info_psy *i);
extern vorbis_info_psy *_vi_psy_copy(vorbis_info_psy *i);

extern void _vp_remove_floor(vorbis_look_psy *p,
                 float *mdct,
                 int *icodedflr,
                 float *residue,
                 int sliding_lowpass);

extern void _vp_noisemask(vorbis_look_psy *p,
              float *logmdct,
              float *logmask);

extern void _vp_tonemask(vorbis_look_psy *p,
             float *logfft,
             float *logmask,
             float global_specmax,
             float local_specmax);

extern void _vp_offset_and_mix(vorbis_look_psy *p,
                   float *noise,
                   float *tone,
                   int offset_select,
                   float *logmask);

extern float _vp_ampmax_decay(float amp,vorbis_dsp_state *vd);



typedef struct private_state {
  /* local lookup storage */
  envelope_lookup        *ve; /* envelope lookup */
  float                  *window[2];
  vorbis_look_transform **transform[2];    /* block, type */
  drft_lookup             fft_look[2];

  int                     modebits;
  vorbis_look_floor     **flr;
  vorbis_look_residue   **residue;
  vorbis_look_psy        *psy;
  vorbis_look_psy_global *psy_g_look;

  /* local storage, only used on the encoding side.  This way the
     application does not need to worry about freeing some packets'
     memory and not others'; packet storage is always tracked.
     Cleared next call to a _dsp_ function */
  unsigned char *header;
  unsigned char *header1;
  unsigned char *header2;

  bitrate_manager_state bms;

  ogg_int64_t sample_count;

} private_state;

/* codec_setup_info contains all the setup information specific to the
   specific compression/decompression mode in progress (eg,
   psychoacoustic settings, channel setup, options, codebook
   etc).
*********************************************************************/

typedef struct codec_setup_info {

  /* Vorbis supports only short and long blocks, but allows the
     encoder to choose the sizes */

  long blocksizes[2];

  /* modes are the primary means of supporting on-the-fly different
     blocksizes, different channel mappings (LR or M/A),
     different residue backends, etc.  Each mode consists of a
     blocksize flag and a mapping (along with the mapping setup */

  int        modes;
  int        maps;
  int        floors;
  int        residues;
  int        books;
  int        psys;     /* encode only */

  vorbis_info_mode       *mode_param[64];
  int                     map_type[64];
  vorbis_info_mapping    *map_param[64];
  int                     floor_type[64];
  vorbis_info_floor      *floor_param[64];
  int                     residue_type[64];
  vorbis_info_residue    *residue_param[64];
  static_codebook        *book_param[256];
  codebook               *fullbooks;

  vorbis_info_psy        *psy_param[4]; /* encode only */
  vorbis_info_psy_global psy_g_param;

  bitrate_manager_info   bi;
  highlevel_encode_setup hi; /* used only by vorbisenc.c.  It's a
                                highly redundant structure, but
                                improves clarity of program flow. */

} codec_setup_info;

extern vorbis_look_psy_global *_vp_global_look(vorbis_info *vi);
extern void _vp_global_free(vorbis_look_psy_global *look);




// ==========================================================================
// ==========================================================================
// lpc.h
// ==========================================================================
// ==========================================================================
//  function: LPC low level routines


typedef struct lpclook{
  /* en/decode lookups */
  drft_lookup fft;

  int ln;
  int m;

} lpc_lookup;


extern void lpc_init(lpc_lookup *l,long mapped, int m);
extern void lpc_clear(lpc_lookup *l);

/* simple linear scale LPC code */
extern float vorbis_lpc_from_data(float *data,float *lpc,int n,int m);
extern float vorbis_lpc_from_curve(float *curve,float *lpc,lpc_lookup *l);

extern void vorbis_lpc_predict(float *coeff,float *prime,int m,
                   float *data,long n);



// ==========================================================================
// ==========================================================================
// lsp.h
// ==========================================================================
// ==========================================================================
//  function: LSP (also called LSF) conversion routines

extern int vorbis_lpc_to_lsp(float *lpc,float *lsp,int m);

extern void vorbis_lsp_to_curve(float *curve,int *map,int n,int ln,
                float *lsp,int m,
                float amp,float ampoffset);



// ==========================================================================
// ==========================================================================
// some common routines
// ==========================================================================
// ==========================================================================
// function: miscellaneous prototypes


extern int analysis_noisy;

extern void *_vorbis_block_alloc(vorbis_block *vb,long bytes);
extern void _vorbis_block_ripcord(vorbis_block *vb);
extern void _analysis_output(char *base,int i,float *v,int n,int bark,int dB,
                 ogg_int64_t off);

#ifdef DEBUG_MALLOC

#define _VDBG_GRAPHFILE "malloc.m"
extern void *_VDBG_malloc(void *ptr,long bytes,char *file,long line);
extern void _VDBG_free(void *ptr,char *file,long line);

#ifndef MISC_C
#undef _ogg_malloc
#undef _ogg_calloc
#undef _ogg_realloc
#undef _ogg_free

#define _ogg_malloc(x) _VDBG_malloc(NULL,(x),__FILE__,__LINE__)
#define _ogg_calloc(x,y) _VDBG_malloc(NULL,(x)*(y),__FILE__,__LINE__)
#define _ogg_realloc(x,y) _VDBG_malloc((x),(y),__FILE__,__LINE__)
#define _ogg_free(x) _VDBG_free((x),__FILE__,__LINE__)
#endif
#endif





// ==========================================================================
// ==========================================================================
// backends.h
// ==========================================================================
// ==========================================================================
// function: libvorbis backend and mapping structures; needed for mode headers

/* this is exposed up here because we need it for modes.
   Lookups for each backend aren't exposed because there's no reason
   to do so */



/* this would all be simpler/shorter with templates, but.... */
/* Floor backend generic *****************************************/
typedef struct{
  void                   (*pack)  (vorbis_info_floor *,oggpack_buffer *);
  vorbis_info_floor     *(*unpack)(vorbis_info *,oggpack_buffer *);
  vorbis_look_floor     *(*look)  (vorbis_dsp_state *,vorbis_info_floor *);
  void (*free_info) (vorbis_info_floor *);
  void (*free_look) (vorbis_look_floor *);
  void *(*inverse1)  (struct vorbis_block *,vorbis_look_floor *);
  int   (*inverse2)  (struct vorbis_block *,vorbis_look_floor *,
             void *buffer,float *);
} vorbis_func_floor;

typedef struct{
  int   order;
  long  rate;
  long  barkmap;

  int   ampbits;
  int   ampdB;

  int   numbooks; /* <= 16 */
  int   books[16];

  float lessthan;     /* encode-only config setting hacks for libvorbis */
  float greaterthan;  /* encode-only config setting hacks for libvorbis */

} vorbis_info_floor0;


#define VIF_POSIT 63
#define VIF_CLASS 16
#define VIF_PARTS 31
typedef struct{
  int   partitions;                /* 0 to 31 */
  int   partitionclass[VIF_PARTS]; /* 0 to 15 */

  int   class_dim[VIF_CLASS];        /* 1 to 8 */
  int   class_subs[VIF_CLASS];       /* 0,1,2,3 (bits: 1<<n poss) */
  int   class_book[VIF_CLASS];       /* subs ^ dim entries */
  int   class_subbook[VIF_CLASS][8]; /* [VIF_CLASS][subs] */


  int   mult;                      /* 1 2 3 or 4 */
  int   postlist[VIF_POSIT+2];    /* first two implicit */


  /* encode side analysis parameters */
  float maxover;
  float maxunder;
  float maxerr;

  float twofitweight;
  float twofitatten;

  int   n;

} vorbis_info_floor1;

/* Residue backend generic *****************************************/
typedef struct{
  void                 (*pack)  (vorbis_info_residue *,oggpack_buffer *);
  vorbis_info_residue *(*unpack)(vorbis_info *,oggpack_buffer *);
  vorbis_look_residue *(*look)  (vorbis_dsp_state *,
                 vorbis_info_residue *);
  void (*free_info)    (vorbis_info_residue *);
  void (*free_look)    (vorbis_look_residue *);
  long **(*class_lu)   (struct vorbis_block *,vorbis_look_residue *,
            float **,int *,int);
  int  (*forward)      (struct vorbis_block *,vorbis_look_residue *,
            float **,float **,int *,int,long **);
  int  (*inverse)      (struct vorbis_block *,vorbis_look_residue *,
            float **,int *,int);
} vorbis_func_residue;

typedef struct vorbis_info_residue0{
/* block-partitioned VQ coded straight residue */
  long  begin;
  long  end;

  /* first stage (lossless partitioning) */
  int    grouping;         /* group n vectors per partition */
  int    partitions;       /* possible codebooks for a partition */
  int    groupbook;        /* huffbook for partitioning */
  int    secondstages[64]; /* expanded out to pointers in lookup */
  int    booklist[256];    /* list of second stage books */

  float  classmetric1[64];
  float  classmetric2[64];

} vorbis_info_residue0;

/* Mapping backend generic *****************************************/
typedef struct{
  void                 (*pack)  (vorbis_info *,vorbis_info_mapping *,
                 oggpack_buffer *);
  vorbis_info_mapping *(*unpack)(vorbis_info *,oggpack_buffer *);
  void (*free_info)    (vorbis_info_mapping *);
  int  (*forward)      (struct vorbis_block *vb);
  int  (*inverse)      (struct vorbis_block *vb,vorbis_info_mapping *);
} vorbis_func_mapping;

typedef struct vorbis_info_mapping0{
  int   submaps;  /* <= 16 */
  int   chmuxlist[256];   /* up to 256 channels in a Vorbis stream */

  int   floorsubmap[16];   /* [mux] submap to floors */
  int   residuesubmap[16]; /* [mux] submap to residue */

  int   coupling_steps;
  int   coupling_mag[256];
  int   coupling_ang[256];

} vorbis_info_mapping0;




// ==========================================================================
// ==========================================================================
// registry.h
// ==========================================================================
// ==========================================================================
// function: registry for time, floor, res backends and channel mappings


#define VI_TRANSFORMB 1
#define VI_WINDOWB 1
#define VI_TIMEB 1
#define VI_FLOORB 2
#define VI_RESB 3
#define VI_MAPB 1

extern vorbis_func_floor     *_floor_P[];
extern vorbis_func_residue   *_residue_P[];
extern vorbis_func_mapping   *_mapping_P[];





// ==========================================================================
// ==========================================================================
// scales.h
// ==========================================================================
// ==========================================================================
// function: linear scale -> dB, Bark and Mel scales


/* 20log10(x) */
#define VORBIS_IEEE_FLOAT32 1
#ifdef VORBIS_IEEE_FLOAT32

float unitnorm(float x){
  ogg_uint32_t *ix=(ogg_uint32_t *)&x;
  *ix=(*ix&0x80000000UL)|(0x3f800000UL);
  return(x);
}

float FABS(float *x){
  ogg_uint32_t *ix=(ogg_uint32_t *)x;
  *ix&=0x7fffffffUL;
  return(*x);
}

float todB(const float *x){
  float calc;
  ogg_int32_t *i=(ogg_int32_t *)x;
  calc = (float)(((*i) & 0x7fffffff));
  calc *= 7.1771144e-7f;
  calc += -764.27118f;
  return calc;
}

#define todB_nn(x) todB(x)

#else

float unitnorm(float x){
  if(x<0)return(-1.f);
  return(1.f);
}

#define FABS(x) fabs(*(x))

#define todB(x)   (*(x)==0?-400.f:log(*(x)**(x))*4.34294480f)
#define todB_nn(x)   (*(x)==0.f?-400.f:log(*(x))*8.6858896f)

#endif

#define fromdB(x) (exp((x)*.11512925f))

/* The bark scale equations are approximations, since the original
   table was somewhat hand rolled.  The below are chosen to have the
   best possible fit to the rolled tables, thus their somewhat odd
   appearance (these are more accurate and over a longer range than
   the oft-quoted bark equations found in the texts I have).  The
   approximations are valid from 0 - 30kHz (nyquist) or so.

   all f in Hz, z in Bark */

#define toBARK(n)   (13.1f*atan(.00074f*(n))+2.24f*atan((n)*(n)*1.85e-8f)+1e-4f*(n))
#define fromBARK(z) (102.f*(z)-2.f*pow(z,2.f)+.4f*pow(z,3.f)+pow(1.46f,z)-1.f)
#define toMEL(n)    (log(1.f+(n)*.001f)*1442.695f)
#define fromMEL(m)  (1000.f*exp((m)/1442.695f)-1000.f)

/* Frequency to octave.  We arbitrarily declare 63.5 Hz to be octave
   0.0 */

#define toOC(n)     (log(n)*1.442695f-5.965784f)
#define fromOC(o)   (exp(((o)+5.965784f)*.693147f))






// ==========================================================================
// ==========================================================================
// window.h
// ==========================================================================
// ==========================================================================
// function: window functions

extern float *_vorbis_window(int type,int left);
extern void _vorbis_apply_window(float *d,float *window[2],long *blocksizes,
                 int lW,int W,int nW);



// ==========================================================================
// ==========================================================================
// vorbisfile.h
// ==========================================================================
// ==========================================================================
// function: stdio-based convenience library for opening/seeking/decoding



/* The function prototypes for the callbacks are basically the same as for
 * the stdio functions fread, fseek, fclose, ftell.
 * The one difference is that the FILE * arguments have been replaced with
 * a void * - this is to be used as a pointer to whatever internal data these
 * functions might need. In the stdio case, it's just a FILE * cast to a void *
 *
 * If you use other functions, check the docs for these functions and return
 * the right values. For seek_func(), you *MUST* return -1 if the stream is
 * unseekable
 */
typedef struct {
  size_t (*read_func)  (void *ptr, size_t size, size_t nmemb, void *datasource);
  int    (*seek_func)  (void *datasource, ogg_int64_t offset, int whence);
  int    (*close_func) (void *datasource);
  long   (*tell_func)  (void *datasource);
} ov_callbacks;

#define  NOTOPEN   0
#define  PARTOPEN  1
#define  OPENED    2
#define  STREAMSET 3
#define  INITSET   4

typedef struct OggVorbis_File {
  void            *datasource; /* Pointer to a FILE *, etc. */
  int              seekable;
  ogg_int64_t      offset;
  ogg_int64_t      end;
  ogg_sync_state   oy;

  /* If the FILE handle isn't seekable (eg, a pipe), only the current
     stream appears */
  int              links;
  ogg_int64_t     *offsets;
  ogg_int64_t     *dataoffsets;
  long            *serialnos;
  ogg_int64_t     *pcmlengths; /* overloaded to maintain binary
                  compatability; x2 size, stores both
                  beginning and end values */
  vorbis_info     *vi;
  vorbis_comment  *vc;

  /* Decoding working state local storage */
  ogg_int64_t      pcm_offset;
  int              ready_state;
  long             current_serialno;
  int              current_link;

  double           bittrack;
  double           samptrack;

  ogg_stream_state os; /* take physical pages, weld into a logical
                          stream of packets */
  vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
  vorbis_block     vb; /* local working space for packet->PCM decode */

  ov_callbacks callbacks;

} OggVorbis_File;

extern int ov_clear(OggVorbis_File *vf);
extern int ov_open(FILE *f,OggVorbis_File *vf,char *initial,long ibytes);
extern int ov_open_callbacks(void *datasource, OggVorbis_File *vf,
        char *initial, long ibytes, ov_callbacks callbacks);

extern int ov_test(FILE *f,OggVorbis_File *vf,char *initial,long ibytes);
extern int ov_test_callbacks(void *datasource, OggVorbis_File *vf,
        char *initial, long ibytes, ov_callbacks callbacks);
extern int ov_test_open(OggVorbis_File *vf);

extern long ov_bitrate(OggVorbis_File *vf,int i);
extern long ov_bitrate_instant(OggVorbis_File *vf);
extern long ov_streams(OggVorbis_File *vf);
extern long ov_seekable(OggVorbis_File *vf);
extern long ov_serialnumber(OggVorbis_File *vf,int i);

extern ogg_int64_t ov_raw_total(OggVorbis_File *vf,int i);
extern ogg_int64_t ov_pcm_total(OggVorbis_File *vf,int i);
extern double ov_time_total(OggVorbis_File *vf,int i);

extern int ov_raw_seek(OggVorbis_File *vf,ogg_int64_t pos);
extern int ov_pcm_seek(OggVorbis_File *vf,ogg_int64_t pos);
extern int ov_pcm_seek_page(OggVorbis_File *vf,ogg_int64_t pos);
extern int ov_time_seek(OggVorbis_File *vf,double pos);
extern int ov_time_seek_page(OggVorbis_File *vf,double pos);

extern ogg_int64_t ov_raw_tell(OggVorbis_File *vf);
extern ogg_int64_t ov_pcm_tell(OggVorbis_File *vf);
extern double ov_time_tell(OggVorbis_File *vf);

extern vorbis_info *ov_info(OggVorbis_File *vf,int link);
extern vorbis_comment *ov_comment(OggVorbis_File *vf,int link);

extern long ov_read_float(OggVorbis_File *vf,float ***pcm_channels,int samples,
              int *bitstream);
extern long ov_read(OggVorbis_File *vf,char *buffer,int length,
            int bigendianp,int word,int sgned,int *bitstream);




// ==========================================================================
// ==========================================================================
// some common routines
// ==========================================================================
// ==========================================================================

ogg_uint32_t bitreverse(ogg_uint32_t x){
  x=    ((x>>16)&0x0000ffffUL) | ((x<<16)&0xffff0000UL);
  x=    ((x>> 8)&0x00ff00ffUL) | ((x<< 8)&0xff00ff00UL);
  x=    ((x>> 4)&0x0f0f0f0fUL) | ((x<< 4)&0xf0f0f0f0UL);
  x=    ((x>> 2)&0x33333333UL) | ((x<< 2)&0xccccccccUL);
  return((x>> 1)&0x55555555UL) | ((x<< 1)&0xaaaaaaaaUL);
}


int ilog(unsigned int v){
  int ret=0;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

int ilog2(unsigned int v){
  int ret=0;
  if(v)--v;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}






// ==========================================================================
// ==========================================================================
// framing.c
// ==========================================================================
// ==========================================================================

// function: code raw [Vorbis] packets into framed OggSquish stream and
//           decode Ogg streams back into raw packets
// note: The CRC code is directly derived from public domain code by
// Ross Williams (ross@guest.adelaide.edu.au).  See docs/framing.html
// for details.


/* A complete description of Ogg framing exists in docs/framing.html */

int ogg_page_version(ogg_page *og){
  return((int)(og->header[4]));
}

int ogg_page_continued(ogg_page *og){
  return((int)(og->header[5]&0x01));
}

int ogg_page_bos(ogg_page *og){
  return((int)(og->header[5]&0x02));
}

int ogg_page_eos(ogg_page *og){
  return((int)(og->header[5]&0x04));
}

ogg_int64_t ogg_page_granulepos(ogg_page *og){
  unsigned char *page=og->header;
  ogg_int64_t granulepos=page[13]&(0xff);
  granulepos= (granulepos<<8)|(page[12]&0xff);
  granulepos= (granulepos<<8)|(page[11]&0xff);
  granulepos= (granulepos<<8)|(page[10]&0xff);
  granulepos= (granulepos<<8)|(page[9]&0xff);
  granulepos= (granulepos<<8)|(page[8]&0xff);
  granulepos= (granulepos<<8)|(page[7]&0xff);
  granulepos= (granulepos<<8)|(page[6]&0xff);
  return(granulepos);
}

int ogg_page_serialno(ogg_page *og){
  return(og->header[14] |
     (og->header[15]<<8) |
     (og->header[16]<<16) |
     (og->header[17]<<24));
}

long ogg_page_pageno(ogg_page *og){
  return(og->header[18] |
     (og->header[19]<<8) |
     (og->header[20]<<16) |
     (og->header[21]<<24));
}



/* returns the number of packets that are completed on this page (if
   the leading packet is begun on a previous page, but ends on this
   page, it's counted */

/* NOTE:
If a page consists of a packet begun on a previous page, and a new
packet begun (but not completed) on this page, the return will be:
  ogg_page_packets(page)   ==1,
  ogg_page_continued(page) !=0

If a page happens to be a single packet that was begun on a
previous page, and spans to the next page (in the case of a three or
more page packet), the return will be:
  ogg_page_packets(page)   ==0,
  ogg_page_continued(page) !=0
*/

int ogg_page_packets(ogg_page *og){
  int i,n=og->header[26],count=0;
  for(i=0;i<n;i++)
    if(og->header[27+i]<255)count++;
  return(count);
}


#if 0
/* helper to initialize lookup for direct-table CRC (illustrative; we
   use the init below) */

ogg_uint32_t _ogg_crc_entry(unsigned long index){
  int           i;
  unsigned long r;

  r = index << 24;
  for (i=0; i<8; i++)
    if (r & 0x80000000UL)
      r = (r << 1) ^ 0x04c11db7; /* The same as the ethernet generator
                    polynomial, although we use an
                    unreflected alg and an init/final
                    of 0, not 0xffffffff */
    else
       r<<=1;
 return (r & 0xffffffffUL);
}
#endif

ogg_uint32_t crc_lookup[256]={
  0x00000000,0x04c11db7,0x09823b6e,0x0d4326d9,
  0x130476dc,0x17c56b6b,0x1a864db2,0x1e475005,
  0x2608edb8,0x22c9f00f,0x2f8ad6d6,0x2b4bcb61,
  0x350c9b64,0x31cd86d3,0x3c8ea00a,0x384fbdbd,
  0x4c11db70,0x48d0c6c7,0x4593e01e,0x4152fda9,
  0x5f15adac,0x5bd4b01b,0x569796c2,0x52568b75,
  0x6a1936c8,0x6ed82b7f,0x639b0da6,0x675a1011,
  0x791d4014,0x7ddc5da3,0x709f7b7a,0x745e66cd,
  0x9823b6e0,0x9ce2ab57,0x91a18d8e,0x95609039,
  0x8b27c03c,0x8fe6dd8b,0x82a5fb52,0x8664e6e5,
  0xbe2b5b58,0xbaea46ef,0xb7a96036,0xb3687d81,
  0xad2f2d84,0xa9ee3033,0xa4ad16ea,0xa06c0b5d,
  0xd4326d90,0xd0f37027,0xddb056fe,0xd9714b49,
  0xc7361b4c,0xc3f706fb,0xceb42022,0xca753d95,
  0xf23a8028,0xf6fb9d9f,0xfbb8bb46,0xff79a6f1,
  0xe13ef6f4,0xe5ffeb43,0xe8bccd9a,0xec7dd02d,
  0x34867077,0x30476dc0,0x3d044b19,0x39c556ae,
  0x278206ab,0x23431b1c,0x2e003dc5,0x2ac12072,
  0x128e9dcf,0x164f8078,0x1b0ca6a1,0x1fcdbb16,
  0x018aeb13,0x054bf6a4,0x0808d07d,0x0cc9cdca,
  0x7897ab07,0x7c56b6b0,0x71159069,0x75d48dde,
  0x6b93dddb,0x6f52c06c,0x6211e6b5,0x66d0fb02,
  0x5e9f46bf,0x5a5e5b08,0x571d7dd1,0x53dc6066,
  0x4d9b3063,0x495a2dd4,0x44190b0d,0x40d816ba,
  0xaca5c697,0xa864db20,0xa527fdf9,0xa1e6e04e,
  0xbfa1b04b,0xbb60adfc,0xb6238b25,0xb2e29692,
  0x8aad2b2f,0x8e6c3698,0x832f1041,0x87ee0df6,
  0x99a95df3,0x9d684044,0x902b669d,0x94ea7b2a,
  0xe0b41de7,0xe4750050,0xe9362689,0xedf73b3e,
  0xf3b06b3b,0xf771768c,0xfa325055,0xfef34de2,
  0xc6bcf05f,0xc27dede8,0xcf3ecb31,0xcbffd686,
  0xd5b88683,0xd1799b34,0xdc3abded,0xd8fba05a,
  0x690ce0ee,0x6dcdfd59,0x608edb80,0x644fc637,
  0x7a089632,0x7ec98b85,0x738aad5c,0x774bb0eb,
  0x4f040d56,0x4bc510e1,0x46863638,0x42472b8f,
  0x5c007b8a,0x58c1663d,0x558240e4,0x51435d53,
  0x251d3b9e,0x21dc2629,0x2c9f00f0,0x285e1d47,
  0x36194d42,0x32d850f5,0x3f9b762c,0x3b5a6b9b,
  0x0315d626,0x07d4cb91,0x0a97ed48,0x0e56f0ff,
  0x1011a0fa,0x14d0bd4d,0x19939b94,0x1d528623,
  0xf12f560e,0xf5ee4bb9,0xf8ad6d60,0xfc6c70d7,
  0xe22b20d2,0xe6ea3d65,0xeba91bbc,0xef68060b,
  0xd727bbb6,0xd3e6a601,0xdea580d8,0xda649d6f,
  0xc423cd6a,0xc0e2d0dd,0xcda1f604,0xc960ebb3,
  0xbd3e8d7e,0xb9ff90c9,0xb4bcb610,0xb07daba7,
  0xae3afba2,0xaafbe615,0xa7b8c0cc,0xa379dd7b,
  0x9b3660c6,0x9ff77d71,0x92b45ba8,0x9675461f,
  0x8832161a,0x8cf30bad,0x81b02d74,0x857130c3,
  0x5d8a9099,0x594b8d2e,0x5408abf7,0x50c9b640,
  0x4e8ee645,0x4a4ffbf2,0x470cdd2b,0x43cdc09c,
  0x7b827d21,0x7f436096,0x7200464f,0x76c15bf8,
  0x68860bfd,0x6c47164a,0x61043093,0x65c52d24,
  0x119b4be9,0x155a565e,0x18197087,0x1cd86d30,
  0x029f3d35,0x065e2082,0x0b1d065b,0x0fdc1bec,
  0x3793a651,0x3352bbe6,0x3e119d3f,0x3ad08088,
  0x2497d08d,0x2056cd3a,0x2d15ebe3,0x29d4f654,
  0xc5a92679,0xc1683bce,0xcc2b1d17,0xc8ea00a0,
  0xd6ad50a5,0xd26c4d12,0xdf2f6bcb,0xdbee767c,
  0xe3a1cbc1,0xe760d676,0xea23f0af,0xeee2ed18,
  0xf0a5bd1d,0xf464a0aa,0xf9278673,0xfde69bc4,
  0x89b8fd09,0x8d79e0be,0x803ac667,0x84fbdbd0,
  0x9abc8bd5,0x9e7d9662,0x933eb0bb,0x97ffad0c,
  0xafb010b1,0xab710d06,0xa6322bdf,0xa2f33668,
  0xbcb4666d,0xb8757bda,0xb5365d03,0xb1f740b4};

/* init the encode/decode logical stream state */

int ogg_stream_init(ogg_stream_state *os,int serialno){
  if(os){
    memset(os,0,sizeof(*os));
    os->body_storage=16*1024;
    os->body_data=static_cast<unsigned char*>(_ogg_malloc(os->body_storage*sizeof(*os->body_data)));

    os->lacing_storage=1024;
    os->lacing_vals=static_cast<int*>(_ogg_malloc(os->lacing_storage*sizeof(*os->lacing_vals)));
    os->granule_vals=static_cast<ogg_int64_t*>(_ogg_malloc(os->lacing_storage*sizeof(*os->granule_vals)));

    os->serialno=serialno;

    return(0);
  }
  return(-1);
}

/* _clear does not free os, only the non-flat storage within */
int ogg_stream_clear(ogg_stream_state *os){
  if(os){
    if(os->body_data)_ogg_free(os->body_data);
    if(os->lacing_vals)_ogg_free(os->lacing_vals);
    if(os->granule_vals)_ogg_free(os->granule_vals);

    memset(os,0,sizeof(*os));
  }
  return(0);
}

int ogg_stream_destroy(ogg_stream_state *os){
  if(os){
    ogg_stream_clear(os);
    _ogg_free(os);
  }
  return(0);
}

/* Helpers for ogg_stream_encode; this keeps the structure and
   what's happening fairly clear */

void _os_body_expand(ogg_stream_state *os,int needed){
  if(os->body_storage<=os->body_fill+needed){
    os->body_storage+=(needed+1024);
    os->body_data=static_cast<unsigned char*>(_ogg_realloc(os->body_data,os->body_storage*sizeof(*os->body_data)));
  }
}

void _os_lacing_expand(ogg_stream_state *os,int needed){
  if(os->lacing_storage<=os->lacing_fill+needed){
    os->lacing_storage+=(needed+32);
    os->lacing_vals=static_cast<int*>(_ogg_realloc(os->lacing_vals,os->lacing_storage*sizeof(*os->lacing_vals)));
    os->granule_vals=static_cast<ogg_int64_t*>(_ogg_realloc(os->granule_vals,os->lacing_storage*sizeof(*os->granule_vals)));
  }
}

/* checksum the page */
/* Direct table CRC; note that this will be faster in the future if we
   perform the checksum silmultaneously with other copies */

void ogg_page_checksum_set(ogg_page *og){
  if(og){
    ogg_uint32_t crc_reg=0;
    int i;

    /* safety; needed for API behavior, but not framing code */
    og->header[22]=0;
    og->header[23]=0;
    og->header[24]=0;
    og->header[25]=0;

    for(i=0;i<og->header_len;i++)
      crc_reg=(crc_reg<<8)^crc_lookup[((crc_reg >> 24)&0xff)^og->header[i]];
    for(i=0;i<og->body_len;i++)
      crc_reg=(crc_reg<<8)^crc_lookup[((crc_reg >> 24)&0xff)^og->body[i]];

    og->header[22]=(unsigned char)(crc_reg&0xff);
    og->header[23]=(unsigned char)((crc_reg>>8)&0xff);
    og->header[24]=(unsigned char)((crc_reg>>16)&0xff);
    og->header[25]=(unsigned char)((crc_reg>>24)&0xff);
  }
}

/* submit data to the internal buffer of the framing engine */
int ogg_stream_packetin(ogg_stream_state *os,ogg_packet *op){
  int lacing_vals=op->bytes/255+1,i;

  if(os->body_returned){
    /* advance packet data according to the body_returned pointer. We
       had to keep it around to return a pointer into the buffer last
       call */

    os->body_fill-=os->body_returned;
    if(os->body_fill)
      memmove(os->body_data,os->body_data+os->body_returned,
          os->body_fill);
    os->body_returned=0;
  }

  /* make sure we have the buffer storage */
  _os_body_expand(os,op->bytes);
  _os_lacing_expand(os,lacing_vals);

  /* Copy in the submitted packet.  Yes, the copy is a waste; this is
     the liability of overly clean abstraction for the time being.  It
     will actually be fairly easy to eliminate the extra copy in the
     future */

  memcpy(os->body_data+os->body_fill,op->packet,op->bytes);
  os->body_fill+=op->bytes;

  /* Store lacing vals for this packet */
  for(i=0;i<lacing_vals-1;i++){
    os->lacing_vals[os->lacing_fill+i]=255;
    os->granule_vals[os->lacing_fill+i]=os->granulepos;
  }
  os->lacing_vals[os->lacing_fill+i]=(op->bytes)%255;
  os->granulepos=os->granule_vals[os->lacing_fill+i]=op->granulepos;

  /* flag the first segment as the beginning of the packet */
  os->lacing_vals[os->lacing_fill]|= 0x100;

  os->lacing_fill+=lacing_vals;

  /* for the sake of completeness */
  os->packetno++;

  if(op->e_o_s)os->e_o_s=1;

  return(0);
}

/* This will flush remaining packets into a page (returning nonzero),
   even if there is not enough data to trigger a flush normally
   (undersized page). If there are no packets or partial packets to
   flush, ogg_stream_flush returns 0.  Note that ogg_stream_flush will
   try to flush a normal sized page like ogg_stream_pageout; a call to
   ogg_stream_flush does not guarantee that all packets have flushed.
   Only a return value of 0 from ogg_stream_flush indicates all packet
   data is flushed into pages.

   since ogg_stream_flush will flush the last page in a stream even if
   it's undersized, you almost certainly want to use ogg_stream_pageout
   (and *not* ogg_stream_flush) unless you specifically need to flush
   an page regardless of size in the middle of a stream. */

int ogg_stream_flush(ogg_stream_state *os,ogg_page *og){
  int i;
  int vals=0; vals; // unused?
  int maxvals=(os->lacing_fill>255?255:os->lacing_fill);
  int bytes=0;
  long acc=0;
  ogg_int64_t granule_pos=os->granule_vals[0];

  if(maxvals==0)return(0);

  /* construct a page */
  /* decide how many segments to include */

  /* If this is the initial header case, the first page must only include
     the initial header packet */
  if(os->b_o_s==0){  /* 'initial header page' case */
    granule_pos=0;
    for(vals=0;vals<maxvals;vals++){
      if((os->lacing_vals[vals]&0x0ff)<255){
    vals++;
    break;
      }
    }
  }else{
    for(vals=0;vals<maxvals;vals++){
      if(acc>4096)break;
      acc+=os->lacing_vals[vals]&0x0ff;
      granule_pos=os->granule_vals[vals];
    }
  }

  /* construct the header in temp storage */
  memcpy(os->header,"OggS",4);

  /* stream structure version */
  os->header[4]=0x00;

  /* continued packet flag? */
  os->header[5]=0x00;
  if((os->lacing_vals[0]&0x100)==0)os->header[5]|=0x01;
  /* first page flag? */
  if(os->b_o_s==0)os->header[5]|=0x02;
  /* last page flag? */
  if(os->e_o_s && os->lacing_fill==vals)os->header[5]|=0x04;
  os->b_o_s=1;

  /* 64 bits of PCM position */
  for(i=6;i<14;i++){
    os->header[i]=(unsigned char)((granule_pos&0xff));
    granule_pos>>=8;
  }

  /* 32 bits of stream serial number */
  {
    long serialno=os->serialno;
    for(i=14;i<18;i++){
      os->header[i]=(unsigned char)((serialno&0xff));
      serialno>>=8;
    }
  }

  /* 32 bits of page counter (we have both counter and page header
     because this val can roll over) */
  if(os->pageno==-1)os->pageno=0; /* because someone called
                     stream_reset; this would be a
                     strange thing to do in an
                     encode stream, but it has
                     plausible uses */
  {
    long pageno=os->pageno++;
    for(i=18;i<22;i++){
      os->header[i]=(unsigned char)(pageno&0xff);
      pageno>>=8;
    }
  }

  /* zero for computation; filled in later */
  os->header[22]=0;
  os->header[23]=0;
  os->header[24]=0;
  os->header[25]=0;

  /* segment table */
  os->header[26]=(unsigned char)(vals&0xff);
  for(i=0;i<vals;i++)
    bytes+=os->header[i+27]=(unsigned char)(os->lacing_vals[i]&0xff);

  /* set pointers in the ogg_page struct */
  og->header=os->header;
  og->header_len=os->header_fill=vals+27;
  og->body=os->body_data+os->body_returned;
  og->body_len=bytes;

  /* advance the lacing data and set the body_returned pointer */

  os->lacing_fill-=vals;
  memmove(os->lacing_vals,os->lacing_vals+vals,os->lacing_fill*sizeof(*os->lacing_vals));
  memmove(os->granule_vals,os->granule_vals+vals,os->lacing_fill*sizeof(*os->granule_vals));
  os->body_returned+=bytes;

  /* calculate the checksum */

  ogg_page_checksum_set(og);

  /* done */
  return(1);
}


/* This constructs pages from buffered packet segments.  The pointers
returned are to buffers; do not free. The returned buffers are
good only until the next call (using the same ogg_stream_state) */

int ogg_stream_pageout(ogg_stream_state *os, ogg_page *og){

  if((os->e_o_s&&os->lacing_fill) ||          /* 'were done, now flush' case */
     os->body_fill-os->body_returned > 4096 ||/* 'page nominal size' case */
     os->lacing_fill>=255 ||                  /* 'segment table full' case */
     (os->lacing_fill&&!os->b_o_s)){          /* 'initial header page' case */

    return(ogg_stream_flush(os,og));
  }

  /* not enough data to construct a page and not end of stream */
  return(0);
}

int ogg_stream_eos(ogg_stream_state *os){
  return os->e_o_s;
}

/* DECODING PRIMITIVES: packet streaming layer **********************/

/* This has two layers to place more of the multi-serialno and paging
   control in the application's hands.  First, we expose a data buffer
   using ogg_sync_buffer().  The app either copies into the
   buffer, or passes it directly to read(), etc.  We then call
   ogg_sync_wrote() to tell how many bytes we just added.

   Pages are returned (pointers into the buffer in ogg_sync_state)
   by ogg_sync_pageout().  The page is then submitted to
   ogg_stream_pagein() along with the appropriate
   ogg_stream_state* (ie, matching serialno).  We then get raw
   packets out calling ogg_stream_packetout() with a
   ogg_stream_state.  See the 'frame-prog.txt' docs for details and
   example code. */

/* initialize the struct to a known state */
int ogg_sync_init(ogg_sync_state *oy){
  if(oy){
    memset(oy,0,sizeof(*oy));
  }
  return(0);
}

/* clear non-flat storage within */
int ogg_sync_clear(ogg_sync_state *oy){
  if(oy){
    if(oy->data)_ogg_free(oy->data);
    ogg_sync_init(oy);
  }
  return(0);
}

int ogg_sync_destroy(ogg_sync_state *oy){
  if(oy){
    ogg_sync_clear(oy);
    _ogg_free(oy);
  }
  return(0);
}

char *ogg_sync_buffer(ogg_sync_state *oy, long size){

  /* first, clear out any space that has been previously returned */
  if(oy->returned){
    oy->fill-=oy->returned;
    if(oy->fill>0)
      memmove(oy->data,oy->data+oy->returned,oy->fill);
    oy->returned=0;
  }

  if(size>oy->storage-oy->fill){
    /* We need to extend the internal buffer */
    long newsize=size+oy->fill+4096; /* an extra page to be nice */

    if(oy->data)
      oy->data=static_cast<unsigned char*>(_ogg_realloc(oy->data,newsize));
    else
      oy->data=static_cast<unsigned char*>(_ogg_malloc(newsize));
    oy->storage=newsize;
  }

  /* expose a segment at least as large as requested at the fill mark */
  return((char *)oy->data+oy->fill);
}

int ogg_sync_wrote(ogg_sync_state *oy, long bytes){
  if(oy->fill+bytes>oy->storage)return(-1);
  oy->fill+=bytes;
  return(0);
}

/* sync the stream.  This is meant to be useful for finding page
   boundaries.

   return values for this:
  -n) skipped n bytes
   0) page not ready; more data (no bytes skipped)
   n) page synced at current location; page length n bytes

*/

long ogg_sync_pageseek(ogg_sync_state *oy,ogg_page *og){
  unsigned char *page=oy->data+oy->returned;
  unsigned char *next;
  long bytes=oy->fill-oy->returned;

  if(oy->headerbytes==0){
    int headerbytes,i;
    if(bytes<27)return(0); /* not enough for a header */

    /* verify capture pattern */
    if(memcmp(page,"OggS",4))goto sync_fail;

    headerbytes=page[26]+27;
    if(bytes<headerbytes)return(0); /* not enough for header + seg table */

    /* count up body length in the segment table */

    for(i=0;i<page[26];i++)
      oy->bodybytes+=page[27+i];
    oy->headerbytes=headerbytes;
  }

  if(oy->bodybytes+oy->headerbytes>bytes)return(0);

  /* The whole test page is buffered.  Verify the checksum */
  {
    /* Grab the checksum bytes, set the header field to zero */
    char chksum[4];
    ogg_page log;

    memcpy(chksum,page+22,4);
    memset(page+22,0,4);

    /* set up a temp page struct and recompute the checksum */
    log.header=page;
    log.header_len=oy->headerbytes;
    log.body=page+oy->headerbytes;
    log.body_len=oy->bodybytes;
    ogg_page_checksum_set(&log);

    /* Compare */
    if(memcmp(chksum,page+22,4)){
      /* D'oh.  Mismatch! Corrupt page (or miscapture and not a page
     at all) */
      /* replace the computed checksum with the one actually read in */
      memcpy(page+22,chksum,4);

      /* Bad checksum. Lose sync */
      goto sync_fail;
    }
  }

  /* yes, have a whole page all ready to go */
  {
    unsigned char *page=oy->data+oy->returned;
    long bytes;

    if(og){
      og->header=page;
      og->header_len=oy->headerbytes;
      og->body=page+oy->headerbytes;
      og->body_len=oy->bodybytes;
    }

    oy->unsynced=0;
    oy->returned+=(bytes=oy->headerbytes+oy->bodybytes);
    oy->headerbytes=0;
    oy->bodybytes=0;
    return(bytes);
  }

 sync_fail:

  oy->headerbytes=0;
  oy->bodybytes=0;

  /* search for possible capture */
  next=static_cast<unsigned char*>(memchr(page+1,'O',bytes-1));
  if(!next)
    next=oy->data+oy->fill;

  oy->returned=(int)(next-oy->data);
  return (long)(-(next-page));
}

/* sync the stream and get a page.  Keep trying until we find a page.
   Supress 'sync errors' after reporting the first.

   return values:
   -1) recapture (hole in data)
    0) need more data
    1) page returned

   Returns pointers into buffered data; invalidated by next call to
   _stream, _clear, _init, or _buffer */

int ogg_sync_pageout(ogg_sync_state *oy, ogg_page *og){

  /* all we need to do is verify a page at the head of the stream
     buffer.  If it doesn't verify, we look for the next potential
     frame */

  for (;;) {
    long ret=ogg_sync_pageseek(oy,og);
    if(ret>0){
      /* have a page */
      return(1);
    }
    if(ret==0){
      /* need more data */
      return(0);
    }

    /* head did not start a synced page... skipped some bytes */
    if(!oy->unsynced){
      oy->unsynced=1;
      return(-1);
    }

    /* loop. keep looking */

  }
}

/* add the incoming page to the stream state; we decompose the page
   into packet segments here as well. */

int ogg_stream_pagein(ogg_stream_state *os, ogg_page *og){
  unsigned char *header=og->header;
  unsigned char *body=og->body;
  long           bodysize=og->body_len;
  int            segptr=0;

  int version=ogg_page_version(og);
  int continued=ogg_page_continued(og);
  int bos=ogg_page_bos(og);
  int eos=ogg_page_eos(og);
  ogg_int64_t granulepos=ogg_page_granulepos(og);
  int serialno=ogg_page_serialno(og);
  long pageno=ogg_page_pageno(og);
  int segments=header[26];

  /* clean up 'returned data' */
  {
    long lr=os->lacing_returned;
    long br=os->body_returned;

    /* body data */
    if(br){
      os->body_fill-=br;
      if(os->body_fill)
    memmove(os->body_data,os->body_data+br,os->body_fill);
      os->body_returned=0;
    }

    if(lr){
      /* segment table */
      if(os->lacing_fill-lr){
    memmove(os->lacing_vals,os->lacing_vals+lr,
        (os->lacing_fill-lr)*sizeof(*os->lacing_vals));
    memmove(os->granule_vals,os->granule_vals+lr,
        (os->lacing_fill-lr)*sizeof(*os->granule_vals));
      }
      os->lacing_fill-=lr;
      os->lacing_packet-=lr;
      os->lacing_returned=0;
    }
  }

  /* check the serial number */
  if(serialno!=os->serialno)return(-1);
  if(version>0)return(-1);

  _os_lacing_expand(os,segments+1);

  /* are we in sequence? */
  if(pageno!=os->pageno){
    int i;

    /* unroll previous partial packet (if any) */
    for(i=os->lacing_packet;i<os->lacing_fill;i++)
      os->body_fill-=os->lacing_vals[i]&0xff;
    os->lacing_fill=os->lacing_packet;

    /* make a note of dropped data in segment table */
    if(os->pageno!=-1){
      os->lacing_vals[os->lacing_fill++]=0x400;
      os->lacing_packet++;
    }

    /* are we a 'continued packet' page?  If so, we'll need to skip
       some segments */
    if(continued){
      bos=0;
      for(;segptr<segments;segptr++){
    int val=header[27+segptr];
    body+=val;
    bodysize-=val;
    if(val<255){
      segptr++;
      break;
    }
      }
    }
  }

  if(bodysize){
    _os_body_expand(os,bodysize);
    memcpy(os->body_data+os->body_fill,body,bodysize);
    os->body_fill+=bodysize;
  }

  {
    int saved=-1;
    while(segptr<segments){
      int val=header[27+segptr];
      os->lacing_vals[os->lacing_fill]=val;
      os->granule_vals[os->lacing_fill]=-1;

      if(bos){
    os->lacing_vals[os->lacing_fill]|=0x100;
    bos=0;
      }

      if(val<255)saved=os->lacing_fill;

      os->lacing_fill++;
      segptr++;

      if(val<255)os->lacing_packet=os->lacing_fill;
    }

    /* set the granulepos on the last granuleval of the last full packet */
    if(saved!=-1){
      os->granule_vals[saved]=granulepos;
    }

  }

  if(eos){
    os->e_o_s=1;
    if(os->lacing_fill>0)
      os->lacing_vals[os->lacing_fill-1]|=0x200;
  }

  os->pageno=pageno+1;

  return(0);
}

/* clear things to an initial state.  Good to call, eg, before seeking */
int ogg_sync_reset(ogg_sync_state *oy){
  oy->fill=0;
  oy->returned=0;
  oy->unsynced=0;
  oy->headerbytes=0;
  oy->bodybytes=0;
  return(0);
}

int ogg_stream_reset(ogg_stream_state *os){
  os->body_fill=0;
  os->body_returned=0;

  os->lacing_fill=0;
  os->lacing_packet=0;
  os->lacing_returned=0;

  os->header_fill=0;

  os->e_o_s=0;
  os->b_o_s=0;
  os->pageno=-1;
  os->packetno=0;
  os->granulepos=0;

  return(0);
}

int ogg_stream_reset_serialno(ogg_stream_state *os,int serialno){
  ogg_stream_reset(os);
  os->serialno=serialno;
  return(0);
}

int _packetout(ogg_stream_state *os,ogg_packet *op,int adv){

  /* The last part of decode. We have the stream broken into packet
     segments.  Now we need to group them into packets (or return the
     out of sync markers) */

  int ptr=os->lacing_returned;

  if(os->lacing_packet<=ptr)return(0);

  if(os->lacing_vals[ptr]&0x400){
    /* we need to tell the codec there's a gap; it might need to
       handle previous packet dependencies. */
    os->lacing_returned++;
    os->packetno++;
    return(-1);
  }

  if(!op && !adv)return(1); /* just using peek as an inexpensive way
                               to ask if there's a whole packet
                               waiting */

  /* Gather the whole packet. We'll have no holes or a partial packet */
  {
    int size=os->lacing_vals[ptr]&0xff;
    int bytes=size;
    int eos=os->lacing_vals[ptr]&0x200; /* last packet of the stream? */
    int bos=os->lacing_vals[ptr]&0x100; /* first packet of the stream? */

    while(size==255){
      int val=os->lacing_vals[++ptr];
      size=val&0xff;
      if(val&0x200)eos=0x200;
      bytes+=size;
    }

    if(op){
      op->e_o_s=eos;
      op->b_o_s=bos;
      op->packet=os->body_data+os->body_returned;
      op->packetno=os->packetno;
      op->granulepos=os->granule_vals[ptr];
      op->bytes=bytes;
    }

    if(adv){
      os->body_returned+=bytes;
      os->lacing_returned=ptr+1;
      os->packetno++;
    }
  }
  return(1);
}

int ogg_stream_packetout(ogg_stream_state *os,ogg_packet *op){
  return _packetout(os,op,1);
}

int ogg_stream_packetpeek(ogg_stream_state *os,ogg_packet *op){
  return _packetout(os,op,0);
}

void ogg_packet_clear(ogg_packet *op) {
  _ogg_free(op->packet);
  memset(op, 0, sizeof(*op));
}












// ==========================================================================
// ==========================================================================
// bitwise.c
// ==========================================================================
// ==========================================================================

// function: packing variable sized words into an octet stream.
// We're 'LSb' endian; if we write a word but read individual bits,
// then we'll read the lsb first


#define BITWISE_BUFFER_INCREMENT 256

unsigned long mask[]=
{0x00000000,0x00000001,0x00000003,0x00000007,0x0000000f,
 0x0000001f,0x0000003f,0x0000007f,0x000000ff,0x000001ff,
 0x000003ff,0x000007ff,0x00000fff,0x00001fff,0x00003fff,
 0x00007fff,0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
 0x000fffff,0x001fffff,0x003fffff,0x007fffff,0x00ffffff,
 0x01ffffff,0x03ffffff,0x07ffffff,0x0fffffff,0x1fffffff,
 0x3fffffff,0x7fffffff,0xffffffff };

unsigned int mask8B[]=
{0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe0,0xff};

void oggpack_writeinit(oggpack_buffer *b){
  memset(b,0,sizeof(*b));
  b->ptr=static_cast<unsigned char*>(_ogg_malloc(BITWISE_BUFFER_INCREMENT));
  b->buffer=b->ptr;
  b->buffer[0]='\0';
  b->storage=BITWISE_BUFFER_INCREMENT;
}

void oggpackB_writeinit(oggpack_buffer *b){
  oggpack_writeinit(b);
}

void oggpack_writetrunc(oggpack_buffer *b,long bits){
  long bytes=bits>>3;
  bits-=bytes*8;
  b->ptr=b->buffer+bytes;
  b->endbit=bits;
  b->endbyte=bytes;
  *b->ptr&=(unsigned char)(mask[bits]);
}

void oggpackB_writetrunc(oggpack_buffer *b,long bits){
  long bytes=bits>>3;
  bits-=bytes*8;
  b->ptr=b->buffer+bytes;
  b->endbit=bits;
  b->endbyte=bytes;
  *b->ptr&=(unsigned char)(mask8B[bits]);
}

/* Takes only up to 32 bits. */
void oggpack_write(oggpack_buffer *b,unsigned long value,int bits){
  if(b->endbyte+4>=b->storage){
    b->buffer=static_cast<unsigned char*>(_ogg_realloc(b->buffer,b->storage+BITWISE_BUFFER_INCREMENT));
    b->storage+=BITWISE_BUFFER_INCREMENT;
    b->ptr=b->buffer+b->endbyte;
  }

  value&=mask[bits];
  bits+=b->endbit;

  b->ptr[0]|=(unsigned char)(value<<b->endbit);

  if(bits>=8){
    b->ptr[1]=(unsigned char)(value>>(8-b->endbit));
    if(bits>=16){
      b->ptr[2]=(unsigned char)(value>>(16-b->endbit));
      if(bits>=24){
    b->ptr[3]=(unsigned char)(value>>(24-b->endbit));
    if(bits>=32){
      if(b->endbit)
        b->ptr[4]=(unsigned char)(value>>(32-b->endbit));
      else
        b->ptr[4]=0;
    }
      }
    }
  }

  b->endbyte+=bits/8;
  b->ptr+=bits/8;
  b->endbit=bits&7;
}

/* Takes only up to 32 bits. */
void oggpackB_write(oggpack_buffer *b,unsigned long value,int bits){
  if(b->endbyte+4>=b->storage){
    b->buffer=static_cast<unsigned char*>(_ogg_realloc(b->buffer,b->storage+BITWISE_BUFFER_INCREMENT));
    b->storage+=BITWISE_BUFFER_INCREMENT;
    b->ptr=b->buffer+b->endbyte;
  }

  value=(value&mask[bits])<<(32-bits);
  bits+=b->endbit;

  b->ptr[0]|=(unsigned char)(value>>(24+b->endbit));

  if(bits>=8){
    b->ptr[1]=(unsigned char)(value>>(16+b->endbit));
    if(bits>=16){
      b->ptr[2]=(unsigned char)(value>>(8+b->endbit));
      if(bits>=24){
    b->ptr[3]=(unsigned char)(value>>(b->endbit));
    if(bits>=32){
      if(b->endbit)
        b->ptr[4]=(unsigned char)(value<<(8-b->endbit));
      else
        b->ptr[4]=0;
    }
      }
    }
  }

  b->endbyte+=bits/8;
  b->ptr+=bits/8;
  b->endbit=bits&7;
}

void oggpack_writealign(oggpack_buffer *b){
  int bits=8-b->endbit;
  if(bits<8)
    oggpack_write(b,0,bits);
}

void oggpackB_writealign(oggpack_buffer *b){
  int bits=8-b->endbit;
  if(bits<8)
    oggpackB_write(b,0,bits);
}

void oggpack_writecopy_helper(oggpack_buffer *b,
                     void *source,
                     long bits,
                     void (*w)(oggpack_buffer *,
                           unsigned long,
                           int)){
  unsigned char *ptr=(unsigned char *)source;

  long bytes=bits/8;
  bits-=bytes*8;

  if(b->endbit){
    int i;
    /* unaligned copy.  Do it the hard way. */
    for(i=0;i<bytes;i++)
      w(b,(unsigned long)(ptr[i]),8);
  }else{
    /* aligned block copy */
    if(b->endbyte+bytes+1>=b->storage){
      b->storage=b->endbyte+bytes+BITWISE_BUFFER_INCREMENT;
      b->buffer=static_cast<unsigned char*>(_ogg_realloc(b->buffer,b->storage));
      b->ptr=b->buffer+b->endbyte;
    }

    memmove(b->ptr,source,bytes);
    b->ptr+=bytes;
    b->buffer+=bytes;
    *b->ptr=0;

  }
  if(bits)
    w(b,(unsigned long)(ptr[bytes]),bits);
}

void oggpack_writecopy(oggpack_buffer *b,void *source,long bits){
  oggpack_writecopy_helper(b,source,bits,oggpack_write);
}

void oggpackB_writecopy(oggpack_buffer *b,void *source,long bits){
  oggpack_writecopy_helper(b,source,bits,oggpackB_write);
}

void oggpack_reset(oggpack_buffer *b){
  b->ptr=b->buffer;
  b->buffer[0]=0;
  b->endbit=b->endbyte=0;
}

void oggpackB_reset(oggpack_buffer *b){
  oggpack_reset(b);
}

void oggpack_writeclear(oggpack_buffer *b){
  _ogg_free(b->buffer);
  memset(b,0,sizeof(*b));
}

void oggpackB_writeclear(oggpack_buffer *b){
  oggpack_writeclear(b);
}

void oggpack_readinit(oggpack_buffer *b,unsigned char *buf,int bytes){
  memset(b,0,sizeof(*b));
  b->buffer=b->ptr=buf;
  b->storage=bytes;
}

void oggpackB_readinit(oggpack_buffer *b,unsigned char *buf,int bytes){
  oggpack_readinit(b,buf,bytes);
}

/* Read in bits without advancing the bitptr; bits <= 32 */
long oggpack_look(oggpack_buffer *b,int bits){
  unsigned long ret;
  unsigned long m=mask[bits];

  bits+=b->endbit;

  if(b->endbyte+4>=b->storage){
    /* not the main path */
    if(b->endbyte*8+bits>b->storage*8)return(-1);
  }

  ret=b->ptr[0]>>b->endbit;
  if(bits>8){
    ret|=b->ptr[1]<<(8-b->endbit);
    if(bits>16){
      ret|=b->ptr[2]<<(16-b->endbit);
      if(bits>24){
    ret|=b->ptr[3]<<(24-b->endbit);
    if(bits>32 && b->endbit)
      ret|=b->ptr[4]<<(32-b->endbit);
      }
    }
  }
  return(m&ret);
}

/* Read in bits without advancing the bitptr; bits <= 32 */
long oggpackB_look(oggpack_buffer *b,int bits){
  unsigned long ret;
  int m=32-bits;

  bits+=b->endbit;

  if(b->endbyte+4>=b->storage){
    /* not the main path */
    if(b->endbyte*8+bits>b->storage*8)return(-1);
  }

  ret=b->ptr[0]<<(24+b->endbit);
  if(bits>8){
    ret|=b->ptr[1]<<(16+b->endbit);
    if(bits>16){
      ret|=b->ptr[2]<<(8+b->endbit);
      if(bits>24){
    ret|=b->ptr[3]<<(b->endbit);
    if(bits>32 && b->endbit)
      ret|=b->ptr[4]>>(8-b->endbit);
      }
    }
  }
  return(ret>>m);
}

long oggpack_look1(oggpack_buffer *b){
  if(b->endbyte>=b->storage)return(-1);
  return((b->ptr[0]>>b->endbit)&1);
}

long oggpackB_look1(oggpack_buffer *b){
  if(b->endbyte>=b->storage)return(-1);
  return((b->ptr[0]>>(7-b->endbit))&1);
}

void oggpack_adv(oggpack_buffer *b,int bits){
  bits+=b->endbit;
  b->ptr+=bits/8;
  b->endbyte+=bits/8;
  b->endbit=bits&7;
}

void oggpackB_adv(oggpack_buffer *b,int bits){
  oggpack_adv(b,bits);
}

void oggpack_adv1(oggpack_buffer *b){
  if(++(b->endbit)>7){
    b->endbit=0;
    b->ptr++;
    b->endbyte++;
  }
}

void oggpackB_adv1(oggpack_buffer *b){
  oggpack_adv1(b);
}

/* bits <= 32 */
long oggpack_read(oggpack_buffer *b,int bits){
  unsigned long ret;
  unsigned long m=mask[bits];

  bits+=b->endbit;

  if(b->endbyte+4>=b->storage){
    /* not the main path */
    ret=(unsigned long)-1;
    if(b->endbyte*8+bits>b->storage*8)goto overflow;
  }

  ret=b->ptr[0]>>b->endbit;
  if(bits>8){
    ret|=b->ptr[1]<<(8-b->endbit);
    if(bits>16){
      ret|=b->ptr[2]<<(16-b->endbit);
      if(bits>24){
    ret|=b->ptr[3]<<(24-b->endbit);
    if(bits>32 && b->endbit){
      ret|=b->ptr[4]<<(32-b->endbit);
    }
      }
    }
  }
  ret&=m;

 overflow:

  b->ptr+=bits/8;
  b->endbyte+=bits/8;
  b->endbit=bits&7;
  return(ret);
}

/* bits <= 32 */
long oggpackB_read(oggpack_buffer *b,int bits){
  unsigned long ret;
  long m=32-bits;

  bits+=b->endbit;

  if(b->endbyte+4>=b->storage){
    /* not the main path */
    ret=(unsigned long)-1;
    if(b->endbyte*8+bits>b->storage*8)goto overflow;
  }

  ret=b->ptr[0]<<(24+b->endbit);
  if(bits>8){
    ret|=b->ptr[1]<<(16+b->endbit);
    if(bits>16){
      ret|=b->ptr[2]<<(8+b->endbit);
      if(bits>24){
    ret|=b->ptr[3]<<(b->endbit);
    if(bits>32 && b->endbit)
      ret|=b->ptr[4]>>(8-b->endbit);
      }
    }
  }
  ret>>=m;

 overflow:

  b->ptr+=bits/8;
  b->endbyte+=bits/8;
  b->endbit=bits&7;
  return(ret);
}

long oggpack_read1(oggpack_buffer *b){
  unsigned long ret;

  if(b->endbyte>=b->storage){
    /* not the main path */
    ret=(unsigned long)-1;
    goto overflow;
  }

  ret=(b->ptr[0]>>b->endbit)&1;

 overflow:

  b->endbit++;
  if(b->endbit>7){
    b->endbit=0;
    b->ptr++;
    b->endbyte++;
  }
  return(ret);
}

long oggpackB_read1(oggpack_buffer *b){
  unsigned long ret;

  if(b->endbyte>=b->storage){
    /* not the main path */
    ret=(unsigned long)-1;
    goto overflow;
  }

  ret=(b->ptr[0]>>(7-b->endbit))&1;

 overflow:

  b->endbit++;
  if(b->endbit>7){
    b->endbit=0;
    b->ptr++;
    b->endbyte++;
  }
  return(ret);
}

long oggpack_bytes(oggpack_buffer *b){
  return(b->endbyte+(b->endbit+7)/8);
}

long oggpack_bits(oggpack_buffer *b){
  return(b->endbyte*8+b->endbit);
}

long oggpackB_bytes(oggpack_buffer *b){
  return oggpack_bytes(b);
}

long oggpackB_bits(oggpack_buffer *b){
  return oggpack_bits(b);
}

unsigned char *oggpack_get_buffer(oggpack_buffer *b){
  return(b->buffer);
}

unsigned char *oggpackB_get_buffer(oggpack_buffer *b){
  return oggpack_get_buffer(b);
}










// ==========================================================================
// ==========================================================================
// window.c
// ==========================================================================
// ==========================================================================


float *_vorbis_window(int type, int left){
  float *ret=static_cast<float*>(_ogg_calloc(left,sizeof(*ret)));
  int i;

  switch(type){
  case 0:
    /* The 'vorbis window' (window 0) is sin(sin(x)*sin(x)*pi*.5) */
    {

      for(i=0;i<left;i++){
    float x=(float)((i+.5f)/left*M_PI/2.);
    x=(float)sin(x);
    x*=x;
    x*=M_PI/2.f;
    x=(float)sin(x);
    ret[i]=x;
      }
    }
    break;
  default:
    _ogg_free(ret);
    return(NULL);
  }
  return(ret);
}

void _vorbis_apply_window(float *d,float *window[2],long *blocksizes,
              int lW,int W,int nW){
  lW=(W?lW:0);
  nW=(W?nW:0);

  {
    long n=blocksizes[W];
    long ln=blocksizes[lW];
    long rn=blocksizes[nW];

    long leftbegin=n/4-ln/4;
    long leftend=leftbegin+ln/2;

    long rightbegin=n/2+n/4-rn/4;
    long rightend=rightbegin+rn/2;

    int i,p;

    for(i=0;i<leftbegin;i++)
      d[i]=0.f;

    for(p=0;i<leftend;i++,p++)
      d[i]*=window[lW][p];

    for(i=rightbegin,p=rn/2-1;i<rightend;i++,p--)
      d[i]*=window[nW][p];

    for(;i<n;i++)
      d[i]=0.f;
  }
}










// ==========================================================================
// ==========================================================================
// vorbisfile.c
// ==========================================================================
// ==========================================================================
// function: stdio-based convenience library for opening/seeking/decoding

/* A 'chained bitstream' is a Vorbis bitstream that contains more than
   one logical bitstream arranged end to end (the only form of Ogg
   multiplexing allowed in a Vorbis bitstream; grouping [parallel
   multiplexing] is not allowed in Vorbis) */

/* A Vorbis file can be played beginning to end (streamed) without
   worrying ahead of time about chaining (see decoder_example.c).  If
   we have the whole file, however, and want random access
   (seeking/scrubbing) or desire to know the total length/time of a
   file, we need to account for the possibility of chaining. */

/* We can handle things a number of ways; we can determine the entire
   bitstream structure right off the bat, or find pieces on demand.
   This example determines and caches structure for the entire
   bitstream, but builds a virtual decoder on the fly when moving
   between links in the chain. */

/* There are also different ways to implement seeking.  Enough
   information exists in an Ogg bitstream to seek to
   sample-granularity positions in the output.  Or, one can seek by
   picking some portion of the stream roughly in the desired area if
   we only want coarse navigation through the stream. */

/*************************************************************************
 * Many, many internal helpers.  The intention is not to be confusing;
 * rampant duplication and monolithic function implementation would be
 * harder to understand anyway.  The high level functions are last.  Begin
 * grokking near the end of the file */

/* read a little more data from the file/pipe into the ogg_sync framer
*/
#define CHUNKSIZE 8500 /* a shade over 8k; anyone using pages well
                          over 8k gets what they deserve */
long _get_data(OggVorbis_File *vf){
  errno=0;
  if(vf->datasource){
    char *buffer=ogg_sync_buffer(&vf->oy,CHUNKSIZE);
    long bytes=(vf->callbacks.read_func)(buffer,1,CHUNKSIZE,vf->datasource);
    if(bytes>0)ogg_sync_wrote(&vf->oy,bytes);
    if(bytes==0 && errno)return(-1);
    return(bytes);
  }else
    return(0);
}

/* save a tiny smidge of verbosity to make the code more readable */
void _seek_helper(OggVorbis_File *vf,ogg_int64_t offset){
  if(vf->datasource){
    (vf->callbacks.seek_func)(vf->datasource, offset, SEEK_SET);
    vf->offset=offset;
    ogg_sync_reset(&vf->oy);
  }else{
    /* shouldn't happen unless someone writes a broken callback */
    return;
  }
}

/* The read/seek functions track absolute position within the stream */

/* from the head of the stream, get the next page.  boundary specifies
   if the function is allowed to fetch more data from the stream (and
   how much) or only use internally buffered data.

   boundary: -1) unbounded search
              0) read no additional data; use cached only
          n) search for a new page beginning for n bytes

   return:   <0) did not find a page (OV_FALSE, OV_EOF, OV_EREAD)
              n) found a page at absolute offset n */

ogg_int64_t _get_next_page(OggVorbis_File *vf,ogg_page *og,
                  ogg_int64_t boundary){
  if(boundary>0)boundary+=vf->offset;
  for(;;){
    long more;

    if(boundary>0 && vf->offset>=boundary)return(OV_FALSE);
    more=ogg_sync_pageseek(&vf->oy,og);

    if(more<0){
      /* skipped n bytes */
      vf->offset-=more;
    }else{
      if(more==0){
    /* send more paramedics */
    if(!boundary)return(OV_FALSE);
    {
      long ret=_get_data(vf);
      if(ret==0)return(OV_EOF);
      if(ret<0)return(OV_EREAD);
    }
      }else{
    /* got a page.  Return the offset at the page beginning,
           advance the internal offset past the page end */
    ogg_int64_t ret=vf->offset;
    vf->offset+=more;
    return(ret);

      }
    }
  }
}

/* find the latest page beginning before the current stream cursor
   position. Much dirtier than the above as Ogg doesn't have any
   backward search linkage.  no 'readp' as it will certainly have to
   read. */
/* returns offset or OV_EREAD, OV_FAULT */
ogg_int64_t _get_prev_page(OggVorbis_File *vf,ogg_page *og){
  ogg_int64_t begin=vf->offset;
  ogg_int64_t end=begin;
  ogg_int64_t ret;
  ogg_int64_t offset=-1;

  while(offset==-1){
    begin-=CHUNKSIZE;
    if(begin<0)
      begin=0;
    _seek_helper(vf,begin);
    while(vf->offset<end){
      ret=_get_next_page(vf,og,end-vf->offset);
      if(ret==OV_EREAD)return(OV_EREAD);
      if(ret<0){
    break;
      }else{
    offset=ret;
      }
    }
  }

  /* we have the offset.  Actually snork and hold the page now */
  _seek_helper(vf,offset);
  ret=_get_next_page(vf,og,CHUNKSIZE);
  if(ret<0)
    /* this shouldn't be possible */
    return(OV_EFAULT);

  return(offset);
}

/* finds each bitstream link one at a time using a bisection search
   (has to begin by knowing the offset of the lb's initial page).
   Recurses for each link so it can alloc the link storage after
   finding them all, then unroll and fill the cache at the same time */
int _bisect_forward_serialno(OggVorbis_File *vf,
                    ogg_int64_t begin,
                    ogg_int64_t searched,
                    ogg_int64_t end,
                    long currentno,
                    long m){
  ogg_int64_t endsearched=end;
  ogg_int64_t next=end;
  ogg_page og;
  ogg_int64_t ret;

  /* the below guards against garbage seperating the last and
     first pages of two links. */
  while(searched<endsearched){
    ogg_int64_t bisect;

    if(endsearched-searched<CHUNKSIZE){
      bisect=searched;
    }else{
      bisect=(searched+endsearched)/2;
    }

    _seek_helper(vf,bisect);
    ret=_get_next_page(vf,&og,-1);
    if(ret==OV_EREAD)return(OV_EREAD);
    if(ret<0 || ogg_page_serialno(&og)!=currentno){
      endsearched=bisect;
      if(ret>=0)next=ret;
    }else{
      searched=ret+og.header_len+og.body_len;
    }
  }

  _seek_helper(vf,next);
  ret=_get_next_page(vf,&og,-1);
  if(ret==OV_EREAD)return(OV_EREAD);

  if(searched>=end || ret<0){
    vf->links=m+1;
    vf->offsets=static_cast<ogg_int64_t*>(_ogg_malloc((vf->links+1)*sizeof(*vf->offsets)));
    vf->serialnos=static_cast<long*>(_ogg_malloc(vf->links*sizeof(*vf->serialnos)));
    vf->offsets[m+1]=searched;
  }else{
    ret=_bisect_forward_serialno(vf,next,vf->offset,
                 end,ogg_page_serialno(&og),m+1);
    if(ret==OV_EREAD)return(OV_EREAD);
  }

  vf->offsets[m]=begin;
  vf->serialnos[m]=currentno;
  return(0);
}

/* uses the local ogg_stream storage in vf; this is important for
   non-streaming input sources */
int _fetch_headers(OggVorbis_File *vf,vorbis_info *vi,vorbis_comment *vc,
              long *serialno,ogg_page *og_ptr){
  ogg_page og;
  ogg_packet op;
  int i,ret;

  if(!og_ptr){
    ogg_int64_t llret=_get_next_page(vf,&og,CHUNKSIZE);
    if(llret==OV_EREAD)return(OV_EREAD);
    if(llret<0)return OV_ENOTVORBIS;
    og_ptr=&og;
  }

  ogg_stream_reset_serialno(&vf->os,ogg_page_serialno(og_ptr));
  if(serialno)*serialno=vf->os.serialno;
  vf->ready_state=STREAMSET;

  /* extract the initial header from the first page and verify that the
     Ogg bitstream is in fact Vorbis data */

  vorbis_info_init(vi);
  vorbis_comment_init(vc);

  i=0;
  while(i<3){
    ogg_stream_pagein(&vf->os,og_ptr);
    while(i<3){
      int result=ogg_stream_packetout(&vf->os,&op);
      if(result==0)break;
      if(result==-1){
    ret=OV_EBADHEADER;
    goto bail_header;
      }
      ret=vorbis_synthesis_headerin(vi,vc,&op);
      if (ret) goto bail_header;
      i++;
    }
    if(i<3)
      if(_get_next_page(vf,og_ptr,CHUNKSIZE)<0){
    ret=OV_EBADHEADER;
    goto bail_header;
      }
  }
  return 0;

 bail_header:
  vorbis_info_clear(vi);
  vorbis_comment_clear(vc);
  vf->ready_state=OPENED;

  return ret;
}

/* last step of the OggVorbis_File initialization; get all the
   vorbis_info structs and PCM positions.  Only called by the seekable
   initialization (local stream storage is hacked slightly; pay
   attention to how that's done) */

/* this is void and does not propogate errors up because we want to be
   able to open and use damaged bitstreams as well as we can.  Just
   watch out for missing information for links in the OggVorbis_File
   struct */
void _prefetch_all_headers(OggVorbis_File *vf, ogg_int64_t dataoffset){
  ogg_page og;
  int i;
  ogg_int64_t ret;

  vf->vi=static_cast<vorbis_info*>(_ogg_realloc(vf->vi,vf->links*sizeof(*vf->vi)));
  vf->vc=static_cast<vorbis_comment*>(_ogg_realloc(vf->vc,vf->links*sizeof(*vf->vc)));
  vf->dataoffsets=static_cast<ogg_int64_t*>(_ogg_malloc(vf->links*sizeof(*vf->dataoffsets)));
  vf->pcmlengths=static_cast<ogg_int64_t*>(_ogg_malloc(vf->links*2*sizeof(*vf->pcmlengths)));

  for(i=0;i<vf->links;i++){
    if(i==0){
      /* we already grabbed the initial header earlier.  Just set the offset */
      vf->dataoffsets[i]=dataoffset;
      _seek_helper(vf,dataoffset);

    }else{

      /* seek to the location of the initial header */

      _seek_helper(vf,vf->offsets[i]);
      if(_fetch_headers(vf,vf->vi+i,vf->vc+i,NULL,NULL)<0){
        vf->dataoffsets[i]=-1;
      }else{
    vf->dataoffsets[i]=vf->offset;
      }
    }

    /* fetch beginning PCM offset */

    if(vf->dataoffsets[i]!=-1){
      ogg_int64_t accumulated=0;
      long        lastblock=-1;
      int         result;

      ogg_stream_reset_serialno(&vf->os,vf->serialnos[i]);

      for(;;){
    ogg_packet op;

    ret=_get_next_page(vf,&og,-1);
    if(ret<0)
      /* this should not be possible unless the file is
             truncated/mangled */
      break;

    if(ogg_page_serialno(&og)!=vf->serialnos[i])
      break;

    /* count blocksizes of all frames in the page */
    ogg_stream_pagein(&vf->os,&og);
        for(;;)
        { result = ogg_stream_packetout(&vf->os,&op);
          if (!result) break;
      if(result>0){ /* ignore holes */
        long thisblock=vorbis_packet_blocksize(vf->vi+i,&op);
        if(lastblock!=-1)
          accumulated+=(lastblock+thisblock)>>2;
        lastblock=thisblock;
      }
    }

    if(ogg_page_granulepos(&og)!=-1){
      /* pcm offset of last packet on the first audio page */
      accumulated= ogg_page_granulepos(&og)-accumulated;
      break;
    }
      }

      /* less than zero?  This is a stream with samples trimmed off
         the beginning, a normal occurrence; set the offset to zero */
      if(accumulated<0)accumulated=0;

      vf->pcmlengths[i*2]=accumulated;
    }

    /* get the PCM length of this link. To do this,
       get the last page of the stream */
    {
      ogg_int64_t end=vf->offsets[i+1];
      _seek_helper(vf,end);

      for(;;){
    ret=_get_prev_page(vf,&og);
    if(ret<0){
      /* this should not be possible */
      vorbis_info_clear(vf->vi+i);
      vorbis_comment_clear(vf->vc+i);
      break;
    }
    if(ogg_page_granulepos(&og)!=-1){
      vf->pcmlengths[i*2+1]=ogg_page_granulepos(&og)-vf->pcmlengths[i*2];
      break;
    }
    vf->offset=ret;
      }
    }
  }
}

void _make_decode_ready(OggVorbis_File *vf){
  if(vf->ready_state!=STREAMSET)return;
  if(vf->seekable){
    vorbis_synthesis_init(&vf->vd,vf->vi+vf->current_link);
  }else{
    vorbis_synthesis_init(&vf->vd,vf->vi);
  }
  vorbis_block_init(&vf->vd,&vf->vb);
  vf->ready_state=INITSET;
  return;
}

int _open_seekable2(OggVorbis_File *vf){
  long serialno=vf->current_serialno;
  ogg_int64_t dataoffset=vf->offset, end;
  ogg_page og;

  /* we're partially open and have a first link header state in
     storage in vf */
  /* we can seek, so set out learning all about this file */
  (vf->callbacks.seek_func)(vf->datasource,0,SEEK_END);
  vf->offset=vf->end=(vf->callbacks.tell_func)(vf->datasource);

  /* We get the offset for the last page of the physical bitstream.
     Most OggVorbis files will contain a single logical bitstream */
  end=_get_prev_page(vf,&og);
  if(end<0)return((int)end);

  /* more than one logical bitstream? */
  if(ogg_page_serialno(&og)!=serialno){

    /* Chained bitstream. Bisect-search each logical bitstream
       section.  Do so based on serial number only */
    if(_bisect_forward_serialno(vf,0,0,end+1,serialno,0)<0)return(OV_EREAD);

  }else{

    /* Only one logical bitstream */
    if(_bisect_forward_serialno(vf,0,end,end+1,serialno,0))return(OV_EREAD);

  }

  /* the initial header memory is referenced by vf after; don't free it */
  _prefetch_all_headers(vf,dataoffset);
  return(ov_raw_seek(vf,0));
}

/* clear out the current logical bitstream decoder */
void _decode_clear(OggVorbis_File *vf){
  vorbis_dsp_clear(&vf->vd);
  vorbis_block_clear(&vf->vb);
  vf->ready_state=OPENED;

  vf->bittrack=0.f;
  vf->samptrack=0.f;
}

/* fetch and process a packet.  Handles the case where we're at a
   bitstream boundary and dumps the decoding machine.  If the decoding
   machine is unloaded, it loads it.  It also keeps pcm_offset up to
   date (seek and read both use this.  seek uses a special hack with
   readp).

   return: <0) error, OV_HOLE (lost packet) or OV_EOF
            0) need more data (only if readp==0)
        1) got a packet
*/

int _fetch_and_process_packet(OggVorbis_File *vf,
                     ogg_packet *op_in,
                     int readp){
  ogg_page og;

  /* handle one packet.  Try to fetch it from current stream state */
  /* extract packets from page */
  for(;;){

    /* process a packet if we can.  If the machine isn't loaded,
       neither is a page */
    if(vf->ready_state==INITSET){
      for(;;) {
        ogg_packet op;
        ogg_packet *op_ptr=(op_in?op_in:&op);
    int result=ogg_stream_packetout(&vf->os,op_ptr);
    ogg_int64_t granulepos;

    op_in=NULL;
    if(result==-1)return(OV_HOLE); /* hole in the data. */
    if(result>0){
      /* got a packet.  process it */
      granulepos=op_ptr->granulepos;
      if(!vorbis_synthesis(&vf->vb,op_ptr)){ /* lazy check for lazy
                            header handling.  The
                            header packets aren't
                            audio, so if/when we
                            submit them,
                            vorbis_synthesis will
                            reject them */

        /* suck in the synthesis data and track bitrate */
        {
          int oldsamples=vorbis_synthesis_pcmout(&vf->vd,NULL);
          /* for proper use of libvorbis within libvorbisfile,
                 oldsamples will always be zero. */
          if(oldsamples)return(OV_EFAULT);

          vorbis_synthesis_blockin(&vf->vd,&vf->vb);
          vf->samptrack+=vorbis_synthesis_pcmout(&vf->vd,NULL)-oldsamples;
          vf->bittrack+=op_ptr->bytes*8;
        }

        /* update the pcm offset. */
        if(granulepos!=-1 && !op_ptr->e_o_s){
          int link=(vf->seekable?vf->current_link:0);
          int i,samples;

          /* this packet has a pcm_offset on it (the last packet
             completed on a page carries the offset) After processing
             (above), we know the pcm position of the *last* sample
             ready to be returned. Find the offset of the *first*

             As an aside, this trick is inaccurate if we begin
             reading anew right at the last page; the end-of-stream
             granulepos declares the last frame in the stream, and the
             last packet of the last page may be a partial frame.
             So, we need a previous granulepos from an in-sequence page
             to have a reference point.  Thus the !op_ptr->e_o_s clause
             above */

          if(vf->seekable && link>0)
        granulepos-=vf->pcmlengths[link*2];
          if(granulepos<0)granulepos=0; /* actually, this
                           shouldn't be possible
                           here unless the stream
                           is very broken */

          samples=vorbis_synthesis_pcmout(&vf->vd,NULL);

          granulepos-=samples;
          for(i=0;i<link;i++)
            granulepos+=vf->pcmlengths[i*2+1];
          vf->pcm_offset=granulepos;
        }
        return(1);
      }
    }
    else
      break;
      }
    }

    if(vf->ready_state>=OPENED){
      if(!readp)return(0);
      if(_get_next_page(vf,&og,-1)<0)return(OV_EOF); /* eof.
                            leave unitialized */
      /* bitrate tracking; add the header's bytes here, the body bytes
     are done by packet above */
      vf->bittrack+=og.header_len*8;

      /* has our decoding just traversed a bitstream boundary? */
      if(vf->ready_state==INITSET){
    if(vf->current_serialno!=ogg_page_serialno(&og)){
      _decode_clear(vf);

      if(!vf->seekable){
        vorbis_info_clear(vf->vi);
        vorbis_comment_clear(vf->vc);
      }
    }
      }
    }

    /* Do we need to load a new machine before submitting the page? */
    /* This is different in the seekable and non-seekable cases.

       In the seekable case, we already have all the header
       information loaded and cached; we just initialize the machine
       with it and continue on our merry way.

       In the non-seekable (streaming) case, we'll only be at a
       boundary if we just left the previous logical bitstream and
       we're now nominally at the header of the next bitstream
    */

    if(vf->ready_state!=INITSET){
      int link;

      if(vf->ready_state<STREAMSET){
    if(vf->seekable){
      vf->current_serialno=ogg_page_serialno(&og);

      /* match the serialno to bitstream section.  We use this rather than
         offset positions to avoid problems near logical bitstream
         boundaries */
      for(link=0;link<vf->links;link++)
        if(vf->serialnos[link]==vf->current_serialno)break;
      if(link==vf->links)return(OV_EBADLINK); /* sign of a bogus
                             stream.  error out,
                             leave machine
                             uninitialized */

      vf->current_link=link;

      ogg_stream_reset_serialno(&vf->os,vf->current_serialno);
      vf->ready_state=STREAMSET;

    }else{
      /* we're streaming */
      /* fetch the three header packets, build the info struct */

      int ret=_fetch_headers(vf,vf->vi,vf->vc,&vf->current_serialno,&og);
      if(ret)return(ret);
      vf->current_link++;
      link=0; link; // unused?
    }
      }

      _make_decode_ready(vf);
    }
    ogg_stream_pagein(&vf->os,&og);
  }
}

/* if, eg, 64 bit stdio is configured by default, this will build with
   fseek64 */
int _fseek64_wrap(FILE *f,ogg_int64_t off,int whence){
  if(f==NULL)return(-1);
  return fseek(f,(long)off,whence);
}

int _ov_open1(void *f,OggVorbis_File *vf,char *initial,
             long ibytes, ov_callbacks callbacks){
  int offsettest=(f?callbacks.seek_func(f,0,SEEK_CUR):-1);
  int ret;

  memset(vf,0,sizeof(*vf));
  vf->datasource=f;
  vf->callbacks = callbacks;

  /* init the framing state */
  ogg_sync_init(&vf->oy);

  /* perhaps some data was previously read into a buffer for testing
     against other stream types.  Allow initialization from this
     previously read data (as we may be reading from a non-seekable
     stream) */
  if(initial){
    char *buffer=ogg_sync_buffer(&vf->oy,ibytes);
    memcpy(buffer,initial,ibytes);
    ogg_sync_wrote(&vf->oy,ibytes);
  }

  /* can we seek? Stevens suggests the seek test was portable */
  if(offsettest!=-1)vf->seekable=1;

  /* No seeking yet; Set up a 'single' (current) logical bitstream
     entry for partial open */
  vf->links=1;
  vf->vi=static_cast<vorbis_info*>(_ogg_calloc(vf->links,sizeof(*vf->vi)));
  vf->vc=static_cast<vorbis_comment*>(_ogg_calloc(vf->links,sizeof(*vf->vc)));
  ogg_stream_init(&vf->os,-1); /* fill in the serialno later */

  /* Try to fetch the headers, maintaining all the storage */
  if((ret=_fetch_headers(vf,vf->vi,vf->vc,&vf->current_serialno,NULL))<0){
    vf->datasource=NULL;
    ov_clear(vf);
  }else if(vf->ready_state < PARTOPEN)
    vf->ready_state=PARTOPEN;
  return(ret);
}

int _ov_open2(OggVorbis_File *vf){
  if(vf->ready_state < OPENED)
    vf->ready_state=OPENED;
  if(vf->seekable){
    int ret=_open_seekable2(vf);
    if(ret){
      vf->datasource=NULL;
      ov_clear(vf);
    }
    return(ret);
  }
  return 0;
}


/* clear out the OggVorbis_File struct */
int ov_clear(OggVorbis_File *vf){
  if(vf){
    vorbis_block_clear(&vf->vb);
    vorbis_dsp_clear(&vf->vd);
    ogg_stream_clear(&vf->os);

    if(vf->vi && vf->links){
      int i;
      for(i=0;i<vf->links;i++){
    vorbis_info_clear(vf->vi+i);
    vorbis_comment_clear(vf->vc+i);
      }
      _ogg_free(vf->vi);
      _ogg_free(vf->vc);
    }
    if(vf->dataoffsets)_ogg_free(vf->dataoffsets);
    if(vf->pcmlengths)_ogg_free(vf->pcmlengths);
    if(vf->serialnos)_ogg_free(vf->serialnos);
    if(vf->offsets)_ogg_free(vf->offsets);
    ogg_sync_clear(&vf->oy);
    if(vf->datasource)(vf->callbacks.close_func)(vf->datasource);
    memset(vf,0,sizeof(*vf));
  }
#ifdef DEBUG_LEAKS
  _VDBG_dump();
#endif
  return(0);
}

/* inspects the OggVorbis file and finds/documents all the logical
   bitstreams contained in it.  Tries to be tolerant of logical
   bitstream sections that are truncated/woogie.

   return: -1) error
            0) OK
*/

int ov_open_callbacks(void *f,OggVorbis_File *vf,char *initial,long ibytes,
    ov_callbacks callbacks){
  int ret=_ov_open1(f,vf,initial,ibytes,callbacks);
  if(ret)return ret;
  return _ov_open2(vf);
}

int ov_open(FILE *f,OggVorbis_File *vf,char *initial,long ibytes){
  ov_callbacks callbacks = {
    (size_t (*)(void *, size_t, size_t, void *))  fread,
    (int (*)(void *, ogg_int64_t, int))              _fseek64_wrap,
    (int (*)(void *))                             fclose,
    (long (*)(void *))                            ftell
  };

  return ov_open_callbacks((void *)f, vf, initial, ibytes, callbacks);
}

/* Only partially open the vorbis file; test for Vorbisness, and load
   the headers for the first chain.  Do not seek (although test for
   seekability).  Use ov_test_open to finish opening the file, else
   ov_clear to close/free it. Same return codes as open. */

int ov_test_callbacks(void *f,OggVorbis_File *vf,char *initial,long ibytes,
    ov_callbacks callbacks)
{
  return _ov_open1(f,vf,initial,ibytes,callbacks);
}

int ov_test(FILE *f,OggVorbis_File *vf,char *initial,long ibytes){
  ov_callbacks callbacks = {
    (size_t (*)(void *, size_t, size_t, void *))  fread,
    (int (*)(void *, ogg_int64_t, int))              _fseek64_wrap,
    (int (*)(void *))                             fclose,
    (long (*)(void *))                            ftell
  };

  return ov_test_callbacks((void *)f, vf, initial, ibytes, callbacks);
}

int ov_test_open(OggVorbis_File *vf){
  if(vf->ready_state!=PARTOPEN)return(OV_EINVAL);
  return _ov_open2(vf);
}

/* How many logical bitstreams in this physical bitstream? */
long ov_streams(OggVorbis_File *vf){
  return vf->links;
}

/* Is the FILE * associated with vf seekable? */
long ov_seekable(OggVorbis_File *vf){
  return vf->seekable;
}

/* returns the bitrate for a given logical bitstream or the entire
   physical bitstream.  If the file is open for random access, it will
   find the *actual* average bitrate.  If the file is streaming, it
   returns the nominal bitrate (if set) else the average of the
   upper/lower bounds (if set) else -1 (unset).

   If you want the actual bitrate field settings, get them from the
   vorbis_info structs */

long ov_bitrate(OggVorbis_File *vf,int i){
  if(vf->ready_state<OPENED)return(OV_EINVAL);
  if(i>=vf->links)return(OV_EINVAL);
  if(!vf->seekable && i!=0)return(ov_bitrate(vf,0));
  if(i<0){
    ogg_int64_t bits=0;
    int i;
    float br;
    for(i=0;i<vf->links;i++)
      bits+=(vf->offsets[i+1]-vf->dataoffsets[i])*8;
    /* This once read: return(rint(bits/ov_time_total(vf,-1)));
     * gcc 3.x on x86 miscompiled this at optimisation level 2 and above,
     * so this is slightly transformed to make it work.
     */
    br = (float)(bits/ov_time_total(vf,-1));
    return (long)(rint(br));
  }else{
    if(vf->seekable){
      /* return the actual bitrate */
      return (long)(rint((vf->offsets[i+1]-vf->dataoffsets[i])*8/ov_time_total(vf,i)));
    }else{
      /* return nominal if set */
      if(vf->vi[i].bitrate_nominal>0){
    return vf->vi[i].bitrate_nominal;
      }else{
    if(vf->vi[i].bitrate_upper>0){
      if(vf->vi[i].bitrate_lower>0){
        return (vf->vi[i].bitrate_upper+vf->vi[i].bitrate_lower)/2;
      }else{
        return vf->vi[i].bitrate_upper;
      }
    }
    return(OV_FALSE);
      }
    }
  }
}

/* returns the actual bitrate since last call.  returns -1 if no
   additional data to offer since last call (or at beginning of stream),
   EINVAL if stream is only partially open
*/
long ov_bitrate_instant(OggVorbis_File *vf){
  int link=(vf->seekable?vf->current_link:0);
  long ret;
  if(vf->ready_state<OPENED)return(OV_EINVAL);
  if(vf->samptrack==0)return(OV_FALSE);
  ret=(long)(vf->bittrack/vf->samptrack*vf->vi[link].rate+.5);
  vf->bittrack=0.f;
  vf->samptrack=0.f;
  return(ret);
}

/* Guess */
long ov_serialnumber(OggVorbis_File *vf,int i){
  if(i>=vf->links)return(ov_serialnumber(vf,vf->links-1));
  if(!vf->seekable && i>=0)return(ov_serialnumber(vf,-1));
  if(i<0){
    return(vf->current_serialno);
  }else{
    return(vf->serialnos[i]);
  }
}

/* returns: total raw (compressed) length of content if i==-1
            raw (compressed) length of that logical bitstream for i==0 to n
        OV_EINVAL if the stream is not seekable (we can't know the length)
        or if stream is only partially open
*/
ogg_int64_t ov_raw_total(OggVorbis_File *vf,int i){
  if(vf->ready_state<OPENED)return(OV_EINVAL);
  if(!vf->seekable || i>=vf->links)return(OV_EINVAL);
  if(i<0){
    ogg_int64_t acc=0;
    int i;
    for(i=0;i<vf->links;i++)
      acc+=ov_raw_total(vf,i);
    return(acc);
  }else{
    return(vf->offsets[i+1]-vf->offsets[i]);
  }
}

/* returns: total PCM length (samples) of content if i==-1 PCM length
        (samples) of that logical bitstream for i==0 to n
        OV_EINVAL if the stream is not seekable (we can't know the
        length) or only partially open
*/
ogg_int64_t ov_pcm_total(OggVorbis_File *vf,int i){
  if(vf->ready_state<OPENED)return(OV_EINVAL);
  if(!vf->seekable || i>=vf->links)return(OV_EINVAL);
  if(i<0){
    ogg_int64_t acc=0;
    int i;
    for(i=0;i<vf->links;i++)
      acc+=ov_pcm_total(vf,i);
    return(acc);
  }else{
    return(vf->pcmlengths[i*2+1]);
  }
}

/* returns: total seconds of content if i==-1
            seconds in that logical bitstream for i==0 to n
        OV_EINVAL if the stream is not seekable (we can't know the
        length) or only partially open
*/
double ov_time_total(OggVorbis_File *vf,int i){
  if(vf->ready_state<OPENED)return(OV_EINVAL);
  if(!vf->seekable || i>=vf->links)return(OV_EINVAL);
  if(i<0){
    double acc=0;
    int i;
    for(i=0;i<vf->links;i++)
      acc+=ov_time_total(vf,i);
    return(acc);
  }else{
    return((double)(vf->pcmlengths[i*2+1])/vf->vi[i].rate);
  }
}

/* seek to an offset relative to the *compressed* data. This also
   scans packets to update the PCM cursor. It will cross a logical
   bitstream boundary, but only if it can't get any packets out of the
   tail of the bitstream we seek to (so no surprises).

   returns zero on success, nonzero on failure */

int ov_raw_seek(OggVorbis_File *vf,ogg_int64_t pos){
  ogg_stream_state work_os;

  if(vf->ready_state<OPENED)return(OV_EINVAL);
  if(!vf->seekable)
    return(OV_ENOSEEK); /* don't dump machine if we can't seek */

  if(pos<0 || pos>vf->end)return(OV_EINVAL);

  /* clear out decoding machine state */
  vf->pcm_offset=-1;
  _decode_clear(vf);

  _seek_helper(vf,pos);

  /* we need to make sure the pcm_offset is set, but we don't want to
     advance the raw cursor past good packets just to get to the first
     with a granulepos.  That's not equivalent behavior to beginning
     decoding as immediately after the seek position as possible.

     So, a hack.  We use two stream states; a local scratch state and
     a the shared vf->os stream state.  We use the local state to
     scan, and the shared state as a buffer for later decode.

     Unfortuantely, on the last page we still advance to last packet
     because the granulepos on the last page is not necessarily on a
     packet boundary, and we need to make sure the granpos is
     correct.
  */

  {
    ogg_page og;
    ogg_packet op;
    int lastblock=0;
    int accblock=0;
    int thisblock=0;
    int eosflag=0;

    ogg_stream_init(&work_os,-1); /* get the memory ready */

    for(;;){
      if(vf->ready_state==STREAMSET){
    /* snarf/scan a packet if we can */
    int result=ogg_stream_packetout(&work_os,&op);

    if(result>0){

      if(vf->vi[vf->current_link].codec_setup)
        thisblock=vorbis_packet_blocksize(vf->vi+vf->current_link,&op);
      if(eosflag)
        ogg_stream_packetout(&vf->os,NULL);
      else
        if(lastblock)accblock+=(lastblock+thisblock)>>2;

      if(op.granulepos!=-1){
        int i,link=vf->current_link;
        ogg_int64_t granulepos=op.granulepos-vf->pcmlengths[link*2];
        if(granulepos<0)granulepos=0;

        for(i=0;i<link;i++)
          granulepos+=vf->pcmlengths[i*2+1];
        vf->pcm_offset=granulepos-accblock;
        break;
      }
      lastblock=thisblock;
      continue;
    }
      }

      if(!lastblock){
    if(_get_next_page(vf,&og,-1)<0){
      vf->pcm_offset=ov_pcm_total(vf,-1);
      break;
    }
      }else{
    /* huh?  Bogus stream with packets but no granulepos */
    vf->pcm_offset=-1;
    break;
      }

      /* has our decoding just traversed a bitstream boundary? */
      if(vf->ready_state==STREAMSET)
    if(vf->current_serialno!=ogg_page_serialno(&og)){
    _decode_clear(vf); /* clear out stream state */
    ogg_stream_clear(&work_os);
      }

      if(vf->ready_state<STREAMSET){
    int link;

    vf->current_serialno=ogg_page_serialno(&og);
    for(link=0;link<vf->links;link++)
      if(vf->serialnos[link]==vf->current_serialno)break;
    if(link==vf->links)goto seek_error; /* sign of a bogus stream.
                           error out, leave
                           machine uninitialized */
    vf->current_link=link;

    ogg_stream_reset_serialno(&vf->os,vf->current_serialno);
    ogg_stream_reset_serialno(&work_os,vf->current_serialno);
    vf->ready_state=STREAMSET;

      }

      ogg_stream_pagein(&vf->os,&og);
      ogg_stream_pagein(&work_os,&og);
      eosflag=ogg_page_eos(&og);
    }
  }

  ogg_stream_clear(&work_os);
  return(0);

 seek_error:
  /* dump the machine so we're in a known state */
  vf->pcm_offset=-1;
  ogg_stream_clear(&work_os);
  _decode_clear(vf);
  return OV_EBADLINK;
}

/* Page granularity seek (faster than sample granularity because we
   don't do the last bit of decode to find a specific sample).

   Seek to the last [granule marked] page preceeding the specified pos
   location, such that decoding past the returned point will quickly
   arrive at the requested position. */
int ov_pcm_seek_page(OggVorbis_File *vf,ogg_int64_t pos){
  int link=-1; link; // unused?
  ogg_int64_t result=0; result; // unused?
  ogg_int64_t total=ov_pcm_total(vf,-1);

  if(vf->ready_state<OPENED)return(OV_EINVAL);
  if(!vf->seekable)return(OV_ENOSEEK);

  if(pos<0 || pos>total)return(OV_EINVAL);

  /* which bitstream section does this pcm offset occur in? */
  for(link=vf->links-1;link>=0;link--){
    total-=vf->pcmlengths[link*2+1];
    if(pos>=total)break;
  }

  /* search within the logical bitstream for the page with the highest
     pcm_pos preceeding (or equal to) pos.  There is a danger here;
     missing pages or incorrect frame number information in the
     bitstream could make our task impossible.  Account for that (it
     would be an error condition) */

  /* new search algorithm by HB (Nicholas Vinen) */
  {
    ogg_int64_t end=vf->offsets[link+1];
    ogg_int64_t begin=vf->offsets[link];
    ogg_int64_t begintime = vf->pcmlengths[link*2];
    ogg_int64_t endtime = vf->pcmlengths[link*2+1]+begintime;
    ogg_int64_t target=pos-total+begintime;
    ogg_int64_t best=begin;

    ogg_page og;
    while(begin<end){
      ogg_int64_t bisect;

      if(end-begin<CHUNKSIZE){
    bisect=begin;
      }else{
    /* take a (pretty decent) guess. */
    bisect=begin +
      (target-begintime)*(end-begin)/(endtime-begintime) - CHUNKSIZE;
    if(bisect<=begin)
      bisect=begin+1;
      }

      _seek_helper(vf,bisect);

      while(begin<end){
    result=_get_next_page(vf,&og,end-vf->offset);
    if(result==OV_EREAD) goto seek_error;
    if(result<0){
      if(bisect<=begin+1)
        end=begin; /* found it */
      else{
        if(bisect==0) goto seek_error;
        bisect-=CHUNKSIZE;
        if(bisect<=begin)bisect=begin+1;
        _seek_helper(vf,bisect);
      }
    }else{
      ogg_int64_t granulepos=ogg_page_granulepos(&og);
      if(granulepos==-1)continue;
      if(granulepos<target){
        best=result;  /* raw offset of packet with granulepos */
        begin=vf->offset; /* raw offset of next page */
        begintime=granulepos;

        if(target-begintime>44100)break;
        bisect=begin; /* *not* begin + 1 */
      }else{
        if(bisect<=begin+1)
          end=begin;  /* found it */
        else{
          if(end==vf->offset){ /* we're pretty close - we'd be stuck in */
        end=result;
        bisect-=CHUNKSIZE; /* an endless loop otherwise. */
        if(bisect<=begin)bisect=begin+1;
        _seek_helper(vf,bisect);
          }else{
        end=result;
        endtime=granulepos;
        break;
          }
        }
      }
    }
      }
    }

    /* found our page. seek to it, update pcm offset. Easier case than
       raw_seek, don't keep packets preceeding granulepos. */
    {
      ogg_page og;
      ogg_packet op;
      /* clear out decoding machine state */
      _decode_clear(vf);
      /* seek */
      _seek_helper(vf,best);

      if(_get_next_page(vf,&og,-1)<0)return(OV_EOF); /* shouldn't happen */
      vf->current_serialno=ogg_page_serialno(&og);
      vf->current_link=link;

      ogg_stream_reset_serialno(&vf->os,vf->current_serialno);
      vf->ready_state=STREAMSET;
      ogg_stream_pagein(&vf->os,&og);

      /* pull out all but last packet; the one with granulepos */
      for(;;){
    result=ogg_stream_packetpeek(&vf->os,&op);
    if(result==0){
      /* !!! the packet finishing this page originated on a
             preceeding page. Keep fetching previous pages until we
             get one with a granulepos or without the 'continued' flag
             set.  Then just use raw_seek for simplicity. */

      _decode_clear(vf);
      _seek_helper(vf,best);

      for(;;){
        result=_get_prev_page(vf,&og);
        if(result<0) goto seek_error;
        if(ogg_page_granulepos(&og)>-1 ||
           !ogg_page_continued(&og)){
          return ov_raw_seek(vf,result);
        }
        vf->offset=result;
      }
    }
    if(result<0){
      result = OV_EBADPACKET;
      goto seek_error;
    }
    if(op.granulepos!=-1){
      vf->pcm_offset=op.granulepos-vf->pcmlengths[vf->current_link*2];
      if(vf->pcm_offset<0)vf->pcm_offset=0;
      vf->pcm_offset+=total;
      break;
    }else
      result=ogg_stream_packetout(&vf->os,NULL); result; // unused?
      }
    }
  }

  /* verify result */
  if(vf->pcm_offset>pos || pos>ov_pcm_total(vf,-1)){
    result=OV_EFAULT;
    goto seek_error;
  }
  return(0);

 seek_error:
  /* dump machine so we're in a known state */
  vf->pcm_offset=-1;
  _decode_clear(vf);
  return (int)result;
}

/* seek to a sample offset relative to the decompressed pcm stream
   returns zero on success, nonzero on failure */

int ov_pcm_seek(OggVorbis_File *vf,ogg_int64_t pos){
  int thisblock,lastblock=0;
  int ret=ov_pcm_seek_page(vf,pos);
  if(ret<0)return(ret);
  _make_decode_ready(vf);

  /* discard leading packets we don't need for the lapping of the
     position we want; don't decode them */

  for(;;){
    ogg_packet op;
    ogg_page og;

    int ret=ogg_stream_packetpeek(&vf->os,&op);
    if(ret>0){
      thisblock=vorbis_packet_blocksize(vf->vi+vf->current_link,&op);
      if(thisblock<0)thisblock=0; /* non audio packet */
      if(lastblock)vf->pcm_offset+=(lastblock+thisblock)>>2;

      if(vf->pcm_offset+((thisblock+
              vorbis_info_blocksize(vf->vi,1))>>2)>=pos)break;

      /* remove the packet from packet queue and track its granulepos */
      ogg_stream_packetout(&vf->os,NULL);
      vorbis_synthesis_trackonly(&vf->vb,&op);  /* set up a vb with
                                                   only tracking, no
                                                   pcm_decode */
      vorbis_synthesis_blockin(&vf->vd,&vf->vb);

      /* end of logical stream case is hard, especially with exact
     length positioning. */

      if(op.granulepos>-1){
    int i;
    /* always believe the stream markers */
    vf->pcm_offset=op.granulepos-vf->pcmlengths[vf->current_link*2];
    if(vf->pcm_offset<0)vf->pcm_offset=0;
    for(i=0;i<vf->current_link;i++)
      vf->pcm_offset+=vf->pcmlengths[i*2+1];
      }

      lastblock=thisblock;

    }else{
      if(ret<0 && ret!=OV_HOLE)break;

      /* suck in a new page */
      if(_get_next_page(vf,&og,-1)<0)break;
      if(vf->current_serialno!=ogg_page_serialno(&og))_decode_clear(vf);

      if(vf->ready_state<STREAMSET){
    int link;

    vf->current_serialno=ogg_page_serialno(&og);
    for(link=0;link<vf->links;link++)
      if(vf->serialnos[link]==vf->current_serialno)break;
    if(link==vf->links)return(OV_EBADLINK);
    vf->current_link=link;

    ogg_stream_reset_serialno(&vf->os,vf->current_serialno);
    vf->ready_state=STREAMSET;
    _make_decode_ready(vf);
    lastblock=0;
      }

      ogg_stream_pagein(&vf->os,&og);
    }
  }

  /* discard samples until we reach the desired position. Crossing a
     logical bitstream boundary with abandon is OK. */
  while(vf->pcm_offset<pos){
    float **pcm;
    ogg_int64_t target=pos-vf->pcm_offset;
    long samples=vorbis_synthesis_pcmout(&vf->vd,&pcm);

    if(samples>target)samples=(long)target;
    vorbis_synthesis_read(&vf->vd,samples);
    vf->pcm_offset+=samples;

    if(samples<target)
      if(_fetch_and_process_packet(vf,NULL,1)<=0)
    vf->pcm_offset=ov_pcm_total(vf,-1); /* eof */
  }
  return 0;
}

/* seek to a playback time relative to the decompressed pcm stream
   returns zero on success, nonzero on failure */
int ov_time_seek(OggVorbis_File *vf,double seconds){
  /* translate time to PCM position and call ov_pcm_seek */

  int link=-1; link; // unused?
  ogg_int64_t pcm_total=ov_pcm_total(vf,-1);
  double time_total=ov_time_total(vf,-1);

  if(vf->ready_state<OPENED)return(OV_EINVAL);
  if(!vf->seekable)return(OV_ENOSEEK);
  if(seconds<0 || seconds>time_total)return(OV_EINVAL);

  /* which bitstream section does this time offset occur in? */
  for(link=vf->links-1;link>=0;link--){
    pcm_total-=vf->pcmlengths[link*2+1];
    time_total-=ov_time_total(vf,link);
    if(seconds>=time_total)break;
  }

  /* enough information to convert time offset to pcm offset */
  {
    ogg_int64_t target=(ogg_int64_t)(pcm_total+(seconds-time_total)*vf->vi[link].rate);
    return(ov_pcm_seek(vf,target));
  }
}

/* page-granularity version of ov_time_seek
   returns zero on success, nonzero on failure */
int ov_time_seek_page(OggVorbis_File *vf,double seconds){
  /* translate time to PCM position and call ov_pcm_seek */

  int link=-1; link; // unused?
  ogg_int64_t pcm_total=ov_pcm_total(vf,-1);
  double time_total=ov_time_total(vf,-1);

  if(vf->ready_state<OPENED)return(OV_EINVAL);
  if(!vf->seekable)return(OV_ENOSEEK);
  if(seconds<0 || seconds>time_total)return(OV_EINVAL);

  /* which bitstream section does this time offset occur in? */
  for(link=vf->links-1;link>=0;link--){
    pcm_total-=vf->pcmlengths[link*2+1];
    time_total-=ov_time_total(vf,link);
    if(seconds>=time_total)break;
  }

  /* enough information to convert time offset to pcm offset */
  {
    ogg_int64_t target=(ogg_int64_t)(pcm_total+(seconds-time_total)*vf->vi[link].rate);
    return(ov_pcm_seek_page(vf,target));
  }
}

/* tell the current stream offset cursor.  Note that seek followed by
   tell will likely not give the set offset due to caching */
ogg_int64_t ov_raw_tell(OggVorbis_File *vf){
  if(vf->ready_state<OPENED)return(OV_EINVAL);
  return(vf->offset);
}

/* return PCM offset (sample) of next PCM sample to be read */
ogg_int64_t ov_pcm_tell(OggVorbis_File *vf){
  if(vf->ready_state<OPENED)return(OV_EINVAL);
  return(vf->pcm_offset);
}

/* return time offset (seconds) of next PCM sample to be read */
double ov_time_tell(OggVorbis_File *vf){
  int link=0;
  ogg_int64_t pcm_total=0;
  double time_total=0.f;

  if(vf->ready_state<OPENED)return(OV_EINVAL);
  if(vf->seekable){
    pcm_total=ov_pcm_total(vf,-1);
    time_total=ov_time_total(vf,-1);

    /* which bitstream section does this time offset occur in? */
    for(link=vf->links-1;link>=0;link--){
      pcm_total-=vf->pcmlengths[link*2+1];
      time_total-=ov_time_total(vf,link);
      if(vf->pcm_offset>=pcm_total)break;
    }
  }

  return((double)time_total+(double)(vf->pcm_offset-pcm_total)/vf->vi[link].rate);
}

/*  link:   -1) return the vorbis_info struct for the bitstream section
                currently being decoded
           0-n) to request information for a specific bitstream section

    In the case of a non-seekable bitstream, any call returns the
    current bitstream.  NULL in the case that the machine is not
    initialized */

vorbis_info *ov_info(OggVorbis_File *vf,int link){
  if(vf->seekable){
    if(link<0)
      if(vf->ready_state>=STREAMSET)
    return vf->vi+vf->current_link;
      else
      return vf->vi;
    else
      if(link>=vf->links)
    return NULL;
      else
    return vf->vi+link;
  }else{
    return vf->vi;
  }
}

/* grr, strong typing, grr, no templates/inheritence, grr */
vorbis_comment *ov_comment(OggVorbis_File *vf,int link){
  if(vf->seekable){
    if(link<0)
      if(vf->ready_state>=STREAMSET)
    return vf->vc+vf->current_link;
      else
    return vf->vc;
    else
      if(link>=vf->links)
    return NULL;
      else
    return vf->vc+link;
  }else{
    return vf->vc;
  }
}

int host_is_big_endian() {
  ogg_int32_t pattern = 0xfeedface; /* deadbeef */
  unsigned char *bytewise = (unsigned char *)&pattern;
  if (bytewise[0] == 0xfe) return 1;
  return 0;
}

/* up to this point, everything could more or less hide the multiple
   logical bitstream nature of chaining from the toplevel application
   if the toplevel application didn't particularly care.  However, at
   the point that we actually read audio back, the multiple-section
   nature must surface: Multiple bitstream sections do not necessarily
   have to have the same number of channels or sampling rate.

   ov_read returns the sequential logical bitstream number currently
   being decoded along with the PCM data in order that the toplevel
   application can take action on channel/sample rate changes.  This
   number will be incremented even for streamed (non-seekable) streams
   (for seekable streams, it represents the actual logical bitstream
   index within the physical bitstream.  Note that the accessor
   functions above are aware of this dichotomy).

   input values: buffer) a buffer to hold packed PCM data for return
         length) the byte length requested to be placed into buffer
         bigendianp) should the data be packed LSB first (0) or
                     MSB first (1)
         word) word size for output.  currently 1 (byte) or
               2 (16 bit short)

   return values: <0) error/hole in data (OV_HOLE), partial open (OV_EINVAL)
                   0) EOF
           n) number of bytes of PCM actually returned.  The
           below works on a packet-by-packet basis, so the
           return length is not related to the 'length' passed
           in, just guaranteed to fit.

        *section) set to the logical bitstream number */

long ov_read(OggVorbis_File *vf,char *buffer,int length,
            int bigendianp,int word,int sgned,int *bitstream){
  int i,j;
  int host_endian = host_is_big_endian();

  float **pcm;
  long samples;

  if(vf->ready_state<OPENED)return(OV_EINVAL);

  for(;;){
    if(vf->ready_state>=STREAMSET){
      samples=vorbis_synthesis_pcmout(&vf->vd,&pcm);
      if(samples)break;
    }

    /* suck in another packet */
    {
      int ret=_fetch_and_process_packet(vf,NULL,1);
      if(ret==OV_EOF)return(0);
      if(ret<=0)return(ret);
    }

  }

  if(samples>0){

    /* yay! proceed to pack data into the byte buffer */

    long channels=ov_info(vf,-1)->channels;
    long bytespersample=word * channels;
    vorbis_fpu_control fpu; fpu=0; fpu; // dummy stuff so the compiler agrees that it really is used
    if(samples>length/bytespersample)samples=length/bytespersample;

    if(samples <= 0)
      return OV_EINVAL;

    /* a tight loop to pack each size */
    {
      int val;
      if(word==1){
    int off=(sgned?0:128);
    vorbis_fpu_setround(&fpu);
    for(j=0;j<samples;j++)
      for(i=0;i<channels;i++){
        val=vorbis_ftoi(pcm[i][j]*128.f);
        if(val>127)val=127;
        else if(val<-128)val=-128;
        *buffer++=(char)(val+off);
      }
    vorbis_fpu_restore(fpu);
      }else{
    int off=(sgned?0:32768);

    if(host_endian==bigendianp){
      if(sgned){

        vorbis_fpu_setround(&fpu);
        for(i=0;i<channels;i++) { /* It's faster in this order */
          float *src=pcm[i];
          short *dest=((short *)buffer)+i;
          for(j=0;j<samples;j++) {
        val=vorbis_ftoi(src[j]*32768.f);
        if(val>32767)val=32767;
        else if(val<-32768)val=-32768;
        *dest=(short)val;
        dest+=channels;
          }
        }
        vorbis_fpu_restore(fpu);

      }else{

        vorbis_fpu_setround(&fpu);
        for(i=0;i<channels;i++) {
          float *src=pcm[i];
          short *dest=((short *)buffer)+i;
          for(j=0;j<samples;j++) {
        val=vorbis_ftoi(src[j]*32768.f);
        if(val>32767)val=32767;
        else if(val<-32768)val=-32768;
        *dest=(short)(val+off);
        dest+=channels;
          }
        }
        vorbis_fpu_restore(fpu);

      }
    }else if(bigendianp){

      vorbis_fpu_setround(&fpu);
      for(j=0;j<samples;j++)
        for(i=0;i<channels;i++){
          val=vorbis_ftoi(pcm[i][j]*32768.f);
          if(val>32767)val=32767;
          else if(val<-32768)val=-32768;
          val+=off;
          *buffer++=(char)(val>>8);
          *buffer++=(char)(val&0xff);
        }
      vorbis_fpu_restore(fpu);

    }else{
      int val;
      vorbis_fpu_setround(&fpu);
      for(j=0;j<samples;j++)
        for(i=0;i<channels;i++){
          val=vorbis_ftoi(pcm[i][j]*32768.f);
          if(val>32767)val=32767;
          else if(val<-32768)val=-32768;
          val+=off;
          *buffer++=(char)(val&0xff);
          *buffer++=(char)(val>>8);
        }
      vorbis_fpu_restore(fpu);

    }
      }
    }

    vorbis_synthesis_read(&vf->vd,samples);
    vf->pcm_offset+=samples;
    if(bitstream)*bitstream=vf->current_link;
    return(samples*bytespersample);
  }else{
    return(samples);
  }
}

/* input values: pcm_channels) a float vector per channel of output
         length) the sample length being read by the app

   return values: <0) error/hole in data (OV_HOLE), partial open (OV_EINVAL)
                   0) EOF
           n) number of samples of PCM actually returned.  The
           below works on a packet-by-packet basis, so the
           return length is not related to the 'length' passed
           in, just guaranteed to fit.

        *section) set to the logical bitstream number */



long ov_read_float(OggVorbis_File *vf,float ***pcm_channels,int length,
           int *bitstream){

  if(vf->ready_state<OPENED)return(OV_EINVAL);

  for(;;){
    if(vf->ready_state>=STREAMSET){
      float **pcm;
      long samples=vorbis_synthesis_pcmout(&vf->vd,&pcm);
      if(samples){
    if(pcm_channels)*pcm_channels=pcm;
    if(samples>length)samples=length;
    vorbis_synthesis_read(&vf->vd,samples);
    vf->pcm_offset+=samples;
    if(bitstream)*bitstream=vf->current_link;
    return samples;

      }
    }

    /* suck in another packet */
    {
      int ret=_fetch_and_process_packet(vf,NULL,1);
      if(ret==OV_EOF)return(0);
      if(ret<=0)return(ret);
    }

  }
}






// ==========================================================================
// ==========================================================================
// synthesis.c
// ==========================================================================
// ==========================================================================
// function: single-block PCM synthesis

int vorbis_synthesis(vorbis_block *vb,ogg_packet *op){
  vorbis_dsp_state     *vd=vb->vd;
  private_state        *b=static_cast<private_state*>(vd->backend_state);
  vorbis_info          *vi=vd->vi;
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  oggpack_buffer       *opb=&vb->opb;
  int                   type,mode,i;

  /* first things first.  Make sure decode is ready */
  _vorbis_block_ripcord(vb);
  oggpack_readinit(opb,op->packet,op->bytes);

  /* Check the packet type */
  if(oggpack_read(opb,1)!=0){
    /* Oops.  This is not an audio data packet */
    return(OV_ENOTAUDIO);
  }

  /* read our mode and pre/post windowsize */
  mode=oggpack_read(opb,b->modebits);
  if(mode==-1)return(OV_EBADPACKET);

  vb->mode=mode;
  vb->W=ci->mode_param[mode]->blockflag;
  if(vb->W){

    /* this doesn;t get mapped through mode selection as it's used
       only for window selection */
    vb->lW=oggpack_read(opb,1);
    vb->nW=oggpack_read(opb,1);
    if(vb->nW==-1)   return(OV_EBADPACKET);
  }else{
    vb->lW=0;
    vb->nW=0;
  }

  /* more setup */
  vb->granulepos=op->granulepos;
  vb->sequence=op->packetno;
  vb->eofflag=op->e_o_s;

  /* alloc pcm passback storage */
  vb->pcmend=ci->blocksizes[vb->W];
  vb->pcm=static_cast<float**>(_vorbis_block_alloc(vb,sizeof(*vb->pcm)*vi->channels));
  for(i=0;i<vi->channels;i++)
    vb->pcm[i]=static_cast<float*>(_vorbis_block_alloc(vb,vb->pcmend*sizeof(*vb->pcm[i])));

  /* unpack_header enforces range checking */
  type=ci->map_type[ci->mode_param[mode]->mapping];

  return(_mapping_P[type]->inverse(vb,ci->map_param[ci->mode_param[mode]->
                           mapping]));
}

/* used to track pcm position without actually performing decode.
   Useful for sequential 'fast forward' */
int vorbis_synthesis_trackonly(vorbis_block *vb,ogg_packet *op){
  vorbis_dsp_state     *vd=vb->vd;
  private_state        *b=static_cast<private_state*>(vd->backend_state);
  vorbis_info          *vi=vd->vi;
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  oggpack_buffer       *opb=&vb->opb;
  int                   mode;

  /* first things first.  Make sure decode is ready */
  _vorbis_block_ripcord(vb);
  oggpack_readinit(opb,op->packet,op->bytes);

  /* Check the packet type */
  if(oggpack_read(opb,1)!=0){
    /* Oops.  This is not an audio data packet */
    return(OV_ENOTAUDIO);
  }

  /* read our mode and pre/post windowsize */
  mode=oggpack_read(opb,b->modebits);
  if(mode==-1)return(OV_EBADPACKET);

  vb->mode=mode;
  vb->W=ci->mode_param[mode]->blockflag;
  if(vb->W){
    vb->lW=oggpack_read(opb,1);
    vb->nW=oggpack_read(opb,1);
    if(vb->nW==-1)   return(OV_EBADPACKET);
  }else{
    vb->lW=0;
    vb->nW=0;
  }

  /* more setup */
  vb->granulepos=op->granulepos;
  vb->sequence=op->packetno;
  vb->eofflag=op->e_o_s;

  /* no pcm */
  vb->pcmend=0;
  vb->pcm=NULL;

  return(0);
}

long vorbis_packet_blocksize(vorbis_info *vi,ogg_packet *op){
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  oggpack_buffer       opb;
  int                  mode;

  oggpack_readinit(&opb,op->packet,op->bytes);

  /* Check the packet type */
  if(oggpack_read(&opb,1)!=0){
    /* Oops.  This is not an audio data packet */
    return(OV_ENOTAUDIO);
  }

  {
    int modebits=0;
    int v=ci->modes;
    while(v>1){
      modebits++;
      v>>=1;
    }

    /* read our mode and pre/post windowsize */
    mode=oggpack_read(&opb,modebits);
  }
  if(mode==-1)return(OV_EBADPACKET);
  return(ci->blocksizes[ci->mode_param[mode]->blockflag]);
}










// ==========================================================================
// ==========================================================================
// smallft.c
// ==========================================================================
// ==========================================================================

/* FFT implementation from OggSquish, minus cosine transforms,
 * minus all but radix 2/4 case.  In Vorbis we only need this
 * cut-down version.
 *
 * To do more than just power-of-two sized vectors, see the full
 * version I wrote for NetLib.
 *
 * Note that the packing is a little strange; rather than the FFT r/i
 * packing following R_0, I_n, R_1, I_1, R_2, I_2 ... R_n-1, I_n-1,
 * it follows R_0, R_1, I_1, R_2, I_2 ... R_n-1, I_n-1, I_n like the
 * FORTRAN version
 */

 void drfti1(int n, float *wa, int *ifac){
  int ntryh[4] = { 4,2,3,5 };
  float tpi = 6.28318530717958648f;
  float arg,argh,argld,fi;
  int ntry=0,i,j=-1;
  int k1, l1, l2, ib;
  int ld, ii, ip, is, nq, nr;
  int ido, ipm, nfm1;
  int nl=n;
  int nf=0;

 L101:
  j++;
  if (j < 4)
    ntry=ntryh[j];
  else
    ntry+=2;

 L104:
  nq=nl/ntry;
  nr=nl-ntry*nq;
  if (nr!=0) goto L101;

  nf++;
  ifac[nf+1]=ntry;
  nl=nq;
  if(ntry!=2)goto L107;
  if(nf==1)goto L107;

  for (i=1;i<nf;i++){
    ib=nf-i+1;
    ifac[ib+1]=ifac[ib];
  }
  ifac[2] = 2;

 L107:
  if(nl!=1)goto L104;
  ifac[0]=n;
  ifac[1]=nf;
  argh=tpi/n;
  is=0;
  nfm1=nf-1;
  l1=1;

  if(nfm1==0)return;

  for (k1=0;k1<nfm1;k1++){
    ip=ifac[k1+2];
    ld=0;
    l2=l1*ip;
    ido=n/l2;
    ipm=ip-1;

    for (j=0;j<ipm;j++){
      ld+=l1;
      i=is;
      argld=(float)ld*argh;
      fi=0.f;
      for (ii=2;ii<ido;ii+=2){
    fi+=1.f;
    arg=fi*argld;
    wa[i++]=(float)cos(arg);
    wa[i++]=(float)sin(arg);
      }
      is+=ido;
    }
    l1=l2;
  }
}

void fdrffti(int n, float *wsave, int *ifac){

  if (n == 1) return;
  drfti1(n, wsave+n, ifac);
}

void dradf2(int ido,int l1,float *cc,float *ch,float *wa1){
  int i,k;
  float ti2,tr2;
  int t0,t1,t2,t3,t4,t5,t6;

  t1=0;
  t0=(t2=l1*ido);
  t3=ido<<1;
  for(k=0;k<l1;k++){
    ch[t1<<1]=cc[t1]+cc[t2];
    ch[(t1<<1)+t3-1]=cc[t1]-cc[t2];
    t1+=ido;
    t2+=ido;
  }

  if(ido<2)return;
  if(ido==2)goto L105;

  t1=0;
  t2=t0;
  for(k=0;k<l1;k++){
    t3=t2;
    t4=(t1<<1)+(ido<<1);
    t5=t1;
    t6=t1+t1;
    for(i=2;i<ido;i+=2){
      t3+=2;
      t4-=2;
      t5+=2;
      t6+=2;
      tr2=wa1[i-2]*cc[t3-1]+wa1[i-1]*cc[t3];
      ti2=wa1[i-2]*cc[t3]-wa1[i-1]*cc[t3-1];
      ch[t6]=cc[t5]+ti2;
      ch[t4]=ti2-cc[t5];
      ch[t6-1]=cc[t5-1]+tr2;
      ch[t4-1]=cc[t5-1]-tr2;
    }
    t1+=ido;
    t2+=ido;
  }

  if(ido%2==1)return;

 L105:
  t3=(t2=(t1=ido)-1);
  t2+=t0;
  for(k=0;k<l1;k++){
    ch[t1]=-cc[t2];
    ch[t1-1]=cc[t3];
    t1+=ido<<1;
    t2+=ido;
    t3+=ido;
  }
}

void dradf4(int ido,int l1,float *cc,float *ch,float *wa1,
        float *wa2,float *wa3){
  float hsqt2 = .70710678118654752f;
  int i,k,t0,t1,t2,t3,t4,t5,t6;
  float ci2,ci3,ci4,cr2,cr3,cr4,ti1,ti2,ti3,ti4,tr1,tr2,tr3,tr4;
  t0=l1*ido;

  t1=t0;
  t4=t1<<1;
  t2=t1+(t1<<1);
  t3=0;

  for(k=0;k<l1;k++){
    tr1=cc[t1]+cc[t2];
    tr2=cc[t3]+cc[t4];

    ch[t5=t3<<2]=tr1+tr2;
    ch[(ido<<2)+t5-1]=tr2-tr1;
    ch[(t5+=(ido<<1))-1]=cc[t3]-cc[t4];
    ch[t5]=cc[t2]-cc[t1];

    t1+=ido;
    t2+=ido;
    t3+=ido;
    t4+=ido;
  }

  if(ido<2)return;
  if(ido==2)goto L105;


  t1=0;
  for(k=0;k<l1;k++){
    t2=t1;
    t4=t1<<2;
    t5=(t6=ido<<1)+t4;
    for(i=2;i<ido;i+=2){
      t3=(t2+=2);
      t4+=2;
      t5-=2;

      t3+=t0;
      cr2=wa1[i-2]*cc[t3-1]+wa1[i-1]*cc[t3];
      ci2=wa1[i-2]*cc[t3]-wa1[i-1]*cc[t3-1];
      t3+=t0;
      cr3=wa2[i-2]*cc[t3-1]+wa2[i-1]*cc[t3];
      ci3=wa2[i-2]*cc[t3]-wa2[i-1]*cc[t3-1];
      t3+=t0;
      cr4=wa3[i-2]*cc[t3-1]+wa3[i-1]*cc[t3];
      ci4=wa3[i-2]*cc[t3]-wa3[i-1]*cc[t3-1];

      tr1=cr2+cr4;
      tr4=cr4-cr2;
      ti1=ci2+ci4;
      ti4=ci2-ci4;

      ti2=cc[t2]+ci3;
      ti3=cc[t2]-ci3;
      tr2=cc[t2-1]+cr3;
      tr3=cc[t2-1]-cr3;

      ch[t4-1]=tr1+tr2;
      ch[t4]=ti1+ti2;

      ch[t5-1]=tr3-ti4;
      ch[t5]=tr4-ti3;

      ch[t4+t6-1]=ti4+tr3;
      ch[t4+t6]=tr4+ti3;

      ch[t5+t6-1]=tr2-tr1;
      ch[t5+t6]=ti1-ti2;
    }
    t1+=ido;
  }
  if(ido&1)return;

 L105:

  t2=(t1=t0+ido-1)+(t0<<1);
  t3=ido<<2;
  t4=ido;
  t5=ido<<1;
  t6=ido;

  for(k=0;k<l1;k++){
    ti1=-hsqt2*(cc[t1]+cc[t2]);
    tr1=hsqt2*(cc[t1]-cc[t2]);

    ch[t4-1]=tr1+cc[t6-1];
    ch[t4+t5-1]=cc[t6-1]-tr1;

    ch[t4]=ti1-cc[t1+t0];
    ch[t4+t5]=ti1+cc[t1+t0];

    t1+=ido;
    t2+=ido;
    t4+=t3;
    t6+=ido;
  }
}

void dradfg(int ido,int ip,int l1,int idl1,float *cc,float *c1,
                          float *c2,float *ch,float *ch2,float *wa){

  float tpi=6.283185307179586f;
  int idij,ipph,i,j,k,l,ic,ik,is;
  int t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
  float dc2,ai1,ai2,ar1,ar2,ds2;
  int nbd;
  float dcp,arg,dsp,ar1h,ar2h;
  int idp2,ipp2;

  arg=tpi/(float)ip;
  dcp=(float)cos(arg);
  dsp=(float)sin(arg);
  ipph=(ip+1)>>1;
  ipp2=ip;
  idp2=ido;
  nbd=(ido-1)>>1;
  t0=l1*ido;
  t10=ip*ido;

  if(ido==1)goto L119;
  for(ik=0;ik<idl1;ik++)ch2[ik]=c2[ik];

  t1=0;
  for(j=1;j<ip;j++){
    t1+=t0;
    t2=t1;
    for(k=0;k<l1;k++){
      ch[t2]=c1[t2];
      t2+=ido;
    }
  }

  is=-ido;
  t1=0;
  if(nbd>l1){
    for(j=1;j<ip;j++){
      t1+=t0;
      is+=ido;
      t2= -ido+t1;
      for(k=0;k<l1;k++){
        idij=is-1;
        t2+=ido;
        t3=t2;
        for(i=2;i<ido;i+=2){
          idij+=2;
          t3+=2;
          ch[t3-1]=wa[idij-1]*c1[t3-1]+wa[idij]*c1[t3];
          ch[t3]=wa[idij-1]*c1[t3]-wa[idij]*c1[t3-1];
        }
      }
    }
  }else{

    for(j=1;j<ip;j++){
      is+=ido;
      idij=is-1;
      t1+=t0;
      t2=t1;
      for(i=2;i<ido;i+=2){
        idij+=2;
        t2+=2;
        t3=t2;
        for(k=0;k<l1;k++){
          ch[t3-1]=wa[idij-1]*c1[t3-1]+wa[idij]*c1[t3];
          ch[t3]=wa[idij-1]*c1[t3]-wa[idij]*c1[t3-1];
          t3+=ido;
        }
      }
    }
  }

  t1=0;
  t2=ipp2*t0;
  if(nbd<l1){
    for(j=1;j<ipph;j++){
      t1+=t0;
      t2-=t0;
      t3=t1;
      t4=t2;
      for(i=2;i<ido;i+=2){
        t3+=2;
        t4+=2;
        t5=t3-ido;
        t6=t4-ido;
        for(k=0;k<l1;k++){
          t5+=ido;
          t6+=ido;
          c1[t5-1]=ch[t5-1]+ch[t6-1];
          c1[t6-1]=ch[t5]-ch[t6];
          c1[t5]=ch[t5]+ch[t6];
          c1[t6]=ch[t6-1]-ch[t5-1];
        }
      }
    }
  }else{
    for(j=1;j<ipph;j++){
      t1+=t0;
      t2-=t0;
      t3=t1;
      t4=t2;
      for(k=0;k<l1;k++){
        t5=t3;
        t6=t4;
        for(i=2;i<ido;i+=2){
          t5+=2;
          t6+=2;
          c1[t5-1]=ch[t5-1]+ch[t6-1];
          c1[t6-1]=ch[t5]-ch[t6];
          c1[t5]=ch[t5]+ch[t6];
          c1[t6]=ch[t6-1]-ch[t5-1];
        }
        t3+=ido;
        t4+=ido;
      }
    }
  }

L119:
  for(ik=0;ik<idl1;ik++)c2[ik]=ch2[ik];

  t1=0;
  t2=ipp2*idl1;
  for(j=1;j<ipph;j++){
    t1+=t0;
    t2-=t0;
    t3=t1-ido;
    t4=t2-ido;
    for(k=0;k<l1;k++){
      t3+=ido;
      t4+=ido;
      c1[t3]=ch[t3]+ch[t4];
      c1[t4]=ch[t4]-ch[t3];
    }
  }

  ar1=1.f;
  ai1=0.f;
  t1=0;
  t2=ipp2*idl1;
  t3=(ip-1)*idl1;
  for(l=1;l<ipph;l++){
    t1+=idl1;
    t2-=idl1;
    ar1h=dcp*ar1-dsp*ai1;
    ai1=dcp*ai1+dsp*ar1;
    ar1=ar1h;
    t4=t1;
    t5=t2;
    t6=t3;
    t7=idl1;

    for(ik=0;ik<idl1;ik++){
      ch2[t4++]=c2[ik]+ar1*c2[t7++];
      ch2[t5++]=ai1*c2[t6++];
    }

    dc2=ar1;
    ds2=ai1;
    ar2=ar1;
    ai2=ai1;

    t4=idl1;
    t5=(ipp2-1)*idl1;
    for(j=2;j<ipph;j++){
      t4+=idl1;
      t5-=idl1;

      ar2h=dc2*ar2-ds2*ai2;
      ai2=dc2*ai2+ds2*ar2;
      ar2=ar2h;

      t6=t1;
      t7=t2;
      t8=t4;
      t9=t5;
      for(ik=0;ik<idl1;ik++){
        ch2[t6++]+=ar2*c2[t8++];
        ch2[t7++]+=ai2*c2[t9++];
      }
    }
  }

  t1=0;
  for(j=1;j<ipph;j++){
    t1+=idl1;
    t2=t1;
    for(ik=0;ik<idl1;ik++)ch2[ik]+=c2[t2++];
  }

  if(ido<l1)goto L132;

  t1=0;
  t2=0;
  for(k=0;k<l1;k++){
    t3=t1;
    t4=t2;
    for(i=0;i<ido;i++)cc[t4++]=ch[t3++];
    t1+=ido;
    t2+=t10;
  }

  goto L135;

 L132:
  for(i=0;i<ido;i++){
    t1=i;
    t2=i;
    for(k=0;k<l1;k++){
      cc[t2]=ch[t1];
      t1+=ido;
      t2+=t10;
    }
  }

 L135:
  t1=0;
  t2=ido<<1;
  t3=0;
  t4=ipp2*t0;
  for(j=1;j<ipph;j++){

    t1+=t2;
    t3+=t0;
    t4-=t0;

    t5=t1;
    t6=t3;
    t7=t4;

    for(k=0;k<l1;k++){
      cc[t5-1]=ch[t6];
      cc[t5]=ch[t7];
      t5+=t10;
      t6+=ido;
      t7+=ido;
    }
  }

  if(ido==1)return;
  if(nbd<l1)goto L141;

  t1=-ido;
  t3=0;
  t4=0;
  t5=ipp2*t0;
  for(j=1;j<ipph;j++){
    t1+=t2;
    t3+=t2;
    t4+=t0;
    t5-=t0;
    t6=t1;
    t7=t3;
    t8=t4;
    t9=t5;
    for(k=0;k<l1;k++){
      for(i=2;i<ido;i+=2){
        ic=idp2-i;
        cc[i+t7-1]=ch[i+t8-1]+ch[i+t9-1];
        cc[ic+t6-1]=ch[i+t8-1]-ch[i+t9-1];
        cc[i+t7]=ch[i+t8]+ch[i+t9];
        cc[ic+t6]=ch[i+t9]-ch[i+t8];
      }
      t6+=t10;
      t7+=t10;
      t8+=ido;
      t9+=ido;
    }
  }
  return;

 L141:

  t1=-ido;
  t3=0;
  t4=0;
  t5=ipp2*t0;
  for(j=1;j<ipph;j++){
    t1+=t2;
    t3+=t2;
    t4+=t0;
    t5-=t0;
    for(i=2;i<ido;i+=2){
      t6=idp2+t1-i;
      t7=i+t3;
      t8=i+t4;
      t9=i+t5;
      for(k=0;k<l1;k++){
        cc[t7-1]=ch[t8-1]+ch[t9-1];
        cc[t6-1]=ch[t8-1]-ch[t9-1];
        cc[t7]=ch[t8]+ch[t9];
        cc[t6]=ch[t9]-ch[t8];
        t6+=t10;
        t7+=t10;
        t8+=ido;
        t9+=ido;
      }
    }
  }
}

void drftf1(int n,float *c,float *ch,float *wa,int *ifac){
  int i,k1,l1,l2;
  int na,kh,nf;
  int ip,iw,ido,idl1,ix2,ix3;

  nf=ifac[1];
  na=1;
  l2=n;
  iw=n;

  for(k1=0;k1<nf;k1++){
    kh=nf-k1;
    ip=ifac[kh+1];
    l1=l2/ip;
    ido=n/l2;
    idl1=ido*l1;
    iw-=(ip-1)*ido;
    na=1-na;

    if(ip!=4)goto L102;

    ix2=iw+ido;
    ix3=ix2+ido;
    if(na!=0)
      dradf4(ido,l1,ch,c,wa+iw-1,wa+ix2-1,wa+ix3-1);
    else
      dradf4(ido,l1,c,ch,wa+iw-1,wa+ix2-1,wa+ix3-1);
    goto L110;

 L102:
    if(ip!=2)goto L104;
    if(na!=0)goto L103;

    dradf2(ido,l1,c,ch,wa+iw-1);
    goto L110;

  L103:
    dradf2(ido,l1,ch,c,wa+iw-1);
    goto L110;

  L104:
    if(ido==1)na=1-na;
    if(na!=0)goto L109;

    dradfg(ido,ip,l1,idl1,c,c,c,ch,ch,wa+iw-1);
    na=1;
    goto L110;

  L109:
    dradfg(ido,ip,l1,idl1,ch,ch,ch,c,c,wa+iw-1);
    na=0;

  L110:
    l2=l1;
  }

  if(na==1)return;

  for(i=0;i<n;i++)c[i]=ch[i];
}

void dradb2(int ido,int l1,float *cc,float *ch,float *wa1){
  int i,k,t0,t1,t2,t3,t4,t5,t6;
  float ti2,tr2;

  t0=l1*ido;

  t1=0;
  t2=0;
  t3=(ido<<1)-1;
  for(k=0;k<l1;k++){
    ch[t1]=cc[t2]+cc[t3+t2];
    ch[t1+t0]=cc[t2]-cc[t3+t2];
    t2=(t1+=ido)<<1;
  }

  if(ido<2)return;
  if(ido==2)goto L105;

  t1=0;
  t2=0;
  for(k=0;k<l1;k++){
    t3=t1;
    t5=(t4=t2)+(ido<<1);
    t6=t0+t1;
    for(i=2;i<ido;i+=2){
      t3+=2;
      t4+=2;
      t5-=2;
      t6+=2;
      ch[t3-1]=cc[t4-1]+cc[t5-1];
      tr2=cc[t4-1]-cc[t5-1];
      ch[t3]=cc[t4]-cc[t5];
      ti2=cc[t4]+cc[t5];
      ch[t6-1]=wa1[i-2]*tr2-wa1[i-1]*ti2;
      ch[t6]=wa1[i-2]*ti2+wa1[i-1]*tr2;
    }
    t2=(t1+=ido)<<1;
  }

  if(ido%2==1)return;

L105:
  t1=ido-1;
  t2=ido-1;
  for(k=0;k<l1;k++){
    ch[t1]=cc[t2]+cc[t2];
    ch[t1+t0]=-(cc[t2+1]+cc[t2+1]);
    t1+=ido;
    t2+=ido<<1;
  }
}

void dradb3(int ido,int l1,float *cc,float *ch,float *wa1,
                          float *wa2){
  float taur = -.5f;
  float taui = .8660254037844386f;
  int i,k,t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
  float ci2,ci3,di2,di3,cr2,cr3,dr2,dr3,ti2,tr2;
  t0=l1*ido;

  t1=0;
  t2=t0<<1;
  t3=ido<<1;
  t4=ido+(ido<<1);
  t5=0;
  for(k=0;k<l1;k++){
    tr2=cc[t3-1]+cc[t3-1];
    cr2=cc[t5]+(taur*tr2);
    ch[t1]=cc[t5]+tr2;
    ci3=taui*(cc[t3]+cc[t3]);
    ch[t1+t0]=cr2-ci3;
    ch[t1+t2]=cr2+ci3;
    t1+=ido;
    t3+=t4;
    t5+=t4;
  }

  if(ido==1)return;

  t1=0;
  t3=ido<<1;
  for(k=0;k<l1;k++){
    t7=t1+(t1<<1);
    t6=(t5=t7+t3);
    t8=t1;
    t10=(t9=t1+t0)+t0;

    for(i=2;i<ido;i+=2){
      t5+=2;
      t6-=2;
      t7+=2;
      t8+=2;
      t9+=2;
      t10+=2;
      tr2=cc[t5-1]+cc[t6-1];
      cr2=cc[t7-1]+(taur*tr2);
      ch[t8-1]=cc[t7-1]+tr2;
      ti2=cc[t5]-cc[t6];
      ci2=cc[t7]+(taur*ti2);
      ch[t8]=cc[t7]+ti2;
      cr3=taui*(cc[t5-1]-cc[t6-1]);
      ci3=taui*(cc[t5]+cc[t6]);
      dr2=cr2-ci3;
      dr3=cr2+ci3;
      di2=ci2+cr3;
      di3=ci2-cr3;
      ch[t9-1]=wa1[i-2]*dr2-wa1[i-1]*di2;
      ch[t9]=wa1[i-2]*di2+wa1[i-1]*dr2;
      ch[t10-1]=wa2[i-2]*dr3-wa2[i-1]*di3;
      ch[t10]=wa2[i-2]*di3+wa2[i-1]*dr3;
    }
    t1+=ido;
  }
}

void dradb4(int ido,int l1,float *cc,float *ch,float *wa1,
              float *wa2,float *wa3){
  float sqrt2=1.414213562373095f;
  int i,k,t0,t1,t2,t3,t4,t5,t6,t7,t8;
  float ci2,ci3,ci4,cr2,cr3,cr4,ti1,ti2,ti3,ti4,tr1,tr2,tr3,tr4;
  t0=l1*ido;

  t1=0;
  t2=ido<<2;
  t3=0;
  t6=ido<<1;
  for(k=0;k<l1;k++){
    t4=t3+t6;
    t5=t1;
    tr3=cc[t4-1]+cc[t4-1];
    tr4=cc[t4]+cc[t4];
    tr1=cc[t3]-cc[(t4+=t6)-1];
    tr2=cc[t3]+cc[t4-1];
    ch[t5]=tr2+tr3;
    ch[t5+=t0]=tr1-tr4;
    ch[t5+=t0]=tr2-tr3;
    ch[t5+=t0]=tr1+tr4;
    t1+=ido;
    t3+=t2;
  }

  if(ido<2)return;
  if(ido==2)goto L105;

  t1=0;
  for(k=0;k<l1;k++){
    t5=(t4=(t3=(t2=t1<<2)+t6))+t6;
    t7=t1;
    for(i=2;i<ido;i+=2){
      t2+=2;
      t3+=2;
      t4-=2;
      t5-=2;
      t7+=2;
      ti1=cc[t2]+cc[t5];
      ti2=cc[t2]-cc[t5];
      ti3=cc[t3]-cc[t4];
      tr4=cc[t3]+cc[t4];
      tr1=cc[t2-1]-cc[t5-1];
      tr2=cc[t2-1]+cc[t5-1];
      ti4=cc[t3-1]-cc[t4-1];
      tr3=cc[t3-1]+cc[t4-1];
      ch[t7-1]=tr2+tr3;
      cr3=tr2-tr3;
      ch[t7]=ti2+ti3;
      ci3=ti2-ti3;
      cr2=tr1-tr4;
      cr4=tr1+tr4;
      ci2=ti1+ti4;
      ci4=ti1-ti4;

      ch[(t8=t7+t0)-1]=wa1[i-2]*cr2-wa1[i-1]*ci2;
      ch[t8]=wa1[i-2]*ci2+wa1[i-1]*cr2;
      ch[(t8+=t0)-1]=wa2[i-2]*cr3-wa2[i-1]*ci3;
      ch[t8]=wa2[i-2]*ci3+wa2[i-1]*cr3;
      ch[(t8+=t0)-1]=wa3[i-2]*cr4-wa3[i-1]*ci4;
      ch[t8]=wa3[i-2]*ci4+wa3[i-1]*cr4;
    }
    t1+=ido;
  }

  if(ido%2 == 1)return;

 L105:

  t1=ido;
  t2=ido<<2;
  t3=ido-1;
  t4=ido+(ido<<1);
  for(k=0;k<l1;k++){
    t5=t3;
    ti1=cc[t1]+cc[t4];
    ti2=cc[t4]-cc[t1];
    tr1=cc[t1-1]-cc[t4-1];
    tr2=cc[t1-1]+cc[t4-1];
    ch[t5]=tr2+tr2;
    ch[t5+=t0]=sqrt2*(tr1-ti1);
    ch[t5+=t0]=ti2+ti2;
    ch[t5+=t0]=-sqrt2*(tr1+ti1);

    t3+=ido;
    t1+=t2;
    t4+=t2;
  }
}

void dradbg(int ido,int ip,int l1,int idl1,float *cc,float *c1,
            float *c2,float *ch,float *ch2,float *wa){
  float tpi=6.283185307179586f;
  int idij,ipph,i,j,k,l,ik,is,t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,
      t11,t12;
  float dc2,ai1,ai2,ar1,ar2,ds2;
  int nbd;
  float dcp,arg,dsp,ar1h,ar2h;
  int ipp2;

  t10=ip*ido;
  t0=l1*ido;
  arg=tpi/(float)ip;
  dcp=(float)cos(arg);
  dsp=(float)sin(arg);
  nbd=(ido-1)>>1;
  ipp2=ip;
  ipph=(ip+1)>>1;
  if(ido<l1)goto L103;

  t1=0;
  t2=0;
  for(k=0;k<l1;k++){
    t3=t1;
    t4=t2;
    for(i=0;i<ido;i++){
      ch[t3]=cc[t4];
      t3++;
      t4++;
    }
    t1+=ido;
    t2+=t10;
  }
  goto L106;

 L103:
  t1=0;
  for(i=0;i<ido;i++){
    t2=t1;
    t3=t1;
    for(k=0;k<l1;k++){
      ch[t2]=cc[t3];
      t2+=ido;
      t3+=t10;
    }
    t1++;
  }

 L106:
  t1=0;
  t2=ipp2*t0;
  t7=(t5=ido<<1);
  for(j=1;j<ipph;j++){
    t1+=t0;
    t2-=t0;
    t3=t1;
    t4=t2;
    t6=t5;
    for(k=0;k<l1;k++){
      ch[t3]=cc[t6-1]+cc[t6-1];
      ch[t4]=cc[t6]+cc[t6];
      t3+=ido;
      t4+=ido;
      t6+=t10;
    }
    t5+=t7;
  }

  if (ido == 1)goto L116;
  if(nbd<l1)goto L112;

  t1=0;
  t2=ipp2*t0;
  t7=0;
  for(j=1;j<ipph;j++){
    t1+=t0;
    t2-=t0;
    t3=t1;
    t4=t2;

    t7+=(ido<<1);
    t8=t7;
    for(k=0;k<l1;k++){
      t5=t3;
      t6=t4;
      t9=t8;
      t11=t8;
      for(i=2;i<ido;i+=2){
        t5+=2;
        t6+=2;
        t9+=2;
        t11-=2;
        ch[t5-1]=cc[t9-1]+cc[t11-1];
        ch[t6-1]=cc[t9-1]-cc[t11-1];
        ch[t5]=cc[t9]-cc[t11];
        ch[t6]=cc[t9]+cc[t11];
      }
      t3+=ido;
      t4+=ido;
      t8+=t10;
    }
  }
  goto L116;

 L112:
  t1=0;
  t2=ipp2*t0;
  t7=0;
  for(j=1;j<ipph;j++){
    t1+=t0;
    t2-=t0;
    t3=t1;
    t4=t2;
    t7+=(ido<<1);
    t8=t7;
    t9=t7;
    for(i=2;i<ido;i+=2){
      t3+=2;
      t4+=2;
      t8+=2;
      t9-=2;
      t5=t3;
      t6=t4;
      t11=t8;
      t12=t9;
      for(k=0;k<l1;k++){
        ch[t5-1]=cc[t11-1]+cc[t12-1];
        ch[t6-1]=cc[t11-1]-cc[t12-1];
        ch[t5]=cc[t11]-cc[t12];
        ch[t6]=cc[t11]+cc[t12];
        t5+=ido;
        t6+=ido;
        t11+=t10;
        t12+=t10;
      }
    }
  }

L116:
  ar1=1.f;
  ai1=0.f;
  t1=0;
  t9=(t2=ipp2*idl1);
  t3=(ip-1)*idl1;
  for(l=1;l<ipph;l++){
    t1+=idl1;
    t2-=idl1;

    ar1h=dcp*ar1-dsp*ai1;
    ai1=dcp*ai1+dsp*ar1;
    ar1=ar1h;
    t4=t1;
    t5=t2;
    t6=0;
    t7=idl1;
    t8=t3;
    for(ik=0;ik<idl1;ik++){
      c2[t4++]=ch2[t6++]+ar1*ch2[t7++];
      c2[t5++]=ai1*ch2[t8++];
    }
    dc2=ar1;
    ds2=ai1;
    ar2=ar1;
    ai2=ai1;

    t6=idl1;
    t7=t9-idl1;
    for(j=2;j<ipph;j++){
      t6+=idl1;
      t7-=idl1;
      ar2h=dc2*ar2-ds2*ai2;
      ai2=dc2*ai2+ds2*ar2;
      ar2=ar2h;
      t4=t1;
      t5=t2;
      t11=t6;
      t12=t7;
      for(ik=0;ik<idl1;ik++){
        c2[t4++]+=ar2*ch2[t11++];
        c2[t5++]+=ai2*ch2[t12++];
      }
    }
  }

  t1=0;
  for(j=1;j<ipph;j++){
    t1+=idl1;
    t2=t1;
    for(ik=0;ik<idl1;ik++)ch2[ik]+=ch2[t2++];
  }

  t1=0;
  t2=ipp2*t0;
  for(j=1;j<ipph;j++){
    t1+=t0;
    t2-=t0;
    t3=t1;
    t4=t2;
    for(k=0;k<l1;k++){
      ch[t3]=c1[t3]-c1[t4];
      ch[t4]=c1[t3]+c1[t4];
      t3+=ido;
      t4+=ido;
    }
  }

  if(ido==1)goto L132;
  if(nbd<l1)goto L128;

  t1=0;
  t2=ipp2*t0;
  for(j=1;j<ipph;j++){
    t1+=t0;
    t2-=t0;
    t3=t1;
    t4=t2;
    for(k=0;k<l1;k++){
      t5=t3;
      t6=t4;
      for(i=2;i<ido;i+=2){
        t5+=2;
        t6+=2;
        ch[t5-1]=c1[t5-1]-c1[t6];
        ch[t6-1]=c1[t5-1]+c1[t6];
        ch[t5]=c1[t5]+c1[t6-1];
        ch[t6]=c1[t5]-c1[t6-1];
      }
      t3+=ido;
      t4+=ido;
    }
  }
  goto L132;

 L128:
  t1=0;
  t2=ipp2*t0;
  for(j=1;j<ipph;j++){
    t1+=t0;
    t2-=t0;
    t3=t1;
    t4=t2;
    for(i=2;i<ido;i+=2){
      t3+=2;
      t4+=2;
      t5=t3;
      t6=t4;
      for(k=0;k<l1;k++){
        ch[t5-1]=c1[t5-1]-c1[t6];
        ch[t6-1]=c1[t5-1]+c1[t6];
        ch[t5]=c1[t5]+c1[t6-1];
        ch[t6]=c1[t5]-c1[t6-1];
        t5+=ido;
        t6+=ido;
      }
    }
  }

L132:
  if(ido==1)return;

  for(ik=0;ik<idl1;ik++)c2[ik]=ch2[ik];

  t1=0;
  for(j=1;j<ip;j++){
    t2=(t1+=t0);
    for(k=0;k<l1;k++){
      c1[t2]=ch[t2];
      t2+=ido;
    }
  }

  if(nbd>l1)goto L139;

  is= -ido-1;
  t1=0;
  for(j=1;j<ip;j++){
    is+=ido;
    t1+=t0;
    idij=is;
    t2=t1;
    for(i=2;i<ido;i+=2){
      t2+=2;
      idij+=2;
      t3=t2;
      for(k=0;k<l1;k++){
        c1[t3-1]=wa[idij-1]*ch[t3-1]-wa[idij]*ch[t3];
        c1[t3]=wa[idij-1]*ch[t3]+wa[idij]*ch[t3-1];
        t3+=ido;
      }
    }
  }
  return;

 L139:
  is= -ido-1;
  t1=0;
  for(j=1;j<ip;j++){
    is+=ido;
    t1+=t0;
    t2=t1;
    for(k=0;k<l1;k++){
      idij=is;
      t3=t2;
      for(i=2;i<ido;i+=2){
        idij+=2;
        t3+=2;
        c1[t3-1]=wa[idij-1]*ch[t3-1]-wa[idij]*ch[t3];
        c1[t3]=wa[idij-1]*ch[t3]+wa[idij]*ch[t3-1];
      }
      t2+=ido;
    }
  }
}

void drftb1(int n, float *c, float *ch, float *wa, int *ifac){
  int i,k1,l1,l2;
  int na;
  int nf,ip,iw,ix2,ix3,ido,idl1;

  nf=ifac[1];
  na=0;
  l1=1;
  iw=1;

  for(k1=0;k1<nf;k1++){
    ip=ifac[k1 + 2];
    l2=ip*l1;
    ido=n/l2;
    idl1=ido*l1;
    if(ip!=4)goto L103;
    ix2=iw+ido;
    ix3=ix2+ido;

    if(na!=0)
      dradb4(ido,l1,ch,c,wa+iw-1,wa+ix2-1,wa+ix3-1);
    else
      dradb4(ido,l1,c,ch,wa+iw-1,wa+ix2-1,wa+ix3-1);
    na=1-na;
    goto L115;

  L103:
    if(ip!=2)goto L106;

    if(na!=0)
      dradb2(ido,l1,ch,c,wa+iw-1);
    else
      dradb2(ido,l1,c,ch,wa+iw-1);
    na=1-na;
    goto L115;

  L106:
    if(ip!=3)goto L109;

    ix2=iw+ido;
    if(na!=0)
      dradb3(ido,l1,ch,c,wa+iw-1,wa+ix2-1);
    else
      dradb3(ido,l1,c,ch,wa+iw-1,wa+ix2-1);
    na=1-na;
    goto L115;

  L109:
/*    The radix five case can be translated later..... */
/*    if(ip!=5)goto L112;

    ix2=iw+ido;
    ix3=ix2+ido;
    ix4=ix3+ido;
    if(na!=0)
      dradb5(ido,l1,ch,c,wa+iw-1,wa+ix2-1,wa+ix3-1,wa+ix4-1);
    else
      dradb5(ido,l1,c,ch,wa+iw-1,wa+ix2-1,wa+ix3-1,wa+ix4-1);
    na=1-na;
    goto L115;

  L112:*/
    if(na!=0)
      dradbg(ido,ip,l1,idl1,ch,ch,ch,c,c,wa+iw-1);
    else
      dradbg(ido,ip,l1,idl1,c,c,c,ch,ch,wa+iw-1);
    if(ido==1)na=1-na;

  L115:
    l1=l2;
    iw+=(ip-1)*ido;
  }

  if(na==0)return;

  for(i=0;i<n;i++)c[i]=ch[i];
}

void drft_forward(drft_lookup *l,float *data){
  if(l->n==1)return;
  drftf1(l->n,data,l->trigcache,l->trigcache+l->n,l->splitcache);
}

void drft_backward(drft_lookup *l,float *data){
  if (l->n==1)return;
  drftb1(l->n,data,l->trigcache,l->trigcache+l->n,l->splitcache);
}

void drft_init(drft_lookup *l,int n){
  l->n=n;
  l->trigcache=static_cast<float*>(_ogg_calloc(3*n,sizeof(*l->trigcache)));
  l->splitcache=static_cast<int*>(_ogg_calloc(32,sizeof(*l->splitcache)));
  fdrffti(n, l->trigcache, l->splitcache);
}

void drft_clear(drft_lookup *l){
  if(l){
    if(l->trigcache)_ogg_free(l->trigcache);
    if(l->splitcache)_ogg_free(l->splitcache);
    memset(l,0,sizeof(*l));
  }
}








// ==========================================================================
// ==========================================================================
// sharedbook.c
// ==========================================================================
// ==========================================================================

/**** pack/unpack helpers ******************************************/
int _ilog(unsigned int v){
  int ret=0;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

/* 32 bit float (not IEEE; nonnormalized mantissa +
   biased exponent) : neeeeeee eeemmmmm mmmmmmmm mmmmmmmm
   Why not IEEE?  It's just not that important here. */

#define VQ_FEXP 10
#define VQ_FMAN 21
#define VQ_FEXP_BIAS 768 /* bias toward values smaller than 1. */

/* doesn't currently guard under/overflow */
long _float32_pack(float val){
  int sign=0;
  long exp;
  long mant;
  if(val<0){
    sign=0x80000000;
    val= -val;
  }
  exp= (long)floor(log(val)/log(2.f));
  mant=(long)rint(ldexp(val,(VQ_FMAN-1)-exp));
  exp=(exp+VQ_FEXP_BIAS)<<VQ_FMAN;

  return(sign|exp|mant);
}

float _float32_unpack(long val){
  double mant=val&0x1fffff;
  int    sign=val&0x80000000;
  long   exp =(val&0x7fe00000L)>>VQ_FMAN;
  if(sign)mant= -mant;
  return (float)(ldexp(mant,exp-(VQ_FMAN-1)-VQ_FEXP_BIAS));
}

/* given a list of word lengths, generate a list of codewords.  Works
   for length ordered or unordered, always assigns the lowest valued
   codewords first.  Extended to handle unused entries (length 0) */
ogg_uint32_t *_make_words(long *l,long n,long sparsecount){
  long i,j,count=0;
  ogg_uint32_t marker[33];
  ogg_uint32_t *r=static_cast<ogg_uint32_t*>(_ogg_malloc((sparsecount?sparsecount:n)*sizeof(*r)));
  memset(marker,0,sizeof(marker));

  for(i=0;i<n;i++){
    long length=l[i];
    if(length>0){
      ogg_uint32_t entry=marker[length];

      /* when we claim a node for an entry, we also claim the nodes
     below it (pruning off the imagined tree that may have dangled
     from it) as well as blocking the use of any nodes directly
     above for leaves */

      /* update ourself */
      if(length<32 && (entry>>length)){
    /* error condition; the lengths must specify an overpopulated tree */
    _ogg_free(r);
    return(NULL);
      }
      r[count++]=entry;

      /* Look to see if the next shorter marker points to the node
     above. if so, update it and repeat.  */
      {
    for(j=length;j>0;j--){

      if(marker[j]&1){
        /* have to jump branches */
        if(j==1)
          marker[1]++;
        else
          marker[j]=marker[j-1]<<1;
        break; /* invariant says next upper marker would already
              have been moved if it was on the same path */
      }
      marker[j]++;
    }
      }

      /* prune the tree; the implicit invariant says all the longer
     markers were dangling from our just-taken node.  Dangle them
     from our *new* node. */
      for(j=length+1;j<33;j++)
    if((marker[j]>>1) == entry){
      entry=marker[j];
      marker[j]=marker[j-1]<<1;
    }else
      break;
    }else
      if(sparsecount==0)count++;
  }

  /* bitreverse the words because our bitwise packer/unpacker is LSb
     endian */
  for(i=0,count=0;i<n;i++){
    ogg_uint32_t temp=0;
    for(j=0;j<l[i];j++){
      temp<<=1;
      temp|=(r[count]>>j)&1;
    }

    if(sparsecount){
      if(l[i])
    r[count++]=temp;
    }else
      r[count++]=temp;
  }

  return(r);
}

/* there might be a straightforward one-line way to do the below
   that's portable and totally safe against roundoff, but I haven't
   thought of it.  Therefore, we opt on the side of caution */
long _book_maptype1_quantvals(const static_codebook *b){
  long vals=(long)floor(pow((float)b->entries,1.f/b->dim));

  /* the above *should* be reliable, but we'll not assume that FP is
     ever reliable when bitstream sync is at stake; verify via integer
     means that vals really is the greatest value of dim for which
     vals^b->bim <= b->entries */
  /* treat the above as an initial guess */
  for(;;){
    long acc=1;
    long acc1=1;
    int i;
    for(i=0;i<b->dim;i++){
      acc*=vals;
      acc1*=vals+1;
    }
    if(acc<=b->entries && acc1>b->entries){
      return(vals);
    }else{
      if(acc>b->entries){
    vals--;
      }else{
    vals++;
      }
    }
  }
}

/* unpack the quantized list of values for encode/decode ***********/
/* we need to deal with two map types: in map type 1, the values are
   generated algorithmically (each column of the vector counts through
   the values in the quant vector). in map type 2, all the values came
   in in an explicit list.  Both value lists must be unpacked */
float *_book_unquantize(const static_codebook *b,int n,int *sparsemap){
  long j,k,count=0;
  if(b->maptype==1 || b->maptype==2){
    int quantvals;
    float mindel=_float32_unpack(b->q_min);
    float delta=_float32_unpack(b->q_delta);
    float *r; if (n==0) r=NULL; // so that codeguard doesn't complain
    else r=static_cast<float*>(_ogg_calloc(n*b->dim,sizeof(*r)));

    /* maptype 1 and 2 both use a quantized value vector, but
       different sizes */
    switch(b->maptype){
    case 1:
      /* most of the time, entries%dimensions == 0, but we need to be
     well defined.  We define that the possible vales at each
     scalar is values == entries/dim.  If entries%dim != 0, we'll
     have 'too few' values (values*dim<entries), which means that
     we'll have 'left over' entries; left over entries use zeroed
     values (and are wasted).  So don't generate codebooks like
     that */
      quantvals=_book_maptype1_quantvals(b);
      for(j=0;j<b->entries;j++){
    if((sparsemap && b->lengthlist[j]) || !sparsemap){
      float last=0.f;
      int indexdiv=1;
      for(k=0;k<b->dim;k++){
        int index= (j/indexdiv)%quantvals;
        float val=(float)b->quantlist[index];
        val=(float)(fabs(val)*delta+mindel+last);
        if(b->q_sequencep)last=val;
        if(sparsemap)
          r[sparsemap[count]*b->dim+k]=val;
        else
          r[count*b->dim+k]=val;
        indexdiv*=quantvals;
      }
      count++;
    }

      }
      break;
    case 2:
      for(j=0;j<b->entries;j++){
    if((sparsemap && b->lengthlist[j]) || !sparsemap){
      float last=0.f;

      for(k=0;k<b->dim;k++){
        float val=(float)b->quantlist[j*b->dim+k];
        val=(float)(fabs(val)*delta+mindel+last);
        if(b->q_sequencep)last=val;
        if(sparsemap)
          r[sparsemap[count]*b->dim+k]=val;
        else
          r[count*b->dim+k]=val;
      }
      count++;
    }
      }
      break;
    }

    return(r);
  }
  return(NULL);
}

void vorbis_staticbook_clear(static_codebook *b){
  if(b->allocedp){
    if(b->quantlist)_ogg_free(b->quantlist);
    if(b->lengthlist)_ogg_free(b->lengthlist);
    if(b->nearest_tree){
      _ogg_free(b->nearest_tree->ptr0);
      _ogg_free(b->nearest_tree->ptr1);
      _ogg_free(b->nearest_tree->p);
      _ogg_free(b->nearest_tree->q);
      memset(b->nearest_tree,0,sizeof(*b->nearest_tree));
      _ogg_free(b->nearest_tree);
    }
    if(b->thresh_tree){
      _ogg_free(b->thresh_tree->quantthresh);
      _ogg_free(b->thresh_tree->quantmap);
      memset(b->thresh_tree,0,sizeof(*b->thresh_tree));
      _ogg_free(b->thresh_tree);
    }

    memset(b,0,sizeof(*b));
  }
}

void vorbis_staticbook_destroy(static_codebook *b){
  if(b->allocedp){
    vorbis_staticbook_clear(b);
    _ogg_free(b);
  }
}

void vorbis_book_clear(codebook *b){
  /* book is not cleared; we're likely called on the lookup and
     the codebook belongs to the info struct */
  if(b->valuelist)_ogg_free(b->valuelist);
  if(b->codelist)_ogg_free(b->codelist);

  if(b->dec_index)_ogg_free(b->dec_index);
  if(b->dec_codelengths)_ogg_free(b->dec_codelengths);
  if(b->dec_firsttable)_ogg_free(b->dec_firsttable);

  memset(b,0,sizeof(*b));
}

int vorbis_book_init_encode(codebook *c,const static_codebook *s){

  memset(c,0,sizeof(*c));
  c->c=s;
  c->entries=s->entries;
  c->used_entries=s->entries;
  c->dim=s->dim;
  c->codelist=_make_words(s->lengthlist,s->entries,0);
  c->valuelist=_book_unquantize(s,s->entries,NULL);

  return(0);
}


int sort32a(const void *a,const void *b){
  return ( **(ogg_uint32_t **)a>**(ogg_uint32_t **)b)-
    ( **(ogg_uint32_t **)a<**(ogg_uint32_t **)b);
}

/* decode codebook arrangement is more heavily optimized than encode */
int vorbis_book_init_decode(codebook *c,const static_codebook *s){
  int i,j,n=0,tabn;
  int *sortindex;
  memset(c,0,sizeof(*c));

  /* count actually used entries */
  for(i=0;i<s->entries;i++)
    if(s->lengthlist[i]>0)
      n++;

  c->entries=s->entries;
  c->used_entries=n;
  c->dim=s->dim;

  /* two different remappings go on here.

     First, we collapse the likely sparse codebook down only to
     actually represented values/words.  This collapsing needs to be
     indexed as map-valueless books are used to encode original entry
     positions as integers.

     Second, we reorder all vectors, including the entry index above,
     by sorted bitreversed codeword to allow treeless decode. */

  {
    /* perform sort */
    ogg_uint32_t *codes=_make_words(s->lengthlist,s->entries,c->used_entries);
    ogg_uint32_t **codep=static_cast<ogg_uint32_t**>(alloca(sizeof(*codep)*n));

    if(codes==NULL)goto err_out;

    for(i=0;i<n;i++){
      codes[i]=bitreverse(codes[i]);
      codep[i]=codes+i;
    }

    qsort(codep,n,sizeof(*codep),sort32a);

    sortindex=static_cast<int*>(alloca(n*sizeof(*sortindex)));
    c->codelist=static_cast<ogg_uint32_t*>(_ogg_malloc(n*sizeof(*c->codelist)));
    /* the index is a reverse index */
    for(i=0;i<n;i++){
      int position=(int)(codep[i]-codes);
      sortindex[position]=i;
    }

    for(i=0;i<n;i++)
      c->codelist[sortindex[i]]=codes[i];
    _ogg_free(codes);
  }

  c->valuelist=_book_unquantize(s,n,sortindex);
  c->dec_index=static_cast<int*>(_ogg_malloc(n*sizeof(*c->dec_index)));

  for(n=0,i=0;i<s->entries;i++)
    if(s->lengthlist[i]>0)
      c->dec_index[sortindex[n++]]=i;

  c->dec_codelengths=static_cast<char*>(_ogg_malloc(n*sizeof(*c->dec_codelengths)));
  for(n=0,i=0;i<s->entries;i++)
    if(s->lengthlist[i]>0)
      c->dec_codelengths[sortindex[n++]]=(char)s->lengthlist[i];

  c->dec_firsttablen=_ilog(c->used_entries)-4; /* this is magic */
  if(c->dec_firsttablen<5)c->dec_firsttablen=5;
  if(c->dec_firsttablen>8)c->dec_firsttablen=8;

  tabn=1<<c->dec_firsttablen;
  c->dec_firsttable=static_cast<ogg_uint32_t*>(_ogg_calloc(tabn,sizeof(*c->dec_firsttable)));
  c->dec_maxlength=0;

  for(i=0;i<n;i++){
    if(c->dec_maxlength<c->dec_codelengths[i])
      c->dec_maxlength=c->dec_codelengths[i];
    if(c->dec_codelengths[i]<=c->dec_firsttablen){
      ogg_uint32_t orig=bitreverse(c->codelist[i]);
      for(j=0;j<(1<<(c->dec_firsttablen-c->dec_codelengths[i]));j++)
    c->dec_firsttable[orig|(j<<c->dec_codelengths[i])]=i+1;
    }
  }

  /* now fill in 'unused' entries in the firsttable with hi/lo search
     hints for the non-direct-hits */
  {
    ogg_uint32_t mask=0xfffffffeUL<<(31-c->dec_firsttablen);
    long lo=0,hi=0;

    for(i=0;i<tabn;i++){
      ogg_uint32_t word=i<<(32-c->dec_firsttablen);
      if(c->dec_firsttable[bitreverse(word)]==0){
    while((lo+1)<n && c->codelist[lo+1]<=word)lo++;
    while(    hi<n && word>=(c->codelist[hi]&mask))hi++;

    /* we only actually have 15 bits per hint to play with here.
           In order to overflow gracefully (nothing breaks, efficiency
           just drops), encode as the difference from the extremes. */
    {
      unsigned long loval=lo;
      unsigned long hival=n-hi;

      if(loval>0x7fff)loval=0x7fff;
      if(hival>0x7fff)hival=0x7fff;
      c->dec_firsttable[bitreverse(word)]=
        0x80000000UL | (loval<<15) | hival;
    }
      }
    }
  }


  return(0);
 err_out:
  vorbis_book_clear(c);
  return(-1);
}

float _dist(int el,float *ref, float *b,int step){
  int i;
  float acc=0.f;
  for(i=0;i<el;i++){
    float val=(ref[i]-b[i*step]);
    acc+=val*val;
  }
  return(acc);
}

int _best(codebook *book, float *a, int step){
  encode_aux_threshmatch *tt=book->c->thresh_tree;

#if 0
  encode_aux_nearestmatch *nt=book->c->nearest_tree;
  encode_aux_pigeonhole *pt=book->c->pigeon_tree;
#endif
  int dim=book->dim;
  int k,o;
  /*int savebest=-1;
    float saverr;*/

  /* do we have a threshhold encode hint? */
  if(tt){
    int index=0,i;
    /* find the quant val of each scalar */
    for(k=0,o=step*(dim-1);k<dim;k++,o-=step){

      i=tt->threshvals>>1;
      if(a[o]<tt->quantthresh[i]){

    for(;i>0;i--)
      if(a[o]>=tt->quantthresh[i-1])
        break;

      }else{

    for(i++;i<tt->threshvals-1;i++)
      if(a[o]<tt->quantthresh[i])break;

      }

      index=(index*tt->quantvals)+tt->quantmap[i];
    }
    /* regular lattices are easy :-) */
    if(book->c->lengthlist[index]>0) /* is this unused?  If so, we'll
                    use a decision tree after all
                    and fall through*/
      return(index);
  }

#if 0
  /* do we have a pigeonhole encode hint? */
  if(pt){
    const static_codebook *c=book->c;
    int i,besti=-1;
    float best=0.f;
    int entry=0;

    /* dealing with sequentialness is a pain in the ass */
    if(c->q_sequencep){
      int pv;
      long mul=1;
      float qlast=0;
      for(k=0,o=0;k<dim;k++,o+=step){
    pv=(int)((a[o]-qlast-pt->min)/pt->del);
    if(pv<0 || pv>=pt->mapentries)break;
    entry+=pt->pigeonmap[pv]*mul;
    mul*=pt->quantvals;
    qlast+=pv*pt->del+pt->min;
      }
    }else{
      for(k=0,o=step*(dim-1);k<dim;k++,o-=step){
    int pv=(int)((a[o]-pt->min)/pt->del);
    if(pv<0 || pv>=pt->mapentries)break;
    entry=entry*pt->quantvals+pt->pigeonmap[pv];
      }
    }

    /* must be within the pigeonholable range; if we quant outside (or
       in an entry that we define no list for), brute force it */
    if(k==dim && pt->fitlength[entry]){
      /* search the abbreviated list */
      long *list=pt->fitlist+pt->fitmap[entry];
      for(i=0;i<pt->fitlength[entry];i++){
    float this=_dist(dim,book->valuelist+list[i]*dim,a,step);
    if(besti==-1 || this<best){
      best=this;
      besti=list[i];
    }
      }

      return(besti);
    }
  }

  if(nt){
    /* optimized using the decision tree */
    for(;;){
      float c=0.f;
      float *p=book->valuelist+nt->p[ptr];
      float *q=book->valuelist+nt->q[ptr];

      for(k=0,o=0;k<dim;k++,o+=step)
    c+=(p[k]-q[k])*(a[o]-(p[k]+q[k])*.5);

      if(c>0.f) /* in A */
    ptr= -nt->ptr0[ptr];
      else     /* in B */
    ptr= -nt->ptr1[ptr];
      if(ptr<=0)break;
    }
    return(-ptr);
  }
#endif

  /* brute force it! */
  {
    const static_codebook *c=book->c;
    int i,besti=-1;
    float best=0.f;
    float *e=book->valuelist;
    for(i=0;i<book->entries;i++){
      if(c->lengthlist[i]>0){
    float thiscp=_dist(dim,e,a,step);
    if(besti==-1 || thiscp<best){
      best=thiscp;
      besti=i;
    }
      }
      e+=dim;
    }

    /*if(savebest!=-1 && savebest!=besti){
      fprintf(stderr,"brute force/pigeonhole disagreement:\n"
          "original:");
      for(i=0;i<dim*step;i+=step)fprintf(stderr,"%g,",a[i]);
      fprintf(stderr,"\n"
          "pigeonhole (entry %d, err %g):",savebest,saverr);
      for(i=0;i<dim;i++)fprintf(stderr,"%g,",
                (book->valuelist+savebest*dim)[i]);
      fprintf(stderr,"\n"
          "bruteforce (entry %d, err %g):",besti,best);
      for(i=0;i<dim;i++)fprintf(stderr,"%g,",
                (book->valuelist+besti*dim)[i]);
      fprintf(stderr,"\n");
      }*/
    return(besti);
  }
}

long vorbis_book_codeword(codebook *book,int entry){
  if(book->c) /* only use with encode; decode optimizations are
                 allowed to break this */
    return book->codelist[entry];
  return -1;
}

long vorbis_book_codelen(codebook *book,int entry){
  if(book->c) /* only use with encode; decode optimizations are
                 allowed to break this */
    return book->c->lengthlist[entry];
  return -1;
}









// ==========================================================================
// ==========================================================================
// res0.c
// ==========================================================================
// ==========================================================================

/* Slow, slow, slow, simpleminded and did I mention it was slow?  The
   encode/decode loops are coded for clarity and performance is not
   yet even a nagging little idea lurking in the shadows.  Oh and BTW,
   it's slow. */



typedef struct {
  vorbis_info_residue0 *info;

  int         parts;
  int         stages;
  codebook   *fullbooks;
  codebook   *phrasebook;
  codebook ***partbooks;

  int         partvals;
  int       **decodemap;

  long      postbits;
  long      phrasebits;
  long      frames;

#ifdef TRAIN_RES
  int        train_seq;
  long      *training_data[8][64];
  float      training_max[8][64];
  float      training_min[8][64];
  float     tmin;
  float     tmax;
#endif

} vorbis_look_residue0;

void res0_free_info(vorbis_info_residue *i){
  vorbis_info_residue0 *info=(vorbis_info_residue0 *)i;
  if(info){
    memset(info,0,sizeof(*info));
    _ogg_free(info);
  }
}

void res0_free_look(vorbis_look_residue *i){
  int j;
  if(i){

    vorbis_look_residue0 *look=(vorbis_look_residue0 *)i;

#ifdef TRAIN_RES
    {
      int j,k,l;
      for(j=0;j<look->parts;j++){
    /*fprintf(stderr,"partition %d: ",j);*/
    for(k=0;k<8;k++)
      if(look->training_data[k][j]){
        char buffer[80];
        FILE *of;
        codebook *statebook=look->partbooks[j][k];

        /* long and short into the same bucket by current convention */
        sprintf(buffer,"res_part%d_pass%d.vqd",j,k);
        of=fopen(buffer,"a");

        for(l=0;l<statebook->entries;l++)
          fprintf(of,"%d:%ld\n",l,look->training_data[k][j][l]);

        fclose(of);

        /*fprintf(stderr,"%d(%.2f|%.2f) ",k,
          look->training_min[k][j],look->training_max[k][j]);*/

        _ogg_free(look->training_data[k][j]);
      }
    /*fprintf(stderr,"\n");*/
      }
    }
    fprintf(stderr,"min/max residue: %g::%g\n",look->tmin,look->tmax);

    /*fprintf(stderr,"residue bit usage %f:%f (%f total)\n",
        (float)look->phrasebits/look->frames,
        (float)look->postbits/look->frames,
        (float)(look->postbits+look->phrasebits)/look->frames);*/
#endif


    /*vorbis_info_residue0 *info=look->info;

    fprintf(stderr,
        "%ld frames encoded in %ld phrasebits and %ld residue bits "
        "(%g/frame) \n",look->frames,look->phrasebits,
        look->resbitsflat,
        (look->phrasebits+look->resbitsflat)/(float)look->frames);

    for(j=0;j<look->parts;j++){
      long acc=0;
      fprintf(stderr,"\t[%d] == ",j);
      for(k=0;k<look->stages;k++)
    if((info->secondstages[j]>>k)&1){
      fprintf(stderr,"%ld,",look->resbits[j][k]);
      acc+=look->resbits[j][k];
    }

      fprintf(stderr,":: (%ld vals) %1.2fbits/sample\n",look->resvals[j],
          acc?(float)acc/(look->resvals[j]*info->grouping):0);
    }
    fprintf(stderr,"\n");*/

    for(j=0;j<look->parts;j++)
      if(look->partbooks[j])_ogg_free(look->partbooks[j]);
    _ogg_free(look->partbooks);
    for(j=0;j<look->partvals;j++)
      _ogg_free(look->decodemap[j]);
    _ogg_free(look->decodemap);

    memset(look,0,sizeof(*look));
    _ogg_free(look);
  }
}


int icount(unsigned int v){
  int ret=0;
  while(v){
    ret+=v&1;
    v>>=1;
  }
  return(ret);
}


void res0_pack(vorbis_info_residue *vr,oggpack_buffer *opb){
  vorbis_info_residue0 *info=(vorbis_info_residue0 *)vr;
  int j,acc=0;
  oggpack_write(opb,info->begin,24);
  oggpack_write(opb,info->end,24);

  oggpack_write(opb,info->grouping-1,24);  /* residue vectors to group and
                         code with a partitioned book */
  oggpack_write(opb,info->partitions-1,6); /* possible partition choices */
  oggpack_write(opb,info->groupbook,8);  /* group huffman book */

  /* secondstages is a bitmask; as encoding progresses pass by pass, a
     bitmask of one indicates this partition class has bits to write
     this pass */
  for(j=0;j<info->partitions;j++){
    if(ilog(info->secondstages[j])>3){
      /* yes, this is a minor hack due to not thinking ahead */
      oggpack_write(opb,info->secondstages[j],3);
      oggpack_write(opb,1,1);
      oggpack_write(opb,info->secondstages[j]>>3,5);
    }else
      oggpack_write(opb,info->secondstages[j],4); /* trailing zero */
    acc+=icount(info->secondstages[j]);
  }
  for(j=0;j<acc;j++)
    oggpack_write(opb,info->booklist[j],8);

}

/* vorbis_info is for range checking */
vorbis_info_residue *res0_unpack(vorbis_info *vi,oggpack_buffer *opb){
  int j,acc=0;
  vorbis_info_residue0 *info=static_cast<vorbis_info_residue0*>(_ogg_calloc(1,sizeof(*info)));
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);

  info->begin=oggpack_read(opb,24);
  info->end=oggpack_read(opb,24);
  info->grouping=oggpack_read(opb,24)+1;
  info->partitions=oggpack_read(opb,6)+1;
  info->groupbook=oggpack_read(opb,8);

  for(j=0;j<info->partitions;j++){
    int cascade=oggpack_read(opb,3);
    if(oggpack_read(opb,1))
      cascade|=(oggpack_read(opb,5)<<3);
    info->secondstages[j]=cascade;

    acc+=icount(cascade);
  }
  for(j=0;j<acc;j++)
    info->booklist[j]=oggpack_read(opb,8);

  if(info->groupbook>=ci->books)goto errout;
  for(j=0;j<acc;j++)
    if(info->booklist[j]>=ci->books)goto errout;

  return(info);
 errout:
  res0_free_info(info);
  return(NULL);
}

vorbis_look_residue *res0_look(vorbis_dsp_state *vd,
                   vorbis_info_residue *vr){
  vorbis_info_residue0 *info=(vorbis_info_residue0 *)vr;
  vorbis_look_residue0 *look=static_cast<vorbis_look_residue0*>(_ogg_calloc(1,sizeof(*look)));
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vd->vi->codec_setup);

  int j,k,acc=0;
  int dim;
  int maxstage=0;
  look->info=info;

  look->parts=info->partitions;
  look->fullbooks=ci->fullbooks;
  look->phrasebook=ci->fullbooks+info->groupbook;
  dim=look->phrasebook->dim;

  look->partbooks=static_cast<codebook***>(_ogg_calloc(look->parts,sizeof(*look->partbooks)));

  for(j=0;j<look->parts;j++){
    int stages=ilog(info->secondstages[j]);
    if(stages){
      if(stages>maxstage)maxstage=stages;
      look->partbooks[j]=static_cast<codebook**>(_ogg_calloc(stages,sizeof(*look->partbooks[j])));
      for(k=0;k<stages;k++)
    if(info->secondstages[j]&(1<<k)){
      look->partbooks[j][k]=ci->fullbooks+info->booklist[acc++];
#ifdef TRAIN_RES
      look->training_data[k][j]=calloc(look->partbooks[j][k]->entries,
                       sizeof(***look->training_data));
#endif
    }
    }
  }

  look->partvals=(int)rint(pow((float)look->parts,(float)dim));
  look->stages=maxstage;
  look->decodemap=static_cast<int**>(_ogg_malloc(look->partvals*sizeof(*look->decodemap)));
  for(j=0;j<look->partvals;j++){
    long val=j;
    long mult=look->partvals/look->parts;
    look->decodemap[j]=static_cast<int*>(_ogg_malloc(dim*sizeof(*look->decodemap[j])));
    for(k=0;k<dim;k++){
      long deco=val/mult;
      val-=deco*mult;
      mult/=look->parts;
      look->decodemap[j][k]=deco;
    }
  }
#ifdef TRAIN_RES
  {
    int train_seq=0;
    look->train_seq=train_seq++;
  }
#endif
  return(look);
}

/* break an abstraction and copy some code for performance purposes */
int local_book_besterror(codebook *book,float *a){
  int dim=book->dim,i,k,o;
  int best=0;
  encode_aux_threshmatch *tt=book->c->thresh_tree;

  /* find the quant val of each scalar */
  for(k=0,o=dim;k<dim;++k){
    float val=a[--o];
    i=tt->threshvals>>1;

    if(val<tt->quantthresh[i]){
      if(val<tt->quantthresh[i-1]){
    for(--i;i>0;--i)
      if(val>=tt->quantthresh[i-1])
        break;
      }
    }else{

      for(++i;i<tt->threshvals-1;++i)
    if(val<tt->quantthresh[i])break;

    }

    best=(best*tt->quantvals)+tt->quantmap[i];
  }
  /* regular lattices are easy :-) */

  if(book->c->lengthlist[best]<=0){
    const static_codebook *c=book->c;
    int i,j;
    float bestf=0.f;
    float *e=book->valuelist;
    best=-1;
    for(i=0;i<book->entries;i++){
      if(c->lengthlist[i]>0){
    float thiscp=0.f;
    for(j=0;j<dim;j++){
      float val=(e[j]-a[j]);
      thiscp+=val*val;
    }
    if(best==-1 || thiscp<bestf){
      bestf=thiscp;
      best=i;
    }
      }
      e++;
    }
  }

  {
    float *ptr=book->valuelist+best*dim;
    for(i=0;i<dim;i++)
      *a++ -= *ptr++;
  }

  return(best);
}

int _encodepart(oggpack_buffer *opb,float *vec, int n,
               codebook *book,long * /*acc unused*/){
  int i,bits=0;
  int dim=book->dim;
  int step=n/dim;

  for(i=0;i<step;i++){
    int entry=local_book_besterror(book,vec+i*dim);

#ifdef TRAIN_RES
    acc[entry]++;
#endif

    bits+=vorbis_book_encode(book,entry,opb);
  }

  return(bits);
}

long **_01class(vorbis_block *vb,vorbis_look_residue *vl,
               float **in,int ch){
  long i,j,k;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;
  //vorbis_info           *vi=vb->vd->vi; unused
  //codec_setup_info      *ci=static_cast<codec_setup_info*>(vi->codec_setup); unused

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  long **partword=static_cast<long**>(_vorbis_block_alloc(vb,ch*sizeof(*partword)));
  float scale=(float)(100./samples_per_partition);

  /* we find the partition type for each partition of each
     channel.  We'll go back and do the interleaved encoding in a
     bit.  For now, clarity */

  for(i=0;i<ch;i++){
    partword[i]=static_cast<long*>(_vorbis_block_alloc(vb,n/samples_per_partition*sizeof(*partword[i])));
    memset(partword[i],0,n/samples_per_partition*sizeof(*partword[i]));
  }

  for(i=0;i<partvals;i++){
    int offset=i*samples_per_partition+info->begin;
    for(j=0;j<ch;j++){
      float max=0.;
      float ent=0.;
      for(k=0;k<samples_per_partition;k++){
    if(fabs(in[j][offset+k])>max) max=(float)fabs(in[j][offset+k]);
    ent+=(float)fabs(rint(in[j][offset+k]));
      }
      ent*=scale;

      for(k=0;k<possible_partitions-1;k++)
    if(max<=info->classmetric1[k] &&
       (info->classmetric2[k]<0 || (int)ent<info->classmetric2[k]))
      break;

      partword[j][i]=k;
    }
  }

#ifdef TRAIN_RESAUX
  {
    FILE *of;
    char buffer[80];

    for(i=0;i<ch;i++){
      sprintf(buffer,"resaux_%d.vqd",look->train_seq);
      of=fopen(buffer,"a");
      for(j=0;j<partvals;j++)
    fprintf(of,"%ld, ",partword[i][j]);
      fprintf(of,"\n");
      fclose(of);
    }
  }
#endif
  look->frames++;

  return(partword);
}

/* designed for stereo or other modes where the partition size is an
   integer multiple of the number of channels encoded in the current
   submap */
long **_2class(vorbis_block *vb,vorbis_look_residue *vl,float **in,
              int ch){
  long i,j,k,l;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  long **partword=static_cast<long**>(_vorbis_block_alloc(vb,sizeof(*partword)));

#ifdef TRAIN_RES
  FILE *of;
  char buffer[80];
#endif

  partword[0]=static_cast<long*>(_vorbis_block_alloc(vb,n*ch/samples_per_partition*sizeof(*partword[0])));
  memset(partword[0],0,n*ch/samples_per_partition*sizeof(*partword[0]));

  for(i=0,l=info->begin/ch;i<partvals;i++){
    float magmax=0.f;
    float angmax=0.f;
    for(j=0;j<samples_per_partition;j+=ch){
      if(fabs(in[0][l])>magmax) magmax=(float)fabs(in[0][l]);
      for(k=1;k<ch;k++)
    if(fabs(in[k][l])>angmax) angmax=(float)fabs(in[k][l]);
      l++;
    }

    for(j=0;j<possible_partitions-1;j++)
      if(magmax<=info->classmetric1[j] &&
     angmax<=info->classmetric2[j])
    break;

    partword[0][i]=j;

  }

#ifdef TRAIN_RESAUX
  sprintf(buffer,"resaux_%d.vqd",look->train_seq);
  of=fopen(buffer,"a");
  for(i=0;i<partvals;i++)
    fprintf(of,"%ld, ",partword[0][i]);
  fprintf(of,"\n");
  fclose(of);
#endif

  look->frames++;

  return(partword);
}

int _01forward(vorbis_block *vb,vorbis_look_residue *vl,
              float **in,int ch,
              long **partword,
              int (*encode)(oggpack_buffer *,float *,int,
                    codebook *,long *)){
  long i,j,k,s;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  //vorbis_dsp_state      *vd=vb->vd; unused

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int partitions_per_word=look->phrasebook->dim;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  long resbits[128];
  long resvals[128];

#ifdef TRAIN_RES
  for(i=0;i<ch;i++)
    for(j=info->begin;j<info->end;j++){
      if(in[i][j]>look->tmax)look->tmax=in[i][j];
      if(in[i][j]<look->tmin)look->tmin=in[i][j];
    }
#endif

  memset(resbits,0,sizeof(resbits));
  memset(resvals,0,sizeof(resvals));

  /* we code the partition words for each channel, then the residual
     words for a partition per channel until we've written all the
     residual words for that partition word.  Then write the next
     partition channel words... */

  for(s=0;s<look->stages;s++){

    for(i=0;i<partvals;){

      /* first we encode a partition codeword for each channel */
      if(s==0){
    for(j=0;j<ch;j++){
      long val=partword[j][i];
      for(k=1;k<partitions_per_word;k++){
        val*=possible_partitions;
        if(i+k<partvals)
          val+=partword[j][i+k];
      }

      /* training hack */
      if(val<look->phrasebook->entries)
        look->phrasebits+=vorbis_book_encode(look->phrasebook,val,&vb->opb);
#if 0 /*def TRAIN_RES*/
      else
        fprintf(stderr,"!");
#endif

    }
      }

      /* now we encode interleaved residual values for the partitions */
      for(k=0;k<partitions_per_word && i<partvals;k++,i++){
    long offset=i*samples_per_partition+info->begin;

    for(j=0;j<ch;j++){
      if(s==0)resvals[partword[j][i]]+=samples_per_partition;
      if(info->secondstages[partword[j][i]]&(1<<s)){
        codebook *statebook=look->partbooks[partword[j][i]][s];
        if(statebook){
          int ret;
          long *accumulator=NULL;

#ifdef TRAIN_RES
          accumulator=look->training_data[s][partword[j][i]];
          {
        int l;
        float *samples=in[j]+offset;
        for(l=0;l<samples_per_partition;l++){
          if(samples[l]<look->training_min[s][partword[j][i]])
            look->training_min[s][partword[j][i]]=samples[l];
          if(samples[l]>look->training_max[s][partword[j][i]])
            look->training_max[s][partword[j][i]]=samples[l];
        }
          }
#endif

          ret=encode(&vb->opb,in[j]+offset,samples_per_partition,
             statebook,accumulator);

          look->postbits+=ret;
          resbits[partword[j][i]]+=ret;
        }
      }
    }
      }
    }
  }

  /*{
    long total=0;
    long totalbits=0;
    fprintf(stderr,"%d :: ",vb->mode);
    for(k=0;k<possible_partitions;k++){
      fprintf(stderr,"%ld/%1.2g, ",resvals[k],(float)resbits[k]/resvals[k]);
      total+=resvals[k];
      totalbits+=resbits[k];
      }

    fprintf(stderr,":: %ld:%1.2g\n",total,(double)totalbits/total);
    }*/
  return(0);
}

/* a truncated packet here just means 'stop working'; it's not an error */
int _01inverse(vorbis_block *vb,vorbis_look_residue *vl,
              float **in,int ch,
              long (*decodepart)(codebook *, float *,
                     oggpack_buffer *,int)){

  long i,j,k,l,s;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int partitions_per_word=look->phrasebook->dim;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  int partwords=(partvals+partitions_per_word-1)/partitions_per_word;
  int ***partword=static_cast<int***>(alloca(ch*sizeof(*partword)));

  for(j=0;j<ch;j++)
    partword[j]=static_cast<int**>(_vorbis_block_alloc(vb,partwords*sizeof(*partword[j])));

  for(s=0;s<look->stages;s++){

    /* each loop decodes on partition codeword containing
       partitions_pre_word partitions */
    for(i=0,l=0;i<partvals;l++){
      if(s==0){
    /* fetch the partition word for each channel */
    for(j=0;j<ch;j++){
      int temp=vorbis_book_decode(look->phrasebook,&vb->opb);
      if(temp==-1)goto eopbreak;
      partword[j][l]=look->decodemap[temp];
      if(partword[j][l]==NULL)goto errout;
    }
      }

      /* now we decode residual values for the partitions */
      for(k=0;k<partitions_per_word && i<partvals;k++,i++)
    for(j=0;j<ch;j++){
      long offset=info->begin+i*samples_per_partition;
      if(info->secondstages[partword[j][l][k]]&(1<<s)){
        codebook *stagebook=look->partbooks[partword[j][l][k]][s];
        if(stagebook){
          if(decodepart(stagebook,in[j]+offset,&vb->opb,
                samples_per_partition)==-1)goto eopbreak;
        }
      }
    }
    }
  }

 errout:
 eopbreak:
  return(0);
}

#if 0
/* residue 0 and 1 are just slight variants of one another. 0 is
   interleaved, 1 is not */
long **res0_class(vorbis_block *vb,vorbis_look_residue *vl,
          float **in,int *nonzero,int ch){
  /* we encode only the nonzero parts of a bundle */
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      in[used++]=in[i];
  if(used)
    /*return(_01class(vb,vl,in,used,_interleaved_testhack));*/
    return(_01class(vb,vl,in,used));
  else
    return(0);
}

int res0_forward(vorbis_block *vb,vorbis_look_residue *vl,
         float **in,float **out,int *nonzero,int ch,
         long **partword){
  /* we encode only the nonzero parts of a bundle */
  int i,j,used=0,n=vb->pcmend/2;
  for(i=0;i<ch;i++)
    if(nonzero[i]){
      if(out)
    for(j=0;j<n;j++)
      out[i][j]+=in[i][j];
      in[used++]=in[i];
    }
  if(used){
    int ret=_01forward(vb,vl,in,used,partword,
              _interleaved_encodepart);
    if(out){
      used=0;
      for(i=0;i<ch;i++)
    if(nonzero[i]){
      for(j=0;j<n;j++)
        out[i][j]-=in[used][j];
      used++;
    }
    }
    return(ret);
  }else{
    return(0);
  }
}
#endif

int res0_inverse(vorbis_block *vb,vorbis_look_residue *vl,
         float **in,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      in[used++]=in[i];
  if(used)
    return(_01inverse(vb,vl,in,used,vorbis_book_decodevs_add));
  else
    return(0);
}

int res1_forward(vorbis_block *vb,vorbis_look_residue *vl,
         float **in,float **out,int *nonzero,int ch,
         long **partword){
  int i,j,used=0,n=vb->pcmend/2;
  for(i=0;i<ch;i++)
    if(nonzero[i]){
      if(out)
    for(j=0;j<n;j++)
      out[i][j]+=in[i][j];
      in[used++]=in[i];
    }

  if(used){
    int ret=_01forward(vb,vl,in,used,partword,_encodepart);
    if(out){
      used=0;
      for(i=0;i<ch;i++)
    if(nonzero[i]){
      for(j=0;j<n;j++)
        out[i][j]-=in[used][j];
      used++;
    }
    }
    return(ret);
  }else{
    return(0);
  }
}

long **res1_class(vorbis_block *vb,vorbis_look_residue *vl,
          float **in,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      in[used++]=in[i];
  if(used)
    return(_01class(vb,vl,in,used));
  else
    return(0);
}

int res1_inverse(vorbis_block *vb,vorbis_look_residue *vl,
         float **in,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      in[used++]=in[i];
  if(used)
    return(_01inverse(vb,vl,in,used,vorbis_book_decodev_add));
  else
    return(0);
}

long **res2_class(vorbis_block *vb,vorbis_look_residue *vl,
          float **in,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])used++;
  if(used)
    return(_2class(vb,vl,in,ch));
  else
    return(0);
}

/* res2 is slightly more different; all the channels are interleaved
   into a single vector and encoded. */

int res2_forward(vorbis_block *vb,vorbis_look_residue *vl,
         float **in,float **out,int *nonzero,int ch,
         long **partword){
  long i,j,k,n=vb->pcmend/2,used=0;

  /* don't duplicate the code; use a working vector hack for now and
     reshape ourselves into a single channel res1 */
  /* ugly; reallocs for each coupling pass :-( */
  float *work=static_cast<float*>(_vorbis_block_alloc(vb,ch*n*sizeof(*work)));
  for(i=0;i<ch;i++){
    float *pcm=in[i];
    if(nonzero[i])used++;
    for(j=0,k=i;j<n;j++,k+=ch)
      work[k]=pcm[j];
  }

  if(used){
    int ret=_01forward(vb,vl,&work,1,partword,_encodepart);
    /* update the sofar vector */
    if(out){
      for(i=0;i<ch;i++){
    float *pcm=in[i];
    float *sofar=out[i];
    for(j=0,k=i;j<n;j++,k+=ch)
      sofar[j]+=pcm[j]-work[k];

      }
    }
    return(ret);
  }else{
    return(0);
  }
}

/* duplicate code here as speed is somewhat more important */
int res2_inverse(vorbis_block *vb,vorbis_look_residue *vl,
         float **in,int *nonzero,int ch){
  long i,k,l,s;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int partitions_per_word=look->phrasebook->dim;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  int partwords=(partvals+partitions_per_word-1)/partitions_per_word;
  int **partword=static_cast<int**>(_vorbis_block_alloc(vb,partwords*sizeof(*partword)));

  for(i=0;i<ch;i++)if(nonzero[i])break;
  if(i==ch)return(0); /* no nonzero vectors */

  for(s=0;s<look->stages;s++){
    for(i=0,l=0;i<partvals;l++){

      if(s==0){
    /* fetch the partition word */
    int temp=vorbis_book_decode(look->phrasebook,&vb->opb);
    if(temp==-1)goto eopbreak;
    partword[l]=look->decodemap[temp];
    if(partword[l]==NULL)goto errout;
      }

      /* now we decode residual values for the partitions */
      for(k=0;k<partitions_per_word && i<partvals;k++,i++)
    if(info->secondstages[partword[l][k]]&(1<<s)){
      codebook *stagebook=look->partbooks[partword[l][k]][s];

      if(stagebook){
        if(vorbis_book_decodevv_add(stagebook,in,
                    i*samples_per_partition+info->begin,ch,
                    &vb->opb,samples_per_partition)==-1)
          goto eopbreak;
      }
    }
    }
  }

 errout:
 eopbreak:
  return(0);
}


vorbis_func_residue residue0_exportbundle={
  NULL,
  res0_unpack,
  res0_look,
  res0_free_info,
  res0_free_look,
  NULL,
  NULL,
  res0_inverse
};

vorbis_func_residue residue1_exportbundle={
  res0_pack,
  res0_unpack,
  res0_look,
  res0_free_info,
  res0_free_look,
  res1_class,
  res1_forward,
  res1_inverse
};

vorbis_func_residue residue2_exportbundle={
  res0_pack,
  res0_unpack,
  res0_look,
  res0_free_info,
  res0_free_look,
  res2_class,
  res2_forward,
  res2_inverse
};









// ==========================================================================
// ==========================================================================
// mdct.c
// ==========================================================================
// ==========================================================================

/********************************************************************
 function: normalized modified discrete cosine transform
           power of two length transform only [64 <= n ]
 last mod: $Id: mdct.c,v 1.32 2002/10/16 02:43:48 xiphmont Exp $

 Original algorithm adapted long ago from _The use of multirate filter
 banks for coding of high quality digital audio_, by T. Sporer,
 K. Brandenburg and B. Edler, collection of the European Signal
 Processing Conference (EUSIPCO), Amsterdam, June 1992, Vol.1, pp
 211-214

 The below code implements an algorithm that no longer looks much like
 that presented in the paper, but the basic structure remains if you
 dig deep enough to see it.

 This module DOES NOT INCLUDE code to generate/apply the window
 function.  Everybody has their own weird favorite including me... I
 happen to like the properties of y=sin(.5PI*sin^2(x)), but others may
 vehemently disagree.

 ********************************************************************/

/* this can also be run as an integer transform by uncommenting a
   define in mdct.h; the integerization is a first pass and although
   it's likely stable for Vorbis, the dynamic range is constrained and
   roundoff isn't done (so it's noisy).  Consider it functional, but
   only a starting point.  There's no point on a machine with an FPU */


/* build lookups for trig functions; also pre-figure scaling and
   some window function algebra. */

void mdct_init(mdct_lookup *lookup,int n){
  int   *bitrev=static_cast<int*>(_ogg_malloc(sizeof(*bitrev)*(n/4)));
  DATA_TYPE *T=static_cast<DATA_TYPE*>(_ogg_malloc(sizeof(*T)*(n+n/4)));

  int i;
  int n2=n>>1;
  lookup->log2n=(int)rint(log((float)n)/log(2.f));
  int log2n=lookup->log2n;
  lookup->n=n;
  lookup->trig=T;
  lookup->bitrev=bitrev;

/* trig lookups... */

  for(i=0;i<n/4;i++){
    T[i*2]=(float)(FLOAT_CONV(cos((M_PI/n)*(4*i))));
    T[i*2+1]=(float)(FLOAT_CONV(-sin((M_PI/n)*(4*i))));
    T[n2+i*2]=(float)(FLOAT_CONV(cos((M_PI/(2*n))*(2*i+1))));
    T[n2+i*2+1]=(float)(FLOAT_CONV(sin((M_PI/(2*n))*(2*i+1))));
  }
  for(i=0;i<n/8;i++){
    T[n+i*2]=(float)(FLOAT_CONV(cos((M_PI/n)*(4*i+2))*.5));
    T[n+i*2+1]=(float)(FLOAT_CONV(-sin((M_PI/n)*(4*i+2))*.5));
  }

  /* bitreverse lookup... */

  {
    int mask=(1<<(log2n-1))-1,i,j;
    int msb=1<<(log2n-2);
    for(i=0;i<n/8;i++){
      int acc=0;
      for(j=0;msb>>j;j++)
    if((msb>>j)&i)acc|=1<<j;
      bitrev[i*2]=((~acc)&mask)-1;
      bitrev[i*2+1]=acc;

    }
  }
  lookup->scale=FLOAT_CONV(4.f/n);
}

/* 8 point butterfly (in place, 4 register) */
STIN void mdct_butterfly_8(DATA_TYPE *x){
  REG_TYPE r0   = x[6] + x[2];
  REG_TYPE r1   = x[6] - x[2];
  REG_TYPE r2   = x[4] + x[0];
  REG_TYPE r3   = x[4] - x[0];

       x[6] = r0   + r2;
       x[4] = r0   - r2;

       r0   = x[5] - x[1];
       r2   = x[7] - x[3];
       x[0] = r1   + r0;
       x[2] = r1   - r0;

       r0   = x[5] + x[1];
       r1   = x[7] + x[3];
       x[3] = r2   + r3;
       x[1] = r2   - r3;
       x[7] = r1   + r0;
       x[5] = r1   - r0;

}

/* 16 point butterfly (in place, 4 register) */
STIN void mdct_butterfly_16(DATA_TYPE *x){
  REG_TYPE r0     = x[1]  - x[9];
  REG_TYPE r1     = x[0]  - x[8];

           x[8]  += x[0];
           x[9]  += x[1];
           x[0]   = MULT_NORM((r0   + r1) * cPI2_8);
           x[1]   = MULT_NORM((r0   - r1) * cPI2_8);

           r0     = x[3]  - x[11];
           r1     = x[10] - x[2];
           x[10] += x[2];
           x[11] += x[3];
           x[2]   = r0;
           x[3]   = r1;

           r0     = x[12] - x[4];
           r1     = x[13] - x[5];
           x[12] += x[4];
           x[13] += x[5];
           x[4]   = MULT_NORM((r0   - r1) * cPI2_8);
           x[5]   = MULT_NORM((r0   + r1) * cPI2_8);

           r0     = x[14] - x[6];
           r1     = x[15] - x[7];
           x[14] += x[6];
           x[15] += x[7];
           x[6]  = r0;
           x[7]  = r1;

       mdct_butterfly_8(x);
       mdct_butterfly_8(x+8);
}

/* 32 point butterfly (in place, 4 register) */
STIN void mdct_butterfly_32(DATA_TYPE *x){
  REG_TYPE r0     = x[30] - x[14];
  REG_TYPE r1     = x[31] - x[15];

           x[30] +=         x[14];
       x[31] +=         x[15];
           x[14]  =         r0;
       x[15]  =         r1;

           r0     = x[28] - x[12];
       r1     = x[29] - x[13];
           x[28] +=         x[12];
       x[29] +=         x[13];
           x[12]  = MULT_NORM( r0 * cPI1_8  -  r1 * cPI3_8 );
       x[13]  = MULT_NORM( r0 * cPI3_8  +  r1 * cPI1_8 );

           r0     = x[26] - x[10];
       r1     = x[27] - x[11];
       x[26] +=         x[10];
       x[27] +=         x[11];
       x[10]  = MULT_NORM(( r0  - r1 ) * cPI2_8);
       x[11]  = MULT_NORM(( r0  + r1 ) * cPI2_8);

       r0     = x[24] - x[8];
       r1     = x[25] - x[9];
       x[24] += x[8];
       x[25] += x[9];
       x[8]   = MULT_NORM( r0 * cPI3_8  -  r1 * cPI1_8 );
       x[9]   = MULT_NORM( r1 * cPI3_8  +  r0 * cPI1_8 );

       r0     = x[22] - x[6];
       r1     = x[7]  - x[23];
       x[22] += x[6];
       x[23] += x[7];
       x[6]   = r1;
       x[7]   = r0;

       r0     = x[4]  - x[20];
       r1     = x[5]  - x[21];
       x[20] += x[4];
       x[21] += x[5];
       x[4]   = MULT_NORM( r1 * cPI1_8  +  r0 * cPI3_8 );
       x[5]   = MULT_NORM( r1 * cPI3_8  -  r0 * cPI1_8 );

       r0     = x[2]  - x[18];
       r1     = x[3]  - x[19];
       x[18] += x[2];
       x[19] += x[3];
       x[2]   = MULT_NORM(( r1  + r0 ) * cPI2_8);
       x[3]   = MULT_NORM(( r1  - r0 ) * cPI2_8);

       r0     = x[0]  - x[16];
       r1     = x[1]  - x[17];
       x[16] += x[0];
       x[17] += x[1];
       x[0]   = MULT_NORM( r1 * cPI3_8  +  r0 * cPI1_8 );
       x[1]   = MULT_NORM( r1 * cPI1_8  -  r0 * cPI3_8 );

       mdct_butterfly_16(x);
       mdct_butterfly_16(x+16);

}

/* N point first stage butterfly (in place, 2 register) */
STLOOPIN void mdct_butterfly_first(DATA_TYPE *T,
                    DATA_TYPE *x,
                    int points){

  DATA_TYPE *x1        = x          + points      - 8;
  DATA_TYPE *x2        = x          + (points>>1) - 8;
  REG_TYPE   r0;
  REG_TYPE   r1;

  do{

               r0      = x1[6]      -  x2[6];
           r1      = x1[7]      -  x2[7];
           x1[6]  += x2[6];
           x1[7]  += x2[7];
           x2[6]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
           x2[7]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);

           r0      = x1[4]      -  x2[4];
           r1      = x1[5]      -  x2[5];
           x1[4]  += x2[4];
           x1[5]  += x2[5];
           x2[4]   = MULT_NORM(r1 * T[5]  +  r0 * T[4]);
           x2[5]   = MULT_NORM(r1 * T[4]  -  r0 * T[5]);

           r0      = x1[2]      -  x2[2];
           r1      = x1[3]      -  x2[3];
           x1[2]  += x2[2];
           x1[3]  += x2[3];
           x2[2]   = MULT_NORM(r1 * T[9]  +  r0 * T[8]);
           x2[3]   = MULT_NORM(r1 * T[8]  -  r0 * T[9]);

           r0      = x1[0]      -  x2[0];
           r1      = x1[1]      -  x2[1];
           x1[0]  += x2[0];
           x1[1]  += x2[1];
           x2[0]   = MULT_NORM(r1 * T[13] +  r0 * T[12]);
           x2[1]   = MULT_NORM(r1 * T[12] -  r0 * T[13]);

    x1-=8;
    x2-=8;
    T+=16;

  }while(x2>=x);
}

/* N/stage point generic N stage butterfly (in place, 2 register) */
STLOOPIN void mdct_butterfly_generic(DATA_TYPE *T,
                      DATA_TYPE *x,
                      int points,
                      int trigint){

  DATA_TYPE *x1        = x          + points      - 8;
  DATA_TYPE *x2        = x          + (points>>1) - 8;
  REG_TYPE   r0;
  REG_TYPE   r1;

  do{

               r0      = x1[6]      -  x2[6];
           r1      = x1[7]      -  x2[7];
           x1[6]  += x2[6];
           x1[7]  += x2[7];
           x2[6]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
           x2[7]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);

           T+=trigint;

           r0      = x1[4]      -  x2[4];
           r1      = x1[5]      -  x2[5];
           x1[4]  += x2[4];
           x1[5]  += x2[5];
           x2[4]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
           x2[5]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);

           T+=trigint;

           r0      = x1[2]      -  x2[2];
           r1      = x1[3]      -  x2[3];
           x1[2]  += x2[2];
           x1[3]  += x2[3];
           x2[2]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
           x2[3]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);

           T+=trigint;

           r0      = x1[0]      -  x2[0];
           r1      = x1[1]      -  x2[1];
           x1[0]  += x2[0];
           x1[1]  += x2[1];
           x2[0]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
           x2[1]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);

           T+=trigint;
    x1-=8;
    x2-=8;

  }while(x2>=x);
}

STLOOPIN void mdct_butterflies(mdct_lookup *init,
                 DATA_TYPE *x,
                 int points){

  DATA_TYPE *T=init->trig;
  int stages=init->log2n-5;
  int i,j;

  if(--stages>0){
    mdct_butterfly_first(T,x,points);
  }

  for(i=1;--stages>0;i++){
    for(j=0;j<(1<<i);j++)
      mdct_butterfly_generic(T,x+(points>>i)*j,points>>i,4<<i);
  }

  for(j=0;j<points;j+=32)
    mdct_butterfly_32(x+j);

}

void mdct_clear(mdct_lookup *l){
  if(l){
    if(l->trig)_ogg_free(l->trig);
    if(l->bitrev)_ogg_free(l->bitrev);
    memset(l,0,sizeof(*l));
  }
}

STLOOPIN void mdct_bitreverse(mdct_lookup *init,
                DATA_TYPE *x){
  int        n       = init->n;
  int       *bit     = init->bitrev;
  DATA_TYPE *w0      = x;
  DATA_TYPE *w1      = x = w0+(n>>1);
  DATA_TYPE *T       = init->trig+n;

  do{
    DATA_TYPE *x0    = x+bit[0];
    DATA_TYPE *x1    = x+bit[1];

    REG_TYPE  r0     = x0[1]  - x1[1];
    REG_TYPE  r1     = x0[0]  + x1[0];
    REG_TYPE  r2     = MULT_NORM(r1     * T[0]   + r0 * T[1]);
    REG_TYPE  r3     = MULT_NORM(r1     * T[1]   - r0 * T[0]);

          w1    -= 4;

              r0     = HALVE(x0[1] + x1[1]);
              r1     = HALVE(x0[0] - x1[0]);

          w0[0]  = r0     + r2;
          w1[2]  = r0     - r2;
          w0[1]  = r1     + r3;
          w1[3]  = r3     - r1;

              x0     = x+bit[2];
              x1     = x+bit[3];

              r0     = x0[1]  - x1[1];
              r1     = x0[0]  + x1[0];
              r2     = MULT_NORM(r1     * T[2]   + r0 * T[3]);
              r3     = MULT_NORM(r1     * T[3]   - r0 * T[2]);

              r0     = HALVE(x0[1] + x1[1]);
              r1     = HALVE(x0[0] - x1[0]);

          w0[2]  = r0     + r2;
          w1[0]  = r0     - r2;
          w0[3]  = r1     + r3;
          w1[1]  = r3     - r1;

          T     += 4;
          bit   += 4;
          w0    += 4;

  }while(w0<w1);
}

void mdct_backward(mdct_lookup *init, DATA_TYPE *in, DATA_TYPE *out){
  int n=init->n;
  int n2=n>>1;
  int n4=n>>2;

  /* rotate */

  DATA_TYPE *iX = in+n2-7;
  DATA_TYPE *oX = out+n2+n4;
  DATA_TYPE *T  = init->trig+n4;

  for(;;) {
    oX         -= 4;
    oX[0]       = MULT_NORM(-iX[2] * T[3] - iX[0]  * T[2]);
    oX[1]       = MULT_NORM (iX[0] * T[3] - iX[2]  * T[2]);
    oX[2]       = MULT_NORM(-iX[6] * T[1] - iX[4]  * T[0]);
    oX[3]       = MULT_NORM (iX[4] * T[1] - iX[6]  * T[0]);
    if (iX<in+8) break;
    iX-=8; T+=4;
  }

  iX            = in+n2-8;
  oX            = out+n2+n4;
  T             = init->trig+n4;

  for(;;) {
    T          -= 4;
    oX[0]       =  MULT_NORM (iX[4] * T[3] + iX[6] * T[2]);
    oX[1]       =  MULT_NORM (iX[4] * T[2] - iX[6] * T[3]);
    oX[2]       =  MULT_NORM (iX[0] * T[1] + iX[2] * T[0]);
    oX[3]       =  MULT_NORM (iX[0] * T[0] - iX[2] * T[1]);
    if (iX<in+8) break;
    iX         -= 8;
    oX         += 4;
  }

  mdct_butterflies(init,out+n2,n2);
  mdct_bitreverse(init,out);

  /* roatate + window */

  {
    DATA_TYPE *oX1=out+n2+n4;
    DATA_TYPE *oX2=out+n2+n4;
    DATA_TYPE *iX =out;
    T             =init->trig+n2;

    do{
      oX1-=4;

      oX1[3]  =  MULT_NORM (iX[0] * T[1] - iX[1] * T[0]);
      oX2[0]  = -MULT_NORM (iX[0] * T[0] + iX[1] * T[1]);

      oX1[2]  =  MULT_NORM (iX[2] * T[3] - iX[3] * T[2]);
      oX2[1]  = -MULT_NORM (iX[2] * T[2] + iX[3] * T[3]);

      oX1[1]  =  MULT_NORM (iX[4] * T[5] - iX[5] * T[4]);
      oX2[2]  = -MULT_NORM (iX[4] * T[4] + iX[5] * T[5]);

      oX1[0]  =  MULT_NORM (iX[6] * T[7] - iX[7] * T[6]);
      oX2[3]  = -MULT_NORM (iX[6] * T[6] + iX[7] * T[7]);

      oX2+=4;
      iX    +=   8;
      T     +=   8;
    }while(iX<oX1);

    iX=out+n2+n4;
    oX1=out+n4;
    oX2=oX1;

    do{
      oX1-=4;
      iX-=4;

      oX2[0] = -(oX1[3] = iX[3]);
      oX2[1] = -(oX1[2] = iX[2]);
      oX2[2] = -(oX1[1] = iX[1]);
      oX2[3] = -(oX1[0] = iX[0]);

      oX2+=4;
    }while(oX2<iX);

    iX=out+n2+n4;
    oX1=out+n2+n4;
    oX2=out+n2;
    do{
      oX1-=4;
      oX1[0]= iX[3];
      oX1[1]= iX[2];
      oX1[2]= iX[1];
      oX1[3]= iX[0];
      iX+=4;
    }while(oX1>oX2);
  }
}

void mdct_forward(mdct_lookup *init, DATA_TYPE *in, DATA_TYPE *out){
  int n=init->n;
  int n2=n>>1;
  int n4=n>>2;
  int n8=n>>3;
  DATA_TYPE *w=static_cast<DATA_TYPE*>(alloca(n*sizeof(*w))); /* forward needs working space */
  DATA_TYPE *w2=w+n2;

  /* rotate */

  /* window + rotate + step 1 */

  REG_TYPE r0;
  REG_TYPE r1;
  DATA_TYPE *x0=in+n2+n4;
  DATA_TYPE *x1=x0+1;
  DATA_TYPE *T=init->trig+n2;

  int i=0; i; // unused?

  for(i=0;i<n8;i+=2){
    x0 -=4;
    T-=2;
    r0= x0[2] + x1[0];
    r1= x0[0] + x1[2];
    w2[i]=   MULT_NORM(r1*T[1] + r0*T[0]);
    w2[i+1]= MULT_NORM(r1*T[0] - r0*T[1]);
    x1 +=4;
  }

  x1=in+1;

  for(;i<n2-n8;i+=2){
    T-=2;
    x0 -=4;
    r0= x0[2] - x1[0];
    r1= x0[0] - x1[2];
    w2[i]=   MULT_NORM(r1*T[1] + r0*T[0]);
    w2[i+1]= MULT_NORM(r1*T[0] - r0*T[1]);
    x1 +=4;
  }

  x0=in+n;

  for(;i<n2;i+=2){
    T-=2;
    x0 -=4;
    r0= -x0[2] - x1[0];
    r1= -x0[0] - x1[2];
    w2[i]=   MULT_NORM(r1*T[1] + r0*T[0]);
    w2[i+1]= MULT_NORM(r1*T[0] - r0*T[1]);
    x1 +=4;
  }


  mdct_butterflies(init,w+n2,n2);
  mdct_bitreverse(init,w);

  /* roatate + window */

  T=init->trig+n2;
  x0=out+n2;

  for(i=0;i<n4;i++){
    x0--;
    out[i] =MULT_NORM((w[0]*T[0]+w[1]*T[1])*init->scale);
    x0[0]  =MULT_NORM((w[0]*T[1]-w[1]*T[0])*init->scale);
    w+=2;
    T+=2;
  }
}











// ==========================================================================
// ==========================================================================
// lsp.c
// ==========================================================================
// ==========================================================================

/*
  function: LSP (also called LSF) conversion routines
  last mod: $Id: lsp.c,v 1.24 2002/10/16 07:44:21 xiphmont Exp $

  The LSP generation code is taken (with minimal modification and a
  few bugfixes) from "On the Computation of the LSP Frequencies" by
  Joseph Rothweiler (see http://www.rothweiler.us for contact info).
  The paper is available at:

  http://www.myown1.com/joe/lsf

 ********************************************************************/

/* Note that the lpc-lsp conversion finds the roots of polynomial with
   an iterative root polisher (CACM algorithm 283).  It *is* possible
   to confuse this algorithm into not converging; that should only
   happen with absurdly closely spaced roots (very sharp peaks in the
   LPC f response) which in turn should be impossible in our use of
   the code.  If this *does* happen anyway, it's a bug in the floor
   finder; find the cause of the confusion (probably a single bin
   spike or accidental near-float-limit resolution problems) and
   correct it. */


/* three possible LSP to f curve functions; the exact computation
   (float), a lookup based float implementation, and an integer
   implementation.  The float lookup is likely the optimal choice on
   any machine with an FPU.  The integer implementation is *not* fixed
   point (due to the need for a large dynamic range and thus a
   seperately tracked exponent) and thus much more complex than the
   relatively simple float implementations. It's mostly for future
   work on a fully fixed point implementation for processors like the
   ARM family. */


#define COS_LOOKUP_SZ 128
float COS_LOOKUP[COS_LOOKUP_SZ+1]={
    +1.0000000000000f,+0.9996988186962f,+0.9987954562052f,+0.9972904566787f,
    +0.9951847266722f,+0.9924795345987f,+0.9891765099648f,+0.9852776423889f,
    +0.9807852804032f,+0.9757021300385f,+0.9700312531945f,+0.9637760657954f,
    +0.9569403357322f,+0.9495281805930f,+0.9415440651830f,+0.9329927988347f,
    +0.9238795325113f,+0.9142097557035f,+0.9039892931234f,+0.8932243011955f,
    +0.8819212643484f,+0.8700869911087f,+0.8577286100003f,+0.8448535652497f,
    +0.8314696123025f,+0.8175848131516f,+0.8032075314806f,+0.7883464276266f,
    +0.7730104533627f,+0.7572088465065f,+0.7409511253550f,+0.7242470829515f,
    +0.7071067811865f,+0.6895405447371f,+0.6715589548470f,+0.6531728429538f,
    +0.6343932841636f,+0.6152315905806f,+0.5956993044924f,+0.5758081914178f,
    +0.5555702330196f,+0.5349976198871f,+0.5141027441932f,+0.4928981922298f,
    +0.4713967368260f,+0.4496113296546f,+0.4275550934303f,+0.4052413140050f,
    +0.3826834323651f,+0.3598950365350f,+0.3368898533922f,+0.3136817403989f,
    +0.2902846772545f,+0.2667127574749f,+0.2429801799033f,+0.2191012401569f,
    +0.1950903220161f,+0.1709618887603f,+0.1467304744554f,+0.1224106751992f,
    +0.0980171403296f,+0.0735645635997f,+0.0490676743274f,+0.0245412285229f,
    +0.0000000000000f,-0.0245412285229f,-0.0490676743274f,-0.0735645635997f,
    -0.0980171403296f,-0.1224106751992f,-0.1467304744554f,-0.1709618887603f,
    -0.1950903220161f,-0.2191012401569f,-0.2429801799033f,-0.2667127574749f,
    -0.2902846772545f,-0.3136817403989f,-0.3368898533922f,-0.3598950365350f,
    -0.3826834323651f,-0.4052413140050f,-0.4275550934303f,-0.4496113296546f,
    -0.4713967368260f,-0.4928981922298f,-0.5141027441932f,-0.5349976198871f,
    -0.5555702330196f,-0.5758081914178f,-0.5956993044924f,-0.6152315905806f,
    -0.6343932841636f,-0.6531728429538f,-0.6715589548470f,-0.6895405447371f,
    -0.7071067811865f,-0.7242470829515f,-0.7409511253550f,-0.7572088465065f,
    -0.7730104533627f,-0.7883464276266f,-0.8032075314806f,-0.8175848131516f,
    -0.8314696123025f,-0.8448535652497f,-0.8577286100003f,-0.8700869911087f,
    -0.8819212643484f,-0.8932243011955f,-0.9039892931234f,-0.9142097557035f,
    -0.9238795325113f,-0.9329927988347f,-0.9415440651830f,-0.9495281805930f,
    -0.9569403357322f,-0.9637760657954f,-0.9700312531945f,-0.9757021300385f,
    -0.9807852804032f,-0.9852776423889f,-0.9891765099648f,-0.9924795345987f,
    -0.9951847266722f,-0.9972904566787f,-0.9987954562052f,-0.9996988186962f,
    -1.0000000000000f,
};

#define INVSQ_LOOKUP_SZ 32
float INVSQ_LOOKUP[INVSQ_LOOKUP_SZ+1]={
    1.414213562373f,1.392621247646f,1.371988681140f,1.352246807566f,
    1.333333333333f,1.315191898443f,1.297771369046f,1.281025230441f,
    1.264911064067f,1.249390095109f,1.234426799697f,1.219988562661f,
    1.206045378311f,1.192569588000f,1.179535649239f,1.166919931983f,
    1.154700538379f,1.142857142857f,1.131370849898f,1.120224067222f,
    1.109400392450f,1.098884511590f,1.088662107904f,1.078719779941f,
    1.069044967650f,1.059625885652f,1.050451462878f,1.041511287847f,
    1.032795558989f,1.024295039463f,1.016001016002f,1.007905261358f,
    1.000000000000f,
};

#define INVSQ2EXP_LOOKUP_MIN -32
#define INVSQ2EXP_LOOKUP_MAX 32
float INVSQ2EXP_LOOKUP[INVSQ2EXP_LOOKUP_MAX-\
                              INVSQ2EXP_LOOKUP_MIN+1]={
             65536.f,    46340.95001f,         32768.f,    23170.47501f,
             16384.f,     11585.2375f,          8192.f,    5792.618751f,
              4096.f,    2896.309376f,          2048.f,    1448.154688f,
              1024.f,    724.0773439f,           512.f,     362.038672f,
               256.f,     181.019336f,           128.f,    90.50966799f,
                64.f,      45.254834f,            32.f,      22.627417f,
                16.f,     11.3137085f,             8.f,    5.656854249f,
                 4.f,    2.828427125f,             2.f,    1.414213562f,
                 1.f,   0.7071067812f,            0.5f,   0.3535533906f,
               0.25f,   0.1767766953f,          0.125f,  0.08838834765f,
             0.0625f,  0.04419417382f,        0.03125f,  0.02209708691f,
           0.015625f,  0.01104854346f,      0.0078125f, 0.005524271728f,
         0.00390625f, 0.002762135864f,    0.001953125f, 0.001381067932f,
       0.0009765625f, 0.000690533966f,  0.00048828125f, 0.000345266983f,
     0.000244140625f,0.0001726334915f,0.0001220703125f,8.631674575e-05f,
    6.103515625e-05f,4.315837288e-05f,3.051757812e-05f,2.157918644e-05f,
    1.525878906e-05f,
};


#define FROMdB_LOOKUP_SZ 35
#define FROMdB2_LOOKUP_SZ 32
#define FROMdB_SHIFT 5
#define FROMdB2_SHIFT 3
#define FROMdB2_MASK 31
float FROMdB_LOOKUP[FROMdB_LOOKUP_SZ]={
                 1.f,   0.6309573445f,   0.3981071706f,   0.2511886432f,
       0.1584893192f,            0.1f,  0.06309573445f,  0.03981071706f,
      0.02511886432f,  0.01584893192f,           0.01f, 0.006309573445f,
     0.003981071706f, 0.002511886432f, 0.001584893192f,          0.001f,
    0.0006309573445f,0.0003981071706f,0.0002511886432f,0.0001584893192f,
             0.0001f,6.309573445e-05f,3.981071706e-05f,2.511886432e-05f,
    1.584893192e-05f,          1e-05f,6.309573445e-06f,3.981071706e-06f,
    2.511886432e-06f,1.584893192e-06f,          1e-06f,6.309573445e-07f,
    3.981071706e-07f,2.511886432e-07f,1.584893192e-07f,
};

float FROMdB2_LOOKUP[FROMdB2_LOOKUP_SZ]={
       0.9928302478f,   0.9786445908f,   0.9646616199f,   0.9508784391f,
       0.9372921937f,     0.92390007f,   0.9106992942f,   0.8976871324f,
       0.8848608897f,   0.8722179097f,   0.8597555737f,   0.8474713009f,
        0.835362547f,   0.8234268041f,   0.8116616003f,   0.8000644989f,
       0.7886330981f,   0.7773650302f,   0.7662579617f,    0.755309592f,
       0.7445176537f,   0.7338799116f,   0.7233941627f,   0.7130582353f,
       0.7028699885f,   0.6928273125f,   0.6829281272f,   0.6731703824f,
       0.6635520573f,   0.6540711597f,   0.6447257262f,   0.6355138211f,
};


/* interpolated lookup based cos function, domain 0 to PI only */
float vorbis_coslook(float a){
  double d=a*(.31830989*(float)COS_LOOKUP_SZ);
  int i=vorbis_ftoi(d-.5);

  return (float)(COS_LOOKUP[i]+ (d-i)*(COS_LOOKUP[i+1]-COS_LOOKUP[i]));
}

/* interpolated 1./sqrt(p) where .5 <= p < 1. */
float vorbis_invsqlook(float a){
  double d=a*(2.f*(float)INVSQ_LOOKUP_SZ)-(float)INVSQ_LOOKUP_SZ;
  int i=vorbis_ftoi(d-.5f);
  return (float)(INVSQ_LOOKUP[i]+ (d-i)*(INVSQ_LOOKUP[i+1]-INVSQ_LOOKUP[i]));
}

/* interpolated 1./sqrt(p) where .5 <= p < 1. */
float vorbis_invsq2explook(int a){
  return INVSQ2EXP_LOOKUP[a-INVSQ2EXP_LOOKUP_MIN];
}

/* interpolated lookup based fromdB function, domain -140dB to 0dB only */
float vorbis_fromdBlook(float a){
  int i=vorbis_ftoi(a*((float)(-(1<<FROMdB2_SHIFT)))-.5f);
  return (i<0)?1.f:
    ((i>=(FROMdB_LOOKUP_SZ<<FROMdB_SHIFT))?0.f:
     FROMdB_LOOKUP[i>>FROMdB_SHIFT]*FROMdB2_LOOKUP[i&FROMdB2_MASK]);
}


/* side effect: changes *lsp to cosines of lsp */
void vorbis_lsp_to_curve(float *curve,int *map,int n,int ln,float *lsp,int m,
                float amp,float ampoffset){
  int i;
  float wdel=M_PI/ln;
  vorbis_fpu_control fpu; fpu=0; fpu; // some silly code to get rid of the "unused" compiler warning

  vorbis_fpu_setround(&fpu);
  for(i=0;i<m;i++)lsp[i]=vorbis_coslook(lsp[i]);

  i=0;
  while(i<n){
    int k=map[i];
    int qexp;
    float p=.7071067812f;
    float q=.7071067812f;
    float w=vorbis_coslook(wdel*k);
    float *ftmp=lsp;
    int c=m>>1;

    do{
      q*=ftmp[0]-w;
      p*=ftmp[1]-w;
      ftmp+=2;
    }while(--c);

    if(m&1){
      /* odd order filter; slightly assymetric */
      /* the last coefficient */
      q*=ftmp[0]-w;
      q*=q;
      p*=p*(1.f-w*w);
    }else{
      /* even order filter; still symmetric */
      q*=q*(1.f+w);
      p*=p*(1.f-w);
    }

    q=(float)frexp(p+q,&qexp);
    q=vorbis_fromdBlook(amp*
            vorbis_invsqlook(q)*
            vorbis_invsq2explook(qexp+m)-
            ampoffset);

    do{
      curve[i++]*=q;
    }while(map[i]==k);
  }
  vorbis_fpu_restore(fpu);
}


void cheby(float *g, int ord) {
  int i, j;

  g[0] *= .5f;
  for(i=2; i<= ord; i++) {
    for(j=ord; j >= i; j--) {
      g[j-2] -= g[j];
      g[j] += g[j];
    }
  }
}

int comp(const void *a,const void *b){
  return (*(float *)a<*(float *)b)-(*(float *)a>*(float *)b);
}

/* Newton-Raphson-Maehly actually functioned as a decent root finder,
   but there are root sets for which it gets into limit cycles
   (exacerbated by zero suppression) and fails.  We can't afford to
   fail, even if the failure is 1 in 100,000,000, so we now use
   Laguerre and later polish with Newton-Raphson (which can then
   afford to fail) */

#define EPSILON 10e-7
int Laguerre_With_Deflation(float *a,int ord,float *r){
  int i,m;
  //double lastdelta=0.f;
  double *defl=static_cast<double*>(alloca(sizeof(*defl)*(ord+1)));
  for(i=0;i<=ord;i++)defl[i]=a[i];

  for(m=ord;m>0;m--){
    double newcp=0.f,delta;

    /* iterate a root */
    for(;;){
      double p=defl[m],pp=0.f,ppp=0.f,denom;

      /* eval the polynomial and its first two derivatives */
      for(i=m;i>0;i--){
    ppp = newcp*ppp + pp;
    pp  = newcp*pp  + p;
    p   = newcp*p   + defl[i-1];
      }

      /* Laguerre's method */
      denom=(m-1) * ((m-1)*pp*pp - m*p*ppp);
      if(denom<0)
    return(-1);  /* complex root!  The LPC generator handed us a bad filter */

      if(pp>0){
    denom = pp + sqrt(denom);
    if(denom<EPSILON)denom=EPSILON;
      }else{
    denom = pp - sqrt(denom);
    if(denom>-(EPSILON))denom=-(EPSILON);
      }

      delta  = m*p/denom;
      newcp   -= delta;

      if(delta<0.f)delta*=-1;

      if(fabs(delta/newcp)<10e-12)break;
      //lastdelta=delta;
    }

    r[m-1]=(float)newcp;

    /* forward deflation */

    for(i=m;i>0;i--)
      defl[i-1]+=newcp*defl[i];
    defl++;

  }
  return(0);
}


/* for spit-and-polish only */
int Newton_Raphson(float *a,int ord,float *r){
  int i, k, count=0;
  double error=1.f;
  double *root=static_cast<double*>(alloca(ord*sizeof(*root)));

  for(i=0; i<ord;i++) root[i] = r[i];

  while(error>1e-20){
    error=0;

    for(i=0; i<ord; i++) { /* Update each point. */
      double pp=0.,delta;
      double rooti=root[i];
      double p=a[ord];
      for(k=ord-1; k>= 0; k--) {

    pp= pp* rooti + p;
    p = p * rooti + a[k];
      }

      delta = p/pp;
      root[i] -= delta;
      error+= delta*delta;
    }

    if(count>40)return(-1);

    count++;
  }

  /* Replaced the original bubble sort with a real sort.  With your
     help, we can eliminate the bubble sort in our lifetime. --Monty */

  for(i=0; i<ord;i++) r[i] = (float)root[i];
  return(0);
}


/* Convert lpc coefficients to lsp coefficients */
int vorbis_lpc_to_lsp(float *lpc,float *lsp,int m){
  int order2=(m+1)>>1;
  int g1_order,g2_order;
  float *g1=static_cast<float*>(alloca(sizeof(*g1)*(order2+1)));
  float *g2=static_cast<float*>(alloca(sizeof(*g2)*(order2+1)));
  float *g1r=static_cast<float*>(alloca(sizeof(*g1r)*(order2+1)));
  float *g2r=static_cast<float*>(alloca(sizeof(*g2r)*(order2+1)));
  int i;

  /* even and odd are slightly different base cases */
  g1_order=(m+1)>>1;
  g2_order=(m)  >>1;

  /* Compute the lengths of the x polynomials. */
  /* Compute the first half of K & R F1 & F2 polynomials. */
  /* Compute half of the symmetric and antisymmetric polynomials. */
  /* Remove the roots at +1 and -1. */

  g1[g1_order] = 1.f;
  for(i=1;i<=g1_order;i++) g1[g1_order-i] = lpc[i-1]+lpc[m-i];
  g2[g2_order] = 1.f;
  for(i=1;i<=g2_order;i++) g2[g2_order-i] = lpc[i-1]-lpc[m-i];

  if(g1_order>g2_order){
    for(i=2; i<=g2_order;i++) g2[g2_order-i] += g2[g2_order-i+2];
  }else{
    for(i=1; i<=g1_order;i++) g1[g1_order-i] -= g1[g1_order-i+1];
    for(i=1; i<=g2_order;i++) g2[g2_order-i] += g2[g2_order-i+1];
  }

  /* Convert into polynomials in cos(alpha) */
  cheby(g1,g1_order);
  cheby(g2,g2_order);

  /* Find the roots of the 2 even polynomials.*/
  if(Laguerre_With_Deflation(g1,g1_order,g1r) ||
     Laguerre_With_Deflation(g2,g2_order,g2r))
    return(-1);

  Newton_Raphson(g1,g1_order,g1r); /* if it fails, it leaves g1r alone */
  Newton_Raphson(g2,g2_order,g2r); /* if it fails, it leaves g2r alone */

  qsort(g1r,g1_order,sizeof(*g1r),comp);
  qsort(g2r,g2_order,sizeof(*g2r),comp);

  for(i=0;i<g1_order;i++)
    lsp[i*2] = (float)acos(g1r[i]);

  for(i=0;i<g2_order;i++)
    lsp[i*2+1] = (float)acos(g2r[i]);
  return(0);
}








// ==========================================================================
// ==========================================================================
// lpc.c
// ==========================================================================
// ==========================================================================

/* Autocorrelation LPC coeff generation algorithm invented by
   N. Levinson in 1947, modified by J. Durbin in 1959. */

/* Input : n elements of time doamin data
   Output: m lpc coefficients, excitation energy */

float vorbis_lpc_from_data(float *data,float *lpc,int n,int m){
  float *aut=static_cast<float*>(alloca(sizeof(*aut)*(m+1)));
  float error;
  int i,j;

  /* autocorrelation, p+1 lag coefficients */

  j=m+1;
  while(j--){
    double d=0; /* double needed for accumulator depth */
    for(i=j;i<n;i++)d+=data[i]*data[i-j];
    aut[j]=(float)d;
  }

  /* Generate lpc coefficients from autocorr values */

  error=aut[0];

  for(i=0;i<m;i++){
    float r= -aut[i+1];

    if(error==0){
      memset(lpc,0,m*sizeof(*lpc));
      return 0;
    }

    /* Sum up this iteration's reflection coefficient; note that in
       Vorbis we don't save it.  If anyone wants to recycle this code
       and needs reflection coefficients, save the results of 'r' from
       each iteration. */

    for(j=0;j<i;j++)r-=lpc[j]*aut[i-j];
    r/=error;

    /* Update LPC coefficients and total error */

    lpc[i]=r;
    for(j=0;j<i/2;j++){
      float tmp=lpc[j];
      lpc[j]+=r*lpc[i-1-j];
      lpc[i-1-j]+=r*tmp;
    }
    if(i%2)lpc[j]+=lpc[j]*r;

    error*=1.f-r*r;
  }

  /* we need the error value to know how big an impulse to hit the
     filter with later */

  return error;
}

/* Input : n element envelope spectral curve
   Output: m lpc coefficients, excitation energy */

float vorbis_lpc_from_curve(float *curve,float *lpc,lpc_lookup *l){
  int n=l->ln;
  int m=l->m;
  float *work=static_cast<float*>(alloca(sizeof(*work)*(n+n)));
  float fscale=.5f/n;
  int i,j;

  /* input is a real curve. make it complex-real */
  /* This mixes phase, but the LPC generation doesn't care. */
  for(i=0;i<n;i++){
    work[i*2]=curve[i]*fscale;
    work[i*2+1]=0;
  }
  work[n*2-1]=curve[n-1]*fscale;

  n*=2;
  drft_backward(&l->fft,work);

  /* The autocorrelation will not be circular.  Shift, else we lose
     most of the power in the edges. */

  for(i=0,j=n/2;i<n/2;){
    float temp=work[i];
    work[i++]=work[j];
    work[j++]=temp;
  }

  /* we *could* shave speed here by skimping on the edges (thus
     speeding up the autocorrelation in vorbis_lpc_from_data) but we
     don't right now. */

  return(vorbis_lpc_from_data(work,lpc,n,m));
}

void lpc_init(lpc_lookup *l,long mapped, int m){
  memset(l,0,sizeof(*l));

  l->ln=mapped;
  l->m=m;

  /* we cheat decoding the LPC spectrum via FFTs */
  drft_init(&l->fft,mapped*2);

}

void lpc_clear(lpc_lookup *l){
  if(l){
    drft_clear(&l->fft);
  }
}

void vorbis_lpc_predict(float *coeff,float *prime,int m,
                     float *data,long n){

  /* in: coeff[0...m-1] LPC coefficients
         prime[0...m-1] initial values (allocated size of n+m-1)
    out: data[0...n-1] data samples */

  long i,j,o,p;
  float y;
  float *work=static_cast<float*>(alloca(sizeof(*work)*(m+n)));

  if(!prime)
    for(i=0;i<m;i++)
      work[i]=0.f;
  else
    for(i=0;i<m;i++)
      work[i]=prime[i];

  for(i=0;i<n;i++){
    y=0;
    o=i;
    p=m;
    for(j=0;j<m;j++)
      y-=work[o++]*coeff[--p];

    data[i]=work[o]=y;
  }
}
















// ==========================================================================
// ==========================================================================
// floor1.c
// ==========================================================================
// ==========================================================================

#define floor1_rangedB 140 /* floor 1 fixed at -140dB to 0dB range */

typedef struct {
  int sorted_index[VIF_POSIT+2];
  int forward_index[VIF_POSIT+2];
  int reverse_index[VIF_POSIT+2];

  int hineighbor[VIF_POSIT];
  int loneighbor[VIF_POSIT];
  int posts;

  int n;
  int quant_q;
  vorbis_info_floor1 *vi;

  long phrasebits;
  long postbits;
  long frames;
} vorbis_look_floor1;

typedef struct lsfit_acc{
  long x0;
  long x1;

  long xa;
  long ya;
  long x2a;
  long y2a;
  long xya;
  long n;
  long an;
} lsfit_acc;

/***********************************************/

void floor1_free_info(vorbis_info_floor *i){
  vorbis_info_floor1 *info=(vorbis_info_floor1 *)i;
  if(info){
    memset(info,0,sizeof(*info));
    _ogg_free(info);
  }
}

void floor1_free_look(vorbis_look_floor *i){
  vorbis_look_floor1 *look=(vorbis_look_floor1 *)i;
  if(look){
    /*fprintf(stderr,"floor 1 bit usage %f:%f (%f total)\n",
        (float)look->phrasebits/look->frames,
        (float)look->postbits/look->frames,
        (float)(look->postbits+look->phrasebits)/look->frames);*/

    memset(look,0,sizeof(*look));
    _ogg_free(look);
  }
}



void floor1_pack (vorbis_info_floor *i,oggpack_buffer *opb){
  vorbis_info_floor1 *info=(vorbis_info_floor1 *)i;
  int j,k;
  int count=0;
  int rangebits;
  int maxposit=info->postlist[1];
  int maxclass=-1;

  /* save out partitions */
  oggpack_write(opb,info->partitions,5); /* only 0 to 31 legal */
  for(j=0;j<info->partitions;j++){
    oggpack_write(opb,info->partitionclass[j],4); /* only 0 to 15 legal */
    if(maxclass<info->partitionclass[j])maxclass=info->partitionclass[j];
  }

  /* save out partition classes */
  for(j=0;j<maxclass+1;j++){
    oggpack_write(opb,info->class_dim[j]-1,3); /* 1 to 8 */
    oggpack_write(opb,info->class_subs[j],2); /* 0 to 3 */
    if(info->class_subs[j])oggpack_write(opb,info->class_book[j],8);
    for(k=0;k<(1<<info->class_subs[j]);k++)
      oggpack_write(opb,info->class_subbook[j][k]+1,8);
  }

  /* save out the post list */
  oggpack_write(opb,info->mult-1,2);     /* only 1,2,3,4 legal now */
  oggpack_write(opb,ilog2(maxposit),4);
  rangebits=ilog2(maxposit);

  for(j=0,k=0;j<info->partitions;j++){
    count+=info->class_dim[info->partitionclass[j]];
    for(;k<count;k++)
      oggpack_write(opb,info->postlist[k+2],rangebits);
  }
}


vorbis_info_floor *floor1_unpack (vorbis_info *vi,oggpack_buffer *opb){
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  int j,k,count=0,maxclass=-1,rangebits;

  vorbis_info_floor1 *info=static_cast<vorbis_info_floor1*>(_ogg_calloc(1,sizeof(*info)));
  /* read partitions */
  info->partitions=oggpack_read(opb,5); /* only 0 to 31 legal */
  for(j=0;j<info->partitions;j++){
    info->partitionclass[j]=oggpack_read(opb,4); /* only 0 to 15 legal */
    if(maxclass<info->partitionclass[j])maxclass=info->partitionclass[j];
  }

  /* read partition classes */
  for(j=0;j<maxclass+1;j++){
    info->class_dim[j]=oggpack_read(opb,3)+1; /* 1 to 8 */
    info->class_subs[j]=oggpack_read(opb,2); /* 0,1,2,3 bits */
    if(info->class_subs[j]<0)
      goto err_out;
    if(info->class_subs[j])info->class_book[j]=oggpack_read(opb,8);
    if(info->class_book[j]<0 || info->class_book[j]>=ci->books)
      goto err_out;
    for(k=0;k<(1<<info->class_subs[j]);k++){
      info->class_subbook[j][k]=oggpack_read(opb,8)-1;
      if(info->class_subbook[j][k]<-1 || info->class_subbook[j][k]>=ci->books)
    goto err_out;
    }
  }

  /* read the post list */
  info->mult=oggpack_read(opb,2)+1;     /* only 1,2,3,4 legal now */
  rangebits=oggpack_read(opb,4);

  for(j=0,k=0;j<info->partitions;j++){
    count+=info->class_dim[info->partitionclass[j]];
    for(;k<count;k++){
      int t=info->postlist[k+2]=oggpack_read(opb,rangebits);
      if(t<0 || t>=(1<<rangebits))
    goto err_out;
    }
  }
  info->postlist[0]=0;
  info->postlist[1]=1<<rangebits;

  return(info);

 err_out:
  floor1_free_info(info);
  return(NULL);
}

int icomp(const void *a,const void *b){
  return(**(int **)a-**(int **)b);
}

vorbis_look_floor *floor1_look(vorbis_dsp_state * /*vd unused*/,
                      vorbis_info_floor *in){

  int *sortpointer[VIF_POSIT+2];
  vorbis_info_floor1 *info=(vorbis_info_floor1 *)in;
  vorbis_look_floor1 *look=static_cast<vorbis_look_floor1*>(_ogg_calloc(1,sizeof(*look)));
  int i,j,n=0;

  look->vi=info;
  look->n=info->postlist[1];

  /* we drop each position value in-between already decoded values,
     and use linear interpolation to predict each new value past the
     edges.  The positions are read in the order of the position
     list... we precompute the bounding positions in the lookup.  Of
     course, the neighbors can change (if a position is declined), but
     this is an initial mapping */

  for(i=0;i<info->partitions;i++)n+=info->class_dim[info->partitionclass[i]];
  n+=2;
  look->posts=n;

  /* also store a sorted position index */
  for(i=0;i<n;i++)sortpointer[i]=info->postlist+i;
  qsort(sortpointer,n,sizeof(*sortpointer),icomp);

  /* points from sort order back to range number */
  for(i=0;i<n;i++) look->forward_index[i]=(int)(sortpointer[i]-info->postlist);
  /* points from range order to sorted position */
  for(i=0;i<n;i++)look->reverse_index[look->forward_index[i]]=i;
  /* we actually need the post values too */
  for(i=0;i<n;i++)look->sorted_index[i]=info->postlist[look->forward_index[i]];

  /* quantize values to multiplier spec */
  switch(info->mult){
  case 1: /* 1024 -> 256 */
    look->quant_q=256;
    break;
  case 2: /* 1024 -> 128 */
    look->quant_q=128;
    break;
  case 3: /* 1024 -> 86 */
    look->quant_q=86;
    break;
  case 4: /* 1024 -> 64 */
    look->quant_q=64;
    break;
  }

  /* discover our neighbors for decode where we don't use fit flags
     (that would push the neighbors outward) */
  for(i=0;i<n-2;i++){
    int lo=0;
    int hi=1;
    int lx=0;
    int hx=look->n;
    int currentx=info->postlist[i+2];
    for(j=0;j<i+2;j++){
      int x=info->postlist[j];
      if(x>lx && x<currentx){
    lo=j;
    lx=x;
      }
      if(x<hx && x>currentx){
    hi=j;
    hx=x;
      }
    }
    look->loneighbor[i]=lo;
    look->hineighbor[i]=hi;
  }

  return(look);
}

int render_point(int x0,int x1,int y0,int y1,int x){
  y0&=0x7fff; /* mask off flag */
  y1&=0x7fff;

  {
    int dy=y1-y0;
    int adx=x1-x0;
    int ady=abs(dy);
    int err=ady*(x-x0);

    int off=err/adx;
    if(dy<0)return(y0-off);
    return(y0+off);
  }
}

int vorbis_dBquant(const float *x){
  int i= (int)(*x*7.3142857f+1023.5f);
  if(i>1023)return(1023);
  if(i<0)return(0);
  return i;
}

float FLOOR1_fromdB_LOOKUP[256]={
  1.0649863e-07F, 1.1341951e-07F, 1.2079015e-07F, 1.2863978e-07F,
  1.3699951e-07F, 1.4590251e-07F, 1.5538408e-07F, 1.6548181e-07F,
  1.7623575e-07F, 1.8768855e-07F, 1.9988561e-07F, 2.128753e-07F,
  2.2670913e-07F, 2.4144197e-07F, 2.5713223e-07F, 2.7384213e-07F,
  2.9163793e-07F, 3.1059021e-07F, 3.3077411e-07F, 3.5226968e-07F,
  3.7516214e-07F, 3.9954229e-07F, 4.2550680e-07F, 4.5315863e-07F,
  4.8260743e-07F, 5.1396998e-07F, 5.4737065e-07F, 5.8294187e-07F,
  6.2082472e-07F, 6.6116941e-07F, 7.0413592e-07F, 7.4989464e-07F,
  7.9862701e-07F, 8.5052630e-07F, 9.0579828e-07F, 9.6466216e-07F,
  1.0273513e-06F, 1.0941144e-06F, 1.1652161e-06F, 1.2409384e-06F,
  1.3215816e-06F, 1.4074654e-06F, 1.4989305e-06F, 1.5963394e-06F,
  1.7000785e-06F, 1.8105592e-06F, 1.9282195e-06F, 2.0535261e-06F,
  2.1869758e-06F, 2.3290978e-06F, 2.4804557e-06F, 2.6416497e-06F,
  2.8133190e-06F, 2.9961443e-06F, 3.1908506e-06F, 3.3982101e-06F,
  3.6190449e-06F, 3.8542308e-06F, 4.1047004e-06F, 4.3714470e-06F,
  4.6555282e-06F, 4.9580707e-06F, 5.2802740e-06F, 5.6234160e-06F,
  5.9888572e-06F, 6.3780469e-06F, 6.7925283e-06F, 7.2339451e-06F,
  7.7040476e-06F, 8.2047000e-06F, 8.7378876e-06F, 9.3057248e-06F,
  9.9104632e-06F, 1.0554501e-05F, 1.1240392e-05F, 1.1970856e-05F,
  1.2748789e-05F, 1.3577278e-05F, 1.4459606e-05F, 1.5399272e-05F,
  1.6400004e-05F, 1.7465768e-05F, 1.8600792e-05F, 1.9809576e-05F,
  2.1096914e-05F, 2.2467911e-05F, 2.3928002e-05F, 2.5482978e-05F,
  2.7139006e-05F, 2.8902651e-05F, 3.0780908e-05F, 3.2781225e-05F,
  3.4911534e-05F, 3.7180282e-05F, 3.9596466e-05F, 4.2169667e-05F,
  4.4910090e-05F, 4.7828601e-05F, 5.0936773e-05F, 5.4246931e-05F,
  5.7772202e-05F, 6.1526565e-05F, 6.5524908e-05F, 6.9783085e-05F,
  7.4317983e-05F, 7.9147585e-05F, 8.4291040e-05F, 8.9768747e-05F,
  9.5602426e-05F, 0.00010181521F, 0.00010843174F, 0.00011547824F,
  0.00012298267F, 0.00013097477F, 0.00013948625F, 0.00014855085F,
  0.00015820453F, 0.00016848555F, 0.00017943469F, 0.00019109536F,
  0.00020351382F, 0.00021673929F, 0.00023082423F, 0.00024582449F,
  0.00026179955F, 0.00027881276F, 0.00029693158F, 0.00031622787F,
  0.00033677814F, 0.00035866388F, 0.00038197188F, 0.00040679456F,
  0.00043323036F, 0.00046138411F, 0.00049136745F, 0.00052329927F,
  0.00055730621F, 0.00059352311F, 0.00063209358F, 0.00067317058F,
  0.00071691700F, 0.00076350630F, 0.00081312324F, 0.00086596457F,
  0.00092223983F, 0.00098217216F, 0.0010459992F, 0.0011139742F,
  0.0011863665F, 0.0012634633F, 0.0013455702F, 0.0014330129F,
  0.0015261382F, 0.0016253153F, 0.0017309374F, 0.0018434235F,
  0.0019632195F, 0.0020908006F, 0.0022266726F, 0.0023713743F,
  0.0025254795F, 0.0026895994F, 0.0028643847F, 0.0030505286F,
  0.0032487691F, 0.0034598925F, 0.0036847358F, 0.0039241906F,
  0.0041792066F, 0.0044507950F, 0.0047400328F, 0.0050480668F,
  0.0053761186F, 0.0057254891F, 0.0060975636F, 0.0064938176F,
  0.0069158225F, 0.0073652516F, 0.0078438871F, 0.0083536271F,
  0.0088964928F, 0.009474637F, 0.010090352F, 0.010746080F,
  0.011444421F, 0.012188144F, 0.012980198F, 0.013823725F,
  0.014722068F, 0.015678791F, 0.016697687F, 0.017782797F,
  0.018938423F, 0.020169149F, 0.021479854F, 0.022875735F,
  0.024362330F, 0.025945531F, 0.027631618F, 0.029427276F,
  0.031339626F, 0.033376252F, 0.035545228F, 0.037855157F,
  0.040315199F, 0.042935108F, 0.045725273F, 0.048696758F,
  0.051861348F, 0.055231591F, 0.058820850F, 0.062643361F,
  0.066714279F, 0.071049749F, 0.075666962F, 0.080584227F,
  0.085821044F, 0.091398179F, 0.097337747F, 0.10366330F,
  0.11039993F, 0.11757434F, 0.12521498F, 0.13335215F,
  0.14201813F, 0.15124727F, 0.16107617F, 0.17154380F,
  0.18269168F, 0.19456402F, 0.20720788F, 0.22067342F,
  0.23501402F, 0.25028656F, 0.26655159F, 0.28387361F,
  0.30232132F, 0.32196786F, 0.34289114F, 0.36517414F,
  0.38890521F, 0.41417847F, 0.44109412F, 0.46975890F,
  0.50028648F, 0.53279791F, 0.56742212F, 0.60429640F,
  0.64356699F, 0.68538959F, 0.72993007F, 0.77736504F,
  0.82788260F, 0.88168307F, 0.9389798F, 1.F,
};

void render_line(int x0,int x1,int y0,int y1,float *d){
  int dy=y1-y0;
  int adx=x1-x0;
  int ady=abs(dy);
  int base=dy/adx;
  int sy=(dy<0?base-1:base+1);
  int x=x0;
  int y=y0;
  int err=0;

  ady-=abs(base*adx);

  d[x]*=FLOOR1_fromdB_LOOKUP[y];
  while(++x<x1){
    err=err+ady;
    if(err>=adx){
      err-=adx;
      y+=sy;
    }else{
      y+=base;
    }
    d[x]*=FLOOR1_fromdB_LOOKUP[y];
  }
}

void render_line0(int x0,int x1,int y0,int y1,int *d){
  int dy=y1-y0;
  int adx=x1-x0;
  int ady=abs(dy);
  int base=dy/adx;
  int sy=(dy<0?base-1:base+1);
  int x=x0;
  int y=y0;
  int err=0;

  ady-=abs(base*adx);

  d[x]=y;
  while(++x<x1){
    err=err+ady;
    if(err>=adx){
      err-=adx;
      y+=sy;
    }else{
      y+=base;
    }
    d[x]=y;
  }
}

/* the floor has already been filtered to only include relevant sections */
int accumulate_fit(const float *flr,const float *mdct,
              int x0, int x1,lsfit_acc *a,
              int n,vorbis_info_floor1 *info){
  long i;
  //int quantized=vorbis_dBquant(flr+x0); unused

  long xa=0,ya=0,x2a=0,y2a=0,xya=0,na=0, xb=0,yb=0,x2b=0,y2b=0,xyb=0,nb=0;

  memset(a,0,sizeof(*a));
  a->x0=x0;
  a->x1=x1;
  if(x1>=n)x1=n-1;

  for(i=x0;i<=x1;i++){
    int quantized=vorbis_dBquant(flr+i);
    if(quantized){
      if(mdct[i]+info->twofitatten>=flr[i]){
    xa  += i;
    ya  += quantized;
    x2a += i*i;
    y2a += quantized*quantized;
    xya += i*quantized;
    na++;
      }else{
    xb  += i;
    yb  += quantized;
    x2b += i*i;
    y2b += quantized*quantized;
    xyb += i*quantized;
    nb++;
      }
    }
  }

  xb+=xa;
  yb+=ya;
  x2b+=x2a;
  y2b+=y2a;
  xyb+=xya;
  nb+=na;

  /* weight toward the actually used frequencies if we meet the threshhold */
  {
    int weight=(int)(nb*info->twofitweight/(na+1));

    a->xa=xa*weight+xb;
    a->ya=ya*weight+yb;
    a->x2a=x2a*weight+x2b;
    a->y2a=y2a*weight+y2b;
    a->xya=xya*weight+xyb;
    a->an=na*weight+nb;
    a->n=nb;
  }

  return(na);
}

void fit_line(lsfit_acc *a,int fits,int *y0,int *y1){
  long x=0,y=0,x2=0,y2=0,xy=0,n=0,an=0,i;
  long x0=a[0].x0;
  long x1=a[fits-1].x1;

  for(i=0;i<fits;i++){
    x+=a[i].xa;
    y+=a[i].ya;
    x2+=a[i].x2a;
    y2+=a[i].y2a;
    xy+=a[i].xya;
    n+=a[i].n;
    an+=a[i].an;
  }

  if(*y0>=0){
    x+=   x0;
    y+=  *y0;
    x2+=  x0 *  x0;
    y2+= *y0 * *y0;
    xy+= *y0 *  x0;
    n++;
    an++;
  }

  if(*y1>=0){
    x+=   x1;
    y+=  *y1;
    x2+=  x1 *  x1;
    y2+= *y1 * *y1; y2; // unused?
    xy+= *y1 *  x1;
    n++; n; // unused?
    an++;
  }

  {
    /* need 64 bit multiplies, which C doesn't give portably as int */
    double fx=x;
    double fy=y;
    double fx2=x2;
    double fxy=xy;
    double denom=1./(an*fx2-fx*fx);
    double a=(fy*fx2-fxy*fx)*denom;
    double b=(an*fxy-fx*fy)*denom;
    *y0=(int)rint(a+b*x0);
    *y1=(int)rint(a+b*x1);

    /* limit to our range! */
    if(*y0>1023)*y0=1023;
    if(*y1>1023)*y1=1023;
    if(*y0<0)*y0=0;
    if(*y1<0)*y1=0;

  }
}

/*void fit_line_point(lsfit_acc *a,int fits,int *y0,int *y1){
  long y=0;
  int i;

  for(i=0;i<fits && y==0;i++)
    y+=a[i].ya;

  *y0=*y1=y;
  }*/

int inspect_error(int x0,int x1,int y0,int y1,const float *mask,
             const float *mdct,
             vorbis_info_floor1 *info){
  int dy=y1-y0;
  int adx=x1-x0;
  int ady=abs(dy);
  int base=dy/adx;
  int sy=(dy<0?base-1:base+1);
  int x=x0;
  int y=y0;
  int err=0;
  int val=vorbis_dBquant(mask+x);
  int mse=0; mse; // unused?
  int n=0;

  ady-=abs(base*adx);

  mse=(y-val);
  mse*=mse;
  n++;
  if(mdct[x]+info->twofitatten>=mask[x]){
    if(y+info->maxover<val)return(1);
    if(y-info->maxunder>val)return(1);
  }

  while(++x<x1){
    err=err+ady;
    if(err>=adx){
      err-=adx;
      y+=sy;
    }else{
      y+=base;
    }

    val=vorbis_dBquant(mask+x);
    mse+=((y-val)*(y-val));
    n++;
    if(mdct[x]+info->twofitatten>=mask[x]){
      if(val){
    if(y+info->maxover<val)return(1);
    if(y-info->maxunder>val)return(1);
      }
    }
  }

  if(info->maxover*info->maxover/n>info->maxerr)return(0);
  if(info->maxunder*info->maxunder/n>info->maxerr)return(0);
  if(mse/n>info->maxerr)return(1);
  return(0);
}

int post_Y(int *A,int *B,int pos){
  if(A[pos]<0)
    return B[pos];
  if(B[pos]<0)
    return A[pos];

  return (A[pos]+B[pos])>>1;
}

//int seq=0; unused



int *floor1_fit(vorbis_block *vb,vorbis_look_floor1 *look,
              const float *logmdct,   /* in */
              const float *logmask){
  long i,j;
  vorbis_info_floor1 *info=look->vi;
  long n=look->n;
  long posts=look->posts;
  long nonzero=0;
  lsfit_acc fits[VIF_POSIT+1];
  int fit_valueA[VIF_POSIT+2]; /* index by range list position */
  int fit_valueB[VIF_POSIT+2]; /* index by range list position */

  int loneighbor[VIF_POSIT+2]; /* sorted index of range list position (+2) */
  int hineighbor[VIF_POSIT+2];
  int *output=NULL;
  int memo[VIF_POSIT+2];

  for(i=0;i<posts;i++)fit_valueA[i]=-200; /* mark all unused */
  for(i=0;i<posts;i++)fit_valueB[i]=-200; /* mark all unused */
  for(i=0;i<posts;i++)loneighbor[i]=0; /* 0 for the implicit 0 post */
  for(i=0;i<posts;i++)hineighbor[i]=1; /* 1 for the implicit post at n */
  for(i=0;i<posts;i++)memo[i]=-1;      /* no neighbor yet */

  /* quantize the relevant floor points and collect them into line fit
     structures (one per minimal division) at the same time */
  if(posts==0){
    nonzero+=accumulate_fit(logmask,logmdct,0,n,fits,n,info);
  }else{
    for(i=0;i<posts-1;i++)
      nonzero+=accumulate_fit(logmask,logmdct,look->sorted_index[i],
                  look->sorted_index[i+1],fits+i,
                  n,info);
  }

  if(nonzero){
    /* start by fitting the implicit base case.... */
    int y0=-200;
    int y1=-200;
    fit_line(fits,posts-1,&y0,&y1);

    fit_valueA[0]=y0;
    fit_valueB[0]=y0;
    fit_valueB[1]=y1;
    fit_valueA[1]=y1;

    /* Non degenerate case */
    /* start progressive splitting.  This is a greedy, non-optimal
       algorithm, but simple and close enough to the best
       answer. */
    for(i=2;i<posts;i++){
      int sortpos=look->reverse_index[i];
      int ln=loneighbor[sortpos];
      int hn=hineighbor[sortpos];

      /* eliminate repeat searches of a particular range with a memo */
      if(memo[ln]!=hn){
    /* haven't performed this error search yet */
    int lsortpos=look->reverse_index[ln];
    int hsortpos=look->reverse_index[hn];
    memo[ln]=hn;

    {
      /* A note: we want to bound/minimize *local*, not global, error */
      int lx=info->postlist[ln];
      int hx=info->postlist[hn];
      int ly=post_Y(fit_valueA,fit_valueB,ln);
      int hy=post_Y(fit_valueA,fit_valueB,hn);

      if(ly==-1 || hy==-1){
        exit(1);
      }

      if(inspect_error(lx,hx,ly,hy,logmask,logmdct,info)){
        /* outside error bounds/begin search area.  Split it. */
        int ly0=-200;
        int ly1=-200;
        int hy0=-200;
        int hy1=-200;
        fit_line(fits+lsortpos,sortpos-lsortpos,&ly0,&ly1);
        fit_line(fits+sortpos,hsortpos-sortpos,&hy0,&hy1);

        /* store new edge values */
        fit_valueB[ln]=ly0;
        if(ln==0)fit_valueA[ln]=ly0;
        fit_valueA[i]=ly1;
        fit_valueB[i]=hy0;
        fit_valueA[hn]=hy1;
        if(hn==1)fit_valueB[hn]=hy1;

        if(ly1>=0 || hy0>=0){
          /* store new neighbor values */
          for(j=sortpos-1;j>=0;j--)
        if(hineighbor[j]==hn)
          hineighbor[j]=i;
        else
          break;
          for(j=sortpos+1;j<posts;j++)
        if(loneighbor[j]==ln)
          loneighbor[j]=i;
        else
          break;

        }
      }else{

        fit_valueA[i]=-200;
        fit_valueB[i]=-200;
      }
    }
      }
    }

    output=static_cast<int*>(_vorbis_block_alloc(vb,sizeof(*output)*posts));

    output[0]=post_Y(fit_valueA,fit_valueB,0);
    output[1]=post_Y(fit_valueA,fit_valueB,1);

    /* fill in posts marked as not using a fit; we will zero
       back out to 'unused' when encoding them so long as curve
       interpolation doesn't force them into use */
    for(i=2;i<posts;i++){
      int ln=look->loneighbor[i-2];
      int hn=look->hineighbor[i-2];
      int x0=info->postlist[ln];
      int x1=info->postlist[hn];
      int y0=output[ln];
      int y1=output[hn];

      int predicted=render_point(x0,x1,y0,y1,info->postlist[i]);
      int vx=post_Y(fit_valueA,fit_valueB,i);

      if(vx>=0 && predicted!=vx){
    output[i]=vx;
      }else{
    output[i]= predicted|0x8000;
      }
    }
  }

  return(output);

}

int *floor1_interpolate_fit(vorbis_block *vb,vorbis_look_floor1 *look,
              int *A,int *B,
              int del){

  long i;
  long posts=look->posts;
  int *output=NULL;

  if(A && B){
    output=static_cast<int*>(_vorbis_block_alloc(vb,sizeof(*output)*posts));

    for(i=0;i<posts;i++){
      output[i]=((65536-del)*(A[i]&0x7fff)+del*(B[i]&0x7fff)+32768)>>16;
      if(A[i]&0x8000 && B[i]&0x8000)output[i]|=0x8000;
    }
  }

  return(output);
}


int floor1_encode(vorbis_block *vb,vorbis_look_floor1 *look,
          int *post,int *ilogmask){

  long i,j;
  vorbis_info_floor1 *info=look->vi;
  //long n=look->n; unused
  long posts=look->posts;
  codec_setup_info *ci=static_cast<codec_setup_info*>(vb->vd->vi->codec_setup);
  int out[VIF_POSIT+2];
  static_codebook **sbooks=ci->book_param;
  codebook *books=ci->fullbooks;
  long seq=0;

  /* quantize values to multiplier spec */
  if(post){
    for(i=0;i<posts;i++){
      int val=post[i]&0x7fff;
      switch(info->mult){
      case 1: /* 1024 -> 256 */
    val>>=2;
    break;
      case 2: /* 1024 -> 128 */
    val>>=3;
    break;
      case 3: /* 1024 -> 86 */
    val/=12;
    break;
      case 4: /* 1024 -> 64 */
    val>>=4;
    break;
      }
      post[i]=val | (post[i]&0x8000);
    }

    out[0]=post[0];
    out[1]=post[1];

    /* find prediction values for each post and subtract them */
    for(i=2;i<posts;i++){
      int ln=look->loneighbor[i-2];
      int hn=look->hineighbor[i-2];
      int x0=info->postlist[ln];
      int x1=info->postlist[hn];
      int y0=post[ln];
      int y1=post[hn];

      int predicted=render_point(x0,x1,y0,y1,info->postlist[i]);

      if((post[i]&0x8000) || (predicted==post[i])){
    post[i]=predicted|0x8000; /* in case there was roundoff jitter
                     in interpolation */
    out[i]=0;
      }else{
    int headroom=(look->quant_q-predicted<predicted?
              look->quant_q-predicted:predicted);

    int val=post[i]-predicted;

    /* at this point the 'deviation' value is in the range +/- max
       range, but the real, unique range can always be mapped to
       only [0-maxrange).  So we want to wrap the deviation into
       this limited range, but do it in the way that least screws
       an essentially gaussian probability distribution. */

    if(val<0)
      if(val<-headroom)
        val=headroom-val-1;
      else
        val=-1-(val<<1);
    else
      if(val>=headroom)
        val= val+headroom;
      else
        val<<=1;

    out[i]=val;
    post[ln]&=0x7fff;
    post[hn]&=0x7fff;
      }
    }

    /* we have everything we need. pack it out */
    /* mark nontrivial floor */
    oggpack_write(&vb->opb,1,1);

    /* beginning/end post */
    look->frames++;
    look->postbits+=ilog(look->quant_q-1)*2;
    oggpack_write(&vb->opb,out[0],ilog(look->quant_q-1));
    oggpack_write(&vb->opb,out[1],ilog(look->quant_q-1));


    /* partition by partition */
    for(i=0,j=2;i<info->partitions;i++){
      int classcp=info->partitionclass[i];
      int cdim=info->class_dim[classcp];
      int csubbits=info->class_subs[classcp];
      int csub=1<<csubbits;
      int bookas[8]={0,0,0,0,0,0,0,0};
      int cval=0;
      int cshift=0;
      int k,l;

      /* generate the partition's first stage cascade value */
      if(csubbits){
    int maxval[8];
    for(k=0;k<csub;k++){
      int booknum=info->class_subbook[classcp][k];
      if(booknum<0){
        maxval[k]=1;
      }else{
        maxval[k]=sbooks[info->class_subbook[classcp][k]]->entries;
      }
    }
    for(k=0;k<cdim;k++){
      for(l=0;l<csub;l++){
        int val=out[j+k];
        if(val<maxval[l]){
          bookas[k]=l;
          break;
        }
      }
      cval|= bookas[k]<<cshift;
      cshift+=csubbits;
    }
    /* write it */
    look->phrasebits+=
      vorbis_book_encode(books+info->class_book[classcp],cval,&vb->opb);

#ifdef TRAIN_FLOOR1
    {
      FILE *of;
      char buffer[80];
      sprintf(buffer,"line_%dx%ld_class%d.vqd",
          vb->pcmend/2,posts-2,classcp);
      of=fopen(buffer,"a");
      fprintf(of,"%d\n",cval);
      fclose(of);
    }
#endif
      }

      /* write post values */
      for(k=0;k<cdim;k++){
    int book=info->class_subbook[classcp][bookas[k]];
    if(book>=0){
      /* hack to allow training with 'bad' books */
      if(out[j+k]<(books+book)->entries)
        look->postbits+=vorbis_book_encode(books+book,
                           out[j+k],&vb->opb);
      /*else
        fprintf(stderr,"+!");*/

#ifdef TRAIN_FLOOR1
      {
        FILE *of;
        char buffer[80];
        sprintf(buffer,"line_%dx%ld_%dsub%d.vqd",
            vb->pcmend/2,posts-2,classcp,bookas[k]);
        of=fopen(buffer,"a");
        fprintf(of,"%d\n",out[j+k]);
        fclose(of);
      }
#endif
    }
      }
      j+=cdim;
    }

    {
      /* generate quantized floor equivalent to what we'd unpack in decode */
      /* render the lines */
      int hx=0;
      int lx=0;
      int ly=post[0]*info->mult;
      for(j=1;j<look->posts;j++){
    int current=look->forward_index[j];
    int hy=post[current]&0x7fff;
    if(hy==post[current]){

      hy*=info->mult;
      hx=info->postlist[current];

      render_line0(lx,hx,ly,hy,ilogmask);

      lx=hx;
      ly=hy;
    }
      }
      for(j=hx;j<vb->pcmend/2;j++)ilogmask[j]=ly; /* be certain */
      seq++;
      return(1);
    }
  }else{
    oggpack_write(&vb->opb,0,1);
    memset(ilogmask,0,vb->pcmend/2*sizeof(*ilogmask));
    seq++;
    return(0);
  }
}

void *floor1_inverse1(vorbis_block *vb,vorbis_look_floor *in){
  vorbis_look_floor1 *look=(vorbis_look_floor1 *)in;
  vorbis_info_floor1 *info=look->vi;
  codec_setup_info   *ci=static_cast<codec_setup_info*>(vb->vd->vi->codec_setup);

  int i,j,k;
  codebook *books=ci->fullbooks;

  /* unpack wrapped/predicted values from stream */
  if(oggpack_read(&vb->opb,1)==1){
    int *fit_value=static_cast<int*>(_vorbis_block_alloc(vb,(look->posts)*sizeof(*fit_value)));

    fit_value[0]=oggpack_read(&vb->opb,ilog(look->quant_q-1));
    fit_value[1]=oggpack_read(&vb->opb,ilog(look->quant_q-1));

    /* partition by partition */
    for(i=0,j=2;i<info->partitions;i++){
      int classcp=info->partitionclass[i];
      int cdim=info->class_dim[classcp];
      int csubbits=info->class_subs[classcp];
      int csub=1<<csubbits;
      int cval=0;

      /* decode the partition's first stage cascade value */
      if(csubbits){
    cval=vorbis_book_decode(books+info->class_book[classcp],&vb->opb);

    if(cval==-1)goto eop;
      }

      for(k=0;k<cdim;k++){
    int book=info->class_subbook[classcp][cval&(csub-1)];
    cval>>=csubbits;
    if(book>=0){
      if((fit_value[j+k]=vorbis_book_decode(books+book,&vb->opb))==-1)
        goto eop;
    }else{
      fit_value[j+k]=0;
    }
      }
      j+=cdim;
    }

    /* unwrap positive values and reconsitute via linear interpolation */
    for(i=2;i<look->posts;i++){
      int predicted=render_point(info->postlist[look->loneighbor[i-2]],
                 info->postlist[look->hineighbor[i-2]],
                 fit_value[look->loneighbor[i-2]],
                 fit_value[look->hineighbor[i-2]],
                 info->postlist[i]);
      int hiroom=look->quant_q-predicted;
      int loroom=predicted;
      int room=(hiroom<loroom?hiroom:loroom)<<1;
      int val=fit_value[i];

      if(val){
    if(val>=room){
      if(hiroom>loroom){
        val = val-loroom;
      }else{
        val = -1-(val-hiroom);
      }
    }else{
      if(val&1){
        val= -((val+1)>>1);
      }else{
        val>>=1;
      }
    }

    fit_value[i]=val+predicted;
    fit_value[look->loneighbor[i-2]]&=0x7fff;
    fit_value[look->hineighbor[i-2]]&=0x7fff;

      }else{
    fit_value[i]=predicted|0x8000;
      }

    }

    return(fit_value);
  }
 eop:
  return(NULL);
}

int floor1_inverse2(vorbis_block *vb,vorbis_look_floor *in,void *memo,
              float *out){
  vorbis_look_floor1 *look=(vorbis_look_floor1 *)in;
  vorbis_info_floor1 *info=look->vi;

  codec_setup_info   *ci=static_cast<codec_setup_info*>(vb->vd->vi->codec_setup);
  int                  n=ci->blocksizes[vb->W]/2;
  int j;

  if(memo){
    /* render the lines */
    int *fit_value=(int *)memo;
    int hx=0;
    int lx=0;
    int ly=fit_value[0]*info->mult;
    for(j=1;j<look->posts;j++){
      int current=look->forward_index[j];
      int hy=fit_value[current]&0x7fff;
      if(hy==fit_value[current]){

    hy*=info->mult;
    hx=info->postlist[current];

    render_line(lx,hx,ly,hy,out);

    lx=hx;
    ly=hy;
      }
    }
    for(j=hx;j<n;j++)out[j]*=FLOOR1_fromdB_LOOKUP[ly]; /* be certain */
    return(1);
  }
  memset(out,0,sizeof(*out)*n);
  return(0);
}

/* export hooks */
vorbis_func_floor floor1_exportbundle={
  floor1_pack,floor1_unpack,floor1_look,floor1_free_info,
  floor1_free_look,floor1_inverse1,floor1_inverse2
};











// ==========================================================================
// ==========================================================================
// floor0.c
// ==========================================================================
// ==========================================================================

typedef struct {
  int ln;
  int  m;
  int **linearmap;
  int  n[2];

  vorbis_info_floor0 *vi;
  lpc_lookup lpclook;
  float *lsp_look;

  long bits;
  long frames;
} vorbis_look_floor0;


/***********************************************/

void floor0_free_info(vorbis_info_floor *i){
  vorbis_info_floor0 *info=(vorbis_info_floor0 *)i;
  if(info){
    memset(info,0,sizeof(*info));
    _ogg_free(info);
  }
}

void floor0_free_look(vorbis_look_floor *i){
  vorbis_look_floor0 *look=(vorbis_look_floor0 *)i;
  if(look){

    if(look->linearmap){

      if(look->linearmap[0])_ogg_free(look->linearmap[0]);
      if(look->linearmap[1])_ogg_free(look->linearmap[1]);

      _ogg_free(look->linearmap);
    }
    if(look->lsp_look)_ogg_free(look->lsp_look);
    lpc_clear(&look->lpclook);
    memset(look,0,sizeof(*look));
    _ogg_free(look);
  }
}

vorbis_info_floor *floor0_unpack (vorbis_info *vi,oggpack_buffer *opb){
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  int j;

  vorbis_info_floor0 *info=static_cast<vorbis_info_floor0*>(_ogg_malloc(sizeof(*info)));
  info->order=oggpack_read(opb,8);
  info->rate=oggpack_read(opb,16);
  info->barkmap=oggpack_read(opb,16);
  info->ampbits=oggpack_read(opb,6);
  info->ampdB=oggpack_read(opb,8);
  info->numbooks=oggpack_read(opb,4)+1;

  if(info->order<1)goto err_out;
  if(info->rate<1)goto err_out;
  if(info->barkmap<1)goto err_out;
  if(info->numbooks<1)goto err_out;

  for(j=0;j<info->numbooks;j++){
    info->books[j]=oggpack_read(opb,8);
    if(info->books[j]<0 || info->books[j]>=ci->books)goto err_out;
  }
  return(info);

 err_out:
  floor0_free_info(info);
  return(NULL);
}

/* initialize Bark scale and normalization lookups.  We could do this
   with tables, but Vorbis allows a number of possible
   combinations, so it's best to do it computationally.

   The below is authoritative in terms of defining scale mapping.
   Note that the scale depends on the sampling rate as well as the
   linear block and mapping sizes */

void floor0_map_lazy_init(vorbis_block      *vb,
                 vorbis_info_floor *infoX,
                 vorbis_look_floor0 *look){
  if(!look->linearmap[vb->W]){
    vorbis_dsp_state   *vd=vb->vd;
    vorbis_info        *vi=vd->vi;
    codec_setup_info   *ci=static_cast<codec_setup_info*>(vi->codec_setup);
    vorbis_info_floor0 *info=(vorbis_info_floor0 *)infoX;
    int W=vb->W;
    int n=ci->blocksizes[W]/2,j;

    /* we choose a scaling constant so that:
       floor(bark(rate/2-1)*C)=mapped-1
     floor(bark(rate/2)*C)=mapped */
    float scale=(float)(look->ln/toBARK(info->rate/2.f));

    /* the mapping from a linear scale to a smaller bark scale is
       straightforward.  We do *not* make sure that the linear mapping
       does not skip bark-scale bins; the decoder simply skips them and
       the encoder may do what it wishes in filling them.  They're
       necessary in some mapping combinations to keep the scale spacing
       accurate */
    look->linearmap[W]=static_cast<int*>(_ogg_malloc((n+1)*sizeof(**look->linearmap)));
    for(j=0;j<n;j++){
      int val=(int)(floor( toBARK((info->rate/2.f)/n*j)*scale)); /* bark numbers represent band edges */
      if(val>=look->ln)val=look->ln-1; /* guard against the approximation */
      look->linearmap[W][j]=val;
    }
    look->linearmap[W][j]=-1;
    look->n[W]=n;
  }
}

vorbis_look_floor *floor0_look(vorbis_dsp_state *vd,
                      vorbis_info_floor *i){
  int j;
  vorbis_info_floor0 *info=(vorbis_info_floor0 *)i;
  vorbis_look_floor0 *look=static_cast<vorbis_look_floor0*>(_ogg_calloc(1,sizeof(*look)));
  look->m=info->order;
  look->ln=info->barkmap;
  look->vi=info;

  if(vd->analysisp)
    lpc_init(&look->lpclook,look->ln,look->m);

  look->linearmap=static_cast<int**>(_ogg_calloc(2,sizeof(*look->linearmap)));

  look->lsp_look=static_cast<float*>(_ogg_malloc(look->ln*sizeof(*look->lsp_look)));
  for(j=0;j<look->ln;j++)
    look->lsp_look[j]=(float)(2*cos(M_PI/look->ln*j));

  return look;
}

void *floor0_inverse1(vorbis_block *vb,vorbis_look_floor *i){
  vorbis_look_floor0 *look=(vorbis_look_floor0 *)i;
  vorbis_info_floor0 *info=look->vi;
  int j,k;

  int ampraw=oggpack_read(&vb->opb,info->ampbits);
  if(ampraw>0){ /* also handles the -1 out of data case */
    long maxval=(1<<info->ampbits)-1;
    float amp=(float)ampraw/maxval*info->ampdB;
    int booknum=oggpack_read(&vb->opb,_ilog(info->numbooks));

    if(booknum!=-1 && booknum<info->numbooks){ /* be paranoid */
      codec_setup_info  *ci=static_cast<codec_setup_info*>(vb->vd->vi->codec_setup);
      codebook *b=ci->fullbooks+info->books[booknum];
      float last=0.f;

      /* the additional b->dim is a guard against any possible stack
         smash; b->dim is provably more than we can overflow the
         vector */
      float *lsp=static_cast<float*>(_vorbis_block_alloc(vb,sizeof(*lsp)*(look->m+b->dim+1)));

      for(j=0;j<look->m;j+=b->dim)
    if(vorbis_book_decodev_set(b,lsp+j,&vb->opb,b->dim)==-1)goto eop;
      for(j=0;j<look->m;){
    for(k=0;k<b->dim;k++,j++)lsp[j]+=last;
    last=lsp[j-1];
      }

      lsp[look->m]=amp;
      return(lsp);
    }
  }
 eop:
  return(NULL);
}

int floor0_inverse2(vorbis_block *vb,vorbis_look_floor *i,
               void *memo,float *out){
  vorbis_look_floor0 *look=(vorbis_look_floor0 *)i;
  vorbis_info_floor0 *info=look->vi;

  floor0_map_lazy_init(vb,info,look);

  if(memo){
    float *lsp=(float *)memo;
    float amp=lsp[look->m];

    /* take the coefficients back to a spectral envelope curve */
    vorbis_lsp_to_curve(out,
            look->linearmap[vb->W],
            look->n[vb->W],
            look->ln,
            lsp,look->m,amp,(float)info->ampdB);
    return(1);
  }
  memset(out,0,sizeof(*out)*look->n[vb->W]);
  return(0);
}

/* export hooks */
vorbis_func_floor floor0_exportbundle={
  NULL,floor0_unpack,floor0_look,floor0_free_info,
  floor0_free_look,floor0_inverse1,floor0_inverse2
};













// ==========================================================================
// ==========================================================================
// envelope.c
// ==========================================================================
// ==========================================================================


void _ve_envelope_init(envelope_lookup *e,vorbis_info *vi){
  codec_setup_info *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  vorbis_info_psy_global *gi=&ci->psy_g_param;
  int ch=vi->channels;
  int i,j;
  int n=e->winlength=128;
  e->searchstep=64; /* not random */

  e->minenergy=gi->preecho_minenergy;
  e->ch=ch;
  e->storage=128;
  e->cursor=ci->blocksizes[1]/2;
  e->mdct_win=static_cast<float*>(_ogg_calloc(n,sizeof(*e->mdct_win)));
  mdct_init(&e->mdct,n);

  for(i=0;i<n;i++){
    e->mdct_win[i]=(float)sin(i/(n-1.)*M_PI);
    e->mdct_win[i]*=(float)e->mdct_win[i];
  }

  /* magic follows */
  e->band[0].begin=2;  e->band[0].end=4;
  e->band[1].begin=4;  e->band[1].end=5;
  e->band[2].begin=6;  e->band[2].end=6;
  e->band[3].begin=9;  e->band[3].end=8;
  e->band[4].begin=13;  e->band[4].end=8;
  e->band[5].begin=17;  e->band[5].end=8;
  e->band[6].begin=22;  e->band[6].end=8;

  for(j=0;j<VE_BANDS;j++){
    n=e->band[j].end;
    e->band[j].window=static_cast<float*>(_ogg_malloc(n*sizeof(*e->band[0].window)));
    for(i=0;i<n;i++){
      e->band[j].window[i]=(float)sin((i+.5)/n*M_PI);
      e->band[j].total+=(float)e->band[j].window[i];
    }
    e->band[j].total=(float)(1./e->band[j].total);
  }

  e->filter=static_cast<envelope_filter_state*>(_ogg_calloc(VE_BANDS*ch,sizeof(*e->filter)));
  e->mark=static_cast<int*>(_ogg_calloc(e->storage,sizeof(*e->mark)));

}

void _ve_envelope_clear(envelope_lookup *e){
  int i;
  mdct_clear(&e->mdct);
  for(i=0;i<VE_BANDS;i++)
    _ogg_free(e->band[i].window);
  _ogg_free(e->mdct_win);
  _ogg_free(e->filter);
  _ogg_free(e->mark);
  memset(e,0,sizeof(*e));
}

/* fairly straight threshhold-by-band based until we find something
   that works better and isn't patented. */

int _ve_amp(envelope_lookup *ve,
           vorbis_info_psy_global *gi,
           float *data,
           envelope_band *bands,
           envelope_filter_state *filters,
           long /*pos unused*/){
  long n=ve->winlength;
  int ret=0;
  long i,j;
  float decay;

  /* we want to have a 'minimum bar' for energy, else we're just
     basing blocks on quantization noise that outweighs the signal
     itself (for low power signals) */

  float minV=ve->minenergy;
  float *vec=static_cast<float*>(alloca(n*sizeof(*vec)));

  /* stretch is used to gradually lengthen the number of windows
     considered prevoius-to-potential-trigger */
  int stretch=max(VE_MINSTRETCH,ve->stretch/2);
  float penalty=gi->stretch_penalty-(ve->stretch/2-VE_MINSTRETCH);
  if(penalty<0.f)penalty=0.f;
  if(penalty>gi->stretch_penalty)penalty=gi->stretch_penalty;

  /*_analysis_output_always("lpcm",seq2,data,n,0,0,
    totalshift+pos*ve->searchstep);*/

 /* window and transform */
  for(i=0;i<n;i++)
    vec[i]=data[i]*ve->mdct_win[i];
  mdct_forward(&ve->mdct,vec,vec);

  /*_analysis_output_always("mdct",seq2,vec,n/2,0,1,0); */

  /* near-DC spreading function; this has nothing to do with
     psychoacoustics, just sidelobe leakage and window size */
  {
    float temp=(float)(vec[0]*vec[0]+.7*vec[1]*vec[1]+.2*vec[2]*vec[2]);
    int ptr=filters->nearptr;

    /* the accumulation is regularly refreshed from scratch to avoid
       floating point creep */
    if(ptr==0){
      decay=filters->nearDC_acc=filters->nearDC_partialacc+temp;
      filters->nearDC_partialacc=temp;
    }else{
      decay=filters->nearDC_acc+=temp;
      filters->nearDC_partialacc+=temp;
    }
    filters->nearDC_acc-=filters->nearDC[ptr];
    filters->nearDC[ptr]=temp;

    decay*=(1./(VE_NEARDC+1));
    filters->nearptr++;
    if(filters->nearptr>=VE_NEARDC)filters->nearptr=0;
    decay=(float)(todB(&decay)*.5-15.f);
  }

  /* perform spreading and limiting, also smooth the spectrum.  yes,
     the MDCT results in all real coefficients, but it still *behaves*
     like real/imaginary pairs */
  for(i=0;i<n/2;i+=2){
    float val=vec[i]*vec[i]+vec[i+1]*vec[i+1];
    val=todB(&val)*.5f;
    if(val<decay)val=decay;
    if(val<minV)val=minV;
    vec[i>>1]=val;
    decay-=8.;
  }

  /*_analysis_output_always("spread",seq2++,vec,n/4,0,0,0);*/

  /* perform preecho/postecho triggering by band */
  for(j=0;j<VE_BANDS;j++){
    float acc=0.;
    float valmax,valmin;

    /* accumulate amplitude */
    for(i=0;i<bands[j].end;i++)
      acc+=vec[i+bands[j].begin]*bands[j].window[i];

    acc*=bands[j].total;

    /* convert amplitude to delta */
    {
      int p,thiscp=filters[j].ampptr;
      float postmax,postmin,premax=-99999.f,premin=99999.f;

      p=thiscp;
      p--;
      if(p<0)p+=VE_AMP;
      postmax=max(acc,filters[j].ampbuf[p]);
      postmin=min(acc,filters[j].ampbuf[p]);

      for(i=0;i<stretch;i++){
    p--;
    if(p<0)p+=VE_AMP;
    premax=max(premax,filters[j].ampbuf[p]);
    premin=min(premin,filters[j].ampbuf[p]);
      }

      valmin=postmin-premin;
      valmax=postmax-premax;

      /*filters[j].markers[pos]=valmax;*/
      filters[j].ampbuf[thiscp]=acc;
      filters[j].ampptr++;
      if(filters[j].ampptr>=VE_AMP)filters[j].ampptr=0;
    }

    /* look at min/max, decide trigger */
    if(valmax>gi->preecho_thresh[j]+penalty){
      ret|=1;
      ret|=4;
    }
    if(valmin<gi->postecho_thresh[j]-penalty)ret|=2;
  }

  return(ret);
}

#if 0
int seq=0;
ogg_int64_t totalshift=-1024;
#endif

long _ve_envelope_search(vorbis_dsp_state *v){
  vorbis_info *vi=v->vi;
  codec_setup_info *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  vorbis_info_psy_global *gi=&ci->psy_g_param;
  envelope_lookup *ve=((private_state *)(v->backend_state))->ve;
  long i,j;

  int first=ve->current/ve->searchstep;
  int last=v->pcm_current/ve->searchstep-VE_WIN;
  if(first<0)first=0;

  /* make sure we have enough storage to match the PCM */
  if(last+VE_WIN+VE_POST>ve->storage){
    ve->storage=last+VE_WIN+VE_POST; /* be sure */
    ve->mark=static_cast<int*>(_ogg_realloc(ve->mark,ve->storage*sizeof(*ve->mark)));
  }

  for(j=first;j<last;j++){
    int ret=0;

    ve->stretch++;
    if(ve->stretch>VE_MAXSTRETCH*2)
      ve->stretch=VE_MAXSTRETCH*2;

    for(i=0;i<ve->ch;i++){
      float *pcm=v->pcm[i]+ve->searchstep*(j);
      ret|=_ve_amp(ve,gi,pcm,ve->band,ve->filter+i*VE_BANDS,j);
    }

    ve->mark[j+VE_POST]=0;
    if(ret&1){
      ve->mark[j]=1;
      ve->mark[j+1]=1;
    }

    if(ret&2){
      ve->mark[j]=1;
      if(j>0)ve->mark[j-1]=1;
    }

    if(ret&4)ve->stretch=-1;
  }

  ve->current=last*ve->searchstep;

  {
    long centerW=v->centerW;
    long testW=
      centerW+
      ci->blocksizes[v->W]/4+
      ci->blocksizes[1]/2+
      ci->blocksizes[0]/4;

    j=ve->cursor;

    while(j<ve->current-(ve->searchstep)){/* account for postecho
                                             working back one window */
      if(j>=testW)return(1);

      ve->cursor=j;

      if(ve->mark[j/ve->searchstep]){
    if(j>centerW){

      #if 0
      if(j>ve->curmark){
        float *marker=alloca(v->pcm_current*sizeof(*marker));
        int l,m;
        memset(marker,0,sizeof(*marker)*v->pcm_current);
        fprintf(stderr,"mark! seq=%d, cursor:%fs time:%fs\n",
            seq,
            (totalshift+ve->cursor)/44100.,
            (totalshift+j)/44100.);
        _analysis_output_always("pcmL",seq,v->pcm[0],v->pcm_current,0,0,totalshift);
        _analysis_output_always("pcmR",seq,v->pcm[1],v->pcm_current,0,0,totalshift);

        _analysis_output_always("markL",seq,v->pcm[0],j,0,0,totalshift);
        _analysis_output_always("markR",seq,v->pcm[1],j,0,0,totalshift);

        for(m=0;m<VE_BANDS;m++){
          char buf[80];
          sprintf(buf,"delL%d",m);
          for(l=0;l<last;l++)marker[l*ve->searchstep]=ve->filter[m].markers[l]*.1;
          _analysis_output_always(buf,seq,marker,v->pcm_current,0,0,totalshift);
        }

        for(m=0;m<VE_BANDS;m++){
          char buf[80];
          sprintf(buf,"delR%d",m);
          for(l=0;l<last;l++)marker[l*ve->searchstep]=ve->filter[m+VE_BANDS].markers[l]*.1;
          _analysis_output_always(buf,seq,marker,v->pcm_current,0,0,totalshift);
        }

        for(l=0;l<last;l++)marker[l*ve->searchstep]=ve->mark[l]*.4;
        _analysis_output_always("mark",seq,marker,v->pcm_current,0,0,totalshift);


        seq++;

      }
#endif

      ve->curmark=j;
      if(j>=testW)return(1);
      return(0);
    }
      }
      j+=ve->searchstep;
    }
  }

  return(-1);
}

int _ve_envelope_mark(vorbis_dsp_state *v){
  envelope_lookup *ve=((private_state *)(v->backend_state))->ve;
  vorbis_info *vi=v->vi;
  codec_setup_info *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  long centerW=v->centerW;
  long beginW=centerW-ci->blocksizes[v->W]/4;
  long endW=centerW+ci->blocksizes[v->W]/4;
  if(v->W){
    beginW-=ci->blocksizes[v->lW]/4;
    endW+=ci->blocksizes[v->nW]/4;
  }else{
    beginW-=ci->blocksizes[0]/4;
    endW+=ci->blocksizes[0]/4;
  }

  if(ve->curmark>=beginW && ve->curmark<endW)return(1);
  {
    long first=beginW/ve->searchstep;
    long last=endW/ve->searchstep;
    long i;
    for(i=first;i<last;i++)
      if(ve->mark[i])return(1);
  }
  return(0);
}

void _ve_envelope_shift(envelope_lookup *e,long shift){
  int smallsize=e->current/e->searchstep+VE_POST; /* adjust for placing marks
                             ahead of ve->current */
  int smallshift=shift/e->searchstep;

  memmove(e->mark,e->mark+smallshift,(smallsize-smallshift)*sizeof(*e->mark));

  #if 0
  int i;
  for(i=0;i<VE_BANDS*e->ch;i++)
    memmove(e->filter[i].markers,
        e->filter[i].markers+smallshift,
        (1024-smallshift)*sizeof(*(*e->filter).markers));
  totalshift+=shift;
  #endif

  e->current-=shift;
  if(e->curmark>=0)
    e->curmark-=shift;
  e->cursor-=shift;
}










// ==========================================================================
// ==========================================================================
// codebook.c
// ==========================================================================
// ==========================================================================

/* packs the given codebook into the bitstream **************************/

int vorbis_staticbook_pack(const static_codebook *c,oggpack_buffer *opb){
  long i,j;
  int ordered=0;

  /* first the basic parameters */
  oggpack_write(opb,0x564342,24);
  oggpack_write(opb,c->dim,16);
  oggpack_write(opb,c->entries,24);

  /* pack the codewords.  There are two packings; length ordered and
     length random.  Decide between the two now. */

  for(i=1;i<c->entries;i++)
    if(c->lengthlist[i-1]==0 || c->lengthlist[i]<c->lengthlist[i-1])break;
  if(i==c->entries)ordered=1;

  if(ordered){
    /* length ordered.  We only need to say how many codewords of
       each length.  The actual codewords are generated
       deterministically */

    long count=0;
    oggpack_write(opb,1,1);  /* ordered */
    oggpack_write(opb,c->lengthlist[0]-1,5); /* 1 to 32 */

    for(i=1;i<c->entries;i++){
      long thiscp=c->lengthlist[i];
      long last=c->lengthlist[i-1];
      if(thiscp>last){
    for(j=last;j<thiscp;j++){
      oggpack_write(opb,i-count,_ilog(c->entries-count));
      count=i;
    }
      }
    }
    oggpack_write(opb,i-count,_ilog(c->entries-count));

  }else{
    /* length random.  Again, we don't code the codeword itself, just
       the length.  This time, though, we have to encode each length */
    oggpack_write(opb,0,1);   /* unordered */

    /* algortihmic mapping has use for 'unused entries', which we tag
       here.  The algorithmic mapping happens as usual, but the unused
       entry has no codeword. */
    for(i=0;i<c->entries;i++)
      if(c->lengthlist[i]==0)break;

    if(i==c->entries){
      oggpack_write(opb,0,1); /* no unused entries */
      for(i=0;i<c->entries;i++)
    oggpack_write(opb,c->lengthlist[i]-1,5);
    }else{
      oggpack_write(opb,1,1); /* we have unused entries; thus we tag */
      for(i=0;i<c->entries;i++){
    if(c->lengthlist[i]==0){
      oggpack_write(opb,0,1);
    }else{
      oggpack_write(opb,1,1);
      oggpack_write(opb,c->lengthlist[i]-1,5);
    }
      }
    }
  }

  /* is the entry number the desired return value, or do we have a
     mapping? If we have a mapping, what type? */
  oggpack_write(opb,c->maptype,4);
  switch(c->maptype){
  case 0:
    /* no mapping */
    break;
  case 1:case 2:
    /* implicitly populated value mapping */
    /* explicitly populated value mapping */

    if(!c->quantlist){
      /* no quantlist?  error */
      return(-1);
    }

    /* values that define the dequantization */
    oggpack_write(opb,c->q_min,32);
    oggpack_write(opb,c->q_delta,32);
    oggpack_write(opb,c->q_quant-1,4);
    oggpack_write(opb,c->q_sequencep,1);

    {
      int quantvals;
      switch(c->maptype){
      case 1:
    /* a single column of (c->entries/c->dim) quantized values for
       building a full value list algorithmically (square lattice) */
    quantvals=_book_maptype1_quantvals(c);
    break;
      case 2:
    /* every value (c->entries*c->dim total) specified explicitly */
    quantvals=c->entries*c->dim;
    break;
      default: /* NOT_REACHABLE */
    quantvals=-1;
      }

      /* quantized values */
      for(i=0;i<quantvals;i++)
    oggpack_write(opb,labs(c->quantlist[i]),c->q_quant);

    }
    break;
  default:
    /* error case; we don't have any other map types now */
    return(-1);
  }

  return(0);
}

/* unpacks a codebook from the packet buffer into the codebook struct,
   readies the codebook auxiliary structures for decode *************/
int vorbis_staticbook_unpack(oggpack_buffer *opb,static_codebook *s){
  long i,j;
  memset(s,0,sizeof(*s));
  s->allocedp=1;

  /* make sure alignment is correct */
  if(oggpack_read(opb,24)!=0x564342)goto _eofout;

  /* first the basic parameters */
  s->dim=oggpack_read(opb,16);
  s->entries=oggpack_read(opb,24);
  if(s->entries==-1)goto _eofout;

  /* codeword ordering.... length ordered or unordered? */
  switch((int)oggpack_read(opb,1)){
  case 0:
    /* unordered */
    s->lengthlist=static_cast<long*>(_ogg_malloc(sizeof(*s->lengthlist)*s->entries));

    /* allocated but unused entries? */
    if(oggpack_read(opb,1)){
      /* yes, unused entries */

      for(i=0;i<s->entries;i++){
    if(oggpack_read(opb,1)){
      long num=oggpack_read(opb,5);
      if(num==-1)goto _eofout;
      s->lengthlist[i]=num+1;
    }else
      s->lengthlist[i]=0;
      }
    }else{
      /* all entries used; no tagging */
      for(i=0;i<s->entries;i++){
    long num=oggpack_read(opb,5);
    if(num==-1)goto _eofout;
    s->lengthlist[i]=num+1;
      }
    }

    break;
  case 1:
    /* ordered */
    {
      long length=oggpack_read(opb,5)+1;
      s->lengthlist=static_cast<long*>(_ogg_malloc(sizeof(*s->lengthlist)*s->entries));

      for(i=0;i<s->entries;){
    long num=oggpack_read(opb,_ilog(s->entries-i));
    if(num==-1)goto _eofout;
    for(j=0;j<num && i<s->entries;j++,i++)
      s->lengthlist[i]=length;
    length++;
      }
    }
    break;
  default:
    /* EOF */
    return(-1);
  }

  /* Do we have a mapping to unpack? */
  switch((s->maptype=oggpack_read(opb,4))){
  case 0:
    /* no mapping */
    break;
  case 1: case 2:
    /* implicitly populated value mapping */
    /* explicitly populated value mapping */

    s->q_min=oggpack_read(opb,32);
    s->q_delta=oggpack_read(opb,32);
    s->q_quant=oggpack_read(opb,4)+1;
    s->q_sequencep=oggpack_read(opb,1);

    {
      int quantvals=0;
      switch(s->maptype){
      case 1:
    quantvals=_book_maptype1_quantvals(s);
    break;
      case 2:
    quantvals=s->entries*s->dim;
    break;
      }

      /* quantized values */
      s->quantlist=static_cast<long*>(_ogg_malloc(sizeof(*s->quantlist)*quantvals));
      for(i=0;i<quantvals;i++)
    s->quantlist[i]=oggpack_read(opb,s->q_quant);

      if(quantvals&&s->quantlist[quantvals-1]==-1)goto _eofout;
    }
    break;
  default:
    goto _errout;
  }

  /* all set */
  return(0);

 _errout:
 _eofout:
  vorbis_staticbook_clear(s);
  return(-1);
}

/* returns the number of bits ************************************************/
int vorbis_book_encode(codebook *book, int a, oggpack_buffer *b){
  oggpack_write(b,book->codelist[a],book->c->lengthlist[a]);
  return(book->c->lengthlist[a]);
}

/* One the encode side, our vector writers are each designed for a
specific purpose, and the encoder is not flexible without modification:

The LSP vector coder uses a single stage nearest-match with no
interleave, so no step and no error return.  This is specced by floor0
and doesn't change.

Residue0 encoding interleaves, uses multiple stages, and each stage
peels of a specific amount of resolution from a lattice (thus we want
to match by threshold, not nearest match).  Residue doesn't *have* to
be encoded that way, but to change it, one will need to add more
infrastructure on the encode side (decode side is specced and simpler) */

/* floor0 LSP (single stage, non interleaved, nearest match) */
/* returns entry number and *modifies a* to the quantization value *****/
int vorbis_book_errorv(codebook *book,float *a){
  int dim=book->dim,k;
  int best=_best(book,a,1);
  for(k=0;k<dim;k++)
    a[k]=(book->valuelist+best*dim)[k];
  return(best);
}

/* returns the number of bits and *modifies a* to the quantization value *****/
int vorbis_book_encodev(codebook *book,int best,float *a,oggpack_buffer *b){
  int k,dim=book->dim;
  for(k=0;k<dim;k++)
    a[k]=(book->valuelist+best*dim)[k];
  return(vorbis_book_encode(book,best,b));
}

/* the 'eliminate the decode tree' optimization actually requires the
   codewords to be MSb first, not LSb.  This is an annoying inelegancy
   (and one of the first places where carefully thought out design
   turned out to be wrong; Vorbis II and future Ogg codecs should go
   to an MSb bitpacker), but not actually the huge hit it appears to
   be.  The first-stage decode table catches most words so that
   bitreverse is not in the main execution path. */


STLOOPIN long decode_packed_entry_number(codebook *book, oggpack_buffer *b){
  int  read=book->dec_maxlength;
  long lo,hi;
  long lok = oggpack_look(b,book->dec_firsttablen);

  if (lok >= 0) {
    long entry = book->dec_firsttable[lok];
    if(entry&0x80000000UL){
      lo=(entry>>15)&0x7fff;
      hi=book->used_entries-(entry&0x7fff);
    }else{
      oggpack_adv(b, book->dec_codelengths[entry-1]);
      return(entry-1);
    }
  }else{
    lo=0;
    hi=book->used_entries;
  }

  lok = oggpack_look(b, read);

  while(lok<0 && read>1)
    lok = oggpack_look(b, --read);
  if(lok<0)return -1;

  /* bisect search for the codeword in the ordered list */
  {
    ogg_uint32_t testword=bitreverse((ogg_uint32_t)lok);

    while(hi-lo>1){
      long p=(hi-lo)>>1;
      long test=book->codelist[lo+p]>testword;
      lo+=p&(test-1);
      hi-=p&(-test);
    }

    if(book->dec_codelengths[lo]<=read){
      oggpack_adv(b, book->dec_codelengths[lo]);
      return(lo);
    }
  }

  oggpack_adv(b, read);
  return(-1);
}

/* Decode side is specced and easier, because we don't need to find
   matches using different criteria; we simply read and map.  There are
   two things we need to do 'depending':

   We may need to support interleave.  We don't really, but it's
   convenient to do it here rather than rebuild the vector later.

   Cascades may be additive or multiplicitive; this is not inherent in
   the codebook, but set in the code using the codebook.  Like
   interleaving, it's easiest to do it here.
   addmul==0 -> declarative (set the value)
   addmul==1 -> additive
   addmul==2 -> multiplicitive */

/* returns the [original, not compacted] entry number or -1 on eof *********/
long vorbis_book_decode(codebook *book, oggpack_buffer *b){
  long packed_entry=decode_packed_entry_number(book,b);
  if(packed_entry>=0)
    return(book->dec_index[packed_entry]);

  /* if there's no dec_index, the codebook unpacking isn't collapsed */
  return(packed_entry);
}

/* returns 0 on OK or -1 on eof *************************************/
long vorbis_book_decodevs_add(codebook *book,float *a,oggpack_buffer *b,int n){
  int step=n/book->dim;
  long *entry = static_cast<long*>(alloca(sizeof(*entry)*step));
  float **t = static_cast<float**>(alloca(sizeof(*t)*step));
  int i,j,o;

  for (i = 0; i < step; i++) {
    entry[i]=decode_packed_entry_number(book,b);
    if(entry[i]==-1)return(-1);
    t[i] = book->valuelist+entry[i]*book->dim;
  }
  for(i=0,o=0;i<book->dim;i++,o+=step)
    for (j=0;j<step;j++)
      a[o+j]+=t[j][i];
  return(0);
}

long vorbis_book_decodev_add(codebook *book,float *a,oggpack_buffer *b,int n){
  int i,j,entry;
  float *t;

  if(book->dim>8){
    for(i=0;i<n;){
      entry = decode_packed_entry_number(book,b);
      if(entry==-1)return(-1);
      t     = book->valuelist+entry*book->dim;
      for (j=0;j<book->dim;)
    a[i++]+=t[j++];
    }
  }else{
    for(i=0;i<n;){
      entry = decode_packed_entry_number(book,b);
      if(entry==-1)return(-1);
      t     = book->valuelist+entry*book->dim;
      j=0;
      switch((int)book->dim){
      case 8:
    a[i++]+=t[j++];
      case 7:
    a[i++]+=t[j++];
      case 6:
    a[i++]+=t[j++];
      case 5:
    a[i++]+=t[j++];
      case 4:
    a[i++]+=t[j++];
      case 3:
    a[i++]+=t[j++];
      case 2:
    a[i++]+=t[j++];
      case 1:
    a[i++]+=t[j++]; j; // unused?
      case 0:
    break;
      }
    }
  }
  return(0);
}

long vorbis_book_decodev_set(codebook *book,float *a,oggpack_buffer *b,int n){
  int i,j,entry;
  float *t;

  for(i=0;i<n;){
    entry = decode_packed_entry_number(book,b);
    if(entry==-1)return(-1);
    t     = book->valuelist+entry*book->dim;
    for (j=0;j<book->dim;)
      a[i++]=t[j++];
  }
  return(0);
}

long vorbis_book_decodevv_add(codebook *book,float **a,long offset,int ch,
                  oggpack_buffer *b,int n){
  long i,j,entry;
  int chptr=0;

  for(i=offset/ch;i<(offset+n)/ch;){
    entry = decode_packed_entry_number(book,b);
    if(entry==-1)return(-1);
    {
      const float *t = book->valuelist+entry*book->dim;
      for (j=0;j<book->dim;j++){
    a[chptr++][i]+=t[j];
    if(chptr==ch){
      chptr=0;
      i++;
    }
      }
    }
  }
  return(0);
}









// ==========================================================================
// ==========================================================================
// block.c
// ==========================================================================
// ==========================================================================
// Handle windowing, overlap-add, etc of the PCM vectors.  This is made
// more amusing by Vorbis' current two allowed block sizes.



/* pcm accumulator examples (not exhaustive):

 <-------------- lW ---------------->
                   <--------------- W ---------------->
:            .....|.....       _______________         |
:        .'''     |     '''_---      |       |\        |
:.....'''         |_____--- '''......|       | \_______|
:.................|__________________|_______|__|______|
                  |<------ Sl ------>|      > Sr <     |endW
                  |beginSl           |endSl  |  |endSr
                  |beginW            |endlW  |beginSr


                      |< lW >|
                   <--------------- W ---------------->
                  |   |  ..  ______________            |
                  |   | '  `/        |     ---_        |
                  |___.'___/`.       |         ---_____|
                  |_______|__|_______|_________________|
                  |      >|Sl|<      |<------ Sr ----->|endW
                  |       |  |endSl  |beginSr          |endSr
                  |beginW |  |endlW
                  mult[0] |beginSl                     mult[n]

 <-------------- lW ----------------->
                          |<--W-->|
:            ..............  ___  |   |
:        .'''             |`/   \ |   |
:.....'''                 |/`....\|...|
:.........................|___|___|___|
                          |Sl |Sr |endW
                          |   |   |endSr
                          |   |beginSr
                          |   |endSl
              |beginSl
              |beginW
*/

/* block abstraction setup *********************************************/

#ifndef WORD_ALIGN
#define WORD_ALIGN 8
#endif

int vorbis_block_init(vorbis_dsp_state *v, vorbis_block *vb){
  memset(vb,0,sizeof(*vb));
  vb->vd=v;
  vb->localalloc=0;
  vb->localstore=NULL;
  if(v->analysisp){
    vorbis_block_internal *vbi=static_cast<vorbis_block_internal*>(_ogg_calloc(1,sizeof(vorbis_block_internal)));
    vb->internal=vbi;
    oggpack_writeinit(&vb->opb);
    vbi->ampmax=-9999;
  }

  return(0);
}

void *_vorbis_block_alloc(vorbis_block *vb,long bytes){
  bytes=(bytes+(WORD_ALIGN-1)) & ~(WORD_ALIGN-1);
  if(bytes+vb->localtop>vb->localalloc){
    /* can't just _ogg_realloc... there are outstanding pointers */
    if(vb->localstore){
      struct alloc_chain *link=static_cast<alloc_chain*>(_ogg_malloc(sizeof(*link)));
      vb->totaluse+=vb->localtop;
      link->next=vb->reap;
      link->ptr=vb->localstore;
      vb->reap=link;
    }
    /* highly conservative */
    vb->localalloc=bytes;
    vb->localstore=_ogg_malloc(vb->localalloc);
    vb->localtop=0;
  }
  {
    void *ret=(void *)(((char *)vb->localstore)+vb->localtop);
    vb->localtop+=bytes;
    return ret;
  }
}

/* reap the chain, pull the ripcord */
void _vorbis_block_ripcord(vorbis_block *vb){
  /* reap the chain */
  struct alloc_chain *reap=vb->reap;
  while(reap){
    struct alloc_chain *next=reap->next;
    _ogg_free(reap->ptr);
    memset(reap,0,sizeof(*reap));
    _ogg_free(reap);
    reap=next;
  }
  /* consolidate storage */
  if(vb->totaluse){
    vb->localstore=_ogg_realloc(vb->localstore,vb->totaluse+vb->localalloc);
    vb->localalloc+=vb->totaluse;
    vb->totaluse=0;
  }

  /* pull the ripcord */
  vb->localtop=0;
  vb->reap=NULL;
}

int vorbis_block_clear(vorbis_block *vb){
  if(vb->vd)
    if(vb->vd->analysisp)
      oggpack_writeclear(&vb->opb);
  _vorbis_block_ripcord(vb);
  if(vb->localstore)_ogg_free(vb->localstore);

  if(vb->internal)
    _ogg_free(vb->internal);

  memset(vb,0,sizeof(*vb));
  return(0);
}

/* Analysis side code, but directly related to blocking.  Thus it's
   here and not in analysis.c (which is for analysis transforms only).
   The init is here because some of it is shared */

int _vds_shared_init(vorbis_dsp_state *v,vorbis_info *vi,int encp){
  int i;
  codec_setup_info *ci=static_cast<codec_setup_info*>(vi->codec_setup);

  memset(v,0,sizeof(*v));
  private_state *b=static_cast<private_state*>(_ogg_calloc(1,sizeof(*b)));
  v->backend_state=b;

  v->vi=vi;
  b->modebits=ilog2(ci->modes);

  b->transform[0]=static_cast<vorbis_look_transform**>(_ogg_calloc(VI_TRANSFORMB,sizeof(*b->transform[0])));
  b->transform[1]=static_cast<vorbis_look_transform**>(_ogg_calloc(VI_TRANSFORMB,sizeof(*b->transform[1])));

  /* MDCT is tranform 0 */

  b->transform[0][0]=_ogg_calloc(1,sizeof(mdct_lookup));
  b->transform[1][0]=_ogg_calloc(1,sizeof(mdct_lookup));
  mdct_init(static_cast<mdct_lookup*>(b->transform[0][0]),ci->blocksizes[0]);
  mdct_init(static_cast<mdct_lookup*>(b->transform[1][0]),ci->blocksizes[1]);

  /* Vorbis I uses only window type 0 */
  b->window[0]=_vorbis_window(0,ci->blocksizes[0]/2);
  b->window[1]=_vorbis_window(0,ci->blocksizes[1]/2);

  if(encp){ /* encode/decode differ here */

    /* analysis always needs an fft */
    drft_init(&b->fft_look[0],ci->blocksizes[0]);
    drft_init(&b->fft_look[1],ci->blocksizes[1]);

    /* finish the codebooks */
    if(!ci->fullbooks){
      ci->fullbooks=static_cast<codebook*>(_ogg_calloc(ci->books,sizeof(*ci->fullbooks)));
      for(i=0;i<ci->books;i++)
    vorbis_book_init_encode(ci->fullbooks+i,ci->book_param[i]);
    }

    b->psy=static_cast<vorbis_look_psy*>(_ogg_calloc(ci->psys,sizeof(*b->psy)));
    for(i=0;i<ci->psys;i++){
      _vp_psy_init(b->psy+i,
           ci->psy_param[i],
           &ci->psy_g_param,
           ci->blocksizes[ci->psy_param[i]->blockflag]/2,
           vi->rate);
    }

    v->analysisp=1;
  }else{
    /* finish the codebooks */
    if(!ci->fullbooks){
      ci->fullbooks=static_cast<codebook*>(_ogg_calloc(ci->books,sizeof(*ci->fullbooks)));
      for(i=0;i<ci->books;i++){
    vorbis_book_init_decode(ci->fullbooks+i,ci->book_param[i]);
    /* decode codebooks are now standalone after init */
    vorbis_staticbook_destroy(ci->book_param[i]);
    ci->book_param[i]=NULL;
      }
    }
  }

  /* initialize the storage vectors. blocksize[1] is small for encode,
     but the correct size for decode */
  v->pcm_storage=ci->blocksizes[1];
  v->pcm=static_cast<float**>(_ogg_malloc(vi->channels*sizeof(*v->pcm)));
  v->pcmret=static_cast<float**>(_ogg_malloc(vi->channels*sizeof(*v->pcmret)));
  {
    int i;
    for(i=0;i<vi->channels;i++)
      v->pcm[i]=static_cast<float*>(_ogg_calloc(v->pcm_storage,sizeof(*v->pcm[i])));
  }

  /* all 1 (large block) or 0 (small block) */
  /* explicitly set for the sake of clarity */
  v->lW=0; /* previous window size */
  v->W=0;  /* current window size */

  /* all vector indexes */
  v->centerW=ci->blocksizes[1]/2;

  v->pcm_current=v->centerW;

  /* initialize all the backend lookups */
  b->flr=static_cast<vorbis_look_floor**>(_ogg_calloc(ci->floors,sizeof(*b->flr)));
  b->residue=static_cast<vorbis_look_residue**>(_ogg_calloc(ci->residues,sizeof(*b->residue)));

  for(i=0;i<ci->floors;i++)
    b->flr[i]=_floor_P[ci->floor_type[i]]->
      look(v,ci->floor_param[i]);

  for(i=0;i<ci->residues;i++)
    b->residue[i]=_residue_P[ci->residue_type[i]]->
      look(v,ci->residue_param[i]);

  return(0);
}

/* arbitrary settings and spec-mandated numbers get filled in here */
int vorbis_analysis_init(vorbis_dsp_state *v,vorbis_info *vi){
  _vds_shared_init(v,vi,1);
  private_state *b=static_cast<private_state*>(v->backend_state);
  b->psy_g_look=_vp_global_look(vi);

  /* Initialize the envelope state storage */
  b->ve=static_cast<envelope_lookup*>(_ogg_calloc(1,sizeof(*b->ve)));
  _ve_envelope_init(b->ve,vi);

  vorbis_bitrate_init(vi,&b->bms);

  return(0);
}

void vorbis_dsp_clear(vorbis_dsp_state *v){
  int i;
  if(v){
    vorbis_info *vi=v->vi;
    codec_setup_info *ci=static_cast<codec_setup_info*>((vi?vi->codec_setup:NULL));
    private_state *b=static_cast<private_state*>(v->backend_state);

    if(b){
      if(b->window[0])
    _ogg_free(b->window[0]);
      if(b->window[1])
    _ogg_free(b->window[1]);

      if(b->ve){
    _ve_envelope_clear(b->ve);
    _ogg_free(b->ve);
      }

      if(b->transform[0]){
    mdct_clear(static_cast<mdct_lookup*>(b->transform[0][0]));
    _ogg_free(b->transform[0][0]);
    _ogg_free(b->transform[0]);
      }
      if(b->transform[1]){
    mdct_clear(static_cast<mdct_lookup*>(b->transform[1][0]));
    _ogg_free(b->transform[1][0]);
    _ogg_free(b->transform[1]);
      }

      if(b->flr){
    for(i=0;i<ci->floors;i++)
      _floor_P[ci->floor_type[i]]->
        free_look(b->flr[i]);
    _ogg_free(b->flr);
      }
      if(b->residue){
    for(i=0;i<ci->residues;i++)
      _residue_P[ci->residue_type[i]]->
        free_look(b->residue[i]);
    _ogg_free(b->residue);
      }
      if(b->psy){
    for(i=0;i<ci->psys;i++)
      _vp_psy_clear(b->psy+i);
    _ogg_free(b->psy);
      }

      if(b->psy_g_look)_vp_global_free(b->psy_g_look);
      vorbis_bitrate_clear(&b->bms);

      drft_clear(&b->fft_look[0]);
      drft_clear(&b->fft_look[1]);

    }

    if(v->pcm){
      for(i=0;i<vi->channels;i++)
    if(v->pcm[i])_ogg_free(v->pcm[i]);
      _ogg_free(v->pcm);
      if(v->pcmret)_ogg_free(v->pcmret);
    }

    if(b){
      /* free header, header1, header2 */
      if(b->header)_ogg_free(b->header);
      if(b->header1)_ogg_free(b->header1);
      if(b->header2)_ogg_free(b->header2);
      _ogg_free(b);
    }

    memset(v,0,sizeof(*v));
  }
}

float **vorbis_analysis_buffer(vorbis_dsp_state *v, int vals){
  int i;
  vorbis_info *vi=v->vi;
  private_state *b=static_cast<private_state*>(v->backend_state);

  /* free header, header1, header2 */
  if(b->header)_ogg_free(b->header);b->header=NULL;
  if(b->header1)_ogg_free(b->header1);b->header1=NULL;
  if(b->header2)_ogg_free(b->header2);b->header2=NULL;

  /* Do we have enough storage space for the requested buffer? If not,
     expand the PCM (and envelope) storage */

  if(v->pcm_current+vals>=v->pcm_storage){
    v->pcm_storage=v->pcm_current+vals*2;

    for(i=0;i<vi->channels;i++){
      v->pcm[i]=static_cast<float*>(_ogg_realloc(v->pcm[i],v->pcm_storage*sizeof(*v->pcm[i])));
    }
  }

  for(i=0;i<vi->channels;i++)
    v->pcmret[i]=v->pcm[i]+v->pcm_current;

  return(v->pcmret);
}

void _preextrapolate_helper(vorbis_dsp_state *v){
  int i;
  int order=32;
  float *lpc=static_cast<float*>(alloca(order*sizeof(*lpc)));
  float *work=static_cast<float*>(alloca(v->pcm_current*sizeof(*work)));
  long j;
  v->preextrapolate=1;

  if(v->pcm_current-v->centerW>order*2){ /* safety */
    for(i=0;i<v->vi->channels;i++){
      /* need to run the extrapolation in reverse! */
      for(j=0;j<v->pcm_current;j++)
    work[j]=v->pcm[i][v->pcm_current-j-1];

      /* prime as above */
      vorbis_lpc_from_data(work,lpc,v->pcm_current-v->centerW,order);

      /* run the predictor filter */
      vorbis_lpc_predict(lpc,work+v->pcm_current-v->centerW-order,
             order,
             work+v->pcm_current-v->centerW,
             v->centerW);

      for(j=0;j<v->pcm_current;j++)
    v->pcm[i][v->pcm_current-j-1]=work[j];

    }
  }
}


/* call with val<=0 to set eof */

int vorbis_analysis_wrote(vorbis_dsp_state *v, int vals){
  vorbis_info *vi=v->vi;
  codec_setup_info *ci=static_cast<codec_setup_info*>(vi->codec_setup);

  if(vals<=0){
    int order=32;
    int i;
    float *lpc=static_cast<float*>(alloca(order*sizeof(*lpc)));

    /* if it wasn't done earlier (very short sample) */
    if(!v->preextrapolate)
      _preextrapolate_helper(v);

    /* We're encoding the end of the stream.  Just make sure we have
       [at least] a few full blocks of zeroes at the end. */
    /* actually, we don't want zeroes; that could drop a large
       amplitude off a cliff, creating spread spectrum noise that will
       suck to encode.  Extrapolate for the sake of cleanliness. */

    vorbis_analysis_buffer(v,ci->blocksizes[1]*3);
    v->eofflag=v->pcm_current;
    v->pcm_current+=ci->blocksizes[1]*3;

    for(i=0;i<vi->channels;i++){
      if(v->eofflag>order*2){
    /* extrapolate with LPC to fill in */
    long n;

    /* make a predictor filter */
    n=v->eofflag;
    if(n>ci->blocksizes[1])n=ci->blocksizes[1];
    vorbis_lpc_from_data(v->pcm[i]+v->eofflag-n,lpc,n,order);

    /* run the predictor filter */
    vorbis_lpc_predict(lpc,v->pcm[i]+v->eofflag-order,order,
               v->pcm[i]+v->eofflag,v->pcm_current-v->eofflag);
      }else{
    /* not enough data to extrapolate (unlikely to happen due to
           guarding the overlap, but bulletproof in case that
           assumtion goes away). zeroes will do. */
    memset(v->pcm[i]+v->eofflag,0,
           (v->pcm_current-v->eofflag)*sizeof(*v->pcm[i]));

      }
    }
  }else{

    if(v->pcm_current+vals>v->pcm_storage)
      return(OV_EINVAL);

    v->pcm_current+=vals;

    /* we may want to reverse extrapolate the beginning of a stream
       too... in case we're beginning on a cliff! */
    /* clumsy, but simple.  It only runs once, so simple is good. */
    if(!v->preextrapolate && v->pcm_current-v->centerW>ci->blocksizes[1])
      _preextrapolate_helper(v);

  }
  return(0);
}

/* do the deltas, envelope shaping, pre-echo and determine the size of
   the next block on which to continue analysis */
int vorbis_analysis_blockout(vorbis_dsp_state *v,vorbis_block *vb){
  int i;
  vorbis_info *vi=v->vi;
  codec_setup_info *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  private_state *b=static_cast<private_state*>(v->backend_state);
  vorbis_look_psy_global *g=b->psy_g_look;
  long beginW=v->centerW-ci->blocksizes[v->W]/2,centerNext;
  vorbis_block_internal *vbi=(vorbis_block_internal *)vb->internal;

  /* check to see if we're started... */
  if(!v->preextrapolate)return(0);

  /* check to see if we're done... */
  if(v->eofflag==-1)return(0);

  /* By our invariant, we have lW, W and centerW set.  Search for
     the next boundary so we can determine nW (the next window size)
     which lets us compute the shape of the current block's window */

  /* we do an envelope search even on a single blocksize; we may still
     be throwing more bits at impulses, and envelope search handles
     marking impulses too. */
  {
    long bp=_ve_envelope_search(v);
    if(bp==-1){

      if(v->eofflag==0)return(0); /* not enough data currently to search for a
                     full long block */
      v->nW=0;
    }else{

      if(ci->blocksizes[0]==ci->blocksizes[1])
    v->nW=0;
      else
    v->nW=bp;
    }
  }

  centerNext=v->centerW+ci->blocksizes[v->W]/4+ci->blocksizes[v->nW]/4;

  {
    /* center of next block + next block maximum right side. */

    long blockbound=centerNext+ci->blocksizes[v->nW]/2;
    if(v->pcm_current<blockbound)return(0); /* not enough data yet;
                                               although this check is
                                               less strict that the
                                               _ve_envelope_search,
                                               the search is not run
                                               if we only use one
                                               block size */


  }

  /* fill in the block.  Note that for a short window, lW and nW are *short*
     regardless of actual settings in the stream */

  _vorbis_block_ripcord(vb);
  vb->lW=v->lW;
  vb->W=v->W;
  vb->nW=v->nW;

  if(v->W){
    if(!v->lW || !v->nW){
      vbi->blocktype=BLOCKTYPE_TRANSITION;
      /*fprintf(stderr,"-");*/
    }else{
      vbi->blocktype=BLOCKTYPE_LONG;
      /*fprintf(stderr,"_");*/
    }
  }else{
    if(_ve_envelope_mark(v)){
      vbi->blocktype=BLOCKTYPE_IMPULSE;
      /*fprintf(stderr,"|");*/

    }else{
      vbi->blocktype=BLOCKTYPE_PADDING;
      /*fprintf(stderr,".");*/

    }
  }

  vb->vd=v;
  vb->sequence=v->sequence++;
  vb->granulepos=v->granulepos;
  vb->pcmend=ci->blocksizes[v->W];

  /* copy the vectors; this uses the local storage in vb */

  /* this tracks 'strongest peak' for later psychoacoustics */
  /* moved to the global psy state; clean this mess up */
  if(vbi->ampmax>g->ampmax)g->ampmax=vbi->ampmax;
  g->ampmax=_vp_ampmax_decay(g->ampmax,v);
  vbi->ampmax=g->ampmax;

  vb->pcm=static_cast<float**>(_vorbis_block_alloc(vb,sizeof(*vb->pcm)*vi->channels));
  vbi->pcmdelay=static_cast<float**>(_vorbis_block_alloc(vb,sizeof(*vbi->pcmdelay)*vi->channels));
  for(i=0;i<vi->channels;i++){
    vbi->pcmdelay[i]=static_cast<float*>(
      _vorbis_block_alloc(vb,(vb->pcmend+beginW)*sizeof(*vbi->pcmdelay[i])));
    memcpy(vbi->pcmdelay[i],v->pcm[i],(vb->pcmend+beginW)*sizeof(*vbi->pcmdelay[i]));
    vb->pcm[i]=vbi->pcmdelay[i]+beginW;

    /* before we added the delay
       vb->pcm[i]=_vorbis_block_alloc(vb,vb->pcmend*sizeof(*vb->pcm[i]));
       memcpy(vb->pcm[i],v->pcm[i]+beginW,ci->blocksizes[v->W]*sizeof(*vb->pcm[i]));
    */

  }

  /* handle eof detection: eof==0 means that we've not yet received EOF
                           eof>0  marks the last 'real' sample in pcm[]
                           eof<0  'no more to do'; doesn't get here */

  if(v->eofflag){
    if(v->centerW>=v->eofflag){
      v->eofflag=-1;
      vb->eofflag=1;
      return(1);
    }
  }

  /* advance storage vectors and clean up */
  {
    int new_centerNext=ci->blocksizes[1]/2;
    int movementW=centerNext-new_centerNext;

    if(movementW>0){

      _ve_envelope_shift(b->ve,movementW);
      v->pcm_current-=movementW;

      for(i=0;i<vi->channels;i++)
    memmove(v->pcm[i],v->pcm[i]+movementW,
        v->pcm_current*sizeof(*v->pcm[i]));


      v->lW=v->W;
      v->W=v->nW;
      v->centerW=new_centerNext;

      if(v->eofflag){
    v->eofflag-=movementW;
    if(v->eofflag<=0)v->eofflag=-1;
    /* do not add padding to end of stream! */
    if(v->centerW>=v->eofflag){
      v->granulepos+=movementW-(v->centerW-v->eofflag);
    }else{
      v->granulepos+=movementW;
    }
      }else{
    v->granulepos+=movementW;
      }
    }
  }

  /* done */
  return(1);
}

int vorbis_synthesis_init(vorbis_dsp_state *v,vorbis_info *vi){
  _vds_shared_init(v,vi,0);

  v->pcm_returned=-1;
  v->granulepos=-1;
  v->sequence=-1;
  ((private_state *)(v->backend_state))->sample_count=-1;

  return(0);
}

/* Unlike in analysis, the window is only partially applied for each
   block.  The time domain envelope is not yet handled at the point of
   calling (as it relies on the previous block). */

int vorbis_synthesis_blockin(vorbis_dsp_state *v,vorbis_block *vb){
  vorbis_info *vi=v->vi;
  codec_setup_info *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  private_state *b=static_cast<private_state*>(v->backend_state);
  int i,j;

  if(!vb)return(OV_EINVAL);
  if(v->pcm_current>v->pcm_returned  && v->pcm_returned!=-1)return(OV_EINVAL);

  v->lW=v->W;
  v->W=vb->W;
  v->nW=-1;

  if((v->sequence==-1)||
     (v->sequence+1 != vb->sequence)){
    v->granulepos=-1; /* out of sequence; lose count */
    b->sample_count=-1;
  }

  v->sequence=vb->sequence;

  if(vb->pcm){  /* not pcm to process if vorbis_synthesis_trackonly
           was called on block */
    int n=ci->blocksizes[v->W]/2;
    int n0=ci->blocksizes[0]/2;
    int n1=ci->blocksizes[1]/2;

    int thisCenter;
    int prevCenter;

    v->glue_bits+=vb->glue_bits;
    v->time_bits+=vb->time_bits;
    v->floor_bits+=vb->floor_bits;
    v->res_bits+=vb->res_bits;

    if(v->centerW){
      thisCenter=n1;
      prevCenter=0;
    }else{
    thisCenter=0;
    prevCenter=n1;
    }

    /* v->pcm is now used like a two-stage double buffer.  We don't want
       to have to constantly shift *or* adjust memory usage.  Don't
       accept a new block until the old is shifted out */

    /* overlap/add PCM */

    for(j=0;j<vi->channels;j++){
      /* the overlap/add section */
      if(v->lW){
    if(v->W){
      /* large/large */
      float *pcm=v->pcm[j]+prevCenter;
      float *p=vb->pcm[j];
      for(i=0;i<n1;i++)
        pcm[i]+=p[i];
    }else{
      /* large/small */
      float *pcm=v->pcm[j]+prevCenter+n1/2-n0/2;
      float *p=vb->pcm[j];
      for(i=0;i<n0;i++)
        pcm[i]+=p[i];
    }
      }else{
    if(v->W){
      /* small/large */
      float *pcm=v->pcm[j]+prevCenter;
      float *p=vb->pcm[j]+n1/2-n0/2;
      for(i=0;i<n0;i++)
        pcm[i]+=p[i];
      for(;i<n1/2+n0/2;i++)
        pcm[i]=p[i];
    }else{
      /* small/small */
      float *pcm=v->pcm[j]+prevCenter;
      float *p=vb->pcm[j];
      for(i=0;i<n0;i++)
        pcm[i]+=p[i];
    }
      }

      /* the copy section */
      {
    float *pcm=v->pcm[j]+thisCenter;
    float *p=vb->pcm[j]+n;
    for(i=0;i<n;i++)
      pcm[i]=p[i];
      }
    }

    if(v->centerW)
      v->centerW=0;
    else
      v->centerW=n1;

    /* deal with initial packet state; we do this using the explicit
       pcm_returned==-1 flag otherwise we're sensitive to first block
       being short or long */

    if(v->pcm_returned==-1){
      v->pcm_returned=thisCenter;
      v->pcm_current=thisCenter;
    }else{
      v->pcm_returned=prevCenter;
      v->pcm_current=prevCenter+
    ci->blocksizes[v->lW]/4+
    ci->blocksizes[v->W]/4;
    }

  }

  /* track the frame number... This is for convenience, but also
     making sure our last packet doesn't end with added padding.  If
     the last packet is partial, the number of samples we'll have to
     return will be past the vb->granulepos.

     This is not foolproof!  It will be confused if we begin
     decoding at the last page after a seek or hole.  In that case,
     we don't have a starting point to judge where the last frame
     is.  For this reason, vorbisfile will always try to make sure
     it reads the last two marked pages in proper sequence */

  if(b->sample_count==-1){
    b->sample_count=0;
  }else{
    b->sample_count+=ci->blocksizes[v->lW]/4+ci->blocksizes[v->W]/4;
  }

  if(v->granulepos==-1){
    if(vb->granulepos!=-1){ /* only set if we have a position to set to */

      v->granulepos=vb->granulepos;

      /* is this a short page? */
      if(b->sample_count>v->granulepos){
    /* corner case; if this is both the first and last audio page,
       then spec says the end is cut, not beginning */
    if(vb->eofflag){
      /* trim the end */
      /* no preceeding granulepos; assume we started at zero (we'd
         have to in a short single-page stream) */
      /* granulepos could be -1 due to a seek, but that would result
         in a long coun`t, not short count */

      v->pcm_current-=(int)(b->sample_count-v->granulepos);
    }else{
      /* trim the beginning */
      v->pcm_returned+=(int)(b->sample_count-v->granulepos);
      if(v->pcm_returned>v->pcm_current)
        v->pcm_returned=v->pcm_current;
    }

      }

    }
  }else{
    v->granulepos+=ci->blocksizes[v->lW]/4+ci->blocksizes[v->W]/4;
    if(vb->granulepos!=-1 && v->granulepos!=vb->granulepos){

      if(v->granulepos>vb->granulepos){
    long extra=(long)(v->granulepos-vb->granulepos);

    if(extra)
      if(vb->eofflag){
        /* partial last frame.  Strip the extra samples off */
        v->pcm_current-=extra;
      } /* else {Shouldn't happen *unless* the bitstream is out of
           spec.  Either way, believe the bitstream } */
      } /* else {Shouldn't happen *unless* the bitstream is out of
       spec.  Either way, believe the bitstream } */
      v->granulepos=vb->granulepos;
    }
  }

  /* Update, cleanup */

  if(vb->eofflag)v->eofflag=1;
  return(0);

}

/* pcm==NULL indicates we just want the pending samples, no more */
int vorbis_synthesis_pcmout(vorbis_dsp_state *v,float ***pcm){
  vorbis_info *vi=v->vi;
  if(v->pcm_returned>-1 && v->pcm_returned<v->pcm_current){
    if(pcm){
      int i;
      for(i=0;i<vi->channels;i++)
    v->pcmret[i]=v->pcm[i]+v->pcm_returned;
      *pcm=v->pcmret;
    }
    return(v->pcm_current-v->pcm_returned);
  }
  return(0);
}

int vorbis_synthesis_read(vorbis_dsp_state *v,int n){
  if(n && v->pcm_returned+n>v->pcm_current)return(OV_EINVAL);
  v->pcm_returned+=n;
  return(0);
}











// ==========================================================================
// ==========================================================================
// bitrate.c
// ==========================================================================
// ==========================================================================

long BINBYTES(bitrate_manager_state *bm,long pos,long bin){
  int bins=bm->queue_bins;
  return(bm->queue_binned[pos*bins+bin]);
}

#define LIMITBYTES(pos,bin) (bm->minmax_binstack[(pos)*bins*2+((bin)+bins)])

long LACING_ADJUST(long bytes){
  int addto=bytes/255+1;
  return(bytes+addto);
}

int floater_interpolate(bitrate_manager_state *bm,vorbis_info *vi,
                  double desired_rate){
  int bin=(int)rint(bm->avgfloat);
  double lobitrate,hibitrate;


  lobitrate=(double)(bm->avg_binacc[bin]*8)/bm->avg_sampleacc*vi->rate;
  while(lobitrate>desired_rate && bin>0){
    bin--;
    lobitrate=(double)(bm->avg_binacc[bin]*8)/bm->avg_sampleacc*vi->rate;
  }

  if(bin+1<bm->queue_bins){
    hibitrate=(double)(bm->avg_binacc[bin+1]*8)/bm->avg_sampleacc*vi->rate;
    if(fabs(hibitrate-desired_rate) < fabs(lobitrate-desired_rate))bin++;
  }
  return(bin);
}

/* try out a new limit */
long limit_sum(bitrate_manager_state *bm,int limit){
  int i=bm->minmax_stackptr;
  long acc=bm->minmax_acctotal;
  long bins=bm->queue_bins;

  acc-=LIMITBYTES(i,0);
  acc+=LIMITBYTES(i,limit);

  while(i-->0){
    if(bm->minmax_limitstack[i]<=limit)break;
    acc-=LIMITBYTES(i,bm->minmax_limitstack[i]);
    acc+=LIMITBYTES(i,limit);
  }
  return(acc);
}

/* compute bitrate tracking setup, allocate circular packet size queue */
void vorbis_bitrate_init(vorbis_info *vi,bitrate_manager_state *bm){
  int i;
  codec_setup_info *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  bitrate_manager_info *bi=&ci->bi;
  long maxlatency;

  memset(bm,0,sizeof(*bm));

  if(bi){

    bm->avg_sampledesired=(ogg_uint32_t)(bi->queue_avg_time*vi->rate);
    bm->avg_centerdesired=(ogg_uint32_t)(bi->queue_avg_time*vi->rate*bi->queue_avg_center);
    bm->minmax_sampledesired=(ogg_uint32_t)(bi->queue_minmax_time*vi->rate);

    /* first find the max possible needed queue size */
    maxlatency=max(bm->avg_sampledesired-bm->avg_centerdesired,
           bm->minmax_sampledesired)+bm->avg_centerdesired;

    if(maxlatency>0 &&
       (bi->queue_avgmin>0 || bi->queue_avgmax>0 || bi->queue_hardmax>0 ||
    bi->queue_hardmin>0)){
      long maxpackets=maxlatency/(ci->blocksizes[0]>>1)+3;
      long bins=PACKETBLOBS;

      bm->queue_size=maxpackets;
      bm->queue_bins=bins;
      bm->queue_binned=static_cast<ogg_uint32_t*>(_ogg_calloc(maxpackets,bins*sizeof(*bm->queue_binned)));
      bm->queue_actual=static_cast<ogg_uint32_t*>(_ogg_calloc(maxpackets,sizeof(*bm->queue_actual)));

      if((bi->queue_avgmin>0 || bi->queue_avgmax>0) &&
     bi->queue_avg_time>0){

    bm->avg_binacc=static_cast<long*>(_ogg_calloc(bins,sizeof(*bm->avg_binacc)));
    bm->avgfloat=PACKETBLOBS/2;

      }else{
    bm->avg_tail= -1;
      }

      if((bi->queue_hardmin>0 || bi->queue_hardmax>0) &&
     bi->queue_minmax_time>0){

    bm->minmax_binstack=static_cast<long*>(_ogg_calloc((bins*2+1)*bins*2,
                    sizeof(*bm->minmax_binstack)));
    bm->minmax_posstack=static_cast<long*>(_ogg_calloc((bins*2+1),
                      sizeof(*bm->minmax_posstack)));
    bm->minmax_limitstack=static_cast<long*>(_ogg_calloc((bins*2+1),
                      sizeof(*bm->minmax_limitstack)));
      }else{
    bm->minmax_tail= -1;
      }

      /* space for the packet queueing */
      bm->packetbuffers=static_cast<oggpack_buffer*>(_ogg_calloc(maxpackets,sizeof(*bm->packetbuffers)));
      bm->packets=static_cast<ogg_packet*>(_ogg_calloc(maxpackets,sizeof(*bm->packets)));
      for(i=0;i<maxpackets;i++)
    oggpack_writeinit(bm->packetbuffers+i);

    }else{
      bm->packetbuffers=static_cast<oggpack_buffer*>(_ogg_calloc(1,sizeof(*bm->packetbuffers)));
      bm->packets=static_cast<ogg_packet*>(_ogg_calloc(1,sizeof(*bm->packets)));
      oggpack_writeinit(bm->packetbuffers);

    }
  }
}

void vorbis_bitrate_clear(bitrate_manager_state *bm){
  int i;
  if(bm){
    if(bm->queue_binned)_ogg_free(bm->queue_binned);
    if(bm->queue_actual)_ogg_free(bm->queue_actual);
    if(bm->avg_binacc)_ogg_free(bm->avg_binacc);
    if(bm->minmax_binstack)_ogg_free(bm->minmax_binstack);
    if(bm->minmax_posstack)_ogg_free(bm->minmax_posstack);
    if(bm->minmax_limitstack)_ogg_free(bm->minmax_limitstack);

    if(bm->packetbuffers){
      if(bm->queue_size==0){
    oggpack_writeclear(bm->packetbuffers);
      }else{
    for(i=0;i<bm->queue_size;i++)
      oggpack_writeclear(bm->packetbuffers+i);
      }
      _ogg_free(bm->packetbuffers);
    }
    if(bm->packets)_ogg_free(bm->packets);

    memset(bm,0,sizeof(*bm));
  }
}

int vorbis_bitrate_managed(vorbis_block *vb){
  vorbis_dsp_state      *vd=vb->vd;
  private_state         *b=static_cast<private_state*>(vd->backend_state);
  bitrate_manager_state *bm=&b->bms;

  if(bm->queue_binned)return(1);
  return(0);
}

/* finish taking in the block we just processed */
int vorbis_bitrate_addblock(vorbis_block *vb){
  int i;
  vorbis_block_internal *vbi=static_cast<vorbis_block_internal*>(vb->internal);
  vorbis_dsp_state      *vd=vb->vd;
  private_state         *b=static_cast<private_state*>(vd->backend_state);
  bitrate_manager_state *bm=&b->bms;
  vorbis_info           *vi=vd->vi;
  codec_setup_info      *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  bitrate_manager_info  *bi=&ci->bi;
  int                    eofflag=vb->eofflag;
  int                    head=bm->queue_head;
  int                    next_head=head+1;
  int                    bins=bm->queue_bins;
  int                    minmax_head,new_minmax_head;

  ogg_uint32_t           *head_ptr;
  oggpack_buffer          temp;

  if(!bm->queue_binned){
    oggpack_buffer temp;
    /* not a bitrate managed stream, but for API simplicity, we'll
       buffer one packet to keep the code path clean */

    if(bm->queue_head)return(-1); /* one has been submitted without
                                     being claimed */
    bm->queue_head++;

    bm->packets[0].packet=oggpack_get_buffer(&vb->opb);
    bm->packets[0].bytes=oggpack_bytes(&vb->opb);
    bm->packets[0].b_o_s=0;
    bm->packets[0].e_o_s=vb->eofflag;
    bm->packets[0].granulepos=vb->granulepos;
    bm->packets[0].packetno=vb->sequence; /* for sake of completeness */

    memcpy(&temp,bm->packetbuffers,sizeof(vb->opb));
    memcpy(bm->packetbuffers,&vb->opb,sizeof(vb->opb));
    memcpy(&vb->opb,&temp,sizeof(vb->opb));

    return(0);
  }

  /* add encoded packet to head */
  if(next_head>=bm->queue_size)next_head=0;
  head_ptr=bm->queue_binned+bins*head;

  /* is there room to add a block? In proper use of the API, this will
     never come up... but guard it anyway */
  if(next_head==bm->avg_tail || next_head==bm->minmax_tail)return(-1);

  /* add the block to the toplevel queue */
  bm->queue_head=next_head;
  bm->queue_actual[head]=(vb->W?0x80000000UL:0);

  /* buffer packet fields */
  bm->packets[head].packet=oggpack_get_buffer(&vb->opb);
  bm->packets[head].bytes=oggpack_bytes(&vb->opb);
  bm->packets[head].b_o_s=0;
  bm->packets[head].e_o_s=vb->eofflag;
  bm->packets[head].granulepos=vb->granulepos;
  bm->packets[head].packetno=vb->sequence; /* for sake of completeness */

  /* swap packet buffers */
  memcpy(&temp,bm->packetbuffers+head,sizeof(vb->opb));
  memcpy(bm->packetbuffers+head,&vb->opb,sizeof(vb->opb));
  memcpy(&vb->opb,&temp,sizeof(vb->opb));

  /* save markers */
  head_ptr[0]=vbi->packetblob_markers[0];
  for(i=1;i<PACKETBLOBS;i++){
    head_ptr[i]=vbi->packetblob_markers[i]-vbi->packetblob_markers[i-1];
  }

  if(bm->avg_binacc)
    new_minmax_head=minmax_head=bm->avg_center;
  else
    new_minmax_head=minmax_head=head;

  /* the average tracking queue is updated first; its results (if it's
     in use) are taken into account by the min/max limiter (if min/max
     is in use) */
  if(bm->avg_binacc){
    unsigned long desired_center=bm->avg_centerdesired;
    if(eofflag)desired_center=0;

    /* update the avg head */
    for(i=0;i<bins;i++)
      bm->avg_binacc[i]+=LACING_ADJUST(head_ptr[i]);
    bm->avg_sampleacc+=ci->blocksizes[vb->W]>>1;
    bm->avg_centeracc+=ci->blocksizes[vb->W]>>1;

    if(bm->avg_sampleacc>bm->avg_sampledesired || eofflag){

      /* update the avg center */
      if(bm->avg_centeracc>desired_center){
    /* choose the new average floater */
    int samples=ci->blocksizes[vb->W]>>1;
    double upper=floater_interpolate(bm,vi,bi->queue_avgmax);
    double lower=floater_interpolate(bm,vi,bi->queue_avgmin);
    double newcp=PACKETBLOBS/2.,slew;
    int bin;

    if(upper<newcp)newcp=upper;
    if(lower>newcp)newcp=lower;

    slew=(newcp-bm->avgfloat)/samples*vi->rate;

    if(slew<bi->avgfloat_downslew_max)
      newcp=bm->avgfloat+bi->avgfloat_downslew_max/vi->rate*samples;
    if(slew>bi->avgfloat_upslew_max)
      newcp=bm->avgfloat+bi->avgfloat_upslew_max/vi->rate*samples;

    bm->avgfloat=newcp;
    /* apply the average floater to new blocks */
    bin=(int)rint(bm->avgfloat);

    /*fprintf(stderr,"%d ",bin);*/

    while(bm->avg_centeracc>desired_center){
      samples=ci->blocksizes[bm->queue_actual[bm->avg_center]&
                0x80000000UL?1:0]>>1;

      bm->queue_actual[bm->avg_center]|=bin;

      bm->avg_centeracc-=samples;
      bm->avg_center++;
      if(bm->avg_center>=bm->queue_size)bm->avg_center=0;
    }
    new_minmax_head=bm->avg_center;

      }

      /* update the avg tail if needed */
      while(bm->avg_sampleacc>bm->avg_sampledesired){
    int samples=
      ci->blocksizes[bm->queue_actual[bm->avg_tail]&0x80000000UL?1:0]>>1;
    for(i=0;i<bm->queue_bins;i++)
      bm->avg_binacc[i]-=LACING_ADJUST(bm->queue_binned[bins*bm->avg_tail+i]);
    bm->avg_sampleacc-=samples;
    bm->avg_tail++;
    if(bm->avg_tail>=bm->queue_size)bm->avg_tail=0;
      }


    }
  }else{
    /* if we're not using an average tracker, the 'float' is nailed to
       the avgfloat_initial value.  It needs to be set for the min/max
       to deal properly */
    long bin=PACKETBLOBS/2;
    bm->queue_actual[head]|=bin;
    new_minmax_head=next_head;
  }

  /* update the min/max queues and enforce limits */
  if(bm->minmax_binstack){
    unsigned long sampledesired=eofflag?0:bm->minmax_sampledesired;

    /* add to stack recent */
    while(minmax_head!=new_minmax_head){
      unsigned int i;
      int samples=ci->blocksizes[bm->queue_actual[minmax_head]&
                0x80000000UL?1:0]>>1;
      int actual=bm->queue_actual[minmax_head]&0x7fffffffUL;

      for(i=0;i<(unsigned int)bins;i++){
    bm->minmax_binstack[bm->minmax_stackptr*bins*2+bins+i]+=
      LACING_ADJUST(BINBYTES(bm,minmax_head,
                (unsigned int)actual>i?actual:i));

    bm->minmax_binstack[bm->minmax_stackptr*bins*2+i]+=
      LACING_ADJUST(BINBYTES(bm,minmax_head,
                (unsigned int)actual<i?actual:i));
      }

      bm->minmax_posstack[bm->minmax_stackptr]=minmax_head; /* not one
                                   past
                                   like
                                   typical */
      bm->minmax_limitstack[bm->minmax_stackptr]=0;
      bm->minmax_sampleacc+=samples;
      bm->minmax_acctotal+=
    LACING_ADJUST(BINBYTES(bm,minmax_head,actual));

      minmax_head++;
      if(minmax_head>=bm->queue_size)minmax_head=0;


    }

    /* check limits, enforce changes */
    if(bm->minmax_sampleacc>sampledesired){
      double bitrate=(double)(bm->minmax_acctotal*8)/
    bm->minmax_sampleacc*vi->rate;
      int limit=0;

      if((bi->queue_hardmax>0 && bitrate>bi->queue_hardmax) ||
     (bi->queue_hardmin>0 && bitrate<bi->queue_hardmin)){
    int newstack;
    int stackctr;
    long bitsum=bm->minmax_acctotal*8;

    bitrate=(double)bitsum/bm->minmax_sampleacc*vi->rate;

    /* we're off rate.  Iteratively try out new hard floater
           limits until we find one that brings us inside.  Here's
           where we see the whole point of the limit stacks.  */
    if(bi->queue_hardmax>0 && bitrate>bi->queue_hardmax){
      for(limit=-1;limit>-bins+1;limit--){
        long bitsum=limit_sum(bm,limit)*8;
        bitrate=(double)bitsum/bm->minmax_sampleacc*vi->rate;
        if(bitrate<=bi->queue_hardmax)break;
      }
    }else if(bitrate<bi->queue_hardmin){
      for(limit=1;limit<bins-1;limit++){
        long bitsum=limit_sum(bm,limit)*8;
        bitrate=(double)bitsum/bm->minmax_sampleacc*vi->rate;
        if(bitrate>=bi->queue_hardmin)break;
      }
      if(bitrate>bi->queue_hardmax)limit--;
    }

    /* trace the limit backward, stop when we see a lower limit */
    newstack=bm->minmax_stackptr-1;
    while(newstack>=0){
      if(bm->minmax_limitstack[newstack]<limit)break;
      newstack--;
    }

    /* update bit counter with new limit and replace any stack
           limits that have been replaced by our new lower limit */
    stackctr=bm->minmax_stackptr;
    while(stackctr>newstack){
      bm->minmax_acctotal-=
        LIMITBYTES(stackctr,bm->minmax_limitstack[stackctr]);
      bm->minmax_acctotal+=LIMITBYTES(stackctr,limit);

      if(stackctr<bm->minmax_stackptr)
        for(i=0;i<bins*2;i++)
          bm->minmax_binstack[stackctr*bins*2+i]+=
        bm->minmax_binstack[(stackctr+1)*bins*2+i];

      stackctr--;
    }
    stackctr++;
    bm->minmax_posstack[stackctr]=bm->minmax_posstack[bm->minmax_stackptr];
    bm->minmax_limitstack[stackctr]=limit;

    /* set up new blank stack entry */
    stackctr++;
    bm->minmax_stackptr=stackctr;
    memset(&bm->minmax_binstack[stackctr*bins*2],
           0,
           sizeof(*bm->minmax_binstack)*bins*2);
    bm->minmax_limitstack[stackctr]=0;
    bm->minmax_posstack[stackctr]=-1;

      }
    }

    /* remove from tail */
    while(bm->minmax_sampleacc>sampledesired){
      int samples=
    ci->blocksizes[bm->queue_actual[bm->minmax_tail]&0x80000000UL?1:0]>>1;
      int actual=bm->queue_actual[bm->minmax_tail]&0x7fffffffUL;

      for(i=0;i<bins;i++){
    bm->minmax_binstack[bins+i]-= /* always comes off the stack bottom */
      LACING_ADJUST(BINBYTES(bm,bm->minmax_tail,
                actual>i?
                actual:i));
    bm->minmax_binstack[i]-=
      LACING_ADJUST(BINBYTES(bm,bm->minmax_tail,
                actual<i?
                actual:i));
      }

      if(bm->minmax_limitstack[0]>actual)
    actual=bm->minmax_limitstack[0];
      if(bins+bm->minmax_limitstack[0]<actual)
    actual=bins+bm->minmax_limitstack[0];

      bm->minmax_acctotal-=LACING_ADJUST(BINBYTES(bm,bm->minmax_tail,actual));
      bm->minmax_sampleacc-=samples;

      /* revise queue_actual to reflect the limit */
      bm->queue_actual[bm->minmax_tail]&=0x80000000UL;
      bm->queue_actual[bm->minmax_tail]|=actual;

      if(bm->minmax_tail==bm->minmax_posstack[0]){
    /* the stack becomes a FIFO; the first data has fallen off */
    memmove(bm->minmax_binstack,bm->minmax_binstack+bins*2,
        sizeof(*bm->minmax_binstack)*bins*2*bm->minmax_stackptr);
    memmove(bm->minmax_posstack,bm->minmax_posstack+1,
        sizeof(*bm->minmax_posstack)*bm->minmax_stackptr);
    memmove(bm->minmax_limitstack,bm->minmax_limitstack+1,
        sizeof(*bm->minmax_limitstack)*bm->minmax_stackptr);
    bm->minmax_stackptr--;
      }

      bm->minmax_tail++;
      if(bm->minmax_tail>=bm->queue_size)bm->minmax_tail=0;

    }


    bm->last_to_flush=bm->minmax_tail;
  }else{
    bm->last_to_flush=bm->avg_center;
  }
  if(eofflag)
    bm->last_to_flush=bm->queue_head;
  return(0);
}

int vorbis_bitrate_flushpacket(vorbis_dsp_state *vd,ogg_packet *op){
  private_state         *b=static_cast<private_state*>(vd->backend_state);
  bitrate_manager_state *bm=&b->bms;

  if(bm->queue_size==0){
    if(bm->queue_head==0)return(0);

    memcpy(op,bm->packets,sizeof(*op));
    bm->queue_head=0;

  }else{

    if(bm->next_to_flush==bm->last_to_flush)return(0);

    {
      long bin=bm->queue_actual[bm->next_to_flush]&0x7fffffff,i;
      long bins=bm->queue_bins;
      ogg_uint32_t *markers=bm->queue_binned+bins*bm->next_to_flush;
      long bytes=markers[bin];

      memcpy(op,bm->packets+bm->next_to_flush,sizeof(*op));

      /* we have [PACKETBLOBS] possible packets all squished together in
     the buffer, in sequence.  count in to number [bin] */
      for(i=0;i<bin;i++)
    op->packet+=markers[i];
      op->bytes=bytes;

    }

    bm->next_to_flush++;
    if(bm->next_to_flush>=bm->queue_size)bm->next_to_flush=0;

  }

  return(1);
}









// ==========================================================================
// ==========================================================================
// info.c
// ==========================================================================
// ==========================================================================

/* general handling of the header and the vorbis_info structure (and
   substructures) */



void _v_writestring(oggpack_buffer *o,char *s, int bytes){

  while(bytes--){
    oggpack_write(o,*s++,8);
  }
}

void _v_readstring(oggpack_buffer *o,char *buf,int bytes){
  while(bytes--){
    *buf++=(char)(oggpack_read(o,8));
  }
}

void vorbis_comment_init(vorbis_comment *vc){
  memset(vc,0,sizeof(*vc));
}

void vorbis_comment_add(vorbis_comment *vc,char *comment){
  vc->user_comments=static_cast<char**>(_ogg_realloc(vc->user_comments,
                (vc->comments+2)*sizeof(*vc->user_comments)));
  vc->comment_lengths=static_cast<int*>(_ogg_realloc(vc->comment_lengths,
                    (vc->comments+2)*sizeof(*vc->comment_lengths)));
  vc->comment_lengths[vc->comments]=(int)strlen(comment);
  vc->user_comments[vc->comments]=static_cast<char*>(_ogg_malloc(vc->comment_lengths[vc->comments]+1));
  strcpy(vc->user_comments[vc->comments], comment);
  vc->comments++;
  vc->user_comments[vc->comments]=NULL;
}

void vorbis_comment_add_tag(vorbis_comment *vc, char *tag, char *contents){
  char *comment=static_cast<char*>(alloca(strlen(tag)+strlen(contents)+2)); /* +2 for = and \0 */
  strcpy(comment, tag);
  strcat(comment, "=");
  strcat(comment, contents);
  vorbis_comment_add(vc, comment);
}

/* This is more or less the same as strncasecmp - but that doesn't exist
 * everywhere, and this is a fairly trivial function, so we include it */
int tagcompare(const char *s1, const char *s2, int n){
  int c=0;
  while(c < n){
    if(toupper(s1[c]) != toupper(s2[c]))
      return !0;
    c++;
  }
  return 0;
}

char *vorbis_comment_query(vorbis_comment *vc, char *tag, int count){
  long i;
  int found = 0;
  int taglen = (int)(strlen(tag)+1); /* +1 for the = we append */
  char *fulltag = static_cast<char*>(alloca(taglen+ 1));

  strcpy(fulltag, tag);
  strcat(fulltag, "=");

  for(i=0;i<vc->comments;i++){
    if(!tagcompare(vc->user_comments[i], fulltag, taglen)){
      if(count == found)
    /* We return a pointer to the data, not a copy */
        return vc->user_comments[i] + taglen;
      else
    found++;
    }
  }
  return NULL; /* didn't find anything */
}

int vorbis_comment_query_count(vorbis_comment *vc, char *tag){
  int i,count=0;
  int taglen = (int)(strlen(tag)+1); /* +1 for the = we append */
  char *fulltag = static_cast<char*>(alloca(taglen+1));
  strcpy(fulltag,tag);
  strcat(fulltag, "=");

  for(i=0;i<vc->comments;i++){
    if(!tagcompare(vc->user_comments[i], fulltag, taglen))
      count++;
  }

  return count;
}

void vorbis_comment_clear(vorbis_comment *vc){
  if(vc){
    long i;
    for(i=0;i<vc->comments;i++)
      if(vc->user_comments[i])_ogg_free(vc->user_comments[i]);
    if(vc->user_comments)_ogg_free(vc->user_comments);
    if(vc->comment_lengths)_ogg_free(vc->comment_lengths);
    if(vc->vendor)_ogg_free(vc->vendor);
  }
  memset(vc,0,sizeof(*vc));
}

/* blocksize 0 is guaranteed to be short, 1 is guarantted to be long.
   They may be equal, but short will never ge greater than long */
int vorbis_info_blocksize(vorbis_info *vi,int zo){
  codec_setup_info *ci = static_cast<codec_setup_info*>(vi->codec_setup);
  return ci ? ci->blocksizes[zo] : -1;
}

/* used by synthesis, which has a full, alloced vi */
void vorbis_info_init(vorbis_info *vi){
  memset(vi,0,sizeof(*vi));
  vi->codec_setup=_ogg_calloc(1,sizeof(codec_setup_info));
}

void vorbis_info_clear(vorbis_info *vi){
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  int i;

  if(ci){

    for(i=0;i<ci->modes;i++)
      if(ci->mode_param[i])_ogg_free(ci->mode_param[i]);

    for(i=0;i<ci->maps;i++) /* unpack does the range checking */
      _mapping_P[ci->map_type[i]]->free_info(ci->map_param[i]);

    for(i=0;i<ci->floors;i++) /* unpack does the range checking */
      _floor_P[ci->floor_type[i]]->free_info(ci->floor_param[i]);

    for(i=0;i<ci->residues;i++) /* unpack does the range checking */
      _residue_P[ci->residue_type[i]]->free_info(ci->residue_param[i]);

    for(i=0;i<ci->books;i++){
      if(ci->book_param[i]){
    /* knows if the book was not alloced */
    vorbis_staticbook_destroy(ci->book_param[i]);
      }
      if(ci->fullbooks)
    vorbis_book_clear(ci->fullbooks+i);
    }
    if(ci->fullbooks)
    _ogg_free(ci->fullbooks);

    for(i=0;i<ci->psys;i++)
      _vi_psy_free(ci->psy_param[i]);

    _ogg_free(ci);
  }

  memset(vi,0,sizeof(*vi));
}

/* Header packing/unpacking ********************************************/

int _vorbis_unpack_info(vorbis_info *vi,oggpack_buffer *opb){
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  if(!ci)return(OV_EFAULT);

  vi->version=oggpack_read(opb,32);
  if(vi->version!=0)return(OV_EVERSION);

  vi->channels=oggpack_read(opb,8);
  vi->rate=oggpack_read(opb,32);

  vi->bitrate_upper=oggpack_read(opb,32);
  vi->bitrate_nominal=oggpack_read(opb,32);
  vi->bitrate_lower=oggpack_read(opb,32);

  ci->blocksizes[0]=1<<oggpack_read(opb,4);
  ci->blocksizes[1]=1<<oggpack_read(opb,4);

  if(vi->rate<1)goto err_out;
  if(vi->channels<1)goto err_out;
  if(ci->blocksizes[0]<8)goto err_out;
  if(ci->blocksizes[1]<ci->blocksizes[0])goto err_out;

  if(oggpack_read(opb,1)!=1)goto err_out; /* EOP check */

  return(0);
 err_out:
  vorbis_info_clear(vi);
  return(OV_EBADHEADER);
}

int _vorbis_unpack_comment(vorbis_comment *vc,oggpack_buffer *opb){
  int i;
  int vendorlen=oggpack_read(opb,32);
  if(vendorlen<0)goto err_out;
  vc->vendor=static_cast<char*>(_ogg_calloc(vendorlen+1,1));
  _v_readstring(opb,vc->vendor,vendorlen);
  vc->comments=oggpack_read(opb,32);
  if(vc->comments<0)goto err_out;
  vc->user_comments=static_cast<char**>(_ogg_calloc(vc->comments+1,sizeof(*vc->user_comments)));
  vc->comment_lengths=static_cast<int*>(_ogg_calloc(vc->comments+1, sizeof(*vc->comment_lengths)));

  for(i=0;i<vc->comments;i++){
    int len=oggpack_read(opb,32);
    if(len<0)goto err_out;
    vc->comment_lengths[i]=len;
    vc->user_comments[i]=static_cast<char*>(_ogg_calloc(len+1,1));
    _v_readstring(opb,vc->user_comments[i],len);
  }
  if(oggpack_read(opb,1)!=1)goto err_out; /* EOP check */

  return(0);
 err_out:
  vorbis_comment_clear(vc);
  return(OV_EBADHEADER);
}

/* all of the real encoding details are here.  The modes, books,
   everything */
int _vorbis_unpack_books(vorbis_info *vi,oggpack_buffer *opb){
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  int i;
  if(!ci)return(OV_EFAULT);

  /* codebooks */
  ci->books=oggpack_read(opb,8)+1;
  /*ci->book_param=_ogg_calloc(ci->books,sizeof(*ci->book_param));*/
  for(i=0;i<ci->books;i++){
    ci->book_param[i]=static_cast<static_codebook*>(_ogg_calloc(1,sizeof(*ci->book_param[i])));
    if(vorbis_staticbook_unpack(opb,ci->book_param[i])) goto err_out;
  }

  /* time backend settings; hooks are unused */
  {
    int times=oggpack_read(opb,6)+1;
    for(i=0;i<times;i++){
      int test=oggpack_read(opb,16);
      if(test<0 || test>=VI_TIMEB) goto err_out;
    }
  }

  /* floor backend settings */
  ci->floors=oggpack_read(opb,6)+1;
  /*ci->floor_type=_ogg_malloc(ci->floors*sizeof(*ci->floor_type));*/
  /*ci->floor_param=_ogg_calloc(ci->floors,sizeof(void *));*/
  for(i=0;i<ci->floors;i++){
    ci->floor_type[i]=oggpack_read(opb,16);
    if(ci->floor_type[i]<0 || ci->floor_type[i]>=VI_FLOORB) goto err_out;
    ci->floor_param[i]=_floor_P[ci->floor_type[i]]->unpack(vi,opb);
    if(!ci->floor_param[i]) goto err_out;
  }

  /* residue backend settings */
  ci->residues=oggpack_read(opb,6)+1;
  /*ci->residue_type=_ogg_malloc(ci->residues*sizeof(*ci->residue_type));*/
  /*ci->residue_param=_ogg_calloc(ci->residues,sizeof(void *));*/
  for(i=0;i<ci->residues;i++){
    ci->residue_type[i]=oggpack_read(opb,16);
    if(ci->residue_type[i]<0 || ci->residue_type[i]>=VI_RESB) goto err_out;
    ci->residue_param[i]=_residue_P[ci->residue_type[i]]->unpack(vi,opb);
    if(!ci->residue_param[i]) goto err_out;
  }

  /* map backend settings */
  ci->maps=oggpack_read(opb,6)+1;
  /*ci->map_type=_ogg_malloc(ci->maps*sizeof(*ci->map_type));*/
  /*ci->map_param=_ogg_calloc(ci->maps,sizeof(void *));*/
  for(i=0;i<ci->maps;i++){
    ci->map_type[i]=oggpack_read(opb,16);
    if(ci->map_type[i]<0 || ci->map_type[i]>=VI_MAPB) goto err_out;
    ci->map_param[i]=_mapping_P[ci->map_type[i]]->unpack(vi,opb);
    if(!ci->map_param[i]) goto err_out;
  }

  /* mode settings */
  ci->modes=oggpack_read(opb,6)+1;
  /*vi->mode_param=_ogg_calloc(vi->modes,sizeof(void *));*/
  for(i=0;i<ci->modes;i++){
    ci->mode_param[i]=static_cast<vorbis_info_mode*>(_ogg_calloc(1,sizeof(*ci->mode_param[i])));
    ci->mode_param[i]->blockflag=oggpack_read(opb,1);
    ci->mode_param[i]->windowtype=oggpack_read(opb,16);
    ci->mode_param[i]->transformtype=oggpack_read(opb,16);
    ci->mode_param[i]->mapping=oggpack_read(opb,8);

    if(ci->mode_param[i]->windowtype>=VI_WINDOWB) goto err_out;
    if(ci->mode_param[i]->transformtype>=VI_WINDOWB) goto err_out;
    if(ci->mode_param[i]->mapping>=ci->maps) goto err_out;
  }

  if(oggpack_read(opb,1)!=1)goto err_out; /* top level EOP check */

  return(0);
 err_out:
  vorbis_info_clear(vi);
  return(OV_EBADHEADER);
}

/* The Vorbis header is in three packets; the initial small packet in
   the first page that identifies basic parameters, a second packet
   with bitstream comments and a third packet that holds the
   codebook. */

int vorbis_synthesis_headerin(vorbis_info *vi,vorbis_comment *vc,ogg_packet *op){
  oggpack_buffer opb;

  if(op){
    oggpack_readinit(&opb,op->packet,op->bytes);

    /* Which of the three types of header is this? */
    /* Also verify header-ness, vorbis */
    {
      char buffer[6];
      int packtype=oggpack_read(&opb,8);
      memset(buffer,0,6);
      _v_readstring(&opb,buffer,6);
      if(memcmp(buffer,"vorbis",6)){
    /* not a vorbis header */
    return(OV_ENOTVORBIS);
      }
      switch(packtype){
      case 0x01: /* least significant *bit* is read first */
    if(!op->b_o_s){
      /* Not the initial packet */
      return(OV_EBADHEADER);
    }
    if(vi->rate!=0){
      /* previously initialized info header */
      return(OV_EBADHEADER);
    }

    return(_vorbis_unpack_info(vi,&opb));

      case 0x03: /* least significant *bit* is read first */
    if(vi->rate==0){
      /* um... we didn't get the initial header */
      return(OV_EBADHEADER);
    }

    return(_vorbis_unpack_comment(vc,&opb));

      case 0x05: /* least significant *bit* is read first */
    if(vi->rate==0 || vc->vendor==NULL){
      /* um... we didn;t get the initial header or comments yet */
      return(OV_EBADHEADER);
    }

    return(_vorbis_unpack_books(vi,&opb));

      default:
    /* Not a valid vorbis header type */
    return(OV_EBADHEADER);
    //break;
      }
    }
  }
  return(OV_EBADHEADER);
}

/* pack side **********************************************************/

int _vorbis_pack_info(oggpack_buffer *opb,vorbis_info *vi){
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  if(!ci)return(OV_EFAULT);

  /* preamble */
  oggpack_write(opb,0x01,8);
  _v_writestring(opb,"vorbis", 6);

  /* basic information about the stream */
  oggpack_write(opb,0x00,32);
  oggpack_write(opb,vi->channels,8);
  oggpack_write(opb,vi->rate,32);

  oggpack_write(opb,vi->bitrate_upper,32);
  oggpack_write(opb,vi->bitrate_nominal,32);
  oggpack_write(opb,vi->bitrate_lower,32);

  oggpack_write(opb,ilog2(ci->blocksizes[0]),4);
  oggpack_write(opb,ilog2(ci->blocksizes[1]),4);
  oggpack_write(opb,1,1);

  return(0);
}

int _vorbis_pack_comment(oggpack_buffer *opb,vorbis_comment *vc){
  char temp[]="Xiph.Org libVorbis I 20020717";
  int bytes = (int)strlen(temp);

  /* preamble */
  oggpack_write(opb,0x03,8);
  _v_writestring(opb,"vorbis", 6);

  /* vendor */
  oggpack_write(opb,bytes,32);
  _v_writestring(opb,temp, bytes);

  /* comments */

  oggpack_write(opb,vc->comments,32);
  if(vc->comments){
    int i;
    for(i=0;i<vc->comments;i++){
      if(vc->user_comments[i]){
    oggpack_write(opb,vc->comment_lengths[i],32);
    _v_writestring(opb,vc->user_comments[i], vc->comment_lengths[i]);
      }else{
    oggpack_write(opb,0,32);
      }
    }
  }
  oggpack_write(opb,1,1);

  return(0);
}

int _vorbis_pack_books(oggpack_buffer *opb,vorbis_info *vi){
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  int i;
  if(!ci)return(OV_EFAULT);

  oggpack_write(opb,0x05,8);
  _v_writestring(opb,"vorbis", 6);

  /* books */
  oggpack_write(opb,ci->books-1,8);
  for(i=0;i<ci->books;i++)
    if(vorbis_staticbook_pack(ci->book_param[i],opb))goto err_out;

  /* times; hook placeholders */
  oggpack_write(opb,0,6);
  oggpack_write(opb,0,16);

  /* floors */
  oggpack_write(opb,ci->floors-1,6);
  for(i=0;i<ci->floors;i++){
    oggpack_write(opb,ci->floor_type[i],16);
    if(_floor_P[ci->floor_type[i]]->pack)
      _floor_P[ci->floor_type[i]]->pack(ci->floor_param[i],opb);
    else
      goto err_out;
  }

  /* residues */
  oggpack_write(opb,ci->residues-1,6);
  for(i=0;i<ci->residues;i++){
    oggpack_write(opb,ci->residue_type[i],16);
    _residue_P[ci->residue_type[i]]->pack(ci->residue_param[i],opb);
  }

  /* maps */
  oggpack_write(opb,ci->maps-1,6);
  for(i=0;i<ci->maps;i++){
    oggpack_write(opb,ci->map_type[i],16);
    _mapping_P[ci->map_type[i]]->pack(vi,ci->map_param[i],opb);
  }

  /* modes */
  oggpack_write(opb,ci->modes-1,6);
  for(i=0;i<ci->modes;i++){
    oggpack_write(opb,ci->mode_param[i]->blockflag,1);
    oggpack_write(opb,ci->mode_param[i]->windowtype,16);
    oggpack_write(opb,ci->mode_param[i]->transformtype,16);
    oggpack_write(opb,ci->mode_param[i]->mapping,8);
  }
  oggpack_write(opb,1,1);

  return(0);
err_out:
  return(-1);
}

int vorbis_commentheader_out(vorbis_comment *vc,
                          ogg_packet *op){

  oggpack_buffer opb;

  oggpack_writeinit(&opb);
  if(_vorbis_pack_comment(&opb,vc)) return OV_EIMPL;

  op->packet = static_cast<unsigned char*>(_ogg_malloc(oggpack_bytes(&opb)));
  memcpy(op->packet, opb.buffer, oggpack_bytes(&opb));

  op->bytes=oggpack_bytes(&opb);
  op->b_o_s=0;
  op->e_o_s=0;
  op->granulepos=0;

  return 0;
}

int vorbis_analysis_headerout(vorbis_dsp_state *v,
                  vorbis_comment *vc,
                  ogg_packet *op,
                  ogg_packet *op_comm,
                  ogg_packet *op_code){
  int ret=OV_EIMPL;
  vorbis_info *vi=v->vi;
  oggpack_buffer opb;
  private_state *b=static_cast<private_state*>(v->backend_state);

  if(!b){
    ret=OV_EFAULT;
    goto err_out;
  }

  /* first header packet **********************************************/

  oggpack_writeinit(&opb);
  if(_vorbis_pack_info(&opb,vi))goto err_out;

  /* build the packet */
  if(b->header)_ogg_free(b->header);
  b->header=static_cast<unsigned char*>(_ogg_malloc(oggpack_bytes(&opb)));
  memcpy(b->header,opb.buffer,oggpack_bytes(&opb));
  op->packet=b->header;
  op->bytes=oggpack_bytes(&opb);
  op->b_o_s=1;
  op->e_o_s=0;
  op->granulepos=0;

  /* second header packet (comments) **********************************/

  oggpack_reset(&opb);
  if(_vorbis_pack_comment(&opb,vc))goto err_out;

  if(b->header1)_ogg_free(b->header1);
  b->header1=static_cast<unsigned char*>(_ogg_malloc(oggpack_bytes(&opb)));
  memcpy(b->header1,opb.buffer,oggpack_bytes(&opb));
  op_comm->packet=b->header1;
  op_comm->bytes=oggpack_bytes(&opb);
  op_comm->b_o_s=0;
  op_comm->e_o_s=0;
  op_comm->granulepos=0;

  /* third header packet (modes/codebooks) ****************************/

  oggpack_reset(&opb);
  if(_vorbis_pack_books(&opb,vi))goto err_out;

  if(b->header2)_ogg_free(b->header2);
  b->header2=static_cast<unsigned char*>(_ogg_malloc(oggpack_bytes(&opb)));
  memcpy(b->header2,opb.buffer,oggpack_bytes(&opb));
  op_code->packet=b->header2;
  op_code->bytes=oggpack_bytes(&opb);
  op_code->b_o_s=0;
  op_code->e_o_s=0;
  op_code->granulepos=0;

  oggpack_writeclear(&opb);
  return(0);
 err_out:
  oggpack_writeclear(&opb);
  memset(op,0,sizeof(*op));
  memset(op_comm,0,sizeof(*op_comm));
  memset(op_code,0,sizeof(*op_code));

  if(b->header)_ogg_free(b->header);
  if(b->header1)_ogg_free(b->header1);
  if(b->header2)_ogg_free(b->header2);
  b->header=NULL;
  b->header1=NULL;
  b->header2=NULL;
  return(ret);
}

double vorbis_granule_time(vorbis_dsp_state *v,ogg_int64_t granulepos){
  if(granulepos>=0)
    return((double)granulepos/v->vi->rate);
  return(-1);
}
















// ==========================================================================
// ==========================================================================
// analysis.c
// ==========================================================================
// ==========================================================================

int analysis_noisy=1;

/* decides between modes, dispatches to the appropriate mapping. */
int vorbis_analysis(vorbis_block *vb, ogg_packet *op){
  int                   ret;

  vb->glue_bits=0;
  vb->time_bits=0;
  vb->floor_bits=0;
  vb->res_bits=0;

  /* first things first.  Make sure encode is ready */
  oggpack_reset(&vb->opb);

  /* we only have one mapping type (0), and we let the mapping code
     itself figure out what soft mode to use.  This allows easier
     bitrate management */

  ret=_mapping_P[0]->forward(vb);
  if (ret) return(ret);

  if(op){
    if(vorbis_bitrate_managed(vb))
      /* The app is using a bitmanaged mode... but not using the
         bitrate management interface. */
      return(OV_EINVAL);

    op->packet=oggpack_get_buffer(&vb->opb);
    op->bytes=oggpack_bytes(&vb->opb);
    op->b_o_s=0;
    op->e_o_s=vb->eofflag;
    op->granulepos=vb->granulepos;
    op->packetno=vb->sequence; /* for sake of completeness */
  }
  return(0);
}

/* there was no great place to put this.... */
void _analysis_output_always(char *base,int i,float *v,int n,int bark,int dB,ogg_int64_t off){
  int j;
  FILE *of;
  char buffer[80];

  /*  if(i==5870){*/
    sprintf(buffer,"%s_%d.m",base,i);
    of=fopen(buffer,"w");

    if(!of)perror("failed to open data dump file");

    for(j=0;j<n;j++){
      if(bark){
    float b=(float)toBARK((4000.f*j/n)+.25);
    fprintf(of,"%f ",b);
      }else
    if(off!=0)
      fprintf(of,"%f ",(double)(j+off)/8000.);
    else
      fprintf(of,"%f ",(double)j);

      if(dB){
    float val;
    if(v[j]==0.)
      val=-140.;
    else
      val=todB(v+j);
    fprintf(of,"%f\n",val);
      }else{
    fprintf(of,"%f\n",v[j]);
      }
    }
    fclose(of);
    /*  } */
}

void _analysis_output(char *base,int i,float *v,int n,int bark,int dB,
              ogg_int64_t off){
  if(analysis_noisy)_analysis_output_always(base,i,v,n,bark,dB,off);
}





// ==========================================================================
// ==========================================================================
// psy.c
// ==========================================================================
// ==========================================================================


/* more detailed ATH; the bass if flat to save stressing the floor
   overly for only a bin or two of savings. */

#define MAX_ATH 88
float ATH[]={
  /*15*/  -51, -52, -53, -54, -55, -56, -57, -58,
  /*31*/  -59, -60, -61, -62, -63, -64, -65, -66,
  /*63*/  -67, -68, -69, -70, -71, -72, -73, -74,
  /*125*/ -75, -76, -77, -78, -80, -81, -82, -83,
  /*250*/ -84, -85, -86, -87, -88, -88, -89, -89,
  /*500*/ -90, -91, -91, -92, -93, -94, -95, -96,
  /*1k*/  -96, -97, -98, -98, -99, -99,-100,-100,
  /*2k*/ -101,-102,-103,-104,-106,-107,-107,-107,
  /*4k*/ -107,-105,-103,-102,-101, -99, -98, -96,
  /*8k*/  -95, -95, -96, -97, -96, -95, -93, -90,
  /*16k*/ -80, -70, -50, -40, -30, -30, -30, -30
};

/* The tone masking curves from Ehmer's and Fielder's papers have been
   replaced by an empirically collected data set.  The previously
   published values were, far too often, simply on crack. */

#define EHMER_OFFSET 16
#define EHMER_MAX 56

/* masking tones from -50 to 0dB, 62.5 through 16kHz at half octaves
   test tones from -2 octaves to +5 octaves sampled at eighth octaves */
/* (Vorbis 0dB, the loudest possible tone, is assumed to be ~100dB SPL
   for collection of these curves) */

float tonemasks[P_BANDS][6][EHMER_MAX]={
  /* 62.5 Hz */
  {{ -60,  -60,  -60,  -60,  -60,  -60,  -60,  -60,
     -60,  -60,  -60,  -60,  -62,  -62,  -65,  -73,
     -69,  -68,  -68,  -67,  -70,  -70,  -72,  -74,
     -75,  -79,  -79,  -80,  -83,  -88,  -93, -100,
     -110, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -48,  -48,  -48,  -48,  -48,  -48,  -48,  -48,
     -48,  -48,  -48,  -48,  -48,  -53,  -61,  -66,
     -66,  -68,  -67,  -70,  -76,  -76,  -72,  -73,
     -75,  -76,  -78,  -79,  -83,  -88,  -93, -100,
     -110, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -37,  -37,  -37,  -37,  -37,  -37,  -37,  -37,
     -38,  -40,  -42,  -46,  -48,  -53,  -55,  -62,
     -65,  -58,  -56,  -56,  -61,  -60,  -65,  -67,
     -69,  -71,  -77,  -77,  -78,  -80,  -82,  -84,
     -88,  -93,  -98, -106, -112, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -25,  -25,  -25,  -25,  -25,  -25,  -25,  -25,
     -25,  -26,  -27,  -29,  -32,  -38,  -48,  -52,
     -52,  -50,  -48,  -48,  -51,  -52,  -54,  -60,
     -67,  -67,  -66,  -68,  -69,  -73,  -73,  -76,
     -80,  -81,  -81,  -85,  -85,  -86,  -88,  -93,
     -100, -110, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -16,  -16,  -16,  -16,  -16,  -16,  -16,  -16,
     -17,  -19,  -20,  -22,  -26,  -28,  -31,  -40,
     -47,  -39,  -39,  -40,  -42,  -43,  -47,  -51,
     -57,  -52,  -55,  -55,  -60,  -58,  -62,  -63,
     -70,  -67,  -69,  -72,  -73,  -77,  -80,  -82,
     -83,  -87,  -90,  -94,  -98, -104, -115, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   {  -8,   -8,   -8,   -8,   -8,   -8,   -8,   -8,
      -8,   -8,  -10,  -11,  -15,  -19,  -25,  -30,
      -34,  -31,  -30,  -31,  -29,  -32,  -35,  -42,
      -48,  -42,  -44,  -46,  -50,  -50,  -51,  -52,
      -59,  -54,  -55,  -55,  -58,  -62,  -63,  -66,
      -72,  -73,  -76,  -75,  -78,  -80,  -80,  -81,
      -84,  -88,  -90,  -94,  -98, -101, -106, -110}},
  /* 88Hz */
  {{ -66,  -66,  -66,  -66,  -66,  -66,  -66,  -66,
     -66,  -66,  -66,  -66,  -66,  -67,  -67,  -67,
     -76,  -72,  -71,  -74,  -76,  -76,  -75,  -78,
     -79,  -79,  -81,  -83,  -86,  -89,  -93,  -97,
     -100, -105, -110, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -47,  -47,  -47,  -47,  -47,  -47,  -47,  -47,
     -47,  -47,  -47,  -48,  -51,  -55,  -59,  -66,
     -66,  -66,  -67,  -66,  -68,  -69,  -70,  -74,
     -79,  -77,  -77,  -78,  -80,  -81,  -82,  -84,
     -86,  -88,  -91,  -95, -100, -108, -116, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -36,  -36,  -36,  -36,  -36,  -36,  -36,  -36,
     -36,  -37,  -37,  -41,  -44,  -48,  -51,  -58,
     -62,  -60,  -57,  -59,  -59,  -60,  -63,  -65,
     -72,  -71,  -70,  -72,  -74,  -77,  -76,  -78,
     -81,  -81,  -80,  -83,  -86,  -91,  -96, -100,
     -105, -110, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -28,  -28,  -28,  -28,  -28,  -28,  -28,  -28,
     -28,  -30,  -32,  -32,  -33,  -35,  -41,  -49,
     -50,  -49,  -47,  -48,  -48,  -52,  -51,  -57,
     -65,  -61,  -59,  -61,  -64,  -69,  -70,  -74,
     -77,  -77,  -78,  -81,  -84,  -85,  -87,  -90,
     -92,  -96, -100, -107, -112, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -19,  -19,  -19,  -19,  -19,  -19,  -19,  -19,
     -20,  -21,  -23,  -27,  -30,  -35,  -36,  -41,
     -46,  -44,  -42,  -40,  -41,  -41,  -43,  -48,
     -55,  -53,  -52,  -53,  -56,  -59,  -58,  -60,
     -67,  -66,  -69,  -71,  -72,  -75,  -79,  -81,
     -84,  -87,  -90,  -93,  -97, -101, -107, -114,
     -999, -999, -999, -999, -999, -999, -999, -999},
   {  -9,   -9,   -9,   -9,   -9,   -9,   -9,   -9,
      -11,  -12,  -12,  -15,  -16,  -20,  -23,  -30,
      -37,  -34,  -33,  -34,  -31,  -32,  -32,  -38,
      -47,  -44,  -41,  -40,  -47,  -49,  -46,  -46,
      -58,  -50,  -50,  -54,  -58,  -62,  -64,  -67,
      -67,  -70,  -72,  -76,  -79,  -83,  -87,  -91,
      -96, -100, -104, -110, -999, -999, -999, -999}},
  /* 125 Hz */
  {{ -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,
     -62,  -62,  -63,  -64,  -66,  -67,  -66,  -68,
     -75,  -72,  -76,  -75,  -76,  -78,  -79,  -82,
     -84,  -85,  -90,  -94, -101, -110, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -59,  -59,  -59,  -59,  -59,  -59,  -59,  -59,
     -59,  -59,  -59,  -60,  -60,  -61,  -63,  -66,
     -71,  -68,  -70,  -70,  -71,  -72,  -72,  -75,
     -81,  -78,  -79,  -82,  -83,  -86,  -90,  -97,
     -103, -113, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -53,  -53,  -53,  -53,  -53,  -53,  -53,  -53,
     -53,  -54,  -55,  -57,  -56,  -57,  -55,  -61,
     -65,  -60,  -60,  -62,  -63,  -63,  -66,  -68,
     -74,  -73,  -75,  -75,  -78,  -80,  -80,  -82,
     -85,  -90,  -96, -101, -108, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -46,  -46,  -46,  -46,  -46,  -46,  -46,  -46,
     -46,  -46,  -47,  -47,  -47,  -47,  -48,  -51,
     -57,  -51,  -49,  -50,  -51,  -53,  -54,  -59,
     -66,  -60,  -62,  -67,  -67,  -70,  -72,  -75,
     -76,  -78,  -81,  -85,  -88,  -94,  -97, -104,
     -112, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -36,  -36,  -36,  -36,  -36,  -36,  -36,  -36,
     -39,  -41,  -42,  -42,  -39,  -38,  -41,  -43,
     -52,  -44,  -40,  -39,  -37,  -37,  -40,  -47,
     -54,  -50,  -48,  -50,  -55,  -61,  -59,  -62,
     -66,  -66,  -66,  -69,  -69,  -73,  -74,  -74,
     -75,  -77,  -79,  -82,  -87,  -91,  -95, -100,
     -108, -115, -999, -999, -999, -999, -999, -999},
   { -28,  -26,  -24,  -22,  -20,  -20,  -23,  -29,
     -30,  -31,  -28,  -27,  -28,  -28,  -28,  -35,
     -40,  -33,  -32,  -29,  -30,  -30,  -30,  -37,
     -45,  -41,  -37,  -38,  -45,  -47,  -47,  -48,
     -53,  -49,  -48,  -50,  -49,  -49,  -51,  -52,
     -58,  -56,  -57,  -56,  -60,  -61,  -62,  -70,
     -72,  -74,  -78,  -83,  -88,  -93, -100, -106}},
  /* 177 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -110, -105, -100,  -95,  -91,  -87,  -83,
    -80,  -78,  -76,  -78,  -78,  -81,  -83,  -85,
    -86,  -85,  -86,  -87,  -90,  -97, -107, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -110, -105, -100,  -95,  -90,
    -85,  -81,  -77,  -73,  -70,  -67,  -67,  -68,
    -75,  -73,  -70,  -69,  -70,  -72,  -75,  -79,
    -84,  -83,  -84,  -86,  -88,  -89,  -89,  -93,
    -98, -105, -112, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-105, -100,  -95,  -90,  -85,  -80,  -76,  -71,
    -68,  -68,  -65,  -63,  -63,  -62,  -62,  -64,
    -65,  -64,  -61,  -62,  -63,  -64,  -66,  -68,
    -73,  -73,  -74,  -75,  -76,  -81,  -83,  -85,
    -88,  -89,  -92,  -95, -100, -108, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   { -80,  -75,  -71,  -68,  -65,  -63,  -62,  -61,
     -61,  -61,  -61,  -59,  -56,  -57,  -53,  -50,
     -58,  -52,  -50,  -50,  -52,  -53,  -54,  -58,
     -67,  -63,  -67,  -68,  -72,  -75,  -78,  -80,
     -81,  -81,  -82,  -85,  -89,  -90,  -93,  -97,
     -101, -107, -114, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -65,  -61,  -59,  -57,  -56,  -55,  -55,  -56,
     -56,  -57,  -55,  -53,  -52,  -47,  -44,  -44,
     -50,  -44,  -41,  -39,  -39,  -42,  -40,  -46,
     -51,  -49,  -50,  -53,  -54,  -63,  -60,  -61,
     -62,  -66,  -66,  -66,  -70,  -73,  -74,  -75,
     -76,  -75,  -79,  -85,  -89,  -91,  -96, -102,
     -110, -999, -999, -999, -999, -999, -999, -999},
   { -52,  -50,  -49,  -49,  -48,  -48,  -48,  -49,
     -50,  -50,  -49,  -46,  -43,  -39,  -35,  -33,
     -38,  -36,  -32,  -29,  -32,  -32,  -32,  -35,
     -44,  -39,  -38,  -38,  -46,  -50,  -45,  -46,
     -53,  -50,  -50,  -50,  -54,  -54,  -53,  -53,
     -56,  -57,  -59,  -66,  -70,  -72,  -74,  -79,
     -83,  -85,  -90, -97, -114, -999, -999, -999}},
  /* 250 Hz */
  {{-999, -999, -999, -999, -999, -999, -110, -105,
    -100,  -95,  -90,  -86,  -80,  -75,  -75,  -79,
    -80,  -79,  -80,  -81,  -82,  -88,  -95, -103,
    -110, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -108, -103,  -98,  -93,
    -88,  -83,  -79,  -78,  -75,  -71,  -67,  -68,
    -73,  -73,  -72,  -73,  -75,  -77,  -80,  -82,
    -88,  -93, -100, -107, -114, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -110, -105, -101,  -96,  -90,
    -86,  -81,  -77,  -73,  -69,  -66,  -61,  -62,
    -66,  -64,  -62,  -65,  -66,  -70,  -72,  -76,
    -81,  -80,  -84,  -90,  -95, -102, -110, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -107, -103,  -97,  -92,  -88,
    -83,  -79,  -74,  -70,  -66,  -59,  -53,  -58,
    -62,  -55,  -54,  -54,  -54,  -58,  -61,  -62,
    -72,  -70,  -72,  -75,  -78,  -80,  -81,  -80,
    -83,  -83,  -88,  -93, -100, -107, -115, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -105, -100,  -95,  -90,  -85,
    -80,  -75,  -70,  -66,  -62,  -56,  -48,  -44,
    -48,  -46,  -46,  -43,  -46,  -48,  -48,  -51,
    -58,  -58,  -59,  -60,  -62,  -62,  -61,  -61,
    -65,  -64,  -65,  -68,  -70,  -74,  -75,  -78,
    -81,  -86,  -95, -110, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999,  -999, -105, -100,  -95,  -90,  -85,  -80,
    -75,  -70,  -65,  -61,  -55,  -49,  -39,  -33,
    -40,  -35,  -32,  -38,  -40,  -33,  -35,  -37,
    -46,  -41,  -45,  -44,  -46,  -42,  -45,  -46,
    -52,  -50,  -50,  -50,  -54,  -54,  -55,  -57,
    -62,  -64,  -66,  -68,  -70,  -76,  -81,  -90,
    -100, -110, -999, -999, -999, -999, -999, -999}},
  /* 354 hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -105,  -98,  -90,  -85,  -82,  -83,  -80,  -78,
    -84,  -79,  -80,  -83,  -87,  -89,  -91,  -93,
    -99, -106, -117, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -105,  -98,  -90,  -85,  -80,  -75,  -70,  -68,
    -74,  -72,  -74,  -77,  -80,  -82,  -85,  -87,
    -92,  -89,  -91,  -95, -100, -106, -112, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -105,  -98,  -90,  -83,  -75,  -71,  -63,  -64,
    -67,  -62,  -64,  -67,  -70,  -73,  -77,  -81,
    -84,  -83,  -85,  -89,  -90,  -93,  -98, -104,
    -109, -114, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -103,  -96,  -88,  -81,  -75,  -68,  -58,  -54,
    -56,  -54,  -56,  -56,  -58,  -60,  -63,  -66,
    -74,  -69,  -72,  -72,  -75,  -74,  -77,  -81,
    -81,  -82,  -84,  -87,  -93,  -96,  -99, -104,
    -110, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -108, -102,  -96,
    -91,  -85,  -80,  -74,  -68,  -60,  -51,  -46,
    -48,  -46,  -43,  -45,  -47,  -47,  -49,  -48,
    -56,  -53,  -55,  -58,  -57,  -63,  -58,  -60,
    -66,  -64,  -67,  -70,  -70,  -74,  -77,  -84,
    -86,  -89,  -91,  -93,  -94, -101, -109, -118,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -108, -103,  -98,  -93,  -88,
    -83,  -78,  -73,  -68,  -60,  -53,  -44,  -35,
    -38,  -38,  -34,  -34,  -36,  -40,  -41,  -44,
    -51,  -45,  -46,  -47,  -46,  -54,  -50,  -49,
    -50,  -50,  -50,  -51,  -54,  -57,  -58,  -60,
    -66,  -66,  -66,  -64,  -65,  -68,  -77,  -82,
    -87,  -95, -110, -999, -999, -999, -999, -999}},
  /* 500 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -107, -102,  -97,  -92,  -87,  -83,  -78,  -75,
    -82,  -79,  -83,  -85,  -89,  -92,  -95,  -98,
    -101, -105, -109, -113, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -106,
    -100,  -95,  -90,  -86,  -81,  -78,  -74,  -69,
    -74,  -74,  -76,  -79,  -83,  -84,  -86,  -89,
    -92,  -97,  -93, -100, -103, -107, -110, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -106, -100,
    -95, -90, -87, -83, -80, -75, -69, -60,
    -66, -66, -68, -70, -74, -78, -79, -81,
    -81, -83, -84, -87, -93, -96, -99, -103,
    -107, -110, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -108, -103, -98,
    -93, -89, -85, -82, -78, -71, -62, -55,
    -58, -58, -54, -54, -55, -59, -61, -62,
    -70, -66, -66, -67, -70, -72, -75, -78,
    -84, -84, -84, -88, -91, -90, -95, -98,
    -102, -103, -106, -110, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -108, -103,  -98,  -94,
    -90,  -87,  -82,  -79,  -73,  -67,  -58,  -47,
    -50,  -45,  -41,  -45,  -48,  -44,  -44,  -49,
    -54,  -51,  -48,  -47,  -49,  -50,  -51,  -57,
    -58,  -60,  -63,  -69,  -70,  -69,  -71,  -74,
    -78,  -82,  -90,  -95, -101, -105, -110, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -105, -101, -97, -93, -90,
    -85, -80, -77, -72, -65, -56, -48, -37,
    -40, -36, -34, -40, -50, -47, -38, -41,
    -47, -38, -35, -39, -38, -43, -40, -45,
    -50, -45, -44, -47, -50, -55, -48, -48,
    -52, -66, -70, -76, -82, -90, -97, -105,
    -110, -999, -999, -999, -999, -999, -999, -999}},
  /* 707 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -108, -103,  -98,  -93,  -86,  -79,  -76,
    -83,  -81,  -85,  -87,  -89,  -93,  -98, -102,
    -107, -112, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -108, -103,  -98,  -93,  -86,  -79,  -71,
    -77,  -74,  -77,  -79,  -81,  -84,  -85,  -90,
    -92,  -93,  -92,  -98, -101, -108, -112, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -108, -103,  -98,  -93,  -87,  -78,  -68,  -65,
    -66,  -62,  -65,  -67,  -70,  -73,  -75,  -78,
    -82,  -82,  -83,  -84,  -91,  -93,  -98, -102,
    -106, -110, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -105, -100, -95, -90, -82, -74, -62, -57,
    -58, -56, -51, -52, -52, -54, -54, -58,
    -66, -59, -60, -63, -66, -69, -73, -79,
    -83, -84, -80, -81, -81, -82, -88, -92,
    -98, -105, -113, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -107,
    -102,  -97,  -92,  -84,  -79,  -69,  -57,  -47,
    -52,  -47,  -44,  -45,  -50,  -52,  -42,  -42,
    -53,  -43,  -43,  -48,  -51,  -56,  -55,  -52,
    -57,  -59,  -61,  -62,  -67,  -71,  -78,  -83,
    -86,  -94,  -98, -103, -110, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -105, -100,
    -95,  -90,  -84,  -78,  -70,  -61,  -51,  -41,
    -40,  -38,  -40,  -46,  -52,  -51,  -41,  -40,
    -46,  -40,  -38,  -38,  -41,  -46,  -41,  -46,
    -47,  -43,  -43,  -45,  -41,  -45,  -56,  -67,
    -68,  -83,  -87,  -90,  -95, -102, -107, -113,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 1000 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -109, -105, -101,  -96,  -91,  -84,  -77,
    -82,  -82,  -85,  -89,  -94, -100, -106, -110,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -106, -103,  -98,  -92,  -85,  -80,  -71,
    -75,  -72,  -76,  -80,  -84,  -86,  -89,  -93,
    -100, -107, -113, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -107,
    -104, -101,  -97,  -92,  -88,  -84,  -80,  -64,
    -66,  -63,  -64,  -66,  -69,  -73,  -77,  -83,
    -83,  -86,  -91,  -98, -104, -111, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -107,
    -104, -101,  -97,  -92,  -90,  -84,  -74,  -57,
    -58,  -52,  -55,  -54,  -50,  -52,  -50,  -52,
    -63,  -62,  -69,  -76,  -77,  -78,  -78,  -79,
    -82,  -88,  -94, -100, -106, -111, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -106, -102,
    -98,  -95,  -90,  -85,  -83,  -78,  -70,  -50,
    -50,  -41,  -44,  -49,  -47,  -50,  -50,  -44,
    -55,  -46,  -47,  -48,  -48,  -54,  -49,  -49,
    -58,  -62,  -71,  -81,  -87,  -92,  -97, -102,
    -108, -114, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -106, -102,
    -98,  -95,  -90,  -85,  -83,  -78,  -70,  -45,
    -43,  -41,  -47,  -50,  -51,  -50,  -49,  -45,
    -47,  -41,  -44,  -41,  -39,  -43,  -38,  -37,
    -40,  -41,  -44,  -50,  -58,  -65,  -73,  -79,
    -85,  -92,  -97, -101, -105, -109, -113, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 1414 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -107, -100,  -95,  -87,  -81,
    -85,  -83,  -88,  -93, -100, -107, -114, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -107, -101,  -95,  -88,  -83,  -76,
    -73,  -72,  -79,  -84,  -90,  -95, -100, -105,
    -110, -115, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -104,  -98,  -92,  -87,  -81,  -70,
    -65,  -62,  -67,  -71,  -74,  -80,  -85,  -91,
    -95,  -99, -103, -108, -111, -114, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -103,  -97,  -90,  -85,  -76,  -60,
    -56,  -54,  -60,  -62,  -61,  -56,  -63,  -65,
    -73,  -74,  -77,  -75,  -78,  -81,  -86,  -87,
    -88,  -91,  -94,  -98, -103, -110, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -105,
    -100,  -97,  -92,  -86,  -81,  -79,  -70,  -57,
    -51,  -47,  -51,  -58,  -60,  -56,  -53,  -50,
    -58,  -52,  -50,  -50,  -53,  -55,  -64,  -69,
    -71,  -85,  -82,  -78,  -81,  -85,  -95, -102,
    -112, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -105,
    -100,  -97,  -92,  -85,  -83,  -79,  -72,  -49,
    -40,  -43,  -43,  -54,  -56,  -51,  -50,  -40,
    -43,  -38,  -36,  -35,  -37,  -38,  -37,  -44,
    -54,  -60,  -57,  -60,  -70,  -75,  -84,  -92,
    -103, -112, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 2000 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -102,  -95,  -89,  -82,
    -83,  -84,  -90,  -92,  -99, -107, -113, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -107, -101,  -95,  -89,  -83,  -72,
    -74,  -78,  -85,  -88,  -88,  -90,  -92,  -98,
    -105, -111, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -109, -103, -97, -93, -87, -81, -70,
    -70, -67, -75, -73, -76, -79, -81, -83,
    -88, -89, -97, -103, -110, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -107, -100,  -94,  -88,  -83,  -75,  -63,
    -59,  -59,  -63,  -66,  -60,  -62,  -67,  -67,
    -77,  -76,  -81,  -88,  -86,  -92,  -96, -102,
    -109, -116, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -105,  -98,  -92,  -86,  -81,  -73,  -56,
    -52,  -47,  -55,  -60,  -58,  -52,  -51,  -45,
    -49,  -50,  -53,  -54,  -61,  -71,  -70,  -69,
    -78,  -79,  -87,  -90,  -96, -104, -112, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -103,  -96,  -90,  -86,  -78,  -70,  -51,
    -42,  -47,  -48,  -55,  -54,  -54,  -53,  -42,
    -35,  -28,  -33,  -38,  -37,  -44,  -47,  -49,
    -54,  -63,  -68,  -78,  -82,  -89,  -94,  -99,
    -104, -109, -114, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 2828 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -110, -100,  -90,  -79,
    -85,  -81,  -82,  -82,  -89,  -94,  -99, -103,
    -109, -115, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -105,  -97,  -85,  -72,
    -74,  -70,  -70,  -70,  -76,  -85,  -91,  -93,
    -97, -103, -109, -115, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -112,  -93,  -81,  -68,
    -62,  -60,  -60,  -57,  -63,  -70,  -77,  -82,
    -90,  -93,  -98, -104, -109, -113, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -113, -100,  -93,  -84,  -63,
    -58,  -48,  -53,  -54,  -52,  -52,  -57,  -64,
    -66,  -76,  -83,  -81,  -85,  -85,  -90,  -95,
    -98, -101, -103, -106, -108, -111, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -105,  -95,  -86,  -74,  -53,
    -50,  -38,  -43,  -49,  -43,  -42,  -39,  -39,
    -46,  -52,  -57,  -56,  -72,  -69,  -74,  -81,
    -87,  -92,  -94,  -97,  -99, -102, -105, -108,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -108,  -99,  -90,  -76,  -66,  -45,
    -43,  -41,  -44,  -47,  -43,  -47,  -40,  -30,
    -31,  -31,  -39,  -33,  -40,  -41,  -43,  -53,
    -59,  -70,  -73,  -77,  -79,  -82,  -84,  -87,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 4000 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -110,  -91,  -76,
    -75,  -85,  -93,  -98, -104, -110, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -110,  -91,  -70,
    -70,  -75,  -86,  -89,  -94,  -98, -101, -106,
    -110, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -110,  -95,  -80,  -60,
    -65,  -64,  -74,  -83,  -88,  -91,  -95,  -99,
    -103, -107, -110, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -110,  -95,  -80,  -58,
    -55,  -49,  -66,  -68,  -71,  -78,  -78,  -80,
    -88,  -85,  -89,  -97, -100, -105, -110, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -110,  -95,  -80,  -53,
    -52,  -41,  -59,  -59,  -49,  -58,  -56,  -63,
    -86,  -79,  -90,  -93,  -98, -103, -107, -112,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110,  -97,  -91,  -73,  -45,
    -40,  -33,  -53,  -61,  -49,  -54,  -50,  -50,
    -60,  -52,  -67,  -74,  -81,  -92,  -96, -100,
    -105, -110, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 5657 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -113, -106,  -99,  -92,  -77,
    -80,  -88,  -97, -106, -115, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -116, -109, -102,  -95,  -89,  -74,
    -72,  -88,  -87,  -95, -102, -109, -116, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -116, -109, -102,  -95,  -89,  -75,
    -66,  -74,  -77,  -78,  -86,  -87,  -90,  -96,
    -105, -115, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -115, -108, -101,  -94,  -88,  -66,
    -56,  -61,  -70,  -65,  -78,  -72,  -83,  -84,
    -93,  -98, -105, -110, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -110, -105,  -95,  -89,  -82,  -57,
    -52,  -52,  -59,  -56,  -59,  -58,  -69,  -67,
    -88,  -82,  -82,  -89,  -94, -100, -108, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -110, -101,  -96,  -90,  -83,  -77,  -54,
    -43,  -38,  -50,  -48,  -52,  -48,  -42,  -42,
    -51,  -52,  -53,  -59,  -65,  -71,  -78,  -85,
    -95, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 8000 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -120, -105,  -86,  -68,
    -78,  -79,  -90, -100, -110, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -120, -105,  -86,  -66,
    -73,  -77,  -88,  -96, -105, -115, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -120, -105,  -92,  -80,  -61,
    -64,  -68,  -80,  -87,  -92, -100, -110, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -120, -104,  -91,  -79,  -52,
    -60,  -54,  -64,  -69,  -77,  -80,  -82,  -84,
    -85,  -87,  -88,  -90, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -118, -100,  -87,  -77,  -49,
    -50,  -44,  -58,  -61,  -61,  -67,  -65,  -62,
    -62,  -62,  -65,  -68, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -115,  -98,  -84,  -62,  -49,
    -44,  -38,  -46,  -49,  -49,  -46,  -39,  -37,
    -39,  -40,  -42,  -43, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 11314 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -110,  -88,  -74,
    -77,  -82,  -82,  -85,  -90,  -94,  -99, -104,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -110,  -88,  -66,
    -70,  -81,  -80,  -81,  -84,  -88,  -91,  -93,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -110,  -88,  -61,
    -63,  -70,  -71,  -74,  -77,  -80,  -83,  -85,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -110, -86, -62,
    -63,  -62,  -62,  -58,  -52,  -50,  -50,  -52,
    -54, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -118, -108,  -84,  -53,
    -50,  -50,  -50,  -55,  -47,  -45,  -40,  -40,
    -40, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -118, -100,  -73,  -43,
    -37,  -42,  -43,  -53,  -38,  -37,  -35,  -35,
    -38, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 16000 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -100,  -91,  -84,  -74,
    -80,  -80,  -80,  -80,  -80, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -100,  -91,  -84,  -74,
    -68,  -68,  -68,  -68,  -68, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -100,  -86,  -78,  -70,
    -60,  -45,  -30,  -21, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -100,  -87,  -78,  -67,
    -48,  -38,  -29,  -21, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -100,  -86,  -69,  -56,
    -45,  -35,  -33,  -29, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -100,  -83,  -71,  -48,
    -27,  -38,  -37,  -34, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}}
};





#define NEGINF -9999.f
double stereo_threshholds[]={0.0, .5, 1.0, 1.5, 2.5, 4.5, 8.5, 16.5, 9e10};

vorbis_look_psy_global *_vp_global_look(vorbis_info *vi){
  codec_setup_info *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  vorbis_info_psy_global *gi=&ci->psy_g_param;
  vorbis_look_psy_global *look=static_cast<vorbis_look_psy_global*>(_ogg_calloc(1,sizeof(*look)));

  look->channels=vi->channels;

  look->ampmax=-9999.;
  look->gi=gi;
  return(look);
}

void _vp_global_free(vorbis_look_psy_global *look){
  if(look){
    memset(look,0,sizeof(*look));
    _ogg_free(look);
  }
}

void _vi_gpsy_free(vorbis_info_psy_global *i){
  if(i){
    memset(i,0,sizeof(*i));
    _ogg_free(i);
  }
}

void _vi_psy_free(vorbis_info_psy *i){
  if(i){
    memset(i,0,sizeof(*i));
    _ogg_free(i);
  }
}

void min_curve(float *c,
               float *c2){
  int i;
  for(i=0;i<EHMER_MAX;i++)if(c2[i]<c[i])c[i]=c2[i];
}
void max_curve(float *c,
               float *c2){
  int i;
  for(i=0;i<EHMER_MAX;i++)if(c2[i]>c[i])c[i]=c2[i];
}

void attenuate_curve(float *c,float att){
  int i;
  for(i=0;i<EHMER_MAX;i++)
    c[i]+=att;
}

float ***setup_tone_curves(float curveatt_dB[P_BANDS],float binHz,int n,
                  float center_boost, float center_decay_rate){
  int i,j,k,m;
  float ath[EHMER_MAX];
  float workc[P_BANDS][P_LEVELS][EHMER_MAX];
  float athc[P_LEVELS][EHMER_MAX];
  float *brute_buffer=static_cast<float*>(alloca(n*sizeof(*brute_buffer)));

  float ***ret=static_cast<float***>(_ogg_malloc(sizeof(*ret)*P_BANDS));

  memset(workc,0,sizeof(workc));

  for(i=0;i<P_BANDS;i++){
    /* we add back in the ATH to avoid low level curves falling off to
       -infinity and unnecessarily cutting off high level curves in the
       curve limiting (last step). */

    /* A half-band's settings must be valid over the whole band, and
       it's better to mask too little than too much */
    int ath_offset=i*4;
    for(j=0;j<EHMER_MAX;j++){
      float min=999.;
      for(k=0;k<4;k++)
    if(j+k+ath_offset<MAX_ATH){
      if(min>ATH[j+k+ath_offset])min=ATH[j+k+ath_offset];
    }else{
      if(min>ATH[MAX_ATH-1])min=ATH[MAX_ATH-1];
    }
      ath[j]=min;
    }

    /* copy curves into working space, replicate the 50dB curve to 30
       and 40, replicate the 100dB curve to 110 */
    for(j=0;j<6;j++)
      memcpy(workc[i][j+2],tonemasks[i][j],EHMER_MAX*sizeof(*tonemasks[i][j]));
    memcpy(workc[i][0],tonemasks[i][0],EHMER_MAX*sizeof(*tonemasks[i][0]));
    memcpy(workc[i][1],tonemasks[i][0],EHMER_MAX*sizeof(*tonemasks[i][0]));

    /* apply centered curve boost/decay */
    for(j=0;j<P_LEVELS;j++){
      for(k=0;k<EHMER_MAX;k++){
    float adj=center_boost+abs(EHMER_OFFSET-k)*center_decay_rate;
    if(adj<0. && center_boost>0)adj=0.;
    if(adj>0. && center_boost<0)adj=0.;
    workc[i][j][k]+=adj;
      }
    }

    /* normalize curves so the driving amplitude is 0dB */
    /* make temp curves with the ATH overlayed */
    for(j=0;j<P_LEVELS;j++){
      attenuate_curve(workc[i][j],(float)(curveatt_dB[i]+100.-(j<2?2:j)*10.-P_LEVEL_0));
      memcpy(athc[j],ath,EHMER_MAX*sizeof(**athc));
      attenuate_curve(athc[j],(float)(+100.-j*10.f-P_LEVEL_0));
      max_curve(athc[j],workc[i][j]);
    }

    /* Now limit the louder curves.

       the idea is this: We don't know what the playback attenuation
       will be; 0dB SL moves every time the user twiddles the volume
       knob. So that means we have to use a single 'most pessimal' curve
       for all masking amplitudes, right?  Wrong.  The *loudest* sound
       can be in (we assume) a range of ...+100dB] SL.  However, sounds
       20dB down will be in a range ...+80], 40dB down is from ...+60],
       etc... */

    for(j=1;j<P_LEVELS;j++){
      min_curve(athc[j],athc[j-1]);
      min_curve(workc[i][j],athc[j]);
    }
  }

  for(i=0;i<P_BANDS;i++){
    int hi_curve,lo_curve,bin;
    ret[i]=static_cast<float**>(_ogg_malloc(sizeof(**ret)*P_LEVELS));

    /* low frequency curves are measured with greater resolution than
       the MDCT/FFT will actually give us; we want the curve applied
       to the tone data to be pessimistic and thus apply the minimum
       masking possible for a given bin.  That means that a single bin
       could span more than one octave and that the curve will be a
       composite of multiple octaves.  It also may mean that a single
       bin may span > an eighth of an octave and that the eighth
       octave values may also be composited. */

    /* which octave curves will we be compositing? */
    bin=(int)floor(fromOC(i*.5)/binHz);
    lo_curve=  (int)ceil(toOC(bin*binHz+1)*2);
    hi_curve=  (int)floor(toOC((bin+1)*binHz)*2);
    if(lo_curve>i)lo_curve=i;
    if(lo_curve<0)lo_curve=0;
    if(hi_curve>=P_BANDS)hi_curve=P_BANDS-1;

    for(m=0;m<P_LEVELS;m++){
      ret[i][m]=static_cast<float*>(_ogg_malloc(sizeof(***ret)*(EHMER_MAX+2)));

      for(j=0;j<n;j++)brute_buffer[j]=999.;

      /* render the curve into bins, then pull values back into curve.
     The point is that any inherent subsampling aliasing results in
     a safe minimum */
      for(k=lo_curve;k<=hi_curve;k++){
    int l=0;

    for(j=0;j<EHMER_MAX;j++){
      int lo_bin= (int)(fromOC(j*.125+k*.5-2.0625)/binHz);
      int hi_bin= (int)(fromOC(j*.125+k*.5-1.9375)/binHz+1);

      if(lo_bin<0)lo_bin=0;
      if(lo_bin>n)lo_bin=n;
      if(lo_bin<l)l=lo_bin;
      if(hi_bin<0)hi_bin=0;
      if(hi_bin>n)hi_bin=n;

      for(;l<hi_bin && l<n;l++)
        if(brute_buffer[l]>workc[k][m][j])
          brute_buffer[l]=workc[k][m][j];
    }

    for(;l<n;l++)
      if(brute_buffer[l]>workc[k][m][EHMER_MAX-1])
        brute_buffer[l]=workc[k][m][EHMER_MAX-1];

      }

      /* be equally paranoid about being valid up to next half ocatve */
      if(i+1<P_BANDS){
    int l=0;
    k=i+1;
    for(j=0;j<EHMER_MAX;j++){
      int lo_bin= (int)(fromOC(j*.125+i*.5-2.0625)/binHz);
      int hi_bin= (int)(fromOC(j*.125+i*.5-1.9375)/binHz+1);

      if(lo_bin<0)lo_bin=0;
      if(lo_bin>n)lo_bin=n;
      if(lo_bin<l)l=lo_bin;
      if(hi_bin<0)hi_bin=0;
      if(hi_bin>n)hi_bin=n;

      for(;l<hi_bin && l<n;l++)
        if(brute_buffer[l]>workc[k][m][j])
          brute_buffer[l]=workc[k][m][j];
    }

    for(;l<n;l++)
      if(brute_buffer[l]>workc[k][m][EHMER_MAX-1])
        brute_buffer[l]=workc[k][m][EHMER_MAX-1];

      }


      for(j=0;j<EHMER_MAX;j++){
    int bin=(int)(fromOC(j*.125+i*.5-2.)/binHz);
    if(bin<0){
      ret[i][m][j+2]=-999.;
    }else{
      if(bin>=n){
        ret[i][m][j+2]=-999.;
      }else{
        ret[i][m][j+2]=brute_buffer[bin];
      }
    }
      }

      /* add fenceposts */
      for(j=0;j<EHMER_OFFSET;j++)
    if(ret[i][m][j+2]>-200.f)break;
      ret[i][m][0]=(float)j;

      for(j=EHMER_MAX-1;j>EHMER_OFFSET+1;j--)
    if(ret[i][m][j+2]>-200.f)
      break;
      ret[i][m][1]=(float)j;

    }
  }

  return(ret);
}

void _vp_psy_init(vorbis_look_psy *p,vorbis_info_psy *vi,
          vorbis_info_psy_global *gi,int n,long rate){
  long i,j,lo=-99,hi=1;
  long maxoc;
  memset(p,0,sizeof(*p));

  p->eighth_octave_lines=gi->eighth_octave_lines;
  p->shiftoc=(long)(rint(log(gi->eighth_octave_lines*8.f)/log(2.f))-1);

  p->firstoc=(long)(toOC(.25f*rate*.5/n)*(1<<(p->shiftoc+1))-gi->eighth_octave_lines);
  maxoc=(long)(toOC((n+.25f)*rate*.5/n)*(1<<(p->shiftoc+1))+.5f);
  p->total_octave_lines=maxoc-p->firstoc+1;
  p->ath=static_cast<float*>(_ogg_malloc(n*sizeof(*p->ath)));

  p->octave=static_cast<long*>(_ogg_malloc(n*sizeof(*p->octave)));
  p->bark=static_cast<long*>(_ogg_malloc(n*sizeof(*p->bark)));
  p->vi=vi;
  p->n=n;
  p->rate=rate;

  /* set up the lookups for a given blocksize and sample rate */

  for(i=0,j=0;i<MAX_ATH-1;i++){
    int endpos=(int)rint(fromOC((i+1)*.125-2.)*2*n/rate);
    float base=ATH[i];
    if(j<endpos){
      float delta=(ATH[i+1]-base)/(endpos-j);
      for(;j<endpos && j<n;j++){
        p->ath[j]=(float)(base+100.);
        base+=delta;
      }
    }
  }

  for(i=0;i<n;i++){
    float bark=(float)toBARK(rate/(2*n)*i);

    for(;lo+vi->noisewindowlomin<i &&
      toBARK(rate/(2*n)*lo)<(bark-vi->noisewindowlo);lo++);

    for(;hi<=n && (hi<i+vi->noisewindowhimin ||
      toBARK(rate/(2*n)*hi)<(bark+vi->noisewindowhi));hi++);

    p->bark[i]=((lo-1)<<16)+(hi-1);

  }

  for(i=0;i<n;i++)
    p->octave[i]=(long)(toOC((i+.25f)*.5*rate/n)*(1<<(p->shiftoc+1))+.5f);

  p->tonecurves=setup_tone_curves(vi->toneatt,(float)(rate*.5/n),n,
                  vi->tone_centerboost,vi->tone_decay);

  /* set up rolling noise median */
  p->noiseoffset=static_cast<float**>(_ogg_malloc(P_NOISECURVES*sizeof(*p->noiseoffset)));
  for(i=0;i<P_NOISECURVES;i++)
    p->noiseoffset[i]=static_cast<float*>(_ogg_malloc(n*sizeof(**p->noiseoffset)));

  for(i=0;i<n;i++){
    float halfoc=(float)(toOC((i+.5)*rate/(2.*n))*2.);
    int inthalfoc;
    float del;

    if(halfoc<0)halfoc=0;
    if(halfoc>=P_BANDS-1)halfoc=P_BANDS-1;
    inthalfoc=(int)halfoc;
    del=halfoc-inthalfoc;

    for(j=0;j<P_NOISECURVES;j++)
      p->noiseoffset[j][i]=(float)(p->vi->noiseoff[j][inthalfoc]*(1.-del) + p->vi->noiseoff[j][inthalfoc+1]*del);

  }
#if 0
  {
    int ls=0;
    _analysis_output_always("noiseoff0",ls,p->noiseoffset[0],n,1,0,0);
    _analysis_output_always("noiseoff1",ls,p->noiseoffset[1],n,1,0,0);
    _analysis_output_always("noiseoff2",ls++,p->noiseoffset[2],n,1,0,0);
  }
#endif
}

void _vp_psy_clear(vorbis_look_psy *p){
  int i,j;
  if(p){
    if(p->ath)_ogg_free(p->ath);
    if(p->octave)_ogg_free(p->octave);
    if(p->bark)_ogg_free(p->bark);
    if(p->tonecurves){
      for(i=0;i<P_BANDS;i++){
    for(j=0;j<P_LEVELS;j++){
      _ogg_free(p->tonecurves[i][j]);
    }
    _ogg_free(p->tonecurves[i]);
      }
      _ogg_free(p->tonecurves);
    }
    if(p->noiseoffset){
      for(i=0;i<P_NOISECURVES;i++){
        _ogg_free(p->noiseoffset[i]);
      }
      _ogg_free(p->noiseoffset);
    }
    memset(p,0,sizeof(*p));
  }
}

/* octave/(8*eighth_octave_lines) x scale and dB y scale */
void seed_curve(float *seed,
               const float **curves,
               float amp,
               int oc, int n,
               int linesper,float dBoffset){
  int i,post1;
  int seedptr;
  const float *posts,*curve;

  int choice=(int)((amp+dBoffset-P_LEVEL_0)*.1f);
  choice=max(choice,0);
  choice=min(choice,P_LEVELS-1);
  posts=curves[choice];
  curve=posts+2;
  post1=(int)posts[1];
  seedptr=(int)(oc+(posts[0]-EHMER_OFFSET)*linesper-(linesper>>1));

  for(i=(int)posts[0];i<post1;i++){
    if(seedptr>0){
      float lin=amp+curve[i];
      if(seed[seedptr]<lin)seed[seedptr]=lin;
    }
    seedptr+=linesper;
    if(seedptr>=n)break;
  }
}

void seed_loop(vorbis_look_psy *p,
              const float ***curves,
              const float *f,
              const float *flr,
              float *seed,
              float specmax){
  vorbis_info_psy *vi=p->vi;
  long n=p->n,i;
  float dBoffset=vi->max_curve_dB-specmax;

  /* prime the working vector with peak values */

  for(i=0;i<n;i++){
    float max=f[i];
    long oc=p->octave[i];
    while(i+1<n && p->octave[i+1]==oc){
      i++;
      if(f[i]>max)max=f[i];
    }

    if(max+6.f>flr[i]){
      oc=oc>>p->shiftoc;

      if(oc>=P_BANDS)oc=P_BANDS-1;
      if(oc<0)oc=0;

      seed_curve(seed,
         curves[oc],
         max,
         p->octave[i]-p->firstoc,
         p->total_octave_lines,
         p->eighth_octave_lines,
         dBoffset);
    }
  }
}

void seed_chase(float *seeds, int linesper, long n){
  long  *posstack=static_cast<long*>(alloca(n*sizeof(*posstack)));
  float *ampstack=static_cast<float*>(alloca(n*sizeof(*ampstack)));
  long   stack=0;
  long   pos=0;
  long   i;

  for(i=0;i<n;i++){
    if(stack<2){
      posstack[stack]=i;
      ampstack[stack++]=seeds[i];
    }else{
      for(;;){
    if(seeds[i]<ampstack[stack-1]){
      posstack[stack]=i;
      ampstack[stack++]=seeds[i];
      break;
    }else{
      if(i<posstack[stack-1]+linesper){
        if(stack>1 && ampstack[stack-1]<=ampstack[stack-2] &&
           i<posstack[stack-2]+linesper){
          /* we completely overlap, making stack-1 irrelevant.  pop it */
          stack--;
          continue;
        }
      }
      posstack[stack]=i;
      ampstack[stack++]=seeds[i];
      break;

    }
      }
    }
  }

  /* the stack now contains only the positions that are relevant. Scan
     'em straight through */

  for(i=0;i<stack;i++){
    long endpos;
    if(i<stack-1 && ampstack[i+1]>ampstack[i]){
      endpos=posstack[i+1];
    }else{
      endpos=posstack[i]+linesper+1; /* +1 is important, else bin 0 is
                    discarded in short frames */
    }
    if(endpos>n)endpos=n;
    for(;pos<endpos;pos++)
      seeds[pos]=ampstack[i];
  }

  /* there.  Linear time.  I now remember this was on a problem set I
     had in Grad Skool... I didn't solve it at the time ;-) */

}

/* bleaugh, this is more complicated than it needs to be */
#include<stdio.h>
void max_seeds(vorbis_look_psy *p,
              float *seed,
              float *flr){
  long   n=p->total_octave_lines;
  int    linesper=p->eighth_octave_lines;
  long   linpos=0;
  long   pos;

  seed_chase(seed,linesper,n); /* for masking */

  pos=p->octave[0]-p->firstoc-(linesper>>1);

  while(linpos+1<p->n){
    float minV=seed[pos];
    long end=((p->octave[linpos]+p->octave[linpos+1])>>1)-p->firstoc;
    if(minV>p->vi->tone_abs_limit)minV=p->vi->tone_abs_limit;
    while(pos+1<=end){
      pos++;
      if((seed[pos]>NEGINF && seed[pos]<minV) || minV==NEGINF)
    minV=seed[pos];
    }

    end=pos+p->firstoc;
    for(;linpos<p->n && p->octave[linpos]<=end;linpos++)
      if(flr[linpos]<minV)flr[linpos]=minV;
  }

  {
    float minV=seed[p->total_octave_lines-1];
    for(;linpos<p->n;linpos++)
      if(flr[linpos]<minV)flr[linpos]=minV;
  }

}

void bark_noise_hybridmp(int n,const long *b,
                                const float *f,
                                float *noise,
                                const float offset,
                                const int fixed){

  float *N=static_cast<float*>(alloca(n*sizeof(*N)));
  float *X=static_cast<float*>(alloca(n*sizeof(*N)));
  float *XX=static_cast<float*>(alloca(n*sizeof(*N)));
  float *Y=static_cast<float*>(alloca(n*sizeof(*N)));
  float *XY=static_cast<float*>(alloca(n*sizeof(*N)));

  float tN, tX, tXX, tY, tXY;
  int i;

  int lo, hi;
  float R, A=0, B=0, D=0;
  float w, x, y;

  tN = tX = tXX = tY = tXY = 0.f;

  y = f[0] + offset;
  if (y < 1.f) y = 1.f;

  w = (float)(y * y * .5);

  tN += w;
  tX += w;
  tY += w * y;

  N[0] = tN;
  X[0] = tX;
  XX[0] = tXX;
  Y[0] = tY;
  XY[0] = tXY;

  for (i = 1, x = 1.f; i < n; i++, x += 1.f) {

    y = f[i] + offset;
    if (y < 1.f) y = 1.f;

    w = y * y;

    tN += w;
    tX += w * x;
    tXX += w * x * x;
    tY += w * y;
    tXY += w * x * y;

    N[i] = tN;
    X[i] = tX;
    XX[i] = tXX;
    Y[i] = tY;
    XY[i] = tXY;
  }

  for (i = 0, x = 0.f;; i++, x += 1.f) {

    lo = b[i] >> 16;
    if( lo>=0 ) break;
    hi = b[i] & 0xffff;

    tN = N[hi] + N[-lo];
    tX = X[hi] - X[-lo];
    tXX = XX[hi] + XX[-lo];
    tY = Y[hi] + Y[-lo];
    tXY = XY[hi] - XY[-lo];

    A = tY * tXX - tX * tXY;
    B = tN * tXY - tX * tY;
    D = tN * tXX - tX * tX;
    R = (A + x * B) / D;
    if (R < 0.f)
      R = 0.f;

    noise[i] = R - offset;
  }

  for ( ;; i++, x += 1.f) {

    lo = b[i] >> 16;
    hi = b[i] & 0xffff;
    if(hi>=n)break;

    tN = N[hi] - N[lo];
    tX = X[hi] - X[lo];
    tXX = XX[hi] - XX[lo];
    tY = Y[hi] - Y[lo];
    tXY = XY[hi] - XY[lo];

    A = tY * tXX - tX * tXY;
    B = tN * tXY - tX * tY;
    D = tN * tXX - tX * tX;
    R = (A + x * B) / D;
    if (R < 0.f) R = 0.f;

    noise[i] = R - offset;
  }
  for ( ; i < n; i++, x += 1.f) {

    R = (A + x * B) / D;
    if (R < 0.f) R = 0.f;

    noise[i] = R - offset;
  }

  if (fixed <= 0) return;

  for (i = 0, x = 0.f;; i++, x += 1.f) {
    hi = i + fixed / 2;
    lo = hi - fixed;
    if(lo>=0)break;

    tN = N[hi] + N[-lo];
    tX = X[hi] - X[-lo];
    tXX = XX[hi] + XX[-lo];
    tY = Y[hi] + Y[-lo];
    tXY = XY[hi] - XY[-lo];


    A = tY * tXX - tX * tXY;
    B = tN * tXY - tX * tY;
    D = tN * tXX - tX * tX;
    R = (A + x * B) / D;

    if (R - offset < noise[i]) noise[i] = R - offset;
  }
  for ( ;; i++, x += 1.f) {

    hi = i + fixed / 2;
    lo = hi - fixed;
    if(hi>=n)break;

    tN = N[hi] - N[lo];
    tX = X[hi] - X[lo];
    tXX = XX[hi] - XX[lo];
    tY = Y[hi] - Y[lo];
    tXY = XY[hi] - XY[lo];

    A = tY * tXX - tX * tXY;
    B = tN * tXY - tX * tY;
    D = tN * tXX - tX * tX;
    R = (A + x * B) / D;

    if (R - offset < noise[i]) noise[i] = R - offset;
  }
  for ( ; i < n; i++, x += 1.f) {
    R = (A + x * B) / D;
    if (R - offset < noise[i]) noise[i] = R - offset;
  }
}

float FLOOR1_fromdB_INV_LOOKUP[256]={
  0.F, 8.81683e+06F, 8.27882e+06F, 7.77365e+06F,
  7.29930e+06F, 6.85389e+06F, 6.43567e+06F, 6.04296e+06F,
  5.67422e+06F, 5.32798e+06F, 5.00286e+06F, 4.69759e+06F,
  4.41094e+06F, 4.14178e+06F, 3.88905e+06F, 3.65174e+06F,
  3.42891e+06F, 3.21968e+06F, 3.02321e+06F, 2.83873e+06F,
  2.66551e+06F, 2.50286e+06F, 2.35014e+06F, 2.20673e+06F,
  2.07208e+06F, 1.94564e+06F, 1.82692e+06F, 1.71544e+06F,
  1.61076e+06F, 1.51247e+06F, 1.42018e+06F, 1.33352e+06F,
  1.25215e+06F, 1.17574e+06F, 1.10400e+06F, 1.03663e+06F,
  973377.F, 913981.F, 858210.F, 805842.F,
  756669.F, 710497.F, 667142.F, 626433.F,
  588208.F, 552316.F, 518613.F, 486967.F,
  457252.F, 429351.F, 403152.F, 378551.F,
  355452.F, 333762.F, 313396.F, 294273.F,
  276316.F, 259455.F, 243623.F, 228757.F,
  214798.F, 201691.F, 189384.F, 177828.F,
  166977.F, 156788.F, 147221.F, 138237.F,
  129802.F, 121881.F, 114444.F, 107461.F,
  100903.F, 94746.3F, 88964.9F, 83536.2F,
  78438.8F, 73652.5F, 69158.2F, 64938.1F,
  60975.6F, 57254.9F, 53761.2F, 50480.6F,
  47400.3F, 44507.9F, 41792.0F, 39241.9F,
  36847.3F, 34598.9F, 32487.7F, 30505.3F,
  28643.8F, 26896.0F, 25254.8F, 23713.7F,
  22266.7F, 20908.0F, 19632.2F, 18434.2F,
  17309.4F, 16253.1F, 15261.4F, 14330.1F,
  13455.7F, 12634.6F, 11863.7F, 11139.7F,
  10460.0F, 9821.72F, 9222.39F, 8659.64F,
  8131.23F, 7635.06F, 7169.17F, 6731.70F,
  6320.93F, 5935.23F, 5573.06F, 5232.99F,
  4913.67F, 4613.84F, 4332.30F, 4067.94F,
  3819.72F, 3586.64F, 3367.78F, 3162.28F,
  2969.31F, 2788.13F, 2617.99F, 2458.24F,
  2308.24F, 2167.39F, 2035.14F, 1910.95F,
  1794.35F, 1684.85F, 1582.04F, 1485.51F,
  1394.86F, 1309.75F, 1229.83F, 1154.78F,
  1084.32F, 1018.15F, 956.024F, 897.687F,
  842.910F, 791.475F, 743.179F, 697.830F,
  655.249F, 615.265F, 577.722F, 542.469F,
  509.367F, 478.286F, 449.101F, 421.696F,
  395.964F, 371.803F, 349.115F, 327.812F,
  307.809F, 289.026F, 271.390F, 254.830F,
  239.280F, 224.679F, 210.969F, 198.096F,
  186.008F, 174.658F, 164.000F, 153.993F,
  144.596F, 135.773F, 127.488F, 119.708F,
  112.404F, 105.545F, 99.1046F, 93.0572F,
  87.3788F, 82.0469F, 77.0404F, 72.3394F,
  67.9252F, 63.7804F, 59.8885F, 56.2341F,
  52.8027F, 49.5807F, 46.5553F, 43.7144F,
  41.0470F, 38.5423F, 36.1904F, 33.9821F,
  31.9085F, 29.9614F, 28.1332F, 26.4165F,
  24.8045F, 23.2910F, 21.8697F, 20.5352F,
  19.2822F, 18.1056F, 17.0008F, 15.9634F,
  14.9893F, 14.0746F, 13.2158F, 12.4094F,
  11.6522F, 10.9411F, 10.2735F, 9.64662F,
  9.05798F, 8.50526F, 7.98626F, 7.49894F,
  7.04135F, 6.61169F, 6.20824F, 5.82941F,
  5.47370F, 5.13970F, 4.82607F, 4.53158F,
  4.25507F, 3.99542F, 3.75162F, 3.52269F,
  3.30774F, 3.10590F, 2.91638F, 2.73842F,
  2.57132F, 2.41442F, 2.26709F, 2.12875F,
  1.99885F, 1.87688F, 1.76236F, 1.65482F,
  1.55384F, 1.45902F, 1.36999F, 1.28640F,
  1.20790F, 1.13419F, 1.06499F, 1.F
};

void _vp_remove_floor(vorbis_look_psy *p,
              float *mdct,
              int *codedflr,
              float *residue,
              int sliding_lowpass){

  int i,n=p->n;

  if(sliding_lowpass>n)sliding_lowpass=n;

  for(i=0;i<sliding_lowpass;i++){
    residue[i]=
      mdct[i]*FLOOR1_fromdB_INV_LOOKUP[codedflr[i]];
  }

  for(;i<n;i++)
    residue[i]=0.;
}

void _vp_noisemask(vorbis_look_psy *p,
           float *logmdct,
           float *logmask){

  int i,n=p->n;
  float *work=static_cast<float*>(alloca(n*sizeof(*work)));

  bark_noise_hybridmp(n,p->bark,logmdct,logmask,
              140.,-1);

  for(i=0;i<n;i++)work[i]=logmdct[i]-logmask[i];

  bark_noise_hybridmp(n,p->bark,work,logmask,0.,
              p->vi->noisewindowfixed);

  for(i=0;i<n;i++)work[i]=logmdct[i]-work[i];

#if 0
  {
    int seq=0;

    float work2[n];
    for(i=0;i<n;i++){
      work2[i]=logmask[i]+work[i];
    }

    if(seq&1)
      _analysis_output("median2R",seq/2,work,n,1,0,0);
    else
      _analysis_output("median2L",seq/2,work,n,1,0,0);

    if(seq&1)
      _analysis_output("envelope2R",seq/2,work2,n,1,0,0);
    else
      _analysis_output("envelope2L",seq/2,work2,n,1,0,0);
    seq++;
  }
#endif

  for(i=0;i<n;i++){
    int dB=(int)(logmask[i]+.5);
    if(dB>=NOISE_COMPAND_LEVELS)dB=NOISE_COMPAND_LEVELS-1;
    if(dB<0)dB=0;
    logmask[i]= work[i]+p->vi->noisecompand[dB];
  }

}

void _vp_tonemask(vorbis_look_psy *p,
          float *logfft,
          float *logmask,
          float global_specmax,
          float local_specmax){

  int i,n=p->n;

  float *seed=static_cast<float*>(alloca(sizeof(*seed)*p->total_octave_lines));
  float att=local_specmax+p->vi->ath_adjatt;
  for(i=0;i<p->total_octave_lines;i++)seed[i]=NEGINF;

  /* set the ATH (floating below localmax, not global max by a
     specified att) */
  if(att<p->vi->ath_maxatt)att=p->vi->ath_maxatt;

  for(i=0;i<n;i++)
    logmask[i]=p->ath[i]+att;

  /* tone masking */
  seed_loop(p,(const float ***)p->tonecurves,logfft,logmask,seed,global_specmax);
  max_seeds(p,seed,logmask);

}

void _vp_offset_and_mix(vorbis_look_psy *p,
            float *noise,
            float *tone,
            int offset_select,
            float *logmask){
  int i,n=p->n;
  float toneatt=p->vi->tone_masteratt[offset_select];

  for(i=0;i<n;i++){
    float val= noise[i]+p->noiseoffset[offset_select][i];
    if(val>p->vi->noisemaxsupp)val=p->vi->noisemaxsupp;
    logmask[i]=max(val,tone[i]+toneatt);
  }
}

float _vp_ampmax_decay(float amp,vorbis_dsp_state *vd){
  vorbis_info *vi=vd->vi;
  codec_setup_info *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  vorbis_info_psy_global *gi=&ci->psy_g_param;

  int n=ci->blocksizes[vd->W]/2;
  float secs=(float)n/vi->rate;

  amp+=secs*gi->ampmax_att_per_sec;
  if(amp<-9999)amp=-9999;
  return(amp);
}

void couple_lossless(float A, float B,
                float *qA, float *qB){
  int test1=fabs(*qA)>fabs(*qB);
  test1-= fabs(*qA)<fabs(*qB);

  if(!test1)test1=((fabs(A)>fabs(B))<<1)-1;
  if(test1==1){
    *qB=(*qA>0.f?*qA-*qB:*qB-*qA);
  }else{
    float temp=*qB;
    *qB=(*qB>0.f?*qA-*qB:*qB-*qA);
    *qA=temp;
  }

  if(*qB>fabs(*qA)*1.9999f){
    *qB= (float)(-fabs(*qA)*2.f);
    *qA= -*qA;
  }
}

float hypot_lookup[32]={
  -0.009935f, -0.011245f, -0.012726f, -0.014397f,
  -0.016282f, -0.018407f, -0.020800f, -0.023494f,
  -0.026522f, -0.029923f, -0.033737f, -0.038010f,
  -0.042787f, -0.048121f, -0.054064f, -0.060671f,
  -0.068000f, -0.076109f, -0.085054f, -0.094892f,
  -0.105675f, -0.117451f, -0.130260f, -0.144134f,
  -0.159093f, -0.175146f, -0.192286f, -0.210490f,
  -0.229718f, -0.249913f, -0.271001f, -0.292893f};

void precomputed_couple_point(float premag,
                     int floorA,int floorB,
                     float *mag, float *ang){

  int test=(floorA>floorB)-1;
  int offset=31-abs(floorA-floorB);
  float floormag=hypot_lookup[((offset<0)-1)&offset]+1.f;

  floormag*=FLOOR1_fromdB_INV_LOOKUP[(floorB&test)|(floorA&(~test))];

  *mag=premag*floormag;
  *ang=0.f;
}

/* just like below, this is currently set up to only do
   single-step-depth coupling.  Otherwise, we'd have to do more
   copying (which will be inevitable later) */

/* doing the real circular magnitude calculation is audibly superior
   to (A+B)/sqrt(2) */
float dipole_hypot(float a, float b){
  if(a>0.){
    if(b>0.)return (float)sqrt(a*a+b*b);
    if(a>-b)return (float)sqrt(a*a-b*b);
    return (float)(-sqrt(b*b-a*a));
  }
  if(b<0.)return (float)(-sqrt(a*a+b*b));
  if(-a>b)return (float)(-sqrt(a*a-b*b));
  return (float)sqrt(b*b-a*a);
}
float round_hypot(float a, float b){
  if(a>0.){
    if(b>0.)return (float)sqrt(a*a+b*b);
    if(a>-b)return (float)sqrt(a*a+b*b);
    return (float)(-sqrt(b*b+a*a));
  }
  if(b<0.)return (float)(-sqrt(a*a+b*b));
  if(-a>b)return (float)(-sqrt(a*a+b*b));
  return (float)sqrt(b*b+a*a);
}

/* revert to round hypot for now */
float **_vp_quantize_couple_memo(vorbis_block *vb,
                 vorbis_info_psy_global *g,
                 vorbis_look_psy *p,
                 vorbis_info_mapping0 *vi,
                 float **mdct){

  int i,j,n=p->n;
  float **ret=static_cast<float**>(_vorbis_block_alloc(vb,vi->coupling_steps*sizeof(*ret)));
  int limit=g->coupling_pointlimit[p->vi->blockflag][PACKETBLOBS/2];

  for(i=0;i<vi->coupling_steps;i++){
    float *mdctM=mdct[vi->coupling_mag[i]];
    float *mdctA=mdct[vi->coupling_ang[i]];
    ret[i]=static_cast<float*>(_vorbis_block_alloc(vb,n*sizeof(**ret)));
    for(j=0;j<limit;j++)
      ret[i][j]=dipole_hypot(mdctM[j],mdctA[j]);
    for(;j<n;j++)
      ret[i][j]=round_hypot(mdctM[j],mdctA[j]);
  }

  return(ret);
}

/* this is for per-channel noise normalization */
int apsort(const void *a, const void *b){
  float f1=(float)fabs(**(float**)a);
  float f2=(float)fabs(**(float**)b);
  return (f1<f2)-(f1>f2);
}

int **_vp_quantize_couple_sort(vorbis_block *vb,
                   vorbis_look_psy *p,
                   vorbis_info_mapping0 *vi,
                   float **mags){


  if(p->vi->normal_point_p){
    int i,j,k,n=p->n;
    int **ret=static_cast<int**>(_vorbis_block_alloc(vb,vi->coupling_steps*sizeof(*ret)));
    int partition=p->vi->normal_partition;
    float **work=static_cast<float**>(alloca(sizeof(*work)*partition));

    for(i=0;i<vi->coupling_steps;i++){
      ret[i]=static_cast<int*>(_vorbis_block_alloc(vb,n*sizeof(**ret)));

      for(j=0;j<n;j+=partition){
    for(k=0;k<partition;k++)work[k]=mags[i]+k+j;
    qsort(work,partition,sizeof(*work),apsort);
    for(k=0;k<partition;k++) ret[i][k+j]=(int)(work[k]-mags[i]);
      }
    }
    return(ret);
  }
  return(NULL);
}

void _vp_noise_normalize_sort(vorbis_look_psy *p,
                  float *magnitudes,int *sortedindex){
  int i,j,n=p->n;
  vorbis_info_psy *vi=p->vi;
  int partition=vi->normal_partition;
  float **work=static_cast<float**>(alloca(sizeof(*work)*partition));
  int start=vi->normal_start;

  for(j=start;j<n;j+=partition){
    if(j+partition>n)partition=n-j;
    for(i=0;i<partition;i++)work[i]=magnitudes+i+j;
    qsort(work,partition,sizeof(*work),apsort);
    for(i=0;i<partition;i++){
      sortedindex[i+j-start]=(int)(work[i]-magnitudes);
    }
  }
}

void _vp_noise_normalize(vorbis_look_psy *p,
             float *in,float *out,int *sortedindex){
  int i,j=0,n=p->n; // ,flag=0
  vorbis_info_psy *vi=p->vi;
  int partition=vi->normal_partition;
  int start=vi->normal_start;

  if(start>n)start=n;

  if(vi->normal_channel_p){
    for(;j<start;j++)
      out[j]=(float)rint(in[j]);

    for(;j+partition<=n;j+=partition){
      float acc=0.;
      int k;

      for(i=j;i<j+partition;i++)
    acc+=in[i]*in[i];

      for(i=0;i<partition;i++){
    k=sortedindex[i+j-start];

    if(in[k]*in[k]>=.25f){
      out[k]=(float)rint(in[k]);
      acc-=in[k]*in[k];
      //flag=1;
    }else{
      if(acc<vi->normal_thresh)break;
      out[k]=unitnorm(in[k]);
      acc-=1.;
    }
      }

      for(;i<partition;i++){
    k=sortedindex[i+j-start];
    out[k]=0.;
      }
    }
  }

  for(;j<n;j++)
    out[j]=(float)rint(in[j]);

}

void _vp_couple(int blobno,
        vorbis_info_psy_global *g,
        vorbis_look_psy *p,
        vorbis_info_mapping0 *vi,
        float **res,
        float **mag_memo,
        int   **mag_sort,
        int   **ifloor,
        int   *nonzero,
        int  sliding_lowpass){

  int i,j,k,n=p->n;

  /* perform any requested channel coupling */
  /* point stereo can only be used in a first stage (in this encoder)
     because of the dependency on floor lookups */
  for(i=0;i<vi->coupling_steps;i++){

    /* once we're doing multistage coupling in which a channel goes
       through more than one coupling step, the floor vector
       magnitudes will also have to be recalculated an propogated
       along with PCM.  Right now, we're not (that will wait until 5.1
       most likely), so the code isn't here yet. The memory management
       here is all assuming single depth couplings anyway. */

    /* make sure coupling a zero and a nonzero channel results in two
       nonzero channels. */
    if(nonzero[vi->coupling_mag[i]] ||
       nonzero[vi->coupling_ang[i]]){


      float *rM=res[vi->coupling_mag[i]];
      float *rA=res[vi->coupling_ang[i]];
      float *qM=rM+n;
      float *qA=rA+n;
      int *floorM=ifloor[vi->coupling_mag[i]];
      int *floorA=ifloor[vi->coupling_ang[i]];
      float prepoint=(float)(stereo_threshholds[g->coupling_prepointamp[blobno]]);
      float postpoint=(float)(stereo_threshholds[g->coupling_postpointamp[blobno]]);
      int partition=(p->vi->normal_point_p?p->vi->normal_partition:p->n);
      int limit=g->coupling_pointlimit[p->vi->blockflag][blobno];
      int pointlimit=limit;

      nonzero[vi->coupling_mag[i]]=1;
      nonzero[vi->coupling_ang[i]]=1;

      for(j=0;j<p->n;j+=partition){
    float acc=0.f;

    for(k=0;k<partition;k++){
      int l=k+j;

      if(l<sliding_lowpass){
        if((l>=limit && fabs(rM[l])<postpoint && fabs(rA[l])<postpoint) ||
           (fabs(rM[l])<prepoint && fabs(rA[l])<prepoint)){


          precomputed_couple_point(mag_memo[i][l],
                       floorM[l],floorA[l],
                       qM+l,qA+l);

          if(rint(qM[l])==0.f)acc+=qM[l]*qM[l];
        }else{
          couple_lossless(rM[l],rA[l],qM+l,qA+l);
        }
      }else{
        qM[l]=0.;
        qA[l]=0.;
      }
    }

    if(p->vi->normal_point_p){
      for(k=0;k<partition && acc>=p->vi->normal_thresh;k++){
        int l=mag_sort[i][j+k];
        if(l<sliding_lowpass && l>=pointlimit && rint(qM[l])==0.f){
          qM[l]=unitnorm(qM[l]);
          acc-=1.f;
        }
      }
    }
      }
    }
  }
}










// ==========================================================================
// ==========================================================================
// mapping0.c
// ==========================================================================
// ==========================================================================

/* simplistic, wasteful way of doing this (unique lookup for each
   mode/submapping); there should be a central repository for
   identical lookups.  That will require minor work, so I'm putting it
   off as low priority.

   Why a lookup for each backend in a given mode?  Because the
   blocksize is set by the mode, and low backend lookups may require
   parameters from other areas of the mode/mapping */

void mapping0_free_info(vorbis_info_mapping *i){
  vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)i;
  if(info){
    memset(info,0,sizeof(*info));
    _ogg_free(info);
  }
}


void mapping0_pack(vorbis_info *vi,vorbis_info_mapping *vm,
              oggpack_buffer *opb){
  int i;
  vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)vm;

  /* another 'we meant to do it this way' hack...  up to beta 4, we
     packed 4 binary zeros here to signify one submapping in use.  We
     now redefine that to mean four bitflags that indicate use of
     deeper features; bit0:submappings, bit1:coupling,
     bit2,3:reserved. This is backward compatable with all actual uses
     of the beta code. */

  if(info->submaps>1){
    oggpack_write(opb,1,1);
    oggpack_write(opb,info->submaps-1,4);
  }else
    oggpack_write(opb,0,1);

  if(info->coupling_steps>0){
    oggpack_write(opb,1,1);
    oggpack_write(opb,info->coupling_steps-1,8);

    for(i=0;i<info->coupling_steps;i++){
      oggpack_write(opb,info->coupling_mag[i],ilog2(vi->channels));
      oggpack_write(opb,info->coupling_ang[i],ilog2(vi->channels));
    }
  }else
    oggpack_write(opb,0,1);

  oggpack_write(opb,0,2); /* 2,3:reserved */

  /* we don't write the channel submappings if we only have one... */
  if(info->submaps>1){
    for(i=0;i<vi->channels;i++)
      oggpack_write(opb,info->chmuxlist[i],4);
  }
  for(i=0;i<info->submaps;i++){
    oggpack_write(opb,0,8); /* time submap unused */
    oggpack_write(opb,info->floorsubmap[i],8);
    oggpack_write(opb,info->residuesubmap[i],8);
  }
}

/* also responsible for range checking */
vorbis_info_mapping *mapping0_unpack(vorbis_info *vi,oggpack_buffer *opb){
  int i;
  vorbis_info_mapping0 *info=static_cast<vorbis_info_mapping0*>(_ogg_calloc(1,sizeof(*info)));
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  memset(info,0,sizeof(*info));

  if(oggpack_read(opb,1))
    info->submaps=oggpack_read(opb,4)+1;
  else
    info->submaps=1;

  if(oggpack_read(opb,1)){
    info->coupling_steps=oggpack_read(opb,8)+1;

    for(i=0;i<info->coupling_steps;i++){
      int testM=info->coupling_mag[i]=oggpack_read(opb,ilog2(vi->channels));
      int testA=info->coupling_ang[i]=oggpack_read(opb,ilog2(vi->channels));

      if(testM<0 ||
     testA<0 ||
     testM==testA ||
     testM>=vi->channels ||
     testA>=vi->channels) goto err_out;
    }

  }

  if(oggpack_read(opb,2)>0)goto err_out; /* 2,3:reserved */

  if(info->submaps>1){
    for(i=0;i<vi->channels;i++){
      info->chmuxlist[i]=oggpack_read(opb,4);
      if(info->chmuxlist[i]>=info->submaps)goto err_out;
    }
  }
  for(i=0;i<info->submaps;i++){
    oggpack_read(opb,8); /* time submap unused */
    info->floorsubmap[i]=oggpack_read(opb,8);
    if(info->floorsubmap[i]>=ci->floors)goto err_out;
    info->residuesubmap[i]=oggpack_read(opb,8);
    if(info->residuesubmap[i]>=ci->residues)goto err_out;
  }

  return info;

 err_out:
  mapping0_free_info(info);
  return(NULL);
}


#if 0
long seq=0;
ogg_int64_t total=0;
float FLOOR1_fromdB_LOOKUP[256]={
  1.0649863e-07F, 1.1341951e-07F, 1.2079015e-07F, 1.2863978e-07F,
  1.3699951e-07F, 1.4590251e-07F, 1.5538408e-07F, 1.6548181e-07F,
  1.7623575e-07F, 1.8768855e-07F, 1.9988561e-07F, 2.128753e-07F,
  2.2670913e-07F, 2.4144197e-07F, 2.5713223e-07F, 2.7384213e-07F,
  2.9163793e-07F, 3.1059021e-07F, 3.3077411e-07F, 3.5226968e-07F,
  3.7516214e-07F, 3.9954229e-07F, 4.2550680e-07F, 4.5315863e-07F,
  4.8260743e-07F, 5.1396998e-07F, 5.4737065e-07F, 5.8294187e-07F,
  6.2082472e-07F, 6.6116941e-07F, 7.0413592e-07F, 7.4989464e-07F,
  7.9862701e-07F, 8.5052630e-07F, 9.0579828e-07F, 9.6466216e-07F,
  1.0273513e-06F, 1.0941144e-06F, 1.1652161e-06F, 1.2409384e-06F,
  1.3215816e-06F, 1.4074654e-06F, 1.4989305e-06F, 1.5963394e-06F,
  1.7000785e-06F, 1.8105592e-06F, 1.9282195e-06F, 2.0535261e-06F,
  2.1869758e-06F, 2.3290978e-06F, 2.4804557e-06F, 2.6416497e-06F,
  2.8133190e-06F, 2.9961443e-06F, 3.1908506e-06F, 3.3982101e-06F,
  3.6190449e-06F, 3.8542308e-06F, 4.1047004e-06F, 4.3714470e-06F,
  4.6555282e-06F, 4.9580707e-06F, 5.2802740e-06F, 5.6234160e-06F,
  5.9888572e-06F, 6.3780469e-06F, 6.7925283e-06F, 7.2339451e-06F,
  7.7040476e-06F, 8.2047000e-06F, 8.7378876e-06F, 9.3057248e-06F,
  9.9104632e-06F, 1.0554501e-05F, 1.1240392e-05F, 1.1970856e-05F,
  1.2748789e-05F, 1.3577278e-05F, 1.4459606e-05F, 1.5399272e-05F,
  1.6400004e-05F, 1.7465768e-05F, 1.8600792e-05F, 1.9809576e-05F,
  2.1096914e-05F, 2.2467911e-05F, 2.3928002e-05F, 2.5482978e-05F,
  2.7139006e-05F, 2.8902651e-05F, 3.0780908e-05F, 3.2781225e-05F,
  3.4911534e-05F, 3.7180282e-05F, 3.9596466e-05F, 4.2169667e-05F,
  4.4910090e-05F, 4.7828601e-05F, 5.0936773e-05F, 5.4246931e-05F,
  5.7772202e-05F, 6.1526565e-05F, 6.5524908e-05F, 6.9783085e-05F,
  7.4317983e-05F, 7.9147585e-05F, 8.4291040e-05F, 8.9768747e-05F,
  9.5602426e-05F, 0.00010181521F, 0.00010843174F, 0.00011547824F,
  0.00012298267F, 0.00013097477F, 0.00013948625F, 0.00014855085F,
  0.00015820453F, 0.00016848555F, 0.00017943469F, 0.00019109536F,
  0.00020351382F, 0.00021673929F, 0.00023082423F, 0.00024582449F,
  0.00026179955F, 0.00027881276F, 0.00029693158F, 0.00031622787F,
  0.00033677814F, 0.00035866388F, 0.00038197188F, 0.00040679456F,
  0.00043323036F, 0.00046138411F, 0.00049136745F, 0.00052329927F,
  0.00055730621F, 0.00059352311F, 0.00063209358F, 0.00067317058F,
  0.00071691700F, 0.00076350630F, 0.00081312324F, 0.00086596457F,
  0.00092223983F, 0.00098217216F, 0.0010459992F, 0.0011139742F,
  0.0011863665F, 0.0012634633F, 0.0013455702F, 0.0014330129F,
  0.0015261382F, 0.0016253153F, 0.0017309374F, 0.0018434235F,
  0.0019632195F, 0.0020908006F, 0.0022266726F, 0.0023713743F,
  0.0025254795F, 0.0026895994F, 0.0028643847F, 0.0030505286F,
  0.0032487691F, 0.0034598925F, 0.0036847358F, 0.0039241906F,
  0.0041792066F, 0.0044507950F, 0.0047400328F, 0.0050480668F,
  0.0053761186F, 0.0057254891F, 0.0060975636F, 0.0064938176F,
  0.0069158225F, 0.0073652516F, 0.0078438871F, 0.0083536271F,
  0.0088964928F, 0.009474637F, 0.010090352F, 0.010746080F,
  0.011444421F, 0.012188144F, 0.012980198F, 0.013823725F,
  0.014722068F, 0.015678791F, 0.016697687F, 0.017782797F,
  0.018938423F, 0.020169149F, 0.021479854F, 0.022875735F,
  0.024362330F, 0.025945531F, 0.027631618F, 0.029427276F,
  0.031339626F, 0.033376252F, 0.035545228F, 0.037855157F,
  0.040315199F, 0.042935108F, 0.045725273F, 0.048696758F,
  0.051861348F, 0.055231591F, 0.058820850F, 0.062643361F,
  0.066714279F, 0.071049749F, 0.075666962F, 0.080584227F,
  0.085821044F, 0.091398179F, 0.097337747F, 0.10366330F,
  0.11039993F, 0.11757434F, 0.12521498F, 0.13335215F,
  0.14201813F, 0.15124727F, 0.16107617F, 0.17154380F,
  0.18269168F, 0.19456402F, 0.20720788F, 0.22067342F,
  0.23501402F, 0.25028656F, 0.26655159F, 0.28387361F,
  0.30232132F, 0.32196786F, 0.34289114F, 0.36517414F,
  0.38890521F, 0.41417847F, 0.44109412F, 0.46975890F,
  0.50028648F, 0.53279791F, 0.56742212F, 0.60429640F,
  0.64356699F, 0.68538959F, 0.72993007F, 0.77736504F,
  0.82788260F, 0.88168307F, 0.9389798F, 1.F,
};

#endif



int mapping0_forward(vorbis_block *vb){
  vorbis_dsp_state      *vd=vb->vd;
  vorbis_info           *vi=vd->vi;
  codec_setup_info      *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  private_state         *b=static_cast<private_state*>(vb->vd->backend_state);
  vorbis_block_internal *vbi=(vorbis_block_internal *)vb->internal;
  int                    n=vb->pcmend;
  int i,j,k;

  int    *nonzero    = static_cast<int*>(alloca(sizeof(*nonzero)*vi->channels));
  float  **gmdct     = static_cast<float**>(_vorbis_block_alloc(vb,vi->channels*sizeof(*gmdct)));
  int    **ilogmaskch= static_cast<int**>(_vorbis_block_alloc(vb,vi->channels*sizeof(*ilogmaskch)));
  int ***floor_posts = static_cast<int***>(_vorbis_block_alloc(vb,vi->channels*sizeof(*floor_posts)));

  float global_ampmax=vbi->ampmax;
  float *local_ampmax=static_cast<float*>(alloca(sizeof(*local_ampmax)*vi->channels));
  int blocktype=vbi->blocktype;

  int modenumber=vb->W;
  vorbis_info_mapping0 *info=static_cast<vorbis_info_mapping0*>(ci->map_param[modenumber]);
  vorbis_look_psy *psy_look=
    b->psy+blocktype+(vb->W?2:0);

  vb->mode=modenumber;

  for(i=0;i<vi->channels;i++){
    float scale=4.f/n;
    float scale_dB;

    float *pcm     =vb->pcm[i];
    float *logfft  =pcm;

    gmdct[i]=static_cast<float*>(_vorbis_block_alloc(vb,n/2*sizeof(**gmdct)));

    scale_dB=todB(&scale);

#if 0
    if(vi->channels==2)
      if(i==0)
    _analysis_output("pcmL",seq,pcm,n,0,0,total-n/2);
      else
    _analysis_output("pcmR",seq,pcm,n,0,0,total-n/2);
#endif

    /* window the PCM data */
    _vorbis_apply_window(pcm,b->window,ci->blocksizes,vb->lW,vb->W,vb->nW);

#if 0
    if(vi->channels==2)
      if(i==0)
    _analysis_output("windowedL",seq,pcm,n,0,0,total-n/2);
      else
    _analysis_output("windowedR",seq,pcm,n,0,0,total-n/2);
#endif

    /* transform the PCM data */
    /* only MDCT right now.... */
    mdct_forward(static_cast<mdct_lookup*>(b->transform[vb->W][0]),pcm,gmdct[i]);

    /* FFT yields more accurate tonal estimation (not phase sensitive) */
    drft_forward(&b->fft_look[vb->W],pcm);
    logfft[0]=scale_dB+todB(pcm);
    local_ampmax[i]=logfft[0];
    for(j=1;j<n-1;j+=2){
      float temp=pcm[j]*pcm[j]+pcm[j+1]*pcm[j+1];
      temp=logfft[(j+1)>>1]=scale_dB+.5f*todB(&temp);
      if(temp>local_ampmax[i])local_ampmax[i]=temp;
    }

    if(local_ampmax[i]>0.f)local_ampmax[i]=0.f;
    if(local_ampmax[i]>global_ampmax)global_ampmax=local_ampmax[i];

#if 0
    if(vi->channels==2)
      if(i==0)
    _analysis_output("fftL",seq,logfft,n/2,1,0,0);
      else
    _analysis_output("fftR",seq,logfft,n/2,1,0,0);
#endif

  }

  {
    float   *noise        = static_cast<float*>(_vorbis_block_alloc(vb,n/2*sizeof(*noise)));
    float   *tone         = static_cast<float*>(_vorbis_block_alloc(vb,n/2*sizeof(*tone)));

    for(i=0;i<vi->channels;i++){
      /* the encoder setup assumes that all the modes used by any
     specific bitrate tweaking use the same floor */

      int submap=info->chmuxlist[i];

      /* the following makes things clearer to *me* anyway */
      float *mdct    =gmdct[i];
      float *logfft  =vb->pcm[i];

      float *logmdct =logfft+n/2;
      float *logmask =logfft;

      vb->mode=modenumber;

      floor_posts[i]=static_cast<int**>(_vorbis_block_alloc(vb,PACKETBLOBS*sizeof(**floor_posts)));
      memset(floor_posts[i],0,sizeof(**floor_posts)*PACKETBLOBS);

      for(j=0;j<n/2;j++)
    logmdct[j]=todB(mdct+j);

#if 0
      if(vi->channels==2){
    if(i==0)
      _analysis_output("mdctL",seq,logmdct,n/2,1,0,0);
    else
      _analysis_output("mdctR",seq,logmdct,n/2,1,0,0);
      }else{
    _analysis_output("mdct",seq,logmdct,n/2,1,0,0);
      }
#endif

      /* first step; noise masking.  Not only does 'noise masking'
         give us curves from which we can decide how much resolution
         to give noise parts of the spectrum, it also implicitly hands
         us a tonality estimate (the larger the value in the
         'noise_depth' vector, the more tonal that area is) */

      _vp_noisemask(psy_look,
            logmdct,
            noise); /* noise does not have by-frequency offset
                               bias applied yet */
#if 0
      if(vi->channels==2){
    if(i==0)
      _analysis_output("noiseL",seq,noise,n/2,1,0,0);
    else
      _analysis_output("noiseR",seq,noise,n/2,1,0,0);
      }
#endif

      /* second step: 'all the other crap'; all the stuff that isn't
         computed/fit for bitrate management goes in the second psy
         vector.  This includes tone masking, peak limiting and ATH */

      _vp_tonemask(psy_look,
           logfft,
           tone,
           global_ampmax,
           local_ampmax[i]);

#if 0
      if(vi->channels==2){
    if(i==0)
      _analysis_output("toneL",seq,tone,n/2,1,0,0);
    else
      _analysis_output("toneR",seq,tone,n/2,1,0,0);
      }
#endif

      /* third step; we offset the noise vectors, overlay tone
     masking.  We then do a floor1-specific line fit.  If we're
     performing bitrate management, the line fit is performed
     multiple times for up/down tweakage on demand. */

      _vp_offset_and_mix(psy_look,
             noise,
             tone,
             1,
             logmask);

#if 0
      if(vi->channels==2){
    if(i==0)
      _analysis_output("mask1L",seq,logmask,n/2,1,0,0);
    else
      _analysis_output("mask1R",seq,logmask,n/2,1,0,0);
      }
#endif

      /* this algorithm is hardwired to floor 1 for now; abort out if
         we're *not* floor1.  This won't happen unless someone has
         broken the encode setup lib.  Guard it anyway. */
      if(ci->floor_type[info->floorsubmap[submap]]!=1)return(-1);


      floor_posts[i][PACKETBLOBS/2]=
    floor1_fit(vb,static_cast<vorbis_look_floor1*>(b->flr[info->floorsubmap[submap]]),
           logmdct,
           logmask);

      /* are we managing bitrate?  If so, perform two more fits for
         later rate tweaking (fits represent hi/lo) */
      if(vorbis_bitrate_managed(vb) && floor_posts[i][PACKETBLOBS/2]){
    /* higher rate by way of lower noise curve */

    _vp_offset_and_mix(psy_look,
               noise,
               tone,
               2,
               logmask);

#if 0
    if(vi->channels==2){
      if(i==0)
        _analysis_output("mask2L",seq,logmask,n/2,1,0,0);
      else
        _analysis_output("mask2R",seq,logmask,n/2,1,0,0);
    }
#endif

    floor_posts[i][PACKETBLOBS-1]=
      floor1_fit(vb,static_cast<vorbis_look_floor1*>(b->flr[info->floorsubmap[submap]]),
             logmdct,
             logmask);

    /* lower rate by way of higher noise curve */
    _vp_offset_and_mix(psy_look,
               noise,
               tone,
               0,
               logmask);

#if 0
    if(vi->channels==2)
      if(i==0)
        _analysis_output("mask0L",seq,logmask,n/2,1,0,0);
      else
        _analysis_output("mask0R",seq,logmask,n/2,1,0,0);
#endif

    floor_posts[i][0]=
      floor1_fit(vb,static_cast<vorbis_look_floor1*>(b->flr[info->floorsubmap[submap]]),
             logmdct,
             logmask);

    /* we also interpolate a range of intermediate curves for
           intermediate rates */
    for(k=1;k<PACKETBLOBS/2;k++)
      floor_posts[i][k]=
        floor1_interpolate_fit(vb,static_cast<vorbis_look_floor1*>(b->flr[info->floorsubmap[submap]]),
                   floor_posts[i][0],
                   floor_posts[i][PACKETBLOBS/2],
                   k*65536/(PACKETBLOBS/2));
    for(k=PACKETBLOBS/2+1;k<PACKETBLOBS-1;k++)
      floor_posts[i][k]=
        floor1_interpolate_fit(vb,static_cast<vorbis_look_floor1*>(b->flr[info->floorsubmap[submap]]),
                   floor_posts[i][PACKETBLOBS/2],
                   floor_posts[i][PACKETBLOBS-1],
                   (k-PACKETBLOBS/2)*65536/(PACKETBLOBS/2));
      }
    }
  }
  vbi->ampmax=global_ampmax;

  /*
    the next phases are performed once for vbr-only and PACKETBLOB
    times for bitrate managed modes.

    1) encode actual mode being used
    2) encode the floor for each channel, compute coded mask curve/res
    3) normalize and couple.
    4) encode residue
    5) save packet bytes to the packetblob vector

  */

  /* iterate over the many masking curve fits we've created */

  {
    float **res_bundle=static_cast<float**>(alloca(sizeof(*res_bundle)*vi->channels));
    float **couple_bundle=static_cast<float**>(alloca(sizeof(*couple_bundle)*vi->channels));
    int *zerobundle=static_cast<int*>(alloca(sizeof(*zerobundle)*vi->channels));
    int **sortindex=static_cast<int**>(alloca(sizeof(*sortindex)*vi->channels));
    float **mag_memo=0;
    int **mag_sort=0;

    if(info->coupling_steps){
      mag_memo=_vp_quantize_couple_memo(vb,
                    &ci->psy_g_param,
                    psy_look,
                    info,
                    gmdct);

      mag_sort=_vp_quantize_couple_sort(vb,
                    psy_look,
                    info,
                    mag_memo);
    }

    memset(sortindex,0,sizeof(*sortindex)*vi->channels);
    if(psy_look->vi->normal_channel_p){
      for(i=0;i<vi->channels;i++){
    float *mdct    =gmdct[i];
    sortindex[i]=static_cast<int*>(alloca(sizeof(**sortindex)*n/2));
    _vp_noise_normalize_sort(psy_look,mdct,sortindex[i]);
      }
    }

    for(k=(vorbis_bitrate_managed(vb)?0:PACKETBLOBS/2);
    k<=(vorbis_bitrate_managed(vb)?PACKETBLOBS-1:PACKETBLOBS/2);
    k++){

      /* start out our new packet blob with packet type and mode */
      /* Encode the packet type */
      oggpack_write(&vb->opb,0,1);
      /* Encode the modenumber */
      /* Encode frame mode, pre,post windowsize, then dispatch */
      oggpack_write(&vb->opb,modenumber,b->modebits);
      if(vb->W){
    oggpack_write(&vb->opb,vb->lW,1);
    oggpack_write(&vb->opb,vb->nW,1);
      }

      /* encode floor, compute masking curve, sep out residue */
      for(i=0;i<vi->channels;i++){
    int submap=info->chmuxlist[i];
    float *mdct    =gmdct[i];
    float *res     =vb->pcm[i];
    int   *ilogmask=static_cast<int*>(_vorbis_block_alloc(vb,n/2*sizeof(**gmdct)));
        ilogmaskch[i]=ilogmask;

    nonzero[i]=floor1_encode(vb,static_cast<vorbis_look_floor1*>(b->flr[info->floorsubmap[submap]]),
                 floor_posts[i][k],
                 ilogmask);
#if 0
    {
      char buf[80];
      sprintf(buf,"maskI%c%d",i?'R':'L',k);
      float work[n/2];
      for(j=0;j<n/2;j++)
        work[j]=FLOOR1_fromdB_LOOKUP[ilogmask[j]];
      _analysis_output(buf,seq,work,n/2,1,1,0);
    }
#endif
    _vp_remove_floor(psy_look,
             mdct,
             ilogmask,
             res,
             ci->psy_g_param.sliding_lowpass[vb->W][k]);

    _vp_noise_normalize(psy_look,res,res+n/2,sortindex[i]);


#if 0
    {
      char buf[80];
      float work[n/2];
      for(j=0;j<n/2;j++)
        work[j]=FLOOR1_fromdB_LOOKUP[ilogmask[j]]*(res+n/2)[j];
      sprintf(buf,"resI%c%d",i?'R':'L',k);
      _analysis_output(buf,seq,work,n/2,1,1,0);

    }
#endif
      }

      /* our iteration is now based on masking curve, not prequant and
     coupling.  Only one prequant/coupling step */

      /* quantize/couple */
      /* incomplete implementation that assumes the tree is all depth
         one, or no tree at all */
      if(info->coupling_steps){
    _vp_couple(k,
           &ci->psy_g_param,
           psy_look,
           info,
           vb->pcm,
           mag_memo,
           mag_sort,
           ilogmaskch,
           nonzero,
           ci->psy_g_param.sliding_lowpass[vb->W][k]);
      }

      /* classify and encode by submap */
      for(i=0;i<info->submaps;i++){
    int ch_in_bundle=0;
    long **classifications;
    int resnum=info->residuesubmap[i];

    for(j=0;j<vi->channels;j++){
      if(info->chmuxlist[j]==i){
        zerobundle[ch_in_bundle]=0;
        if(nonzero[j])zerobundle[ch_in_bundle]=1;
        res_bundle[ch_in_bundle]=vb->pcm[j];
        couple_bundle[ch_in_bundle++]=vb->pcm[j]+n/2;
      }
    }

    classifications=_residue_P[ci->residue_type[resnum]]->
      class_lu(vb,b->residue[resnum],couple_bundle,zerobundle,ch_in_bundle);

    _residue_P[ci->residue_type[resnum]]->
      forward(vb,b->residue[resnum],
          couple_bundle,NULL,zerobundle,ch_in_bundle,classifications);
      }

      /* ok, done encoding.  Mark this protopacket and prepare next. */
      oggpack_writealign(&vb->opb);
      vbi->packetblob_markers[k]=oggpack_bytes(&vb->opb);

    }

  }

#if 0
  seq++;
  total+=ci->blocksizes[vb->W]/4+ci->blocksizes[vb->nW]/4;
#endif
  return(0);
}

int mapping0_inverse(vorbis_block *vb,vorbis_info_mapping *l){
  vorbis_dsp_state     *vd=vb->vd;
  vorbis_info          *vi=vd->vi;
  codec_setup_info     *ci=static_cast<codec_setup_info*>(vi->codec_setup);
  private_state        *b=static_cast<private_state*>(vd->backend_state);
  vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)l;

  int                   i,j;
  long                  n=vb->pcmend=ci->blocksizes[vb->W];

  float **pcmbundle=static_cast<float**>(alloca(sizeof(*pcmbundle)*vi->channels));
  int    *zerobundle=static_cast<int*>(alloca(sizeof(*zerobundle)*vi->channels));

  int   *nonzero  =static_cast<int*>(alloca(sizeof(*nonzero)*vi->channels));
  void **floormemo=static_cast<void**>(alloca(sizeof(*floormemo)*vi->channels));

  /* recover the spectral envelope; store it in the PCM vector for now */
  for(i=0;i<vi->channels;i++){
    int submap=info->chmuxlist[i];
    floormemo[i]=_floor_P[ci->floor_type[info->floorsubmap[submap]]]->
      inverse1(vb,b->flr[info->floorsubmap[submap]]);
    if(floormemo[i])
      nonzero[i]=1;
    else
      nonzero[i]=0;
    memset(vb->pcm[i],0,sizeof(*vb->pcm[i])*n/2);
  }

  /* channel coupling can 'dirty' the nonzero listing */
  for(i=0;i<info->coupling_steps;i++){
    if(nonzero[info->coupling_mag[i]] ||
       nonzero[info->coupling_ang[i]]){
      nonzero[info->coupling_mag[i]]=1;
      nonzero[info->coupling_ang[i]]=1;
    }
  }

  /* recover the residue into our working vectors */
  for(i=0;i<info->submaps;i++){
    int ch_in_bundle=0;
    for(j=0;j<vi->channels;j++){
      if(info->chmuxlist[j]==i){
    if(nonzero[j])
      zerobundle[ch_in_bundle]=1;
    else
      zerobundle[ch_in_bundle]=0;
    pcmbundle[ch_in_bundle++]=vb->pcm[j];
      }
    }

    _residue_P[ci->residue_type[info->residuesubmap[i]]]->
      inverse(vb,b->residue[info->residuesubmap[i]],
          pcmbundle,zerobundle,ch_in_bundle);
  }

  /* channel coupling */
  for(i=info->coupling_steps-1;i>=0;i--){
    float *pcmM=vb->pcm[info->coupling_mag[i]];
    float *pcmA=vb->pcm[info->coupling_ang[i]];

    for(j=0;j<n/2;j++){
      float mag=pcmM[j];
      float ang=pcmA[j];

      if(mag>0)
    if(ang>0){
      pcmM[j]=mag;
      pcmA[j]=mag-ang;
    }else{
      pcmA[j]=mag;
      pcmM[j]=mag+ang;
    }
      else
    if(ang>0){
      pcmM[j]=mag;
      pcmA[j]=mag+ang;
    }else{
      pcmA[j]=mag;
      pcmM[j]=mag-ang;
    }
    }
  }

  /* compute and apply spectral envelope */
  for(i=0;i<vi->channels;i++){
    float *pcm=vb->pcm[i];
    int submap=info->chmuxlist[i];
    _floor_P[ci->floor_type[info->floorsubmap[submap]]]->
      inverse2(vb,b->flr[info->floorsubmap[submap]],
           floormemo[i],pcm);
  }

  /* transform the PCM data; takes PCM vector, vb; modifies PCM vector */
  /* only MDCT right now.... */
  for(i=0;i<vi->channels;i++){
    float *pcm=vb->pcm[i];
    mdct_backward(static_cast<mdct_lookup*>(b->transform[vb->W][0]),pcm,pcm);
  }

  /* window the data */
  for(i=0;i<vi->channels;i++){
    float *pcm=vb->pcm[i];
    if(nonzero[i])
      _vorbis_apply_window(pcm,b->window,ci->blocksizes,vb->lW,vb->W,vb->nW);
    else
      for(j=0;j<n;j++)
    pcm[j]=0.f;

  }

  /* all done! */
  return(0);
}

/* export hooks */
vorbis_func_mapping mapping0_exportbundle={
  mapping0_pack,
  mapping0_unpack,
  mapping0_free_info,
  mapping0_forward,
  mapping0_inverse
};









// ==========================================================================
// ==========================================================================
// registry.c
// ==========================================================================
// ==========================================================================

/* seems like major overkill now; the backend numbers will grow into
   the infrastructure soon enough */

vorbis_func_floor     *_floor_P[]={
  &floor0_exportbundle,
  &floor1_exportbundle,
};

vorbis_func_residue   *_residue_P[]={
  &residue0_exportbundle,
  &residue1_exportbundle,
  &residue2_exportbundle,
};

vorbis_func_mapping   *_mapping_P[]={
  &mapping0_exportbundle,
};

#pragma warning(default: 4995)
