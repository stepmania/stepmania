
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

// ssgLoaderWriterStuff.cxx
// Here you will find classes and functions you can use to 
// implement loaders and writers for ssg
// Also, there is the parser for loading ASCII files, which 
// has its own file ssgParser.cxx and there are functions like
// the stripifier that are usefull not only for loaders/writers.
//
// 1. Version written by Wolfram Kuss (Wolfram.Kuss@t-online.de) 
// in Nov of 2000
// Distributed with Steve Bakers plib under the LGPL licence

#include  "ssgLocal.h"
#include "ssgLoaderWriterStuff.h"

// need a prototype for alloca:
#if defined(__sgi) || defined(__MWERKS__)
#include <alloca.h>
#endif
#if defined(_MSC_VER)
#include <malloc.h>
#endif

#if defined(__MINGW32__)
#include <libiberty.h>
#endif

#undef ABS
#undef MIN
#undef MAX
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define MIN(a,b) ((a) <= (b) ? (a) : (b))
#define MAX(a,b) ((a) >= (b) ? (a) : (b))
#define MIN3(a,b,c) ((a) <= (b) ? MIN(a,c) : MIN(b,c))
#define MAX3(a,b,c) ((a) >= (b) ? MAX(a,c) : MAX(b,c))

// texture coord epsilon
#define TC_EPSILON 0.01


sgVec4 currentDiffuse;

// ***********************************************************************
// ********************  small utility functions  ************************
// ***********************************************************************

void ssgAccumVerticesAndFaces( ssgEntity* node, sgMat4 transform, ssgVertexArray* vertices,
			       ssgIndexArray*  indices, SGfloat epsilon, ssgSimpleStateArray* ssa,
			       ssgIndexArray*  materialIndices, ssgTexCoordArray *texCoordArray)
// Accumulates all vertices and Faces (indexes of vertices making up faces)
// from node and any node below.
// Calls itself recursively.
// If indices is NULL, no face info is accumulated
// if epsilon is < 0.0, it is ignored. Else vertices are only accumulated if 
// there is no vertex inside epsilon yet
{

   assert( vertices != NULL );
   assert( (epsilon < 0.0) || (indices == NULL) ); // sorry: using epsilon AND using indices not implemented
   if ( ssa != NULL ) { assert( indices != NULL ); }
   assert ( ((ssa==NULL) && (materialIndices==NULL)) || ((ssa!=NULL) && (materialIndices!=NULL)));
   if ( node->isAKindOf( ssgTypeTransform() ) ) {
      sgMat4 local_transform;
      ssgTransform *t_node = (ssgTransform*)node;
    
      t_node->getTransform(local_transform);
      sgPostMultMat4( local_transform, transform );

      for (ssgEntity* kid = t_node->getKid(0); kid != NULL; 
	   kid = t_node->getNextKid()) {
	 ssgAccumVerticesAndFaces( kid, local_transform, vertices, indices, epsilon, ssa, materialIndices, texCoordArray);
      }
   } 
   else if ( node->isAKindOf( ssgTypeBranch() ) ) {
      ssgBranch *b_node = (ssgBranch*)node;
      for (ssgEntity* kid = b_node->getKid(0); kid != NULL;
	   kid = b_node->getNextKid()) {
	 ssgAccumVerticesAndFaces( kid, transform, vertices, indices, epsilon, ssa, materialIndices, texCoordArray);
      }    
   } 
   else if ( node->isAKindOf( ssgTypeLeaf() ) ) {
      ssgLeaf* l_node = (ssgLeaf*)node;
      int i, vert_low = vertices->getNum();
      
      int useTexture = FALSE;
      if ( texCoordArray )
	if ( l_node->getState() )
	  if (l_node->getState()->isAKindOf(ssgTypeSimpleState())) 
	    { 
	       ssgSimpleState * ss = (ssgSimpleState *) l_node->getState();
	       if ( ss->isEnabled ( GL_TEXTURE_2D ) )
// wk kludge!!!						if ( l_node -> getNumTexCoords () == l_node -> getNumVertices() )
		 useTexture = TRUE;
	    }
    
      for (i = 0; i < l_node->getNumVertices(); i++) {
	 sgVec3 new_vertex;
	 sgXformVec3(new_vertex, l_node->getVertex(i), transform);
	 
	 if ( epsilon < 0.0 )
	   {
	      vertices->add(new_vertex);
	      if ( useTexture )
		texCoordArray ->add ( l_node->getTexCoord(i) );
	      else if ( texCoordArray )
		texCoordArray ->add ( _ssgTexCoord00 );
	   }
	 else
	   { 
	      int j, bFound = FALSE, nv1 = vertices -> getNum ();
	      for ( j = 0; j < nv1; j++)
		{
		   float *oldvertex = vertices -> get(j);
		   if (( new_vertex[0] - oldvertex[0] > -epsilon) &&
		       ( new_vertex[0] - oldvertex[0] < epsilon) &&
		       ( new_vertex[1] - oldvertex[1] > -epsilon) &&
		       ( new_vertex[1] - oldvertex[1] < epsilon) &&
		       ( new_vertex[2] - oldvertex[2] > -epsilon) &&
		       ( new_vertex[2] - oldvertex[2] < epsilon))
		     { 
			float *f;
			if ( useTexture )
			  {
			     assert( texCoordArray ); // if texCoordArray would be NULL, useTexture would not be set.
			     f = texCoordArray -> get(j);
			  }
			if ( !useTexture || ((l_node->getTexCoord(i)[0] == f[0]) &&
					     (l_node->getTexCoord(i)[1] == f[1])))
			  {
			     bFound = TRUE;
			     break;
			  }
		     }
		}
	      if ( ! bFound )
		{
		   vertices -> add ( new_vertex );
		   if ( useTexture )
		     texCoordArray ->add ( l_node->getTexCoord(i) );
		   else if ( texCoordArray )
		     texCoordArray ->add ( _ssgTexCoord00 );
		}
	   }
      }

      if ( indices != NULL )
	{
	   int index=-1;
	   if ( ssa != NULL )
	     { 
		ssgState *s = l_node->getState();
		if ( s != NULL )
		  { index = ssa->findIndex (reinterpret_cast <class ssgSimpleState *> (s) );
		     if ( index < 0 )
		       { ssa -> add(reinterpret_cast <class ssgSimpleState *> (s) );
			  index = ssa->getNum()-1;
		       }
		  }
	     }
	   for (i = 0; i < l_node->getNumTriangles(); i++) {
	      short v1, v2, v3;
	      l_node->getTriangle(i, &v1, &v2, &v3);
	      indices->add( vert_low + v1 );
	      indices->add( vert_low + v2 );
	      indices->add( vert_low + v3 );
	      if ( materialIndices != NULL )
		materialIndices->add(index); // index is -1 for leafs without state
	   }
	}
   }
   if ( texCoordArray )
     {
	assert( vertices->getNum() == texCoordArray->getNum() );
     }
} ;


