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

static ssgLoaderOptions* currentOptions = NULL ;
static _nodeIndex *definedNodes = NULL;

static bool iv_parseSeparator( ssgBranch *parentBranch, _traversalState *parentData, char *defName );
static bool iv_parseSwitch( ssgBranch *parentBranch, _traversalState *parentData, char *defName );
static bool iv_parseIndexedFaceSet( ssgBranch *parentBranch, _traversalState *parentData, char *defName );
static bool iv_parseTexture2( ssgBranch *parentBranch, _traversalState *currentData, char *defName );

static _parseTag ivTags [] =
{
     { "Separator", iv_parseSeparator },
     { "Switch", iv_parseSwitch },
     { "IndexedFaceSet", iv_parseIndexedFaceSet },
     { "Coordinate3", vrml1_parseCoordinate3 },
     { "TextureCoordinate2", vrml1_parseTextureCoordinate2 },
     { "Texture2", iv_parseTexture2 },
     { "ShapeHints", vrml1_parseShapeHints },
     { "MatrixTransform", vrml1_parseMatrixTransform },
     { "Scale", vrml1_parseScale },
     { "Rotation", vrml1_parseRotation },
     { "Translation", vrml1_parseTranslation },
     { NULL, NULL },
};

ssgEntity *ssgLoadIV( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  currentOptions = ssgGetCurrentOptions () ;

   if ( !vrmlParser.openFile( fname, &parser_spec ) ) {
    ulSetError ( UL_WARNING, "ssgLoadIV: Failed to open '%s' for reading", fname ) ;
    return 0;
  }

   definedNodes = new _nodeIndex();
   
   // check for a valid header header
   char *token;
   if( !(token =  vrmlParser.getRawLine()) )
     return 0;
     if( strstr( token, "#Inventor V2.1 ascii" ) == NULL ) // should we handle different flavours of inventor?
       {
	  ulSetError ( UL_WARNING, "ssgLoadIV: valid iv header not found" );
	  return 0;
       }
   
   // creating a root node..
   ssgBranch *rootBranch = new ssgBranch();
   
   vrmlParser.expectNextToken( "Separator" );

   if( !iv_parseSeparator( rootBranch, NULL, NULL ) )
     {
	ulSetError ( UL_WARNING, "ssgLoadVRML: Failed to extract valid object(s) from %s", fname ) ;
	delete( rootBranch );
	delete( definedNodes );
	return NULL ;
     }
   
   vrmlParser.closeFile();
   delete( definedNodes );
   
   return rootBranch ;
}

static bool iv_parseSeparator( ssgBranch *parentBranch, _traversalState *parentData, char *defName )
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
	int i=0; bool tokenFound = FALSE;
	while( ivTags[i].token != NULL && !tokenFound ) 
	  {
	     if( !strcmp( token, ivTags[i].token ) )
	       {
		  if( !(ivTags[i].func( currentBranch, currentData, childDefName ) ) )
		    {
		       delete( currentBranch );
		       delete( currentData );
		       return FALSE;
		    }
		  tokenFound = TRUE;
	       }
	     
	     i++;
	  }
	if( !tokenFound )
	  parseUnidentified();
	
	token = vrmlParser.getNextToken( NULL );
     }
   
   parentBranch->addKid( currentBranch );
   
   delete( currentData ); // delete the currentData structure (we may use its content, but not its form)
   
   return TRUE;
}

static bool iv_parseSwitch( ssgBranch *parentBranch, _traversalState *parentData, char *defName )
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
	int i=0; bool tokenFound = FALSE;
	while( ivTags[i].token != NULL && !tokenFound ) 
	  {
	     if( !strcmp( token, ivTags[i].token ) )
	       {
		  if( !(ivTags[i].func( currentBranch, currentData, childDefName ) ) )
		    {
		       delete( currentBranch );
		       delete( currentData );
		       return FALSE;
		    }
		       
		  tokenFound = TRUE;
	       }
	     i++;
	  }
	if( !tokenFound )
	  parseUnidentified();
	
	token = vrmlParser.getNextToken( NULL );
     }  

   parentBranch->addKid( currentBranch );
   
   delete( currentData ); // delete the currentData structure (we may use its content, but not its form)
   
   return TRUE;
}

static bool iv_parseIndexedFaceSet( ssgBranch *parentBranch, _traversalState *currentData, char *defName )
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
		  delete( loaderMesh );
		  return FALSE;
	       }
	  }
	else
	  token = vrmlParser.getNextToken( NULL );
   
	token = vrmlParser.peekAtNextToken( NULL );
     }
   
   //ulSetError(UL_DEBUG, "Level: %i. Found %i faces here.\n", vrmlParser.level, numFaces);

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
	delete( loaderMesh );
	return FALSE; 
     }
   
   if( currentData->getTransform() != NULL ) 
     {
	currentBranch->addKid( currentData->getTransform() ); // FIXME: in case we're reusing transforms, perhaps they should be reinstanced? (currently we don't allow this)
 	loaderMesh->addToSSG( ss, currentOptions, currentData->getTransform() );
     }
   else
 	loaderMesh->addToSSG( ss, currentOptions, currentBranch );
   
   parentBranch->addKid( currentBranch );
   
   return TRUE;
}

static bool iv_parseTexture2( ssgBranch *parentBranch, _traversalState *currentData, char *defName )
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
	     if( !vrmlParser.getNextString( token, NULL ) )
	       return FALSE;
	     fileName = new char[ strlen( token ) + 1 ];
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
