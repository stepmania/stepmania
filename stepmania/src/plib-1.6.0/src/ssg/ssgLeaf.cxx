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

//~~ T.G. moved hee from header amd ref counting accounted for
 
void ssgLeaf::setState ( ssgState *st )
{
    ssgDeRefDelete ( state ) ;

    state = st ;

    if ( state != NULL )
      state->ref() ;
}

void ssgLeaf::copy_from ( ssgLeaf *src, int clone_flags )
{
  ssgEntity::copy_from ( src, clone_flags ) ;

  cull_face = src -> getCullFace () ;

  ssgState *s = src -> getState () ;

  //~~ T.G. Deref
  ssgDeRefDelete(state); 

  if ( s != NULL && ( clone_flags & SSG_CLONE_STATE ) )
    state = (ssgState *)( s -> clone ( clone_flags ) ) ;
  else
    state = s ;

   //~~ T.G. increment ref counter 
   if (state != NULL)  
       state->ref(); 
}



ssgLeaf::ssgLeaf (void)
{
  cull_face = TRUE ;
  state = NULL ;
  type = ssgTypeLeaf () ;

#ifdef _SSG_USE_DLIST
  dlist = 0 ;
#endif

  preDrawCB = NULL ;
  postDrawCB = NULL ;
}

ssgLeaf::~ssgLeaf (void)
{
  //~~ T.G. ssgDeRefDelete checks for null case
  ssgDeRefDelete ( state ) ;

#ifdef _SSG_USE_DLIST
  deleteDList () ;
#endif
}


#ifdef _SSG_USE_DLIST
void ssgLeaf::deleteDList ()
{
  if ( dlist != 0 )
    glDeleteLists ( dlist, 1 ) ;

  dlist = 0 ;
}

void ssgLeaf::makeDList ()
{
  deleteDList () ;  /* Just to be sure */
  dlist = glGenLists ( 1 ) ;
  glNewList ( dlist, GL_COMPILE ) ;
    draw_geometry () ;
  glEndList () ; 
}
#endif // #ifdef _SSG_USE_DLIST


void ssgLeaf::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  int cull_result = cull_test ( f, m, test_needed ) ;

  if ( cull_result == SSG_OUTSIDE )
    return ;

  if ( isTranslucent () )
    _ssgDrawLeaf ( this ) ;
  else
    draw () ;
}

void ssgLeaf::hot ( sgVec3 s, sgMat4 m, int test_needed )
{
  int hot_result = hot_test ( s, m, test_needed ) ;

  if ( hot_result == SSG_OUTSIDE )
    return ;

  /* Add polygons to hit list! */

  hot_triangles ( s, m, hot_result != SSG_INSIDE ) ;
}

void ssgLeaf::los ( sgVec3 s, sgMat4 m, int test_needed )
{
  int los_result = los_test ( s, m, test_needed ) ;

  if ( los_result == SSG_OUTSIDE )
    return ;

  /* Add polygons to hit list! */

  los_triangles ( s, m, los_result != SSG_INSIDE ) ;
}



void ssgLeaf::isect ( sgSphere *s, sgMat4 m, int test_needed )
{
  int isect_result = isect_test ( s, m, test_needed ) ;

  if ( isect_result == SSG_OUTSIDE )
    return ;

  /* Add polygons to hit list! */

  isect_triangles ( s, m, isect_result != SSG_INSIDE ) ;
}


void ssgLeaf::print ( FILE *fd, char *indent, int how_much )
{
  if ( how_much == 0 ) 
    return ;

  ssgEntity::print ( fd, indent, how_much  ) ;

  if ( getNumParents () != getRef () )
    fprintf ( fd, "****** WARNING: Ref count doesn't equal parent count!\n" ) ;

  if ( state != NULL )
  {
    char in [ 100 ] ;
    sprintf ( in, "%s  ", indent );

    if ( how_much == 1 )
      fprintf ( fd, "%s  %s: %p\n", indent , state->getTypeName(), state) ;
    else
      state -> print ( fd, in, how_much ) ;
  }
  else
    fprintf ( fd, "%s  No State assigned to this node\n", indent ) ;
}


int ssgLeaf::preDraw ()
{
  if ( preDrawCB != NULL && ! (*preDrawCB)(this) )
    return FALSE ;

  _ssgCurrentContext->setCullface ( getCullFace() ) ;

  return TRUE ;
}


int ssgLeaf::load ( FILE *fd )
{
  _ssgReadInt ( fd, &cull_face ) ;

  ssgState *st;

  if ( ! _ssgLoadObject ( fd, (ssgBase **) &st, ssgTypeState () ) )
     return FALSE ;

  if ( st -> isAKindOf ( ssgTypeSimpleState () ) )
  {
    ssgSimpleState *ss = (ssgSimpleState *) st ;
    char *tfname = ss -> getTextureFilename () ;

    if ( tfname != NULL )
    {
      ssgState *new_st = ssgGetCurrentOptions () -> createState ( tfname ) ;

      if ( new_st != NULL )
        st = new_st ;
    }
  }

  setState ( st ) ;

  return ssgEntity::load(fd) ;
}


int ssgLeaf::save ( FILE *fd )
{
  _ssgWriteInt ( fd, cull_face ) ;

  if ( ! _ssgSaveObject ( fd, getState () ) )
     return FALSE ;

  return ssgEntity::save(fd) ;
}



