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

/****
* NAME
*   ssgLoadASE -- ASE model loader
*
* DESCRIPTION
*   ssgLoadASE will load an ASE model exported from 3dsmax using
*   the standard ascii export v2.00 plugin.
*
*   The material list and geometry objects are parsed and converted
*   into an SSG scene graph.  shape, light, camera, and helper
*   nodes are ignored.
*
*   If the geometry has vertex colours (pre-lit) then lighting is
*   disabled for that leaf.
*
*   mesh animation creates ssgSelector entities.  transform
*   animation creates ssgTransform entities with ssgTransformArray
*   user_data.  see the viewer example for details on controlling
*   animation.
*
*   NOTE: be sure you _reset the transform_ of the top-level object
*   in 3dsmax before exporting transform animation.
*
*   Please report any bugs to the author.  Complete information about
*   the ASE format can be found in the 3dsmax SDK source code.
*
* AUTHOR
*   Dave McClurg <dpm@efn.org>
*
* CREATION DATE
*   Feb-2000
*
* MODIFICATION HISTORY
*   November 2000 - v1.1, Dave McClurg <dpm@efn.org>
*     o support for transform animation samples
****/

#include "ssgLocal.h"
#include "ssgParser.h"

#ifndef _WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#define u32 unsigned int
#define f32 float
#define cchar const char


static ssgLoaderOptions* current_options = NULL ;

struct aseVertexBuffer
{
  bool use_flag ;
  int index ;
  sgVec3 v ;
  sgVec2 tv ;
  sgVec3 cv ;
} ;


struct aseFace
{
  u32 v[3];
  u32 tv[3];
  u32 cv[3];
  u32 sub_index;

	// SAC: Additional vals to track normals for each face vertex.
  // Since each vertex can supposedly have various normals if shared by two
  // non-smoothed faces, we track them per face.  I haven't checked this as
  // good as I should have with smoothed & unsmoothed shapes :)
	bool has_vertex_normals;
	sgVec3 vn [3];
};


struct aseTransform
{
  sgVec3 pos ;
  sgVec3 axis ;   // axis of rotation (unit vector)
  f32 angle ;     // angle of rotation in radians (always >0)
  sgVec3 scale ;
} ;


struct aseMesh
{
  u32 num_faces ;
  u32 num_verts ;
  u32 num_tverts ;
  u32 num_cverts ;
  
  aseFace* faces ;
  sgVec3* verts ;
  sgVec2* tverts ;
  sgVec3* cverts ;
  
  aseMesh();
  ~aseMesh();
};


struct aseObject
{
  enum Type { GEOM, HELPER, CAMERA } ;
  int type ;
  char* name ;
  char* parent ;
  bool inherit_pos [3] ;
  sgVec3 pos ;
  sgVec3 target ;
  u32 mat_index ;
  u32 num_tkeys ;
  aseTransform* tkeys ;
  
  enum { MAX_FRAMES = 256 };
  aseMesh* mesh_list[ MAX_FRAMES ];
  int mesh_count ;
  
  aseObject( Type type );
  ~aseObject();
};

struct aseMaterial
{
  char* name ;
  u32 mat_index ;
  u32 sub_index ;
  bool sub_flag ;
  
  sgVec4 amb ;
  sgVec4 diff ;
  sgVec4 spec ;
  f32 shine ;
  f32 transparency ;
  
  char* tfname ;
  sgVec2 texrep ;
  sgVec2 texoff ;
};


#define MAX_MATERIALS 1000
static aseMaterial** materials ;
static u32 num_materials ;

static u32 first_frame ;
static u32 last_frame ;
static u32 frame_speed ;
static u32 ticks_per_frame ;
static u32 num_frames ;

static _ssgParserSpec parser_spec =
{
   "\r\n\t ",  // delim_chars_skipable 
	 0,          // delim_chars_non_skipable
   "{",        // open_brace_chars
   "}",        // close_brace_chars
   '"',        // quote_char
   0,          // comment_char
	 0           // comment_string
} ;

static _ssgParser parser;
static ssgBranch* top_branch;

aseMesh::aseMesh()
{
  memset(this,0,sizeof(aseMesh));
}


aseMesh::~aseMesh()
{
  delete[] faces;
  delete[] verts;
  delete[] tverts;
  delete[] cverts;
}


aseObject::aseObject( aseObject::Type _type )
{
  memset(this,0,sizeof(aseObject));
  type = _type ;
}


aseObject::~aseObject()
{
  delete[] name;
  delete[] parent;
  delete[] tkeys;
  for ( int i=0; i<MAX_FRAMES; i++ )
    delete mesh_list [ i ] ;
  memset(this,0,sizeof(aseObject));
}


static u32 count_sub_materials( u32 mat_index )
{
  u32 count = 0 ;
  for ( u32 i=0; i<num_materials; i++ )
  {
    aseMaterial* mat = materials[ i ] ;
    if ( mat->mat_index == mat_index && mat->sub_flag )
      count ++;
  }
  return count ;
}


static aseMaterial* find_material( u32 mat_index, u32 sub_index )
{
  u32 i;
  
  //find sub-material
  for ( i=0; i<num_materials; i++ )
  {
    aseMaterial* mat = materials[ i ] ;
    if ( mat->mat_index == mat_index && mat->sub_index == sub_index )
      return(mat);
  }
  
  //just match material #
  for ( i=0; i<num_materials; i++ )
  {
    aseMaterial* mat = materials[ i ] ;
    if ( mat->mat_index == mat_index )
      return(mat);
  }
  
  parser.error("unknown material #%d",mat_index);
  return(0);
}


