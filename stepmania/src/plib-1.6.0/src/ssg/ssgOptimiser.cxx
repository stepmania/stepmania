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

static float optimise_vtol [3] =
{
  0.01f,   /* DISTANCE_SLOP = One centimeter */
  0.04f,   /* COLOUR_SLOP = Four percent */
  0.004f,  /* TEXCOORD_SLOP = One texel on a 256 map */
} ;

static float* current_vtol = 0 ;

#define DISTANCE_SLOP   current_vtol[0]
#define COLOUR_SLOP     current_vtol[1]
#define TEXCOORD_SLOP   current_vtol[2]

inline float frac ( float x )
{
  return x - (float) floor(x) ;
}

struct OptVertex
{
  sgVec3 vertex ;
  sgVec3 normal ;
  sgVec2 texcoord ;
  sgVec4 colour ;
  int    counter ;
  
  void print () { ulSetError ( UL_DEBUG, "%d:(%g,%g,%g):(%g,%g):(%g,%g,%g,%g):(%g,%g,%g)",
    counter,
    vertex[0],vertex[1],vertex[2],
    texcoord[0],texcoord[1],
    colour[0],colour[1],colour[2],colour[3],
    normal[0],normal[1],normal[2] ) ; }
  
  OptVertex ( sgVec3 v, sgVec2 t, sgVec4 c )
  {
    sgCopyVec3 ( vertex  , v ) ;
    sgCopyVec2 ( texcoord, t ) ;
    sgCopyVec4 ( colour  , c ) ;
    sgSetVec3  ( normal  , 0.0f, 0.0f, 0.0f ) ;
    counter = 1 ;
  }
  
  int equal ( sgVec3 v, sgVec2 t, sgVec4 c, int tex_frac )
  {
    if ( ! sgCompareVec3 ( vertex  , v, DISTANCE_SLOP ) ||
         ! sgCompareVec4 ( colour  , c, COLOUR_SLOP   ) )
      return FALSE ;
    
    if ( ! tex_frac )
      return sgCompareVec2 ( texcoord, t, TEXCOORD_SLOP ) ;
    
    return ( fabs ( frac ( texcoord[0] ) - frac ( t[0] ) ) <= TEXCOORD_SLOP &&
	     fabs ( frac ( texcoord[1] ) - frac ( t[1] ) ) <= TEXCOORD_SLOP ) ;
  }
  
  void bump () { counter++ ; }
  void dent () { counter-- ; }
  int getCount () { return counter ; }
} ;


#define MAX_OPT_VERTEX_LIST 10000

class OptVertexList
{
public:
  short vnum, tnum ;
  OptVertex **vlist ;
  short      *tlist ;
  ssgState  *state ;
  int        cullface ;
  
  OptVertexList ( ssgState *s, int cf )
  {
    /*
    Have to dynamically allocate these to get
    around Mac's CodeWarrior restriction on
    32Kb as max structure size.
    */
    
    vlist = new OptVertex* [ MAX_OPT_VERTEX_LIST ] ;
    tlist = new short [ MAX_OPT_VERTEX_LIST * 3 ] ;
    state = s ;
    if (state != NULL) state -> ref(); //~T.G.
    cullface = cf ;
    vnum = tnum = 0 ;
  }
  
  ~OptVertexList ()
  {
    for ( int i = 0 ; i < vnum ; i++ )
      delete vlist [ i ] ;
    
    delete [] vlist ;
    delete [] tlist ;

    if (state != NULL) ssgDeRefDelete(state); //~T.G.
  }
  
  short find ( sgVec3 v, sgVec2 t, sgVec4 c, int tex_fraction_only = FALSE ) ;
  
  short add ( sgVec3 v1, sgVec2 t1, sgVec4 c1,
    sgVec3 v2, sgVec2 t2, sgVec4 c2,
    sgVec3 v3, sgVec2 t3, sgVec4 c3 ) ;
  short add ( sgVec3 v, sgVec2 t, sgVec4 c ) ;
  short add ( short v1, short v2, short v3 ) ;
  void  add ( ssgLeaf *l ) ;
  
  void makeNormals () ;
  
  void print ()
  {
    ulSetError ( UL_DEBUG, "LIST: %d unique vertices and %d triangles",
      vnum, tnum ) ;
  }
  
  void follow ( int tri, int v1, int v2, int backwards, int *len, short *list, short *next ) ;

