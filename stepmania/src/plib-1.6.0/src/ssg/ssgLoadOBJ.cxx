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
// Wavefront OBJ import for SSG/PLIB
// Ported from IVCON by Dave McClurg (dpm@efn.org) in March-2000
// Updated by Dave McClurg in April-2000 (added textures)
//
/*
  Purpose:
   
    OBJ_READ reads a Wavefront OBJ file.

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

    20 October 1998

  Author:
 
    John Burkardt
*/

#include "ssgLocal.h"

#define MAX_LINE_LEN 1024
#define MAX_LINE 100000
#define MAX_FACE 100000
#define MAX_VERT 100000
#define MAX_MATERIALS 1000


struct faceData
{
   int vlist [3] ;
   int tlist [3] ;
   int nlist [3] ;
   int mat_index ;
} ;


struct matData
{
  char* name ;
  sgVec4 amb ;
  sgVec4 diff ;
  sgVec4 spec ;
  float shine ;
  float trans ;
  char* tfname ;
} ;


static int num_line ;
static int* line_dex ;

static int num_face ;
static faceData* face ;

static int num_vert ;
static int num_vert_tex ;
static int num_vert_normal ;
static sgVec3* vert ;
static sgVec3* vert_tex ;
static sgVec3* vert_normal ;

static int num_mat ;
static matData* materials ;

static ssgBranch       *current_branch   = NULL ;

static ssgState* get_state( matData* mat )
{
  if (mat->tfname != NULL)
  {
    ssgState *st = ssgGetCurrentOptions () -> createState ( mat->tfname ) ;
    if ( st != NULL )
      return st ;
  }

  ssgSimpleState *st = new ssgSimpleState () ;

  st -> setMaterial ( GL_AMBIENT, mat -> amb ) ;
  st -> setMaterial ( GL_DIFFUSE, mat -> diff ) ;
  st -> setMaterial ( GL_SPECULAR, mat -> spec ) ;
  st -> setShininess ( mat -> shine ) ;

  st -> enable ( GL_COLOR_MATERIAL ) ;
  st -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

  st -> enable  ( GL_LIGHTING       ) ;
  st -> setShadeModel ( GL_SMOOTH ) ;

  if ( mat -> trans < 0.99f )
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

  if (mat->tfname != NULL)
  {
    st -> setTexture( ssgGetCurrentOptions () -> createTexture(mat->tfname) ) ;
    st -> enable( GL_TEXTURE_2D ) ;
  }
  else
  {
    st -> disable( GL_TEXTURE_2D ) ;
  }

  return st ;
}

