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


typedef unsigned char ModSample [ 30 ] ;
typedef unsigned char ModNote   [  4 ] ;
struct SampleInfo ;
struct Note       ;

class MODfile
{
  unsigned char *buffer   ;
  unsigned char *p0       ;
  unsigned char *songName ;

  int ordNum ;
  int insNum ;
  int patNum ;
  int  chNum ;
  int rstOrd ;

  unsigned char *ord      ;
  ModSample     *smpInfop ;
  unsigned char *smp0p    ;
  ModNote       *pat      ;
  short         *note     ;
  unsigned char *fileEnd  ;
  unsigned char *repCounter;
  SampleInfo    *sip      ;

  int  firsttime    ;
  int  broken       ;

  int  play_nextOrd ;
  int  play_loopBeg ;
  int  play_loopCnt ;
  int  play_row0    ;
  int  play_row     ;
  int  play_ord0    ;
  int  play_ord     ;

  void makeNoteTable  ( void ) ;
  void tellChSettings ( void ) ;
  int  roundToNote    ( int p ) ;
  void modToS3m       ( ModNote *mp, Note *np ) ;
  void makeSampleInfo ( int smp15 ) ;
  void parseMod       ( unsigned char *pp0, int smp15 ) ;
  void play_one       ( int ppat ) ;
  unsigned char *read_whole_file ( const char *fname, int *len ) ;

public:

   MODfile ( const char *fname, int speed = 44100, int stereo = 0 ) ;
  ~MODfile () ;

  int update () ;
} ;

