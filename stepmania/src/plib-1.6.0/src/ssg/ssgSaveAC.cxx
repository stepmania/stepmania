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
#include "ssgLoaderWriterStuff.h"

static ssgSimpleStateArray gSSL ;
static const int writeTextureWithoutPath = TRUE ;

struct saveTriangle
{
  int    v [ 3 ] ;
  sgVec2 t [ 3 ] ;
} ;

static FILE       *save_fd ;
static sgVec3       *vlist ;
static saveTriangle *tlist ;


static int ssgSaveLeaf ( ssgEntity *ent )
{ int i;
  assert ( ent -> isAKindOf ( ssgTypeLeaf() ) ) ;

  ssgLeaf *vt = (ssgLeaf *) ent ;

  int num_verts = vt -> getNumVertices  () ;
  int num_tris  = vt -> getNumTriangles () ;

  vlist = new sgVec3 [ num_verts ] ;
  tlist = new saveTriangle [ num_tris ] ;

  for ( i = 0 ; i < num_verts; i++ )
    sgCopyVec3 ( vlist[i], vt->getVertex ( i ) ) ;

  for ( i = 0 ; i < num_tris ; i++ )
  {
    short vv0, vv1, vv2 ;

    vt -> getTriangle ( i, &vv0, &vv1, &vv2 ) ;

    tlist[i].v[0] = vv0 ;
    tlist[i].v[1] = vv1 ;
    tlist[i].v[2] = vv2 ;

    sgCopyVec2 ( tlist[i].t[0], vt->getTexCoord ( vv0 ) ) ;
    sgCopyVec2 ( tlist[i].t[1], vt->getTexCoord ( vv1 ) ) ;
    sgCopyVec2 ( tlist[i].t[2], vt->getTexCoord ( vv2 ) ) ;
  }

  fprintf ( save_fd, "OBJECT poly\n" ) ;
  fprintf ( save_fd, "name \"%s\"\n", ent->getPrintableName() ) ;

  ssgState* st = vt -> getState () ;

  if ( st && st -> isAKindOf ( ssgTypeSimpleState() ) )
  {
    ssgSimpleState* ss = (ssgSimpleState*) vt -> getState () ;

    if ( ss -> isEnabled ( GL_TEXTURE_2D ) )
    {
      const char* tfname = ss -> getTextureFilename () ;

      if (( tfname != NULL ) && ( tfname[0] != 0 ))
      {
        if ( writeTextureWithoutPath )
        {
          char *s = strrchr ( tfname, '\\' ) ;

          if ( s == NULL )
            s = strrchr ( tfname, '/' ) ;

          if ( s == NULL )
            fprintf ( save_fd, "texture \"%s\"\n", tfname ) ;
          else
            fprintf ( save_fd, "texture \"%s\"\n", ++s ) ;
        }
        else
          fprintf ( save_fd, "texture \"%s\"\n", tfname ) ;
      }
    }
  }

  fprintf ( save_fd, "numvert %d\n", num_verts ) ;
  
  for ( i = 0 ; i < num_verts ; i++ )
    fprintf ( save_fd, "%g %g %g\n", vlist[i][0],vlist[i][2],-vlist[i][1] ) ;

  fprintf ( save_fd, "numsurf %d\n", num_tris ) ;

  for ( i = 0 ; i < num_tris ; i++ )
  {
    fprintf ( save_fd, "SURF 0x0\n" ) ;
    ssgState *s = vt->getState ();

    int istate = 0;

    if ( s != NULL )
      if ( s -> isAKindOf ( ssgTypeSimpleState() ) )
      {
        istate = gSSL.findIndex ( (ssgSimpleState *) s ) ;
        assert ( istate >= 0 ) ;
      }

    fprintf ( save_fd, "mat %d\n", istate ) ;
    fprintf ( save_fd, "refs 3\n" ) ;
    fprintf ( save_fd, "%d %g %g\n",
                  tlist[i].v[0],tlist[i].t[0][0],tlist[i].t[0][1] ) ;
    fprintf ( save_fd, "%d %g %g\n",
                  tlist[i].v[1],tlist[i].t[1][0],tlist[i].t[1][1] ) ;
    fprintf ( save_fd, "%d %g %g\n",
                  tlist[i].v[2],tlist[i].t[2][0],tlist[i].t[2][1] ) ;
  } 

  fprintf ( save_fd, "kids 0\n" ) ;

  delete [] vlist ;
  delete [] tlist ;

  return TRUE ;
}