static int leqi ( const char* string1, const char* string2 )
//LEQI compares two strings for equality, disregarding case.
{
  int i;
  int nchar;
  int nchar1;
  int nchar2;

  nchar1 = strlen ( string1 );
  nchar2 = strlen ( string2 );

  if ( nchar1 < nchar2 ) {
    nchar = nchar1;
  }
  else {
    nchar = nchar2;
  }
/*
  The strings are not equal if they differ over their common length.
*/
  for ( i = 0; i < nchar; i++ ) {

    if ( toupper ( string1[i] ) != toupper ( string2[i] ) ) {
      return FALSE;
    }
  }
/*
  The strings are not equal if the longer one includes nonblanks
  in the tail.
*/
  if ( nchar1 > nchar ) {
    for ( i = nchar; i < nchar1; i++ ) {
      if ( string1[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  else if ( nchar2 > nchar ) {
    for ( i = nchar; i < nchar2; i++ ) {
      if ( string2[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  return TRUE;
}

static void load_materials ( const char* fname )
{
  num_mat = 0 ;

  char path [ 1024 ] ;
  ssgGetCurrentOptions () -> makeModelPath ( path, fname ) ;

  FILE* filein = fopen (path,"r") ;
  if ( filein == 0 )
     return ;

  int index = -1 ;

  char  input[MAX_LINE_LEN];
  while ( fgets ( input, MAX_LINE_LEN, filein ) != NULL )
  {
    char *next;
    for ( next = input; *next != '\0' && isspace(*next); next++ )
       ;
    if ( *next == '\0' )
      continue;
    if ( *next == '#' || *next == '$' )
      continue;

    char  token[MAX_LINE_LEN];
    int   width;
    int   count;
    float r1,r2,r3;

    sscanf ( next, "%s%n", token, &width ) ;
    next = next + width ;

    if ( leqi ( token, "NEWMTL" ) == TRUE ) {

      char name[MAX_LINE_LEN];
      count = sscanf ( next, "%s%n", name, &width ) ;
 
      if ( count == 1 ) {
        index ++ ;
        memset( &materials[ index ], 0, sizeof(matData) ) ;
        materials[ index ].name = new char [ strlen ( name ) + 1 ] ;
        strcpy ( materials[ index ].name, name ) ;
        materials[ index ].amb[3] = 1.0f ;
        materials[ index ].diff[3] = 1.0f ;
        materials[ index ].spec[3] = 1.0f ;
        materials[ index ].shine = 0.0f ;
        materials[ index ].trans = 1.0f ;
      }
    }
    else if ( leqi ( token, "Ka" ) == TRUE ) {

      count = sscanf ( next, "%e %e %e", &r1, &r2, &r3 ) ;

      if ( count == 3 && index >= 0 ) {
        materials[ index ].amb[0] = r1 ;
        materials[ index ].amb[1] = r2 ;
        materials[ index ].amb[2] = r3 ;
      }
    }
    else if ( leqi ( token, "Kd" ) == TRUE ) {

      count = sscanf ( next, "%e %e %e", &r1, &r2, &r3 ) ;

      if ( count == 3 && index >= 0 ) {
        materials[ index ].diff[0] = r1 ;
        materials[ index ].diff[1] = r2 ;
        materials[ index ].diff[2] = r3 ;
      }
    }
    else if ( leqi ( token, "Ks" ) == TRUE ) {

      count = sscanf ( next, "%e %e %e", &r1, &r2, &r3 ) ;

      if ( count == 3 && index >= 0 ) {
        materials[ index ].spec[0] = r1 ;
        materials[ index ].spec[1] = r2 ;
        materials[ index ].spec[2] = r3 ;
      }
    }
    else if ( leqi ( token, "map_Kd" ) == TRUE ) {

      char tfname[MAX_LINE_LEN];
      count = sscanf ( next, "%s%n", tfname, &width ) ;

      if ( count == 1 && index >= 0 ) {
        materials[ index ].tfname = new char [ strlen( tfname )+1 ] ;
        strcpy ( materials[ index ].tfname, tfname ) ;
      }
    }
  }

  fclose (filein) ;

  num_mat = (index+1) ;
}


static void add_mesh ( int mat_index )
{
  int i ;

  //count faces with same material
  int num = 0 ;
  for ( i=0; i<num_face; i++ )
     if ( face[i].mat_index == mat_index )
        num ++ ;
  if ( num == 0 )
    return ;

  //add to scene graph
  ssgVertexArray   *vlist = new ssgVertexArray ( num * 3 ) ;
  ssgTexCoordArray *tlist = 0 ;
  ssgNormalArray   *nlist = 0 ;
  if ( num_vert_tex )
    tlist = new ssgTexCoordArray ( num * 3 ) ;
  if ( num_vert_normal )
    nlist = new ssgNormalArray ( num * 3 ) ;

  for ( i=0; i<num_face; i++ ) {
    if ( face[i].mat_index == mat_index ) {
      for ( int j=0; j<3; j++ ) {
        vlist -> add ( vert[ face[i].vlist[j] ] ) ;
        if ( num_vert_tex )
          tlist -> add ( vert_tex[ face[i].tlist[j] ] ) ;
        if ( num_vert_normal )
          nlist -> add ( vert_normal[ face[i].nlist[j] ] ) ;
      }
    }
  }

  ssgState *st = NULL ;
  if ( mat_index < num_mat ) {
    matData* mat = &materials[ mat_index ];
    if ( mat->tfname != 0 )
      st = ssgGetCurrentOptions() -> createState ( mat->tfname ) ;
    if ( st == NULL )
      st = get_state ( mat ) ;
  }

  ssgVtxTable *vtab = new ssgVtxTable ( GL_TRIANGLES,
    vlist, nlist, tlist, 0 ) ;

  vtab -> setCullFace ( TRUE );
  if ( st )
     vtab -> setState ( st ) ;

  current_branch -> addKid ( vtab ) ;
}


static int obj_read ( FILE *filein )
{
  int   count;
  char *next;
  char *next2;
  char *next3;
  int   node;
  float r1;
  float r2;
  float r3;
  char  input[MAX_LINE_LEN];
  char  token[MAX_LINE_LEN];
  char  token2[MAX_LINE_LEN];
  int   width;
  int   mat_current = 0 ;
/* 
  Initialize. 
*/
  num_line = 0 ;
  line_dex = new int[ MAX_LINE ] ;

  num_face = 0 ;
  face = new faceData[ MAX_FACE ] ;

  num_vert = 0 ;
  num_vert_tex = 0 ;
  num_vert_normal = 0 ;
  vert = new sgVec3[ MAX_VERT ] ;
  vert_tex = new sgVec3[ MAX_VERT ] ;
  vert_normal = new sgVec3[ MAX_VERT ] ;

  num_mat = 0 ;
  materials = new matData [ MAX_MATERIALS ] ;

/* 
  Read the next line of the file into INPUT. 
*/
  while ( fgets ( input, MAX_LINE_LEN, filein ) != NULL ) {

    for ( next = input; *next != '\0' && isspace(*next); next++ )
       ;
    if ( *next == '\0' )
      continue;
    if ( *next == '#' || *next == '$' )
      continue;

    sscanf ( next, "%s%n", token, &width );
    next = next + width;

/*  
  F V1 V2 V3
    or
  F V1/VT1/VN1 V2/VT2/VN2 ...
    or
  F V1//VN1 V2//VN2 ...

  Face.
  A face is defined by the vertices.
  Optionally, slashes may be used to include the texture vertex
  and vertex normal indices.

  OBJ line node indices are 1 based rather than 0 based.
  So we have to decrement them before loading them into FACE.
*/

    if ( leqi ( token, "F" ) == TRUE ) {

      int vlist [4] ;
      int tlist [4] ;
      int nlist [4] ;
      int ivert = 0 ;
/*
  Read each item in the F definition as a token, and then
  take it apart.
*/
      for ( ;; ) {

        count = sscanf ( next, "%s%n", token2, &width );
        next = next + width;
 
        if ( count != 1 ) {
          break;
        }
 
        count = sscanf ( token2, "%d%n", &node, &width );
        next2 = token2 + width;

        if ( count != 1 ) {
          break;
        }

        if ( ivert >= 4 ) {
          break;
        }

        vlist[ivert] = 0 ;
        tlist[ivert] = 0 ;
        nlist[ivert] = 0 ;

        if ( 0 < node && node <= num_vert ) {
          vlist[ivert] = node-1;
        } 
/*
  If there's a slash, extract the index of the texture vector.
  If there's another slash, extract the index of the normal vector.
*/
        if ( *next2 == '/' ) {

          next3 = next2 + 1;
          count = sscanf ( next3, "%d%n", &node, &width );

          if ( 0 < node && node <= num_vert_tex ) {
            tlist[ivert] = node-1;
          }

          for ( ; next3 < token2 + MAX_LINE_LEN; next3++ ) {

            if ( *next3 == '/' ) {
              next3 = next3 + 1;
              count = sscanf ( next3, "%d%n", &node, &width );

              if ( 0 < node && node <= num_vert_normal ) {
                nlist[ivert] = node-1;
              }
              break;
            }
          }
        }
        ivert = ivert + 1;
      }

      if ( ivert >= 3 ) {

         //quad?
         if ( ivert == 4 ) {
            if ( num_face+1 < MAX_FACE ) {
              //0,1,3
                face[num_face].vlist[0] = vlist[0] ;
                face[num_face].tlist[0] = tlist[0] ;
                face[num_face].nlist[0] = nlist[0] ;
                face[num_face].vlist[1] = vlist[1] ;
                face[num_face].tlist[1] = tlist[1] ;
                face[num_face].nlist[1] = nlist[1] ;
                face[num_face].vlist[2] = vlist[3] ;
                face[num_face].tlist[2] = tlist[3] ;
                face[num_face].nlist[2] = nlist[3] ;
              face[num_face].mat_index = mat_current ;
              num_face = num_face + 1;
              //3,1,2
                face[num_face].vlist[0] = vlist[3] ;
                face[num_face].tlist[0] = tlist[3] ;
                face[num_face].nlist[0] = nlist[3] ;
                face[num_face].vlist[1] = vlist[1] ;
                face[num_face].tlist[1] = tlist[1] ;
                face[num_face].nlist[1] = nlist[1] ;
                face[num_face].vlist[2] = vlist[2] ;
                face[num_face].tlist[2] = tlist[2] ;
                face[num_face].nlist[2] = nlist[2] ;
              face[num_face].mat_index = mat_current ;
              num_face = num_face + 1;
            }
         }
         else {
            if ( num_face < MAX_FACE ) {
              for ( int i = 0 ; i < 3 ; i ++ ) {
                face[num_face].vlist[i] = vlist[i] ;
                face[num_face].tlist[i] = tlist[i] ;
                face[num_face].nlist[i] = nlist[i] ;
              }
              face[num_face].mat_index = mat_current ;
              num_face = num_face + 1;
            }
         }
      }
    }

/*  
  G  
  Group name.
*/

    else if ( leqi ( token, "G" ) == TRUE ) {
      continue;
    }
/*  
  L  
  I believe OBJ line node indices are 1 based rather than 0 based.
  So we have to decrement them before loading them into LINE_DEX.
*/

    else if ( leqi ( token, "L" ) == TRUE ) {

      int lastnode = -1 ;

      for ( ;; ) {

        count = sscanf ( next, "%d%n", &node, &width );
        next = next + width;

        if ( count != 1 ) {
          break;
        }

        if ( lastnode != -1 && num_line + 2 <= MAX_LINE  ) {

          line_dex[num_line] = lastnode-1;
          num_line = num_line + 1;
          line_dex[num_line] = node-1;
          num_line = num_line + 1;
        }

        lastnode = node ;
      } 
    }

/*
  LOD
  Level of detail.
*/
    else if ( leqi ( token, "LOD" ) == TRUE ) {
      continue;
    }
/*
  MG
  Merging group.
*/
    else if ( leqi ( token, "MG" ) == TRUE ) {
      continue;
    }
/*
  MTLLIB
  Material library.
*/

    else if ( leqi ( token, "MTLLIB" ) == TRUE ) {

      count = sscanf ( next, "%s%n", token2, &width );
      next = next + width;
 
      if ( count != 1 ) {
        continue;
      }

      load_materials ( token2 ) ;
      mat_current = 0 ;
    }
/*
  USEMTL  
  Material name.
*/
    else if ( leqi ( token, "USEMTL" ) == TRUE ) {

      mat_current = 0 ;

      count = sscanf ( next, "%s%n", token2, &width );
      next = next + width;
 
      if ( count != 1 ) {
        continue;
      }

      for ( int i = 0 ; i < num_mat ; i ++ )
         if ( strcmp ( token2, materials[ i ].name ) == 0 )
         {
            mat_current = i ;
            break ;
         }
    }
/*
  O
  Object name.
*/
    else if ( leqi ( token, "O" ) == TRUE ) {
      continue;
    }
/*
  P
  Point.
*/
    else if ( leqi ( token, "P" ) == TRUE ) {
      continue;
    }
/*
  SURF
  Surface.
*/
    else if ( leqi ( token, "SURF" ) == TRUE ) {
      continue;
    }
/*
  V X Y Z W
  Geometric vertex.
  W is optional, a weight for rational curves and surfaces.
  The default for W is 1.
*/
    else if ( leqi ( token, "V" ) == TRUE ) {

      sscanf ( next, "%e %e %e", &r1, &r2, &r3 );

      if ( num_vert < MAX_VERT ) {
        vert[num_vert][0] = r1;
        vert[num_vert][1] = r2;
        vert[num_vert][2] = r3;

        num_vert = num_vert + 1;
      }
    }
/*
  VT
  Vertex texture.
*/
    else if ( leqi ( token, "VT" ) == TRUE ) {

      sscanf ( next, "%e %e %e", &r1, &r2, &r3 );

      if ( num_vert_tex < MAX_VERT ) {
        vert_tex[num_vert_tex][0] = r1;
        vert_tex[num_vert_tex][1] = r2;
        vert_tex[num_vert_tex][2] = r3;

        num_vert_tex = num_vert_tex + 1;
      }
    }
/*
  VN
  Vertex normals.
*/
    else if ( leqi ( token, "VN" ) == TRUE ) {

      sscanf ( next, "%e %e %e", &r1, &r2, &r3 );

      if ( num_vert_normal < MAX_VERT ) {
        vert_normal[num_vert_normal][0] = r1;
        vert_normal[num_vert_normal][1] = r2;
        vert_normal[num_vert_normal][2] = r3;

        num_vert_normal = num_vert_normal + 1;
      }
    }
  }

  //create ssg nodes
  if ( num_face )
  {
    if ( num_mat ) {
      for ( int i=0; i<num_mat; i++ )
         add_mesh ( i );
    }
    else
      add_mesh ( 0 );
  }

  if ( num_line )
  {
    ssgVertexArray* vlist = new ssgVertexArray ( num_line ) ;
    for ( int i=0; i<num_line; i++ )
      vlist -> add ( vert[ line_dex[i] ] ) ;
    ssgVtxTable *vtab = new ssgVtxTable ( GL_LINES, vlist, 0, 0, 0 );
    current_branch -> addKid ( vtab ) ;
  }

  delete[] materials ;
  delete[] vert ;
  delete[] vert_tex ;
  delete[] vert_normal ;
  delete[] face ;
  delete[] line_dex ;

  return TRUE;
}

ssgEntity *ssgLoadOBJ ( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  const ssgLoaderOptions* current_options = ssgGetCurrentOptions () ;

  current_branch   = NULL ;

  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;

  FILE *loader_fd = fopen ( filename, "ra" ) ;

  if ( loader_fd == NULL )
  {
    ulSetError ( UL_WARNING, "ssgLoadOBJ: Failed to open '%s' for reading", filename ) ;
    return NULL ;
  }

  current_branch = new ssgTransform () ;

  obj_read ( loader_fd ) ;

  fclose ( loader_fd ) ;

  return current_branch ;
}
