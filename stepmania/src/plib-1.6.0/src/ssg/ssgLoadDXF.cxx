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
// DXF loader for SSG/PLIB
// Ported from IVCON by Dave McClurg (dpm@efn.org) in March-2000
//   IVCON::dxf_read() by John Burkardt, 23 May 1999
// Added better support for meshes, August 2001
// Added support for colors, blocks and block references, September 2001
//

#include "ssgLocal.h"


#define MAX_LINE_LEN 1024

class dxfVert {
public:
  sgVec3 pos ;
  int color_index ;
	int isEqual ( class dxfVert * other );
} ;

class dxfVertArray : public ssgSimpleList
{
public:
  dxfVertArray () : ssgSimpleList ( sizeof(dxfVert), 0, NULL ) {} 
  dxfVert *get ( unsigned int n ) { return (dxfVert *) raw_get ( n ) ; }
  void add ( const dxfVert* thing ) { raw_add ( (char *) thing ) ; } ;
} ;

struct dxfMesh
{
  int flags;
  int size[2];
  int vlist[4];
  int vnum;
} ;

enum Entities {
  ENT_NONE,
  ENT_LINE,
  ENT_FACE,
  ENT_POLYLINE,
  ENT_VERTEX,
  ENT_INSERT
};

static int ent_type = ENT_NONE ;

static const ssgLoaderOptions* current_options = NULL ;
static ssgState* current_state = NULL ;
static ssgBranch* top_branch = NULL ;
static ssgBranch* current_block = NULL ;
static ssgBranch* blocks = NULL ;

static int num_line;
static int num_face;
static dxfVertArray tempvert;
static dxfVertArray linevert;
static dxfVertArray facevert;
static dxfVertArray meshvert;
static dxfMesh mesh;

static int cflags;
static sgVec3 cvec;
static sgVec3 scale_vec;
static float rot_angle;
static char* block_name;
static int color_index ;