static ssgSimpleState* make_state( aseMaterial* mat, bool prelit )
{
  if ( mat -> tfname  != NULL)
  {
    ssgSimpleState *st = current_options -> createSimpleState ( mat -> tfname ) ;
    if ( st != NULL )
      return st ;
  }

  ssgSimpleState *st = new ssgSimpleState () ;

  bool has_alpha = false ;

  if (mat -> tfname != NULL)
  {
    ssgTexture* tex = current_options -> createTexture ( mat->tfname ) ;
    has_alpha = tex -> hasAlpha () ;

    st -> setTexture ( tex ) ;
    st -> enable ( GL_TEXTURE_2D ) ;
  }
  else
  {
    st -> disable( GL_TEXTURE_2D ) ;
  }

  st -> disable ( GL_ALPHA_TEST ) ;

  if ( sgCompareFloat ( mat -> transparency, 0.0f, 0.01f ) > 0 || has_alpha )
  {
    st -> enable  ( GL_BLEND ) ;
    st -> setTranslucent () ;
  }
  else
  {
    st -> disable ( GL_BLEND ) ;
    st -> setOpaque () ;
  }
  
  if ( prelit )
  {
    st -> disable ( GL_LIGHTING ) ;
  }
  else
  {
    st -> setMaterial ( GL_AMBIENT, mat -> amb ) ;
    st -> setMaterial ( GL_DIFFUSE, mat -> diff ) ;
    st -> setMaterial ( GL_SPECULAR, mat -> spec ) ;
    st -> setShininess ( mat -> shine ) ;
    st -> disable ( GL_COLOR_MATERIAL ) ;
    st -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
    st -> enable  ( GL_LIGHTING ) ;
  }
  
  st -> setShadeModel ( GL_SMOOTH ) ;
  
  return st ;
}


static ssgSimpleState* get_state( aseMaterial* mat, bool prelit )
{
  // is material an ifl (image file list)
  if ( strnicmp ( "ifl_", mat -> name, 4 ) == 0 )
  {
    u32 num_subs = count_sub_materials ( mat -> mat_index );
    if ( num_subs < 2 )
      parser.error("ifl material only has <2 frames: %s",mat -> name);

    ssgStateSelector* selector = new ssgStateSelector ( num_subs ) ;
    for ( u32 i=0; i<num_subs; i++ )
    {
      aseMaterial* mat2 = find_material ( mat -> mat_index, i ) ;
      assert ( mat2 != NULL ) ;

      ssgSimpleState* st = make_state ( mat2, prelit ) ;
      selector -> setStep ( i, st ) ;
    }
    selector -> selectStep ( 0 ) ;
    return selector ;
  }
  return make_state ( mat, prelit ) ;
}


static int parse_map( aseMaterial* mat )
// return TRUE on success
{
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (!strcmp(token,"*BITMAP"))
    {
      if ( mat->tfname != NULL )
        parser.error("multiple textures for material: %s",mat->name);
      else
      {
        char* fname;
				if (! parser.parseString(fname, "bitmap filename") )
					return FALSE;
				
        //strip existing directory from fname
        char* slash = strrchr ( fname, '/' ) ;
        if ( !slash )
          slash = strrchr ( fname, '\\' ) ; //for dos
        if ( slash )
          fname = slash + 1 ;
        
        mat->tfname = new char [ strlen(fname)+1 ] ;
        strcpy ( mat->tfname, fname ) ;
      }
    }
    else if (!strcmp(token,"*UVW_U_TILING"))
    {
      if (! parser.parseFloat(mat->texrep[0], "tiling.u"))
				return FALSE;
    }
    else if (!strcmp(token,"*UVW_V_TILING"))
    {
      if (! parser.parseFloat(mat->texrep[1], "tiling.v"))
				return FALSE;
    }
    else if (!strcmp(token,"*UVW_U_OFFSET"))
    {
      if (! parser.parseFloat(mat->texoff[0] , "offset.u"))
				return FALSE;
    }
    else if (!strcmp(token,"*UVW_V_OFFSET"))
    {
      if (! parser.parseFloat(mat->texoff[1] , "offset.v"))
				return FALSE;
    }
  }
	return TRUE;
}


static int parse_material( u32 mat_index, u32 sub_index, cchar* mat_name )
// return TRUE on success
{
  if ( num_materials >= MAX_MATERIALS )
  {
    parser.error( "too many materials" );
    
    // skip material definition
    int startLevel = parser.level;
    while (parser.getLine( startLevel ) != NULL)
      ;
    return TRUE; // go on parsing
  }
  aseMaterial* mat = new aseMaterial;
  materials [ num_materials++ ] = mat ;
  
  memset ( mat, 0, sizeof(aseMaterial) ) ;
  mat->mat_index = mat_index ;
  mat->sub_index = sub_index ;
  mat->sub_flag = ( mat_name != 0 ) ;
  mat->texrep[0] = 1.0f ;
  mat->texrep[1] = 1.0f ;
  mat->texoff[0] = 0.0f ;
  mat->texoff[1] = 0.0f ;
  
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (!strcmp(token,"*MATERIAL_NAME"))
    {
      char* name;
			if (! parser.parseString(name, "mat name") )
				return FALSE;
			
      if ( mat->sub_flag )
      {
        char buff [ 256 ] ;
        sprintf( buff, "%s, sub#%d", mat_name, sub_index );
        
        mat->name = new char [ strlen(buff)+1 ] ;
        strcpy ( mat->name, buff ) ;
      }
      else
      {
        mat->name = new char [ strlen(name)+1 ] ;
        strcpy ( mat->name, name ) ;
      }
    }
    else if (!strcmp(token,"*MATERIAL_AMBIENT"))
    {
      if (! parser.parseFloat(mat->amb[ 0 ], "amb.r"))
				return FALSE;
      if (! parser.parseFloat(mat->amb[ 1 ], "amb.g"))
				return FALSE;
      if (! parser.parseFloat(mat->amb[ 2 ], "amb.b"))
				return FALSE;
      mat->amb[ 3 ] = 1.0f;
    }
    else if (!strcmp(token,"*MATERIAL_DIFFUSE"))
    {
      if (! parser.parseFloat(mat->diff[ 0 ], "diff.r"))
				return FALSE;
      if (! parser.parseFloat(mat->diff[ 1 ], "diff.g"))
				return FALSE;
      if (! parser.parseFloat(mat->diff[ 2 ], "diff.b"))
				return FALSE;
      mat->diff[ 3 ] = 1.0f;
    }
    else if (!strcmp(token,"*MATERIAL_SPECULAR"))
    {
      if (! parser.parseFloat(mat->spec[ 0 ], "spec.r"))
				return FALSE;
      if (! parser.parseFloat(mat->spec[ 1 ], "spec.g"))
				return FALSE;
      if (! parser.parseFloat(mat->spec[ 2 ], "spec.b"))
				return FALSE;
      mat->spec[ 3 ] = 1.0f;
    }
    else if (!strcmp(token,"*MATERIAL_SHINE"))
    {
      if (! parser.parseFloat(mat->shine, "shine"))
			  return FALSE;

			// SAC: Shine seems off.  If I load a 3ds of the same shape
      // the shine is 256x as much and works better, so I'll tweak
      // the val here  :)    I couldn't clarify the range used by
      // MAX SDK anywhere in the docs (though this assumes 0 - 0.5).
      // OpenGL is 0-128.
			mat->shine = mat->shine * 256.0f;
			if ( mat->shine > 128 )
				mat->shine = 128;
    }
    else if (!strcmp(token,"*MATERIAL_TRANSPARENCY"))
    {
      if (! parser.parseFloat(mat->transparency, "transparency"))
				return FALSE;
    }
    else if (!strcmp(token,"*MAP_DIFFUSE"))
    {
      //Need: what about MAP_GENERIC, MAP_AMBIENT, etc??
      if (! parse_map( mat ))
				return FALSE;
    }
    else if (!strcmp(token,"*SUBMATERIAL"))
    {
      u32 sub_index;
			if (! parser.parseUInt(sub_index, "sub mat #"))
				return FALSE;
      if (! parse_material( mat_index, sub_index, mat->name ))
				return FALSE;
    }
  }
  
  //parser.message("material: %s (%s)",mat->name,mat->tfname);
	return TRUE;
}