  int getLeastConnected ( short *t, short *v )
  {
    int least = 32767 ;
    *v = 0 ;

    /* Find the least connected vertex that *is* connected */

    int i ;

    for ( i = 0 ; i < vnum ; i++ )
    {
      int c = vlist [ i ] -> getCount () ;

      if ( c > 0 && c < least )
      {
        least = c ;
        *v = i ;
      }
    }

    if ( least == 32767 )  /* Didn't find an unused vertex - so punt. */
      return FALSE ;

    least = 32767 ;
    *t = 32767 ;

    for ( i = 0 ; i < tnum ; i++ )
      if ( tlist[i*3+0] == *v || tlist[i*3+1] == *v || tlist[i*3+2] == *v )
      {
        int c = vlist [ tlist[i*3+0] ] -> getCount () +
                vlist [ tlist[i*3+1] ] -> getCount () +
                vlist [ tlist[i*3+2] ] -> getCount () ;

        if ( c < least )
        {
          least = c ;
          *t = i ;
        }
      }

    if ( least == 32767 )  /* Didn't find an unused vertex - so punt. */
      return FALSE ;

    return TRUE ;  /* Got it! */
  }

  void sub ( short t )
  {
    vlist [ tlist[t*3+0] ] -> dent () ;
    vlist [ tlist[t*3+1] ] -> dent () ;
    vlist [ tlist[t*3+2] ] -> dent () ;
    
    tlist[t*3+0] = -1 ;
    tlist[t*3+1] = -1 ;
    tlist[t*3+2] = -1 ;
  }
} ;


short OptVertexList::add ( sgVec3 v1, sgVec2 t1, sgVec4 c1,
                          sgVec3 v2, sgVec2 t2, sgVec4 c2,
                          sgVec3 v3, sgVec2 t3, sgVec4 c3 )
{
	// If possible, this routine moves the texturecoordinates of
	// all three vertices of a Tria so that one needs less vertices.
	// This doesnÄt affect looks, but enhances the speed a bit.
	// This is not possible, if warpu or wrapv is FALSE

	int bWrapsInBothDirections = FALSE;
	short vi1, vi2, vi3;
/*
  if ( state->isAKindOf( ssgTypeSimpleState() ) )
		bWrapsInBothDirections = 
	      ( (((ssgSimpleState *)state)->getWrapU()) &&
          (((ssgSimpleState *)state)->getWrapV()) );
	*/
  if (!bWrapsInBothDirections)
	{
		/* Find which (if any) of the vertices are a match for one in the list */
  
		 vi1 = add ( v1, t1, c1 ) ;
		 vi2 = add ( v2, t2, c2 ) ;
		 vi3 = add ( v3, t3, c3 ) ;
	}
	else
	{
		/*
		Sharing vertices is tricky because of texture coordinates
		that have the same all-important fractional part - but
		differ in their integer parts.
		*/
  
		sgVec2 adjust ;
  
		/* Find which (if any) of the vertices are a match for one in the list */
  
		vi1 = find ( v1, t1, c1, TRUE ) ;
		vi2 = find ( v2, t2, c2, TRUE ) ;
		vi3 = find ( v3, t3, c3, TRUE ) ;
  
		/* Compute texture offset coordinates (if needed) to make everything match */
  
		if ( vi1 >= 0 )
			sgSubVec2 ( adjust, t1, vlist[vi1]->texcoord ) ;
		else
			if ( vi2 >= 0 )
				sgSubVec2 ( adjust, t2, vlist[vi2]->texcoord ) ;
			else
				if ( vi3 >= 0 )
					sgSubVec2 ( adjust, t3, vlist[vi3]->texcoord ) ;
				else
				{
				/*
				OK, there was no match - so just remove
				any large numbers from the texture coords
					*/
        
					adjust [ 0 ] = (float) floor ( t1[0] ) ;
					adjust [ 1 ] = (float) floor ( t1[1] ) ;
				}
      
		/*
		Now adjust the texture coordinates and add them into the list
		*/
		sgVec2 tmp ;
		sgSubVec2 ( tmp, t1, adjust ) ; vi1 = add ( v1, tmp, c1 ) ;
		sgSubVec2 ( tmp, t2, adjust ) ; vi2 = add ( v2, tmp, c2 ) ;
		sgSubVec2 ( tmp, t3, adjust ) ; vi3 = add ( v3, tmp, c3 ) ;
  }
  return add ( vi1, vi2, vi3 ) ;
}