static int ssgSaveACInner ( ssgEntity *ent )
{
  /* WARNING - RECURSIVE! */

  if ( ent -> isAKindOf ( ssgTypeBranch() ) )
  {
    ssgBranch *br = (ssgBranch *) ent ;

    fprintf ( save_fd, "OBJECT group\n" ) ;
    fprintf ( save_fd, "kids %d\n", ent->getNumKids() ) ;

    for ( int i = 0 ; i < br -> getNumKids () ; i++ )
      if ( ! ssgSaveACInner( br -> getKid ( i ) ) )
        return FALSE ;

    return TRUE ;
  }
 
  return ssgSaveLeaf ( ent ) ;
}




int ssgSaveAC ( const char *filename, ssgEntity *ent )
{ 
  int i ;
  
  save_fd = fopen ( filename, "wa" ) ;

  if ( save_fd == NULL )
  {
    ulSetError ( UL_WARNING,
                 "ssgSaveAC: Failed to open '%s' for writing", filename ) ;
    return FALSE ;
  }

  fprintf ( save_fd, "AC3Db\n" ) ;

  gSSL.collect( ent );

  /*
    There is a *huge* problem here.

    The AC3D format doesn't support per-vertex or even
    per-polygon colours.

    You *must* use materials.

    Hence what we really should do is to collect together
    all the per-vertex colours in the SSG model and make
    a material from each one (possibly more if that colour
    is on many polygons which have different ssgSimpleState's.

    For now, I'll cheat and set the GL_COLORMATERIAL colours
    to white - which works *reasonably* well for most heavily
    textured models...well, at least you don't get uninitialised
    colours coming from SSG.
  */

  for (i = 0 ; i < gSSL.getNum(); i++)
  {
    sgVec4 white = { 1, 1, 1, 1 } ;

    ssgSimpleState * ss = gSSL.get(i);

    float *em = ss->getMaterial (GL_EMISSION );
    float *sp = ss->getMaterial (GL_SPECULAR );
    float *am = ss->getMaterial (GL_AMBIENT  );
    float *di = ss->getMaterial (GL_DIFFUSE  );

    if ( ss->isEnabled ( GL_COLOR_MATERIAL ) )
    {
      switch ( ss->getColourMaterial () )
      {
        case GL_AMBIENT_AND_DIFFUSE : am = di = white ; break ;
        case GL_EMISSION : em = white ; break ;
        case GL_SPECULAR : sp = white ; break ;
        case GL_AMBIENT  : am = white ; break ;
        case GL_DIFFUSE  : di = white ; break ;
      }
    }

    int shiny = (int) ss->getShininess ();

    fprintf ( save_fd, "MATERIAL \"%s\""
                       " rgb %f %f %f"
                       " amb %f %f %f"
                       " emis %f %f %f"
                       " spec %f %f %f"
                       " shi %d  trans %f\n",
                       ss->getPrintableName(),
                       di[0], di[1], di[2],
                       am[0], am[1], am[2],
                       em[0], em[1], em[2],
                       sp[0], sp[1], sp[2],
                       shiny, 1.0-di[3] ) ;
  }

  fprintf ( save_fd, "OBJECT world\n" ) ;
  fprintf ( save_fd, "kids 1\n" ) ;

  int bReturn = ssgSaveACInner ( ent ) ;
  
  gSSL.removeAll () ;

  fclose ( save_fd ) ;

  return bReturn ;
}


