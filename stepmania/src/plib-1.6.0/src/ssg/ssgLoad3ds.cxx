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

/*******************************************************
 **  ssgLoad3ds.cxx
 **  
 **  Written by Per Liedman (liedman@home.se)
 **  Last updated: 2001-02-09
 **
 **  This was written to be a part of Stephen J Bakers
 **  PLIB (http://plib.sourceforge.net)
 *******************************************************/

/* KNOWN ISSUES:

  * Some models (one of the test cases) gets turned "inside out" - obviously 
    the triangle winding has been reversed so that all triangles that should be
    culled are shown and vice versa. I don't know where this information can
    be found in the file.

  * Models that uses double-sided materials sometimes look strange. My approach
    to double sided materials is to add a copy of each triangle with the 
    winding reversed and also negate the normals...this seems to *almost* work.

  * The transformation matrix in the 3DS files is still a mystery to me -
    whatever I do with it seems to make things worse than ignoring it, although
    that doesn't work perfect either. I've taken a look at other loaders, and
    it seems that no one really knows what to do with them.

*/

#define USE_VTXARRAYS

#include "ssgLocal.h"
#include "ssg3ds.h"

#define MAX_MATERIALS 512

#define PARSE_OK 1
#define PARSE_ERROR 0
#define CHUNK_HEADER_SIZE (2 + 4)

#define IS_DOUBLESIDED 1

/* Define DEBUG if you want debug output
   (this might be a nice way of looking at the
   structure of a 3DS file). */
//#define DEBUG 1


#ifdef DEBUG
#define DEBUGPRINT(f, x, y, z) ulSetError(UL_DEBUG, f, debug_indent, x, y, z)
#else
#define DEBUGPRINT(f, x, y, z)
#endif

#ifdef DEBUG
static char debug_indent[256];
#endif

/* this is the minimum value of the dot product for
   to faces if their normals should be smoothed, if
   they don't use smooth groups. */
static const float _ssg_smooth_threshold = 0.8f;

// parsing functions for chunks that need separate treatment.
static int parse_material( unsigned int length);
static int parse_objblock( unsigned int length);
static int parse_rgb1( unsigned int length);
static int parse_rgb2( unsigned int length);
static int parse_material_name( unsigned int length);
static int parse_ambient( unsigned int length);
static int parse_diffuse( unsigned int length);
static int parse_specular( unsigned int length);
static int parse_shininess( unsigned int length);
static int parse_transparency( unsigned int length);
static int parse_doublesided( unsigned int length);
static int parse_vert_list( unsigned int length);
static int parse_face_list( unsigned int length);
static int parse_map_list( unsigned int length);
static int parse_tra_matrix( unsigned int length);
static int parse_trimesh( unsigned int length);
static int parse_smooth_list( unsigned int length);
static int parse_face_materials( unsigned int length);
static int parse_mapname( unsigned int length);
static int parse_mapoptions( unsigned int length);
static int parse_uscale( unsigned int length);
static int parse_vscale( unsigned int length);
static int parse_uoffst( unsigned int length);
static int parse_voffst( unsigned int length);
static int parse_oneunit( unsigned int length);
static int parse_version( unsigned int length);
static int parse_frame  ( unsigned int length);
static int parse_frame_objname  ( unsigned int length);
static int parse_frame_hierarchy( unsigned int length);
static int identify_face_materials( unsigned int length );

/* _ssg3dsChunk defines how a certain chunk is handled when encountered -
   what subchunks it might have and what parse function should be used
   for it. */
struct _ssg3dsChunk {
  unsigned short id;
  _ssg3dsChunk *subchunks;
  int (*parse_func) ( unsigned int );
};

// following arrays define the structure of the chunks in the 3ds file.
static _ssg3dsChunk FaceListDataChunks[] =
{ { CHUNK_SMOOLIST, NULL, parse_smooth_list             },
  { CHUNK_FACEMAT, NULL,  identify_face_materials       },
  { 0, NULL, NULL }
};

static _ssg3dsChunk FaceListChunks[] =
{ { CHUNK_FACEMAT, NULL, parse_face_materials           },
  { 0, NULL, NULL }
};

static _ssg3dsChunk TriMeshDataChunks[] =
{ { CHUNK_VERTLIST, NULL, parse_vert_list               },
  { CHUNK_MAPLIST, NULL, parse_map_list                 },
  { 0, NULL, NULL }
};  /* these chunks have to be known *before* we call parse_face_list
       (see parse_trimesh for more info) */

static _ssg3dsChunk TriMeshChunks[] =
{ { CHUNK_FACELIST, FaceListChunks, parse_face_list     },
  { CHUNK_TRMATRIX, NULL, parse_tra_matrix              },
  { 0, NULL, NULL }
};

static _ssg3dsChunk ObjBlockChunks[] =
{ { CHUNK_TRIMESH, TriMeshChunks, parse_trimesh         },
  { 0, NULL, NULL }
};

static _ssg3dsChunk TextureChunks[] =
{ { CHUNK_MAPFILENAME, NULL, parse_mapname              },
  { CHUNK_MAP_USCALE, NULL, parse_uscale                },
  { CHUNK_MAP_VSCALE, NULL, parse_vscale                },
  { CHUNK_MAP_UOFFST, NULL, parse_uoffst                },
  { CHUNK_MAP_VOFFST, NULL, parse_voffst                },
  { CHUNK_MAPOPTIONS, NULL, parse_mapoptions            },
  { 0, NULL, NULL }
};