static int parse_material_list()
// return TRUE on success
{
  if ( num_materials )
    parser.error("multiple material lists");
  
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (!strcmp(token,"*MATERIAL"))
    {
      u32 mat_index;
			if (! parser.parseUInt(mat_index, "mat #"))
				return FALSE;
      if (! parse_material( mat_index, 9999, NULL ))
				return FALSE;
    }
  }
	return TRUE;
}


static int parse_mesh( aseObject* obj )
{
  aseMesh* mesh = NULL ;
	u32 mesh_face_normal_index = 0x7fffffff;
	u32 mesh_face_normal_count = 0;
  
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if ( mesh == NULL )
    {
      u32 frame = aseObject::MAX_FRAMES ;
      
      if (!strcmp(token,"*TIMEVALUE"))
      {
        u32 time;
				if (! parser.parseUInt(time, "time"))
					return FALSE;
        frame = (time + (ticks_per_frame-1)) / ticks_per_frame - first_frame;
      }
      else
      {
        parser.error("missing *TIMEVALUE");
        frame = aseObject::MAX_FRAMES ;
      }
      
      if ( frame >= aseObject::MAX_FRAMES || obj->mesh_list [ frame ] != NULL )
      {
        //ignore this mesh
        while (parser.getLine( startLevel )) ;
        return TRUE; // go on parsing
      }
      
      mesh = new aseMesh ;
      obj->mesh_list [ frame ] = mesh ;
      obj->mesh_count ++ ;
    }
    else if (!strcmp(token,"*MESH_NUMFACES"))
    {
      if (mesh -> faces)
        parser.error("%s already seen",token);
      else
      {
        if (! parser.parseUInt(mesh -> num_faces, "num_faces"))
					return FALSE;
        mesh -> faces = new aseFace[ mesh -> num_faces ];

        // SAC: Help clear all the has_vertex_normals flags
				memset ( mesh -> faces, 0, mesh -> num_faces * sizeof (aseFace) );
      }
    }
    else if (!strcmp(token,"*MESH_NUMTVFACES"))
    {
      u32 ntfaces;
			if (! parser.parseUInt(ntfaces, "ntfaces"))
			  return FALSE;
      if (ntfaces != mesh -> num_faces)
        parser.error("NUMTFACES(%d) != NUMFACES(%d)",ntfaces,mesh -> num_faces);
    }
    else if (!strcmp(token,"*MESH_NUMCVFACES"))
    {
      u32 ncfaces;
			if (! parser.parseUInt(ncfaces, "ncfaces"))
				return FALSE;
      if (ncfaces != mesh -> num_faces)
        parser.error("NUMCFACES(%d) != NUMFACES(%d)",ncfaces,mesh -> num_faces);
    }
    else if (!strcmp(token,"*MESH_NUMVERTEX"))
    {
      if (mesh -> verts)
        parser.error("%s already seen",token);
      else
      {
				if (! parser.parseUInt(mesh -> num_verts, "num_verts"))
					return FALSE;
        mesh -> verts = new sgVec3[ mesh -> num_verts ];
      }
    }
    else if (!strcmp(token,"*MESH_NUMTVERTEX"))
    {
      if (mesh -> tverts)
        parser.error("%s already seen",token);
      else
      {
        if (! parser.parseUInt(mesh -> num_tverts, "num_tverts"))
					return FALSE;
        if (mesh -> num_tverts)
          mesh -> tverts = new sgVec2[ mesh -> num_tverts ];
      }
    }
    else if (!strcmp(token,"*MESH_NUMCVERTEX"))
    {
      if (mesh -> cverts)
        parser.error("%s already seen",token);
      else
      {
        if (! parser.parseUInt(mesh -> num_cverts, "num_cverts"))
				  return FALSE;
        if (mesh -> num_cverts)
          mesh -> cverts = new sgVec3[ mesh -> num_cverts ];
      }
    }
    else if (!strcmp(token,"*MESH_FACE"))
    {
      u32 index;
			// wk: The old parseInt converted a "3:" into 3,
			// the new would give an error. Therefore I had to change this code:
			char *my_endptr, *my_token = parser.parseToken("face #");
			if (my_token[strlen(my_token)-1]==':')
				my_token[strlen(my_token)-1]=0;

			index = (unsigned int)(strtol( my_token, &my_endptr, 10));
			if ( (my_endptr != NULL) && (*my_endptr != 0))
			{ parser.error("The field face # should contain an integer number but contains %s", my_token) ;
				return FALSE;
			}

			
			if (index >= mesh -> num_faces)
        parser.error("bad face #");
      else
      {
        aseFace& face = mesh -> faces[ index ];
        
        parser.expect("A:");
        if (! parser.parseUInt(face.v[0], "face.v[0]"))
					return FALSE;
        parser.expect("B:");
        if (! parser.parseUInt(face.v[1], "face.v[1]"))
					return FALSE;
        parser.expect("C:");
        if (! parser.parseUInt(face.v[2], "face.v[2]"))
					return FALSE;
        
        //search for other flags
				token = parser.parseToken(0);
        while ( ! parser.eol )
        {
          if ( strcmp(token,"*MESH_MTLID") == 0 )
          {
            if (! parser.parseUInt(face.sub_index, "mat #"))
							return FALSE;
          }
					token = parser.parseToken(0);
        }
      }
    }
    else if (!strcmp(token,"*MESH_TFACE"))
    {
      u32 index;
			if (! parser.parseUInt(index, "tface #"))
				return FALSE;
      if (index >= mesh -> num_faces)
        parser.error("bad tface #");
      else
      {
        aseFace& face = mesh -> faces[ index ];
        
        if (! parser.parseUInt(face.tv[0], "tface.tv[0]"))
					return FALSE;
        if (! parser.parseUInt(face.tv[1], "tface.tv[1]"))
					return FALSE;
        if (! parser.parseUInt(face.tv[2], "tface.tv[2]"))
					return FALSE;
      }
    }
    else if (!strcmp(token,"*MESH_CFACE"))
    {
      u32 index;
			if (! parser.parseUInt(index, "cface #"))
				return FALSE;
      if (index >= mesh -> num_faces)
        parser.error("bad cface #");
      else
      {
        aseFace& face = mesh -> faces[ index ];
        
        if (! parser.parseUInt(face.cv[0], "tface.cv[0]"))
					return FALSE;
        if (! parser.parseUInt(face.cv[1], "tface.cv[1]"))
					return FALSE;
        if (! parser.parseUInt(face.cv[2], "tface.cv[2]"))
					return FALSE;
      }
    }
		// SAC: START OF VERTEX NORMAL PARSING SECTION   
		// NOTE: Assumes that three correctly-ordered *MESH_VERTEXNORMAL lines
    // come after each *MESH_FACENORMAL line.  Which seems to be the case
    // in current exporter
    else if (!strcmp(token,"*MESH_FACENORMAL"))		
		{
			if (! parser.parseUInt(mesh_face_normal_index, "face normal #"))
				return FALSE;
			mesh_face_normal_count = 0;
		}
    else if (!strcmp(token,"*MESH_VERTEXNORMAL"))
    {
			//int nverts = mesh -> num_verts;

      if ( mesh_face_normal_index >= mesh -> num_faces )
        parser.error("bad mesh_face_normal_index #");
      if ( mesh_face_normal_count >= 3 )
        parser.error("bad mesh_face_normal_count");

			aseFace& face = mesh -> faces[ mesh_face_normal_index ];
      sgVec3& vn = face . vn [ mesh_face_normal_count ];
			face.has_vertex_normals = true;
			u32 vertex_index;
			if (! parser.parseUInt(vertex_index, "vertex normal #"))
				return FALSE;
			if  ( face.v [mesh_face_normal_count] != vertex_index )
        parser.error("bad MESH_VERTEXNORMAL entries out of order!");
      if (! parser.parseFloat(vn[0], "vertexNormal.x"))
				return FALSE;
      if (! parser.parseFloat(vn[1], "vertexNormal.y"))
				return FALSE;
      if (! parser.parseFloat(vn[2], "vertexNormal.z"))
				return FALSE;

			// sgNormaliseVec3 ( vn ) ;
			mesh_face_normal_count++;
    }
		// SAC: END OF VERTEX NORMAL PARSING SECTION   
    else if (!strcmp(token,"*MESH_VERTEX"))
    {
      u32 index; 
			if (! parser.parseUInt(index, "vertex #"))
				return FALSE;
      if (index >= mesh -> num_verts)
        parser.error("bad vertex #");
      else
      {
        sgVec3& vert = mesh -> verts[ index ];
        
        if (! parser.parseFloat(vert[0], "vert.x"))
					return FALSE;
        if (! parser.parseFloat(vert[1], "vert.y"))
					return FALSE;
        if (! parser.parseFloat(vert[2], "vert.z"))
					return FALSE;
      }
    }
    else if (!strcmp(token,"*MESH_TVERT"))
    {
      u32 index;
			if (! parser.parseUInt(index, "tvertex #"))
			  return FALSE;
      if (index >= mesh -> num_tverts)
        parser.error("bad tvertex #");
      else
      {
        sgVec2& tvert = mesh -> tverts[ index ];
        
        if (! parser.parseFloat(tvert[0], "tvert.x"))
					return FALSE;
        if (! parser.parseFloat(tvert[1], "tvert.y"))
					return FALSE;
      }
    }
    else if (!strcmp(token,"*MESH_VERTCOL"))
    {
      u32 index;
			if (! parser.parseUInt(index, "cvertex #"))
				return FALSE;
      if (index >= mesh -> num_cverts)
        parser.error("bad cvertex #");
      else
      {
        sgVec3& cvert = mesh -> cverts[ index ];
        
        if (! parser.parseFloat(cvert[0], "cvert.x"))
					return FALSE;
        if (! parser.parseFloat(cvert[1], "cvert.y"))
					return FALSE;
        if (! parser.parseFloat(cvert[2], "cvert.z"))
					return FALSE;
      }
    }
  }
	return TRUE;
}


