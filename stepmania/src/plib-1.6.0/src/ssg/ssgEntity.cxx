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

void ssgEntity::copy_from ( ssgEntity *src, int clone_flags )
{
  ssgBase::copy_from ( src, clone_flags ) ;

  traversal_mask = src -> getTraversalMask () ;

  dirtyBSphere () ;
}


ssgEntity::ssgEntity (void)
{
  traversal_mask = 0xFFFFFFFF ;
  type = ssgTypeEntity () ;
  bsphere_is_invalid = TRUE ;

  preTravCB = NULL ;
  postTravCB = NULL ;
}


ssgEntity::~ssgEntity (void)
{
}


void ssgEntity::dirtyBSphere ()
{
  if ( bsphere_is_invalid ) return ;

  bsphere_is_invalid = TRUE ;

  int np = getNumParents () ; 

  for ( int i = 0  ; i < np ; i++ )
    getParent ( i ) -> dirtyBSphere () ;
}

void ssgEntity::visualiseBSphere ()
{
  if ( bsphere.isEmpty () )
    return ;

  glDisable ( GL_LIGHTING ) ;

  glTranslatef ( bsphere.getCenter()[0],
                 bsphere.getCenter()[1],
                 bsphere.getCenter()[2] ) ;

#ifdef USE_GLUT_SPHERES
  int spherebotch = (int)(this) % 9 ;

  switch ( spherebotch++ )
  {
    case 0 : glutWireSphere(bsphere.getRadius(),10,10); break ;
    case 1 : glutWireSphere(bsphere.getRadius(),11,10); break ;
    case 2 : glutWireSphere(bsphere.getRadius(),12,10); break ;
    case 3 : glutWireSphere(bsphere.getRadius(),10,11); break ;
    case 4 : glutWireSphere(bsphere.getRadius(),11,11); break ;
    case 5 : glutWireSphere(bsphere.getRadius(),12,11); break ;
    case 6 : glutWireSphere(bsphere.getRadius(),10,12); break ;
    case 7 : glutWireSphere(bsphere.getRadius(),11,12); break ;
    case 8 : glutWireSphere(bsphere.getRadius(),12,12); spherebotch=0 ; break ;
  }
#endif

  glTranslatef ( -bsphere.getCenter()[0],
                 -bsphere.getCenter()[1],
                 -bsphere.getCenter()[2] ) ;

  glEnable ( GL_LIGHTING ) ;
}


void ssgEntity::print ( FILE *fd, char *indent, int how_much )
{
  ssgBase::print ( fd, indent, how_much ) ;

  if ( how_much > 1 )
    fprintf ( fd, "%s  Num Parents=%d\n", indent, parents.getNumEntities () ) ;
}


/*
  Get the entity whos name equals match (case sensitive) or 0 if there isn't 
   This will be overridden by descendant classes to search trees, etc.
   This makes recovery of tagged parts of objects simple.
   I envisage it being used to find axles, gimbles etc and to seperate many objects
   from a single loaded model file.  The base version in ssgEntity returns the
   ssgEntity if the name refers to this entity or 0 if not.
*/

ssgEntity* ssgEntity::getByName ( char *match )
{
  return ( getName() != NULL && strcmp ( getName(), match ) == 0 ) ?
                                                this : (ssgEntity *) NULL ;
}


/*
  Get the entity specified by the path.  A path is a series of names
  seperated by '/'.  Each sub-unit is searched so long as the path matches.
  A single leading '/' on the path is ignored.
*/

ssgEntity* ssgEntity::getByPath ( char *path )
{
  /* ignore leading '/' */

  if ( *path == '/' )
    ++path ;

  /* return this if the name is the path else NULL */

  return ( getName() != NULL && strcmp ( getName(), path ) == 0 ) ?
                                                this : (ssgEntity *) NULL ;
}
  

ssgCallback ssgEntity::getCallback ( int cb_type )
{
  if ( isAKindOf ( ssgTypeLeaf() ) )
    return ((ssgLeaf*)this) -> getCallback ( cb_type ) ;

  /*
   *  Because of transparency sorting, having a pre/post draw callback
   *  doesn't make sense for anything but a leaf.
   */

  ulSetError ( UL_WARNING, "getCallback() ignored for non-leaf entity");
 
  return NULL ;
}


void ssgEntity::setCallback ( int cb_type, ssgCallback cb )
{
  if ( isAKindOf ( ssgTypeLeaf() ) )
  {
    ((ssgLeaf*)this) -> setCallback ( cb_type, cb ) ;
  }
  else
  {
    /*
     *  Because of transparency sorting, having a pre/post draw callback
     *  doesn't make sense for anything but a leaf.
     */

    ulSetError ( UL_WARNING, "setCallback() ignored for non-leaf entity");
  }
}


int ssgEntity::preTravTests ( int *test_needed, int which )
{
  if ( (getTraversalMask() & which) == 0 )
  {
    if ( which & SSGTRAV_HOT )
      stats_hot_no_trav ++ ;
    /*
    if ( which & SSGTRAV_LOS )
      stats_hot_no_trav ++ ;*/
    return FALSE ;
  }

  if ( preTravCB != NULL )
  {
    int result = (*preTravCB)(this,which) ;

    if ( result == 0 ) return FALSE ;
    if ( result == 2 ) *test_needed = 0 ;
  }

  return TRUE ;
}