static _ssg3dsChunk MaterialColourChunks[] =
{ { CHUNK_RGB1, NULL, parse_rgb1                        },
  { CHUNK_RGB2, NULL, parse_rgb2                        },
  { CHUNK_RGB3, NULL, parse_rgb2                        },
  { 0, NULL, NULL }
};

static _ssg3dsChunk MaterialChunks[] =
{ { CHUNK_MATNAME, NULL, parse_material_name            },
  { CHUNK_AMBIENT, MaterialColourChunks, parse_ambient  },
  { CHUNK_DIFFUSE, MaterialColourChunks, parse_diffuse  },
  { CHUNK_SPECULAR, MaterialColourChunks, parse_specular},
  { CHUNK_SHINE_STRENGTH, NULL, parse_shininess         },
  { CHUNK_TRANSPARENCY, NULL, parse_transparency        },
  { CHUNK_TEXTURE, TextureChunks, NULL                  },
  { CHUNK_DOUBLESIDED, NULL, parse_doublesided          },
  { 0, NULL, NULL }
};

static _ssg3dsChunk ObjMeshChunks[] =
{ { CHUNK_MATERIAL, MaterialChunks, parse_material      },
  { CHUNK_OBJBLOCK, ObjBlockChunks, parse_objblock      },
  { CHUNK_ONEUNIT,  NULL,           parse_oneunit       },
  { 0, NULL, NULL }
};

static _ssg3dsChunk FrameChunks[] =
{ { CHUNK_FRAME_OBJNAME  , NULL, parse_frame_objname    },
  { CHUNK_FRAME_HIERARCHY, NULL, parse_frame_hierarchy  },
//    { CHUNK_FRAME_ROTATION , NULL, parse_frame_rotation   },
//    { CHUNK_FRAME_POSITION , NULL, parse_frame_position   },
//    { CHUNK_FRAME_SCALE    , NULL, parse_frame_scale      },
  { 0, NULL, NULL }
};

static _ssg3dsChunk KeyframerChunks[] =
{ { CHUNK_FRAMES       , NULL       , parse_frame       },
  { CHUNK_KEYFRAME_MESH, FrameChunks, NULL              },
  { 0, NULL, NULL }
};

static _ssg3dsChunk MainChunks[] =
{ { CHUNK_OBJMESH,   ObjMeshChunks  , NULL              },
  { CHUNK_KEYFRAMER, KeyframerChunks, NULL              },
  { CHUNK_VERSION, NULL,    parse_version               },
  { 0, NULL, NULL }
};

static _ssg3dsChunk TopChunk[] = 
{ { CHUNK_MAIN, MainChunks, NULL                        },
  { 0, NULL, NULL }
};

/* The material properties are temporarily stored in this structure before
   creating ssgSimpleStates out of them. */
struct _3dsMat {
  char *name ;
  int flags;
  sgVec3 colour[4];
  float shi, alpha;
  
  char *tex_name;
  sgVec2 tex_scale, tex_offset;
  bool wrap_s, wrap_t;
};
/* 
   These are the indices used for identifying the materials colour-properties: 
*/   
#define _3DSMAT_AMB 0
#define _3DSMAT_DIF 1
#define _3DSMAT_EMI 2
#define _3DSMAT_SPE 3


/* Some 3ds files does not have any materials defined. This material is
   used in that case: */
static _3dsMat default_material= { "ssgLoad3ds default material",
				   0,
				   { { 1.0f, 1.0f, 1.0f },
				     { 1.0f, 1.0f, 1.0f },
				     { 0.0f, 0.0f, 0.0f },
				     { 0.0f, 0.0f, 0.0f },
				   },
				   0.0f, 1.0f,
				   NULL,
				   {1.0f, 1.0f}, {0.0f, 0.0f},
				   false, false };

/* A _ssg3dsStructureNode holds a mesh or a transformation node. All
   geometry is collected into structure nodes before actually being assembled
   into the scene-graph, since the hierarchy information is at the end of the
   3ds file. */
struct _ssg3dsStructureNode {
  _ssg3dsStructureNode() {
    id            = -1;
    object        = NULL;
    has_been_used = false;
    next          = NULL;
  }

  short id;
  ssgBranch *object;
  bool has_been_used;
  _ssg3dsStructureNode *next;
};

static _ssg3dsStructureNode *findStructureNode( char  *name );
static _ssg3dsStructureNode *findStructureNode( short id    );

static int  parse_chunks( _ssg3dsChunk *chunk_list, unsigned int length);
static void add_leaf( _3dsMat *material, int listed_faces, 
		      unsigned short *face_indices );

/* Each vertex in a mesh has a face list associated with it, which
   contains all faces that use this vertex. This list is used when
   smoothing normals, since the adjacent faces of each vertex is needed. */
struct _ssg3dsFaceList {
  int             face_index;
  _ssg3dsFaceList *next;
};

static _ssg3dsFaceList *addFaceListEntry( _ssg3dsFaceList *face_list, 
					  int face_index ) {
  _ssg3dsFaceList *new_entry = new _ssg3dsFaceList;
  new_entry -> face_index = face_index;
  new_entry -> next = face_list;    

  return new_entry;
}

