
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
#include "ul.h"

static float current_tween_state = 0.0f ;

const char *ssgTween::getTypeName (void) { return "ssgTween" ; }

float _ssgGetCurrentTweenState () { return current_tween_state ; }


void  _ssgSetCurrentTweenState ( float tstate )
{
  current_tween_state = tstate ;
}

void ssgTween::copy_from ( ssgTween *src, int clone_flags )
{
  ssgLeaf::copy_from ( src, clone_flags ) ;

  gltype = src -> getPrimitiveType () ;

  if ( src->vertices != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    vertices = (ssgVertexArray *)( src -> vertices -> clone ( clone_flags )) ;
  else
    vertices = src -> vertices ;

  if ( src->normals != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    normals = (ssgNormalArray *)( src -> normals -> clone ( clone_flags )) ;
  else
    normals = src -> normals ;

  if ( src->texcoords != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    texcoords = (ssgTexCoordArray *)( src -> texcoords -> clone ( clone_flags )) ;
  else
    texcoords = src -> texcoords ;

  if ( src->colours != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    colours = (ssgColourArray *)( src -> colours -> clone ( clone_flags )) ;
  else
    colours = src -> colours ;

  if ( vertices  != NULL ) vertices  -> ref () ;
  if ( normals   != NULL ) normals   -> ref () ;
  if ( texcoords != NULL ) texcoords -> ref () ;
  if ( colours   != NULL ) colours   -> ref () ;

  recalcBSphere () ;
}

ssgBase *ssgTween::clone ( int clone_flags )
{
  ssgTween *b = new ssgTween ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


void ssgTween::init ()
{
  curr_bank = 0 ;
  type = ssgTypeTween () ;

  render_vertices  = new ssgVertexArray   () ;
  render_normals   = new ssgNormalArray   () ;
  render_texcoords = new ssgTexCoordArray () ;
  render_colours   = new ssgColourArray   () ;

  render_vertices  -> ref () ;
  render_normals   -> ref () ;
  render_texcoords -> ref () ;
  render_colours   -> ref () ;

  banked_vertices  = new ulList ( 2 ) ;
  banked_normals   = new ulList ( 2 ) ;
  banked_texcoords = new ulList ( 2 ) ;
  banked_colours   = new ulList ( 2 ) ;

  vertices  = render_vertices  ;
  normals   = render_normals   ;
  texcoords = render_texcoords ;
  colours   = render_colours   ;

  recalcBSphere () ;
}


ssgTween::ssgTween ()
{
  init () ;
  gltype = GL_POINTS ;
}


ssgTween::ssgTween ( GLenum ty )
{
  init () ;
  gltype = ty ;
}


int ssgTween::newBank ( ssgVertexArray   *vl, ssgNormalArray   *nl,
                        ssgTexCoordArray *tl, ssgColourArray   *cl )
{
  int bank = banked_vertices -> getNumEntities () ;

  banked_vertices  -> addEntity ( vl != NULL ? vl :
                                ( banked_vertices -> getEntity(bank-1) ) ) ;

  banked_normals   -> addEntity ( nl != NULL ? nl :
                                ( banked_normals -> getEntity(bank-1) ) ) ;

  banked_texcoords -> addEntity ( tl != NULL ? tl :
                                ( banked_texcoords -> getEntity(bank-1) ) ) ;

  banked_colours   -> addEntity ( cl != NULL ? cl :
                                ( banked_colours -> getEntity(bank-1) ) ) ;

  setBank ( bank ) ;

  vertices  -> ref () ;
  normals   -> ref () ;
  texcoords -> ref () ;
  colours   -> ref () ;

  dirtyBSphere () ;  /* Cause parents to redo their bspheres */
  bsphere_is_invalid = TRUE ;
  return bank ;
}



int ssgTween::newBank ( int newVertices , int newNormals,
                        int newTexCoords, int newColours )
{
  return newBank ( newVertices  ? ( new ssgVertexArray   () ) : NULL,
                   newNormals   ? ( new ssgNormalArray   () ) : NULL,
                   newTexCoords ? ( new ssgTexCoordArray () ) : NULL,
                   newColours   ? ( new ssgColourArray   () ) : NULL ) ;
}


void ssgTween::setBank ( int bank )
{
  assert ( bank < banked_vertices -> getNumEntities () ) ;

  curr_bank = bank ;

  vertices  = (ssgVertexArray   *) banked_vertices  -> getEntity ( bank ) ;
  normals   = (ssgNormalArray   *) banked_normals   -> getEntity ( bank ) ;
  texcoords = (ssgTexCoordArray *) banked_texcoords -> getEntity ( bank ) ;
  colours   = (ssgColourArray   *) banked_colours   -> getEntity ( bank ) ;
}


void ssgTween::setVertices ( ssgVertexArray *vl )
{
  banked_vertices -> replaceEntity ( curr_bank, vl ) ;
  ssgVtxTable::setVertices ( vl ) ;
  dirtyBSphere () ;  /* Cause parents to redo their bspheres */
  bsphere_is_invalid = TRUE ;
}

void ssgTween::setNormals ( ssgNormalArray *nl )
{
  banked_normals -> replaceEntity ( curr_bank, nl ) ;
  ssgVtxTable::setNormals ( nl ) ;
  dirtyBSphere () ;  /* Cause parents to redo their bspheres */
  bsphere_is_invalid = TRUE ;
}

void ssgTween::setTexCoords ( ssgTexCoordArray *tl )
{
  banked_texcoords -> replaceEntity ( curr_bank, tl ) ;
  ssgVtxTable::setTexCoords ( tl ) ;
  dirtyBSphere () ;  /* Cause parents to redo their bspheres */
  bsphere_is_invalid = TRUE ;
}

void ssgTween::setColours ( ssgColourArray *cl )
{
  banked_colours -> replaceEntity ( curr_bank, cl ) ;
  ssgVtxTable::setColours ( cl ) ;
  dirtyBSphere () ;  /* Cause parents to redo their bspheres */
  bsphere_is_invalid = TRUE ;
}

ssgTween::~ssgTween ()
{
  for ( int i = 0 ; i < getNumBanks () ; i++ )
  {
    ssgDeRefDelete ( (ssgVertexArray   *) banked_vertices  -> getEntity (i)) ;
    ssgDeRefDelete ( (ssgNormalArray   *) banked_normals   -> getEntity (i)) ;
    ssgDeRefDelete ( (ssgTexCoordArray *) banked_texcoords -> getEntity (i)) ;
    ssgDeRefDelete ( (ssgColourArray   *) banked_colours   -> getEntity (i)) ;
  }

  delete banked_vertices  ;
  delete banked_normals   ;
  delete banked_texcoords ;
  delete banked_colours   ;

  ssgDeRefDelete ( render_vertices  ) ;
  ssgDeRefDelete ( render_normals   ) ;
  ssgDeRefDelete ( render_texcoords ) ;
  ssgDeRefDelete ( render_colours   ) ;
} 


void ssgTween::recalcBSphere ()
{
  emptyBSphere () ;
  bbox . empty () ;

  for ( int b = 0 ; b < banked_vertices -> getNumEntities () ; b++ )
  {
    ssgVertexArray *va = (ssgVertexArray *) banked_vertices -> getEntity ( b ) ;

    for ( int i = 0 ; i < va -> getNum() ; i++ )
      bbox . extend ( va->get(i) ) ;
  }

  extendBSphere ( & bbox ) ;
  dirtyBSphere () ;  /* Cause parents to redo their bspheres */
  bsphere_is_invalid = FALSE ;
}


void ssgTween::draw ()
{
  if ( ! preDraw () )
    return ;

  if ( hasState () ) getState () -> apply () ;

  stats_num_leaves++ ;
  stats_num_vertices += getNumVertices() ;

  float tstate = _ssgGetCurrentTweenState () ;
  int num_banks = banked_vertices -> getNumEntities () ;

  if ( tstate < 0.0f ) tstate = 0.0f ;

  int   state1 = (int) floor ( tstate ) ;
  int   state2 = state1 + 1 ;
  float tween  = tstate - (float) state1 ;

  if ( state1 >= num_banks ) state1 = num_banks - 1 ;
  if ( state2 >= num_banks ) state2 = num_banks - 1 ;
  if ( state1 == state2    ) tween  = 0.0f ;

  int l1, l2 ;

  /* Lerp the vertices... */

  ssgVertexArray *v1 = (ssgVertexArray *) banked_vertices->getEntity (state1) ; 
  ssgVertexArray *v2 = (ssgVertexArray *) banked_vertices->getEntity (state2) ; 
  l1 = v1 -> getNum () ;
  l2 = v2 -> getNum () ;

  assert ( l1 == l2 ) ;

  if ( render_vertices -> getNum () < l1 )
    render_vertices -> setNum ( l1 ) ;

  if ( v1 == v2 )
    vertices = v1 ;
  else
  {
    vertices = render_vertices ;

    for ( int i = 0 ; i < l1 ; i++ )
      sgLerpVec3 ( vertices->get(i),v1->get(i),v2->get(i), tween ) ;
  }

  /* Lerp the normals */

  ssgNormalArray *n1 = (ssgNormalArray *) banked_normals->getEntity (state1) ; 
  ssgNormalArray *n2 = (ssgNormalArray *) banked_normals->getEntity (state2) ; 
  l1 = n1 -> getNum () ;
  l2 = n2 -> getNum () ;

  assert ( l1 == l2 ) ;

  if ( render_normals -> getNum () < l1 )
    render_normals -> setNum ( l1 ) ;

  if ( n1 == n2 )
    normals = n1 ;
  else
  {
    normals = render_normals ;

    for ( int i = 0 ; i < l1 ; i++ )
      sgLerpVec3 ( normals->get(i),n1->get(i),n2->get(i), tween ) ;
  }

  /* Lerp the texcoords */

  ssgTexCoordArray *t1 = (ssgTexCoordArray *) banked_texcoords->getEntity (state1) ; 
  ssgTexCoordArray *t2 = (ssgTexCoordArray *) banked_texcoords->getEntity (state2) ; 
  l1 = t1 -> getNum () ;
  l2 = t2 -> getNum () ;

  assert ( l1 == l2 ) ;

  if ( render_texcoords -> getNum () < l1 )
    render_texcoords -> setNum ( l1 ) ;

  if ( t1 == t2 )
    texcoords = t1 ;
  else
  {
    texcoords = render_texcoords ;

    for ( int i = 0 ; i < l1 ; i++ )
      sgLerpVec2 ( texcoords->get(i),t1->get(i),t2->get(i), tween ) ;
  }

  /* Lerp the colours */

  ssgColourArray *c1 = (ssgColourArray *) banked_colours->getEntity (state1) ; 
  ssgColourArray *c2 = (ssgColourArray *) banked_colours->getEntity (state2) ; 
  l1 = c1 -> getNum () ;
  l2 = c2 -> getNum () ;

  assert ( l1 == l2 ) ;

  if ( render_colours -> getNum () < l1 )
    render_colours -> setNum ( l1 ) ;

  if ( c1 == c2 )
    colours = c1 ;
  else
  {
    colours = render_colours ;

    for ( int i = 0 ; i < l1 ; i++ )
      sgLerpVec4 ( colours->get(i),c1->get(i),c2->get(i), tween ) ;
  }

  draw_geometry () ;

  setBank ( state1 ) ;

  if ( postDrawCB != NULL )
    (*postDrawCB)(this) ;
}



void ssgTween::transform ( const sgMat4 m )
{
  int prev_bank = curr_bank ;

  for ( int i = 0 ; i < getNumBanks(); i++ )
  {
    // see if this bank has been transformed already
    // (it is really a deeper problem, but this will work in simple cases)
    int j ;
    for ( j = 0 ; j < i ; j++ )
      if ( banked_vertices -> getEntity ( i ) == banked_vertices -> getEntity ( j ) )
	break;
    if ( j == i ) 
    {
      setBank ( i ) ;
      ssgVtxTable::transform ( m ) ;
    }
  }

  setBank ( prev_bank ) ;
}



int ssgTween::load ( FILE *fd )
{
  sgVec3 temp;
  int num_banks ;

  _ssgReadVec3  ( fd, temp ); bbox.setMin( temp ) ;
  _ssgReadVec3  ( fd, temp ); bbox.setMax( temp ) ;
  _ssgReadInt   ( fd, (int *)(&gltype) ) ;
  _ssgReadInt   ( fd, & num_banks ) ;

  if ( ! ssgLeaf::load(fd) )
    return FALSE ;

  for ( int i = 0 ; i < num_banks ; i++ )
  {
    if ( ! _ssgLoadObject (fd,(ssgBase **)&vertices,  ssgTypeVertexArray()  ) ||
         ! _ssgLoadObject (fd,(ssgBase **)&normals,   ssgTypeNormalArray()  ) ||
         ! _ssgLoadObject (fd,(ssgBase **)&texcoords, ssgTypeTexCoordArray()) ||
         ! _ssgLoadObject (fd,(ssgBase **)&colours,   ssgTypeColourArray()  ) )
      return FALSE ;

    newBank ( vertices, normals, texcoords, colours ) ;
  }
 
  return TRUE ;
}


int ssgTween::save ( FILE *fd )
{
  int num_banks = banked_vertices -> getNumEntities () ;
  _ssgWriteVec3  ( fd, bbox.getMin() ) ;
  _ssgWriteVec3  ( fd, bbox.getMax() ) ;
  _ssgWriteInt   ( fd, (int) gltype ) ;
  _ssgWriteInt   ( fd, num_banks ) ;

  if ( ! ssgLeaf::save(fd) )
    return FALSE ;

  for ( int i = 0 ; i < num_banks ; i++ )
  {
    setBank ( i ) ;

    if ( ! _ssgSaveObject ( fd, vertices  ) ||
         ! _ssgSaveObject ( fd, normals   ) ||
         ! _ssgSaveObject ( fd, texcoords ) ||
         ! _ssgSaveObject ( fd, colours   ) )
      return FALSE ;
  }

  return TRUE ;
}




void ssgTween::print ( FILE *fd, char *indent, int how_much )
{
  char in [ 100 ] ;

  if ( how_much == 0 ) 
    return ;
  
  sprintf ( in, "%s  ", indent );
	
  ssgLeaf  ::print ( fd, indent, how_much ) ;
		
  vertices  -> print ( fd, in, how_much ) ;
  normals   -> print ( fd, in, how_much ) ;
  texcoords -> print ( fd, in, how_much ) ;
  colours   -> print ( fd, in, how_much ) ;
}



