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

//
// .X loader for SSG/PLIB
// .X is the 3D file format for Micro$ofts directX retained mode.
// Written by Wolfram Kuss (Wolfram.Kuss@t-online.de) in Oct/Nov of 2000
//
#include  "ssgLocal.h"
#include "ssgLoaderWriterStuff.h" 
#include "ssgParser.h"

#define u32 unsigned int

// These functions return TRUE on success
typedef int HandlerFunctionType(const char *sName, const char *firstToken);


static char *globEmpty="";

static ssgBranch *curr_branch_;

struct EntityType
{
  const char * sName;
	HandlerFunctionType *HandleEntity;
	int bMayBeIgnored;
} ;



static /* const */ ssgLoaderOptions* current_options = NULL ;

static _ssgParserSpec parser_spec =
{
   "\r\n\t ",  // delim_chars_skipable
   ",;",       // delim_chars_non_skipable
   "{",          // open_brace_chars
   "}",          // close_brace_chars
   '"',        // quote_char
   '#',          // comment_char
	 "//"        // comment_string
} ;

   
static _ssgParser parser;
static ssgBranch* top_branch;

static int Ascii2Int(int &retVal, const char *token, const char* name )
// returns TRUE on success
{
  char *endptr;
  retVal = int(strtol( token, &endptr, 10));
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ parser.error("The field %s should contain an integer number but contains %s",name, token) ;
		return FALSE;
	}
}

static int Ascii2UInt(unsigned int &retVal, const char *token, const char* name )
// returns TRUE on success
{
  char *endptr;
  retVal = (unsigned int)(strtol( token, &endptr, 10));
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ parser.error("The field %s should contain an integer number but contains %s",name, token) ;
		return FALSE;
	}
}

static int Ascii2Float(SGfloat &retVal, const char *token, const char* name )
// returns TRUE on success
{
  char *endptr;
  retVal = SGfloat(strtod( token, &endptr));
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ parser.error("The field %s should contain an integer number but contains %s",name, token) ;
		return FALSE;
	}
}

static int HandleHeader(const char * /* sName */, const char *firstToken)
{
	//parser.expectNextToken("{");
	int Dummy;
  if (! Ascii2Int(Dummy, firstToken, "Header.major"))
		return FALSE;
		
	parser.expectNextToken(";");
	if (!parser.getNextInt(Dummy, "Header.minor"))
		return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextInt(Dummy, "Header.flags"))
		return FALSE;
	parser.expectNextToken(";");
	parser.expectNextToken("}");
	return TRUE;
}

static void IgnoreEntity(int startLevel)
// startLevel should be 0 when you are "in front of the {" (normally)
// or 1 when you have already parsed the "{"
{ 
	int Level = startLevel;
  char *token;

	while ( TRUE)
	{ token = parser.getNextToken(0);
		if ( parser.eof ) 
		{ parser.error("unexpected end fo file\n");
			return ; //FALSE;
		}
        
		assert(token!=NULL); 
    if ( strcmp(token,"{") == 0 )
			Level++;
		else if ( strcmp(token,"}") == 0 )
		{ assert(Level>0); // Fixme, NIV14: Handle this gracefully.
			if (Level==1) 
		    return; // found THE closing brace of entitiy
		  Level--; // Found A closing brace
		}
	}      
}

static int HandleMesh(const char *sName, const char *firstToken);
static int HandleMeshMaterialList(const char *sName, const char *firstToken);
static int HandleTextureCoords(const char *sName, const char *firstToken);
static int HandleMaterial(const char *sName, const char *firstToken);
static int HandleTextureFileName(const char *sName, const char *firstToken);