static void freeFaceList( _ssg3dsFaceList *face_list ) {
  for (_ssg3dsFaceList *i = face_list, *temp = NULL; i != NULL; i = temp) {
    temp = i -> next;
    delete i;
  }
}

// globals
static FILE *model;

static int num_objects, num_materials, num_textures;
static int double_sided;     // is there some double sided material?

static ssgBranch *top_object, *current_branch;

static ssgLoaderOptions* current_options = NULL ;

static _3dsMat **materials, *current_material;

static unsigned short *vertex_index, *normal_index, num_vertices, num_faces;
static unsigned int  *smooth_list;
static _ssg3dsFaceList **face_lists;

static ssgTransform *current_transform;

static sgVec3 *vertex_list;
static sgVec3 *face_normals, *vertex_normals;
static sgVec2 *texcrd_list;
static int smooth_found, facemat_found;

static int colour_mode;

static _ssg3dsStructureNode *object_list            = NULL;
static short current_structure_id                   = -1;

// convenient functions
static unsigned char get_byte() {
  unsigned char b;
  fread( &b, 1, 1, model );
  return b;
}

/* NOTE: This string has to be freed by the caller
   Also note: You can't fetch strings longer than 256
   characters! */
static char* get_string() {
  char *s = new char[256], read;
  int c = 0;

  while ( (read = getc(model)) != 0 ) {
    if (c < 255)
      s[c++] = read;
  }
  s[c] = 0;

  return s;
}

//==========================================================
// STRUCTURE NODE FUNCTIONS

static _ssg3dsStructureNode *findStructureNode( char *name ) {
  for (   _ssg3dsStructureNode *n = object_list; n != NULL; n = n->next ) {
    if ( strcmp( n->object->getName(), name ) == 0 ) {
      return n;
    }
  }

  return NULL;
}

static _ssg3dsStructureNode *findStructureNode( short id ) {
  for ( _ssg3dsStructureNode *n = object_list; n != NULL; n = n->next ) {
    if ( n->id == id ) {
      return n;
    }
  }

  return NULL;
}

static void addStructureNode( _ssg3dsStructureNode *node ) {
  if (object_list == NULL) {
    object_list = node;
  } else {
    node -> next = object_list;
    object_list = node;
  }
}

//==========================================================
// MATERIAL PARSERS
static int parse_mapname( unsigned int length )
{
  current_material->tex_name = get_string();
  DEBUGPRINT("%sMap name: %s %s%s", current_material->tex_name, "", "");
  return PARSE_OK;
}

static int parse_mapoptions( unsigned int length )
{
  unsigned short value = ulEndianReadLittle16(model);
  // bit 4: 0=tile (default), 1=do not tile (a single bit for both u and v)
  current_material->wrap_s = current_material->wrap_t = ((value & 0x10) == 0);
  DEBUGPRINT("%sMap options (wrap): %c %s%s", 
	     (current_material->wrap_s)?'Y':'N', "", "");

  return PARSE_OK;
}

static int parse_uscale( unsigned int length )
{
  current_material->tex_scale[1] = ulEndianReadLittleFloat(model);
  DEBUGPRINT("%sU-scale: %.3f %s%s", current_material->tex_scale[1], "", "");
  return PARSE_OK;
}

static int parse_vscale( unsigned int length )
{
  current_material->tex_scale[0] = ulEndianReadLittleFloat(model);
  DEBUGPRINT("%sV-scale: %.3f %s%s", current_material->tex_scale[0], "", "");
  return PARSE_OK;
}

static int parse_uoffst( unsigned int length )
{
  current_material->tex_offset[1] = ulEndianReadLittleFloat(model);
  DEBUGPRINT("%sU-offset: %.3f %s%s",
	     current_material->tex_offset[1], "", "");
  return PARSE_OK;
}

static int parse_voffst( unsigned int length )
{
  current_material->tex_offset[0] = ulEndianReadLittleFloat(model);
  DEBUGPRINT("%sV-offset: %.3f %s%s",
	     current_material->tex_offset[0], "", "");
  return PARSE_OK;
}

static int parse_material( unsigned int length ) {
  materials[num_materials] = current_material = new _3dsMat;
  current_material->flags = 0;
  current_material->tex_name = NULL;
  num_materials++;

  /* set default value for material colours (taken from glMaterial man page) */
  sgSetVec3(current_material -> colour[_3DSMAT_AMB], 0.2f, 0.2f, 0.2f );
  sgSetVec3(current_material -> colour[_3DSMAT_DIF], 0.8f, 0.8f, 0.8f );
  sgSetVec3(current_material -> colour[_3DSMAT_SPE], 0.0f, 0.0f, 0.0f );
  sgSetVec3(current_material -> colour[_3DSMAT_EMI], 0.0f, 0.0f, 0.0f );
  current_material -> shi = 0.0f;
  current_material -> alpha = 1.0f;

  /* set up texture info */
  sgSetVec2(current_material -> tex_scale , 1.0f, 1.0f);
  sgSetVec2(current_material -> tex_offset, 0.0f, 0.0f);
  current_material -> wrap_s = current_material -> wrap_t = true;
  
  DEBUGPRINT("%sNew material found.%s%s%s", "", "", "");
  return PARSE_OK;
}

