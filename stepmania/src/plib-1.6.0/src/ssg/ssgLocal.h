/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/


#define _SSG_PUBLIC  public

#include "ssg.h"

void _ssgStartOfFrameInit  () ;
void _ssgEndOfFrameCleanup () ;


extern void (*__ssgEnableTable[64])() ;
extern void (*__ssgDisableTable[64])() ;

extern sgMat4 _ssgOpenGLAxisSwapMatrix ;
extern int    _ssgIsHotTest ;
extern int    _ssgIsLosTest ;
extern int    _ssgFileVersionNumber ;

void _ssgForceLineState () ;

void _ssgDrawDList () ;
void _ssgPushMatrix ( sgMat4 m ) ;
void _ssgPopMatrix  () ;
void _ssgLoadMatrix ( sgMat4 m ) ;
void _ssgLoadTexMatrix ( sgMat4 m ) ;
void _ssgUnloadTexMatrix () ;
void _ssgDrawLeaf   ( ssgLeaf *l ) ;

void _ssgAddHit ( ssgLeaf *l, int trinum, sgMat4 mat, sgVec4 pl ) ;
void _ssgPushPath ( ssgEntity *l ) ;
void _ssgPopPath () ;

extern int stats_num_vertices    ;
extern int stats_num_leaves      ;
extern int stats_isect_triangles ;
extern int stats_cull_test       ;
extern int stats_isect_test      ;
extern int stats_bind_textures   ;

extern int stats_hot_triangles   ;
extern int stats_hot_test        ;
extern int stats_hot_no_trav     ;
extern int stats_hot_radius_reject ;
extern int stats_hot_triv_accept ;
extern int stats_hot_straddle    ;

extern int stats_los_triangles   ;
extern int stats_los_test        ;
extern int stats_los_no_trav     ;
extern int stats_los_radius_reject ;
extern int stats_los_triv_accept ;
extern int stats_los_straddle    ;

extern ssgState *( *_ssgGetAppState)( char *) ;

void _ssgReadFloat   ( FILE *fd,                float *var ) ;
void _ssgWriteFloat  ( FILE *fd, const          float  var ) ;
void _ssgReadUInt    ( FILE *fd,       unsigned int   *var ) ;
void _ssgWriteUInt   ( FILE *fd, const unsigned int    var ) ;
void _ssgReadInt     ( FILE *fd,                int   *var ) ;
void _ssgWriteInt    ( FILE *fd, const          int    var ) ;
void _ssgReadUShort  ( FILE *fd,       unsigned short *var ) ;
void _ssgWriteUShort ( FILE *fd, const unsigned short  var ) ;
void _ssgReadShort   ( FILE *fd,                short *var ) ;
void _ssgWriteShort  ( FILE *fd, const          short  var ) ;

void _ssgReadFloat   ( FILE *fd, const unsigned int n,                float *var ) ;
void _ssgWriteFloat  ( FILE *fd, const unsigned int n, const          float *var ) ;
void _ssgReadUInt    ( FILE *fd, const unsigned int n,       unsigned int   *var ) ;
void _ssgWriteUInt   ( FILE *fd, const unsigned int n, const unsigned int   *var ) ;
void _ssgReadInt     ( FILE *fd, const unsigned int n,                int   *var ) ;
void _ssgWriteInt    ( FILE *fd, const unsigned int n, const          int   *var ) ;
void _ssgReadUShort  ( FILE *fd, const unsigned int n,       unsigned short *var ) ;
void _ssgWriteUShort ( FILE *fd, const unsigned int n, const unsigned short *var ) ;
void _ssgReadShort   ( FILE *fd, const unsigned int n,                short *var ) ;
void _ssgWriteShort  ( FILE *fd, const unsigned int n, const          short *var ) ;
void _ssgReadBytes   ( FILE *fd, const unsigned int n,                 void *var ) ;
void _ssgWriteBytes  ( FILE *fd, const unsigned int n, const           void *var ) ;

void _ssgReadString  ( FILE *fd,       char **var ) ;
void _ssgWriteString ( FILE *fd, const char  *var ) ;

void _ssgReadVec2    ( FILE *fd, sgVec2 var ) ;
void _ssgWriteVec2   ( FILE *fd, const sgVec2 var ) ;
void _ssgReadVec3    ( FILE *fd, sgVec3 var ) ;
void _ssgWriteVec3   ( FILE *fd, const sgVec3 var ) ;
void _ssgReadVec4    ( FILE *fd, sgVec4 var ) ;
void _ssgWriteVec4   ( FILE *fd, const sgVec4 var ) ;

void _ssgReadMat4    ( FILE *fd, sgMat4 var ) ;
void _ssgWriteMat4   ( FILE *fd, const sgMat4 var ) ;

int _ssgReadError    ( void ) ;
int _ssgWriteError   ( void ) ;

float _ssgGetCurrentTweenState () ;
void  _ssgSetCurrentTweenState ( float tweenstate ) ;

/*
  Routines for storing arbitrary ssgBase derived objects within SSG files.
  Both functions return 1 on success, and 0 on failure.
  If an object is encountered that is not derived from type_mask, then
  the loading is aborted and 0 returned.
*/
int _ssgSaveObject ( FILE * , ssgBase * ) ;
int _ssgLoadObject ( FILE * , ssgBase ** , int type_mask = 0 ) ;