static EntityType aEntities[] =
{
	{ "Header", HandleHeader, FALSE}, 
	{ "Vector", NULL, FALSE}, 
	{ "Coords2d", NULL, FALSE}, 
	{ "Quaternion", NULL, FALSE}, 
	{ "Matrix4x4", NULL, FALSE}, 
	{ "ColorRGBA", NULL, FALSE}, 
	{ "ColorRGB", NULL, FALSE}, 
	{ "Indexed Color", NULL, FALSE}, 
	{ "Boolean", NULL, FALSE}, 
	{ "Boolean2d", NULL, FALSE}, 
	{ "Material", HandleMaterial, FALSE}, 
	{ "TextureFilename", HandleTextureFileName, FALSE},  
	{ "MeshFace", NULL, FALSE}, 
	{ "MeshFaceWraps", NULL, FALSE}, 
	{ "MeshTextureCoords", HandleTextureCoords, FALSE}, 
	{ "MeshNormals", NULL, TRUE}, 
	{ "MeshVertexColors", NULL, FALSE}, 
	{ "MeshMaterialList", HandleMeshMaterialList, FALSE}, 
	{ "Mesh", HandleMesh, FALSE}, 
	{ "FrameTransformMatrix", NULL, FALSE}, 
	{ "Frame", NULL, FALSE}, 
	{ "FloatKeys", NULL, FALSE}, 
	{ "TimedFloatKeys", NULL, FALSE}, 
	{ "AnimationKey", NULL, FALSE}, 
	{ "AnimationOptions", NULL, FALSE}, 
	{ "Animation", NULL, FALSE}, 
	{ "AnimationSet", NULL, FALSE}, 
	{ "template", NULL, TRUE},
  { NULL, NULL, FALSE}
};


static int ParseEntity(char *token)
// called recursively
{ int i=0;

	while(aEntities[i].sName!=NULL)
	{ if (!strcmp(token,aEntities[i].sName))
		{	if (aEntities[i].HandleEntity)
			{	char *sNextToken, *sName=globEmpty;
				sNextToken=parser.getNextToken(0);
				if ( parser.eof ) 
				{ parser.error("unexpected end fo file\n");
					return FALSE;
				}
        sName = NULL;
				if (0 != strcmp(sNextToken, "{"))
				{ sName = new char[ strlen(sNextToken) + 1 ];
					assert ( sName );
					strcpy(sName, sNextToken);
					sNextToken = parser.getNextToken(0);
					if (0 != strcmp(sNextToken, "{"))
						parser.error("\"{\" expected\n");
				}
				sNextToken = parser.getNextToken(0);
				
				if(sNextToken[0] == '<') // UUID
					sNextToken = parser.getNextToken(0);
				if ( parser.eof ) 
				{ parser.error("unexpected end fo file\n");
					return FALSE;
				}
        
				if (!aEntities[i].HandleEntity(sName, sNextToken))
					return FALSE;
				if ( sName )
					delete [] sName;
			}
			else
				if (aEntities[i].bMayBeIgnored)
					IgnoreEntity ( 0 );
				else
				{
					parser.error("I am sorry, but Entity-typ '%s' is not yet implemented.", aEntities[i].sName);
					return FALSE ;
				}
				
			break;
		}
		i++;
	}
	if (aEntities[i].sName==NULL)
	{
		parser.error("unexpected token %s", token);
		return FALSE ;
	}
	return TRUE;
}



#define MAX_NO_VERTICES_PER_FACE 1000

static class ssgLoaderWriterMesh currentMesh;

static ssgSimpleState *currentState;
extern sgVec4 currentDiffuse;

static int HandleTextureFileName(const char * /*sName*/, const char *firstToken)
{/*
	  TextureFilename {
    "../image/box_top.gif";
   } #TextureFilename
 */
  char *filename_ptr, *filename = new char [ strlen(firstToken)+1 ] ;
	assert(filename!=NULL);
  strcpy ( filename, firstToken ) ;
	filename_ptr = filename ;
	
	if ( filename_ptr[0] == '"' ) 
		filename_ptr++;
	if (filename_ptr[strlen(filename_ptr)-1] == '"')
		filename_ptr[strlen(filename_ptr)-1] = 0;
  currentState -> setTexture( current_options -> createTexture( filename_ptr ) );
  currentState -> enable( GL_TEXTURE_2D );


	parser.expectNextToken(";");
	parser.expectNextToken("}");
	delete [] filename;
	return TRUE;
}

