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

//===========================================================================
//                                                                           
// File: GngMsfsIO.cpp                                                       
//                                                                           
// Created: Tue Feb 29 22:20:31 2000                                         
//                                                                           
// Author: Thomas Engh Sevaldrud <tse@math.sintef.no>
//                                                                           
// Revision: $Id$
//                                                                           
// Description:
//                                                                           
//===========================================================================
// Copyright (c) 2000 Thomas E. Sevaldrud <tse@math.sintef.no>
//===========================================================================

#include "ssgLocal.h"

#ifdef SSG_LOAD_MDL_SUPPORTED

#include "ssgLoadMDL.h"

#define DEF_SHININESS 50

//#define DEBUG

#ifdef DEBUG
#include <iostream>
#define DEBUGPRINT(x) std::cerr << x
#else
#define DEBUGPRINT(x)
#endif

static ssgLoaderOptions *current_options;

// Temporary vertex arrays
static ssgVertexArray 	  *curr_vtx_;
static ssgNormalArray 	  *curr_norm_;
static ssgIndexArray      *curr_index_;

// Vertex arrays
static ssgVertexArray 	  *vertex_array_;
static ssgNormalArray 	  *normal_array_;
static ssgTexCoordArray   *tex_coords_;

// Current part (index array)
static ssgLeaf  		  *curr_part_;
static ssgBranch		  *model_;

// Moving parts
static ssgBranch		  *ailerons_grp_, *elevator_grp_, *rudder_grp_;
static ssgBranch		  *gear_grp_, *spoilers_grp_, *flaps_grp_;
static ssgBranch          *prop_grp_;

static sgMat4   		  curr_matrix_;
static sgVec3			  curr_rot_pt_;
static sgVec4			  curr_col_;
static char		         *curr_tex_name_;
static ssgAxisTransform	  *curr_xfm_;

#ifdef EXPERIMENTAL_CULL_FACE_CODE
static bool 			  curr_cull_face_;
#endif

// File Address Stack
static const int          MAX_STACK_DEPTH = 64; // wk: 32 is too small
static long       	      stack_        [MAX_STACK_DEPTH];
static int                stack_depth_;

//  static sgMat4	      matrix_stack_ [MAX_STACK_DEPTH];
//  static char	        *textures_    [MAX_STACK_DEPTH];
//  static ssgBranch		*groups_      [MAX_STACK_DEPTH];

static int			    start_idx_, last_idx_;
static int              curr_var_;

static bool			    has_normals_, vtx_dirty_, tex_vtx_dirty_;
//static bool             join_children_, override_normals_;

//static char             *tex_fmt_;

//===========================================================================

/*static void initLoader()
{
  start_idx_        = 0;
  join_children_    = true;
  override_normals_ = true;
  tex_fmt_          = "tif";
  stack_depth_      = 0;
#ifdef EXPERIMENTAL_CULL_FACE_CODE
  curr_cull_face_   = false ;    
#endif
}*/

//===========================================================================

static void newPart()
{
  vtx_dirty_     = true;
  tex_vtx_dirty_ = true;
  curr_tex_name_ = NULL;
  sgSetVec4( curr_col_, 1.0f, 1.0f, 1.0f, 1.0f );
  
  delete curr_vtx_;
  delete curr_norm_;
  curr_vtx_  = new ssgVertexArray ;
  curr_norm_ = new ssgNormalArray ;
} 

//===========================================================================

static void push_stack( long entry ) {
  assert( stack_depth_ < MAX_STACK_DEPTH - 1 );
  
  stack_[stack_depth_++] = entry;
}

static long pop_stack() {
  assert( stack_depth_ > 0 );
  
  return stack_[--stack_depth_];
}

//===========================================================================

static void recalcNormals() {
  DEBUGPRINT( "Calculating normals." << std::endl);
  sgVec3 n;
  
  for (int i = 0; i < curr_index_->getNum() - 2; i++) {
    unsigned short ix0 = *curr_index_->get(i    );
    unsigned short ix1 = *curr_index_->get(i + 1);
    unsigned short ix2 = *curr_index_->get(i + 2);
    
    sgMakeNormal( n, 
      vertex_array_->get(ix0),
      vertex_array_->get(ix1),
      vertex_array_->get(ix2) );
    
    sgCopyVec3( normal_array_->get(ix0), n );
    sgCopyVec3( normal_array_->get(ix1), n );
    sgCopyVec3( normal_array_->get(ix2), n );
  }
}

//===========================================================================

static void readPoint(FILE* fp, sgVec3 p)
{
  short x_int, y_int, z_int;
  y_int = ulEndianReadLittle16(fp);
  z_int = ulEndianReadLittle16(fp);
  x_int = ulEndianReadLittle16(fp);
  
  // Convert from .MDL units (ca 2mm) to meters
  p[0] =  -x_int/512.0f;
  p[1] =  y_int/512.0f;
  p[2] =  z_int/512.0f;
}

//===========================================================================

//  MtkPoint3D readPoint(FILE* fp)
//  {
//   short x_int, y_int, z_int;
//   fread(&x_int, 2, 1, fp);
//   fread(&y_int, 2, 1, fp);
//   fread(&z_int, 2, 1, fp);

//   // Convert from .MDL units (ca 2mm) to meters
//   MtkPoint3D p;
//   p.x() = (double)x_int/512.0;
//   p.y() = (double)y_int/512.0;
//   p.z() = (double)z_int/512.0;

//   /*
//   for(list<MtkTransMatrix3D>::iterator i = matrix_stack_.begin();
//       i != matrix_stack_.end(); i++)
//      {
//      p = p*(*i);
//      p += (*i).getCol(3);
//      }
//   */