static float dxf_colors [256][3] =
{
  { 0.0000f, 0.0000f, 0.0000f },
  { 1.0000f, 0.0000f, 0.0000f },
  { 1.0000f, 1.0000f, 0.0000f },
  { 0.0000f, 1.0000f, 0.0000f },
  { 0.0000f, 1.0000f, 1.0000f },
  { 0.0000f, 0.0000f, 1.0000f },
  { 1.0000f, 0.0000f, 1.0000f },
  { 0.0000f, 0.0000f, 0.0000f },
  { 0.0000f, 0.0000f, 0.0000f },
  { 0.0000f, 0.0000f, 0.0000f },
  { 1.0000f, 0.0000f, 0.0000f },
  { 1.0000f, 0.5000f, 0.5000f },
  { 0.6500f, 0.0000f, 0.0000f },
  { 0.6500f, 0.3250f, 0.3250f },
  { 0.5000f, 0.0000f, 0.0000f },
  { 0.5000f, 0.2500f, 0.2500f },
  { 0.3000f, 0.0000f, 0.0000f },
  { 0.3000f, 0.1500f, 0.1500f },
  { 0.1500f, 0.0000f, 0.0000f },
  { 0.1500f, 0.0750f, 0.0750f },
  { 1.0000f, 0.2500f, 0.0000f },
  { 1.0000f, 0.6250f, 0.5000f },
  { 0.6500f, 0.1625f, 0.0000f },
  { 0.6500f, 0.4063f, 0.3250f },
  { 0.5000f, 0.1250f, 0.0000f },
  { 0.5000f, 0.3125f, 0.2500f },
  { 0.3000f, 0.0750f, 0.0000f },
  { 0.3000f, 0.1875f, 0.1500f },
  { 0.1500f, 0.0375f, 0.0000f },
  { 0.1500f, 0.0938f, 0.0750f },
  { 1.0000f, 0.5000f, 0.0000f },
  { 1.0000f, 0.7500f, 0.5000f },
  { 0.6500f, 0.3250f, 0.0000f },
  { 0.6500f, 0.4875f, 0.3250f },
  { 0.5000f, 0.2500f, 0.0000f },
  { 0.5000f, 0.3750f, 0.2500f },
  { 0.3000f, 0.1500f, 0.0000f },
  { 0.3000f, 0.2250f, 0.1500f },
  { 0.1500f, 0.0750f, 0.0000f },
  { 0.1500f, 0.1125f, 0.0750f },
  { 1.0000f, 0.7500f, 0.0000f },
  { 1.0000f, 0.8750f, 0.5000f },
  { 0.6500f, 0.4875f, 0.0000f },
  { 0.6500f, 0.5688f, 0.3250f },
  { 0.5000f, 0.3750f, 0.0000f },
  { 0.5000f, 0.4375f, 0.2500f },
  { 0.3000f, 0.2250f, 0.0000f },
  { 0.3000f, 0.2625f, 0.1500f },
  { 0.1500f, 0.1125f, 0.0000f },
  { 0.1500f, 0.1313f, 0.0750f },
  { 1.0000f, 1.0000f, 0.0000f },
  { 1.0000f, 1.0000f, 0.5000f },
  { 0.6500f, 0.6500f, 0.0000f },
  { 0.6500f, 0.6500f, 0.3250f },
  { 0.5000f, 0.5000f, 0.0000f },
  { 0.5000f, 0.5000f, 0.2500f },
  { 0.3000f, 0.3000f, 0.0000f },
  { 0.3000f, 0.3000f, 0.1500f },
  { 0.1500f, 0.1500f, 0.0000f },
  { 0.1500f, 0.1500f, 0.0750f },
  { 0.7500f, 1.0000f, 0.0000f },
  { 0.8750f, 1.0000f, 0.5000f },
  { 0.4875f, 0.6500f, 0.0000f },
  { 0.5688f, 0.6500f, 0.3250f },
  { 0.3750f, 0.5000f, 0.0000f },
  { 0.4375f, 0.5000f, 0.2500f },
  { 0.2250f, 0.3000f, 0.0000f },
  { 0.2625f, 0.3000f, 0.1500f },
  { 0.1125f, 0.1500f, 0.0000f },
  { 0.1313f, 0.1500f, 0.0750f },
  { 0.5000f, 1.0000f, 0.0000f },
  { 0.7500f, 1.0000f, 0.5000f },
  { 0.3250f, 0.6500f, 0.0000f },
  { 0.4875f, 0.6500f, 0.3250f },
  { 0.2500f, 0.5000f, 0.0000f },
  { 0.3750f, 0.5000f, 0.2500f },
  { 0.1500f, 0.3000f, 0.0000f },
  { 0.2250f, 0.3000f, 0.1500f },
  { 0.0750f, 0.1500f, 0.0000f },
  { 0.1125f, 0.1500f, 0.0750f },
  { 0.2500f, 1.0000f, 0.0000f },
  { 0.6250f, 1.0000f, 0.5000f },
  { 0.1625f, 0.6500f, 0.0000f },
  { 0.4063f, 0.6500f, 0.3250f },
  { 0.1250f, 0.5000f, 0.0000f },
  { 0.3125f, 0.5000f, 0.2500f },
  { 0.0750f, 0.3000f, 0.0000f },
  { 0.1875f, 0.3000f, 0.1500f },
  { 0.0375f, 0.1500f, 0.0000f },
  { 0.0938f, 0.1500f, 0.0750f },
  { 0.0000f, 1.0000f, 0.0000f },
  { 0.5000f, 1.0000f, 0.5000f },
  { 0.0000f, 0.6500f, 0.0000f },
  { 0.3250f, 0.6500f, 0.3250f },
  { 0.0000f, 0.5000f, 0.0000f },
  { 0.2500f, 0.5000f, 0.2500f },
  { 0.0000f, 0.3000f, 0.0000f },
  { 0.1500f, 0.3000f, 0.1500f },
  { 0.0000f, 0.1500f, 0.0000f },
  { 0.0750f, 0.1500f, 0.0750f },
  { 0.0000f, 1.0000f, 0.2500f },
  { 0.5000f, 1.0000f, 0.6250f },
  { 0.0000f, 0.6500f, 0.1625f },
  { 0.3250f, 0.6500f, 0.4063f },
  { 0.0000f, 0.5000f, 0.1250f },
  { 0.2500f, 0.5000f, 0.3125f },
  { 0.0000f, 0.3000f, 0.0750f },
  { 0.1500f, 0.3000f, 0.1875f },
  { 0.0000f, 0.1500f, 0.0375f },
  { 0.0750f, 0.1500f, 0.0938f },
  { 0.0000f, 1.0000f, 0.5000f },
  { 0.5000f, 1.0000f, 0.7500f },
  { 0.0000f, 0.6500f, 0.3250f },
  { 0.3250f, 0.6500f, 0.4875f },
  { 0.0000f, 0.5000f, 0.2500f },
  { 0.2500f, 0.5000f, 0.3750f },
  { 0.0000f, 0.3000f, 0.1500f },
  { 0.1500f, 0.3000f, 0.2250f },
  { 0.0000f, 0.1500f, 0.0750f },
  { 0.0750f, 0.1500f, 0.1125f },
  { 0.0000f, 1.0000f, 0.7500f },
  { 0.5000f, 1.0000f, 0.8750f },
  { 0.0000f, 0.6500f, 0.4875f },
  { 0.3250f, 0.6500f, 0.5688f },
  { 0.0000f, 0.5000f, 0.3750f },
  { 0.2500f, 0.5000f, 0.4375f },
  { 0.0000f, 0.3000f, 0.2250f },
  { 0.1500f, 0.3000f, 0.2625f },
  { 0.0000f, 0.1500f, 0.1125f },
  { 0.0750f, 0.1500f, 0.1313f },
  { 0.0000f, 1.0000f, 1.0000f },
  { 0.5000f, 1.0000f, 1.0000f },
  { 0.0000f, 0.6500f, 0.6500f },
  { 0.3250f, 0.6500f, 0.6500f },
  { 0.0000f, 0.5000f, 0.5000f },
  { 0.2500f, 0.5000f, 0.5000f },
  { 0.0000f, 0.3000f, 0.3000f },
  { 0.1500f, 0.3000f, 0.3000f },
  { 0.0000f, 0.1500f, 0.1500f },
  { 0.0750f, 0.1500f, 0.1500f },
  { 0.0000f, 0.7500f, 1.0000f },
  { 0.5000f, 0.8750f, 1.0000f },
  { 0.0000f, 0.4875f, 0.6500f },
  { 0.3250f, 0.5688f, 0.6500f },
  { 0.0000f, 0.3750f, 0.5000f },
  { 0.2500f, 0.4375f, 0.5000f },
  { 0.0000f, 0.2250f, 0.3000f },
  { 0.1500f, 0.2625f, 0.3000f },
  { 0.0000f, 0.1125f, 0.1500f },
  { 0.0750f, 0.1313f, 0.1500f },
  { 0.0000f, 0.5000f, 1.0000f },
  { 0.5000f, 0.7500f, 1.0000f },
  { 0.0000f, 0.3250f, 0.6500f },
  { 0.3250f, 0.4875f, 0.6500f },
  { 0.0000f, 0.2500f, 0.5000f },
  { 0.2500f, 0.3750f, 0.5000f },
  { 0.0000f, 0.1500f, 0.3000f },
  { 0.1500f, 0.2250f, 0.3000f },
  { 0.0000f, 0.0750f, 0.1500f },
  { 0.0750f, 0.1125f, 0.1500f },
  { 0.0000f, 0.2500f, 1.0000f },
  { 0.5000f, 0.6250f, 1.0000f },
  { 0.0000f, 0.1625f, 0.6500f },
  { 0.3250f, 0.4063f, 0.6500f },
  { 0.0000f, 0.1250f, 0.5000f },
  { 0.2500f, 0.3125f, 0.5000f },
  { 0.0000f, 0.0750f, 0.3000f },
  { 0.1500f, 0.1875f, 0.3000f },
  { 0.0000f, 0.0375f, 0.1500f },
  { 0.0750f, 0.0938f, 0.1500f },
  { 0.0000f, 0.0000f, 1.0000f },
  { 0.5000f, 0.5000f, 1.0000f },
  { 0.0000f, 0.0000f, 0.6500f },
  { 0.3250f, 0.3250f, 0.6500f },
  { 0.0000f, 0.0000f, 0.5000f },
  { 0.2500f, 0.2500f, 0.5000f },
  { 0.0000f, 0.0000f, 0.3000f },
  { 0.1500f, 0.1500f, 0.3000f },
  { 0.0000f, 0.0000f, 0.1500f },
  { 0.0750f, 0.0750f, 0.1500f },
  { 0.2500f, 0.0000f, 1.0000f },
  { 0.6250f, 0.5000f, 1.0000f },
  { 0.1625f, 0.0000f, 0.6500f },
  { 0.4063f, 0.3250f, 0.6500f },
  { 0.1250f, 0.0000f, 0.5000f },
  { 0.3125f, 0.2500f, 0.5000f },
  { 0.0397f, 0.0000f, 0.3000f },
  { 0.1875f, 0.1500f, 0.3000f },
  { 0.0375f, 0.0000f, 0.1500f },
  { 0.0938f, 0.0750f, 0.1500f },
  { 0.5000f, 0.0000f, 1.0000f },
  { 0.7500f, 0.5000f, 1.0000f },
  { 0.3250f, 0.0000f, 0.6500f },
  { 0.4875f, 0.3250f, 0.6500f },
  { 0.2500f, 0.0000f, 0.5000f },
  { 0.3750f, 0.2500f, 0.5000f },
  { 0.1500f, 0.0000f, 0.3000f },
  { 0.2250f, 0.1500f, 0.3000f },
  { 0.0750f, 0.0000f, 0.1500f },
  { 0.1125f, 0.0750f, 0.1500f },
  { 0.7500f, 0.0000f, 1.0000f },
  { 0.8750f, 0.5000f, 1.0000f },
  { 0.4875f, 0.0000f, 0.6500f },
  { 0.5688f, 0.3250f, 0.6500f },
  { 0.3750f, 0.0000f, 0.5000f },
  { 0.4375f, 0.2500f, 0.5000f },
  { 0.2250f, 0.0000f, 0.3000f },
  { 0.2625f, 0.1500f, 0.3000f },
  { 0.1125f, 0.0000f, 0.1500f },
  { 0.1313f, 0.0750f, 0.1500f },
  { 1.0000f, 0.0000f, 1.0000f },
  { 1.0000f, 0.5000f, 1.0000f },
  { 0.6500f, 0.0000f, 0.6500f },
  { 0.6500f, 0.3250f, 0.6500f },
  { 0.5000f, 0.0000f, 0.5000f },
  { 0.5000f, 0.2500f, 0.5000f },
  { 0.3000f, 0.0000f, 0.3000f },
  { 0.3000f, 0.1500f, 0.3000f },
  { 0.1500f, 0.0000f, 0.1500f },
  { 0.1500f, 0.0750f, 0.1500f },
  { 1.0000f, 0.0000f, 0.7500f },
  { 1.0000f, 0.5000f, 0.8750f },
  { 0.6500f, 0.0000f, 0.4875f },
  { 0.6500f, 0.3250f, 0.5688f },
  { 0.5000f, 0.0000f, 0.3750f },
  { 0.5000f, 0.2500f, 0.4375f },
  { 0.3000f, 0.0000f, 0.2250f },
  { 0.3000f, 0.1500f, 0.2625f },
  { 0.1500f, 0.0000f, 0.1125f },
  { 0.1500f, 0.0750f, 0.1313f },
  { 1.0000f, 0.0000f, 0.5000f },
  { 1.0000f, 0.5000f, 0.7500f },
  { 0.6500f, 0.0000f, 0.3250f },
  { 0.6500f, 0.3250f, 0.4875f },
  { 0.5000f, 0.0000f, 0.2500f },
  { 0.5000f, 0.2500f, 0.3750f },
  { 0.3000f, 0.0000f, 0.1500f },
  { 0.3000f, 0.1500f, 0.2250f },
  { 0.1500f, 0.0000f, 0.0750f },
  { 0.1500f, 0.0750f, 0.1125f },
  { 1.0000f, 0.0000f, 0.2500f },
  { 1.0000f, 0.5000f, 0.6250f },
  { 0.6500f, 0.0000f, 0.1625f },
  { 0.6500f, 0.3250f, 0.4063f },
  { 0.5000f, 0.0000f, 0.1250f },
  { 0.5000f, 0.2500f, 0.3125f },
  { 0.3000f, 0.0000f, 0.0750f },
  { 0.3000f, 0.1500f, 0.1875f },
  { 0.1500f, 0.0000f, 0.0375f },
  { 0.1500f, 0.0750f, 0.0938f },
  { 0.3300f, 0.3300f, 0.3300f },
  { 0.4640f, 0.4640f, 0.4640f },
  { 0.5980f, 0.5980f, 0.5980f },
  { 0.7320f, 0.7320f, 0.7320f },
  { 0.8660f, 0.8660f, 0.8660f },
  { 1.0000f, 1.0000f, 1.0000f },
};