static int HandleMaterial(const char * /*sName*/, const char *firstToken)
// return TRUE on success
{ SGfloat power;
  int bFoundTextureFileName = FALSE;
	sgVec4 specularColour, EmissiveColour;

	// read body
	if (! Ascii2Float(currentDiffuse[0], firstToken, "Facecolour R"))
		return FALSE;

	parser.expectNextToken(";");
	if (!parser.getNextFloat(currentDiffuse[1], "Facecolour G")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(currentDiffuse[2], "Facecolour B")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(currentDiffuse[3], "Facecolour A")) return FALSE;
	parser.expectNextToken(";");
	parser.expectNextToken(";");
	if (!parser.getNextFloat(power, "power")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(specularColour[0], "Specular R")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(specularColour[1], "Specular G")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(specularColour[2], "Specular B")) return FALSE;
	specularColour[3] = 0.0;
	parser.expectNextToken(";");
	parser.expectNextToken(";");
	if (!parser.getNextFloat(EmissiveColour[0], "Emissive R")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(EmissiveColour[1], "Emissive G")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(EmissiveColour[2], "Emissive B")) return FALSE;
	EmissiveColour[3] = 0.0;
	parser.expectNextToken(";");
	parser.expectNextToken(";");

	// create SimpleState

  currentState = new ssgSimpleState () ;

//  currentState -> setMaterial ( GL_AMBIENT, mat -> amb ) ;
  currentState -> setMaterial ( GL_DIFFUSE, currentDiffuse) ;
  currentState -> setMaterial ( GL_SPECULAR, specularColour) ;
  currentState -> setMaterial ( GL_SPECULAR, specularColour[0], 
		                      specularColour[1], specularColour[2], currentDiffuse[3] ) ;
  currentState -> setMaterial ( GL_EMISSION, EmissiveColour[0], 
		                      EmissiveColour[1], EmissiveColour[2], currentDiffuse[3] ) ;
	
	currentState -> setShininess ( power ) ; // Fixme, NIV14: Is that correct?

  currentState -> enable ( GL_COLOR_MATERIAL ) ;
  currentState -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

  currentState -> enable  ( GL_LIGHTING       ) ;
  currentState -> setShadeModel ( GL_SMOOTH ) ;

  if ( currentDiffuse[3] > 0.0f )
  {
    currentState -> disable ( GL_ALPHA_TEST ) ;
    currentState -> enable  ( GL_BLEND ) ;
    currentState -> setTranslucent () ;
  }
  else
  {
    currentState -> disable ( GL_BLEND ) ;
    currentState -> setOpaque () ;
  }
  currentState -> disable( GL_TEXTURE_2D );



	while(TRUE)
	{ char *nextToken =parser.getNextToken(0);
	  if (0==strcmp("}", nextToken))
		{ currentMesh.addMaterial( &currentState );
			return TRUE; // Material is finished. success
		}
		
		if ( 0!= strcmp("TextureFilename", nextToken) )
		{ parser.error("TextureFilename expected!\n");
			return FALSE; 
		}
		if ( bFoundTextureFileName )
		{ parser.error("Only one TextureFileName per Material please!\n");
			return FALSE; 
		}
		if (!ParseEntity(nextToken)) // read "TextureFileName"
			return FALSE;
		bFoundTextureFileName = TRUE;
	}
	return TRUE; //lint !e527
}

static int HandleTextureCoords(const char * /* sName */, const char *firstToken)
{
	u32 nNoOfVertices, i;

	  
	if (! Ascii2UInt(nNoOfVertices, firstToken, "nNoOfVertices"))
		return FALSE;

	if ( nNoOfVertices != currentMesh.getNumVertices())
	{ parser.error("No of vertices of mesh (%d) and no "
	            "of texture coordinates (%d) do not match!\n" 
							"Therefore the texture coordinates are ignored!",
							( int ) currentMesh.getNumVertices(), ( int ) nNoOfVertices );
	  IgnoreEntity ( 1 ); // ignores TC.
		return FALSE;
	}
	currentMesh.createPerVertexTextureCoordinates2( nNoOfVertices ) ;

	parser.expectNextToken(";");
	for(i=0;i<nNoOfVertices;i++)
	{ 
		sgVec2 tv;
      
    if (!parser.getNextFloat(tv[0], "x"))
			return FALSE;
		parser.expectNextToken(";");
		if (!parser.getNextFloat(tv[1], "y"))
			return FALSE;
		parser.expectNextToken(";");
		if(i==nNoOfVertices-1)
			parser.expectNextToken(";");
		else
			parser.expectNextToken(",");

		currentMesh.addPerVertexTextureCoordinate2( tv ) ;
	}
	parser.expectNextToken("}");
	return TRUE;
}