//   if(matrix_stack_.size() > 0)
//      {
//      MtkTransMatrix3D m = matrix_stack_.front();
//      p -= m.getCol(3);
//      p = p*m;
//      }

//   MtkPoint3D r;
//   r.y() =  p.x();
//   r.z() = -p.y();
//   r.x() =  p.z();

//   return r;
//  }

//===========================================================================

static void readVector(FILE* fp, sgVec3 v)
{
  short x_int, y_int, z_int;
  y_int = ulEndianReadLittle16(fp);
  z_int = ulEndianReadLittle16(fp);
  x_int = ulEndianReadLittle16(fp);
  
  v[0] = -(float)x_int;
  v[1] = (float)y_int;
  v[2] = (float)z_int;
  
  sgNormaliseVec3( v );
}

//===========================================================================

static void createTriangIndices(ssgIndexArray *ixarr,
                                int numverts, const sgVec3 s_norm)
{
  sgVec3 v1, v2, cross;
  
  if ( numverts > ixarr->getNum() ) {
    ulSetError( UL_WARNING, "ssgLoadMDL: Index array with too few entries." );
    return;
  }
  
  // triangulate polygons
  if(numverts == 1)
  {
    unsigned short ix0 = *ixarr->get(0);
    if ( ix0 >= vertex_array_->getNum() ) {
      ulSetError(UL_WARNING, "ssgLoadMDL: Index out of bounds (%d/%d).",
        ix0, vertex_array_->getNum() );
      return;
    }
    
    curr_index_->add(ix0);
    curr_index_->add(ix0);
    curr_index_->add(ix0);
  }
  
  else if(numverts == 2)
  {
    unsigned short ix0 = *ixarr->get(0);
    unsigned short ix1 = *ixarr->get(1);
    if ( ix0 >= vertex_array_->getNum() ||
      ix1 >= vertex_array_->getNum() ) {
      ulSetError(UL_WARNING, "ssgLoadMDL: Index out of bounds. (%d,%d / %d",
        ix0, ix1, vertex_array_->getNum() );
      return;
    }
    
    curr_index_->add(ix0);
    curr_index_->add(ix1);
    curr_index_->add(ix0);
  }
  
  else if(numverts == 3)
  {
    unsigned short ix0 = *ixarr->get(0);
    unsigned short ix1 = *ixarr->get(1);
    unsigned short ix2 = *ixarr->get(2);
    if ( ix0 >= vertex_array_->getNum() ||
      ix1 >= vertex_array_->getNum() ||
      ix2 >= vertex_array_->getNum() ) {
      ulSetError(UL_WARNING, "ssgLoadMDL: Index out of bounds. " \
        "(%d,%d,%d / %d)", ix0, ix1, ix2, vertex_array_->getNum());
      return;
    }
    
    sgSubVec3(v1, 
      vertex_array_->get(ix1), 
      vertex_array_->get(ix0));
    sgSubVec3(v2, 
      vertex_array_->get(ix2),
      vertex_array_->get(ix0));
    
    sgVectorProductVec3(cross, v1, v2);
    
    if(sgScalarProductVec3(cross, s_norm) > 0.0f)
    {
      curr_index_->add(ix0);
      curr_index_->add(ix1);
      curr_index_->add(ix2);
    }
    else
    {
      curr_index_->add(ix0);
      curr_index_->add(ix2);
      curr_index_->add(ix1);
    }
  }
  
  else
  {
    unsigned short ix0 = *ixarr->get(0);
    unsigned short ix1 = *ixarr->get(1);
    unsigned short ix2 = *ixarr->get(2);
    if ( ix0 >= vertex_array_->getNum() ||
      ix1 >= vertex_array_->getNum() ||
      ix2 >= vertex_array_->getNum() ) {
      ulSetError(UL_WARNING, "ssgLoadMDL: Index out of bounds. " \
        "(%d,%d,%d / %d)", ix0, ix1, ix2, vertex_array_->getNum());
      return;
    }
    
    // Ensure counter-clockwise ordering
    sgMakeNormal(cross, 
      vertex_array_->get(ix0), 
      vertex_array_->get(ix1), 
      vertex_array_->get(ix2));
    bool flip = (sgScalarProductVec3(cross, s_norm) < 0.0);
    
    curr_index_->add(ix0);
    for(int i = 1; i < numverts; i++)
    {
      ix1 = *ixarr->get( flip ? numverts-i : i);
      
      if ( ix1 >= vertex_array_->getNum() ) {
        ulSetError(UL_WARNING, "ssgLoadMDL: Index out of bounds. (%d/%d)",
          ix1, vertex_array_->getNum());
        continue;
      }
      
      curr_index_->add(ix1);
    }
    
  }
}

//===========================================================================