int dxfVert::isEqual ( class dxfVert * other )
// compares strictly (without epsilon)
{
	if ( color_index != other->color_index ) 
		return FALSE;
  return sgCompareVec3 ( pos, other->pos, 0.0 );

}

/*static void copy_vert ( dxfVert& dst, const dxfVert& src )
{
  dst.color_index = src.color_index ;
  sgCopyVec3 ( dst.pos, src.pos ) ;
}*/

static void add_tri ( const dxfVert* p, const dxfVert* q, const dxfVert* r )
{
  facevert.add ( p ) ;
  facevert.add ( q ) ;
  facevert.add ( r ) ;
  num_face ++;
}

static void add_face ( void )
{
  int num_vert = tempvert.getNum() ;
	
	if ( num_vert >= 4 ) //quad?
	  if ( tempvert.get(3)->isEqual ( tempvert.get(2) ) )
			// pseudo quad.
			num_vert=3;

  if ( num_vert >= 3 )
  {
    if ( num_vert >= 4 ) //quad?
    {
      add_tri ( tempvert.get(0), tempvert.get(1), tempvert.get(3) ) ;
      add_tri ( tempvert.get(3), tempvert.get(1), tempvert.get(2) ) ;
    }
    else //triangle
    {
      add_tri ( tempvert.get(0), tempvert.get(1), tempvert.get(2) ) ;
    }
  }
}