static void get_texcoord ( aseMaterial* mat, sgVec2 tv )
{
  //invert Y coordinate
  tv[1] = 1.0f - tv[1] ;

  tv[0] *= mat->texrep[0] ;
  tv[1] *= mat->texrep[1] ;
  tv[0] += mat->texoff[0] ;
  tv[1] += mat->texoff[1] ;
}


static ssgLeaf* add_points( aseObject* obj, aseMesh* mesh )
{
  ssgVertexArray* vl = NULL ;

  if ( obj->type == aseObject::CAMERA )
  {
    /* compute a normalized target vector */
    sgVec3 target ;
		sgCopyVec3 ( target, obj->target ) ;
		sgSubVec3 ( target, obj->pos ) ;

    SGfloat len = sgLengthVec3 ( target ) ;
    if ( len == SG_ZERO )
    {
      vl = new ssgVertexArray ( 1 ) ;
      vl -> add ( obj->pos ) ;
    }
    else
    {
      vl = new ssgVertexArray ( 2 ) ;

      sgNormaliseVec3 ( target ) ;
      sgAddVec3 ( target, obj->pos ) ;
  
      vl -> add ( obj->pos ) ;
      vl -> add ( target ) ;
    }
  }
  else if ( mesh != NULL )
  {
    u32 num_verts = mesh -> num_verts ;
    if ( num_verts == 0 )
      return NULL;

    vl = new ssgVertexArray ( num_verts ) ;

    //pass the data to ssg
    sgVec3* vert = mesh -> verts ;
    for ( u32 i=0; i < num_verts; i++, vert++ )
      vl -> add ( *vert ) ;
  }
  else
  {
    return NULL ;
  }

  ssgVtxTable* leaf = new ssgVtxTable ( GL_POINTS,
    vl, NULL, NULL, NULL ) ;

  if ( leaf != NULL )
  {
    // leaves with no faces are used for game logic
    // and should not be culled and rendered
    leaf -> clrTraversalMaskBits ( SSGTRAV_CULL ) ;
  }

  return ssgGetCurrentOptions () -> createLeaf ( leaf, obj -> name ) ;
}


