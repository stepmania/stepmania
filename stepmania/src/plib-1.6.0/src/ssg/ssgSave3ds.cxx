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
 **  ssgSave3ds.cxx
 **  
 **  Written by Per Liedman (liedman@home.se)
 **  Last updated: 2001-02-25
 **
 **  This was written to be a part of Stephen J Bakers
 **  PLIB (http://plib.sourceforge.net)
 *******************************************************/

/* This writer is highly experimental. It has *not* been tested with
   3D Studio. Please note that this writer does not save hierarchal
   information */

#include <stdio.h>
#include "ssgLocal.h"
#include "ssg3ds.h"

static FILE* save_fd;

static const int MAX_MATERIALS = 128;
static ssgSimpleState* mat[MAX_MATERIALS];
static int mat_count;

static void writeChunkHeader( unsigned short id, unsigned int length ) {
  ulEndianWriteLittle16( save_fd, id     );
  ulEndianWriteLittle32( save_fd, length );
}

class _ssgSave3dsData {
public:
  _ssgSave3dsData( void* ptr, size_t memb_size, size_t nmemb ) :
    swabbed(false), memb_size(memb_size), nmemb(nmemb), ptr(ptr), next(NULL)
  { }
  ~_ssgSave3dsData() {
    if (next != NULL) {
      delete next;
    }

    free(ptr);
  }

  unsigned int size() { 
    unsigned int s = memb_size*nmemb;
    if (next != NULL) {
      s += next->size();
    }
    return s;
  }

  void write() {
    if (!swabbed) {
      switch (memb_size) {
      case 2:
	ulEndianLittleArray16( (unsigned short*)ptr, nmemb );
	break;
      case 4:
	ulEndianLittleArray32( (unsigned int*)ptr, nmemb );
	break;
      default:
	break;
      }

      swabbed = true;
    }

    fwrite( ptr, memb_size, nmemb, save_fd );
    if (next != NULL) {
      next->write();
    }
  }

  void append( _ssgSave3dsData* next ) {
    this->next = next;
  }

protected:
  bool swabbed;

  size_t memb_size, nmemb;
  void*  ptr;
  _ssgSave3dsData* next;
};

class _ssgSave3dsChunk {
public:
  _ssgSave3dsChunk( unsigned short id ) : 
    id(id), first_data(NULL), last_data(NULL), next_sibling(NULL), 
    first_kid(NULL), last_kid(NULL) {}

  ~_ssgSave3dsChunk() {
    if (first_data != NULL)
      delete first_data;
    if (next_sibling != NULL)
      delete next_sibling;
    if (first_kid != NULL)
      delete first_kid;
  }

  void addData( _ssgSave3dsData* data ) {
    if (first_data == NULL) {
      first_data = last_data = data;
    } else {
      last_data->append(data);
      last_data = data;
    }
  }

  void addKid( _ssgSave3dsChunk* kid ) {
    if (first_kid == NULL) {
      first_kid = last_kid = kid;
    } else {
      last_kid->next_sibling = kid;
      last_kid = kid;
    }
  }

  void addSibling( _ssgSave3dsChunk* sibling ) {
    sibling->next_sibling = next_sibling;
    next_sibling = sibling;
  }

  unsigned int size() {
    unsigned int s = 6;
    if (first_data != NULL)
      s += first_data->size();

    _ssgSave3dsChunk* kid = first_kid;
    while (kid != NULL) {
      s += kid->size();
      kid = kid->next_sibling;
    }

    return s;
  }

  void write() {
    unsigned int s = size();
    writeChunkHeader(id, s);

    if (first_data != NULL) {
      first_data->write();
    }

    _ssgSave3dsChunk* kid = first_kid;
    while (kid != NULL) {
      kid->write();
      kid = kid->next_sibling;
    }
  }
  
protected:
  unsigned short id;
  _ssgSave3dsData *first_data, *last_data;
  _ssgSave3dsChunk *next_sibling;
  _ssgSave3dsChunk *first_kid, *last_kid;
};

static char* get_material_name( ssgSimpleState *state ) {
  static char matnamebuff[16];

  if (state->getName() != NULL) {
    return state->getName();
  } else {
    for (int i = 0; i < mat_count; i++) {
      if (state == mat[i]) {
	sprintf(matnamebuff, "Material #%d", i+1);
	return matnamebuff;
      }
    }

    ulSetError(UL_WARNING, "ssgSave3ds: Material not found.");
    return NULL;
  }
}

static _ssgSave3dsChunk* create_vertex_chunk( ssgLeaf* leaf, 
					     sgMat4 transform ) {
  _ssgSave3dsChunk* vertexlist = new _ssgSave3dsChunk( CHUNK_VERTLIST );
 
  unsigned short *num_verts;
  _ssgSave3dsData *nverts;
  float* vdata;
  
  num_verts = new unsigned short;
  *num_verts = leaf->getNumVertices();
  nverts = new _ssgSave3dsData(num_verts, 2, 1);
  
  vdata = new float[ *num_verts * 3];
  for (int i = 0; i < *num_verts; i++) {
    sgXformVec3(&vdata[i*3], leaf->getVertex(i), transform);
  }
  _ssgSave3dsData *vertices = new _ssgSave3dsData(vdata, 4, 
						*num_verts * 3);
  vertexlist->addData(nverts);
  vertexlist->addData(vertices);

  return vertexlist;
}

static _ssgSave3dsChunk* create_maplist_chunk( ssgLeaf* leaf ) {
  _ssgSave3dsChunk* maplist = new _ssgSave3dsChunk( CHUNK_MAPLIST );
 
  unsigned short *num_verts;
  _ssgSave3dsData *nverts;
  float* vdata;
  int flip_texture_y = FALSE;
  
  /* flip textures y-coord if texture is a BMP */
  char *texture_filename = 
    ((ssgSimpleState*)leaf->getState())->getTextureFilename();
  if (texture_filename != NULL) {
    char *texture_extension = 
      texture_filename + strlen(texture_filename) - 3;
    
    flip_texture_y = ulStrEqual( texture_extension, "BMP" );
  }

  num_verts = new unsigned short;
  *num_verts = leaf->getNumTexCoords();
  nverts = new _ssgSave3dsData(num_verts, 2, 1);
  
  vdata = new float[ *num_verts * 2];
  for (int i = 0; i < *num_verts; i++) {
    sgCopyVec2(&vdata[i*2], leaf->getTexCoord(i));

    if (flip_texture_y)
      vdata[i * 2 + 1] = 1.0f - vdata[i * 2 + 1];
  }
  _ssgSave3dsData *mapdata = new _ssgSave3dsData(vdata, 4, 
					       *num_verts * 2);
  maplist->addData(nverts);
  maplist->addData(mapdata);

  return maplist;
}

static _ssgSave3dsChunk* create_facemat_chunk( ssgLeaf* leaf ) {
  char* matname = get_material_name( (ssgSimpleState*)leaf->getState() );
  char* namecopy = new char[ strlen(matname)+1 ];
  strcpy(namecopy, matname);

  _ssgSave3dsData* namedata = new _ssgSave3dsData(namecopy, 1, 
						strlen(matname) + 1);

  unsigned short *fmdata = new unsigned short[ leaf->getNumTriangles() + 1 ];
  fmdata[0] = leaf->getNumTriangles();
  for (int i = 0; i < leaf->getNumTriangles(); i++) {
    fmdata[i+1] = i;
  }

  _ssgSave3dsData* fmlist = new _ssgSave3dsData(fmdata, 2, 
					      leaf->getNumTriangles() + 1);

  _ssgSave3dsChunk* facemat = new _ssgSave3dsChunk( CHUNK_FACEMAT );
  facemat->addData(namedata);
  facemat->addData(fmlist  );
  
  return facemat;
}

static _ssgSave3dsChunk* create_facelist_chunk( ssgLeaf* leaf ) {
  _ssgSave3dsChunk* facelist = new _ssgSave3dsChunk( CHUNK_FACELIST );
 
  short* fdata = new short[leaf->getNumTriangles() * 4 + 1];
  fdata[0] = leaf->getNumTriangles();

  for (int i = 0; i < leaf->getNumTriangles(); i++) {
    leaf->getTriangle(i, 
		      &fdata[i * 4 + 1], 
		      &fdata[i * 4 + 2], 
		      &fdata[i * 4 + 3]);
    fdata[i * 4 + 4] = 7;  // flag indicating anti-clockwise ordering (?)
  }
  _ssgSave3dsData *facedata = new _ssgSave3dsData(fdata, 2, 
						leaf->getNumTriangles()*4 + 1);
  facelist->addData(facedata);

  facelist->addKid( create_facemat_chunk(leaf) );

  return facelist;
}

static _ssgSave3dsChunk* create_transform_chunk() {
  // creates and identity transform
  _ssgSave3dsChunk* transform = new _ssgSave3dsChunk( CHUNK_TRMATRIX );
  
  float* matrix = new float[12];

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      matrix[i * 3 + j] = (i == j) ? 1.0f : 0.0f;
    }
  }

  _ssgSave3dsData* matdata = new _ssgSave3dsData( matrix, 4, 12 );
  transform->addData(matdata);

  return transform;
}