static void dxf_flush ( void )
{
  int num_vert = tempvert.getNum() ;
  if ( ent_type == ENT_LINE )
  {
    if ( num_vert >= 2 )
    {
      linevert.add( tempvert.get(0) ) ;
      linevert.add( tempvert.get(1) ) ;
      num_line ++;
    }
  }
  else if ( ent_type == ENT_FACE )
  {
    add_face () ;
  }
  else if ( ent_type == ENT_POLYLINE )
  {
    mesh.flags = cflags;
    meshvert.removeAll();
    mesh.size[0] = mesh.vlist[0];
    mesh.size[1] = mesh.vlist[1];
  }
  else if ( ent_type == ENT_INSERT )
  {
    if ( block_name != NULL )
    {
      //block_name
      ssgEntity* found = blocks -> getByName ( block_name ) ;
      if ( found != NULL )
      {
        //cvec
        //scale_vec
        //rot_angle

        sgMat4 mat, tmp;
        sgVec3 axis = { 0.0f, 0.0f, 1.0f };
        sgMakeRotMat4 ( mat, rot_angle, axis ) ;

        sgMakeIdentMat4 ( tmp ) ;
        sgScaleVec4 ( tmp[0], scale_vec[0] );
        sgScaleVec4 ( tmp[1], scale_vec[1] );
        sgScaleVec4 ( tmp[2], scale_vec[2] );
        sgPostMultMat4 ( mat, tmp ) ;

        sgMakeTransMat4 ( tmp, cvec ) ;
        sgPostMultMat4 ( mat, tmp ) ;

        //printf("cvec(%.2f,%.2f,%.2f), scale(%.2f,%.2f,%.2f), rot(%.2f)\n",
        //  cvec[0], cvec[1], cvec[2], cvec[0], cvec[1], cvec[2], rot_angle ) ;
        //found -> print ( stderr, "", 99 );

        ssgTransform* block_tr = new ssgTransform ;
        block_tr -> setName ( block_name ) ;
        block_tr -> setTransform ( mat ) ;
        block_tr -> addKid ( found ) ;
        top_branch -> addKid ( block_tr ) ;
      }
    }
  }
  else if ( ent_type == ENT_VERTEX )
  {
    if ( (mesh.flags & 8) != 0 )
    {
      //This is a 3D Polyline
      if ( (cflags & 32) != 0 )
      {
        //This is a 3D Polyline vertex
        meshvert.add ( tempvert.get(0) ) ;
      }
    }
    else if ( (mesh.flags & 16) != 0 )
    {
      //This is a 3D polygon MxN mesh. (uniform grid)
      if ( (cflags & 64) != 0 )
      {
        //This is a 3D polygon mesh vertex
        meshvert.add ( tempvert.get(0) ) ;
      }
    }
    else if ( (mesh.flags & 64) != 0 )
    {
      //This Polyline is a polyface mesh.
      if ( (cflags & 128) != 0 )
      {
        if ( (cflags & 64) != 0 )
        {
          //This is a 3D polygon mesh vertex
          meshvert.add ( tempvert.get(0) ) ;
        }
        else if ( mesh.vnum >= 3 )
        {
          //copy each vertex where the first is numbered 1
          tempvert.removeAll();
          int error = 0;
          for ( int i=0; i<mesh.vnum; i++ )
          {
            int ival = mesh.vlist[i];
            if ( ival < 0 )
              ival = -ival;  //invisible vertex whatever that means :)
            if ( ival > 0 && ival <= meshvert.getNum() )
              tempvert.add ( meshvert.get(ival-1) ) ;
            else
              error = 1;
          }
          
          if ( error == 0 )
            add_face () ;
        }
      }
    }
  }

  cflags = 0;
  cvec[0] = 0.0f;
  cvec[1] = 0.0f;
  cvec[2] = 0.0f;
  scale_vec[0] = 1.0f;
  scale_vec[1] = 1.0f;
  scale_vec[2] = 1.0f;
  rot_angle = 0.0f;
  color_index = 7 ;

  tempvert.removeAll();

  mesh.vnum = 0 ;
  mesh.vlist[0] = 0;
  mesh.vlist[1] = 0;
  mesh.vlist[2] = 0;
  mesh.vlist[3] = 0;

  delete[] block_name;
  block_name = NULL;
}


