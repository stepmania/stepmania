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

//
// TRI ( AC3D triangle file ) export for SSG/PLIB
// Written by Dave McClurg (dpm@efn.org) in March-2000
//

#include "ssgLocal.h"

static FILE *fileout ;


static void save_vtx_table ( ssgVtxTable *vt )
{
  GLenum mode = vt -> getPrimitiveType () ;
  if ( mode == GL_TRIANGLES ||
    mode == GL_TRIANGLE_FAN ||
    mode == GL_TRIANGLE_STRIP )
  {
    int num_tri = vt -> getNumTriangles () ;
    for ( int i = 0; i < num_tri; i++ )
    {
      short tri[3];
      vt -> getTriangle ( i, &tri[0], &tri[1], &tri[2] ) ;

      for ( int j = 0; j < 3; j ++ )
      {
        sgVec3 vert;
        sgCopyVec3 ( vert, vt->getVertex ( tri[j] ) ) ;
        fprintf ( fileout, "%f %f %f ", vert[0], vert[1], vert[2] ) ;
      }

      fprintf ( fileout, "0xFFFFFF\n" ) ;
    }
  }
}


static void save_entities ( ssgEntity *e )
{
  if ( e -> isAKindOf ( ssgTypeBranch() ) )
  {
    ssgBranch *br = (ssgBranch *) e ;

    for ( int i = 0 ; i < br -> getNumKids () ; i++ )
      save_entities ( br -> getKid ( i ) ) ;
  }
  else
  if ( e -> isAKindOf ( ssgTypeVtxTable() ) )
  {
    ssgVtxTable *vt = (ssgVtxTable *) e ;
    save_vtx_table ( vt ) ;
  }
}


/******************************************************************************/

int ssgSaveTRI ( const char *filename, ssgEntity *ent )

/******************************************************************************/

/*
  Purpose:
   
    writes an AC3D triangle file.

  Example:

    Each line contains 9 floating point values and a 1 hex value for color.
    the 9 floating point values represent 3 vertices of a triangle
    the color format is 0xRRGGBB (eg 0xffffff is white)

    0.0 0.0 0.0 1.0 0.0 0.0 1.0 1.0 0.0 0xffffff

*/
{
  fileout = fopen ( filename, "wa" ) ;

  if ( fileout == NULL )
  {
    ulSetError ( UL_WARNING, "ssgSaveTRI: Failed to open '%s' for writing", filename ) ;
    return FALSE ;
  }

  save_entities ( ent ) ;

  fclose ( fileout ) ;
  return TRUE;
}