/*
  ssgTriangulate - triangulate a simple polygon.
*/

static int triangulateConcave(sgVec3 *coords, int *w, int n, int x, int y, int *tris) // was: triangulate_concave
{
   struct Vtx {
      int index;
      float x, y;
      Vtx *next;
   };

   Vtx *p0, *p1, *p2, *m0, *m1, *m2, *t;
   int i, chk, num_tris;
   float a0, a1, a2, b0, b1, b2, c0, c1, c2;

   /* construct a circular linked list of the vertices */
   p0 = (Vtx *) alloca(sizeof(Vtx));
   p0->index = w ? w[0] : 0;
   p0->x = coords[p0->index][x];
   p0->y = coords[p0->index][y];
   p1 = p0;
   p2 = 0;
   for (i = 1; i < n; i++) {
      p2 = (Vtx *) alloca(sizeof(Vtx));
      p2->index = w ? w[i] : i;
      p2->x = coords[p2->index][x];
      p2->y = coords[p2->index][y];
      p1->next = p2;
      p1 = p2;
   }
   p2->next = p0;

   m0 = p0;
   m1 = p1 = p0->next;
   m2 = p2 = p1->next;
   chk = 0;
   num_tris = 0;

   while (p0 != p2->next) {
      if (chk && m0 == p0 && m1 == p1 && m2 == p2) {
	 /* no suitable vertex found.. */
         ulSetError(UL_WARNING, "ssgTriangulate: Self-intersecting polygon.");	 
         return 0;
      }
      chk = 1;

      a0 = p1->y - p2->y;
      a1 = p2->y - p0->y;
      a2 = p0->y - p1->y;
      b0 = p2->x - p1->x;
      b1 = p0->x - p2->x;
      b2 = p1->x - p0->x;
      
      if (b0 * a2 - b2 * a0 < 0) {
	 /* current angle is concave */
         p0 = p1;
         p1 = p2;
         p2 = p2->next;
      }
      else {
	 /* current angle is convex */
         float xmin = MIN3(p0->x, p1->x, p2->x);
         float xmax = MAX3(p0->x, p1->x, p2->x);
         float ymin = MIN3(p0->y, p1->y, p2->y);
         float ymax = MAX3(p0->y, p1->y, p2->y);
         
	 c0 = p1->x * p2->y - p2->x * p1->y;
	 c1 = p2->x * p0->y - p0->x * p2->y;
	 c2 = p0->x * p1->y - p1->x * p0->y;

         for (t = p2->next; t != p0; t = t->next) {
	    /* see if the triangle contains this vertex */
            if (xmin <= t->x && t->x <= xmax && 
                ymin <= t->y && t->y <= ymax &&
		a0 * t->x + b0 * t->y + c0 > 0 &&
		a1 * t->x + b1 * t->y + c1 > 0 &&		   
		a2 * t->x + b2 * t->y + c2 > 0)
	       break;
	 }

         if (t != p0) {
            p0 = p1;
            p1 = p2;
            p2 = p2->next;
         }
         else {
	    /* extract this triangle */
	    tris[3 * num_tris + 0] = p0->index;
	    tris[3 * num_tris + 1] = p1->index;
	    tris[3 * num_tris + 2] = p2->index;
	    num_tris++;
            
	    p0->next = p1 = p2;
	    p2 = p2->next;
            
            m0 = p0;
            m1 = p1;
            m2 = p2;
            chk = 0;
         }
      }
   }

   tris[3 * num_tris + 0] = p0->index;
   tris[3 * num_tris + 1] = p1->index;
   tris[3 * num_tris + 2] = p2->index;
   num_tris++;

   return num_tris;
}