static int parse_material_name( unsigned int length ) {
  current_material -> name = get_string();
  DEBUGPRINT("%sMaterial name:%s%s%s", current_material->name, "", "");
  
  return PARSE_OK;
}

static int parse_rgb1( unsigned int length ) {
  float r, g, b;

  r = ulEndianReadLittleFloat(model);
  g = ulEndianReadLittleFloat(model);
  b = ulEndianReadLittleFloat(model);
  DEBUGPRINT("%sColour: R:%.2f, G:%.2f, B:%.2f", r, g, b);

  sgSetVec3(current_material->colour[colour_mode], r, g, b);

  return PARSE_OK;
}

static int parse_rgb2( unsigned int length ) {
  float r, g, b;

  r = (float)get_byte() / 255.0f;
  g = (float)get_byte() / 255.0f;
  b = (float)get_byte() / 255.0f;
  DEBUGPRINT("%sColour: R:%.2f, G:%.2f, B:%.2f", r, g, b);

  sgSetVec3(current_material->colour[colour_mode], r, g, b);
  return PARSE_OK;
}

static int parse_ambient( unsigned int length ) {
  colour_mode = _3DSMAT_AMB;
  DEBUGPRINT("%sAmbient colour%s%s%s", "", "", "");
  return PARSE_OK;
}

static int parse_diffuse( unsigned int length ) {
  colour_mode = _3DSMAT_DIF;
  DEBUGPRINT("%sDiffuse colour%s%s%s", "", "", "");
  return PARSE_OK;
}

static int parse_specular( unsigned int length ) {
  colour_mode = _3DSMAT_SPE;
  DEBUGPRINT("%sSpecular colour%s%s%s", "", "", "");
  return PARSE_OK;
}

static int parse_shininess( unsigned int length ) {
  // this chunk contains a percentage chunk,
  // so just read that chunks header
  ulEndianReadLittle16(model); ulEndianReadLittle32(model);
  current_material -> shi = (float)ulEndianReadLittle16(model) * 128.0f / 100.0f;
  DEBUGPRINT("%sShininess:%.1f%s%s", current_material->shi, "", "");
  return PARSE_OK;
}

static int parse_transparency( unsigned int length ) {
  // this chunk contains a percentage chunk,
  // so just read that chunks header
  ulEndianReadLittle16(model); ulEndianReadLittle32(model);
  current_material->alpha = 1.0f - (float)ulEndianReadLittle16(model) / 100.0f;
  DEBUGPRINT("%sAlpha:%.1f%s%s", current_material->alpha, "", "");
  return PARSE_OK;
}

static int parse_doublesided( unsigned int length ) {
  double_sided = current_material->flags |= IS_DOUBLESIDED;

  DEBUGPRINT("%sMaterial is double sided.%s%s%s", "", "", "");

  return PARSE_OK;
}

static ssgSimpleState *get_state( _3dsMat *mat ) {

  if ( mat -> name != NULL )
  {
    ssgSimpleState *st = current_options -> createSimpleState ( mat -> name ) ;
    if ( st != NULL )
      return st ;
  }

  ssgSimpleState *st = new ssgSimpleState () ;

  st -> setName( mat -> name );

  st -> setMaterial ( GL_AMBIENT, 
		      mat->colour[_3DSMAT_AMB][0], 
		      mat->colour[_3DSMAT_AMB][1], 
		      mat->colour[_3DSMAT_AMB][2], mat->alpha ) ;
  st -> setMaterial ( GL_DIFFUSE,
		      mat->colour[_3DSMAT_DIF][0], 
		      mat->colour[_3DSMAT_DIF][1], 
		      mat->colour[_3DSMAT_DIF][2], mat->alpha ) ;
  st -> setMaterial ( GL_SPECULAR, 
		      mat->colour[_3DSMAT_SPE][0], 
		      mat->colour[_3DSMAT_SPE][1], 
		      mat->colour[_3DSMAT_SPE][2], mat->alpha ) ;
  st -> setMaterial ( GL_EMISSION, 
		      mat->colour[_3DSMAT_EMI][0], 
		      mat->colour[_3DSMAT_EMI][1], 
		      mat->colour[_3DSMAT_EMI][2], mat->alpha ) ;
  st -> setShininess( mat -> shi ) ;

  st -> disable ( GL_COLOR_MATERIAL ) ;
  st -> enable  ( GL_LIGHTING       ) ;
  st -> setShadeModel ( GL_SMOOTH ) ;

  if ( mat -> alpha < 0.99f )
  {
    st -> disable ( GL_ALPHA_TEST ) ;
    st -> enable  ( GL_BLEND ) ;
    st -> setTranslucent () ;
  }
  else
  {
    st -> disable ( GL_BLEND ) ;
    st -> setOpaque () ;
  }

  if (mat -> tex_name != NULL) {
    st -> setTexture( current_options -> createTexture(mat->tex_name,
						       mat->wrap_s, 
						       mat->wrap_t) ) ;
    st -> enable( GL_TEXTURE_2D );
  } else {
    st -> disable( GL_TEXTURE_2D );
  }

  return st ;
}


//==========================================================
// TRIMESH PARSERS