static ssgLeaf* add_mesh( aseObject* obj, aseMesh* mesh, u32 sub_index )
{
  u32 i ;

  aseMaterial* mat = find_material ( obj->mat_index, sub_index ) ;
  if ( mat == NULL )
    return NULL ;
  
  //compute number of faces for this sub-material
  u32 num_faces = mesh -> num_faces ;
  if ( mat->sub_flag )
  {
    num_faces = 0 ;
    aseFace* face = mesh -> faces ;
    for ( i=0; i<mesh -> num_faces; i++, face++ )
    {
      if ( face->sub_index == mat->sub_index )
        num_faces ++ ;
    }
  }
  if ( num_faces == 0 )
    return NULL ;
  
  if ( mesh -> cverts == NULL )
  {
    u32 num_verts = num_faces * 3 ;

    //pass the data to ssg
    ssgSimpleState* st = get_state ( mat, false ) ;
    ssgColourArray* cl = new ssgColourArray ( 1 ) ;
    ssgVertexArray* vl = new ssgVertexArray ( num_verts ) ;
    ssgNormalArray* nl = new ssgNormalArray ( num_verts ) ;
    ssgTexCoordArray* tl = 0 ;
    if ( mesh -> tverts )  
      tl = new ssgTexCoordArray ( num_verts ) ;

    //set the material colour
    sgVec4 c ;
    sgCopyVec3 ( c, mat -> diff ) ;
    c[3] = 1.0f - mat -> transparency ;
    cl -> add ( c ) ;

    aseFace* face = mesh -> faces ;
    for ( i=0; i<mesh -> num_faces; i++, face++ )
    {
      if ( mat->sub_flag && face->sub_index != mat->sub_index )
        continue ;
      
      sgVec3 n ;
      sgMakeNormal ( n,
        mesh -> verts[ face->v[0] ] ,
        mesh -> verts[ face->v[1] ] ,
        mesh -> verts[ face->v[2] ] ) ;
      
      for ( u32 j=0; j<3; j++ )
      {
        vl -> add ( mesh -> verts[ face->v[j] ] ) ;

				// SAC: If per-vertex normals were found, use 'em.
        // Else use the face normal computed above
				if  ( face -> has_vertex_normals )
					nl -> add ( face->vn [j] ) ;
				else
					nl -> add ( n ) ;
        
        if ( mesh -> tverts )
        {
          sgVec2 tv ;
          sgCopyVec2 ( tv, mesh -> tverts[ face->tv[j] ] ) ;
          get_texcoord ( mat, tv ) ;
          tl -> add ( tv ) ;
        }
      }
    }

    ssgVtxTable* leaf = new ssgVtxTable ( GL_TRIANGLES,
      vl, nl, tl, cl ) ;
    leaf -> setCullFace ( TRUE ) ;
    leaf -> setState ( st ) ;
    return current_options -> createLeaf ( leaf, obj->name ) ;
  }
  
  //allocate map_index array
  int* map_index = new int [ mesh -> num_faces * 3 ] ;
  
  //allocate the vertex list
  u32 max_verts = mesh -> num_verts + mesh -> num_faces * 3 ;
  aseVertexBuffer* vert_list = new aseVertexBuffer [ max_verts ] ;
  
  //mark each vertex as *not* used
  aseVertexBuffer* vert = vert_list ;
  for ( i=0; i < max_verts; i++, vert++ )
  {
    vert -> use_flag = false ;
  }
  
  u32 extra_verts = 0 ;
  
  //build the vertex list
  aseFace* face = mesh -> faces ;
  for ( i=0; i<mesh -> num_faces; i++, face++ )
  {
    if ( mat->sub_flag && face->sub_index != mat->sub_index )
      continue ;
    
    for ( u32 j=0; j<3; j++ )
    {
      int k = i*3+j;
      
      map_index [k] = face->v[j] ;
      vert = vert_list + map_index[k];
      
      if ( vert -> use_flag )
      {
        //check for match
        bool match = true ;
        if ( mesh -> tverts &&
          sgCompareVec2 ( vert -> tv, mesh -> tverts[ face->tv[j] ], 0.0f ) != 0 )
          match = false ;
        else if ( mesh -> cverts &&
          sgCompareVec3 ( vert -> cv, mesh -> cverts[ face->cv[j] ], 0.0f ) != 0 )
          match = false ;
        if ( match )
          continue ;  //texcoord and color matches other vertex
        
        extra_verts ++ ;
        
        map_index [k] = mesh -> num_verts + k ;
        vert = vert_list + map_index[k];
      }
      
      //add the vertex
      vert -> use_flag = true;
      sgCopyVec3 ( vert -> v, mesh -> verts[ face->v[j] ] ) ;
      if ( mesh -> tverts )
        sgCopyVec2 ( vert -> tv, mesh -> tverts[ face->tv[j] ] ) ;
      if ( mesh -> cverts )
        sgCopyVec3 ( vert -> cv, mesh -> cverts[ face->cv[j] ] ) ;
    }
  }
  
  //assign a unique index to each vertex
  int num_verts = 0 ;
  vert = vert_list;
  for ( i=0; i < max_verts; i++, vert++ )
  {
    if ( vert -> use_flag )
      vert -> index = num_verts ++;
  }
  
  //if ( extra_verts > 0 )
  //   ulSetError( UL_DEBUG, "%d verts; %d added", num_verts-extra_verts, extra_verts );
  //else
  //   ulSetError( UL_DEBUG, "%d verts", num_verts );
  
  //pass the data to ssg
  ssgSimpleState* st = get_state ( mat, ( mesh -> cverts != NULL ) ) ;
  ssgIndexArray* il = new ssgIndexArray ( num_faces * 3 ) ;
  ssgVertexArray* vl = new ssgVertexArray ( num_verts ) ;
  ssgTexCoordArray* tl = 0 ;
  ssgColourArray* cl = 0 ;
  if ( mesh -> tverts )  
    tl = new ssgTexCoordArray ( num_verts ) ;
  if ( mesh -> cverts )
    cl = new ssgColourArray ( num_verts ) ;
  
  //build the index list
  face = mesh -> faces ;
  for ( i=0; i<mesh -> num_faces; i++, face++ )
  {
    if ( mat->sub_flag && face->sub_index != mat->sub_index )
      continue ;
    
    for ( u32 j=0; j<3; j++ )
    {
      int k = i*3+j;
      vert = vert_list + map_index[k];
      
      if ( ! vert -> use_flag )
        ulSetError ( UL_FATAL, "internal error" ) ;
      
      il -> add ( vert -> index ) ;
    }
  }
  
  //copy the vertex lists
  vert = vert_list;
  for ( i=0; i < max_verts; i++, vert++ )
  {
    if ( vert -> use_flag )
    {
      vl -> add ( vert -> v ) ;
      
      if ( mesh -> tverts )
      {
        sgVec2 tv ;
        sgCopyVec2 ( tv, vert -> tv ) ;
        get_texcoord ( mat, tv ) ;
        tl -> add ( tv ) ;
      }
      
      if ( mesh -> cverts )
      {
        sgVec4 c ;
        sgCopyVec3 ( c, vert -> cv ) ;
        c[3] = 1.0f - mat -> transparency ;
        
        cl -> add ( c ) ;
      }
    }
  }
  
  delete[] vert_list ;
  delete[] map_index ;

  ssgVtxArray* leaf = new ssgVtxArray ( GL_TRIANGLES,
    vl, NULL, tl, cl, il ) ;
  leaf -> setCullFace ( TRUE ) ;
  leaf -> setState ( st ) ;
  return current_options -> createLeaf ( leaf, obj -> name ) ;
}