short OptVertexList::add ( short v1, short v2, short v3 )
{
  if ( v1 == v2 || v2 == v3 || v3 == v1 ) /* Toss degenerate triangles */
  {
    vlist [ v1 ] -> dent () ; /* Un-reference their vertices */
    vlist [ v2 ] -> dent () ;
    vlist [ v3 ] -> dent () ;
    return -1 ;
  }
  
  tlist [ tnum*3+ 0 ] = v1 ;
  tlist [ tnum*3+ 1 ] = v2 ;
  tlist [ tnum*3+ 2 ] = v3 ;
  
  return tnum++ ;
}


void OptVertexList::makeNormals()
{
  short i ;
  
  for ( i = 0 ; i < vnum ; i++ )
    sgSetVec3 ( vlist [ i ] -> normal, 0.0f, 0.0f, 0.0f ) ;
  
  for ( i = 0 ; i < tnum ; i++ )
  {
    sgVec3 tmp ;
    short j ;
    
    sgMakeNormal ( tmp, vlist [ tlist [ i*3+ 0 ] ] -> vertex,
      vlist [ tlist [ i*3+ 1 ] ] -> vertex,
      vlist [ tlist [ i*3+ 2 ] ] -> vertex ) ;

    for ( j = 0; j < vnum; j++ ) {
      if (sgEqualVec3(vlist[j]->vertex, vlist[tlist[i*3+0]]->vertex))
	sgAddVec3(vlist[j]->normal, tmp);
      if (sgEqualVec3(vlist[j]->vertex, vlist[tlist[i*3+1]]->vertex))
	sgAddVec3(vlist[j]->normal, tmp);
      if (sgEqualVec3(vlist[j]->vertex, vlist[tlist[i*3+2]]->vertex))
	sgAddVec3(vlist[j]->normal, tmp);
    }
  }

  for ( i = 0 ; i < vnum ; i++ )
    if ( sgScalarProductVec2 ( vlist[i]->normal, vlist[i]->normal ) < 0.001 )
      sgSetVec3 ( vlist[i]->normal, 0.0f, 0.0f, 1.0f ) ;
    else
      sgNormaliseVec3 ( vlist [ i ] -> normal ) ;
}

short OptVertexList::find ( sgVec3 v, sgVec2 t, sgVec4 c, int tex_frac )
{
  for ( short i = 0 ; i < vnum ; i++ )
  {
    if ( vlist[i] -> equal ( v, t, c, tex_frac ) )
      return i ;
  }
  return -1 ;
}

short OptVertexList::add ( sgVec3 v, sgVec2 t, sgVec4 c )
{
  short i = find ( v, t, c, FALSE ) ;
  
  if ( i >= 0 )
  {
    vlist [ i ] -> bump () ;
    return i ;
  }
  
  vlist [ vnum ] = new OptVertex ( v, t, c ) ;
  return vnum++ ;
} 

void OptVertexList::add ( ssgLeaf *l )
{
  int j ;

  for ( j = 0 ; j < l -> getNumTriangles () ; j ++ )
  {
    short v1, v2, v3 ;
    l -> getTriangle ( j, &v1, &v2, &v3 ) ;
    add ( l->getVertex ( v1 ), l->getTexCoord ( v1 ), l->getColour ( v1 ),
      l->getVertex ( v2 ), l->getTexCoord ( v2 ), l->getColour ( v2 ),
      l->getVertex ( v3 ), l->getTexCoord ( v3 ), l->getColour ( v3 ) ) ;
  }
}