static void traverse_objects( ssgEntity* node, _ssgSave3dsChunk *parent, 
			      sgMat4 transform ) {
  if ( node->isAKindOf( ssgTypeTransform() ) ) {
    sgMat4 local_transform;
    ssgTransform *t_node = (ssgTransform*)node;
    
    t_node->getTransform(local_transform);
    sgPostMultMat4( local_transform, transform );

    for (ssgEntity* kid = t_node->getKid(0); kid != NULL; 
	 kid = t_node->getNextKid()) {
      traverse_objects( kid, parent, local_transform );
    }
  } else if ( node->isAKindOf( ssgTypeBranch() ) ) {
    ssgBranch *b_node = (ssgBranch*)node;
    for (ssgEntity* kid = b_node->getKid(0); kid != NULL; 
	 kid = b_node->getNextKid()) {
      traverse_objects( kid, parent, transform );
    }    
  } else if ( node->isAKindOf( ssgTypeLeaf() ) ) {
    ssgLeaf* l_node = (ssgLeaf*)node;

    _ssgSave3dsChunk *mesh = new _ssgSave3dsChunk(CHUNK_TRIMESH );

    mesh -> addKid( create_vertex_chunk  (l_node, transform) );
    mesh -> addKid( create_facelist_chunk(l_node)            );
    mesh -> addKid( create_transform_chunk()                 );
    if (l_node->getNumTexCoords() > 0) {
      mesh -> addKid( create_maplist_chunk(l_node) );
    }

    parent->addKid(mesh);
  }
}