static void dxf_free ()
{
  tempvert.removeAll();
  linevert.removeAll() ;
  facevert.removeAll() ;
  meshvert.removeAll() ;

  num_line = 0;
  num_face = 0;

  ent_type = ENT_NONE;
}


static sgVec4& get_color ( int ci )
{
  if ( ci < 0 )
    ci = 0 ;
  else if ( ci > 255 )
    ci = 255 ;
  float* rgb = dxf_colors[ci];
  static sgVec4 color ;
  sgSetVec4 ( color, rgb[0], rgb[1], rgb[2], 1.0f ) ;
  return color ;
}


static void dxf_create ( ssgBranch* br )
{
  dxf_flush () ;

  //create ssg nodes
  if ( num_face )
  {
    int num_vert = facevert.getNum () ;
    ssgVertexArray* vlist = new ssgVertexArray ( num_vert ) ;
    ssgColourArray* clist = new ssgColourArray ( num_vert ) ;
    ssgNormalArray* nlist = new ssgNormalArray ( num_vert ) ;
    sgVec3 normal ;
    for ( int i=0; i<num_vert; i++ )
    {
      if ( (i % 3) == 0 )
      {
        sgMakeNormal ( normal,
          facevert.get(i) -> pos,
          facevert.get(i+1) -> pos,
          facevert.get(i+2) -> pos ) ;
      }
      
      vlist -> add ( facevert.get(i) -> pos ) ;
      nlist -> add ( normal ) ;
      clist -> add ( get_color( facevert.get(i) -> color_index ) ) ;
    }
    ssgVtxTable *vtab = new ssgVtxTable ( GL_TRIANGLES, vlist, nlist, 0, clist );
    vtab -> setState ( current_state ) ;
    br -> addKid ( vtab ) ;
  }

  if ( num_line )
  {
    int num_vert = linevert.getNum () ;
    ssgVertexArray* vlist = new ssgVertexArray ( num_vert ) ;
    ssgColourArray* clist = new ssgColourArray ( num_vert ) ;
    for ( int i=0; i<num_vert; i++ )
    {
      vlist -> add ( linevert.get(i) -> pos ) ;
      clist -> add ( get_color(linevert.get(i) -> color_index) ) ;
    }
    ssgVtxTable *vtab = new ssgVtxTable ( GL_LINES, vlist, 0, 0, clist );
    vtab -> setState ( current_state ) ;
    br -> addKid ( vtab ) ;
  }

  dxf_free () ;
}


