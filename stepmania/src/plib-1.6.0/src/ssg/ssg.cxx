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


#define _UL_GENERATE_CODE_
#include "ssgLocal.h"

#ifndef WIN32
#  if defined(macintosh)
#    include <agl.h>
#  elif defined(__APPLE__)
#    include <OpenGL/CGLCurrent.h>
#  else
#    include <GL/glx.h>
#  endif
#endif

static bool glIsValidContext ( void )
{
#if defined(CONSOLE)
  return true ;
#elif defined(WIN32)
  return ( wglGetCurrentContext () != NULL ) ;
#elif defined(macintosh)
  return ( aglGetCurrentContext () != NULL ) ;
#elif defined(__APPLE__)
  return ( CGLGetCurrentContext () != NULL ) ;
#else
  return ( glXGetCurrentContext () != NULL ) ;
#endif
}

sgMat4 _ssgOpenGLAxisSwapMatrix =
{
  {  1.0f,  0.0f,  0.0f,  0.0f },
  {  0.0f,  0.0f, -1.0f,  0.0f },
  {  0.0f,  1.0f,  0.0f,  0.0f },
  {  0.0f,  0.0f,  0.0f,  1.0f }
} ;

sgVec3 _ssgVertex000   = { 0.0f, 0.0f, 0.0f } ;
sgVec4 _ssgColourWhite = { 1.0f, 1.0f, 1.0f, 1.0f } ;
sgVec3 _ssgNormalUp    = { 0.0f, 0.0f, 1.0f } ;
sgVec2 _ssgTexCoord00  = { 0.0f, 0.0f } ;
short  _ssgIndex0      = 0;

static ssgLight _ssgLights [ 8 ] ;
static int      _ssgFrameCounter = 0 ;

int  ssgGetFrameCounter () { return _ssgFrameCounter ; }
void ssgSetFrameCounter ( int fc ) { _ssgFrameCounter = fc ; }

const char *ssgGetVersion ()
{
#ifdef VERSION
  return VERSION ;
#else
  return "Unknown" ;
#endif
}

void ssgDeRefDelete ( ssgBase *s )
{
  if ( s == NULL ) return ;

  s -> deRef () ;

  if ( s -> getRef () <= 0 )
    delete s ;
}

void ssgDelete ( ssgBranch *br )
{
  if ( br == NULL )
    return ;

  br -> removeAllKids () ;
  delete br ;
}

ssgLight *ssgGetLight ( int i )
{
  return &_ssgLights [ i ] ;
}

