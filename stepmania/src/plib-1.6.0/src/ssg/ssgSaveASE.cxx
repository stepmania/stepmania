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
// ASE ( 3DSMAX ASCII EXPORT Version 2.00 ) export for SSG/PLIB
// Written by Dave McClurg (dpm@efn.org) in Feb-2000
//

#include "ssgLocal.h"
#include "ssgLoaderWriterStuff.h"

static FILE *save_fd ;
static ssgSimpleStateArray gSSL;

static void save_state ( ssgSimpleState* st, int istate )
{
  
  float* amb = st -> getMaterial ( GL_AMBIENT ) ;
  float* diff = st -> getMaterial ( GL_DIFFUSE ) ;
  float* spec = st -> getMaterial ( GL_SPECULAR ) ;
  float shine = st -> getShininess () ;
  float trans = st -> isTranslucent () ? 1.0f : 0.0f ;
  
  fprintf ( save_fd, "  *MATERIAL %d {\n", istate );
  fprintf ( save_fd, "    *MATERIAL_NAME \"Material #%d\"\n", istate  );
  fprintf ( save_fd, "    *MATERIAL_CLASS \"Standard\"\n" );
  fprintf ( save_fd, "    *MATERIAL_AMBIENT %f %f %f\n", amb[0], amb[1], amb[2] ) ;
  fprintf ( save_fd, "    *MATERIAL_DIFFUSE %f %f %f\n", diff[0], diff[1], diff[2] ) ;
  fprintf ( save_fd, "    *MATERIAL_SPECULAR %f %f %f\n", spec[0], spec[1], spec[2] ) ;
  fprintf ( save_fd, "    *MATERIAL_SHINE %f\n", shine ) ;
  fprintf ( save_fd, "    *MATERIAL_SHINESTRENGTH %f\n", shine ) ;
  fprintf ( save_fd, "    *MATERIAL_TRANSPARENCY %f\n", trans ) ;
	fprintf ( save_fd, "    *MATERIAL_WIRESIZE 1.0000\n" );
	fprintf ( save_fd, "    *MATERIAL_SHADING Blinn\n" );
	fprintf ( save_fd, "    *MATERIAL_XP_FALLOFF 0.0000\n" );
	fprintf ( save_fd, "    *MATERIAL_SELFILLUM 0.0000\n" );
	fprintf ( save_fd, "    *MATERIAL_TWOSIDED\n" );
	fprintf ( save_fd, "    *MATERIAL_FALLOFF In\n" );
	fprintf ( save_fd, "    *MATERIAL_SOFTEN\n" );
	fprintf ( save_fd, "    *MATERIAL_XP_TYPE Filter\n" ) ;
  
  if ( st -> isEnabled ( GL_TEXTURE_2D ) )
  {
    const char* tfname = st -> getTextureFilename() ;

    fprintf ( save_fd, "    *MAP_DIFFUSE {\n" );
		fprintf ( save_fd, "      *MAP_NAME \"Map #%d\"\n", istate ) ;
		fprintf ( save_fd, "      *MAP_CLASS \"Bitmap\"\n" ) ;
		fprintf ( save_fd, "      *MAP_SUBNO 1\n" ) ;
		fprintf ( save_fd, "      *MAP_AMOUNT 1.0000\n" ) ;
		fprintf ( save_fd, "      *BITMAP \"%s\"\n", tfname ) ;
		fprintf ( save_fd, "      *MAP_TYPE Spherical\n" ) ;
		fprintf ( save_fd, "      *UVW_U_OFFSET 0.0000\n" ) ;
		fprintf ( save_fd, "      *UVW_V_OFFSET 0.0000\n" ) ;
		fprintf ( save_fd, "      *UVW_U_TILING 1.0000\n" ) ;
		fprintf ( save_fd, "      *UVW_V_TILING 1.0000\n" ) ;
		fprintf ( save_fd, "      *UVW_ANGLE 0.0000\n" ) ;
		fprintf ( save_fd, "      *UVW_BLUR 1.0000\n" ) ;
		fprintf ( save_fd, "      *UVW_BLUR_OFFSET 0.0000\n" ) ;
		fprintf ( save_fd, "      *UVW_NOUSE_AMT 1.0000\n" ) ;
		fprintf ( save_fd, "      *UVW_NOISE_SIZE 1.0000\n" ) ;
		fprintf ( save_fd, "      *UVW_NOISE_LEVEL 1\n" ) ;
		fprintf ( save_fd, "      *UVW_NOISE_PHASE 0.0000\n" ) ;
		fprintf ( save_fd, "      *BITMAP_FILTER Pyramidal\n" ) ;
    fprintf ( save_fd, "    }\n" );
  }
  
  fprintf ( save_fd, "  }\n" );
}


static void save_states ()
{
  fprintf ( save_fd, "*MATERIAL_LIST {\n" ) ;
  fprintf ( save_fd, "  *MATERIAL_COUNT %d\n", gSSL.getNum() ) ;
  
  for ( int i=0; i < gSSL.getNum(); i++ )
    save_state ( gSSL.get(i) , i ) ;
  
  fprintf ( save_fd, "}\n" );
}