int _ssgTriangulate( sgVec3 *coords, int *w, int n, int *tris )
{
   float *a, *b;
   int i, x, y;

   /* trivial case */
   if (n <= 3) {
      if (n == 3) {
	 tris[0] = w ? w[0] : 0;
	 tris[1] = w ? w[1] : 1;
	 tris[2] = w ? w[2] : 2;
	 return 1;
      }
      ulSetError(UL_WARNING, "ssgTriangulate: Invalid number of vertices (%d).", n);
      return 0;
   }

   /* compute areas */
   {
      float s[3], t[3];
      int swap;

      s[0] = s[1] = s[2] = 0;
      b = coords[w ? w[n - 1] : n - 1];

      for (i = 0; i < n; i++) {
	 a = b;
	 b = coords[w ? w[i] : i];
	 s[0] += a[1] * b[2] - a[2] * b[1];
	 s[1] += a[2] * b[0] - a[0] * b[2];
	 s[2] += a[0] * b[1] - a[1] * b[0];
      }
   
      /* select largest area */
      t[0] = ABS(s[0]);
      t[1] = ABS(s[1]);
      t[2] = ABS(s[2]);
      i = t[0] > t[1] ? t[0] > t[2] ? 0 : 2 : t[1] > t[2] ? 1 : 2;
      swap = (s[i] < 0); /* swap coordinates if clockwise */
      x = (i + 1 + swap) % 3;
      y = (i + 2 - swap) % 3;
   }

   /* concave check */
   {
      float x0, y0, x1, y1;

      a = coords[w ? w[n - 2] : n - 2];
      b = coords[w ? w[n - 1] : n - 1];
      x1 = b[x] - a[x];
      y1 = b[y] - a[y];

      for (i = 0; i < n; i++) {
	 a = b;
	 b = coords[w ? w[i] : i];
	 x0 = x1;
	 y0 = y1;
	 x1 = b[x] - a[x];
	 y1 = b[y] - a[y];
	 if (x0 * y1 - x1 * y0 < 0)
	    return triangulateConcave(coords, w, n, x, y, tris);
      }
   }

   /* convert to triangles */
   {
      int v0 = 0, v1 = 1, v = n - 1; 
      int even = 1;
      for (i = 0; i < n - 2; i++) {
	 if (even) {
	    tris[3 * i + 0] = w ? w[v0] : v0;
	    tris[3 * i + 1] = w ? w[v1] : v1;
	    tris[3 * i + 2] = w ? w[v] : v;
	    v0 = v1;
	    v1 = v;
	    v = v0 + 1;
	 }
	 else {
	    tris[3 * i + 0] = w ? w[v1] : v1;
	    tris[3 * i + 1] = w ? w[v0] : v0;
	    tris[3 * i + 2] = w ? w[v] : v;
	    v0 = v1;
	    v1 = v;
	    v = v0 - 1;
	 }
	 even = !even;
      }
   }
   return n - 2;
}



// ***********************************************************************
// ******************** class ssgLoaderWriterMesh ************************
// ***********************************************************************
 
