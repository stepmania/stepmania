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


#include "ssgLocal.h"

static FILE *loader_fd ;

struct _ssgMaterial
{
  sgVec4 spec ;
  sgVec4 emis ;
  sgVec4 rgb  ;
  float  shi  ;
} ;

static int num_materials = 0 ;
static sgVec3 *vtab = NULL ;

static ssgLoaderOptions* current_options = NULL ;
static _ssgMaterial    *current_material = NULL ;
static sgVec4          *current_colour   = NULL ;
static ssgBranch       *current_branch   = NULL ;
static char            *current_tfname   = NULL ;
static char            *current_data     = NULL ;

#define MAX_MATERIALS 1000    /* This *ought* to be enough! */
static _ssgMaterial   *mlist    [ MAX_MATERIALS ] ;
static sgVec4         *clist    [ MAX_MATERIALS ] ;

static sgMat4 current_matrix ;
static sgVec2 texrep ;
static sgVec2 texoff ;

static int do_material ( char *s ) ;
static int do_object   ( char *s ) ;
static int do_name     ( char *s ) ;
static int do_data     ( char *s ) ;
static int do_texture  ( char *s ) ;
static int do_texrep   ( char *s ) ;
static int do_texoff   ( char *s ) ;
static int do_rot      ( char *s ) ;
static int do_loc      ( char *s ) ;
static int do_url      ( char *s ) ;
static int do_numvert  ( char *s ) ;
static int do_numsurf  ( char *s ) ;
static int do_surf     ( char *s ) ;
static int do_mat      ( char *s ) ;
static int do_refs     ( char *s ) ;
static int do_kids     ( char *s ) ;

/*static int do_obj_world ( char *s ) ;
static int do_obj_poly  ( char *s ) ;
static int do_obj_group ( char *s ) ;
static int do_obj_light ( char *s ) ;*/

#define PARSE_CONT   0
#define PARSE_POP    1

struct Tag
{
  const char *token ;
  int (*func) ( char *s ) ;
} ;

 
static void skip_spaces ( char **s )
{
  while ( **s == ' ' || **s == '\t' )
    (*s)++ ;
}


static void skip_quotes ( char **s )
{
  skip_spaces ( s ) ;

  if ( **s == '\"' )
  {
    (*s)++ ;

    char *t = *s ;

    while ( *t != '\0' && *t != '\"' )
      t++ ;

    if ( *t != '\"' )
      ulSetError ( UL_WARNING, "ac_to_gl: Mismatched double-quote ('\"') in '%s'", *s ) ;

    *t = '\0' ;
  }
  else
    ulSetError ( UL_WARNING, "ac_to_gl: Expected double-quote ('\"') in '%s'", *s ) ;
}



static int search ( Tag *tags, char *s )
{
  skip_spaces ( & s ) ;

  for ( int i = 0 ; tags[i].token != NULL ; i++ )
    if ( ulStrNEqual ( tags[i].token, s, strlen(tags[i].token) ) )
    {
      s += strlen ( tags[i].token ) ;

      skip_spaces ( & s ) ;

      return (*(tags[i].func))( s ) ;
    }

  ulSetError ( UL_FATAL, "ac_to_gl: Unrecognised token '%s'", s ) ;

  return 0 ;  /* Should never get here */
}

static Tag top_tags [] =
{
  { "MATERIAL", do_material },
  { "OBJECT"  , do_object   },
} ;


static Tag object_tags [] =
{
  { "name"    , do_name     },
  { "data"    , do_data     },
  { "texture" , do_texture  },
  { "texrep"  , do_texrep   },
  { "texoff"  , do_texoff   },
  { "rot"     , do_rot      },
  { "loc"     , do_loc      },
  { "url"     , do_url      },
  { "numvert" , do_numvert  },
  { "numsurf" , do_numsurf  },
  { "kids"    , do_kids     },
  { NULL, NULL }
} ;