static void create_objects_chunk( ssgEntity* ent, _ssgSave3dsChunk* parent ) {
  _ssgSave3dsChunk* objs = new _ssgSave3dsChunk(CHUNK_OBJBLOCK);
  static const char const_objname[] = "Object written by ssgSave3ds";

  char *objname = new char[ strlen(const_objname) + 1 ];
  strcpy(objname, const_objname);
  objs->addData( new _ssgSave3dsData(objname, 1, strlen(objname)+1) );
  parent->addKid(objs);

  sgMat4 ident;
  sgMakeIdentMat4( ident );

  traverse_objects( ent, objs, ident );
}

static _ssgSave3dsChunk *create_colour_chunk( unsigned short id, 
					     float* colour ) {
  _ssgSave3dsChunk *chunk = new _ssgSave3dsChunk(id);

  _ssgSave3dsChunk *rgbchunk = new _ssgSave3dsChunk(CHUNK_RGB1);
  float *colvec = new sgVec3;
  sgCopyVec3(colvec, colour);
  _ssgSave3dsData *coldata = new _ssgSave3dsData(colvec, 4, 3);
  rgbchunk->addData(coldata);

  chunk->addKid(rgbchunk);
  return chunk;
}

static _ssgSave3dsChunk *create_shininess_chunk( float shininess ) {
  _ssgSave3dsChunk *chunk = new _ssgSave3dsChunk(CHUNK_SHINE_STRENGTH);

  _ssgSave3dsChunk *percent = new _ssgSave3dsChunk(CHUNK_AMOUNT);
  unsigned short *amount = new unsigned short;
  *amount = (unsigned short)(shininess * 100.0f / 128.0f);
  _ssgSave3dsData *sdata = new _ssgSave3dsData(amount, 2, 1);
  percent->addData(sdata);

  chunk->addKid(percent);
  return chunk;
}

static _ssgSave3dsChunk *create_mapparam_chunk( unsigned short id, float val ) {
  _ssgSave3dsChunk *chunk = new _ssgSave3dsChunk( id );
  float *f = new float;
  *f = val;
  chunk -> addData( new _ssgSave3dsData(f, 4, 1) );
  return chunk;
}