void ssgLoaderWriterMesh::reInit(void) // was: ReInit
{
   theVertices = NULL ; 
   materialIndices = NULL ; 
   theFaces = NULL ;
   perFaceAndVertexTextureCoordinate2Lists = NULL ;
   theMaterials	= NULL ;
   perVertexTextureCoordinates2 = NULL ;
	 name = NULL ;
   textureCoordinatesArePerVertex = TRUE ;
}
	
ssgLoaderWriterMesh::ssgLoaderWriterMesh()
{
   reInit();
}

ssgLoaderWriterMesh::~ssgLoaderWriterMesh()
{}

void ssgLoaderWriterMesh::deletePerFaceAndVertexTextureCoordinates2() // was: deleteTCPFAV
{}

// creation stuff:

void ssgLoaderWriterMesh::createVertices( int numReservedVertices ) // was: ThereAreNVertices
{
   assert( theVertices == NULL );
   theVertices = new ssgVertexArray ( numReservedVertices );
}

void ssgLoaderWriterMesh::addVertex( sgVec3 v ) 
{
	assert( theVertices!=NULL );
	theVertices->add ( v );
}

void ssgLoaderWriterMesh::setVertices( class ssgVertexArray *vertexArray )
{
   assert( theVertices == NULL );
   theVertices = vertexArray;
}

void ssgLoaderWriterMesh::setPerVertexTextureCoordinates2( class ssgTexCoordArray *texCoordArray )
{
   assert( perVertexTextureCoordinates2 == NULL );
   perVertexTextureCoordinates2 = texCoordArray;
}

void ssgLoaderWriterMesh::createFaces( int numReservedFaces ) // was: ThereAreNFaces
{
	assert( theFaces == NULL );
	theFaces = new ssgListOfLists ( numReservedFaces );
}

void ssgLoaderWriterMesh::addFace( ssgIndexArray **indexArray ) 
{
	assert( theFaces!=NULL );
	theFaces->add ( (ssgSimpleList **)indexArray );
}

void ssgLoaderWriterMesh::addFaceFromIntegerArray( int numVertices, int *vertices )
{
   int j;
   class ssgIndexArray *oneFace = new ssgIndexArray( numVertices ); 
   oneFace->ref();
   for( j=0; j<numVertices; j++ )
     oneFace->add( vertices[j] );

   addFace( (ssgIndexArray **) &oneFace );
}

void ssgLoaderWriterMesh::createPerFaceAndVertexTextureCoordinates2( int numReservedTextureCoordinateLists ) // was: ThereAreNTCPFAV
{
   assert( perFaceAndVertexTextureCoordinate2Lists == NULL );
   perFaceAndVertexTextureCoordinate2Lists = new ssgListOfLists( numReservedTextureCoordinateLists );
}

void ssgLoaderWriterMesh::addPerFaceAndVertexTextureCoordinate2( ssgTexCoordArray **textureCoordinates2 ) // was: addTCPFAV
{
   assert( perFaceAndVertexTextureCoordinate2Lists != NULL );
   perFaceAndVertexTextureCoordinate2Lists->add( (ssgSimpleList **)textureCoordinates2 );
}

void ssgLoaderWriterMesh::createPerVertexTextureCoordinates2( int numReservedTextureCoordinates )
{
   assert( perVertexTextureCoordinates2 == NULL );
   perVertexTextureCoordinates2 = new ssgTexCoordArray ( numReservedTextureCoordinates );
}

void ssgLoaderWriterMesh::addPerVertexTextureCoordinate2( sgVec2 textureCoordinate ) 
{
   assert( perVertexTextureCoordinates2 != NULL );
   perVertexTextureCoordinates2->add ( textureCoordinate );
}

void ssgLoaderWriterMesh::createMaterialIndices( int numReservedMaterialIndices ) // ThereAreNMaterialIndexes
{
   assert( materialIndices == NULL );
   materialIndices = new ssgIndexArray ( numReservedMaterialIndices );
}

void ssgLoaderWriterMesh::addMaterialIndex( short materialIndex ) 
{
   assert( materialIndices != NULL );
   materialIndices->add ( materialIndex );
}

void ssgLoaderWriterMesh::createMaterials( int numReservedMaterials ) // ThereAreNMaterials( int n ) 
{
   assert( theMaterials == NULL );
   theMaterials = new ssgSimpleStateList( numReservedMaterials );
}

void ssgLoaderWriterMesh::addMaterial ( class ssgSimpleState **simpleState ) 
{
   assert( theMaterials != NULL );
   theMaterials->add( simpleState );
}