void OptVertexList::follow ( int tri, int v1, int v2, int backwards, int *len,
                                             short *new_vlist, short *new_vc )
{
  /*  WARNING  -  RECURSIVE !!  */

  v1 = tlist [ tri*3+ v1 ] ;
  v2 = tlist [ tri*3+ v2 ] ;

  /*
    This triangle's work is done - dump it.
  */

  (*len)++ ;
  sub ( tri ) ;

  /*
    If the exit edge vertices don't *both* have a reference
    then we are done.
  */

  if ( vlist [ v1 ] -> getCount () <= 0 ||
       vlist [ v2 ] -> getCount () <= 0 )
    return ;

  /*
    Search for a polygon that shares that edge in the correct
    direction - and follow it.
  */

  for ( int i = 0 ; i < tnum ; i++ )
  {
    if ( tlist [ i*3+ 0 ] < 0 )  /* Deleted triangle */
      continue ;

    if ( backwards )
    {
      /* If the previous polygon was backwards */

      if ( tlist [ i*3+ 0 ] == v1 && tlist [ i*3+ 2 ] == v2 )
      {
	new_vlist [ (*new_vc)++ ] = tlist [ i*3+ 1 ] ;
	follow ( i, 0, 1, !backwards, len, new_vlist, new_vc ) ;
        return ;
      }
      else
      if ( tlist [ i*3+ 1 ] == v1 && tlist [ i*3+ 0 ] == v2 )
      {
	new_vlist [ (*new_vc)++ ] = tlist [ i*3+ 2 ] ;
	follow ( i, 1, 2, !backwards, len, new_vlist, new_vc ) ;
        return ;
      }
      else
      if ( tlist [ i*3+ 2 ] == v1 && tlist [ i*3+ 1 ] == v2 )
      {
	new_vlist [ (*new_vc)++ ] = tlist [ i*3+ 0 ] ;
	follow ( i, 2, 0, !backwards, len, new_vlist, new_vc ) ;
        return ;
      }
    }
    else
    {
      /* If the previous polygon was forwards... */

      if ( tlist [ i*3+ 0 ] == v1 && tlist [ i*3+ 2 ] == v2 )
      {
	new_vlist [ (*new_vc)++ ] = tlist [ i*3+ 1 ] ;
	follow ( i, 1, 2, !backwards, len, new_vlist, new_vc ) ;
        return ;
      }
      else
      if ( tlist [ i*3+ 1 ] == v1 && tlist [ i*3+ 0 ] == v2 )
      {
	new_vlist [ (*new_vc)++ ] = tlist [ i*3+ 2 ] ;
	follow ( i, 2, 0, !backwards, len, new_vlist, new_vc ) ;
        return ;
      }
      else
      if ( tlist [ i*3+ 2 ] == v1 && tlist [ i*3+ 1 ] == v2 )
      {
	new_vlist [ (*new_vc)++ ] = tlist [ i*3+ 0 ] ;
	follow ( i, 0, 1, !backwards, len, new_vlist, new_vc ) ;
        return ;
      }
    }
  }
}


static ssgLeaf** build_leaf_list ( ssgEntity *ent, ssgLeaf** leaf_list=0 )
{
  enum { MAX_LEAF_COUNT = 10000 } ;
  static int leaf_count ;

  if ( leaf_list == NULL )
  {
    leaf_list = new ssgLeaf* [ MAX_LEAF_COUNT+1 ] ;
    leaf_count = 0 ;
    leaf_list [ leaf_count ] = NULL ;
  }

  if ( ent -> isAKindOf ( ssgTypeBranch () ) )
  {
    ssgBranch *b_ent = (ssgBranch *) ent ;
    for ( ssgEntity *k = b_ent -> getKid ( 0 ) ; k != NULL ;
      k = b_ent -> getNextKid () )
    {
      build_leaf_list ( k, leaf_list ) ;
    }
  }
  else if ( ent -> isAKindOf ( ssgTypeLeaf () ) )
  {
    ssgLeaf  *l = (ssgLeaf *) ent ;

    bool found = false ;
    for ( int i = 0 ; leaf_list [ i ] != NULL ; i ++ )
    {
      if ( leaf_list [ i ] == l )
      {
        found = true ;
        break ;
      }
    }

    if ( !found && leaf_count < MAX_LEAF_COUNT )
    {
      leaf_list [ leaf_count ++ ] = l ;
      leaf_list [ leaf_count ] = NULL ;
    }
  }

  return leaf_list ;
}