static _ssgSave3dsChunk *create_map_chunk( ssgSimpleState *state ) {
  _ssgSave3dsChunk *map_chunk = new _ssgSave3dsChunk(CHUNK_TEXTURE);

  _ssgSave3dsChunk *mapname_chunk = new _ssgSave3dsChunk(CHUNK_MAPFILENAME);
  char *mapname = new char[ strlen(state->getTextureFilename()) + 1 ];
  strcpy(mapname, state->getTextureFilename());
  mapname_chunk -> addData( new _ssgSave3dsData(mapname, 1, 
					       strlen(mapname) + 1) );
  map_chunk -> addKid(mapname_chunk);

  map_chunk -> addKid( create_mapparam_chunk( CHUNK_MAP_USCALE, 1.0f ) );
  map_chunk -> addKid( create_mapparam_chunk( CHUNK_MAP_VSCALE, 1.0f ) );
  map_chunk -> addKid( create_mapparam_chunk( CHUNK_MAP_UOFFST, 0.0f ) );
  map_chunk -> addKid( create_mapparam_chunk( CHUNK_MAP_VOFFST, 0.0f ) );

  return map_chunk;
}

static _ssgSave3dsChunk *create_material_chunk( ssgSimpleState *state ) {
  _ssgSave3dsChunk *mat = new _ssgSave3dsChunk(CHUNK_MATERIAL);
  
  _ssgSave3dsChunk *matname_chunk = new _ssgSave3dsChunk(CHUNK_MATNAME);
  char *matname;
  if (state->getName() != NULL) {
    matname = new char[ strlen(state->getName()) + 1 ];
    strcpy(matname, state->getName());
  } else {
    matname = new char[16];
    sprintf(matname, "Material #%d", mat_count);
  }
  _ssgSave3dsData *matname_data = new _ssgSave3dsData(matname, 1, 
						    strlen(matname) + 1);
  matname_chunk->addData(matname_data);
  mat->addKid(matname_chunk);

  mat->addKid( create_colour_chunk( CHUNK_AMBIENT, 
				    state->getMaterial(GL_AMBIENT) ) );
  mat->addKid( create_colour_chunk( CHUNK_DIFFUSE, 
				    state->getMaterial(GL_DIFFUSE) ) );
  mat->addKid( create_colour_chunk( CHUNK_SPECULAR, 
				    state->getMaterial(GL_SPECULAR) ) );
  mat->addKid( create_shininess_chunk( state->getShininess() ) );

  if (state->getTextureFilename() != NULL) {
    mat->addKid( create_map_chunk(state) );
  }

  return mat;
}

static void traverse_materials( ssgEntity* node, _ssgSave3dsChunk* parent ) {
  if ( node->isAKindOf( ssgTypeBranch() ) ) {
    ssgBranch *b_node = (ssgBranch*)node;
    for (ssgEntity* kid = b_node->getKid(0); kid != NULL; 
	 kid = b_node->getNextKid()) {
      traverse_materials( kid, parent );
    }    
  } else if ( node->isAKindOf( ssgTypeLeaf() ) ) {
    ssgLeaf* l_node = (ssgLeaf*)node;
    ssgSimpleState *state = (ssgSimpleState*)l_node->getState();

    for (int i = 0; i < mat_count; i++) {
      if (state == mat[i])
	return;
    }

    mat[mat_count++] = state;
    parent->addKid( create_material_chunk(state) );
  }
}

static void create_materials_chunk( ssgEntity* ent, _ssgSave3dsChunk* parent ) {
  mat_count = 0;
  for (int i = 0; i < MAX_MATERIALS; i++) {
    mat[i] = NULL;
  }

  traverse_materials( ent, parent );
}

int ssgSave3ds( const char* filename, ssgEntity* ent ) {
  save_fd = fopen( filename, "wba" );

  if (save_fd == NULL) {
    ulSetError( UL_WARNING, "ssgSave3ds: Failed to open '%s' for writing",
		filename );
    return FALSE;
  }

  _ssgSave3dsChunk *top_chunk     = new _ssgSave3dsChunk( CHUNK_MAIN    );
  _ssgSave3dsChunk *main_chunk    = new _ssgSave3dsChunk( CHUNK_OBJMESH );

  _ssgSave3dsChunk *version_chunk = new _ssgSave3dsChunk( CHUNK_VERSION );  
  unsigned int *version = new unsigned int;
  *version = 3;
  _ssgSave3dsData *versiondata = new _ssgSave3dsData(version, 4, 1);
  version_chunk -> addData(versiondata);

  top_chunk -> addKid( main_chunk    );
  top_chunk -> addKid( version_chunk );

  create_materials_chunk (ent, main_chunk);
  create_objects_chunk   (ent, main_chunk);
  top_chunk->write();

  delete top_chunk;
  
  fclose(save_fd);

  return TRUE;
}