static aseTransform* get_tkey( aseObject* obj, u32 time )
{
  if ( obj->tkeys == NULL )
  {
    obj->num_tkeys = 0;
    obj->tkeys = new aseTransform [ num_frames ] ;
    
    //initialize
    aseTransform* tkey = obj->tkeys ;
    for ( u32 i=0; i<num_frames; i++, tkey++ )
    {
      sgSetVec3 ( tkey->pos, 0, 0, 0 ) ;
      sgSetVec3 ( tkey->axis, 0, 0, 1 ) ;
      tkey->angle = 0.0f ;
      sgSetVec3 ( tkey->scale, 1, 1, 1 ) ;
    }
  }
  
  //compute frame number
  u32 frame = time / ticks_per_frame - first_frame;
  if ((time % ticks_per_frame) != 0 || frame >= num_frames)
    parser.error("bad time");
  
  if ( frame+1 > obj->num_tkeys )
    obj->num_tkeys = frame+1;
  
  return( &obj->tkeys[ frame ] );
}


static int parse_tkeys( aseObject* obj )
{
  bool match = false ;
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (!strcmp(token,"*NODE_NAME"))
    {
      char* name;
      if (! parser.parseString(name, "obj name"))
        return FALSE;
      if ( obj->name && !strcmp(name,obj->name) )
        match = true ;
    }
    else if (match)
    {
      if (!strcmp(token,"*CONTROL_POS_SAMPLE"))
      {
        u32 time;
        if (! parser.parseUInt(time, "time"))
          return FALSE;
        aseTransform* tkey = get_tkey( obj, time );
        
        if (! parser.parseFloat(tkey->pos[0], "pos.x"))
          return FALSE;
        if (! parser.parseFloat(tkey->pos[1], "pos.y"))
          return FALSE;
        if (! parser.parseFloat(tkey->pos[2], "pos.z"))
          return FALSE;
        
        if ( obj->parent == NULL )
        {
          sgSubVec3 ( tkey->pos, obj->pos ) ;
        }
        else
        {
          for ( int i=0; i<3; i++ )
          {
            if ( obj->inherit_pos[i] )
              tkey->pos[i] -= obj->pos[i] ;
          }
        }
        
        //copy the position forward
        for ( u32 i=obj->num_tkeys; i<num_frames; i++ )
          sgCopyVec3 ( obj->tkeys[ i ].pos, tkey->pos ) ;
      }
      else if (!strcmp(token,"*CONTROL_ROT_SAMPLE"))
      {
        u32 time;
        if (!parser.parseUInt(time, "time"))
          return FALSE;
        aseTransform* tkey = get_tkey( obj, time );
        
        if (! parser.parseFloat(tkey->axis[0], "axis.x"))
          return FALSE;
        if (! parser.parseFloat(tkey->axis[1], "axis.y"))
          return FALSE;
        if (! parser.parseFloat(tkey->axis[2], "axis.z"))
          return FALSE;
        if (! parser.parseFloat(tkey->angle, "angle"))
          return FALSE;
      }
      else if (!strcmp(token,"*CONTROL_SCALE_SAMPLE"))
      {
        u32 time;
        if (! parser.parseUInt(time, "time"))
          return FALSE;
        aseTransform* tkey = get_tkey( obj, time );
        
        if (! parser.parseFloat(tkey->scale[0], "scale.x"))
          return FALSE;
        if (! parser.parseFloat(tkey->scale[1], "scale.y"))
          return FALSE;
        if (! parser.parseFloat(tkey->scale[2], "scale.z"))
          return FALSE;
      }
    }
  }
	return TRUE;
}


