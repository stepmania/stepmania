/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002 William Lachance, Steve Baker
 
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
*/

// ssgLoadVRML1: loads vrml1 files into the scenegraph
// Written by William Lachance (wlach@interlog.com)
// known bugs/limitations:
// - implicit texture mapping is not supported
// - explicit normal definitions not supported (they are calculated automatically)
// - only a very small subset of the inventor spec is supported
// - there is no support for primitives (cubes, spheres, cylinders, or cones)
// - no support for materials
// - no support for weblinks in vrml(well, this isn't much of a tragedy) :-)
// ..
// this loader borrows, to some extent, from the previous vrml file loader:
// - array based token identification
// - flipping the axises so that z is up

#include "ssgLocal.h"
#include "ssgParser.h"
#include "ssgLoaderWriterStuff.h"

#include "ssgLoadVRML.h"

static _ssgParserSpec parser_spec =
{
   "\r\n\t, ",  // delim_chars_skipable
     0,          // delim_chars_non_skipable
     "{[",        // open_brace_chars
     "}]",        // close_brace_chars
     '"',        // quote_char
     '#',          // comment_char
     0           // comment_string
};

_ssgParser vrmlParser;
static ssgLoaderOptions* currentOptions = NULL ;
static _nodeIndex *definedNodes = NULL;

static bool vrml1_parseSeparator( ssgBranch *parentBranch, _traversalState *parentData, char *defName );
static bool vrml1_parseSwitch( ssgBranch *parentBranch, _traversalState *parentData, char *defName );
static bool vrml1_parseIndexedFaceSet( ssgBranch *parentBranch, _traversalState *currentData, char *defName );
static bool vrml1_parseTexture2( ssgBranch *parentBranch, _traversalState *currentData, char *defName );

static _parseTag vrmlTags [] =
{
     { "Separator", vrml1_parseSeparator },
     { "Switch", vrml1_parseSwitch },
     { "IndexedFaceSet", vrml1_parseIndexedFaceSet },
     { "Coordinate3", vrml1_parseCoordinate3 },
     { "TextureCoordinate2", vrml1_parseTextureCoordinate2 },
     { "Texture2", vrml1_parseTexture2 },
     { "ShapeHints", vrml1_parseShapeHints },
     { "MatrixTransform", vrml1_parseMatrixTransform },
     { "Scale", vrml1_parseScale },
     { "Rotation", vrml1_parseRotation },
     { "Translation", vrml1_parseTranslation },
     { NULL, NULL },
};

ssgEntity *ssgLoadVRML1( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  currentOptions = ssgGetCurrentOptions () ;

   if ( !vrmlParser.openFile( fname, &parser_spec ) ) {
    ulSetError ( UL_WARNING, "ssgLoadVRML1: Failed to open '%s' for reading", fname ) ;
    return 0;
   }

   definedNodes = new _nodeIndex();
   
   // check for a valid header header
   char *token;
   if( !(token =  vrmlParser.getRawLine()) )
     return 0;
   if( strstr( token, "#VRML V1.0 ascii" ) == NULL )
       {
	  ulSetError ( UL_WARNING, "ssgLoadVRML1: valid vrml1 header not found" );
	  return 0;
       }
   
   // creating a root node.. (a transform that changes Zup with Yup)
   ssgBranch *rootTransform = new ssgTransform();
   sgCoord *tmpCoord = new sgCoord();
   sgSetCoord( tmpCoord, 0.0f, 0.0f, 0.0f, 0.0f, 90.0f, 0.0f );
   ((ssgTransform *)rootTransform)->setTransform( tmpCoord );
   
   vrmlParser.expectNextToken( "Separator" );

   if( !vrml1_parseSeparator( (ssgBranch *)rootTransform, NULL, NULL ) )
     {
	ulSetError ( UL_WARNING, "ssgLoadVRML: Failed to extract valid object(s) from %s", fname ) ;
	delete( definedNodes );
	delete( rootTransform );
	return NULL ;
     }
   
   vrmlParser.closeFile();
   delete( definedNodes );
   
   return (ssgBranch *)rootTransform;
}