static void save_vtx_table ( ssgVtxTable *vt )
{
  GLenum mode = vt -> getPrimitiveType () ;
  if ( mode != GL_TRIANGLES &&
    mode != GL_TRIANGLE_FAN &&
    mode != GL_TRIANGLE_STRIP )
  {
    //only triangle export
    return;
  }

  const char* name = vt->getPrintableName() ;
  int j ;

  int istate = gSSL.findIndex ( (ssgSimpleState*)vt->getState () ) ;
  ssgSimpleState* st = ( istate != -1 )? gSSL.get( istate ): 0;

/*
  Begin the big geometry block.
*/
  fprintf ( save_fd, "*GEOMOBJECT {\n" );
  fprintf ( save_fd, "  *NODE_NAME \"%s\"\n", name );

/*
  Sub block NODE_TM:
*/
  fprintf ( save_fd, "  *NODE_TM {\n" );
  fprintf ( save_fd, "    *NODE_NAME \"%s\"\n", name );
  fprintf ( save_fd, "    *INHERIT_POS 0 0 0\n" );
  fprintf ( save_fd, "    *INHERIT_ROT 0 0 0\n" );
  fprintf ( save_fd, "    *INHERIT_SCL 0 0 0\n" );
  fprintf ( save_fd, "    *TM_ROW0 1.0000 0.0000 0.0000\n" );
  fprintf ( save_fd, "    *TM_ROW1 0.0000 1.0000 0.0000\n" );
  fprintf ( save_fd, "    *TM_ROW2 0.0000 0.0000 1.0000\n" );
  fprintf ( save_fd, "    *TM_ROW3 0.0000 0.0000 0.0000\n" );
  fprintf ( save_fd, "    *TM_POS 0.0000 0.0000 0.0000\n" );
  fprintf ( save_fd, "    *TM_ROTAXIS 0.0000 0.0000 0.0000\n" );
  fprintf ( save_fd, "    *TM_ROTANGLE 0.0000\n" );
  fprintf ( save_fd, "    *TM_SCALE 1.0000 1.0000 1.0000\n" );
  fprintf ( save_fd, "    *TM_SCALEAXIS 0.0000 0.0000 0.0000\n" );
  fprintf ( save_fd, "    *TM_SCALEAXISANG 0.0000\n" );
  fprintf ( save_fd, "  }\n" );

/*
  Sub block MESH:
    Items
*/
  int num_vert = vt -> getNumVertices () ;
  int num_face = vt -> getNumTriangles () ;
  fprintf ( save_fd, "  *MESH {\n" );
  fprintf ( save_fd, "    *TIMEVALUE 0\n" );
  fprintf ( save_fd, "    *MESH_NUMVERTEX %d\n", num_vert );
  fprintf ( save_fd, "    *MESH_NUMFACES %d\n", num_face );
  
/*
  Sub sub block MESH_VERTEX_LIST
*/
  fprintf ( save_fd, "    *MESH_VERTEX_LIST {\n" );
  for ( j = 0; j < num_vert; j++ )
  {
    sgVec3 v;
    sgCopyVec3 ( v, vt->getVertex ( j ) ) ;
    fprintf ( save_fd, "      *MESH_VERTEX %d %f %f %f\n",
      j, v[0], v[1], v[2] );
  }
  fprintf ( save_fd, "    }\n" );
  
/*
  Sub sub block MESH_FACE_LIST
    Items MESH_FACE
*/
  fprintf ( save_fd, "    *MESH_FACE_LIST {\n" );
  for ( j = 0; j < num_face; j++ )
  {
    short i1,i2,i3;
    vt -> getTriangle ( j, &i1, &i2, &i3 ) ;

    fprintf ( save_fd, "      *MESH_FACE %d: A: %d B: %d C: %d", j, i1, i2, i3 ); 
    fprintf ( save_fd, " AB: 1 BC: 1 CA: 1 *MESH_SMOOTHING *MESH_MTLID 1\n" );
  }
  fprintf ( save_fd, "    }\n" );
  
/*
  Item MESH_NUMTVERTEX.
*/
  if ( st && st -> isEnabled ( GL_TEXTURE_2D ) &&
    vt -> getNumTexCoords () == num_vert )
  {
    fprintf ( save_fd, "    *MESH_NUMTVERTEX %d\n", num_vert );
    fprintf ( save_fd, "    *MESH_TVERTLIST {\n" );
    for ( j = 0; j < num_vert; j++ )
    {
      sgVec2 tv ;
      sgCopyVec2 ( tv, vt->getTexCoord ( j ) ) ;
      fprintf ( save_fd, "      *MESH_TVERT %d %f %f %f\n",
        j, tv[0], 1.0f - tv[1], 1.0f );
    }
    fprintf ( save_fd, "    }\n" );
    fprintf ( save_fd, "    *MESH_NUMTVFACES %d\n", num_face );
    fprintf ( save_fd, "    *MESH_TFACELIST {\n" );
    for ( j = 0; j < num_face; j++ )
    {
      short i1,i2,i3;
      vt -> getTriangle ( j, &i1, &i2, &i3 ) ;

      fprintf ( save_fd, "      *MESH_TFACE %d %d %d %d\n", j, i1, i2, i3 ); 
    }
    fprintf ( save_fd, "    }\n" );
  }
  else
  {
    fprintf ( save_fd, "    *MESH_NUMTVERTEX 0\n" );
  }
  
  fprintf ( save_fd, "done\n" ) ;
  fflush ( save_fd ) ;
/*
  Item NUMCVERTEX.
*/
  fprintf ( save_fd, "    *MESH_NUMCVERTEX 0\n" );

#if 0  
/*
  Sub block MESH_NORMALS
    Items MESH_FACENORMAL, MESH_VERTEXNORMAL (repeated)
*/
  fprintf ( save_fd, "    *MESH_NORMALS {\n" );
  for ( j = 0; j < num_face; j++ ) {

    fprintf ( save_fd, "      *MESH_FACENORMAL %d %f %f %f\n", 
      iface, face_normal[0][j], face_normal[1][j], face_normal[2][j] );

    for ( ivert = 0; ivert < face_order[j]; ivert++ ) {
      fprintf ( save_fd, "      *MESH_VERTEXNORMAL %d %f %f %f\n", 
        face[ivert][iface], vertex_normal[0][ivert][iface], 
        vertex_normal[1][ivert][j], vertex_normal[2][ivert][j] );
    }
  }
  fprintf ( save_fd, "    }\n" );
#endif
  
/*
  Close the MESH object.
*/
  fprintf ( save_fd, "  }\n" );
  
/*
  A few closing parameters.
*/
  fprintf ( save_fd, "  *PROP_MOTIONBLUR 0\n" );
  fprintf ( save_fd, "  *PROP_CASTSHADOW 1\n" );
  fprintf ( save_fd, "  *PROP_RECVSHADOW 1\n" );
  
  if ( st )
    fprintf ( save_fd, "  *MATERIAL_REF %d\n", gSSL.findIndex ( st ) );

/*
  Close the GEOM object.
*/
  fprintf ( save_fd, "}\n" );
}