static void free_trimesh()
{
  DEBUGPRINT("%sFreeing trimesh object%s%s%s", "","","");

  if (vertex_list)
    delete [] vertex_list;

  if (face_normals)
    delete [] face_normals;

  if (vertex_normals)
    delete [] vertex_normals;

  if (texcrd_list)
    delete [] texcrd_list;

  if (smooth_list)
    delete [] smooth_list;

  if (vertex_index)
    delete [] vertex_index;

  if (face_lists) {
    for (int i = 0; i < num_vertices; i++) {
      freeFaceList( face_lists[i] );
    }

    delete [] face_lists;
  }

  vertex_list    = NULL;
  face_normals   = NULL;
  vertex_normals = NULL;
  texcrd_list    = NULL;
  smooth_list    = NULL;
  vertex_index   = NULL;
  face_lists     = NULL;
}

static int parse_trimesh( unsigned int length ) {
  current_transform = new ssgTransform();

  free_trimesh();

  current_branch -> addKid( current_transform );

  /* Before we parse CHUNK_FACEMAT, we have to know vertices and texture
     coordinates. To ensure this, we make a special pass of the Trimesh
     chunks, just extracting this information.
     This is kind of a kludge, but it was the easiest way to solve this problem
  */
  DEBUGPRINT("%sPrescanning sub-chunks for vertices and texture coords." \
	     "%s%s%s", "","","");
#ifdef DEBUG
  strcat(debug_indent, "    ");
#endif

  unsigned long p = ftell(model);
  int parse_ok = parse_chunks( TriMeshDataChunks, length );
  fseek(model, p, SEEK_SET);

#ifdef DEBUG
  debug_indent[strlen(debug_indent)-4] = 0;
#endif
  DEBUGPRINT("%sDone prescanning.%s%s%s", "","","");

  return parse_ok;
}

static int parse_vert_list( unsigned int length ) {
  num_vertices = ulEndianReadLittle16(model);
  vertex_list = new sgVec3[num_vertices];
  face_lists = new _ssg3dsFaceList*[num_vertices];

  DEBUGPRINT("%sReading %d vertices.%s%s", num_vertices, "", "");

  for (int i = 0; i < num_vertices; i++) {
    vertex_list[i][0] = ulEndianReadLittleFloat(model);
    vertex_list[i][1] = ulEndianReadLittleFloat(model);
    vertex_list[i][2] = ulEndianReadLittleFloat(model);

    face_lists [i]    = NULL;
  }

  return PARSE_OK;
}

static int parse_smooth_list( unsigned int length )
{
  int i;
  smooth_found = TRUE;

  smooth_list = new unsigned int[num_faces];
  DEBUGPRINT("%sReading smoothlist%s%s%s", "", "", "");

  for (i = 0; i < num_faces; i++)
    smooth_list[i] = ulEndianReadLittle32(model);

  return PARSE_OK;
}

static void smooth_normals( int use_smooth_list ) {
  for (int i = 0; i < num_faces; i++) {
    for (int j = 0; j < 3; j++) {
      int nindex = i * 3 +j;
      int vindex = vertex_index[ nindex ];
      sgCopyVec3( vertex_normals[nindex], face_normals[i] );

      // find all faces containing vertex vindex
      for ( _ssg3dsFaceList *l = face_lists[vindex]; l != NULL; l = l->next ) {
	int findex = l -> face_index;

	if ( findex != i ) {
	  int should_smooth;	  
	  if (use_smooth_list) {
	    should_smooth = (smooth_list[i] & smooth_list[findex]);
	  } else {
	    float scalar = sgScalarProductVec3( face_normals[i], 
						face_normals[findex] );
	    should_smooth = ( scalar > _ssg_smooth_threshold );
	  }
	  
	  if (should_smooth) {
	    sgAddVec3( vertex_normals[nindex], face_normals[findex] );
	  }
	}
      }

      sgNormaliseVec3( vertex_normals[nindex] );
    }
  }
}

static int identify_face_materials( unsigned int length ) {
  facemat_found = TRUE;
  DEBUGPRINT("%sFace materials found.%s%s%s", "","","");

  fseek( model, length, SEEK_CUR );

  return PARSE_OK;
}