static bool vrml1_parseSeparator( ssgBranch *parentBranch, _traversalState *parentData, char *defName )
{   
   char *childDefName = NULL;
   
   char *token;
   
   vrmlParser.expectNextToken( "{" );

   // create a branch for this node
   ssgBranch *currentBranch = new ssgBranch();
   
   if( defName != NULL ) 
     {
	currentBranch->setName( defName );
	definedNodes->insert( currentBranch );
     }
   
   _traversalState *currentData;
   if( parentData == NULL )
     currentData = new _traversalState();
   else
     currentData = parentData->clone();
   
   token = vrmlParser.getNextToken( NULL );
   while( strcmp( token, "}" ) )
     {	
 	if( !strcmp( token, "DEF" ) )
	  {
	     token = vrmlParser.getNextToken( NULL );
	     ulSetError(UL_DEBUG, "DEF: Found an object definition %s.", token);
	     childDefName = new char[50];
	     strncpy( childDefName, token, 50);
	  }
	else if( !strcmp( token, "USE" ) )
	  {
	     token = vrmlParser.getNextToken( NULL );
	     ulSetError(UL_DEBUG, "USE: Found a use directive %s.", token);
	     if( !vrml1_parseUseDirective( currentBranch, currentData, token, childDefName ) )
	       {
		  delete( currentBranch );
		  delete( currentData );
		  if( childDefName != NULL )
		    delete [] childDefName;		  
		  return FALSE;
	       }
	  }
	else
	  {  
	     int i=0; bool tokenFound = FALSE;
	     while( vrmlTags[i].token != NULL && !tokenFound ) 
	       {		  
		  if( !strcmp( token, vrmlTags[i].token ) )
		    {
		       if( !(vrmlTags[i].func( currentBranch, currentData, childDefName ) ) )
			 {
			    delete( currentBranch );
			    delete( currentData );
			    if( childDefName != NULL )
			      delete [] childDefName;			    
			    return FALSE;
			 }
		       
		       tokenFound = TRUE;
		    }
		  i++;
	       }
	     if( !tokenFound )
	       parseUnidentified();
	  }
	token = vrmlParser.getNextToken( NULL );
     }  

   parentBranch->addKid( currentBranch );
   
   // delete the currentData structure (we may use its content, but not its form)
   delete( currentData );
   
   return TRUE;
}

static bool vrml1_parseSwitch( ssgBranch *parentBranch, _traversalState *parentData, char *defName )
// UNSUPPORTED BEHAVIOUR: does not do a check for a whichChild parameter. Assumes that a switch
// "hides" all of its children.
{   
   char *childDefName = NULL;
   
   char *token;
   
   vrmlParser.expectNextToken( "{" );

   // create a branch for this node
   ssgBranch *currentBranch = new ssgSelector();
   ((ssgSelector *)currentBranch)->select( 0 ); // fixme: allow for children to be traversed
   
   
   if( defName != NULL ) 
     {
	currentBranch->setName( defName );
	definedNodes->insert( currentBranch );
      }
   
   _traversalState *currentData;
   if( parentData == NULL )
     currentData = new _traversalState();
   else
     currentData = parentData->clone();
   
   token = vrmlParser.getNextToken( NULL );
   
   while( strcmp( token, "}" ) )
     {
 	if( !strcmp( token, "DEF" ) )
	  {
	     token = vrmlParser.getNextToken( NULL );
	     ulSetError(UL_DEBUG, "DEF: Found an object definition %s.", token);
	     if( childDefName != NULL )
	       delete [] childDefName;
	     childDefName = new char[ strlen( token ) + 1];
	     strcpy( childDefName, token );
	  }
	else if( !strcmp( token, "USE" ) )
	  {
	     token = vrmlParser.getNextToken( NULL );
	     ulSetError(UL_DEBUG, "USE: Found a use directive %s.", token);
	     if( !vrml1_parseUseDirective( currentBranch, currentData, token, childDefName ) )
	       {
		  delete( currentBranch );
		  delete( currentData );
		  if( childDefName != NULL )
		    delete [] childDefName;			    
		  return FALSE;
	       }
	  }
	else
	  {  
	     int i=0; bool tokenFound = FALSE;
	     while( vrmlTags[i].token != NULL && !tokenFound ) 
	       {		  
		  if( !strcmp( token, vrmlTags[i].token ) )
		    {
		       if( !(vrmlTags[i].func( currentBranch, currentData, childDefName ) ) )
			 {
			    delete( currentBranch );
			    delete( currentData );
			    if( childDefName != NULL )
			      delete [] childDefName;			    
			    return FALSE;
			 }

		       tokenFound = TRUE;
		    }
		  i++;
	       }
	     if( !tokenFound )
	       parseUnidentified();
	  }

	token = vrmlParser.getNextToken( NULL );
     }  

   parentBranch->addKid( currentBranch );
   
   delete( currentData ); // delete the currentData structure (we may use its content, but not its form)
   
   return TRUE;
}