void ssgInit ()
{
  if ( ! glIsValidContext () )
  {
    ulSetError ( UL_FATAL, "ssgInit called without a valid OpenGL context.");
  }

  ssgTexturePath ( "." ) ;
  ssgModelPath   ( "." ) ;

  _ssgLights [ 0 ] . setID ( 0 ) ;
  _ssgLights [ 0 ] . on    () ;

  for ( int i = 1 ; i < 8 ; i++ )
  {
    _ssgLights [ i ] . setID ( i ) ;
    _ssgLights [ i ] . off   () ;
  }

  new ssgContext ;  /* Sets the current context with defaults */

  ssgAddModelFormat ( ".ssg",   ssgLoadSSG  , ssgSaveSSG ) ;

  ssgAddModelFormat ( ".3ds",   ssgLoad3ds  , ssgSave3ds ) ;
  ssgAddModelFormat ( ".ac" ,   ssgLoadAC3D , ssgSaveAC  ) ;
  ssgAddModelFormat ( ".ase",   ssgLoadASE  , ssgSaveASE ) ;
  ssgAddModelFormat ( ".dxf",   ssgLoadDXF  , ssgSaveDXF ) ;
  ssgAddModelFormat ( ".obj",   ssgLoadOBJ  , ssgSaveOBJ ) ;
  ssgAddModelFormat ( ".tri",   ssgLoadTRI  , ssgSaveTRI ) ;
  ssgAddModelFormat ( ".md2",   ssgLoadMD2  , NULL       ) ;
  ssgAddModelFormat ( ".x"  ,   ssgLoadX    , ssgSaveX   ) ;
  ssgAddModelFormat ( ".flt",   ssgLoadFLT  , NULL       ) ;
  ssgAddModelFormat ( ".strip", ssgLoadStrip, NULL       ) ;
  ssgAddModelFormat ( ".m"  ,   ssgLoadM    , ssgSaveM   ) ;
  ssgAddModelFormat ( ".off"  , ssgLoadOFF  , ssgSaveOFF ) ;
  ssgAddModelFormat ( ".atg"  , ssgLoadATG  , ssgSaveATG ) ;
  ssgAddModelFormat ( ".qhi"  , NULL        , ssgSaveQHI ) ;
  ssgAddModelFormat ( ".wrl",   ssgLoadVRML1, ssgSaveVRML1 ) ;
  ssgAddModelFormat ( ".iv", ssgLoadIV , NULL ) ;


#ifdef SSG_LOAD_MDL_SUPPORTED
  ssgAddModelFormat ( ".mdl",   ssgLoadMDL  , NULL       ) ;
#endif

#ifdef SSG_LOAD_TGA_SUPPORTED
  ssgAddTextureFormat ( ".tga" ,   ssgLoadTGA ) ;
#endif

#ifdef SSG_LOAD_BMP_SUPPORTED
  ssgAddTextureFormat ( ".bmp" ,   ssgLoadBMP ) ;
#endif

#ifdef SSG_LOAD_PNG_SUPPORTED
  ssgAddTextureFormat ( ".png" ,   ssgLoadPNG ) ;
#endif

#ifdef SSG_LOAD_SGI_SUPPORTED
  ssgAddTextureFormat ( ".rgb" ,   ssgLoadSGI ) ;
  ssgAddTextureFormat ( ".rgba" ,  ssgLoadSGI ) ;
  ssgAddTextureFormat ( ".int" ,   ssgLoadSGI ) ;
  ssgAddTextureFormat ( ".inta" ,  ssgLoadSGI ) ;
  ssgAddTextureFormat ( ".bw" ,    ssgLoadSGI ) ;
#endif

#if defined(SSG_LOAD_MDL_SUPPORTED) || defined(SSG_LOAD_MDL_BGL_TEXTURE_SUPPORTED)
  ssgAddTextureFormat ( ".0af" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".1af" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".2af" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".3af" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".4af" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".5af" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".6af" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".7af" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".8af" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".9af" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".aaf" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".baf" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".caf" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".daf" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".eaf" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".faf" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".gaf" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".haf" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".iaf" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".jaf" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".kaf" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".pat" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".r8"  ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".naz" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".ktx" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".oav" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".sky" ,   ssgLoadMDLTexture ) ;
  ssgAddTextureFormat ( ".ngt" ,   ssgLoadMDLTexture ) ;
#endif

}



void ssgCullAndPick ( ssgRoot *r, sgVec2 botleft, sgVec2 topright )
{
  if ( _ssgCurrentContext == NULL )
  {
    ulSetError ( UL_FATAL, "ssg: No Current Context: Did you forgot to call ssgInit()?" ) ;
  }

  ssgForceBasicState () ;

  GLint vp [ 4 ] ;
  sgVec4 viewport ;
  sgMat4 mat ;

  float w = (topright[0] - botleft[0]) ;
  float h = (topright[1] - botleft[1]) ;

  float x = (botleft[0] + topright[0]) / 2.0f ;
  float y = (botleft[1] + topright[1]) / 2.0f ;

  glGetIntegerv ( GL_VIEWPORT, vp ) ;

  sgSetVec4 ( viewport, (float)vp[0], (float)vp[1],
                        (float)vp[2], (float)vp[3] ) ;
  sgMakePickMatrix ( mat, x, y, w, h, viewport ) ;

  glMatrixMode ( GL_PROJECTION ) ;
  glLoadIdentity () ;
  glMultMatrixf ( (float *) mat ) ;

  _ssgCurrentContext->pushProjectionMatrix () ;

  glMatrixMode ( GL_MODELVIEW ) ;
  glLoadIdentity () ;

  int i ;

  for ( i = 0 ; i < 8 ; i++ )
    if ( _ssgLights [ i ] . isHeadlight () )
      _ssgLights [ i ] . setup () ;

  _ssgCurrentContext->loadModelviewMatrix () ;

  for ( i = 0 ; i < 8 ; i++ )
    if ( ! _ssgLights [ i ] . isHeadlight () )
      _ssgLights [ i ] . setup () ;

  _ssgCurrentContext->cull(r) ;
  _ssgDrawDList () ;

  glMatrixMode ( GL_MODELVIEW ) ;
  glLoadIdentity () ;
}