static int HandleMeshMaterialList(const char * /* sName */, const char *firstToken)
{
	u32 i, nFaceIndexes, nMaterialsRead = 0, nMaterials;
	  
	if (! Ascii2UInt(nMaterials, firstToken, "nMaterials"))
		return FALSE;

	parser.expectNextToken(";");
	currentMesh.createMaterials( nMaterials );
	if (!parser.getNextUInt(nFaceIndexes, "number of Face Indexes"))
		return FALSE;
	currentMesh.createMaterialIndices( nFaceIndexes ) ;
	parser.expectNextToken(";");


	if ( nFaceIndexes > currentMesh.getNumFaces())
	{ parser.error("No of face indexes of materiallist (%d) is greater than then no "
	            "of faces (%d)!\n" 
							"Therefore the material list is ignored!",
							( int ) nFaceIndexes, ( int ) currentMesh.getNumFaces());
	  IgnoreEntity ( 1 ); // ignores TC.
		return TRUE; // go on parsing
	}
	if ( nFaceIndexes > currentMesh.getNumFaces())
	  parser.message("Informational: No of face indexes of materiallist (%d) is less than then no "
	            "of faces (%d)\n" ,
							( int ) nFaceIndexes, ( int ) currentMesh.getNumFaces());
	for ( i=0 ; i<nFaceIndexes ; i++ )
	{
		int iIndex, j;
		char *ptr;
		if (!parser.getNextInt(iIndex, "Face index"))
			return FALSE;
		currentMesh.addMaterialIndex ( iIndex ) ;
		// I don't quite know why, but different .X files I have have a 
		// different syntax here, some have one ";" and some two.
		// Therefore, the following code
		for (j=0;j<2;j++)
		{ ptr = parser.peekAtNextToken( "," );
		  if ( strlen(ptr) == 1)
				if ( (ptr[0]==',') || (ptr[0]==';') )
				{ ptr = parser.getNextToken( "," ); // ignore this token
				}
  
		}
	}
	while(TRUE)
	{ char *nextToken =parser.getNextToken(0);
	  if (0==strcmp("}", nextToken))
		{ if ( nMaterialsRead < nMaterials )
		    parser.error("Too few Materials!\n");
			//else	parser.error("Success! MeshMaterialList!\n");
			return TRUE; // Mesh is finished. success
		}
		if ( 0!= strcmp("Material", nextToken) )
		{ parser.error("Material expected!\n");
			return FALSE; 
		}
		if ( nMaterialsRead >= nMaterials )
		{ parser.error("Too many Materials!\n");
			return FALSE; 
		}
		if (!ParseEntity(nextToken)) // read "Material"
			return FALSE;
		nMaterialsRead++;
	}
	return TRUE; //lint !e527
}