static int parse_face_list( unsigned int length ) {
  int i;
  num_faces = ulEndianReadLittle16(model);

  DEBUGPRINT("%sReading %d faces.%s%s", num_faces, "", "");

  vertex_index   = new unsigned short[num_faces*3];
  face_normals   = new sgVec3[num_faces];

  vertex_normals = new sgVec3[num_faces * 3];

  for (i = 0; i < num_faces; i++) {
    int v1 = ulEndianReadLittle16(model);
    int v2 = ulEndianReadLittle16(model);
    int v3 = ulEndianReadLittle16(model);
    vertex_index[i*3    ] = v1;
    vertex_index[i*3 + 1] = v2;
    vertex_index[i*3 + 2] = v3;

    face_lists[ v1 ] = addFaceListEntry( face_lists[ v1 ], i );
    face_lists[ v2 ] = addFaceListEntry( face_lists[ v2 ], i );
    face_lists[ v3 ] = addFaceListEntry( face_lists[ v3 ], i );

    unsigned short flags  = ulEndianReadLittle16(model);

    if (flags & 7 == 0) {     // Triangle vertices order should be swapped
      unsigned short tmp    = vertex_index[i*3 + 1];
      vertex_index[i*3 + 1] = vertex_index[i*3 + 2];
      vertex_index[i*3 + 2] = tmp;
    }

    sgMakeNormal( face_normals[i], vertex_list[vertex_index[i*3    ]], 
                  vertex_list[vertex_index[i*3 + 1]], 
                  vertex_list[vertex_index[i*3 + 2]] );

  }

  /* this is a special "hack" for the face list chunk
     because we HAVE to know the smooth list (if there is one)
     before building the ssgVtxTable objects, this parsing has to
     be done first...*ugh*/

  smooth_found = FALSE;
  facemat_found = FALSE;
  DEBUGPRINT("%sPrescanning sub-chunks for smooth list...%s%s%s", "","","");
#ifdef DEBUG
  strcat(debug_indent, "    ");
#endif

  unsigned long p = ftell(model);
  parse_chunks( FaceListDataChunks, length - (2 + 8*num_faces) );
  fseek(model, p, SEEK_SET);

#ifdef DEBUG
  debug_indent[strlen(debug_indent)-4] = 0;
#endif
  DEBUGPRINT("%sDone prescanning.%s%s%s", "","","");

  /* now apply correct smoothing. If smooth list has been found,
     use it, otherwise use threshold value. */
  smooth_normals( 0 /*smooth_found*/ );

  if (!facemat_found) {
    DEBUGPRINT("%sNo CHUNK_FACEMAT found. Adding default faces of material " \
	       "\"%s\"%s%s.", materials[0]->name, "", "");
    unsigned short *face_indices = new unsigned short[num_faces];
    for (i = 0; i < num_faces; i++) {
      face_indices[i] = i;
    }
    add_leaf(materials[0], num_faces, face_indices);
  }

  return PARSE_OK;
}

static int parse_map_list( unsigned int length ) {
  unsigned short num_v = ulEndianReadLittle16(model);
  texcrd_list = new sgVec2[num_v];

  DEBUGPRINT("%sReading %d texture coords.%s%s", num_v, "", "");

  for (int i = 0; i < num_v; i++) {
    texcrd_list[i][0] = ulEndianReadLittleFloat(model);
    texcrd_list[i][1] = ulEndianReadLittleFloat(model);
  }

  return PARSE_OK;
}

static int parse_tra_matrix( unsigned int length ) {
  sgMat4 m;

  sgMakeIdentMat4( m );

  DEBUGPRINT("%sReading transformation matrix.%s%s%s", "","","");

  /* Strange things seems to be going on with the
     local coordinate system in 3ds - I have commented
     this out, but things seems to work better without
     it (which is odd).*/
  
  int i, j;
  
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 3; j++) {
      m[j][i] = ulEndianReadLittleFloat(model);
    }
  }
  
  m[3][3] = 1.0f;
  sgTransposeNegateMat4( m );
    
#ifdef DEBUG
  for (int a = 0; a < 4; a++) {
    fputs(debug_indent, stderr);
    for (int b = 0; b < 4; b++) {
      fprintf(stderr, "%.2f\t", m[b][a]);
    }
   putc('\n', stderr);
  }
#endif  

  //current_transform -> setTransform( m );
  
  return PARSE_OK;
}