void ssgCullAndDraw ( ssgRoot *r )
{
  if ( _ssgCurrentContext == NULL )
  {
    ulSetError ( UL_FATAL, "ssg: No Current Context: Did you forgot to call ssgInit()?" ) ;
  }

  _ssgStartOfFrameInit () ;

  ssgForceBasicState () ;

  glMatrixMode ( GL_PROJECTION ) ;
  _ssgCurrentContext->loadProjectionMatrix () ;

  glMatrixMode ( GL_MODELVIEW ) ;
  glLoadIdentity () ;

  int i ;

  for ( i = 0 ; i < 8 ; i++ )
    if ( _ssgLights [ i ] . isHeadlight () )
      _ssgLights [ i ] . setup () ;

  _ssgCurrentContext->loadModelviewMatrix () ;
  _ssgCurrentContext->applyClipPlanes () ;

  for ( i = 0 ; i < 8 ; i++ )
    if ( ! _ssgLights [ i ] . isHeadlight () )
      _ssgLights [ i ] . setup () ;

  _ssgCurrentContext->cull(r) ;
  _ssgDrawDList () ;

  _ssgCurrentContext->removeClipPlanes () ;

  glMatrixMode ( GL_MODELVIEW ) ;
  glLoadIdentity () ;

  _ssgEndOfFrameCleanup () ;
  _ssgFrameCounter++ ;
}


const char *ssgAxisTransform::getTypeName (void) { return "ssgAxisTransform" ; }
const char *ssgBase         ::getTypeName (void) { return "ssgBase"          ; }
const char *ssgTexture      ::getTypeName (void) { return "ssgTexture"       ; }
const char *ssgState        ::getTypeName (void) { return "ssgState"         ; }
const char *ssgSimpleState  ::getTypeName (void) { return "ssgSimpleState"   ; }
const char *ssgStateSelector::getTypeName (void) { return "ssgStateSelector" ; }
const char *ssgEntity       ::getTypeName (void) { return "ssgEntity"        ; }
const char *ssgLeaf         ::getTypeName (void) { return "ssgLeaf"          ; }
const char *ssgVTable       ::getTypeName (void) { return "ssgVTable"        ; }
const char *ssgVtxTable     ::getTypeName (void) { return "ssgVtxTable"      ; }
const char *ssgVtxArray     ::getTypeName (void) { return "ssgVtxArray"      ; }
const char *ssgBranch       ::getTypeName (void) { return "ssgBranch"        ; }
const char *ssgSelector     ::getTypeName (void) { return "ssgSelector"      ; }
const char *ssgRangeSelector::getTypeName (void) { return "ssgRangeSelector" ; }
const char *ssgTimedSelector::getTypeName (void) { return "ssgTimedSelector" ; }
const char *ssgBaseTransform::getTypeName (void) { return "ssgBaseTransform" ; }
const char *ssgTransform    ::getTypeName (void) { return "ssgTransform"     ; }
const char *ssgTexTrans     ::getTypeName (void) { return "ssgTexTrans"      ; }
const char *ssgCutout       ::getTypeName (void) { return "ssgCutout"        ; }
const char *ssgRoot         ::getTypeName (void) { return "ssgRoot"          ; }

const char *ssgSimpleList   ::getTypeName (void) { return "ssgSimpleList"    ; }
const char *ssgColourArray  ::getTypeName (void) { return "ssgColourArray"   ; }
const char *ssgIndexArray   ::getTypeName (void) { return "ssgIndexArray"    ; }
const char *ssgTransformArray::getTypeName (void) { return "ssgTransformArray" ; }
const char *ssgTexCoordArray::getTypeName (void) { return "ssgTexCoordArray" ; }
const char *ssgVertexArray  ::getTypeName (void) { return "ssgVertexArray"   ; }
const char *ssgNormalArray  ::getTypeName (void) { return "ssgNormalArray"   ; }
const char *ssgInterleavedArray::getTypeName (void) { return "ssgInterleavedArray"; }



static ssgBase *createBase ()              { return new ssgBase             ; }

//static ssgBase *createEntity ()            { return new ssgEntity           ; }
//static ssgBase *createLeaf ()              { return new ssgLeaf             ; }
static ssgBase *createVTable ()            { return new ssgVTable           ; }
static ssgBase *createVtxTable ()          { return new ssgVtxTable         ; }
static ssgBase *createVtxArray ()          { return new ssgVtxArray         ; }
static ssgBase *createTween ()             { return new ssgTween            ; }
static ssgBase *createBranch ()            { return new ssgBranch           ; }
//static ssgBase *createBaseTransform ()     { return new ssgBaseTransform    ; }
static ssgBase *createTransform ()         { return new ssgTransform        ; }
static ssgBase *createTexTrans ()          { return new ssgTexTrans         ; }
static ssgBase *createAxisTransform()      { return new ssgAxisTransform; }
static ssgBase *createSelector ()          { return new ssgSelector         ; }
static ssgBase *createRangeSelector ()     { return new ssgRangeSelector    ; }
static ssgBase *createTimedSelector ()     { return new ssgTimedSelector    ; }
static ssgBase *createTweenController ()   { return new ssgTweenController  ; }
static ssgBase *createRoot ()              { return new ssgRoot             ; }
static ssgBase *createCutout ()            { return new ssgCutout           ; }
static ssgBase *createInvisible ()         { return new ssgInvisible        ; }