static bool readTexIndices(FILE *fp, int numverts, const sgVec3 s_norm, bool flip_y)
{
	ssgIndexArray temp_index_;

  if(numverts <= 0)
    return false;
  
  if(tex_coords_->getNum() <
    vertex_array_->getNum())
  {
    sgVec2 dummy_pt;
    sgSetVec2(dummy_pt, FLT_MAX, FLT_MAX);
    for(int i = tex_coords_->getNum();
    i < vertex_array_->getNum(); i++)
      tex_coords_->add(dummy_pt);
  }
  
  // Read index values and texture coordinates
  for(int v = 0; v < numverts; v++) 
  {
    unsigned short ix;
    short tx_int, ty_int;
    
    ix     = ulEndianReadLittle16(fp);
    tx_int = ulEndianReadLittle16(fp);
    ty_int = ulEndianReadLittle16(fp);
    
    if (flip_y) {
      ty_int = 255 - ty_int;
    }
    
    int tex_idx = ix - start_idx_ + last_idx_;
    
    sgVec2 tc;
    sgSetVec2(tc, tx_int/255.0f, ty_int/255.0f);
    
    sgVec2 curr_tc;
    
    if ( tex_idx >= 0 && tex_idx < tex_coords_->getNum() ) {
      sgCopyVec2(curr_tc, tex_coords_->get(tex_idx));
    } else {
      ulSetError( UL_WARNING, "ssgLoadMDL: Texture coord out of range (%d).",
        tex_idx );
      continue;
    }
    
    double dist = sgDistanceVec2(curr_tc, tc);
    
    if((curr_tc[0] >= FLT_MAX - 1 && curr_tc[1] >= FLT_MAX - 1))
    {
      //DEBUGPRINT( "." );
      sgCopyVec2(tex_coords_->get(tex_idx), tc);
    }
    
    else if(dist > 0.0001)
    {
      // We have a different texture coordinate for an existing vertex,
      // so we have to copy this vertex and create a new index for it
      // to get the correct texture mapping.
      
      //DEBUGPRINT( "duplicating texture coordinate!\n");
      
      int idx = ix - start_idx_ + last_idx_;
      tex_idx = vertex_array_->getNum();
      
      ssgVertexArray* vtx_arr  = vertex_array_; //curr_vtx_;
      ssgNormalArray* norm_arr = normal_array_; //curr_norm_;
      
      sgVec3 vtx, nrm;
      sgCopyVec3( vtx, vtx_arr ->get(idx) );
      sgCopyVec3( nrm, norm_arr->get(idx) );
      vtx_arr ->add(vtx);
      norm_arr->add(nrm);
      
      tex_coords_->add(tc);
    }
    
    temp_index_.add(tex_idx);
    
#ifdef DEBUG
    int check_index = *temp_index_.get(v);
    float *check_tc = tex_coords_->get(check_index);
    DEBUGPRINT( "ix[" << v << "] = " << check_index <<
      " (u=" << check_tc[0] << ", v=" << 
      check_tc[1] << ")" << std::endl);
#endif
    
  }
  
  createTriangIndices(&temp_index_, numverts, s_norm);
  
  return true;
}

//===========================================================================

static bool readIndices(FILE* fp, int numverts, const sgVec3 s_norm)
{
	ssgIndexArray temp_index_;

  if(numverts <= 0)
    return false;
  
  // Read index values
  for(int v = 0; v < numverts; v++)
  {
    unsigned short ix;
    ix = ulEndianReadLittle16(fp);
    temp_index_.add(ix - start_idx_ + last_idx_);
    DEBUGPRINT( "ix[" << v << "] = " << *temp_index_.get(v) << std::endl);
  }
  
  createTriangIndices(&temp_index_, numverts, s_norm);
  
  return true;
}

//===========================================================================

static void setColor(int color, int pal_id)
{
  if(pal_id == 0x68) 
  {
    curr_col_[0] = fsAltPalette[color].r / 255.0f;
    curr_col_[1] = fsAltPalette[color].g / 255.0f;
    curr_col_[2] = fsAltPalette[color].b / 255.0f;
    curr_col_[3] = 0.2f;
  }
  else 
  {
    curr_col_[0] = fsAcPalette[color].r / 255.0f;
    curr_col_[1] = fsAcPalette[color].g / 255.0f;
    curr_col_[2] = fsAcPalette[color].b / 255.0f;
    curr_col_[3] = 1.0f;
  }
}

//===========================================================================

static void setColor(int r, int g, int b, int attr)
{
  curr_col_[0] = r / 255.0f;
  curr_col_[1] = g / 255.0f;
  curr_col_[2] = b / 255.0f;
  if(attr < 239)
    curr_col_[3] = 0.2f;
} 

//===========================================================================

static bool setTexture(char* name)
{
  curr_tex_name_ = name;
  
  return true;
}

//===========================================================================

static ssgSimpleState *createState(bool use_texture)
{
  DEBUGPRINT("new State: col = " << curr_col_[0] << ", " << curr_col_[1] <<
    ", " << curr_col_[2] << ", " << curr_col_[3]);
  if ( curr_tex_name_  == NULL )
    DEBUGPRINT(", tex = <NULL> " << std::endl);
  else
    DEBUGPRINT(", tex = " << curr_tex_name_ << std::endl);
  
  ssgSimpleState *state = new ssgSimpleState();
  
  state->setShininess(DEF_SHININESS);
  state->setShadeModel(GL_SMOOTH);
  
  state->enable   (GL_LIGHTING);
  state->enable   (GL_CULL_FACE);
  state->disable  (GL_COLOR_MATERIAL);
  
  if(curr_col_[3] < 0.99f)
  {
    state->setTranslucent();
    state->enable(GL_BLEND);
    state->enable(GL_ALPHA_TEST);
  }
  else
  {
    state->setOpaque();
    state->disable(GL_BLEND);
    state->disable(GL_ALPHA_TEST);
  }
  
  if(curr_tex_name_ != NULL && use_texture)
  {
    state->setMaterial( GL_AMBIENT, 1.0f, 1.0f, 1.0f, curr_col_[3]);
    state->setMaterial( GL_DIFFUSE, 1.0f, 1.0f, 1.0f, curr_col_[3]);
    state->enable(GL_TEXTURE_2D);
    state->setTexture( current_options -> 
      createTexture(curr_tex_name_, FALSE, FALSE) ) ;
  }
  else
  {
    state->setMaterial( GL_AMBIENT, curr_col_);
    state->setMaterial( GL_DIFFUSE, curr_col_);
    state->disable(GL_TEXTURE_2D);
  }
  
  state->setMaterial( GL_SPECULAR, 1.0f, 1.0f, 1.0f, curr_col_[3] );
  state->setMaterial( GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f );
  
  return state;
}