// parseVec: tries to parse a vec (of vSize), returns true if successful, false otherwise
bool parseVec( SGfloat *v, int vSize )
{
  for( int i=0; i<vSize; i++ ) {
    if( !vrmlParser.getNextFloat( v[i], NULL ) )
       {
	  ulSetError ( UL_WARNING, "ssgLoadVRML: Expected a float for a vector, didn't get it." ) ;
	  return FALSE;
       }
  }
   
   return TRUE;
}

// tries to parse an index (ie: integer) array (of arbitrary size, delimited by -1)
ssgIndexArray * parseIndexArray( _traversalState *currentData )
{
   ssgIndexArray *indexArray = new ssgIndexArray();
   char *token;
      
   token = vrmlParser.peekAtNextToken( NULL );
   while( strcmp( token, "-1" ) ) 
     {
	int index;
	if( vrmlParser.getNextInt( index, NULL ) )
	  indexArray->add( index );
	else
	  return NULL;
	
	token = vrmlParser.peekAtNextToken( NULL );
     }
   vrmlParser.expectNextToken( "-1" );

   // we have to reverse vertex ordering if vertices are in clockwise order
   if( currentData->getFrontFace() == GL_CW )
     {
	// so return something else that goes in reverse order
       	ssgIndexArray *reversedIndexArray = new ssgIndexArray( indexArray->getNum() );
	for( int i=(indexArray->getNum()-1); i>=0; i-- )
	  {
	     int newIndex = (int)*indexArray->get( i );
	     reversedIndexArray->add( newIndex );
	  }
	delete( indexArray );
	return reversedIndexArray;
     }
   
   return indexArray;
}

// parseCoordinate3: parses a list of 3d coordinates, adds them to the current
// vertice array
bool vrml1_parseCoordinate3( ssgBranch *parentBranch, _traversalState *currentData, char *defName )
{
   char *token;
   int numVertices = 0;
   
   // Ok, now we can allocate a new vertex table
   //ssgVertexArray *currentVertices = new ssgVertexArray();
   ssgVertexArray *currentVertices = new ssgVertexArray();
   if( defName != NULL ) 
     {
	currentVertices->setName( defName );
	definedNodes->insert( currentVertices );
     }
   
   vrmlParser.expectNextToken("{");
   vrmlParser.expectNextToken("point");
   
   // an array? most likely..
   token = vrmlParser.peekAtNextToken( NULL );
   if( !strcmp( token, "[" ) )
       {
	  vrmlParser.expectNextToken("[");
	  // begin parsing vertices
	  token = vrmlParser.peekAtNextToken( NULL );

	  while( strcmp( token, "]" ) )
	    {
	       sgVec3 v;
	       if( ! parseVec( v, 3 ) )
		 return FALSE;
	       numVertices++;
	       currentVertices->add( v );
	       
	       token = vrmlParser.peekAtNextToken( NULL );
	    }
	  vrmlParser.expectNextToken("]");
       }
    
   // otherwise it must be a singular value
   else 
     {
	sgVec3 v;
	if( ! parseVec( v, 3 ) )
	  return FALSE;
	numVertices++;
	currentVertices->add( v );
     }
       
   ulSetError(UL_DEBUG, "Level: %i. Found %i vertices here.", vrmlParser.level, numVertices);

   vrmlParser.expectNextToken("}");
   
   currentData->setVertices( currentVertices );

   return TRUE;
}