static void add_leaf( _3dsMat *material, int listed_faces, 
		      unsigned short *face_indices ) {
  int is_ds          = material->flags & IS_DOUBLESIDED;
  int has_texture    = material->tex_name != NULL;
  int flip_texture_y = FALSE;
  ssgVertexArray   *vertices = new ssgVertexArray();
  ssgNormalArray   *normals  = new ssgNormalArray();
  ssgTexCoordArray *texcrds  = NULL;
#ifdef USE_VTXARRAYS
  ssgIndexArray* indices = new ssgIndexArray();
#endif
  
  if (has_texture) {
    if (texcrd_list == NULL) {
      ulSetError(UL_WARNING, "ssgLoad3ds: Texture coords missing.");
    } else {
      texcrds = new ssgTexCoordArray();

      /* flip textures y-coord if texture is a BMP */
      char *texture_extension = 
	material->tex_name + strlen(material->tex_name) - 3;
      
      flip_texture_y = ulStrEqual( texture_extension, "BMP" );
    }
  }

  int tri_idx = 0;

  for (int i = 0; i < listed_faces; i++) {
    unsigned short faceindex = face_indices[i];
    int v1 = faceindex * 3,
      v2 = faceindex * 3 + 1,
      v3 = faceindex * 3 + 2;

#ifdef USE_VTXARRAYS
    indices->add( tri_idx++ );
    indices->add( tri_idx++ );
    indices->add( tri_idx++ );
#endif

    vertices->add( vertex_list[ vertex_index[v1] ] );
    vertices->add( vertex_list[ vertex_index[v2] ] );
    vertices->add( vertex_list[ vertex_index[v3] ] );

    normals ->add( vertex_normals[ v1 ] );
    normals ->add( vertex_normals[ v2 ] );
    normals ->add( vertex_normals[ v3 ] );

    if (has_texture && texcrd_list != NULL) {
      int num_texcrds = 3;
      sgVec2 _texcrds[6];
      sgCopyVec2( _texcrds[0], texcrd_list[ vertex_index[v1] ] );
      sgCopyVec2( _texcrds[1], texcrd_list[ vertex_index[v2] ] );
      sgCopyVec2( _texcrds[2], texcrd_list[ vertex_index[v3] ] );
      if (is_ds) {
        num_texcrds = 6;
	sgCopyVec2( _texcrds[3], texcrd_list[ vertex_index[v1] ] );
	sgCopyVec2( _texcrds[4], texcrd_list[ vertex_index[v3] ] );
	sgCopyVec2( _texcrds[5], texcrd_list[ vertex_index[v2] ] );
      }

      for (int j = 0; j < num_texcrds; j++) {
        _texcrds[j][0] *= material->tex_scale[0];
        _texcrds[j][1] *= material->tex_scale[1];

	if (flip_texture_y) {
	  _texcrds[j][1] = 1.0f - _texcrds[j][1];
	}

        sgAddVec2( _texcrds[j], material->tex_offset );
	texcrds->add( _texcrds[j] );
      }
    }

    if (is_ds) {
      sgVec3 n[3];  /* we have to use the *negated* normals for back faces */

      vertices->add( vertex_list[ vertex_index[v1] ] );
      vertices->add( vertex_list[ vertex_index[v3] ] );
      vertices->add( vertex_list[ vertex_index[v2] ] );

      sgCopyVec3( n[0], vertex_normals[v1] );
      sgCopyVec3( n[1], vertex_normals[v3] );
      sgCopyVec3( n[2], vertex_normals[v2] );

      for (int j = 0; j < 3; j++) {
	sgNegateVec3( n[j] );
	normals->add( n[j] );
      }

#ifdef USE_VTXARRAYS
      indices->add( tri_idx++ );
      indices->add( tri_idx++ );
      indices->add( tri_idx++ );
#endif
    }

  }

#ifdef USE_VTXARRAYS
  ssgVtxArray* vtab = new ssgVtxArray ( GL_TRIANGLES, vertices, normals, texcrds, NULL, indices );
#else
  ssgVtxTable* vtab = new ssgVtxTable ( GL_TRIANGLES, vertices, normals, texcrds, NULL ) ;
#endif
  vtab -> setState ( get_state( material ) ) ;
  vtab -> setCullFace ( TRUE ) ;

  ssgLeaf* leaf = current_options -> createLeaf ( vtab, 0 ) ;

  if ( leaf )
    current_transform -> addKid( leaf );
}

static int parse_face_materials( unsigned int length ) {
  int mat_num;
  char *mat_name = get_string();
  _3dsMat *material = NULL;

  // find the material
  for (mat_num = 0; mat_num < num_materials; mat_num++) {
    if ( strcmp( mat_name, materials[mat_num]->name ) == 0 ) {
      material = materials[mat_num];
      break;
    }
  }

  if (material == NULL) {
    ulSetError(UL_WARNING, "ssgLoad3ds: Undefined reference to material " \
	    "\"%s\" found.", mat_name);
    return PARSE_ERROR;
  }

  unsigned short listed_faces = ulEndianReadLittle16(model);

  DEBUGPRINT("%sFaces of \"%s\" list with %d faces.%s", 
	     mat_name, listed_faces, "");

  delete mat_name;  // no longer needed
  
  unsigned short *face_indices = new unsigned short[listed_faces];
  for (int i = 0; i < listed_faces; i++) {
    face_indices[i] = ulEndianReadLittle16(model);
  }

  add_leaf(material, listed_faces, face_indices);

  delete [] face_indices;

  return PARSE_OK;
}

//==========================================================
// OBJBLOCK PARSER

static int parse_objblock( unsigned int length ) {
  char *object_name    = get_string();
  current_branch = new ssgTransform;
  current_branch -> setName( object_name );
  
  _ssg3dsStructureNode *object_node = new _ssg3dsStructureNode;
  object_node -> object = current_branch;

  addStructureNode( object_node );

  DEBUGPRINT("%sObject block \"%s\"%s%s", object_name, "", "");
  delete object_name;

  return PARSE_OK;
}

static int parse_oneunit( unsigned int length ) {
#ifdef DEBUG
  float oneunit = ulEndianReadLittleFloat(model);
  DEBUGPRINT("%sOne unit: %.3f%s%s", oneunit, "", "");
#else
  ulEndianReadLittleFloat(model) ;
#endif

  return PARSE_OK;
}

static int parse_version( unsigned int length ) {
#ifdef DEBUG
  unsigned int version = ulEndianReadLittle32(model);
  DEBUGPRINT("%s3DS Version: %d%s%s", version, "", "");
#else
  ulEndianReadLittle32(model) ;
#endif

  return PARSE_OK;
}

//==========================================================
// KEYFRAME CHUNK PARSER

static int parse_frame( unsigned int length ) {
  // this chunk is not used for anything right now
#ifdef DEBUG
  DEBUGPRINT("%sFrame start: %d, end: %d%s",
	     ulEndianReadLittle32(model), ulEndianReadLittle32(model), "");
#else
  ulEndianReadLittle32(model);
  ulEndianReadLittle32(model);
#endif

  return PARSE_OK;
}