static void recalcNormals( ssgIndexArray* indexList, ssgVertexArray* vertexList, ssgNormalArray *normalList ) 
// wl: modified to use more code from sg
{
   sgVec3 v1, v2, n;
   
   for (int i = 0; i < indexList->getNum() / 3; i++) {
      short indices[3] = { *indexList->get( i*3 ), *indexList->get( i*3 + 1), *indexList->get( i*3 + 2) };
      
      sgSubVec3(v1, vertexList->get(indices[1]), vertexList->get(indices[0]));
      sgSubVec3(v2, vertexList->get(indices[2]), vertexList->get(indices[0]));
      
      sgVectorProductVec3( n, v1, v2 );
      SGfloat normalLength = sgLengthVec3( n );
      if( normalLength > 0.00001 )
	   sgNormaliseVec3( n );

      sgCopyVec3( normalList->get( indices[0] ), n );
      sgCopyVec3( normalList->get( indices[1] ), n );
      sgCopyVec3( normalList->get( indices[2] ), n );
   }
 }

// addOneNodeToSSGFromPerFaceAndVertexTextureCoordinates: this function replicates each vertex (based on face usage) and
// assigns the appropriate texture coordinates to them (based on the per-face texture indices)
void ssgLoaderWriterMesh::addOneNodeToSSGFromPerFaceAndVertexTextureCoordinates2( class ssgVertexArray *theVertices,
										 class ssgListOfLists *thePerFaceAndVertexTextureCoordinates2,
										 class ssgListOfLists *theFaces,
										 class ssgSimpleState *currentState,// Pfusch, kludge. NIV135
										 class ssgLoaderOptions* current_options,
										 class ssgBranch *curr_branch_) // was: AddOneNode2SSGFromCPFAV

{
   int i, j;
   
   assert( theVertices!=NULL );
   assert( theFaces!=NULL );
   
   // note: I am changing theVertices here, but that is allowed.
   class ssgTexCoordArray *perVertexTextureCoordinates2 = new ssgTexCoordArray( theVertices->getNum() );
   sgVec2 unUsed;
   unUsed[0]=-99999; // FixMe: It would be nicer to have an extra array of booleans
   unUsed[1]=-99999;
   for( i=0; i<theVertices->getNum(); i++)
     perVertexTextureCoordinates2->add( unUsed ); 
   for( i=0; i<theFaces->getNum(); i++)
     {
	class ssgIndexArray *oneFace = *((class ssgIndexArray **) theFaces->get( i ));
	class ssgTexCoordArray *textureCoordsForOneFace = *( (ssgTexCoordArray **) thePerFaceAndVertexTextureCoordinates2->get( i ) );
	if ( textureCoordsForOneFace != NULL ) // It is allowed that some or even all faces are untextured.
	  {
	     for( j=0; j<oneFace->getNum(); j++ )
	       { 
		  short *ps = oneFace->get(j);
		  float *newTextureCoordinate2 = textureCoordsForOneFace->get( j );
		  float *oldTextureCoordinate2 = perVertexTextureCoordinates2->get( *ps );
					
		  assert( oldTextureCoordinate2 != NULL );
		  if ((oldTextureCoordinate2[0]==-99999) && (oldTextureCoordinate2[1]==-99999)) // tc unused until now. Use it
		    { 
		       sgVec2 pv; // FixMe: mem leak?
		       pv[0]=newTextureCoordinate2[0];
		       pv[1]=newTextureCoordinate2[1];
		       perVertexTextureCoordinates2->set( pv, *ps );
		    }
		  else
		    { // can we simply use the "old" value?
		       if ( TC_EPSILON < ABS ( newTextureCoordinate2[0]-oldTextureCoordinate2[0] ) +
			    ABS ( newTextureCoordinate2[1]-oldTextureCoordinate2[1] ))
			 { // NO, we can't. Duplicate vertex
			    // not allowed: theVertices->add(theVertices->get(*ps)); 
			    // create duplicate 3D. FixMe: clone needed?
			    			   
			    float * f = theVertices->get(*ps);
			    sgVec3 v;
			    v[0] = f[0]; v[1] = f[1]; v[2] = f[2]; 
			    theVertices->add( v );
			    sgVec2 pv;
			    pv[0]=newTextureCoordinate2[0];
			    pv[1]=newTextureCoordinate2[1];
			    perVertexTextureCoordinates2->add( pv ); // create duplicate 2D
			    *ps=theVertices->getNum()-1;  // use duplicate
			    assert ( *oneFace->get(j) == theVertices->getNum()-1);
			 }
		    }
	       }
	  }
     }
   addOneNodeToSSGFromPerVertexTextureCoordinates2(theVertices, perVertexTextureCoordinates2, theFaces, currentState,
						  current_options, curr_branch_);
}