/*
* NAME
*   ssgArrayTool
*
* DESCRIPTION
*   Process the graph and convert all leaf entities into vertex arrays.
*
*   Each vertex is described by a single index (instead of different
*   indices into the v, vc, vt-arrays). This may introduce new redundant
*   vertex data into the data set, which may seem silly. However, when
*   rendering through OpenGL, it is in most cases the optimal solution
*   to use indexed vertex arrays, which only have ONE index for all
*   the vertex data.
*
* INPUTS
*
* ent - the entity to process
*
* vtol - an array of 3 floats used to specify tolerances
*   vtol[0] - tolerance value for vertices
*   vtol[1] - tolerance for colours
*   vtol[2] - tolerance for texture coordinates
*
* make_normals - if true then averaged vertex normals are computed
*/
void ssgArrayTool ( ssgEntity *ent, float* vtol, bool make_normals )
{
  current_vtol = vtol? vtol: optimise_vtol ;

  ssgLeaf** leaf_list = build_leaf_list ( ent ) ;
  for ( int i = 0 ; leaf_list [ i ] != NULL ; i ++ )
  {
    ssgLeaf  *l = leaf_list [ i ] ;
    ssgState *s = l -> getState() ;
    int       cf = l -> getCullFace() ;
    
    OptVertexList list ( s, cf ) ;
    list . add ( l ) ;
    
    if ( list . tnum > 0 )  /* If all the triangles are degenerate maybe */
    {
      ssgIndexArray    *new_index     = new ssgIndexArray    ( list . tnum * 3 ) ;
      ssgVertexArray   *new_coords    = new ssgVertexArray   ( list . vnum ) ;
      ssgTexCoordArray *new_texcoords = new ssgTexCoordArray ( list . vnum ) ;
      ssgColourArray   *new_colours   = new ssgColourArray   ( list . vnum ) ;
      ssgNormalArray   *new_normals   = 0 ;

      if ( make_normals )
      {
        list . makeNormals () ;
        new_normals = new ssgNormalArray ( list . vnum ) ;
      }
      
      for ( int t = 0 ; t < list . tnum ; t++ )
      {
        new_index -> add ( list . tlist [ t*3+ 0 ] ) ;
        new_index -> add ( list . tlist [ t*3+ 1 ] ) ;
        new_index -> add ( list . tlist [ t*3+ 2 ] ) ;
      }

      for ( int v = 0 ; v < list . vnum ; v++ )
      {
        new_coords   -> add ( list . vlist[ v ]->vertex   ) ;
        new_texcoords-> add ( list . vlist[ v ]->texcoord ) ;
        new_colours  -> add ( list . vlist[ v ]->colour   ) ;
        if ( make_normals )
          new_normals  -> add ( list . vlist[ v ]->normal ) ;
      }
      
      ssgVtxArray *new_varray = new ssgVtxArray ( GL_TRIANGLES,
        new_coords, new_normals, new_texcoords, new_colours, new_index ) ;
      new_varray -> setState ( list.state ) ;
      new_varray -> setCullFace ( list.cullface ) ;
      
      ssgBranch *p ;

      /*
      Add the new leaf
      */
      for ( p = l -> getParent ( 0 ) ; p != NULL ;
        p = l -> getNextParent () )
        p -> addKid ( new_varray ) ;

      /*
      Remove the old leaf
      */
      for ( p = new_varray -> getParent ( 0 ) ; p != NULL ;
        p = new_varray -> getNextParent () )
        p -> removeKid ( l ) ;
    }
  }
  delete[] leaf_list ;
  
  ent -> recalcBSphere () ;
}