static Tag surf_tag [] =
{
  { "SURF"    , do_surf     },
  { NULL, NULL }
} ;

static Tag surface_tags [] =
{
  { "mat"     , do_mat      },
  { "refs"    , do_refs     },
  { NULL, NULL }
} ;

/*static Tag obj_type_tags [] =
{
  { "world", do_obj_world },
  { "poly" , do_obj_poly  },
  { "group", do_obj_group },
  { "light", do_obj_light },
  { NULL, NULL }
} ;*/


#define OBJ_WORLD  0
#define OBJ_POLY   1
#define OBJ_GROUP  2
#define OBJ_LIGHT  3

/*static int do_obj_world ( char * ) { return OBJ_WORLD ; } 
static int do_obj_poly  ( char * ) { return OBJ_POLY  ; }
static int do_obj_group ( char * ) { return OBJ_GROUP ; }
static int do_obj_light ( char * ) { return OBJ_LIGHT ; }*/

static int last_num_kids    = -1 ;
static int current_flags    = -1 ;

static ssgState *get_state ( _ssgMaterial *mat )
{
  if (current_tfname != NULL) {
    ssgState *st = current_options -> createState ( current_tfname ) ;
    if ( st != NULL )
      return st ;
  }

  ssgSimpleState *st = new ssgSimpleState () ;

  st -> setMaterial ( GL_SPECULAR, mat -> spec ) ;
  st -> setMaterial ( GL_EMISSION, mat -> emis ) ;
  st -> setShininess ( mat -> shi ) ;

  st -> enable ( GL_COLOR_MATERIAL ) ;
  st -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

  st -> enable  ( GL_LIGHTING ) ;
  st -> setShadeModel ( GL_SMOOTH ) ;

  bool has_alpha = false ;

  if ( current_tfname != NULL )
  {
    ssgTexture *tex = current_options -> createTexture ( current_tfname ) ;
    has_alpha = tex -> hasAlpha () ;
    st -> setTexture( tex ) ;
    st -> enable( GL_TEXTURE_2D ) ;
  }
  else
  {
    st -> disable( GL_TEXTURE_2D ) ;
  }

  if ( mat -> rgb[3] < 0.99 || has_alpha )
  {
    st -> disable ( GL_ALPHA_TEST ) ;
    st -> enable  ( GL_BLEND ) ;
    st -> setTranslucent () ;
  }
  else
  {
    st -> disable ( GL_ALPHA_TEST ) ;
    st -> disable ( GL_BLEND ) ;
    st -> setOpaque () ;
  }

  return st ;
}


static int do_material ( char *s )
{
  char name [ 1024 ] ;
  sgVec4 rgb  ;
  sgVec4 amb  ;
  sgVec4 emis ;
  sgVec4 spec ;
  int   shi ;
  float trans ;

  if ( sscanf ( s,
  "%s rgb %f %f %f amb %f %f %f emis %f %f %f spec %f %f %f shi %d trans %f",
    name,
    &rgb [0], &rgb [1], &rgb [2],
    &amb [0], &amb [1], &amb [2],
    &emis[0], &emis[1], &emis[2],
    &spec[0], &spec[1], &spec[2],
    &shi,
    &trans ) != 15 )
  {
    ulSetError ( UL_WARNING, "ac_to_gl: Can't parse this MATERIAL:" ) ;
    ulSetError ( UL_WARNING, "ac_to_gl: MATERIAL %s", s ) ;
  }
  else
  {
    char *nm = name ;

    skip_quotes ( &nm ) ;

    amb [ 3 ] = emis [ 3 ] = spec [ 3 ] = 1.0f ;
    rgb [ 3 ] = 1.0f - trans ;

    mlist [ num_materials ] = new _ssgMaterial ;
    clist [ num_materials ] = new sgVec4 [ 1 ] ;

    sgCopyVec4 ( clist [ num_materials ][ 0 ], rgb ) ;

    current_material = mlist [ num_materials ] ;
    sgCopyVec4 ( current_material -> spec, spec ) ;
    sgCopyVec4 ( current_material -> emis, emis ) ;
    sgCopyVec4 ( current_material -> rgb , rgb  ) ;
    current_material -> shi = (float) shi ;
  }

  num_materials++ ;
  return PARSE_CONT ;
}