static void save_geom ( ssgEntity *e )
{
  if ( e -> isAKindOf ( ssgTypeBranch() ) )
  {
    ssgBranch *br = (ssgBranch *) e ;

    if ( br -> isAKindOf ( ssgTypeSelector() ) )
    {
      save_geom ( br -> getKid ( 0 ) ) ;
    }
    else
    {
      for ( int i = 0 ; i < br -> getNumKids () ; i++ )
        save_geom ( br -> getKid ( i ) ) ;
    }
  }
  else
  if ( e -> isAKindOf ( ssgTypeVtxTable() ) )
  {
    ssgVtxTable *vt = (ssgVtxTable *) e ;
    save_vtx_table ( vt ) ;
  }
}


int ssgSaveASE ( FILE* fileout, ssgEntity *ent )
{
  save_fd = fileout ;
  
/*
  Write the header.
*/
  fprintf ( save_fd, "*3DSMAX_ASCIIEXPORT 200\n" );
  fprintf ( save_fd, "*COMMENT \"created by SSG.\"\n" );
  
/*
  Write the scene block.
*/
  fprintf ( save_fd, "*SCENE {\n" );
  fprintf ( save_fd, "  *SCENE_FILENAME \"\"\n" );
  fprintf ( save_fd, "  *SCENE_FIRSTFRAME 0\n" );
  fprintf ( save_fd, "  *SCENE_LASTFRAME 100\n" );
  fprintf ( save_fd, "  *SCENE_FRAMESPEED 30\n" );
  fprintf ( save_fd, "  *SCENE_TICKSPERFRAME 160\n" );
  fprintf ( save_fd, "  *SCENE_BACKGROUND_STATIC 0.0000 0.0000 0.0000\n" );
  fprintf ( save_fd, "  *SCENE_AMBIENT_STATIC 0.0431 0.0431 0.0431\n" );
  fprintf ( save_fd, "}\n" );

  
  gSSL.collect ( ent ) ;
  save_states () ;
  save_geom ( ent ) ;  
	gSSL.removeAll();  

  fflush ( save_fd ) ;
  
  return TRUE ;
}


int ssgSaveASE ( const char *filename, ssgEntity *ent )
{
  save_fd = fopen ( filename, "wa" ) ;

  if ( save_fd == NULL )
  {
    ulSetError ( UL_WARNING, "ssgSaveASE: Failed to open '%s' for writing", filename ) ;
    return FALSE ;
  }
  
  int result = ssgSaveASE ( save_fd, ent ) ;

  fclose ( save_fd ) ;
  
  return result ;
}