void ssgStripify ( ssgEntity *ent )
{
  current_vtol = optimise_vtol ;

  /*
    Walk down until we find a leaf node, then
    back up one level, collect all the ssgVtxTables
    into one big heap and triangulate them.
  */

  if ( ent -> isAKindOf ( ssgTypeLeaf () ) )
    return ;

  ssgBranch *b_ent = (ssgBranch *) ent ;

  /*
    Count number of unique materials (and cull-facedness)
    - make a list of them.  Recursively stripify non-leaf nodes.
  */

  int stot = 0 ;
  ssgState **slist = new ssgState *[ b_ent -> getNumKids () ] ;
  int      *cflist = new int       [ b_ent -> getNumKids () ] ;

  for ( ssgEntity *k = b_ent -> getKid ( 0 ) ; k != NULL ;
				 k = b_ent -> getNextKid () )
  {
    if ( k -> isAKindOf ( ssgTypeVtxTable () ) )
    {
			GLenum thisType = ((ssgVtxTable *)k)->getPrimitiveType ();
			if ((thisType != GL_POINTS) && (thisType != GL_LINES) &&
				  (thisType != GL_LINE_STRIP) && (thisType != GL_LINE_LOOP))
			{
				int i ;
				ssgState *s = ((ssgLeaf *) k ) -> getState() ;
				int       c = ((ssgLeaf *) k ) -> getCullFace() ;

				for ( i = 0 ; i < stot ; i++ )
		if ( s == slist [ i ] && c == cflist [ i ] )
			break ;

				if ( i >= stot )
				{
		slist  [ i ] = s ;
		cflist [ i ] = c ;
		stot++ ;
				}
			}
    }
    else
    if ( k -> isAKindOf ( ssgTypeBranch () ) )
      ssgStripify ( k ) ;
  }

  /*
    Now, for each unique state, grab all the VtxTable leaf nodes
    and smoosh them into one.
  */

  for ( int i = 0 ; i < stot ; i++ )
  {
    /*
      Put it into a triangle-oriented structure and
      then do stripifying and average normal generation.

      Ick!
    */

    OptVertexList list ( slist [ i ], cflist [ i ] ) ;

    ssgEntity *k = b_ent -> getKid ( 0 ) ;
    
    while ( k != NULL )
    {
      if ( k -> isAKindOf ( ssgTypeVtxTable () ) &&
           ((ssgLeaf *) k ) -> getState() == slist [ i ] &&
           ((ssgLeaf *) k ) -> getCullFace() == cflist [ i ] )
      {
				GLenum thisType = ((ssgVtxTable *)k)->getPrimitiveType ();
				if ((thisType != GL_POINTS) && (thisType != GL_LINES) &&
						(thisType != GL_LINE_STRIP) && (thisType != GL_LINE_LOOP))
        {	list . add ( (ssgVtxTable *) k ) ;
					b_ent -> removeKid ( k ) ;
					k = b_ent -> getKid ( 0 ) ;
				}
				else
          k = b_ent -> getNextKid () ;
      }
      else
        k = b_ent -> getNextKid () ;
    }

    if ( list . tnum == 0 )  /* If all the triangles are degenerate maybe */
      continue ;

    /*
      So, now we have all the important information sucked out of
      all those nodes and safely tucked away in the OptVertexList 

      Let's take this opportunity to compute vertex normals.
    */

    list . makeNormals () ;

    /*
      Find the least connected triangle.
      Use it as the starting point.
    */

    short tleast, nleast ;

    while ( list . getLeastConnected ( & tleast, & nleast ) )
    {
      /* OK, we have our starting point - follow where it
         leads - but which way to start? We need two vertices
         with at least two references - and not the least
         referenced vertex please. */

      short *new_vlist = new short [ list.tnum * 3 ] ;
      short new_vc = 0 ;

      int striplength = 0 ;

      if ( nleast == list.tlist[tleast*3+0] )
      {
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 0 ] ;
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 1 ] ;
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 2 ] ;
        list . follow ( tleast, 1, 2, FALSE, &striplength, new_vlist, & new_vc ) ;
      }
      else
      if ( nleast == list.tlist[tleast*3+1] )
      {
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 1 ] ;
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 2 ] ;
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 0 ] ;
        list . follow ( tleast, 2, 0, FALSE, &striplength, new_vlist, & new_vc ) ;
      }
      else
      if ( nleast == list.tlist[tleast*3+2] )
      {
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 2 ] ;
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 0 ] ;
	new_vlist [ new_vc++ ] = list.tlist [ tleast*3+ 1 ] ;
        list . follow ( tleast, 0, 1, FALSE, &striplength, new_vlist, & new_vc ) ;
      }
      else
        ulSetError ( UL_WARNING, "Tleast doesn't contain nleast!" ) ;

      ssgVertexArray   *new_coords    = new ssgVertexArray   ( new_vc ) ;
      ssgNormalArray   *new_normals   = new ssgNormalArray   ( new_vc ) ;
      ssgTexCoordArray *new_texcoords = new ssgTexCoordArray ( new_vc ) ;
      ssgColourArray   *new_colours   = new ssgColourArray   ( new_vc ) ;

      for ( int m = 0 ; m < new_vc ; m++ )
      {
        new_coords   -> add ( list.vlist[new_vlist[m]]->vertex   ) ;
        new_normals  -> add ( list.vlist[new_vlist[m]]->normal   ) ;
        new_texcoords-> add ( list.vlist[new_vlist[m]]->texcoord ) ;
        new_colours  -> add ( list.vlist[new_vlist[m]]->colour   ) ;
      }

      delete [] new_vlist ;

      ssgVtxTable *new_vtable = new ssgVtxTable ( GL_TRIANGLE_STRIP,
                    new_coords, new_normals, new_texcoords, new_colours ) ;
      new_vtable -> setState ( list.state ) ;
      new_vtable -> setCullFace ( list.cullface ) ;

      b_ent -> addKid ( new_vtable ) ;
    }
  }

  delete []  slist ;
  delete [] cflist ;
}


/*
  These routines are essentially non-realtime tree optimisations.
*/

static void safe_replace_kid ( ssgBranch *parent, ssgEntity *old_kid, ssgEntity *new_kid )
{
  /*
    Replace old_kid by new_kid in a "safe" manner.
    new_kid may be null, in which case old_kid is removed.
    If parent is null then loop over all parents of old_kid.
  */

  if ( old_kid == new_kid )
    return ;

  if ( parent == NULL )
  {
    int n = old_kid -> getNumParents () ;
    while ( n-- > 0 )
      safe_replace_kid ( old_kid -> getParent ( 0 ), old_kid, new_kid ) ;
    return ;
  }

  // assert ( parent -> searchForKid ( old_kid ) >= 0 ) ;

  if ( new_kid == NULL )
  {
    if ( parent -> isAKindOf ( ssgTypeSelector () ) )
    { 
      /* cannot remove kids from selectors */
      static ssgInvisible empty ;
      parent -> replaceKid ( old_kid, &empty ) ;
    }
    else
    {
      parent -> removeKid ( old_kid ) ;
    }
  }
  else
  {
    parent -> replaceKid ( old_kid, new_kid ) ;
  }
}