static int parse_frame_objname( unsigned int length ) {
  /* This chunk defines a hierarchy elements name and its parent object's
     identifier. 

     This function assumes that the hierarchy is defined from
     root to leaf, i.e. a nodes parent must have been declared before the
     node itself is declared. */

  char *objname = get_string();
  ulEndianReadLittle16(model);
  ulEndianReadLittle16(model);
  short parent_id = ulEndianReadLittle16(model);

  DEBUGPRINT("%sObject name: %s, parent: %d%s",
	     objname, parent_id, "");

  _ssg3dsStructureNode *current_structure_node = findStructureNode( objname );

  if ( current_structure_node == NULL ) {
    current_structure_node = new _ssg3dsStructureNode;
    current_structure_node -> object = new ssgTransform;
    current_structure_node -> object -> setName( objname );

    addStructureNode( current_structure_node );
  }

  current_structure_node -> id = current_structure_id;
  
  if ( parent_id != -1 ) {
    _ssg3dsStructureNode *parent = findStructureNode( parent_id );
    if (parent == NULL) {
      ulSetError( UL_WARNING, "ssgLoad3ds: Hierarchy entry \"%d\" does "\
		  "not match any defined objects.", parent_id );      
    } else {
      parent -> object -> addKid( current_structure_node -> object );
      current_structure_node -> has_been_used = true;
    }
  } else {
    top_object -> addKid( current_structure_node -> object );
    current_structure_node -> has_been_used = true;
  }

  delete objname;

  return PARSE_OK;
}

static int parse_frame_hierarchy( unsigned int length ) {
  current_structure_id = ulEndianReadLittle16(model);
  DEBUGPRINT("%sThis object's hierarchy id: %d.%s%s", 
	     current_structure_id, "", "");

  return PARSE_OK;
}

//==========================================================
// GENERAL CHUNK PARSER

static int parse_chunks( _ssg3dsChunk *chunk_list, unsigned int length )
{
  int parse_ok = PARSE_OK;
  unsigned short id;
  unsigned int sub_length;
  unsigned int p = 0;
  _ssg3dsChunk *t;

  while (parse_ok && p < length) {
    id = ulEndianReadLittle16(model);
    sub_length = ulEndianReadLittle32(model);

    if (p + sub_length > length) {
      ulSetError(UL_WARNING, "ssgLoad3ds: Illegal chunk %X of length %i. " \
		 "Chunk is longer than parent chunk.", (int)id, sub_length);
      return PARSE_ERROR;
    }

    p += sub_length;
    sub_length -= CHUNK_HEADER_SIZE;

    for (t = chunk_list; t->id != 0 && t->id != id; t++);

    if (t->id == id) {
      DEBUGPRINT("%sFound chunk %X of length %d%s (known)", 
		 id, sub_length,"");
      // this is a known chunk
      // do chunk-specific parsing if available
      unsigned long cp = ftell(model);

      if (t->parse_func)
        parse_ok = t->parse_func( sub_length );

#ifdef DEBUG
      strcat(debug_indent, "    ");
#endif

      // if chunk can have subchunks, parse these
      if (t->subchunks && parse_ok) {
        parse_ok = parse_chunks( t->subchunks, 
				 sub_length - (ftell(model)-cp) );
      }

#ifdef DEBUG
      debug_indent[strlen(debug_indent)-4] = 0;
#endif

    } else {
      DEBUGPRINT("%sFound chunk %X of length %d%s (unknown)", 
		 id, sub_length,"");
      fseek( model, sub_length, SEEK_CUR );
    }

  }

  return parse_ok;
}


ssgEntity *ssgLoad3ds( const char *filename, const ssgLoaderOptions* options ) {
  int i;
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  char filepath [ 1024 ] ;
  current_options -> makeModelPath ( filepath, filename ) ;
  
  model = fopen ( filepath, "rb" );
  if ( model == NULL ) {
    ulSetError(UL_WARNING, "ssgLoad3ds: Failed to open '%s' for reading", 
	       filepath ) ;
    return NULL ;
  }

  fseek(model, 0, SEEK_END);
  unsigned long size = ftell(model);
  rewind(model);

  num_objects  = num_materials = num_textures = 0;
  object_list  = NULL;
  vertex_list  = NULL;
  texcrd_list  = NULL;
  face_normals = NULL;
  face_lists   = NULL;
  vertex_index = normal_index = NULL;
  top_object = new ssgBranch();

  // initialize some storage room for materials
  // (ok, could be implemented as linked list, but...well I'm lazy)
  materials = new _3dsMat*[MAX_MATERIALS];
  // strange enough, the 3ds file does not have to include any materials,
  // in which case we will use this one.
  materials[0] = & default_material;
  
  parse_chunks( TopChunk, size );

  fclose(model);

  // clean up the materials array
  for (i = 0; i < num_materials; i++) {
    if (materials[i] -> name != NULL) {
      delete [] materials[i] -> name;
    }
    if (materials[i] -> tex_name != NULL) {
      delete [] materials[i] -> tex_name;
    }

    delete materials[i];
  }

  for ( _ssg3dsStructureNode *n = object_list, *temp; n != NULL; n = temp ) {
    if ( !n -> has_been_used ) {
      top_object -> addKid( n -> object );
    }
    temp = n -> next;
    delete n;
  }

  delete [] materials;

  free_trimesh();

  return top_object; 
}


