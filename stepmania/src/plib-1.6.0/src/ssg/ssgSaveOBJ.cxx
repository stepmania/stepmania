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
// Wavefront OBJ export for SSG/PLIB
// Ported from IVCON by Dave McClurg (dpm@efn.org) in March-2000
//

#include "ssgLocal.h"

static FILE *fileout ;
static int total_vert ;
static int total_normal ;

static void save_vtx_table ( ssgVtxTable *vt )
{
  float w = 1.0f ;
	short iv1, iv2;

  GLenum mode = vt -> getPrimitiveType () ;
	//fprintf ( fileout, "g test\n" ); wk: Pfusch fixme: is this necessary? NIV135
  
  if (( mode == GL_LINES ) || ( mode == GL_LINE_LOOP) || ( mode == GL_LINE_STRIP))
  {
    int num_vert = vt -> getNumVertices () ;
    
    // V: vertex coordinates. 
    for ( int i = 0; i < num_vert; i++ ) {
      sgVec3 vert ;
      sgCopyVec3 ( vert, vt -> getVertex ( i ) ) ;
      fprintf ( fileout, "v %f %f %f %f\n", 
        vert[0], vert[1], vert[2], w );
    }


  
    // L: lines. 
    fprintf ( fileout, "\n" );
		int num_lines = vt -> getNumLines () ;
    for ( int j = 0; j < num_lines; j ++ )
    { vt -> getLine ( j, &iv1, &iv2 ) ;
      fprintf ( fileout, "l %d %d\n",
         total_vert + j + 1,
         total_vert + j + 2 );
    }

    total_vert += num_vert ;
  }
  else if ( mode == GL_TRIANGLES ||
    mode == GL_TRIANGLE_FAN ||
    mode == GL_TRIANGLE_STRIP )
  {
    int num_vert = vt -> getNumVertices () ;

    // V: vertex coordinates. 
    for ( int i = 0; i < num_vert; i++ ) {
      sgVec3 vert ;
      sgCopyVec3 ( vert, vt -> getVertex ( i ) ) ;
      fprintf ( fileout, "v %f %f %f %f\n", 
        vert[0], vert[1], vert[2], w );
    }

    // VN: Vertex face normal vectors. 
    bool haveNormals = ( vt -> getNumNormals () >= num_vert ) ;
    if ( haveNormals )
    {
      fprintf ( fileout, "\n" );
      for ( int i = 0; i < num_vert; i++ ) {
        sgVec3 vert ;
        sgCopyVec3 ( vert, vt -> getNormal ( i ) ) ;
        fprintf ( fileout, "vn %f %f %f\n",
          vert[0], vert[1], vert[2] );
      }
    }

    // F: faces.
    fprintf ( fileout, "\n" );
    int num_face = vt -> getNumTriangles () ;
    for ( int j = 0; j < num_face; j++ )
    {
      short face[3];
      vt -> getTriangle ( j, &face[0], &face[1], &face[2] ) ;

      fprintf ( fileout, "f" );
      for ( int ivert = 0; ivert < 3; ivert++ ) {
        if ( haveNormals )
          fprintf ( fileout, " %d//%d",
            total_vert + face[ivert] + 1,
            total_normal + face[ivert] + 1 );
        else
          fprintf ( fileout, " %d",
            total_vert + face[ivert] + 1 );
      }
      fprintf ( fileout, "\n" );
    }

    total_vert += num_vert ;
    if ( haveNormals )
       total_normal += num_vert ;
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

int ssgSaveOBJ ( const char *filename, ssgEntity *ent )

/******************************************************************************/

/*
  Purpose:
   
    OBJ_WRITE writes a Wavefront OBJ file.

  Example:

    #  magnolia.obj

    mtllib ./vp.mtl

    g
    v -3.269770 -39.572201 0.876128
    v -3.263720 -39.507999 2.160890
    ...
    v 0.000000 -9.988540 0.000000
    g stem
    s 1
    usemtl brownskn
    f 8 9 11 10
    f 12 13 15 14
    ...
    f 788 806 774

  Modified:

    01 September 1998

  Author:
 
    John Burkardt
*/
{
  fileout = fopen ( filename, "wa" ) ;

  if ( fileout == NULL )
  {
    ulSetError ( UL_WARNING, "ssgSaveOBJ: Failed to open '%s' for writing", filename ) ;
    return FALSE ;
  }

/* 
  Initialize. 
*/
  fprintf ( fileout, "# %s created by SSG.\n", filename );
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "g SSG\n" );
  fprintf ( fileout, "\n" );

  total_vert = 0 ;
  total_normal = 0 ;

  save_entities ( ent ) ;

/*
  Close.
*/
  fclose ( fileout ) ;
  return TRUE;
}