static void strip ( ssgEntity *ent )
{
  /*
    Strip off all branches with no kids - and snip out all
    simple branches with just one kid.
    A node with user data is always left unchanged.
  */
  
  if ( ! ent -> isAKindOf ( ssgTypeBranch () ) )
    return ;

  ssgBranch *b_ent = (ssgBranch *) ent ;

  for ( ssgEntity *k = b_ent -> getKid ( 0 ) ; k != NULL ;
				 k = b_ent -> getNextKid () )
    strip ( k ) ;

  switch ( b_ent -> getNumKids () )
  {
  case 0:
    if ( b_ent -> getUserData() == NULL )
      safe_replace_kid ( NULL, b_ent, NULL ) ;
    break;

  case 1:
    if ( b_ent -> isA ( ssgTypeBranch () ) &&
	 b_ent -> getUserData () == NULL )
    {
      safe_replace_kid ( NULL, b_ent, b_ent -> getKid ( 0 ) ) ;
    }
    else if ( ! b_ent -> isAKindOf ( ssgTypeSelector () ) &&
	      b_ent -> getKid ( 0 ) -> isA ( ssgTypeBranch () ) &&
	      b_ent -> getKid ( 0 ) -> getUserData () == NULL )
    {
      ssgBranch *b_kid = (ssgBranch *) b_ent -> getKid ( 0 ) ;
      for ( ssgEntity *k = b_kid -> getKid ( 0 ) ; k != NULL ;
  	               k = b_kid -> getNextKid () )
        b_ent -> addKid ( k ) ;
      b_ent -> removeKid ( b_kid ) ;
      b_ent -> recalcBSphere () ;
    }
    break;

  default:
    if ( b_ent -> isDirtyBSphere () )
      b_ent -> recalcBSphere () ;
  }
}

static void flatten ( ssgBranch *parent, ssgEntity *ent, sgMat4 mat )
{
  /*
    Move all transforms down to the leaf nodes and
    then multiply them out. You need to strip() the
    tree after calling this.
  */

  sgMat4 mat2 ;

  /*
    The following nodes may (currently) not be flattened:
    - ssgCutout,
    - ssgRangeSelector, and
    - ssgTransform with user data.
  */
  if ( ent -> isAKindOf ( ssgTypeCutout () ) ||
       ent -> isAKindOf ( ssgTypeRangeSelector () ) ||
       ( ent -> isA ( ssgTypeTransform () ) &&
         ent -> getUserData () != NULL ) )
  {
    /* Insert a transform node if needed. */
    if ( mat != NULL ) {
      ssgTransform *tr = new ssgTransform ;
      tr -> setTransform ( mat ) ;
      tr -> addKid ( ent ) ;
      safe_replace_kid ( parent, ent, tr ) ;
    }

    /* Traverse as usual. */
    if ( ent -> isAKindOf ( ssgTypeBranch () ) )
    {
      ssgBranch *b_ent = (ssgBranch *) ent ;
      for ( ssgEntity *k = b_ent -> getKid ( 0 ) ; k != NULL ;
                       k = b_ent -> getNextKid () )
        flatten ( b_ent, k, NULL ) ;
    }

    return ;
  }

  /*
    Clone the node if needed (there is no need to clone it recursively,
    especially not past unflattable nodes).
  */
  if ( ent -> getRef () > 1 && mat != NULL ) 
  {
    ssgEntity *clone = (ssgEntity *) ent -> clone ( SSG_CLONE_GEOMETRY |
  				                    SSG_CLONE_USERDATA ) ;
    safe_replace_kid ( parent, ent, clone ) ;
    ent = clone ;
  }

  /*
    Apply the transformation on leaf nodes.
  */
  if ( ent -> isAKindOf ( ssgTypeLeaf () ) )
  {
    if ( mat != NULL )
      ((ssgLeaf *) ent) -> transform ( mat ) ;
    return ;
  }

  /*
    Replace transform nodes with simple branches.
   */
  if ( ent -> isAKindOf ( ssgTypeTransform () ) )
  {
    ssgTransform *t_ent = (ssgTransform *) ent ;

    t_ent -> getTransform ( mat2 ) ;
    if ( mat != NULL )
      sgPostMultMat4 ( mat2, mat ) ;

    mat = sgClassifyMat4 ( mat2 ) != 0 ? mat2 : NULL ;
    
    ssgBranch *br = new ssgBranch ;
    /*
      FIXME! It would have been very neat to do:
      br -> copy_from ( t_ent, 0 ) ;
    */
    br -> setName ( t_ent -> getName () ) ;
    for ( ssgEntity *k = t_ent -> getKid ( 0 ) ; k != NULL ;
	             k = t_ent -> getNextKid () )
      br -> addKid ( k ) ;
    t_ent -> removeAllKids () ;

    safe_replace_kid ( NULL, ent, br ) ;
    ent = br ;
  }

  /*
    Finally traverse the kids.
  */
  if ( ent -> isAKindOf ( ssgTypeBranch () ) )
  {
    ssgBranch *b_ent = (ssgBranch *) ent ;
    for ( ssgEntity *k = b_ent -> getKid ( 0 ) ; k != NULL ;
                     k = b_ent -> getNextKid () )
      flatten ( b_ent, k, mat ) ;
  }

}