void ssgEntity::postTravTests ( int which )
{
  if ( postTravCB != NULL )
    (*postTravCB)(this,which) ;
}


ssgCullResult ssgEntity::cull_test ( sgFrustum *f, sgMat4 m, int test_needed )
{
  if ( ! test_needed )
    return SSG_INSIDE ;

  stats_cull_test++ ;
  sgSphere tmp = *(getBSphere()) ;

  if ( tmp.isEmpty () )
    return SSG_OUTSIDE ;

  tmp . orthoXform ( m ) ;

  if ( _ssgCurrentContext->isOrtho() )
    return SSG_STRADDLE ;   /* XXX Fix Me!! XXX */
  else
    return (ssgCullResult) f -> contains ( &tmp ) ;
}


ssgCullResult ssgEntity::isect_test ( sgSphere *s, sgMat4 m, int test_needed )
{
  if ( ! test_needed )
    return SSG_INSIDE ;

  stats_isect_test++ ;
  sgSphere tmp = *(getBSphere()) ;

  if ( tmp.isEmpty () )
    return SSG_OUTSIDE ;

  tmp . orthoXform ( m ) ;

  /* Check axial distances for trivial reject */

  sgVec3 center_vec ;
  float sum_radii = s->getRadius() + tmp.getRadius() ;

  sgSubVec3 ( center_vec, s->getCenter(), tmp.getCenter() ) ;

  if ( sgAbs(center_vec[0]) > sum_radii ||
       sgAbs(center_vec[1]) > sum_radii ||
       sgAbs(center_vec[2]) > sum_radii )
    return SSG_OUTSIDE ;

  float separation_sqd = sgScalarProductVec3 ( center_vec, center_vec ) ;

  float tmp_radius_sqd = sgSquare ( tmp.getRadius() ) ;
  float sph_radius_sqd = sgSquare ( s ->getRadius() ) ;

  float sum_radii_sqd = tmp_radius_sqd + sph_radius_sqd ;

  if ( separation_sqd >= sum_radii_sqd )
    return SSG_OUTSIDE ;

  if ( separation_sqd + tmp_radius_sqd <= sph_radius_sqd )
    return SSG_INSIDE ;

  return SSG_STRADDLE ;
}


/*
  Places the addresses of the entities whose names or paths
   are listed in bind into the variables whose addresses are
   listed in bind.  The bind array must terminate with a
   NULL name.  Returns TRUE if all names were found and
   bound to variables.
*/

int ssgEntity::bindEntities ( ssgEntityBinding *bind )
{
  int success = TRUE ;

  while ( bind -> nameOrPath != NULL )
  {
    ssgEntity *e = NULL ;

    if ( strchr ( bind -> nameOrPath, '/' ) )
      e = getByPath ( bind -> nameOrPath ) ;
    else
      e = getByName ( bind -> nameOrPath ) ;

    if ( e != NULL )
      *( bind -> entity ) = e ;
    else
      success = FALSE ;

    ++bind ;
  }

  return success ;
}


ssgCullResult ssgEntity::hot_test ( sgVec3 s, sgMat4 m, int test_needed )
{
  stats_hot_test++ ;

  if ( !test_needed )
{
stats_hot_triv_accept++ ;
    return SSG_INSIDE ;
}

  sgSphere tmp = *(getBSphere()) ;

  if ( tmp.isEmpty () )
    return SSG_OUTSIDE ;

  tmp . orthoXform ( m ) ;

  float d = sgSquare ( s[0] - tmp.getCenter()[0] ) +
            sgSquare ( s[1] - tmp.getCenter()[1] ) ;

  if ( d > sgSquare ( tmp.getRadius() ) )
{
stats_hot_radius_reject++ ;
    return SSG_OUTSIDE ;
}

stats_hot_straddle++ ;
  return SSG_STRADDLE ;
}



ssgCullResult ssgEntity::los_test ( sgVec3 s, sgMat4 m, int test_needed )
{
  stats_los_test++ ;

  if ( !test_needed )
{
stats_los_triv_accept++ ;
    return SSG_INSIDE ;
}

  sgSphere tmp = *(getBSphere()) ;

  if ( tmp.isEmpty () )
    return SSG_OUTSIDE ;

  tmp . orthoXform ( m ) ;

  float d = sgSquare ( s[0] - tmp.getCenter()[0] ) +
            sgSquare ( s[1] - tmp.getCenter()[1] ) ;

  if ( d > sgSquare ( tmp.getRadius() ) )
{
stats_los_radius_reject++ ;
    return SSG_OUTSIDE ;
}

stats_los_straddle++ ;
  return SSG_STRADDLE ;
}


int ssgEntity::load ( FILE *fd )
{
  bsphere_is_invalid = TRUE ;
  _ssgReadInt    ( fd, &traversal_mask ) ;
  return ssgBase::load(fd) ;
}



int ssgEntity::save ( FILE *fd )
{
  _ssgWriteInt    ( fd, traversal_mask ) ;
  return ssgBase::save(fd) ;
}