static ssgBranch *getCurrGroup() {
  //Find the correct parent for the new group
  if(curr_xfm_)
    return curr_xfm_;
  else
  {
    return model_;
  }
}

//===========================================================================

static ssgBranch *getMPGroup(int var)
{
  
  switch(var)
  {
  case 0x4c: 		// Rudder
		if(!rudder_grp_)
    {
      rudder_grp_ = new ssgBranch();
      rudder_grp_->setName("rudder");
      model_->addKid(rudder_grp_);
    }
    return rudder_grp_;
    break;
    
  case 0x4e: 		// Elevator
		if(!elevator_grp_)
    {
      elevator_grp_ = new ssgBranch();
      elevator_grp_->setName("elevator");
      model_->addKid(elevator_grp_);
    }
    return elevator_grp_;
    break;
    
  case 0x6a: 		// Ailerons
		if(!ailerons_grp_)
    {
      ailerons_grp_ = new ssgBranch();
      ailerons_grp_->setName("ailerons");
      model_->addKid(ailerons_grp_);
    }
    return ailerons_grp_;
    break;
    
  case 0x6c: 		// Flaps
		if(!flaps_grp_)
    {
      flaps_grp_ = new ssgBranch();
      flaps_grp_->setName("flaps");
      model_->addKid(flaps_grp_);
    }
    return flaps_grp_;
    break;
    
  case 0x6e: 		// Gear
		if(!gear_grp_)
    {
      gear_grp_ = new ssgBranch();
      gear_grp_->setName("gear");
      model_->addKid(gear_grp_);
    }
    return gear_grp_;
    break;
    
  case 0x7c: 		// Spoilers
		if(!spoilers_grp_)
    {
      spoilers_grp_ = new ssgBranch();
      spoilers_grp_->setName("spoilers");
      model_->addKid(spoilers_grp_);
    }
    return spoilers_grp_;
    break;
    
  case 0x58:
	case 0x7a: 		// Propeller
		if(!prop_grp_)
    {
      prop_grp_ = new ssgBranch();
      prop_grp_->setName("propeller");
      model_->addKid(prop_grp_);
    }
    return prop_grp_;
    break;
    
  default:
    return model_;
  }
  return NULL;
} 

//===========================================================================

static void getMPLimits(int var, float *min, float *max)
{
  switch(var)
  {
  case 0x4c: 		// Rudder
		*min = -30.0;
    *max =  30.0;
    break;
    
  case 0x4e: 		// Elevator
		*min = -30.0;
    *max =  30.0;
    break;
    
  case 0x6a: 		// Ailerons
		*min = -30.0;
    *max =  30.0;
    break;
    
  case 0x6c: 		// Flaps
		*min = 0.0;
    *max = 70.0;
    break;
    
  case 0x6e: 		// Gear
		*min = 0.0;
    *max = -90.0;
    break;
    
  case 0x7c: 		// Spoilers
		*min = 0.0;
    *max = 90.0;
    break;
    
  case 0x58:
	case 0x7a: 		// Propeller
		*min = 0.0;
    *max = 360.0;
    break;
  }
} 

//===========================================================================