void ssgFlatten ( ssgEntity *ent )
{
  if ( ! ent -> isAKindOf ( ssgTypeBranch () ) )
     return ;

  ssgBranch *b_ent = (ssgBranch *) ent ;
  sgVec4 *mat = NULL ;
  sgMat4 xform, ident ;

  /*
    If the top level node is a ssgTransform, then do not replace it;
    instead load an identity transform and multiply out the matrix.
   */

  if ( b_ent -> isA ( ssgTypeTransform () ) && 
       b_ent -> getUserData () == NULL )
  {
    sgMakeIdentMat4 ( ident ) ;
    ((ssgTransform *) b_ent) -> getTransform ( xform ) ;
    ((ssgTransform *) b_ent) -> setTransform ( ident ) ;
    mat = xform ;
  }

  /*
    Since the top level node may not be removed, loop over the kids.
    Done in two passes because *kid* may be removed.
  */
	ssgEntity *kid;

  for ( kid = b_ent -> getKid ( 0 ) ; kid != NULL ;
	           kid = b_ent -> getNextKid () )
    flatten ( b_ent, kid, mat ) ;

  for ( kid = b_ent -> getKid ( 0 ) ; kid != NULL ;
	           kid = b_ent -> getNextKid () )
    strip ( kid ) ;

  if ( b_ent -> isDirtyBSphere () )
    b_ent -> recalcBSphere () ;
}


/*
* NAME
*   ssgTransTool
*
* DESCRIPTION
*   Apply a transform (translate, rotate, scale) to all verticies of a graph.
*
* INPUTS
*   ent   -- the entity to process
*   trans -- transform
*/
void ssgTransTool ( ssgEntity *ent, const sgMat4 trans )
{
  if ( ent -> isAKindOf ( ssgTypeLeaf () ) )
  {
		((ssgLeaf *) ent) -> transform ( trans ) ;
    return ;
  }

  if ( ! ent -> isAKindOf ( ssgTypeBranch () ) )
    return ;

  ssgBranch *b_ent = (ssgBranch *) ent ;
  sgMat4 mat, ident, xform ;
  
  sgCopyMat4 ( mat, trans ) ;
  
  if ( b_ent -> isA ( ssgTypeTransform () ) && 
       b_ent -> getUserData () == NULL )
  {
    sgMakeIdentMat4 ( ident ) ;
    ((ssgTransform *) b_ent) -> getTransform ( xform ) ;
    ((ssgTransform *) b_ent) -> setTransform ( ident ) ;
    sgPreMultMat4 ( mat, xform ) ;
  }

  else if ( b_ent -> isAKindOf ( ssgTypeTransform () ) ||
	    b_ent -> isAKindOf ( ssgTypeCutout () ) ||
	    b_ent -> isAKindOf ( ssgTypeRangeSelector () ) )
  {
    ulSetError ( UL_WARNING, 
		 "ssgTransTool: Cannot handle this kind of node at top level." ) ;
    return ;
  }

	ssgEntity *kid;
  for ( kid = b_ent -> getKid ( 0 ) ; kid != NULL ;
	           kid = b_ent -> getNextKid () )
    flatten ( b_ent, kid, mat ) ;

  for ( kid = b_ent -> getKid ( 0 ) ; kid != NULL ;
	           kid = b_ent -> getNextKid () )
    strip ( kid ) ;

  b_ent -> recalcBSphere () ;
}