bool vrml1_parseTextureCoordinate2( ssgBranch *parentBranch, _traversalState *currentData, char *defName )
{
   char *token;
   int numTextureCoordinates = 0;
   
   ssgTexCoordArray *currentTextureCoordinates = new ssgTexCoordArray();
   if( defName != NULL )
     {
	currentTextureCoordinates->setName( defName );
	definedNodes->insert( currentTextureCoordinates );
     }
   
   vrmlParser.expectNextToken("{");
   vrmlParser.expectNextToken("point");
   
   // an array? most likely..
   token = vrmlParser.peekAtNextToken( NULL );
   if( !strcmp( token, "[" ) )
       {
	  vrmlParser.expectNextToken("[");
	  // begin parsing TexCoords
	  token = vrmlParser.peekAtNextToken( NULL );

	  while( strcmp( token, "]" ) )
	    {
	       sgVec2 v;
	       if( ! parseVec( v, 2 ) )
		 return FALSE;
	       numTextureCoordinates++;
	       currentTextureCoordinates->add( v );
	       
	       token = vrmlParser.peekAtNextToken( NULL );
	    }
	  vrmlParser.expectNextToken("]");
       }
    
   // otherwise it must be a singular value
   else 
     {
	sgVec2 v;
	if( ! parseVec( v, 2 ) )
	  return FALSE;
	numTextureCoordinates++;
	currentTextureCoordinates->add( v );
     }
       
   ulSetError(UL_DEBUG, "Level: %i. Found %i TexCoords here.", vrmlParser.level, numTextureCoordinates);

   vrmlParser.expectNextToken("}");
   
   currentData->setTextureCoordinates( currentTextureCoordinates );

   return TRUE;
}