ssgEntity *ssgLoadMDL(const char *fname, const ssgLoaderOptions *options)
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  ailerons_grp_ = NULL;
  elevator_grp_ = NULL;
  rudder_grp_ = NULL;
  gear_grp_ = NULL;
  spoilers_grp_ = NULL;
  flaps_grp_ = NULL;
  prop_grp_ = NULL;
  
  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;
  
  FILE *fp = fopen(filename, "rb");
  if(!fp) 
  {
    ulSetError( UL_WARNING, "ssgLoadMDL: Couldn't open MDL file '%s'!", 
      filename );
    return NULL;
  }
  
  // Find beginning of BGL Code segment
  unsigned short op1, op2;
  fread(&op1, 2, 1, fp);
  while(!feof(fp))
  {
    fread(&op2, 2, 1, fp);
    if(op1 == 0x76 && op2 == 0x3a)
    {
      fseek(fp, -4, SEEK_CUR);
      break;
    }
    op1 = op2;
  }
  
  if(feof(fp))
  {
    ulSetError( UL_WARNING, "ssgLoadMDL: No BGL Code found in file '%s'!",
      filename );
		fclose(fp);
    return NULL;
  }
  
  // Initialize object graph
  model_ = new ssgBranch();
  char* model_name = new char[128];
  char *ptr = (char*)&fname[strlen(fname) - 1];
  while(ptr != &fname[0] && *ptr != '/') ptr--;
  if(*ptr == '/') ptr++;
  strcpy(model_name, ptr);
  ptr = &model_name[strlen(model_name)];
  while(*ptr != '.' && ptr != &model_name[0]) ptr--; 
  *ptr = '\0';
  model_->setName(model_name);
  
  // Create group nodes for textured and non-textured objects
  curr_vtx_  = new ssgVertexArray();
  curr_norm_ = new ssgNormalArray();
  
  vertex_array_ = new ssgVertexArray();
  normal_array_ = new ssgNormalArray();
  
  tex_coords_ = new ssgTexCoordArray();
  
  start_idx_ = 0;
  last_idx_  = 0;
  curr_var_ = 0;
  stack_depth_ = 0;
  sgMakeIdentMat4(curr_matrix_);
  
  // Parse opcodes
  bool done = false;
  while(!feof(fp) && !done) 
  {
    unsigned int   skip_offset = 0;
    unsigned short opcode;
    fread(&opcode, 2, 1, fp);
    
    DEBUGPRINT( "opcode = " << std::hex << opcode << std::dec << std::endl );
    
    switch(opcode)
    {
    case 0x23:	// BGL_CALL
			{
        short offset;
        offset = ulEndianReadLittle16(fp);
        long addr = ftell(fp);
        DEBUGPRINT( "BGL_CALL(" << offset << ")\n" );
        push_stack(addr);
        long dst = addr + offset - 4;
        fseek(fp, dst, SEEK_SET);
      }
      break;
      
    case 0x8a:	// BGL_CALL32
			{
        int offset;
        offset = ulEndianReadLittle32(fp);
        long addr = ftell(fp);
        DEBUGPRINT( "BGL_CALL32(" << offset << ")\n" );
        push_stack(addr);
        long dst = addr + offset - 6;
        fseek(fp, dst, SEEK_SET);
      }
      break;
      
    case 0x0d:	// BGL_JUMP
			{
        short offset;
        offset = ulEndianReadLittle16(fp);
        long addr = ftell(fp);
        long dst = addr + offset - 4;
        fseek(fp, dst, SEEK_SET);
        DEBUGPRINT( "BGL_JUMP(" << offset << ")\n" );
      }
      break;
      
    case 0x88:	// BGL_JUMP32
			{
        int offset;
        offset = ulEndianReadLittle32(fp);
        long addr = ftell(fp);
        DEBUGPRINT( "BGL_JUMP32(" << offset << ")\n" );
        long dst = addr + offset - 6;
        fseek(fp, dst, SEEK_SET);
      }
      break;
      
    case 0x8e:	// BGL_VFILE_MARKER
			{
        short offset;
        offset = ulEndianReadLittle16(fp);
        DEBUGPRINT( "vars: " << offset << std::endl);
        break;
      }
      
    case 0x39: 	// BGL_IFMSK
			{
        short offset, var, mask;
        offset = ulEndianReadLittle16(fp);
        var    = ulEndianReadLittle16(fp);
        mask   = ulEndianReadLittle16(fp);
        long addr = ftell(fp);
        long dst = addr + offset - 8;
        DEBUGPRINT( "BGL_IFMSK(" << offset << ", 0x" << std::hex << var << 
          ", 0x" << mask << std::dec << ")\n" );
        //          if(var & mask == 0)
        switch(var)
        {
        case 0x7e:
					fseek(fp, dst, SEEK_SET);
          break;
          
        default:
          break;
        }
      }
      break;
      
    case 0x24: 	// BGL_IFIN1
			{
        short offset, lo, hi;
        unsigned short var;
        offset = ulEndianReadLittle16(fp);
        var    = ulEndianReadLittle16(fp);
        lo     = ulEndianReadLittle16(fp);
        hi     = ulEndianReadLittle16(fp);
        DEBUGPRINT( "BGL_IFIN1(" << offset << ", 0x" << std::hex << var << 
          ", " << std::dec << lo << ", " << hi << ")\n" );
        curr_var_ = var;
      }
      break;
      
    case 0x46:	// BGL_POINT_VICALL
			{
        short offset, var_rot_x, var_rot_y, var_rot_z;
        unsigned short int_rot_x, int_rot_y, int_rot_z;
        offset = ulEndianReadLittle16(fp);
        sgVec3 ctr;
        readPoint(fp, ctr);
        
        int_rot_y = ulEndianReadLittle16(fp);
        var_rot_y = ulEndianReadLittle16(fp);
        
        int_rot_x = ulEndianReadLittle16(fp);
        var_rot_x = ulEndianReadLittle16(fp);
        
        int_rot_z = ulEndianReadLittle16(fp);
        var_rot_z = ulEndianReadLittle16(fp);
        
        float rx =  360.0f*(float)int_rot_x/0xffff;
        float ry =  360.0f*(float)int_rot_y/0xffff;
        float rz =  360.0f*(float)int_rot_z/0xffff;
        
        // We build a rotation matrix by adding all constant 
        // rotations (int_rot_*) to current_matrix_. As soon as we reach 
        // the actual variable rotation, we multiply
        // the axis of the variable rotation with our current matrix. 
        // This will be the axis of rotation in the original coordinate 
        // system. This can now be inserted into a GngLinearControl 
        // transform.
        if(var_rot_x > 0 || var_rot_y > 0 || var_rot_z > 0)
        {
          ssgAxisTransform* tmp = NULL;
          if(curr_xfm_)
            tmp = curr_xfm_;
          curr_xfm_ = new ssgAxisTransform();
          curr_xfm_->setCenter(curr_rot_pt_);
          
          int var = 0;
          if(var_rot_x > 0)
            var = var_rot_x;
          else if(var_rot_y > 0)
            var = var_rot_y;
          else if(var_rot_z > 0)
            var = var_rot_z;
          
          float min_limit, max_limit;
          getMPLimits(var, & min_limit, & max_limit);
          
          sgVec3 axis = { (float)var_rot_y, (float)var_rot_z, 
            (float)var_rot_x };
          sgNormaliseVec3( axis ) ;
          sgXformVec3( axis, curr_matrix_ ) ;
          sgNegateVec3(axis);
          curr_xfm_->setAxis(axis);
          curr_xfm_->setRotationLimits(min_limit, max_limit);
          
          char name[256];
          sprintf(name, "ssgAxisRotation(%x)", var);
          curr_xfm_->setName(name);
          if(tmp)
            tmp->addKid(curr_xfm_);
          else
          {
            ssgBranch* grp = getMPGroup(var);
            grp->addKid(curr_xfm_);
          }
        }
        
        // Build up the constant rotations
        sgMat4 rot_mat;
        sgMakeRotMat4( rot_mat, ry, rz, rx );
        sgPostMultMat4( curr_matrix_, rot_mat );
        sgAddVec3( curr_rot_pt_, ctr );
        
        long addr = ftell(fp);
        long dst = addr + offset - 22;
        fseek(fp, dst, SEEK_SET);
        push_stack(addr);
        
        break;
      }
      
    case 0x5f:	// BGL_IFSIZEV
			{
        short offset;
        unsigned short real_size, pixels_ref;
        offset     = ulEndianReadLittle16(fp);
        real_size  = ulEndianReadLittle16(fp);
        pixels_ref = ulEndianReadLittle16(fp);
        DEBUGPRINT("BGL_IFSIZEV: jmp = " << offset << ", sz = " << 
          real_size << ", px = " << pixels_ref << std::endl);
        
        long addr = ftell(fp);
        long dst = addr + offset - 8;
        fseek(fp, dst, SEEK_SET);
        push_stack(addr);
        break;
      }
      
    case 0x3b:	// BGL_VINSTANCE
			{
        short offset, var;
        offset = ulEndianReadLittle16(fp);
        var    = ulEndianReadLittle16(fp);
        long addr = ftell(fp);
        long var_abs = addr + var - 6;
        fseek(fp, var_abs, SEEK_SET);
        float p = 360.0f * (float)ulEndianReadLittle32(fp) / 0xffffffff;
        float r = 360.0f * (float)ulEndianReadLittle32(fp) / 0xffffffff;
        float h = 360.0f * (float)ulEndianReadLittle32(fp) / 0xffffffff;
        sgMat4 rot_mat;
        sgMakeRotMat4(rot_mat, h, p, r);
        sgPostMultMat4(curr_matrix_, rot_mat);
        DEBUGPRINT( "BGL_VINSTANCE(" << offset << ", h=" << h << ", p=" <<
          p << ", r=" << r << ")\n");
        long dst = addr + offset - 6;
        fseek(fp, dst, SEEK_SET);
      }
      break;
      
    case 0x0:	// EOF
		case 0x22: 	// BGL return
			{
        curr_xfm_    = NULL;
        sgMakeIdentMat4( curr_matrix_ );
        sgZeroVec3( curr_rot_pt_ );
        curr_var_    = 0;
        DEBUGPRINT( "BGL return\n\n");
        if(stack_depth_ == 0)
          done = true;
        else
        {
          long addr = pop_stack();
          fseek(fp, addr, SEEK_SET);
        }
      }
      break;
      
    case 0x1a: 	// RESLIST (point list with no normals)
			{
        newPart();
        has_normals_ = false;
        
        start_idx_               = ulEndianReadLittle16(fp);
        unsigned short numpoints = ulEndianReadLittle16(fp);
        
        DEBUGPRINT( "New group (unlit): start_idx = " << start_idx_ 
          << ", num vertices = " << numpoints << std::endl);
        
        sgVec3 null_normal;
        sgZeroVec3( null_normal );
        
        for(int i = 0; i < numpoints; i++) 
        {
          sgVec3 p;
          readPoint(fp, p);
          curr_vtx_->add(p);
          curr_norm_->add(null_normal);
        }
      }
      break;
      
    case 0x29: 	// GORAUD RESLIST (point list with normals)
			{
        newPart();
        has_normals_ = true;
        
        start_idx_               = ulEndianReadLittle16(fp);
        unsigned short numpoints = ulEndianReadLittle16(fp);
        
        DEBUGPRINT( "New group (goraud): start_idx = " << start_idx_
          << ", num vertices = " << numpoints << std::endl);
        
        for(int i = 0; i < numpoints; i++) 
        {
          sgVec3 p;
          readPoint(fp, p);
          sgVec3 v;
          readVector(fp, v);
          curr_vtx_->add(p);
          curr_norm_->add(v);
        }
      }
      break;
      
    case 0x0f:	// STRRES: Start line definition
			{
        unsigned short idx = ulEndianReadLittle16(fp);
        DEBUGPRINT( "Start line: idx = " << idx << std::endl);
        if(vtx_dirty_)
        {
          last_idx_ = vertex_array_->getNum();
          for(int i = 0; i < curr_vtx_->getNum(); i++)
          {
            vertex_array_->add(curr_vtx_ ->get(i));
            normal_array_->add(curr_norm_->get(i));
          }
          vtx_dirty_ = false;
        }
        
        curr_index_ = new ssgIndexArray();
        curr_part_ = new ssgVtxArray( GL_LINES,
          vertex_array_,
          normal_array_,
          NULL,
          NULL,
          curr_index_ );
        curr_part_->setState( createState(false) );
#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_part_->setCullFace ( curr_cull_face_ ) ;    
#endif

        curr_index_->add(idx - start_idx_ + last_idx_);
        ssgBranch *grp = getCurrGroup();
        grp->addKid(curr_part_);
        
        //assert(curr_part_->getState()->getTexture() == NULL);
      }
      break;
      
    case 0x10:	// CNTRES: Continue line definition
			{
        unsigned short idx = ulEndianReadLittle16(fp);
        DEBUGPRINT( "Cont. line: idx = " << idx << std::endl);
        curr_index_->add(idx - start_idx_ + last_idx_);
      }
      break;
      
    case 0x20:
		case 0x7a: 	// Goraud shaded Texture-mapped ABCD Facet
			{
        if(tex_vtx_dirty_)
        {
          last_idx_ = vertex_array_->getNum();
          for(int i = 0; i < curr_vtx_->getNum(); i++)
          {
            vertex_array_->add(curr_vtx_ ->get(i));
            normal_array_->add(curr_norm_->get(i));
          }
          tex_vtx_dirty_ = false;
        }
        
        curr_index_ = new ssgIndexArray();
        curr_part_ = new ssgVtxArray( GL_TRIANGLE_FAN, vertex_array_,
          normal_array_,
          tex_coords_,
          NULL,
          curr_index_ );
        curr_part_->setState( createState(true) );
        
        //assert(curr_part_->getState()->getTexture() != NULL);
        
        unsigned short numverts = ulEndianReadLittle16(fp);
        DEBUGPRINT( "New part: (goraud/texture), num indices = " << 
          numverts << std::endl);
        
        // Normal vector
        sgVec3 v;
        readVector(fp, v);
        
        // Dot product reference
#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_cull_face_ = ulEndianReadLittle32(fp) >= 0;
        curr_part_->setCullFace ( curr_cull_face_ ) ;    
#else
        ulEndianReadLittle32(fp);
#endif
        // Read vertex inidices and texture coordinates
        bool flip_y = FALSE;
        if(curr_tex_name_!=NULL)
        { char *texture_extension = 
        curr_tex_name_ + strlen(curr_tex_name_) - 3;
        flip_y = ulStrEqual( texture_extension, "BMP" ) != 0 ;
        }
        /*old:
        char *texture_extension = 
        curr_tex_name_ + strlen(curr_tex_name_) - 3;
        bool flip_y = ulStrEqual( texture_extension, "BMP" );
        */
        readTexIndices(fp, numverts, v, flip_y);
        
        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }
        
        ssgBranch* grp = getCurrGroup();
        ((ssgVtxArray *)curr_part_)->removeUnusedVertices();
        grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
      }
      break;
      
    case 0x60: 	// BGL_FACE_TMAP
			{
        if(tex_vtx_dirty_)
        {
          last_idx_ = vertex_array_->getNum();
          for(int i = 0; i < curr_vtx_->getNum(); i++)
          {
            vertex_array_->add(curr_vtx_ ->get(i));
            normal_array_->add(curr_norm_->get(i));
          }
          tex_vtx_dirty_ = false;
        }
        
        curr_index_ = new ssgIndexArray();
        curr_part_  = new ssgVtxArray( GL_TRIANGLE_FAN,
          vertex_array_,
          normal_array_,
          tex_coords_,
          NULL,
          curr_index_ );
        curr_part_->setState( createState(true) );
#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_part_->setCullFace ( curr_cull_face_ ) ;    
#endif        
        //assert(curr_part_->getState()->getTexture() != NULL);
        
        unsigned short numverts = ulEndianReadLittle16(fp);
        DEBUGPRINT( "New part: (goraud/texture), num indices = " << 
          numverts << std::endl);
        
        // Point in polygon
        sgVec3 p;
        readPoint(fp, p);
        
        // Normal vector
        sgVec3 v;
        readVector(fp, v);
        
        // Read vertex inidices and texture coordinates
        bool flip_y = FALSE;
        if(curr_tex_name_!=NULL)
        { char *texture_extension = 
        curr_tex_name_ + strlen(curr_tex_name_) - 3;
        flip_y = ulStrEqual( texture_extension, "BMP" ) != 0 ;
        }
        /*
        char *texture_extension = 
        curr_tex_name_ + strlen(curr_tex_name_) - 3;
        bool flip_y = ulStrEqual( texture_extension, "BMP" );
        */
        readTexIndices(fp, numverts, v, flip_y);
        
        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }
        
        ssgBranch* grp = getCurrGroup();
        ((ssgVtxArray *)curr_part_)->removeUnusedVertices();
        grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
      }
      break;
      
    case 0x1d:	// BGL_FACE
			{
        if(vtx_dirty_)
        {
          last_idx_ = vertex_array_->getNum();
          for(int i = 0; i < curr_vtx_->getNum(); i++)
          {
            vertex_array_->add(curr_vtx_->get(i));
            normal_array_->add(curr_norm_->get(i));
          }
          vtx_dirty_ = false;
        }
        
        curr_index_ = new ssgIndexArray();
        curr_part_ = new ssgVtxArray(GL_TRIANGLE_FAN,
          vertex_array_,
          normal_array_,
          NULL,
          NULL,
          curr_index_);
        curr_part_->setState( createState(false) );
#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_part_->setCullFace ( curr_cull_face_ ) ;    
#endif
        //assert(curr_part_->getState()->getTexture() == NULL);
        
        unsigned short numverts = ulEndianReadLittle16(fp);
        DEBUGPRINT( "BGL_FACE: num indices = " << numverts << std::endl);
        
        sgVec3 p;
        readPoint(fp, p);
        // Surface normal
        sgVec3 v;
        readVector(fp, v);
        
        // Read vertex indices
        readIndices(fp, numverts, v);
        
        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }
        
        ssgBranch* grp = getCurrGroup();
        ((ssgVtxArray *)curr_part_)->removeUnusedVertices();
        grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
      }
      break;
      
    case 0x3e:	// FACETN (no texture)
		case 0x2a:	// Goraud shaded ABCD Facet
			{
        if(vtx_dirty_)
        {
          last_idx_ = vertex_array_->getNum();
          for(int i = 0; i < curr_vtx_->getNum(); i++)
          {
            vertex_array_->add(curr_vtx_->get(i));
            normal_array_->add(curr_norm_->get(i));
          }
          vtx_dirty_ = false;
        }
        
        curr_index_ = new ssgIndexArray();
        curr_part_ = new ssgVtxArray(GL_TRIANGLE_FAN,
          vertex_array_,
          normal_array_,
          NULL,
          NULL,
          curr_index_);
        curr_part_->setState( createState(false) );
        
        //assert(curr_part_->getState()->getTexture() == NULL);
        
        unsigned short numverts = ulEndianReadLittle16(fp);
        DEBUGPRINT( "BGL_FACETN: num indices = " << numverts << std::endl);
        
        // Surface normal
        sgVec3 v;
        readVector(fp, v);
        
        // dot-ref
#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_cull_face_ = ulEndianReadLittle32(fp) >= 0;
        curr_part_->setCullFace ( curr_cull_face_ ) ;    
#else
        ulEndianReadLittle32(fp);
#endif
        // Read vertex indices
        readIndices(fp, numverts, v);
        
        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }
        
        ssgBranch* grp = getCurrGroup();
        ((ssgVtxArray *)curr_part_)->removeUnusedVertices();
        grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
      }
      break;
      
    case 0x18: 	// Set texture
			{
        unsigned short id, dx, scale, dy;
        id    = ulEndianReadLittle16(fp);
        dx    = ulEndianReadLittle16(fp);
        scale = ulEndianReadLittle16(fp);
        dy    = ulEndianReadLittle16(fp);
        char tex_name[14];
        fread(tex_name, 1, 14, fp);
        char tex_filename[14];
        int j = 0;
        for(int i = 0; i < 14; i++) 
        {
          if(!isspace(tex_name[i]))
            tex_filename[j++] = tolower(tex_name[i]);
        }
        tex_filename[j] = '\0';
        DEBUGPRINT( "Set texture: name = " << tex_filename << ", id = " << id
          << ", dx = " << dx << ", dy = " << dy << ", scale = " <<
          scale << std::endl);
        setTexture(tex_filename);
      }
      break;
      
    case 0x43:	// TEXTURE2
			{
        unsigned short length, idx;
        unsigned char  flags, chksum;
        unsigned int   color;
        char tex_filename[128];
        length = ulEndianReadLittle16(fp);
        idx    = ulEndianReadLittle16(fp);
        fread(&flags, 1, 1, fp);
        fread(&chksum, 1, 1, fp);
        color = ulEndianReadLittle32(fp);
        if(chksum != 0)
        {
          DEBUGPRINT( "warning: TEXTURE2 Checksum != 0\n");
        }
        
        char c;
        int i = 0;
        while((c = getc(fp)) != 0)
        {
          if(!isspace(c))
            tex_filename[i++] = tolower(c);
        }
        tex_filename[i] = '\0';
        
        // Padding byte
        if((strlen(tex_filename) + 1) % 2)
          c = getc(fp);
        
        DEBUGPRINT( "TEXTURE2: Set texture: name = " << tex_filename << 
          std::endl);
        setTexture(tex_filename);
        break;
      }
      
    case 0x50: 	// GCOLOR (Goraud shaded color)
		case 0x51:	// LCOLOR (Line color)
		case 0x52:     	// SCOLOR (Light source shaded surface color)
			{
        unsigned char color, param;
        fread(&color, 1, 1, fp);
        fread(&param, 1, 1, fp);
        DEBUGPRINT( "Set color = " << (int)color << " (" << std::hex << 
          (int)param << std::dec << ")\n");
        setColor((int)color, (int)param);
      }
      break;
      
    case 0x2D: 	// BGL_SCOLOR24
			{
        unsigned char col[4];
        fread(col, 1, 4, fp);
        DEBUGPRINT( "color = " << (int)col[0] << ", " << (int)col[2] << 
          ", " << (int)col[3] << ", " << (int)col[1] << std::endl);
        setColor(col[0], col[2], col[3], col[1]);
        break;
      }
      
      
      //-------------------------------------------
      // The rest of the codes are either ignored
      // or for experimental use..
      //-------------------------------------------
      
    case 0x03:
			{
        //DEBUGPRINT( "BGL_CASE\n" );
        unsigned short number_cases;
        fread(&number_cases, 2, 1, fp);
        skip_offset = 6 + 2 * number_cases;
      }
      break;
      
    default: // Unknown opcode
      {
        if (opcode < 256)
        {
          if ( opcodes[opcode].size != -1)
          {
            DEBUGPRINT( "** " << opcodes[opcode].name << " (size " <<
              opcodes[opcode].size << ")" << std::endl );
            skip_offset = opcodes[opcode].size - 2; // opcode already read
          }
          else
          {
            DEBUGPRINT( "Unhandled opcode " << opcodes[opcode].name
              << " (" << std::hex << opcode << std::dec << ")" <<
              std::endl );
          }
        }
        else
        {
          DEBUGPRINT( "Op-code out of range: " << std::hex << opcode <<
            std::dec << std::endl );
        }
      }
      break;
       }
       
       if (skip_offset > 0) 
         fseek( fp, skip_offset, SEEK_CUR );
       
    }
    
    fclose(fp);
    
    delete curr_vtx_;
    delete curr_norm_;
    
    DEBUGPRINT("\n" << vertex_array_->getNum() << " vertices\n");

    return model_;
}

#else

bool ssgLoadMDLTexture ( const char *fname, ssgTextureInfo* info )
{
  ulSetError ( UL_WARNING,
    "ssgLoadTexture: '%s' - MDL support not configured", fname ) ;
  return false ;
}


#endif