static int do_object   ( char *  /* s */ )
{
/*
  int obj_type = search ( obj_type_tags, s ) ;  
*/

  delete [] current_tfname ;
  current_tfname = NULL ;

  char buffer [ 1024 ] ;

  sgSetVec2 ( texrep, 1.0f, 1.0f ) ;
  sgSetVec2 ( texoff, 0.0f, 0.0f ) ;

  sgMakeIdentMat4 ( current_matrix ) ;

  ssgEntity *old_cb = current_branch ;

  ssgTransform *tr = new ssgTransform () ;

  tr -> setTransform ( current_matrix ) ;

  current_branch -> addKid ( tr ) ;
  current_branch = tr ;

  while ( fgets ( buffer, 1024, loader_fd ) != NULL )
    if ( search ( object_tags, buffer ) == PARSE_POP )
      break ;

  int num_kids = last_num_kids ;

  for ( int i = 0 ; i < num_kids ; i++ )
  {
    fgets ( buffer, 1024, loader_fd ) ;
    search ( top_tags, buffer ) ;
  }

  current_branch = (ssgBranch *) old_cb ;
  return PARSE_CONT ;
}


static int do_name ( char *s )
{
  skip_quotes ( &s ) ;

  current_branch -> setName ( s ) ;

  return PARSE_CONT ;
}


static int do_data     ( char *s )
{
  int len = strtol ( s, NULL, 0 ) ;

  current_data = new char [ len + 1 ] ;

  for ( int i = 0 ; i < len ; i++ )
    current_data [ i ] = getc ( loader_fd ) ;

  current_data [ len ] = '\0' ;

  getc ( loader_fd ) ;  /* Final RETURN */

  ssgBranch *br = current_options -> createBranch ( current_data ) ;

  if ( br != NULL )
  {
    current_branch -> addKid ( br ) ;
    current_branch = br ;
  }

  /* delete [] current_data ; */
  current_data = NULL ;

  return PARSE_CONT ;
}


static int do_texture  ( char *s )
{
  skip_quotes ( &s ) ;

  delete [] current_tfname ;

  if ( s == NULL || s[0] == '\0' )
    current_tfname = NULL ;
  else
  {
    current_tfname = new char [ strlen(s)+1 ] ;
    strcpy ( current_tfname, s ) ;
  }

  return PARSE_CONT ;
}