void ssgLoaderWriterMesh::addOneNodeToSSGFromPerVertexTextureCoordinates2( class ssgVertexArray *theVertices,
									   class ssgTexCoordArray *theTextureCoordinates2,
									   class ssgListOfLists *theFaces,
									   class ssgSimpleState *currentState,// kludge NIV135
									   class ssgLoaderOptions* current_options,
									   class ssgBranch *curr_branch_) // was: AddOneNode2SSGFromCPV
{
   int i, j;
   //start Normals, FixMe, kludge NIV135

   ssgNormalArray *normalList = new ssgNormalArray( theVertices->getNum() );
   sgVec3 kludge;
   for( i=0; i<theVertices->getNum(); i++ )
     normalList->add(kludge); //currentMesh.vl->get(i));

   class ssgIndexArray* indexList = new ssgIndexArray ( theFaces->getNum() * 3 ) ; // there are MINIMAL n * 3 indexes

   for( i=0; i<theFaces->getNum(); i++ )
     {
	class ssgIndexArray *oneFace = *((class ssgIndexArray **) theFaces->get( i )); 
	if ( oneFace->getNum() >= 3 )
	  {	
	     for(j=0;j<oneFace->getNum();j++)
	       { 
		  if (j<3)
		    indexList->add(*oneFace->get(j));
		  else // add a complete triangle
		    {
		       indexList->add(*oneFace->get(0));
		       indexList->add(*oneFace->get(j-1));
		       indexList->add(*oneFace->get(j));
		    }
	       }
	  }
     }
   recalcNormals( indexList, theVertices, normalList ); // Fixme, NIV14: only do this if there are no normals in the file
	
   ssgColourArray* colours = NULL ;
  
   if ( currentState -> isEnabled ( GL_LIGHTING ) )
     {
	if ( colours == NULL )
	  {
	     colours = new ssgColourArray ( 1 ) ;
	     colours -> add ( currentDiffuse ) ;
	  }
     }
   
   ssgVtxArray* leaf = new ssgVtxArray ( GL_TRIANGLES, theVertices, normalList, theTextureCoordinates2, colours, indexList ) ;
   leaf -> setCullFace ( TRUE ) ;
   leaf -> setState ( currentState ) ;
   
   ssgEntity *model = current_options -> createLeaf ( leaf, NULL)  ;
   assert( model != NULL );
	 model->setName(name);
   curr_branch_->addKid(model);
}

void ssgLoaderWriterMesh::setName( const char *meshName )
{
	delete [] name;
	if ( !meshName )
		name = NULL;
	else
	{
		name = new char [ strlen(meshName) + 1 ];
		strcpy(name, meshName);
	}
}


