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
// DXF export for SSG/PLIB
// Ported from IVCON by Dave McClurg (dpm@efn.org) in March-2000
//

#include "ssgLocal.h"

static FILE *fileout ;

static void save_vtx_table ( ssgVtxTable *vt )
{
  GLenum mode = vt -> getPrimitiveType () ;
  if (( mode == GL_LINES ) || ( mode == GL_LINE_LOOP) || ( mode == GL_LINE_STRIP))
  {
    int num_vert = vt -> getNumVertices () ;
    num_vert = num_vert - ( num_vert & 1 ) ; //discard odd vertex
		int local_num_lines = vt -> getNumLines();

    for ( int j = 0; j < local_num_lines ; j ++ )
    {
      sgVec3 vert1, vert2;
			short iv1, iv2;
			vt -> getLine (j, &iv1, &iv2);

      sgCopyVec3 ( vert1, vt->getVertex ( iv1 ) ) ;
      sgCopyVec3 ( vert2, vt->getVertex ( iv2 ) ) ;

      fprintf ( fileout, "0\n" );
      fprintf ( fileout, "LINE\n" );
      fprintf ( fileout, "8\n" );
      fprintf ( fileout, "0\n" );
      fprintf ( fileout, "10\n" );
      fprintf ( fileout, "%f\n", vert1[0] );
      fprintf ( fileout, "20\n" );
      fprintf ( fileout, "%f\n", vert1[1] );
      fprintf ( fileout, "30\n" );
      fprintf ( fileout, "%f\n", vert1[2] );
      fprintf ( fileout, "11\n" );
      fprintf ( fileout, "%f\n", vert2[0] );
      fprintf ( fileout, "21\n" );
      fprintf ( fileout, "%f\n", vert2[1] );
      fprintf ( fileout, "31\n" );
      fprintf ( fileout, "%f\n", vert2[2] );
    }
  }
  else if ( mode == GL_TRIANGLES ||
    mode == GL_TRIANGLE_FAN ||
    mode == GL_TRIANGLE_STRIP )
  {
    int num_face = vt -> getNumTriangles () ;
    for ( int j = 0; j < num_face; j++ )
    {
      short face[3];
      vt -> getTriangle ( j, &face[0], &face[1], &face[2] ) ;

      fprintf ( fileout, "0\n" );
      fprintf ( fileout, "3DFACE\n" );
      fprintf ( fileout, "8\n" );
      fprintf ( fileout, "Cube\n" );
    
      sgVec3 vert;
      for ( int ivert = 0; ivert < 3; ivert++ ) {

        sgCopyVec3 ( vert, vt->getVertex ( face[ivert] ) ) ;
   
        fprintf ( fileout, "1%d\n", ivert );
        fprintf ( fileout, "%f\n", vert[0] );
        fprintf ( fileout, "2%d\n", ivert );
        fprintf ( fileout, "%f\n", vert[1] );
        fprintf ( fileout, "3%d\n", ivert );
        fprintf ( fileout, "%f\n", vert[2] );
      }
      fprintf ( fileout, "13\n");
      fprintf ( fileout, "%f\n", vert[0] );
      fprintf ( fileout, "23\n");
      fprintf ( fileout, "%f\n", vert[1] );
      fprintf ( fileout, "33\n");
      fprintf ( fileout, "%f\n", vert[2] );
    }
  }
	else
		ulSetError ( UL_WARNING, "ssgSaveDXF: OpenGL mode %d not implmented yet. Parts or all of the model are ignored!' for writing", (int)mode ) ;
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


int ssgSaveDXF ( const char *filename, ssgEntity *ent )
{
  fileout = fopen ( filename, "wa" ) ;

  if ( fileout == NULL )
  {
    ulSetError ( UL_WARNING, "ssgSaveDXF: Failed to open '%s' for writing", filename ) ;
    return FALSE ;
  }

/* 
  Initialize. 
*/
  fprintf ( fileout, "0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "2\n" );
  fprintf ( fileout, "HEADER\n" );
  fprintf ( fileout, "999\n" );
  fprintf ( fileout, "%s created by SSG.\n", filename );
  fprintf ( fileout, "0\n" );
  fprintf ( fileout, "ENDSEC\n" );

  fprintf ( fileout, "0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "2\n" );
  fprintf ( fileout, "TABLES\n" );
  fprintf ( fileout, "0\n" );
  fprintf ( fileout, "ENDSEC\n" );

  fprintf ( fileout, "0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "2\n" );
  fprintf ( fileout, "BLOCKS\n" );
  fprintf ( fileout, "0\n" );
  fprintf ( fileout, "ENDSEC\n" );

  fprintf ( fileout, "0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "2\n" );
  fprintf ( fileout, "ENTITIES\n" );

  save_entities ( ent ) ;

  fprintf ( fileout, "0\n" );
  fprintf ( fileout, "ENDSEC\n" );
  fprintf ( fileout, "0\n" );
  fprintf ( fileout, "EOF\n" );
/*
  Close.
*/
  fclose ( fileout ) ;
  return TRUE;
}