static bool vrml1_parseIndexedFaceSet( ssgBranch *parentBranch, _traversalState *currentData, char *defName )
{
   char *token;
   bool texCoordIndexGiven = FALSE;
      
   ssgBranch *currentBranch = new ssgBranch();
   if( defName != NULL )
     {
	currentBranch->setName( defName );
	definedNodes->insert( currentBranch );
     }
   
   ssgLoaderWriterMesh *loaderMesh = new ssgLoaderWriterMesh();
   loaderMesh->createFaces();
   loaderMesh->setVertices( currentData->getVertices() );
   if( currentData->getTexture() != NULL && currentData->getTextureCoordinates() != NULL )
     loaderMesh->createPerFaceAndVertexTextureCoordinates2();

   vrmlParser.expectNextToken("{");
   
   token = vrmlParser.peekAtNextToken( NULL );
   while( strcmp( token, "}" ) )
     {
	if( !strcmp( token, "coordIndex" ) )
	  {
	     vrmlParser.expectNextToken("coordIndex");
	     if( !vrml1_parseCoordIndex( loaderMesh, currentData ) ) 
	       {
		  delete( currentBranch );
		  delete( loaderMesh );
		  return FALSE;	     
	       }
	     
	  }
	
	else if( !strcmp( token, "textureCoordIndex" ) )
	  {
	     texCoordIndexGiven = TRUE;
	     vrmlParser.expectNextToken("textureCoordIndex");
	     if( !vrml1_parseTextureCoordIndex( loaderMesh, currentData ) ) 
	       {
		  delete( currentBranch );
		  delete( loaderMesh );
		  return FALSE;
	       }
	     
	  }
	else
	  token = vrmlParser.getNextToken( NULL );
   
	token = vrmlParser.peekAtNextToken( NULL );
     }
   
   //ulSetError(UL_DEBUG, "Level: %i. Found %i faces here.", vrmlParser.level, numFaces);

   vrmlParser.expectNextToken( "}" );
   
   // -------------------------------------------------------
   // add the face set to ssg
   // -------------------------------------------------------

   // kludge. We need a state for addToSSG:
   ssgSimpleState * ss = new ssgSimpleState () ; // (0) ?
   ss -> setMaterial ( GL_AMBIENT, 0.5, 0.5, 0.5, 1.0);
   ss -> setMaterial ( GL_DIFFUSE, 1.0, 1.0, 1.0, 1.0) ; // 0.8, 0.8, 1.0, 1.0f
   ss -> setMaterial ( GL_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   ss -> setMaterial ( GL_EMISSION, 0.0, 0.0, 0.0, 1.0);
   ss -> setShininess ( 20 ) ; // Fixme, NIV14: Is that correct?

   // -------------------------------------------------------
   // texturing stuff
   // -------------------------------------------------------
   // todo: give an implicit mapping if texture coordinates are not given
   // todo: add support for per-vertex texturing
   if( currentData->getTexture() != NULL && currentData->getTextureCoordinates() != NULL && texCoordIndexGiven ) 
     {
	ss -> setTexture ( currentData->getTexture() );
	ss -> enable( GL_TEXTURE_2D );	
     }
   else
	ss -> disable( GL_TEXTURE_2D );
   
   ss -> disable ( GL_COLOR_MATERIAL ) ;
   //ss -> enable ( GL_COLOR_MATERIAL ) ;
   //ss -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
   
   ss -> enable  ( GL_LIGHTING       ) ;
   ss -> setShadeModel ( GL_SMOOTH ) ;
   
   ss  ->disable(GL_ALPHA_TEST); //needed?
   
   ss -> disable ( GL_BLEND ) ;
   
   ss -> setOpaque () ;

   if( !currentData->getEnableCullFace() )
     ss->disable( GL_CULL_FACE );
   
   if( !loaderMesh->checkMe() )
     {
	delete( currentBranch );
	delete( loaderMesh );
	return FALSE;
     }
   
   if( currentData->getTransform() != NULL )
     {
	currentBranch->addKid( currentData->getTransform() ); // FIXME: in case we're reusing transforms, perhaps they should be reinstanced? (currently we don't allow transforms to be defed)
 	loaderMesh->addToSSG( ss, currentOptions, currentData->getTransform() );
     }
   else
 	loaderMesh->addToSSG( ss, currentOptions, currentBranch );
   
   parentBranch->addKid( currentBranch );

   delete( loaderMesh );
   
   return TRUE;
}

static bool vrml1_parseTexture2( ssgBranch *parentBranch, _traversalState *currentData, char *defName )
{
   char *token;
   char *fileName = NULL; bool wrapU = FALSE, wrapV = FALSE;

   vrmlParser.expectNextToken("{");

   token = vrmlParser.peekAtNextToken( NULL );
   while( strcmp( token, "}" ) )
     {
	if( !strcmp( token, "filename") )
	  {
	     vrmlParser.expectNextToken("filename");
	     token = vrmlParser.getNextToken( NULL );
	     fileName = new char[ strlen( token ) + 1];
	     strcpy( fileName, token );
	  }
	else if( !strcmp( token, "wrapS") )
	  {
	     vrmlParser.expectNextToken("wrapS");
	     token = vrmlParser.getNextToken( NULL );
	     if( !strcmp( token, "REPEAT") )
	       wrapU = TRUE;
	  }
	else if( !strcmp( token, "wrapT") ) 
	  {
	     vrmlParser.expectNextToken("wrapT");
	     token = vrmlParser.getNextToken( NULL );
	     if( !strcmp( token, "REPEAT") )
	       wrapV = TRUE;
	  }
	else
	  token = vrmlParser.getNextToken( NULL );
	
	token = vrmlParser.peekAtNextToken( NULL );
     }
   
   
   if( fileName == NULL )
     return FALSE;
   
   //ssgTexture *currentTexture = new ssgTexture( fileName, wrapU, wrapV );
   ssgTexture *currentTexture = currentOptions -> createTexture ( fileName, wrapU, wrapV );
   currentData->setTexture( currentTexture );
   vrmlParser.expectNextToken("}");

   delete [] fileName;
   
   return TRUE;
}

bool vrml1_parseShapeHints( ssgBranch *parentBranch, _traversalState *currentData, char *defName )
{
   char *token;
   vrmlParser.expectNextToken("{");

   token = vrmlParser.peekAtNextToken( NULL );
   while( strcmp( token, "}" ) )
     {
	if( !strcmp( token, "vertexOrdering") )
	  {
	     vrmlParser.expectNextToken("vertexOrdering");
	     token = vrmlParser.getNextToken( NULL );
	     if( !strcmp( token, "CLOCKWISE") )
	       {
		  currentData->setEnableCullFace( TRUE );
		  currentData->setFrontFace( GL_CW );
	       }
	     else if( !strcmp( token, "COUNTERCLOCKWISE") ) 
	       {
		  currentData->setEnableCullFace( TRUE );
		  currentData->setFrontFace( GL_CCW );
	       }
	     else if( !strcmp( token, "UNKNOWN_ORDERING") )
		  currentData->setEnableCullFace( FALSE );
	     else
	       {
		  ulSetError ( UL_WARNING, "ssgLoadVRML: invalid vertex ordering directive" ) ;
		  return FALSE;
	       }
	     
	  }
	else
	  token = vrmlParser.getNextToken( NULL );

	token = vrmlParser.peekAtNextToken( NULL );	
     }
   vrmlParser.expectNextToken("}");
   
   return TRUE;
}

bool vrml1_parseMatrixTransform( ssgBranch *parentBranch, _traversalState *currentData, char *defName )
{
   ssgTransform *currentTransform = new ssgTransform();
   sgMat4 transformMat;

   vrmlParser.expectNextToken("{");
   vrmlParser.expectNextToken("matrix");
   for( unsigned int i=0; i<4; i++ )
     for( unsigned int j=0; j<4; j++ ) 
       {
	  if( !vrmlParser.getNextFloat( transformMat[i][j], NULL ) ) 
	    {
	       ulSetError ( UL_WARNING, "ssgLoadVRML: Expected a float for a matrix, didn't get it." ) ;
	       return FALSE;
	    }
       }
   vrmlParser.expectNextToken("}");

   currentTransform->setTransform( transformMat );

   applyTransform( currentTransform, currentData );

   //ulSetError(UL_DEBUG, "Found a Matrix Transform (%f, %f, %f %f), (%f, %f, %f %f), (%f, %f, %f %f), (%f, %f, %f %f)", xForm[0][0], xForm[1][0], xForm[2][0], xForm[3][0],
   //	  xForm[0][1], xForm[1][1], xForm[2][1], xForm[3][1],
   //	  xForm[0][2], xForm[1][2], xForm[2][2], xForm[3][2],
   //	  xForm[0][3], xForm[1][3], xForm[2][3], xForm[3][3] );
    
   return TRUE;
}

bool vrml1_parseScale( ssgBranch *parentBranch, _traversalState *currentData, char *defName )
{
   ssgTransform *currentTransform = new ssgTransform();
   sgVec3 scaleFactor;
   
   sgCoord moveFactor; sgZeroCoord( &moveFactor );
   
   vrmlParser.expectNextToken("{");
   vrmlParser.expectNextToken("scaleFactor");
   if( !parseVec( scaleFactor, 3 ) )
     return FALSE;
   vrmlParser.expectNextToken("}");

   currentTransform->setTransform( &moveFactor, scaleFactor[0], scaleFactor[1], scaleFactor[2] );
   
   applyTransform( currentTransform, currentData );
   
   ulSetError(UL_DEBUG, "Found a scale transform: %f %f %f", scaleFactor[0], scaleFactor[1], scaleFactor[2] );
   
   return TRUE;
}

bool vrml1_parseRotation( ssgBranch *parentBranch, _traversalState *currentData, char *defName )
{
   ssgTransform *currentTransform = new ssgTransform();
   sgVec3 axis;
   SGfloat angle;
   sgMat4 rotation;
   
   vrmlParser.expectNextToken("{");
   vrmlParser.expectNextToken("rotation");
   if( !parseVec( axis, 3 ) )
     return FALSE;
   if( !vrmlParser.getNextFloat( angle, NULL ) )
     return FALSE;
   vrmlParser.expectNextToken("}");

   angle *= SG_RADIANS_TO_DEGREES;
   
   sgMakeRotMat4( rotation, angle, axis ) ;
   currentTransform->setTransform( rotation );
   
   applyTransform( currentTransform, currentData );
   
   ulSetError(UL_DEBUG, "Found a rotation: %f %f %f %f", axis[0], axis[1], axis[2], angle );
   
   return TRUE;
}

bool vrml1_parseTranslation( ssgBranch *parentBranch, _traversalState *currentData, char *defName )
{
   ssgTransform *currentTransform = new ssgTransform();
   sgVec3 transform;
   
   vrmlParser.expectNextToken("{");
   vrmlParser.expectNextToken("translation");
   if( !parseVec( transform, 3 ) )
     return FALSE;
   vrmlParser.expectNextToken("}");

   currentTransform->setTransform( transform );
   
   applyTransform( currentTransform, currentData );
   
   ulSetError(UL_DEBUG, "Found a translation: %f %f %f", transform[0], transform[1], transform[2] );
   
   return TRUE;
}


bool vrml1_parseUseDirective( ssgBranch *parentBranch, _traversalState *currentData, char *useName, char *defName )
{
   // find the node within the list of defined nodes
   ssgBase *node = definedNodes->extract( useName );

   if( node==NULL )
     return TRUE;

   if( node->isA( ssgTypeBranch() ) ) 
     {
	ssgBranch *currentBranch = NULL;
	if( currentData->getTransform() != NULL )
	  {
	     currentBranch = currentData->getTransform();
	     currentBranch->addKid( (ssgEntity *)node );
	  }
	else 
	  currentBranch = (ssgBranch *)node;
	
	parentBranch->addKid( currentBranch );
	
	return TRUE;
     }
   
   return TRUE;
}


// parseUnidentified: A node that we either don't support or isn't part of the
// VRML/IV spec. Just skip it.
bool parseUnidentified()
{
   char *token;
   
   int startLevel = vrmlParser.level;
   int currentLevel = startLevel + 1;
   
   vrmlParser.expectNextToken("{");
   
   while( currentLevel != startLevel )
     {
	token = vrmlParser.getNextToken( NULL );

	if( !strcmp( token, "{" ) )
	  currentLevel++;
	else if( !strcmp( token, "}" ) )
	  currentLevel--;	
     }
   
   return TRUE;
}

void applyTransform( ssgTransform *currentTransform, _traversalState *currentData )
{
   if( currentData->getTransform() == NULL )
     currentData->setTransform( currentTransform );
   else
     {
	ssgTransform *newTransform = new ssgTransform();
	mergeTransformNodes( newTransform, currentTransform, currentData->getTransform() );
	// this will have to be changed when we allow use declarations on transforms
	delete( currentTransform );
	currentData->setTransform( newTransform );
     }
}

bool vrml1_parseCoordIndex( ssgLoaderWriterMesh *loaderMesh, _traversalState *currentData )
{
   char *token = vrmlParser.peekAtNextToken( NULL );
   int numFaces = 0;
   
   // an array? most likely..
   if( !strcmp( token, "[" ) ) 
     {
	vrmlParser.expectNextToken("[");
	token = vrmlParser.peekAtNextToken( NULL );
	while( strcmp( token, "]" ) ) 
	  {
	     ssgIndexArray *currentFaceIndices = parseIndexArray( currentData );
	     if( currentFaceIndices == NULL )
	       {
		  ulSetError ( UL_WARNING, "ssgLoadVRML1: invalid index list" ) ;
		  return FALSE;
	       }
	     loaderMesh->addFace( (ssgIndexArray **) &currentFaceIndices );
	     //ulSetError(UL_DEBUG, "Level: %i. Added a face with %i vertices", vrmlParser.level, numVerticesInFace );
	     numFaces++;
	     
	     token = vrmlParser.peekAtNextToken( NULL );
	  }
	vrmlParser.expectNextToken( "]" );
     }
   
   // otherwise a single point
   else 
     {
	ssgIndexArray *currentFaceIndices = parseIndexArray( currentData );
	if( currentFaceIndices == NULL )
	  {
	     ulSetError ( UL_WARNING, "ssgLoadVRML1: invalid index list" ) ;
	     return FALSE;
	  }
	loaderMesh->addFace( (ssgIndexArray **) &currentFaceIndices );
	numFaces++;
	
	vrmlParser.expectNextToken( "-1" ); 
     }	     
   
   return TRUE;
}

bool vrml1_parseTextureCoordIndex( ssgLoaderWriterMesh *loaderMesh, _traversalState *currentData )
{
   char *token = vrmlParser.peekAtNextToken( NULL );

   // an array? most likely..
   if( !strcmp( token, "[" ) ) 
     {
	vrmlParser.expectNextToken("[");
	token = vrmlParser.peekAtNextToken( NULL );
	while( strcmp( token, "]" ) ) 
	  {
	     ssgIndexArray *currentTextureCoordinateIndices = parseIndexArray( currentData );
	     if( currentTextureCoordinateIndices == NULL )
	       return FALSE;
	     ssgTexCoordArray *currentPerFaceAndVertexTextureCoordinateList = new ssgTexCoordArray( currentTextureCoordinateIndices->getNum() );
	     for( int i=0; i<currentTextureCoordinateIndices->getNum(); i++ )
	       currentPerFaceAndVertexTextureCoordinateList->add( (currentData->getTextureCoordinates())->get( (unsigned int)*currentTextureCoordinateIndices->get( i ) ) );
	     loaderMesh->addPerFaceAndVertexTextureCoordinate2( (ssgTexCoordArray **) &currentPerFaceAndVertexTextureCoordinateList );
	     
	     delete( currentTextureCoordinateIndices );
	     //ulSetError(UL_DEBUG, "Level: %i. Added a face with %i vertices", vrmlParser.level, numVerticesInFace );
	     
	     token = vrmlParser.peekAtNextToken( NULL );
	  }
	vrmlParser.expectNextToken( "]" );
     }
   // otherwise a single point
   else
     {
	ssgIndexArray *currentTextureCoordinateIndices = parseIndexArray( currentData );
	if( currentTextureCoordinateIndices == NULL )
	  return FALSE;
	ssgTexCoordArray *currentPerFaceAndVertexTextureCoordinateList = new ssgTexCoordArray( currentTextureCoordinateIndices->getNum() );
	for( int i=0; i<currentTextureCoordinateIndices->getNum(); i++ )
	  currentPerFaceAndVertexTextureCoordinateList->add( (currentData->getTextureCoordinates())->get( (unsigned int)*currentTextureCoordinateIndices->get( i ) ) );
	loaderMesh->addPerFaceAndVertexTextureCoordinate2( (ssgTexCoordArray **) &currentPerFaceAndVertexTextureCoordinateList );
	
	delete( currentTextureCoordinateIndices );
	//ulSetError(UL_DEBUG, "Level: %i. Added a face with %i vertices", vrmlParser.level, numVerticesInFace );
     }
   
   return TRUE;
}

   
void mergeTransformNodes( ssgTransform *newTransform, ssgTransform *oldTransform1, ssgTransform *oldTransform2 )
{
   sgMat4 oldTransformMat1;
   sgMat4 oldTransformMat2;
   sgMat4 newTransformMat;

   oldTransform1->getTransform( oldTransformMat1 );
   oldTransform2->getTransform( oldTransformMat2 );
   sgMultMat4( newTransformMat, oldTransformMat1, oldTransformMat2 ) ;
   newTransform->setTransform( newTransformMat );
}