static int HandleMesh(const char * sName, const char *firstToken)
{ u32 i, j, nNoOfVertices, nNoOfVerticesForThisFace, nNoOfFaces;
	int iVertex, aiVertices[MAX_NO_VERTICES_PER_FACE];
	
	//char *sMeshName = parser.getNextToken("Mesh name");
	//parser.expectNextToken("{");
	if (! Ascii2UInt(nNoOfVertices, firstToken, "nNoOfVertices"))
		return FALSE;

	//parser.getNextInt("number of vertices");

	currentMesh.reInit ();
	currentMesh.setName( sName );
	currentMesh.createVertices( nNoOfVertices );
 
	parser.expectNextToken(";");
	for(i=0;i<nNoOfVertices;i++)
	{ 
		sgVec3 vert;
      
    if (!parser.getNextFloat(vert[0], "x"))
			return FALSE;
		parser.expectNextToken(";");
		if (!parser.getNextFloat(vert[1], "y"))
			return FALSE;
		parser.expectNextToken(";");
		if (!parser.getNextFloat(vert[2], "z"))
			return FALSE;
		parser.expectNextToken(";");
		if(i==nNoOfVertices-1)
			parser.expectNextToken(";");
		else
			parser.expectNextToken(",");

		currentMesh.addVertex(vert);
	}
	if (!parser.getNextUInt(nNoOfFaces, "number of faces"))
		 return FALSE;
	currentMesh.createFaces( nNoOfFaces );

  
  
	parser.expectNextToken(";");
	for(i=0;i<nNoOfFaces;i++)
	{ if (!parser.getNextUInt(nNoOfVerticesForThisFace , "number of vertices for this face"))
	    return FALSE;
		assert(nNoOfVerticesForThisFace<MAX_NO_VERTICES_PER_FACE);
	
		// parse faces and put the info into the array aiVertices

		parser.expectNextToken(";");
		for(j=0;j<nNoOfVerticesForThisFace;j++)
		{ if (!parser.getNextInt(iVertex, "Vertex index"))
				return FALSE;

			aiVertices[j]=iVertex;
			
			if(j==nNoOfVerticesForThisFace-1)
				parser.expectNextToken(";");
			else
				parser.expectNextToken(",");
		}
		if(i==nNoOfFaces-1)
			parser.expectNextToken(";");
		else
			parser.expectNextToken(",");
		
		// use array aiVertices
#ifdef NOT_PLIB
		CreateFaceInMdi_Edit(vl, nNoOfVerticesForThisFace, aiVertices); // This line is for the "Mdi" 3D Editor, NOT for plib
#else
		currentMesh.addFaceFromIntegerArray(nNoOfVerticesForThisFace, aiVertices); 
#endif
	}
	while(TRUE)
	{ char *nextToken =parser.getNextToken(0);
	  if ( parser.eof ) 
		{ parser.error("unexpected end fo file\n");
			return FALSE;
		}
    
	  if (0==strcmp("}", nextToken))
			break; // Mesh is finished
		if (!ParseEntity(nextToken))
			return FALSE;
	}

//
	if ( currentState == NULL )
		currentState = new ssgSimpleState();

	currentMesh.addToSSG(
		currentState // Pfusch, kludge. NIV135
		,
		current_options,
		curr_branch_);
	return TRUE;
}

inline int TwoCharsToInt(char char1, char char2)
{
  return ((int)(char1-'0'))*256+char2-'0';
}

static int HeaderIsValid(char *firstToken)
{	// xof 0302txt 0064
  if (strcmp(firstToken,"xof"))
  {
    parser.error("not X format, invalid Header");
    return FALSE ;
  }
	char* token = parser.getNextToken("2nd Header field");
	if (strlen(token)!=7)
	{
    parser.error("not X format, invalid Header");
    return FALSE ;
  }
	char *sp=&(token[4]);
	if (strcmp(sp,"txt"))
  {
    if (strcmp(sp,"bin"))
			parser.error("not X format, invalid Header");
		else
			parser.error("Binary X format files are not supported. If you have access to Windows, "
			             "please use Microsofts conversion-utility convx from the directX-SDK "
									 "to convert to ascii.");
    return FALSE ;
  }
	if (strncmp(token, "0302", 4))
		parser.message("This loader is written for X-file-format version 3.2.\n" 
					"AFAIK this is the only documented version.\n"
					"Your file has version %d.%d\n"
					"Use the file at your own risk\n", 
					TwoCharsToInt(token[0], token[1]),
					TwoCharsToInt(token[2], token[3]));
	token = parser.getNextToken("3rd Header field");
	if (strcmp(token,"0032") && strcmp(token,"0064"))
  {
    parser.error("not X format, invalid Header");
    return FALSE ;
  }
  return TRUE;
}

static int parse()
{
  int firsttime = TRUE;
  char* token;
  //int startLevel = parser.level;
	token = parser.getNextToken(0);
  while (! parser.eof )
	{ if (firsttime)
		{ 
			if(!HeaderIsValid(token))
					return FALSE;
			firsttime = FALSE;
		}
		else 
		{ if (!ParseEntity(token))
				return FALSE;
		}
		token = parser.getNextToken(0);
	}
  return TRUE ;
}


ssgEntity *ssgLoadX ( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  top_branch = new ssgBranch ;
	curr_branch_ = top_branch;
	if ( !parser.openFile( fname, &parser_spec ))
	{
    delete top_branch ;
		return 0;
  }
  if ( !parse() )
  {
		delete top_branch ;
		top_branch = 0 ;
  }
//  parse_free();
  parser.closeFile();

  return top_branch ;
}