void ssgLoaderWriterMesh::addToSSG(
				   class ssgSimpleState *currentState,// FixMe, kludge. NIV135
				   class ssgLoaderOptions* current_options,
				   class ssgBranch *curr_branch_ )
{ 
   int i, j, k;
   unsigned short oldVertexIndex, newVertexIndex;
   class ssgIndexArray *thisFace;
  
#ifdef WRITE_MESH_TO_STDOUT	
	if ( theMaterials == NULL )
		ulSetError(UL_DEBUG, "( theMaterials == NULL )");
	else
	{	
		ulSetError(UL_DEBUG, "%d Materials:", theMaterials->getNum());
		for(i=0;i<theMaterials->getNum();i++)
		{ ulSetError(UL_DEBUG, "%ld", (long)theMaterials->get(i));
		}
	}
	if ( materialIndices == NULL )
		ulSetError(UL_DEBUG, "( materialIndices == NULL )");
	else
	{
		ulSetError(UL_DEBUG, "%d Material Indexes:", materialIndices->getNum());
		for(i=0;i<materialIndices->getNum();i++)
		{ short s=*(materialIndices->get(i));
			ulSetError(UL_DEBUG, "%ld", (long)s);
		}
	}
#endif
   if ( theMaterials == NULL )
	{ 
	   if ( perFaceAndVertexTextureCoordinate2Lists == NULL )
	     addOneNodeToSSGFromPerVertexTextureCoordinates2( theVertices, perVertexTextureCoordinates2 /* may be NULL */, theFaces, currentState, current_options, curr_branch_);
	   else
	     addOneNodeToSSGFromPerFaceAndVertexTextureCoordinates2(theVertices, perFaceAndVertexTextureCoordinate2Lists, theFaces, currentState, 
				     current_options, curr_branch_);
	}
   else
     {	
	assert( theVertices != NULL );
	assert( theFaces != NULL );
	// FixMe: What about faces without state? They should have material -1
	for( i=0; i < theMaterials->getNum(); i++ )
	  {	
	     // I often allocate too much; This is wastefull on memory, but fast since it never "resizes":
	     class ssgVertexArray *newVertices = new ssgVertexArray ( theVertices->getNum() );
	     class ssgListOfLists *newFaces = new ssgListOfLists ( theFaces->getNum() );
	     class ssgIndexArray *oldVertexIndexToNewVertexIndex = new ssgIndexArray ( theVertices->getNum() ); 
	     class ssgListOfLists *newPerFaceAndVertexTextureCoordinate2Lists = NULL;
	     class ssgTexCoordArray *newPerVertexTextureCoordinates2 = NULL;
	     
	     if(  perFaceAndVertexTextureCoordinate2Lists != NULL )
	       newPerFaceAndVertexTextureCoordinate2Lists = new ssgListOfLists();
	     if ( perVertexTextureCoordinates2 != NULL )
	       newPerVertexTextureCoordinates2 = new ssgTexCoordArray();
	     
	     for (j=0; j<theVertices->getNum(); j++)
	       oldVertexIndexToNewVertexIndex->add ( short(0xFFFF) ); // 0xFFFF stands for "unused in new Mesh"
	     
	     // Go through all the old Faces, look for the correct material and copy those
	     // faces and indexes into the new
	     // FixMe, 2do, NIV135: if the Materials just differ through the colour, one would not need
	     // several meshes, but could use the colour array. However, this is not possible,
	     // if they differ by for example the texture
	     assert( materialIndices != NULL );
	     for ( j=0; j<theFaces->getNum(); j++ )
	       if ( i == *(materialIndices->get(
						// for *.x-files, there may be less materialIndices than faces. I then simply repeat 
						// the last index all the time:
						j<materialIndices->getNum() ? j : materialIndices->getNum()-1 )) 
				 							)
		 { 
		    // take this face
		    thisFace = *((class ssgIndexArray **) theFaces->get( j )); 
		    newFaces->add( (class ssgSimpleList **)&thisFace); 
		    //thisFace = *((class ssgIndexArray **) newFaces->get( newFaces->getNum()-1 )); 
		    
		    if( perFaceAndVertexTextureCoordinate2Lists != NULL )
		      newPerFaceAndVertexTextureCoordinate2Lists ->add( perFaceAndVertexTextureCoordinate2Lists -> get( j ) );
					 
		    for( k=0; k < thisFace->getNum(); k++ )
		      { 
			 oldVertexIndex = * thisFace->get(k);
			 newVertexIndex = *oldVertexIndexToNewVertexIndex->get( oldVertexIndex );

			 if ( 0xFFFF == newVertexIndex )
			   { 
			      newVertexIndex = newVertices->getNum();
			      newVertices->add( theVertices->get( oldVertexIndex ) );
			      oldVertexIndexToNewVertexIndex->set( newVertexIndex, oldVertexIndex );
			   }
			 if ( perVertexTextureCoordinates2 != NULL )
			   newPerVertexTextureCoordinates2 -> add( perVertexTextureCoordinates2->get( oldVertexIndex ) );
			 // From here on the indexes in thisFace are only valid in relation to
			 // newVertices and newtextureCoordinatePerVertex. Since this face will not be used for any 
			 // further material, this doesn't lead to problems.
			 thisFace->set( newVertexIndex, k );
		      }	
		 }
#ifdef WRITE_MESH_TO_STDOUT	
	     ulSetError(UL_DEBUG, "NumVert: %d", newVertices->getNum());
	     for(j=0;j<newVertices->getNum();j++)
	       { float *f=newVertices->get(j);
		  ulSetError(UL_DEBUG, "%f, %f, %f",f[0], f[1], f[2]);
	       }
	     for(j=0;j<newFaces->getNum();j++)
			{
			   thisFace = *((class ssgIndexArray **) newFaces->get( j )); 	
			   fprintf(stderr, "%d EP:", thisFace->getNum());
			   for(k=0;k<thisFace->getNum();k++)
			     {
				oldVertexIndex = * thisFace->get(k);
				fprintf(stderr, "%d, ", oldVertexIndex);
			     }
			   putc('\n', stderr);
			}
#endif	
	     if ( newFaces->getNum() > 0 )
	       {
		  currentState = *theMaterials->get(i);
		  if ( perFaceAndVertexTextureCoordinate2Lists == NULL )
		    // FixMe: textureCoordinatePerVertex-indices are not compatible to newVertices-indices?!?
		    addOneNodeToSSGFromPerVertexTextureCoordinates2( newVertices, newPerVertexTextureCoordinates2 /* may be NULL */, newFaces, currentState, current_options, curr_branch_);
		  else
		    addOneNodeToSSGFromPerFaceAndVertexTextureCoordinates2(newVertices, newPerFaceAndVertexTextureCoordinate2Lists, newFaces, currentState, 
					    current_options, curr_branch_);
	       }
	  }
     }
}
	