static int do_texrep ( char *s )
{
  if ( sscanf ( s, "%f %f", & texrep [ 0 ], & texrep [ 1 ] ) != 2 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal texrep record." ) ;

  return PARSE_CONT ;
}


static int do_texoff ( char *s )
{
  if ( sscanf ( s, "%f %f", & texoff [ 0 ], & texoff [ 1 ] ) != 2 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal texoff record." ) ;

  return PARSE_CONT ;
}

static int do_rot ( char *s )
{
  current_matrix [ 0 ][ 3 ] = current_matrix [ 1 ][ 3 ] = current_matrix [ 2 ][ 3 ] =
    current_matrix [ 3 ][ 0 ] = current_matrix [ 3 ][ 1 ] = current_matrix [ 3 ][ 2 ] = 0.0f ;
  current_matrix [ 3 ][ 3 ] = 1.0f ; 

  if ( sscanf ( s, "%f %f %f %f %f %f %f %f %f",
        & current_matrix [ 0 ] [ 0 ], & current_matrix [ 0 ] [ 1 ], & current_matrix [ 0 ] [ 2 ],
        & current_matrix [ 1 ] [ 0 ], & current_matrix [ 1 ] [ 1 ], & current_matrix [ 1 ] [ 2 ],
        & current_matrix [ 2 ] [ 0 ], & current_matrix [ 2 ] [ 1 ], & current_matrix [ 2 ] [ 2 ] ) != 9 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal rot record." ) ;

  ((ssgTransform *)current_branch) -> setTransform ( current_matrix ) ;
  return PARSE_CONT ;
}

static int do_loc      ( char *s )
{
  if ( sscanf ( s, "%f %f %f", & current_matrix [ 3 ][ 0 ], & current_matrix [ 3 ][ 2 ], & current_matrix [ 3 ][ 1 ] ) != 3 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal loc record." ) ;

  current_matrix [ 3 ][ 1 ] = - current_matrix [ 3 ][ 1 ] ;
  current_matrix [ 3 ][ 3 ] = 1.0f ;
  ((ssgTransform *)current_branch) -> setTransform ( current_matrix ) ;

  return PARSE_CONT ;
}

static int do_url      ( char *s )
{
  skip_quotes ( & s ) ;

#ifdef PRINT_URLS
  ulSetError ( UL_DEBUG, "/* URL: \"%s\" */\n", s ) ;
#endif

  return PARSE_CONT ;
}

static int do_numvert  ( char *s )
{
  char buffer [ 1024 ] ;

  int nv = strtol ( s, NULL, 0 ) ;
 
  delete [] vtab ;

  vtab = new sgVec3 [ nv ] ;

  for ( int i = 0 ; i < nv ; i++ )
  {
    fgets ( buffer, 1024, loader_fd ) ;

    if ( sscanf ( buffer, "%f %f %f",
                          &vtab[i][0], &vtab[i][1], &vtab[i][2] ) != 3 )
    {
      ulSetError ( UL_FATAL, "ac_to_gl: Illegal vertex record." ) ;
    }

    float tmp  =  vtab[i][1] ;
    vtab[i][1] = -vtab[i][2] ;
    vtab[i][2] = tmp ;
  }

  return PARSE_CONT ;
}

static int do_numsurf  ( char *s )
{
  int ns = strtol ( s, NULL, 0 ) ;

  for ( int i = 0 ; i < ns ; i++ )
  {
    char buffer [ 1024 ] ;

    fgets ( buffer, 1024, loader_fd ) ;
    search ( surf_tag, buffer ) ;
  }

  return PARSE_CONT ;
}

static int do_surf     ( char *s )
{
  current_flags = strtol ( s, NULL, 0 ) ;

  char buffer [ 1024 ] ;

  while ( fgets ( buffer, 1024, loader_fd ) != NULL )
    if ( search ( surface_tags, buffer ) == PARSE_POP )
      break ;

  return PARSE_CONT ;
}


static int do_mat ( char *s )
{
  int mat = strtol ( s, NULL, 0 ) ;

  current_material = mlist [ mat ] ;
  current_colour   = clist [ mat ] ;

  return PARSE_CONT ;
}


static int do_refs     ( char *s )
{
  int nrefs = strtol ( s, NULL, 0 ) ;
  char buffer [ 1024 ] ;

  if ( nrefs == 0 )
    return PARSE_POP ;

  ssgVertexArray   *vlist = new ssgVertexArray ( nrefs ) ;
  ssgTexCoordArray *tlist = new ssgTexCoordArray ( nrefs ) ;
 
  for ( int i = 0 ; i < nrefs ; i++ )
  {
    fgets ( buffer, 1024, loader_fd ) ;

    int vtx ;
    sgVec2 tc ;

    if ( sscanf ( buffer, "%d %f %f", &vtx,
                                      &tc[0],
                                      &tc[1] ) != 3 )
    {
      ulSetError ( UL_FATAL, "ac_to_gl: Illegal ref record." ) ;
    }

    tc[0] *= texrep[0] ;
    tc[1] *= texrep[1] ;
    tc[0] += texoff[0] ;
    tc[1] += texoff[1] ;

    tlist -> add ( tc ) ;
    vlist -> add ( vtab[vtx] ) ;
  }

  ssgNormalArray *nrm = new ssgNormalArray ( 1 ) ;
  ssgColourArray *col = new ssgColourArray ( 1 ) ;

  col -> add ( *current_colour ) ;

  sgVec3 nm ;

  if ( nrefs < 3 )
    sgSetVec3 ( nm, 0.0f, 0.0f, 1.0f ) ;
  else
    sgMakeNormal ( nm, vlist->get(0), vlist->get(1), vlist->get(2) ) ;

  nrm -> add ( nm ) ;

  int type = ( current_flags & 0x0F ) ;
  if ( type >= 0 && type <= 2 )
  {
    GLenum gltype = GL_TRIANGLES ;
    switch ( type )
    {
      case 0 : gltype = GL_TRIANGLE_FAN ;
               break ;
      case 1 : gltype = GL_LINE_LOOP ;
               break ;
      case 2 : gltype = GL_LINE_STRIP ;
               break ;
    }

    ssgVtxTable* vtab = new ssgVtxTable ( gltype,
      vlist, nrm, tlist, col ) ;
    vtab -> setState ( get_state ( current_material ) ) ;
    vtab -> setCullFace ( ! ( (current_flags>>4) & 0x02 ) ) ;

    ssgLeaf* leaf = current_options -> createLeaf ( vtab, 0 ) ;

    if ( leaf )
       current_branch -> addKid ( leaf ) ;
  }

  return PARSE_POP ;
}

static int do_kids ( char *s )
{
  last_num_kids = strtol ( s, NULL, 0 ) ;

  return PARSE_POP ;
}


ssgEntity *ssgLoadAC3D ( const char *fname, const ssgLoaderOptions* options )
{
  ssgEntity *obj = ssgLoadAC ( fname, options ) ;

  if ( obj == NULL )
    return NULL ;

  /* Do some simple optimisations */

  ssgBranch *model = new ssgBranch () ;
  model -> addKid ( obj ) ;
  ssgFlatten      ( obj ) ;
  ssgStripify   ( model ) ;
  return model ;
}

/*
  Original function for backwards compatibility...
*/

ssgEntity *ssgLoadAC ( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;

  num_materials = 0 ;
  vtab = NULL ;

  current_material = NULL ;
  current_colour   = NULL ;
  current_tfname   = NULL ;
  current_branch   = NULL ;

  sgSetVec2 ( texrep, 1.0, 1.0 ) ;
  sgSetVec2 ( texoff, 0.0, 0.0 ) ;

  loader_fd = fopen ( filename, "ra" ) ;

  if ( loader_fd == NULL )
  {
    ulSetError ( UL_WARNING, "ssgLoadAC: Failed to open '%s' for reading", filename ) ;
    return NULL ;
  }

  char buffer [ 1024 ] ;
  int firsttime = TRUE ;

  current_branch = new ssgTransform () ;

  while ( fgets ( buffer, 1024, loader_fd ) != NULL )
  {
    char *s = buffer ;

    /* Skip leading whitespace */

    skip_spaces ( & s ) ;

    /* Skip blank lines and comments */

    if ( *s < ' ' && *s != '\t' ) continue ;
    if ( *s == '#' || *s == ';' ) continue ;

    if ( firsttime )
    {
      firsttime = FALSE ;

      if ( ! ulStrNEqual ( s, "AC3D", 4 ) )
      {
        fclose ( loader_fd ) ;
        ulSetError ( UL_WARNING, "ssgLoadAC: '%s' is not in AC3D format.", filename ) ;
        return NULL ;
      }
    }
    else
      search ( top_tags, s ) ;
  }

  delete [] current_tfname ;
  current_tfname = NULL ;
  delete [] vtab ;
  fclose ( loader_fd ) ;

  return current_branch ;
}

