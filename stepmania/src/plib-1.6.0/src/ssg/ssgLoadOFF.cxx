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
// .off loader for SSG/PLIB
// Warning: There are two formats called OFF
//
// There is Geomview off, see http://www.neuro.sfc.keio.ac.jp/~aly/polygon/format/off.html
// and http://www.graphics.cornell.edu/~gordon/peek/old/off.html
// This is the format implmented in this file.
//
// And there is off (Object File Format) by Digital Equipment Corporation, Workstation Systems Engineering
// (WSE), see for ex. http://www.dcs.ed.ac.uk/home/mxr/gfx/3d/OFF.spec.
// and http://www.landfield.com/faqs/graphics/fileformats-faq/part3/section-95.html
// We don't support this format.
                                 
// Written by Wolfram Kuss (Wolfram.Kuss@t-online.de) in February 2001
//
// We only support 2D and 3D data

#include  "ssgLocal.h"
#include "ssgLoaderWriterStuff.h" 
#include "ssgParser.h"

#define u32 unsigned int

static int _ssgLoadTranslucent=TRUE;

extern sgVec4 currentDiffuse;

void ssgSetLoadOFFTranslucent ( int i )
// this is a kludge. 
// it might be removed/replaced later on.
{
	_ssgLoadTranslucent = i;
}


static /* const */ ssgLoaderOptions* current_options = NULL ;

static _ssgParserSpec parser_spec =
{
   "\r\n\t ",  // delim_chars_skipable
   ",;",       // delim_chars_non_skipable
   "",          // open_brace_chars
   "",          // close_brace_chars
   '"',        // quote_char. not used for OFF
   '#',          // comment_char
	 "%"        // comment_string
} ;

   
static _ssgParser parser;
static ssgBranch* top_branch;

/*static int Ascii2Int(int &retVal, const char *token, const char* name )
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
}*/

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




#define MAX_NO_VERTICES_PER_FACE 1000

static class ssgLoaderWriterMesh theMesh;





static int _ssgNoFacesToRead=-1, _ssgNoVerticesToRead;

static int thereIs_ST=FALSE, thereIs_C=FALSE, thereIs_N=FALSE,
					thereIs_4=FALSE, thereIs_n=TRUE;
static unsigned int dimension; // may be 2 or 3