static int parse_object( aseObject::Type type )
{
  aseObject* obj = new aseObject ( type ) ;
  
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (!strcmp(token,"*NODE_NAME"))
    {
      if ( !obj->name )
      {
        char* name;
				if (! parser.parseString(name, "obj name"))
					return FALSE;
				
        
        obj->name = new char [ strlen(name)+1 ] ;
        strcpy ( obj->name, name ) ;
      }
    }
    else if (!strcmp(token,"*NODE_PARENT"))
    {
      if ( !obj->parent )
      {
        char* name;
				if (! parser.parseString(name, "parent name"))
					return FALSE;
				
        
        obj->parent = new char [ strlen(name)+1 ] ;
        strcpy ( obj->parent, name ) ;
      }
    }
    else if (!strcmp(token,"*NODE_TM"))
    {
      bool target = false ;
      bool match = false ;
      int startLevel = parser.level;
      while ((token = parser.getLine( startLevel )) != NULL)
      {
        if (!strcmp(token,"*NODE_NAME"))
        {
          char* name;
  				if (! parser.parseString(name, "obj name"))
  					return FALSE;
          if ( obj->name && !strcmp(name,obj->name) )
            match = true ;
          if ( strstr(name,".Target") != NULL )
            target = true ;
        }
        else if (match)
        {
          if (!strcmp(token,"*TM_POS"))
          {
            if (! parser.parseFloat(obj->pos[0], "pos.x"))
              return FALSE;
            if (! parser.parseFloat(obj->pos[1], "pos.y"))
              return FALSE;
            if (! parser.parseFloat(obj->pos[2], "pos.z"))
              return FALSE;
            sgCopyVec3 ( obj->target, obj->pos ) ;
          }
          else if (!strcmp(token,"*INHERIT_POS"))
          {
            int temp ;
            if (! parser.parseInt(temp, "inherit_pos.x"))
              return FALSE;
            obj->inherit_pos[0] = ( temp != 0 ) ;
            if (! parser.parseInt(temp, "inherit_pos.y"))
              return FALSE;
            obj->inherit_pos[1] = ( temp != 0 ) ;
            if (! parser.parseInt(temp, "inherit_pos.z"))
              return FALSE;
            obj->inherit_pos[2] = ( temp != 0 ) ;
          }
        }
        else if (target)
        {
          if (!strcmp(token,"*TM_POS"))
          {
            if (! parser.parseFloat(obj->target[0], "pos.x"))
              return FALSE;
            if (! parser.parseFloat(obj->target[1], "pos.y"))
              return FALSE;
            if (! parser.parseFloat(obj->target[2], "pos.z"))
              return FALSE;
          }
        }
      }
    }
    else if (!strcmp(token,"*MESH"))
    {
      if (! parse_mesh( obj ))
				return FALSE;
    }
    else if (!strcmp(token,"*MESH_ANIMATION"))
    {
      int startLevel = parser.level;
      while ((token = parser.getLine( startLevel )) != NULL)
      {
        if (!strcmp(token,"*MESH"))
        {
          if (! parse_mesh( obj ))
						return FALSE;
        }
      }
    }
    else if (!strcmp(token,"*TM_ANIMATION"))
    {
      if (! parse_tkeys( obj ))
				return FALSE;
    }
    else if (!strcmp(token,"*MATERIAL_REF"))
    {
      if (! parser.parseUInt(obj->mat_index, "mat #"))
				return FALSE;
    }
  }
  
  //if ( obj->parent != NULL )
  //  ulSetError( UL_DEBUG, "add_mesh: %s, parent=%s", obj->name, obj->parent ) ;
  //else
  //  ulSetError( UL_DEBUG, "add_mesh: %s", obj->name ) ;
  
  ssgEntity* mesh_entity = NULL ;
  
  if ( obj->mesh_count > 1 )
  {
    //how many frames?
    int num_frames = 0 ;
    int i ;
    for ( i=0; i<aseObject::MAX_FRAMES; i++ )
    {
      aseMesh* mesh = obj->mesh_list [ i ] ;
      if ( mesh != NULL )
        num_frames ++ ;
    }
    
    //allocate selector
    ssgSelector* selector = new ssgSelector ( num_frames ) ;
    
    //init
    for ( i=0; i<aseObject::MAX_FRAMES; i++ )
    {
      aseMesh* mesh = obj->mesh_list [ i ] ;
      if ( mesh == NULL )
        continue ;
      
      u32 num_subs = count_sub_materials ( obj->mat_index );
      if ( num_subs > 1 )
      {
        //break apart the mesh for each sub material
        ssgBranch* branch = new ssgBranch ;
        for ( u32 sub_index=0; sub_index<num_subs; sub_index++ )
        {
          ssgLeaf* leaf = add_mesh ( obj, mesh, sub_index );
          if ( leaf )
            branch -> addKid ( leaf ) ;
        }
        selector -> addKid ( branch ) ;
      }
      else
      {
        ssgLeaf* leaf = add_mesh ( obj, mesh, 0 ) ;
        if ( leaf )
          selector -> addKid ( leaf ) ;
      }
    }

    selector -> selectStep ( 0 ) ;
    mesh_entity = current_options -> createSelector ( selector ) ;
  }
  else if ( obj->mesh_list [ 0 ] != NULL )
  {
    aseMesh* mesh = obj->mesh_list [ 0 ] ;
    ssgBranch* branch = new ssgBranch ;

    u32 num_faces = mesh -> num_faces ;
    if ( num_faces == 0 )
    {
      ssgLeaf* leaf = add_points( obj, mesh ) ;
      if ( leaf )
        branch -> addKid ( leaf ) ;
    }
    else
    {
      u32 num_subs = count_sub_materials ( obj->mat_index );
      if ( num_subs > 1 )
      {
        //break apart the mesh for each sub material
        for ( u32 sub_index=0; sub_index<num_subs; sub_index++ )
        {
          ssgLeaf* leaf = add_mesh ( obj, mesh, sub_index );
          if ( leaf )
            branch -> addKid ( leaf ) ;
        }
      }
      else
      {
        ssgLeaf* leaf = add_mesh ( obj, mesh, 0 );
        if ( leaf )
          branch -> addKid ( leaf ) ;
      }
    }
    
    mesh_entity = branch ;
  }
  else
  {
    ssgBranch* branch = new ssgBranch ;
    ssgLeaf* leaf = add_points( obj, NULL ) ;
    if ( leaf )
      branch -> addKid ( leaf ) ;
    mesh_entity = branch ;
  }
  
  if ( mesh_entity != NULL )
  {
    //add to graph -- find parent branch
    ssgBranch* parent_branch ;
    if ( obj->parent )
    {
      ssgEntity* found = top_branch -> getByName ( obj->parent ) ;
      if ( found != NULL )
      {
        assert ( found -> isAKindOf ( ssgTypeBranch() ) ) ;
        parent_branch = (ssgBranch *) found ;
      }
      else
      {
        //parser.error("object %s: parent %s not seen",obj->name,obj->parent);
        parent_branch = top_branch ;
      }
    }
    else
    {
      parent_branch = top_branch ;
    }
    
    if ( obj->num_tkeys > 0 )
    {
      ssgTransformArray* ta = new ssgTransformArray ( obj->num_tkeys ) ;

      /*
      Build the transforms
      */
      sgMat4 rmat;
      sgMakeIdentMat4 ( rmat ) ;
      ta -> add ( rmat ) ;

      for ( u32 i = 1 ; i < obj->num_tkeys ; i++ )
      {
        aseTransform* tkey = obj->tkeys + i ;
        
        sgVec3 pos ;
        sgMat4 tmp ;
        sgMat4 mat ;
        
        /*
        *  compute rmat
        *  the key rotation is additive to the last rotation
        */
        sgMakeRotMat4 ( tmp, -tkey->angle * SG_RADIANS_TO_DEGREES, tkey->axis ) ;
        sgPostMultMat4 ( rmat, tmp ) ;
        
        /*
        *  rotation is around the mesh pivot point (obj->pos)
        *  translate -obj->pos
        */
        sgCopyVec3 ( pos, obj->pos ) ;
        sgNegateVec3 ( pos ) ;
        sgMakeTransMat4 ( mat, pos ) ;
        
        /*
        *  perform the rotation
        */
        sgPostMultMat4 ( mat, rmat ) ;
        
        /*
        *  translate obj->pos + tkey->pos
        */
        sgCopyVec3 ( pos, obj->pos ) ;
        sgAddVec3 ( pos, tkey->pos ) ;
        sgMakeTransMat4 ( tmp, pos ) ;
        sgPostMultMat4 ( mat, tmp ) ;
        
        ta -> add ( mat ) ;
      }
      
      ssgTransform* tr = new ssgTransform ;
      tr = current_options -> createTransform ( tr, ta ) ;
      tr -> addKid ( mesh_entity ) ;
      mesh_entity = tr ;
    }

    parent_branch -> addKid ( mesh_entity ) ;
    mesh_entity -> setName ( obj->name ) ;
  }
  
  delete obj ;
	return TRUE;
}