//static ssgBase *createState ()             { return new ssgState            ; }
static ssgBase *createSimpleState ()       { return new ssgSimpleState      ; }
static ssgBase *createStateSelector ()     { return new ssgStateSelector    ; }

static ssgBase *createSimpleList ()        { return new ssgSimpleList       ; }
static ssgBase *createVertexArray ()       { return new ssgVertexArray      ; }
static ssgBase *createNormalArray ()       { return new ssgNormalArray      ; }
static ssgBase *createTexCoordArray ()     { return new ssgTexCoordArray    ; }
static ssgBase *createColourArray ()       { return new ssgColourArray      ; }
static ssgBase *createIndexArray ()        { return new ssgIndexArray       ; }
static ssgBase *createTransformArray ()    { return new ssgTransformArray   ; }
static ssgBase *createInterleavedArray ()  { return new ssgInterleavedArray ; }

static ssgBase *createTexture ()           { return new ssgTexture          ; }


static struct {

  int type ;
  ssgBase * ( *func ) () ;

} table[256] = {

  { ssgTypeBase ()              , createBase              },

//{ ssgTypeEntity ()            , createEntity            },
//{ ssgTypeLeaf ()              , createLeaf              },
  { ssgTypeVTable ()            , createVTable            },
  { ssgTypeVtxTable ()          , createVtxTable          },
  { ssgTypeVtxArray ()          , createVtxArray          },
  { ssgTypeTween ()             , createTween             },
  { ssgTypeBranch ()            , createBranch            },
//{ ssgTypeBaseTransform ()     , createBaseTransform     },
  { ssgTypeTransform ()         , createTransform         },
  { ssgTypeTexTrans ()          , createTexTrans          },
	{ ssgTypeAxisTransform ()     , createAxisTransform     },
	{ ssgTypeSelector ()          , createSelector          },
  { ssgTypeRangeSelector ()     , createRangeSelector     },
  { ssgTypeTimedSelector ()     , createTimedSelector     },
  { ssgTypeTweenController ()   , createTweenController   },
  { ssgTypeRoot ()              , createRoot              },
  { ssgTypeCutout ()            , createCutout            },
  { ssgTypeInvisible ()         , createInvisible         },
  
//{ ssgTypeState ()             , createState             },
  { ssgTypeSimpleState ()       , createSimpleState       },
  { ssgTypeStateSelector ()     , createStateSelector     },
  
  { ssgTypeSimpleList ()        , createSimpleList        },
  { ssgTypeVertexArray ()       , createVertexArray       },
  { ssgTypeNormalArray ()       , createNormalArray       },
  { ssgTypeTexCoordArray ()     , createTexCoordArray     },
  { ssgTypeColourArray ()       , createColourArray       },
  { ssgTypeIndexArray ()        , createIndexArray        },
  { ssgTypeTransformArray ()    , createTransformArray    },
  { ssgTypeInterleavedArray ()  , createInterleavedArray  },
  
  { ssgTypeTexture ()           , createTexture           },
  
  { 0, NULL }

};

void ssgRegisterType ( int type, ssgBase * ( *func ) () )
{
  if ( type == 0 || func == NULL ) 
  {
    ulSetError ( UL_WARNING, "ssgRegisterType: Bad arguments (type %#x, func %p).",
		 type, func ) ;
    return ;
  }

  int i ;
  for ( i = 0 ; table[i].type != 0 && table[i].type != type ; i++ )
    ;

  if ( table[i].type == type && table[i].func != func )
    ulSetError ( UL_WARNING, "ssgRegisterType: Type %#x redefined differently.", type ) ;

  table[i].type = type ;
  table[i].func = func ;
}

ssgBase *ssgCreateOfType ( int type )
{
  // XXX linear search
  int i ;

  for ( i = 0 ; table[i].type != 0 && table[i].type != type ; i++ )
    ;

  if ( table[i].type == 0 ) 
  {
    ulSetError ( UL_WARNING, "ssgCreateOfType: Unrecognized type %#x.", type ) ;
    return NULL ;
  }

  ssgBase *obj = (*table[i].func) () ;

  if ( obj == NULL )
    ulSetError ( UL_WARNING, "ssgCreateOfType: Got null object for type %#x.", type ) ;
  else if ( obj -> getType () != type )
    ulSetError ( UL_WARNING,
		 "ssgCreateOfType: Created object has wrong type %#x (%s), expected %#x.",
		 obj -> getType (), obj -> getTypeName (), type ) ;

  return obj ;
}
