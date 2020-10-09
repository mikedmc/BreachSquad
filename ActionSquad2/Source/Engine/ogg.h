#ifndef __ogglib_h
#define __ogglib_h

// OGG functions -- for decompressing ogg files.
// This file is a repackaged form of extracts from Ogg Vorbis code
// available at www.xiph.org. The original copyright notice may be found
// in ogg.cpp. The repackaging was done by Lucian Wischik to simplify
// its use in Windows/C++.


#include <stdio.h>

typedef struct vorbis_info{
  int version;
  int channels;
  long rate;
  long bitrate_upper;   // all these bitrates are just *hints*.
  long bitrate_nominal;
  long bitrate_lower;
  long bitrate_window;
  void *codec_setup;
} vorbis_info;

typedef struct vorbis_comment{
  char **user_comments;
  int   *comment_lengths;
  int    comments;
  char  *vendor;
} vorbis_comment;

typedef struct {
  size_t (*read_func)  (void *ptr, size_t size, size_t nmemb, void *datasource);
  int    (*seek_func)  (void *datasource, __int64 offset, int whence);
  int    (*close_func) (void *datasource);
  long   (*tell_func)  (void *datasource);
} ov_callbacks;


typedef struct OggVorbis_File {
  void        *datasource;
  int          seekable;
  __int64      offset;
  __int64      end;
  char         reserved1[28];
  int          links;
  __int64     *reserved2;
  __int64     *reserved3;
  long        *reserved4;
  __int64     *reserved5;
  void        *reserved6;
  void        *reserved7;
  __int64      reserved8;
  int          reserved9;
  long         reserved10;
  int          reserved11;
  double       reserved12;
  double       reserved13;
  char         reserved14[360];
  char         reserved15[112];
  char         reserved16[112];
  ov_callbacks callbacks;
} OggVorbis_File;



int ov_clear(OggVorbis_File *vf);
int ov_open(FILE *f,OggVorbis_File *vf,char *initial,long ibytes);
int ov_open_callbacks(void *datasource, OggVorbis_File *vf, char *initial, long ibytes, ov_callbacks callbacks);
int ov_test(FILE *f,OggVorbis_File *vf,char *initial,long ibytes);
int ov_test_callbacks(void *datasource, OggVorbis_File *vf, char *initial, long ibytes, ov_callbacks callbacks);
int ov_test_open(OggVorbis_File *vf);
vorbis_info *ov_info(OggVorbis_File *vf,int link);
vorbis_comment *ov_comment(OggVorbis_File *vf,int link);

long ov_bitrate(OggVorbis_File *vf,int i);
long ov_bitrate_instant(OggVorbis_File *vf);
long ov_streams(OggVorbis_File *vf);
long ov_seekable(OggVorbis_File *vf);
long ov_serialnumber(OggVorbis_File *vf,int i);
__int64 ov_raw_total(OggVorbis_File *vf,int i);
__int64 ov_pcm_total(OggVorbis_File *vf,int i);
double ov_time_total(OggVorbis_File *vf,int i);
int ov_raw_seek(OggVorbis_File *vf,__int64 pos);
int ov_pcm_seek(OggVorbis_File *vf,__int64 pos);
int ov_pcm_seek_page(OggVorbis_File *vf,__int64 pos);
int ov_time_seek(OggVorbis_File *vf,double pos);
int ov_time_seek_page(OggVorbis_File *vf,double pos);
__int64 ov_raw_tell(OggVorbis_File *vf);
__int64 ov_pcm_tell(OggVorbis_File *vf);
double ov_time_tell(OggVorbis_File *vf);

long ov_read_float(OggVorbis_File *vf,float ***pcm_channels,int samples,  int *bitstream);
long ov_read(OggVorbis_File *vf,char *buffer,int length, int bigendianp,int word,int sgned,int *bitstream);




#endif