static int dxf_read ( FILE *filein )
{
  dxf_free () ;

/* 
  Read the next two lines of the file into INPUT1 and INPUT2. 
*/
  for ( ;; ) {

    int   count;
    int   code;
    int   width;
    char  input1[MAX_LINE_LEN];
    char  input2[MAX_LINE_LEN];

/* 
  INPUT1 should contain a single integer, which tells what INPUT2
  will contain.
*/
    if ( fgets ( input1, MAX_LINE_LEN, filein ) == NULL ) {
      break;
    }

    count = sscanf ( input1, "%d%n", &code, &width );
    if ( count <= 0 ) {
      break;
    }
/*
  Read the second line, and interpret it according to the code.
*/
    if ( fgets ( input2, MAX_LINE_LEN, filein ) == NULL ) {
      break;
    }

    if ( code == 0 ) {

      dxf_flush () ;

      //set ent_type
      if ( strncmp( input2, "LINE", 4 ) == 0 )
        ent_type = ENT_LINE ;
      else if ( strncmp( input2, "3DFACE", 6 ) == 0 )
        ent_type = ENT_FACE ;
      else if ( strncmp( input2, "POLYLINE", 8 ) == 0 )
        ent_type = ENT_POLYLINE ;
      else if ( strncmp( input2, "VERTEX", 6 ) == 0 )
        ent_type = ENT_VERTEX;
      else if ( strncmp( input2, "INSERT", 6 ) == 0 )
        ent_type = ENT_INSERT;
      else if ( strncmp( input2, "BLOCK", 5 ) == 0 ) {
        if ( current_block == NULL ) {
          current_block = new ssgBranch ;
          blocks -> addKid ( current_block ) ;
        }
      }
      else if ( strncmp( input2, "ENDBLK", 6 ) == 0 ) {
        if ( current_block != NULL ) {
          dxf_create ( current_block ) ;
          current_block = NULL ;
        }
      }
      else if ( strncmp( input2, "SEQEND", 6 ) == 0 ) {

#define PL_CLOSED_IN_M 		0x01
#define PL_CURVE_FIT_ADDED 	0x02
#define PL_SPLINE_FIT_ADDED 	0x04
#define PL_3D_POLYLINE 		0x08
#define PL_3D_MESH 		0x10
#define PL_CLOSED_IN_N 		0x20
#define PL_POLYFACE_MESH 	0x40
#define PL_USE_LINETYPE 	0x80

        int polyline_flags = mesh.flags ;

        if ( (polyline_flags & PL_3D_POLYLINE) != 0 ) {

          //This is a 3D Polyline
          int last = 0;
          int i = 1;
          int num_vert = meshvert.getNum () ;

          if ( (polyline_flags & (PL_CLOSED_IN_M|PL_CLOSED_IN_N)) != 0 ) {

            //Polyline is closed
            last = num_vert - 1;
            i = 0;
          }

          for ( ; i<num_vert; i++ )
          {
            linevert.add ( meshvert.get ( last ) ) ;
            linevert.add ( meshvert.get ( i ) ) ;
            num_line ++ ;
            last = i ;
          }
        }
        else if ( (polyline_flags & PL_3D_MESH) != 0 ) {

          //This is a 3D polygon MxN mesh. (uniform grid)
          int num_vert = meshvert.getNum () ;
          if ( num_vert >= ( mesh.size[0] * mesh.size[1] ) ) {

            int mesh_m = mesh.size[0];
            int mesh_n = mesh.size[1];

            dxfVert *buff[2];
            buff[0] = meshvert.get(0);
            buff[1] = meshvert.get(mesh_n);

            /* create triangles */
            int i,j;
            for (i=1;i<mesh_m;i++) {
              buff[1] = meshvert.get( mesh_n*i ) ;
              dxfVert* p = &buff[0][0];
              dxfVert* q = &buff[0][1];
              for (j=1;j<mesh_n;j++) {
                dxfVert* r = &buff[1][j-1];
                add_tri ( p, q, r ) ;
                p = q;
                q = &buff[1][j];
                add_tri ( p, q, r ) ;
                q = &buff[0][j+1];
              }
              if (polyline_flags & PL_CLOSED_IN_N) {
                dxfVert* p = &buff[0][mesh_n-1];
                dxfVert* q = &buff[0][0];
                dxfVert* r = &buff[1][mesh_n-1];
                add_tri ( p, q, r ) ;
                p = q;
                q = &buff[1][0];
                add_tri ( p, q, r ) ;
              }
              buff[0] = buff[1];
            }
            if (polyline_flags & PL_CLOSED_IN_M) {
              dxfVert* p = &buff[0][0];
              dxfVert* q = &buff[0][1];
              for (j=1;j<mesh_n;j++) {
                dxfVert* r = meshvert.get(j-1);
                add_tri ( p, q, r ) ;
                p = q;
                q = meshvert.get(j);
                add_tri ( p, q, r ) ;
                q = &buff[0][j+1];
              }
            }
          }
        }

        mesh.flags = 0;
        ent_type = ENT_NONE;
      }
      else
        ent_type = ENT_NONE;
    }
    else {
      
      int cpos;
      for (cpos = 0; input1[cpos] == ' '; cpos++)
        ;
      
      if ( input1[cpos] == '1' || input1[cpos] == '2' || input1[cpos] == '3' ) {

        char ch = input1[cpos+1];

        if ( input1[cpos] == '2' && ( ch == 0 || ch == ' ' || ch == '\n' ) ) {

          if ( ent_type == ENT_INSERT || current_block != NULL ) {

            if ( ent_type == ENT_INSERT && block_name == NULL ) {
              block_name = new char[ strlen(input2)+1 ] ;
              strcpy ( block_name, input2 ) ;
            }

            if ( current_block != NULL && current_block->getName() == NULL ) {
              char* name = new char[ strlen(input2)+1 ] ;
              strcpy ( name, input2 ) ;
              current_block->setName( name ) ;
            }
          }
        }
        else if ( ch == '0' || ch == '1' || ch == '2' || ch == '3' ) {

          float rval;
          count = sscanf ( input2, "%e%n", &rval, &width );

          switch ( input1[cpos] )
          {
            case '1':
              cvec[0] = rval;
              break;
  
            case '2':
              cvec[1] = rval;
              break;
  
            case '3':
              cvec[2] = rval;

              {
                dxfVert vert ;
                vert.color_index = color_index ;
                sgCopyVec3 ( vert.pos, cvec ) ;
  
                tempvert.add ( &vert ) ;
              }
              break;
          }
        }
      }
      else if ( input1[cpos] == '4' ) {

        float rval;
        count = sscanf ( input2, "%e%n", &rval, &width );

        switch ( input1[cpos+1] )
        {
          case '1':
            scale_vec[0] = rval;
            break;

          case '2':
            scale_vec[1] = rval;
            break;

          case '3':
            scale_vec[2] = rval;
            break;
        }
      }
      else if ( input1[cpos] == '5' ) {

        char ch = input1[cpos+1];
        if ( ch == '0' ) {

          count = sscanf ( input2, "%e%n", &rot_angle, &width );
        }
      }
      else if ( input1[cpos] == '6' ) {

        if ( input1[cpos+1] == '2' ) {

          count = sscanf ( input2, "%d%n", &color_index, &width );
        }
      }
      else if ( input1[cpos] == '7' ) {

        int ival ;
        count = sscanf ( input2, "%d%n", &ival, &width );

        switch ( input1[cpos+1] )
        {
          case '0':
            cflags = ival;
            break;

          case '1':
            mesh.vlist[0] = ival;
            mesh.vnum = 1;
            break;

          case '2':
            mesh.vlist[1] = ival;
            mesh.vnum = 2;
            break;

          case '3':
            mesh.vlist[2] = ival;
            mesh.vnum = 3;
            break;

          case '4':
            mesh.vlist[3] = ival;
            mesh.vnum = 4;
            break;
        }
      }
    }
  }

  dxf_create ( top_branch ) ;

  return TRUE;
}


