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


#ifndef _SSGASHAPES_H_
#define _SSGASHAPES_H_  1

#include "ssg.h"

typedef float sgVec9 [ 9 ] ;  /* Needed for ssgaPatch */

class ssgaShape : public ssgBranch
{
  int corrupted ;

protected:
  virtual void copy_from ( ssgaShape *src, int clone_flags ) ;

  sgVec4 colour ;
  sgVec3 center ;
  sgVec3 size   ;

  int ntriangles ;

  ssgState   *kidState      ;
  ssgCallback kidPreDrawCB  ;
  ssgCallback kidPostDrawCB ;

  void init () ;

protected:

  ssgState    *getKidState      () { return kidState      ; }
  ssgCallback  getKidPreDrawCB  () { return kidPreDrawCB  ; }
  ssgCallback  getKidPostDrawCB () { return kidPostDrawCB ; }

public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaShape (void) ;
  ssgaShape ( int numtris ) ;
  virtual ~ssgaShape (void) ;
  virtual const char *getTypeName(void) ;

  void makeCorrupt  () { corrupted = TRUE  ; }
  int  isCorrupt    () { return corrupted  ; }

  float *getCenter  () { return center     ; }
  float *getSize    () { return size       ; }
  int    getNumTris () { return ntriangles ; }

  void setColour  ( sgVec4 c ) { sgCopyVec4 ( colour, c ) ; regenerate () ; }
  void setCenter  ( sgVec3 c ) { sgCopyVec3 ( center, c ) ; regenerate () ; }
  void setSize    ( sgVec3 s ) { sgCopyVec3 ( size  , s ) ; regenerate () ; }
  void setSize    ( float  s ) { sgSetVec3  ( size,s,s,s) ; regenerate () ; }
  void setNumTris ( int ntri ) { ntriangles = ntri ; regenerate () ; }

  void setKidState    ( ssgState *s )
  {
    kidState = s ;

    for ( int i = 0 ; i < getNumKids() ; i++ )
      ((ssgLeaf *)getKid(i)) -> setState ( s ) ;
  }

  void setKidCallback ( int cb_type, ssgCallback cb )
  {
    if ( cb_type == SSG_CALLBACK_PREDRAW )
      kidPreDrawCB = cb ;
    else
      kidPostDrawCB = cb ;

    for ( int i = 0 ; i < getNumKids() ; i++ )
      ((ssgLeaf *)getKid(i)) -> setCallback ( cb_type, cb ) ;
  }

  virtual void regenerate () = 0 ;

  virtual int load ( FILE * ) ;
  virtual int save ( FILE * ) ;
} ;



class ssgaCube : public ssgaShape
{
protected:
  virtual void copy_from ( ssgaCube *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaCube (void) ;
  ssgaCube ( int numtris ) ;
  virtual ~ssgaCube (void) ;
  virtual const char *getTypeName(void) ;
  virtual void regenerate () ;
} ;



#define SSGA_HAVE_PATCH 1

class ssgaPatch : public ssgaShape
{
  int   levels ;
  sgVec9 control_points[4][4] ;
  void  makePatch  ( sgVec9 points[4][4], int levels ) ;
  void  writePatch ( sgVec9 points[4][4] ) ;
  void  makeHSpline  ( sgVec9 points[4]   , sgVec9 nv[7]    ) ;
  void  makeVSplines ( sgVec9 points[4][7], sgVec9 nv[7][7] ) ;


protected:
  virtual void copy_from ( ssgaPatch *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaPatch (void) ;
  ssgaPatch ( int numtris ) ;

  void setControlPoint ( int s, int t, sgVec3 xyz, sgVec2 uv, sgVec4 rgba ) ;
  void setControlPoint ( int s, int t,
                         float x, float y, float z,
                         float u, float v,
                         float r, float g, float b, float a ) ;
  void getControlPoint ( int s, int t, sgVec3 xyz, sgVec2 uv, sgVec4 rgba ) ;

  virtual ~ssgaPatch (void) ;
  virtual const char *getTypeName(void) ;
  virtual void regenerate () ;

  virtual int load ( FILE * ) ;
  virtual int save ( FILE * ) ;
} ;



#define SSGA_HAVE_TEAPOT 1

class ssgaTeapot : public ssgaShape
{
protected:
  virtual void copy_from ( ssgaTeapot *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaTeapot (void) ;
  ssgaTeapot ( int numtris ) ;
  virtual ~ssgaTeapot (void) ;
  virtual const char *getTypeName(void) ;
  virtual void regenerate () ;

  virtual int load ( FILE * ) ;
  virtual int save ( FILE * ) ;
} ;



class ssgaSphere : public ssgaShape
{
  int latlong_style ;

  void regenerateLatLong () ;
  void regenerateTessellatedIcosahedron () ;
protected:
  virtual void copy_from ( ssgaSphere *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaSphere (void) ;
  ssgaSphere ( int numtris ) ;
  virtual ~ssgaSphere (void) ;
  virtual const char *getTypeName(void) ;
  virtual void regenerate () ;

  void setLatLongStyle ( int ll ) { latlong_style = ll ; regenerate () ; }
  int  isLatLongStyle  ()         { return latlong_style ; }

  virtual int load ( FILE * ) ;
  virtual int save ( FILE * ) ;
} ;



class ssgaCylinder : public ssgaShape
{
  int capped ;

protected:
  virtual void copy_from ( ssgaCylinder *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaCylinder (void) ;
  ssgaCylinder ( int numtris ) ;
  virtual ~ssgaCylinder (void) ;
  virtual const char *getTypeName(void) ;
  virtual void regenerate () ;

  void makeCapped ( int c ) { capped = c ; regenerate () ; }
  int  isCapped   ()        { return capped ; }

  virtual int load ( FILE * ) ;
  virtual int save ( FILE * ) ;
} ;

#endif

