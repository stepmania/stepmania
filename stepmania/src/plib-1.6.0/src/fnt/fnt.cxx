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

#include "fntLocal.h"

fntFont:: fntFont () {}
fntFont::~fntFont () {}

int fntTexFont::load ( const char *fname, GLenum mag, GLenum min )
{
  const char *p ;

  for ( p = & fname [ strlen ( fname ) -1 ] ;
        p != fname && *p != '.' && *p != '/' ; p-- )
    /* Do nothing */ ;

  if ( strcmp ( p, ".txf" ) == 0 ) {
    return loadTXF ( fname, mag, min ) ;
  }
  else
  {
    ulSetError ( UL_WARNING,
      "fnt::load: Error - Unrecognised file format for '%s'", fname ) ;
    return FNT_FALSE ;
  }
}



float fntTexFont::low_putch ( sgVec3 curpos, float pointsize,
                               float italic, char c )
{
  unsigned int cc = (unsigned char) c ;

  /* Auto case-convert if character is absent from font. */

  if ( ! exists [ cc ] )
  {
    if ( cc >= 'A' && cc <= 'Z' )
      cc = cc - 'A' + 'a' ;
    else
    if ( cc >= 'a' && cc <= 'z' )
      cc = cc - 'a' + 'A' ;

    if ( cc == ' ' )
    {
      curpos [ 0 ] += pointsize / 2.0f ;
      return pointsize / 2.0f ;
    }
  }

  /*
    We might want to consider making some absent characters from
    others (if they exist): lowercase 'l' could be made into digit '1'
    or letter 'O' into digit '0'...or vice versa. We could also
    make 'b', 'd', 'p' and 'q' by mirror-imaging - this would
    save a little more texture memory in some fonts.
  */

  if ( ! exists [ cc ] )
    return 0.0f ;
  
  glBegin ( GL_TRIANGLE_STRIP ) ;
    glTexCoord2f ( t_left [cc], t_bot[cc] ) ;
    glVertex3f   ( curpos[0] +          v_left [cc] * pointsize,
                   curpos[1] +          v_bot  [cc] * pointsize,
                   curpos[2] ) ;

    glTexCoord2f ( t_left [cc], t_top[cc] ) ;
    glVertex3f   ( curpos[0] + (italic + v_left [cc]) * pointsize,
                   curpos[1] +           v_top  [cc]  * pointsize,
                   curpos[2] ) ;

    glTexCoord2f ( t_right[cc], t_bot[cc] ) ;
    glVertex3f   ( curpos[0] +          v_right[cc] * pointsize,
                   curpos[1] +          v_bot  [cc] * pointsize,
                   curpos[2] ) ;

    glTexCoord2f ( t_right[cc], t_top[cc] ) ;
    glVertex3f   ( curpos[0] + (italic + v_right[cc]) * pointsize,
                   curpos[1] +           v_top  [cc]  * pointsize,
                   curpos[2] ) ;
  glEnd () ;

  float ww = ( gap + ( fixed_pitch ? width : widths[cc] ) ) * pointsize ;
  curpos[0] += ww ;
  return ww ;
}



void fntTexFont::setGlyph ( char c, float wid,
        float tex_left, float tex_right,
        float tex_bot , float tex_top  ,
        float vtx_left, float vtx_right,
        float vtx_bot , float vtx_top  )
{
  unsigned int cc = (unsigned char) c ;

  exists[cc] = FNT_TRUE ;

  widths[cc] = wid;

  t_left[cc] = tex_left ; t_right[cc] = tex_right ;
  t_bot [cc] = tex_bot  ; t_top  [cc] = tex_top   ;

  v_left[cc] = vtx_left ; v_right[cc] = vtx_right ;
  v_bot [cc] = vtx_bot  ; v_top  [cc] = vtx_top   ;
}


int fntTexFont::getGlyph ( char c, float* wid,
        float *tex_left, float *tex_right,
        float *tex_bot , float *tex_top  ,
        float *vtx_left, float *vtx_right,
        float *vtx_bot , float *vtx_top  )
{
  unsigned int cc = (unsigned char) c ;

  if ( ! exists[cc] ) return FNT_FALSE ;

  if ( wid       != NULL ) *wid       = widths [cc] ;

  if ( tex_left  != NULL ) *tex_left  = t_left [cc] ;
  if ( tex_right != NULL ) *tex_right = t_right[cc] ;
  if ( tex_bot   != NULL ) *tex_bot   = t_bot  [cc] ;
  if ( tex_top   != NULL ) *tex_top   = t_top  [cc] ;

  if ( vtx_left  != NULL ) *vtx_left  = v_left [cc] ;
  if ( vtx_right != NULL ) *vtx_right = v_right[cc] ;
  if ( vtx_bot   != NULL ) *vtx_bot   = v_bot  [cc] ;
  if ( vtx_top   != NULL ) *vtx_top   = v_top  [cc] ;

  return FNT_TRUE ;
}


void fntTexFont::getBBox ( const char *s,
                           float pointsize, float italic,
                           float *left, float *right,
                           float *bot , float *top  )
{
  float h_pos = 0.0f ;
  float v_pos = 0.0f ;
  float l, r, b, t ;

  l = r = b = t = 0.0f ;

  while ( *s != '\0' )
  {
    if ( *s == '\n' )
    {
      h_pos = 0.0f ;
      v_pos -= 1.333f ;
      s++ ;
      continue ;
    }

    unsigned int cc = (unsigned char) *(s++) ;

    if ( ! exists [ cc ] )
    {
      if ( cc >= 'A' && cc <= 'Z' )
        cc = cc - 'A' + 'a' ;
      else
      if ( cc >= 'a' && cc <= 'z' )
        cc = cc - 'a' + 'A' ;
  
      if ( cc == ' ' )
      {
        r += 0.5f ;
        h_pos += 0.5f ;

        continue ;
      }
    }

    if ( ! exists [ cc ] )
      continue ;

    if ( italic >= 0 )
    {
      if ( l >       h_pos + v_left [cc]        ) l =       h_pos + v_left [cc]          ;
      if ( r < gap + h_pos + v_right[cc]+italic ) r = gap + h_pos + v_right[cc] + italic ;
    }
    else
    {
      if ( l >       h_pos + v_left [cc]+italic ) l =       h_pos + v_left [cc] + italic ;
      if ( r < gap + h_pos + v_right[cc] )        r = gap + h_pos + v_right[cc]          ;
    }

    
    if ( b > v_pos + v_bot [cc] ) b = v_pos + v_bot [cc] ;
    if ( t < v_pos + v_top [cc] ) t = v_pos + v_top [cc] ;

    h_pos += gap + ( fixed_pitch ? width : widths[cc] ) ;
  }

  if ( left  != NULL ) *left  = l * pointsize ;
  if ( right != NULL ) *right = r * pointsize ;
  if ( top   != NULL ) *top   = t * pointsize ;
  if ( bot   != NULL ) *bot   = b * pointsize ;
}


void fntTexFont::puts ( sgVec3 curpos, float pointsize, float italic, const char *s )
{
  SGfloat origx = curpos[0] ;
    
  if ( ! bound )
    bind_texture () ;

  while ( *s != '\0' )
  {
    if (*s == '\n')
    {
      curpos[0]  = origx ;
      curpos[1] -= pointsize * 1.333f ;
    }
    else
      low_putch ( curpos, pointsize, italic, *s ) ;

    s++ ;
  }
}