int ssgLoaderWriterMesh::checkMe()
// returns TRUE; if ok.
// Writes out errors by calling ulSetError with severity UL_WARNING,
// and a bit of debug info as UL_DEBUG
// May stop on first error.

// FixMe; todo: textureCoordinatePerVertex and tCPFAV. NIV135
{
   int i, oneIndex;
   class ssgIndexArray * vertexIndsForOneFace;
   class ssgTexCoordArray * textureCoordsForOneFace;

  // **** check theVertices *****
	if ( theVertices == NULL )
	{ if (( materialIndices == NULL ) &&
	      (theFaces == NULL ) &&
	      ( perFaceAndVertexTextureCoordinate2Lists == NULL ))
       {	ulSetError( UL_DEBUG, "LoaderWriterMesh::checkMe(): The mesh is empty\n");
	  return TRUE;
       }
	   else
	     {	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): No theVertices is NULL; but not the rest!\n");
		return FALSE;
	     }
	   
	}
	// **** check materialIndices and theMaterials *****
	/* FixMe; kludge: 2do. NIV135
	// one index per face:
	class ssgIndexArray *materialIndices; 

	theMaterials
	*/
	if ((( theMaterials == NULL ) && ( materialIndices != NULL )) ||
		  (( theMaterials != NULL ) && ( materialIndices == NULL )))
	{	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): "
	                     "One of theMaterials and materialIndices was NULL and the other != NULL!\n");
		return FALSE;
	}
	if ( materialIndices != NULL ) 
	{ for (i=0;i<materialIndices->getNum();i++)
		{ oneIndex = *materialIndices->get(i);
			assert(theMaterials!=NULL);
	    if (( oneIndex < 0 ) || ( oneIndex >= theMaterials->getNum()))
			{	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): "
													 "Material index out of range. Index = %d, "
													 "theMaterials->getNum() = %d.\n",
													 oneIndex, theMaterials->getNum());
				return FALSE;
			}
		}
	}


	// **** check theFaces *****
	// Each sublist is of type ssgIndexArray and contains the indexes of the vertices:
	if ( theFaces == NULL )
	{	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): There are vertices but no faces.\n");
		return FALSE;
	}
	for(i=0;i<theFaces->getNum();i++)
	{
	   vertexIndsForOneFace = *((ssgIndexArray **) theFaces->get ( i ));
	   if ( vertexIndsForOneFace == NULL )
	     {	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): the vertexindexes for one face are NULL!\n");
		return FALSE;
	     }
	}
   // **** check textureCoordinates *****
   // Each sublist is of type ssgTexCoordArray and contains the texture coordinates
	if ( perFaceAndVertexTextureCoordinate2Lists != NULL ) // may be NULL
	{ if ( theFaces->getNum() != perFaceAndVertexTextureCoordinate2Lists->getNum())
		{	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): "
		              "There must be as many faces in theFaces as in textureCoordinates. But "
		              "theFaces->getNum() =%d, tCPFAV->getNum() = %d!\n",
									theFaces->getNum(), perFaceAndVertexTextureCoordinate2Lists->getNum());
			return FALSE;
		}
		for(i=0;i<perFaceAndVertexTextureCoordinate2Lists->getNum();i++)
		{
		   textureCoordsForOneFace = *((ssgTexCoordArray **) perFaceAndVertexTextureCoordinate2Lists->get ( i ));
		   if ( textureCoordsForOneFace  != NULL ) // It is allowed that some or even all faces are untextured.
		     {
			vertexIndsForOneFace = *((ssgIndexArray **) theFaces->get ( i ));
			if ( textureCoordsForOneFace->getNum() != vertexIndsForOneFace ->getNum())
			  {
			     ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): Face %d: "
					 "Each face must have as many texture corrdinates (or none) as vertices. But "
					 "textureCoordsForOneFace->getNum() =%d, vertexIndsForOneFace ->getNum() = %d!\n",
					 i, textureCoordsForOneFace->getNum(), vertexIndsForOneFace ->getNum());
			     return FALSE;
			  }
		     }
		}
	}
   return TRUE; // success
}