static bool parse()
{
  materials = new aseMaterial* [ MAX_MATERIALS ];
  if ( !materials )
  {
    parser.error("not enough memory");
    return false ;
  }
  
  num_materials = 0 ;
  
  first_frame = 0 ;
  last_frame = 0 ;
  frame_speed = 0 ;
  ticks_per_frame = 0 ;
  num_frames = 0 ;
  
  bool firsttime = true;
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getLine( startLevel )) != NULL)
  {
    if (firsttime)
    {
      if (strcmp(token,"*3DSMAX_ASCIIEXPORT"))
      {
        parser.error("not ASE format");
        return false ;
      }
			u32 version_number;
			if (! parser.parseUInt(version_number, "version number") )
        return false ;
      if ( version_number != 200)
      {
        parser.error("invalid %s version",token);
        return false ;
      }
      firsttime = false;
    }
    else if (!strcmp(token,"*SCENE"))
    {
      int startLevel = parser.level;
      while ((token = parser.getLine( startLevel )) != NULL)
      {
        if (!strcmp(token,"*SCENE_FIRSTFRAME"))
        {
          if (! parser.parseUInt(first_frame, "FIRSTFRAME #"))
						return FALSE;
        }
        else if (!strcmp(token,"*SCENE_LASTFRAME"))
        {
          if (! parser.parseUInt(last_frame, "LASTFRAME #"))
						return FALSE;
          num_frames = last_frame - first_frame + 1;
        }
        else if (!strcmp(token,"*SCENE_FRAMESPEED"))
        {
          if (! parser.parseUInt(frame_speed, "FRAMESPEED #"))
						return FALSE;
        }
        else if (!strcmp(token,"*SCENE_TICKSPERFRAME"))
        {
          if (! parser.parseUInt(ticks_per_frame, "TICKSPERFRAME #"))
						return FALSE;
        }
      }
    }
    else if (!strcmp(token,"*MATERIAL_LIST"))
    {
      if (! parse_material_list())
				return FALSE;
    }
    else if (!strcmp(token,"*GEOMOBJECT"))
    {
      if (! parse_object( aseObject::GEOM ))
				return FALSE;
    }
    else if (!strcmp(token,"*HELPEROBJECT"))
    {
      if (! parse_object( aseObject::HELPER ))
				return FALSE;
    }
    else if (!strcmp(token,"*CAMERAOBJECT"))
    {
      if (! parse_object( aseObject::CAMERA ))
				return FALSE;
    }
  }
  return true ;
}


static void parse_free()
{
  u32 i ;
  for ( i = 0 ; i < num_materials ; i++ )
  {
    delete [] materials[ i ] -> name ;
    delete [] materials[ i ] -> tfname ;
    delete materials[ i ] ;
  }
  delete[] materials ;
  materials = 0 ;
}


ssgEntity *ssgLoadASE ( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  top_branch = new ssgBranch ;
  if ( !parser.openFile( fname, &parser_spec ) )
  {
    delete top_branch ;
		return 0;
  }
  if ( !parse() )
  {
    delete top_branch ;
    top_branch = 0 ;
  }
  parse_free();
  parser.closeFile();
  
  return top_branch ;
}
