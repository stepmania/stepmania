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

//#include  "ssgLocal.h"

// ********************  small utility functions  ************************

void ssgAccumVerticesAndFaces( ssgEntity* node, sgMat4 transform, ssgVertexArray* vertices,
			       ssgIndexArray*  indices, SGfloat epsilon, 
			       ssgSimpleStateArray* ssa = NULL,
			       ssgIndexArray*  materialIndices = NULL,
			       ssgTexCoordArray *texCoordArray = NULL);

/*
  ssgTriangulate -
  Triangulate a simple polygon (possibly concave, but not self-intersecting).
  The number of triangles written to 'triangles' is returned, which is always
  less than or equal to num - 2. The polygon index array 'indices' may be null,
  in which case an identity mapping is assumed.
  Note that the implementation is optimized for small polygons, and since 
  the algorithm is O(num^2) it is not efficient on large polygons.
*/
int _ssgTriangulate( sgVec3 *vertices, int *indices, int num, int *triangles );


// ******************** class ssgLoaderWriterMesh ************************

class ssgListOfLists : public ssgSimpleList
// list of POINTERs to ssgSimpleLists
{
public:

   virtual ssgBase *clone ( int clone_flags = 0 ) { return NULL; }; // Fixme NIV14: 2do
   ssgListOfLists ( int init = 3 ) : ssgSimpleList ( sizeof(class ssgSimpleList*), init ) {} 
   class ssgSimpleList **get ( unsigned int n ) { return (class ssgSimpleList **) raw_get ( n ) ; }
   void   add ( class ssgSimpleList **thing ) { raw_add ( (char *) thing ) ; } ;
   void   set ( class ssgSimpleList **thing, unsigned int n ) { raw_set ( (char *) thing, n ) ; } ;
  
   virtual void print ( FILE *fd = stderr, char *indent = "", int how_much = 2 ) {}; // Fixme NIV14: 2do
} ;

//ssgSimpleState* 

class ssgSimpleStateList : public ssgSimpleList
// list of POINTERs to ssgSimpleStates
{
public:

  virtual ssgBase *clone ( int clone_flags = 0 ) { return NULL;  }; // Fixme NIV14: 2do
  ssgSimpleStateList( int init = 3 ) : ssgSimpleList ( sizeof(class ssgSimpleState*), init ) {} 
  class ssgSimpleState **get ( unsigned int n ) { return (class ssgSimpleState **) raw_get ( n ) ; }
  void   add ( class ssgSimpleState **thing ) { raw_add ( (char *) thing ) ; } ;
  void   set ( class ssgSimpleState **thing, unsigned int n  ) { raw_set ( (char *) thing, n ) ; } ;
  virtual void print ( FILE *fd = stderr, char *indent = "", int how_much = 2 ) {}; // Fixme NIV14: 2do
} ;

class ssgLoaderWriterMesh
{
   // ***** general ****
	 char *name;
   // array of Vec3s:
   class ssgVertexArray *theVertices;
   // one index per face:
   class ssgIndexArray *materialIndices; 

   // Each sublist is of type ssgIndexArray and contains the indexes of the vertices:
   class ssgListOfLists *theFaces; 
   // material list: 
   class ssgSimpleStateList *theMaterials;

   // ***** mode switches *****
   int textureCoordinatesArePerVertex; // and not per vertex and face (bTCs_are_per_vertex)
   // ***** complicated (texture coordinates are per face and vertex) mode *****
   // Each sublist is of type ssgTexCoordArray and contains the texture coordinates
   class ssgListOfLists *perFaceAndVertexTextureCoordinate2Lists; // was: tCPFAV = TextureCoordinatesPerFaceAndVertex
	
   // ***** easy (texture coordinates are per vertex) mode *********
   class ssgTexCoordArray *perVertexTextureCoordinates2; // was: tCPV = TextureCoordinatesPerVertex
   
   void addOneNodeToSSGFromPerVertexTextureCoordinates2( class ssgVertexArray *theVertices, 
							class ssgTexCoordArray *theTextureCoordinates2,
							class ssgListOfLists *theFaces,
							class ssgSimpleState *currentState,// Pfusch, kludge. NIV135
							class ssgLoaderOptions* current_options,
							class ssgBranch *curr_branch_ ); // was: AddOneNode2SSGFromCPV
   void addOneNodeToSSGFromPerFaceAndVertexTextureCoordinates2( class ssgVertexArray *theVertices, 
							       class ssgListOfLists *theTextureCoordinate2Lists,
							       class ssgListOfLists *theFaces,
							       class ssgSimpleState *currentState,// Pfusch, kludge. NIV135
							       class ssgLoaderOptions* current_options,
							       class ssgBranch *curr_branch_ ); // was: AddOneNodeToSSGFromCPFAV
   
 public:
   
   class ssgVertexArray *getVertices(void) { return theVertices; }
   void setVertices( class ssgVertexArray *vertexArray );
   class ssgTexCoordArray *getPerVertexTextureCoordinates2(void) { return perVertexTextureCoordinates2; }
   void setPerVertexTextureCoordinates2( class ssgTexCoordArray *texCoordArray );
	 void setName( const char *meshName ); 

   void addToSSG(
		 class ssgSimpleState *currentstate, 
		 class ssgLoaderOptions* currentOptions,
		 class ssgBranch *curr_branch_ );
	
   // construction/destruction:
   ssgLoaderWriterMesh();
   ~ssgLoaderWriterMesh();
   void reInit(void);
   void deletePerFaceAndVertexTextureCoordinates2(); // was: deleteTCPFAV

  // creation:
   void createVertices( int numReservedVertices = 8 ) ; // was: ThereAreNVertices
   void addVertex( sgVec3 v ) ;
   
   void createFaces( int numReservedFaces = 3 ) ; // was: ThereAreNFaces
   void addFace( ssgIndexArray **indexArray ) ;
   void addFaceFromIntegerArray( int numVertices, int *vertices );// AddFaceFromCArray

   void createPerFaceAndVertexTextureCoordinates2( int numReservedTextureCoordinate2Lists = 3 ) ; // ThereAreNTCPFAV
   void addPerFaceAndVertexTextureCoordinate2( ssgTexCoordArray **textureCoordinateArray ) ; // addTCPFAV

   void createPerVertexTextureCoordinates2( int numReservedTextureCoordinates2 = 3 ); // was: ThereAreNTCPV
   void addPerVertexTextureCoordinate2( sgVec2 textureCoordinate ); // was; addTCPV
   
   void createMaterialIndices( int numReservedMaterialIndices = 3 ); // ThereAreNMaterialIndexes
   void addMaterialIndex( short materialIndex ) ;

   void createMaterials( int numReservedMaterials = 3 ); // was: ThereAreNMaterials
   void addMaterial( class ssgSimpleState **simpleState ) ;

   unsigned int getNumVertices(void) { return theVertices->getNum(); } ;
   unsigned int getNumFaces   (void) { return theFaces->getNum(); } ;
   unsigned int getNumMaterials(void) { return theMaterials->getNum(); } ;


   // tools:
   int checkMe();
};