static ssgState *make_state ()
{
  ssgSimpleState *st = new ssgSimpleState () ;

  sgVec4 spec = { 0.0f, 0.0f, 0.0f, 1.0f };
  sgVec4 emis = { 0.0f, 0.0f, 0.0f, 1.0f } ;
  float  shi  = { 0.0f } ;

  st -> setMaterial ( GL_SPECULAR, spec ) ;
  st -> setMaterial ( GL_EMISSION, emis ) ;
  st -> setShininess ( shi ) ;
  st -> enable ( GL_COLOR_MATERIAL ) ;
  st -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  st -> enable  ( GL_LIGHTING ) ;
  st -> setShadeModel ( GL_SMOOTH ) ;

  st -> disable ( GL_BLEND ) ;
  st -> setOpaque () ;
  st -> disable( GL_TEXTURE_2D ) ;

  return st ;
}


ssgEntity *ssgLoadDXF ( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  top_branch = NULL ;

  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;

  FILE *loader_fd = fopen ( filename, "ra" ) ;

  if ( loader_fd == NULL )
  {
    ulSetError ( UL_WARNING, "ssgLoadDXF: Failed to open '%s' for reading", filename ) ;
    return NULL ;
  }

  top_branch = new ssgTransform () ;

  blocks = new ssgBranch ;
  current_state = make_state () ;

  blocks -> ref () ;
  current_state -> ref () ;

  dxf_read ( loader_fd ) ;

  fclose ( loader_fd ) ;

  ssgDeRefDelete ( current_state ) ;
  ssgDeRefDelete ( blocks ) ;

  return top_branch ;
}