static int parse()
{
  char* token;

	token = parser.getLine( 0 );
	if ( token == NULL )
	{ parser.error("The file seems to be empty");
		return FALSE;
	}
	// If first line is "off", swallow it
	if ( strlen(token) >= 3 )
	{	char *p = token;
		p += strlen(token)-3;
		if (0==strcmp("OFF", p) )
		{ if ( strlen(token) > 3 )
		  // parse line
			{	thereIs_ST = ( NULL != strstr( token, "ST" ));
				thereIs_C = ( NULL != strstr( token, "C" ));
				thereIs_N = ( NULL != strstr( token, "N" ));
				thereIs_4 = ( NULL != strstr( token, "4" ));
				thereIs_n = ( NULL != strstr( token, "n" ));
				if ( thereIs_ST || thereIs_C || thereIs_N )
				{ parser.error("This is a \"ST\"- \"C\"- or \"N\"-OFF. These are not supoorted, sorry.");
					return FALSE;
				}
				if ( thereIs_4 && !thereIs_n )
				{ parser.error("This is a 4D-OFF file. We only support 3D data, sorry.");
					return FALSE;
				}
			}
			token = parser.getLine( 0 );
			assert( token != NULL );
		}
	}
	if ( thereIs_n )
	{ 
		if ( ! Ascii2UInt(dimension, token, "Dimension" ))
			return FALSE;
		if ( thereIs_4 && thereIs_n )
			dimension++;
		if ( (dimension != 3) &&  (dimension != 2))
		{ parser.error("This is a %udD-OFF file. We only support 2D and 3D data, sorry.", dimension);
			return FALSE;
		}
		token = parser.getLine( 0 ); // read space dimension
		assert( token != NULL );
	}
	// ****** read NVertices, NFaces and NEdges ************

	unsigned int uInt;
	int i;
  if ( ! Ascii2UInt(uInt, token, "NVertices"))
		return FALSE;
	_ssgNoVerticesToRead = uInt;

  token = parser.parseToken( "NFaces" );
  if ( ! Ascii2UInt(uInt, token, "NFaces"))
  	return FALSE;
	_ssgNoFacesToRead = uInt;

  token = parser.parseToken( "NEdges" );
  if ( ! Ascii2UInt(uInt, token, "NEdges"))
		return FALSE;
	
	// **** init theMesh ***
	theMesh.reInit ();
	theMesh.createVertices( _ssgNoVerticesToRead );
   theMesh.createFaces ( _ssgNoFacesToRead );

  	
	// ***** read Vertex Coords
	for ( i = 0; i < _ssgNoVerticesToRead ; i++ )
	{ 
		sgVec3 vert;
    token = parser.getLine( ); // may return NULL?
		assert(token!=NULL);
	 
		if (!Ascii2Float(vert[0], token, "x"))
			return FALSE;
		if (!parser.parseFloat(vert[1], "y"))
			return FALSE;
		if ( dimension == 2 )
			vert[2] = 0.0;
		else
		  if (!parser.parseFloat(vert[2], "z"))
			  return FALSE;
		theMesh.addVertex(vert);
	}
	// ********* Read Faces **********
	int iVertex, aiVertices[MAX_NO_VERTICES_PER_FACE];
	unsigned int nNoOfVerticesForThisFace, j;
	
	for(i=0;i<_ssgNoFacesToRead ;i++)
	{ token = parser.getLine( ); // may return NULL?
		assert(token!=NULL);
		if (!Ascii2UInt(nNoOfVerticesForThisFace , token, "number of vertices for this face"))
	    return FALSE;
		assert(nNoOfVerticesForThisFace<MAX_NO_VERTICES_PER_FACE);
	
		// parse faces and put the info into the array aiVertices

		for(j=0;j<nNoOfVerticesForThisFace;j++)
		{ if (!parser.parseInt(iVertex, "Vertex index"))
				return FALSE;

			aiVertices[nNoOfVerticesForThisFace-1-j] // experience says I have to invert the 
				               //order of vertices or I will get all backfaces in plib
				=iVertex;
			
		}
		
		// use array aiVertices
		theMesh.addFaceFromIntegerArray(nNoOfVerticesForThisFace, aiVertices); 
	}


	// ************** State stuff. *****************
	// wk: This is a mess :-(
	// I havent got this to work 100%, but it works partly:
	// If you say you want translucency, it is 100% transparent, not translucent/semi-transparent
	// If you say you want opaque, then it comes out black, whyever.
	ssgSimpleState * ss = new ssgSimpleState(0);
  if ( _ssgLoadTranslucent )
	{	ss -> setTranslucent () ;
		ss -> disable ( GL_ALPHA_TEST ) ;
		ss -> enable  ( GL_BLEND ) ;

	//new
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ss->setAlphaClamp(0.1f);
	}
	float *ssf=ss->getMaterial ( GL_DIFFUSE );
	ssf[0]=0.8f;
	ssf[1]=0.8f;
	ssf[2]=1.0f;
  if ( _ssgLoadTranslucent )
  	ssf[3]=0.4f;
	else
		ssf[3]=1.0f;
	sgCopyVec4 ( currentDiffuse, ssf );

	ssf=ss->getMaterial ( GL_EMISSION );
	ssf[0]=0.0f;
	ssf[1]=0.0f;
	ssf[2]=0.0f;
	ssf[3]=1.0f;
	ssf=ss->getMaterial ( GL_SPECULAR );
	ssf[0]=1.0f;
	ssf[1]=1.0f;
	ssf[2]=1.0f;
	ssf[3]=1.0f;
	ssf=ss->getMaterial ( GL_AMBIENT );
	ssf[0]=0.3f;
	ssf[1]=0.3f;
	ssf[2]=0.3f;
	ssf[3]=1.0f;


  if ( !_ssgLoadTranslucent )
	// This code has simply been copyied over from ssgLoadM, where it works
  {	ss->setOpaque();
		ss->disable(GL_BLEND);
		ss->disable(GL_ALPHA_TEST);
		ss->disable(GL_TEXTURE_2D);
		ss->enable(GL_COLOR_MATERIAL);
		ss->enable(GL_LIGHTING);
		ss->setShadeModel(GL_SMOOTH);
		ss->setMaterial(GL_AMBIENT , 0.7f, 0.7f, 0.0f, 1.0f);
		ss->setMaterial(GL_DIFFUSE , 0.7f, 0.7f, 0.0f, 1.0f);
		ss->setMaterial(GL_SPECULAR, 1.0f, 1.0f, 1.0f, 1.0f);
		ss->setMaterial(GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f);
		ss->setShininess(50);
	}


	theMesh.createMaterials( 1 );
	theMesh.addMaterial( &ss );
	theMesh.createMaterialIndices( _ssgNoFacesToRead ) ;
	for(i=0;i<_ssgNoFacesToRead ;i++)
	   theMesh.addMaterialIndex ( 0 ) ;
		

	theMesh.addToSSG(
		ss, // kludge. NIV135
		current_options,
		top_branch);

  return TRUE ;
}


ssgEntity *ssgLoadOFF ( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  top_branch = new ssgBranch ;
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
