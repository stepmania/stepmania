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

/***************************************************************************
                          ssgloadmd2.cpp  -  description
                             -------------------
    begin                : Thu Sep 7 2000
    copyright            : (C) 2000 by Jon Anderson
    email                : janderson@onelink.com
 ***************************************************************************/
/*******************************************************
 **  This was written to be a part of Stephen J Bakers
 **  PLIB (http://plib.sourceforge.net)
 *******************************************************/

#include "ssgLocal.h"

//defaults
#define FRAME_DELAY 3
#define NUM_SEQUENCES 22

/**
  MD2 file loader for SSG.

  ssgLoadMD2 loads a given MD2 file into an
  ssgSelector hiearchy. Each animation sequence is
  loaded into a ssgTimedSelector.  Each sequence is
  then put into a top level ssgSelector.  In addition,
  all the skins are loaded into a ssgStateSelector that
  is attached to each frame's ssgVtxTable.  Animation
  sequences can then be played by:

  ssgSelector::selectStep ( SSG_MD2_STAND );
	ssgTimedSelector *ts = (ssgTimedSelector *)s -> getKid ( i );
	ts -> setMode ( SSG_ANIM_SHUTTLE );
	ts -> control ( SSG_ANIM_START );
	
	
	Notes:
	-Not all md2 files will contain the complete animation
	 sequences.	
	-Not all md2 files contain skin info/locations.
	-By default, it displays each animation frame for
	 3 screen frames. MD2 animations were designed to be
	 played at ~ 10hz
	
	MD2 File format documentaion:
	http://www.ugrad.cs.jhu.edu/~dansch/md2	

	MD2 Frames documentation:
	http://www.planetquake.com/polycount/resources/quake2/q2frameslist.shtml
	
	MD2 Models:
	http://www.planetquake.com/polycount/

*/


/*
int seq_sizes[][2] = {
	{ SSG_MD2_STAND,	40 },
	{ SSG_Md2_RUN,		6 },
	{ SSG_MD2_ATTACK, 8 },
	{ SSG_MD2_PAIN_1, 4 },
	{ SSG_MD2_PAIN_2, 4 },
	{ SSG_MD2_PAIN_3, 4 },
	{ SSG_MD2_JUMP,   6 },
	{ SSG_MD2_FLIPOFF,12 },
	{ SSG_MD2_SALUTE, 11 },
	{ SSG_MD2_TAUNT,  17 },
	{ SSG_MD2_WAVE,   11 },
	{ SSG_MD2_POINT,  12 },
	{ SSG_MD2_CROUCH_STAND, 19 },
	{ SSG_MD2_CROUCH_WALK,  9 },
	{ SSG_MD2_CROUCH_ATTACK, 9 },
	{ SSG_MD2_CROUCH_PAIN,  4 },
	{ SSG_MD2_CROUCH_DEATH, 5 },
	{ SSG_MD2_DEATH_1,      6 },
	{ SSG_MD2_DEATH_2,      6 },
	{ SSG_MD2_DEATH_3,      6 },
	{ SSG_MD2_ALL,				1 },
	{ SSG_MD2_POSE,      1 }
};
	
*/

static int seq_frames[] = {
	40,
	6,
	8,
	4,
	4,
	4,
	6,
	12,
	11,
	17,
	11,
	12,
	19,
	9,
	9,
	4,
	5,
	6,
	6,
	6,
	1,    //this may or may not be in the file.
	1     //this may or may not be in the file.
};


typedef struct
{
	int magic;
	int version;
	int skinWidth;
	int skinHeight;
	int frameSize;
	int numSkins;
	int numVertices;
	int numTexCoords;
	int numTriangles;
	int numGlCommands;
	int numFrames;
	int offsetSkins;
	int offsetTexCoords;
	int offsetTriangles;
	int offsetFrames;
	int offsetGlCommands;
	int offsetEnd;
} t_model;

typedef struct
{
	unsigned char vertex[3];
	unsigned char lightNormalIndex;
} t_vertex;


typedef struct
{
	short vertexIndices[3];
	short textureIndices[3];

} t_triangle;
		
typedef struct
{
	short s, t;
} t_tcoord;

typedef struct
{
	float s, t;
	int vertexIndex;
} t_glCommandVertex;

typedef struct
{
		float scale[3];
		float translate[3];
		char name[16];
} t_frame;


static int is_little_endian;

static ssgLoaderOptions* current_options = NULL ;
static FILE *loader_fd;

static t_tcoord *uvs;
static t_frame *frames;
static t_vertex **vertices;
static t_triangle *triangles;
static char skins[32][1024];
static t_model header;


static void read_header()
{
	fread(&header, sizeof(t_model), 1, loader_fd);	
}
	
static void read_frames(int offset)
{
	fseek(loader_fd, offset, SEEK_SET);	
		
	frames = new t_frame[header.numFrames];
	vertices = new t_vertex* [header.numFrames];

	for(int i=0; i<header.numFrames; i++){
		vertices[i] = new t_vertex[header.numVertices];

		fread(&frames[i], sizeof(t_frame), 1, loader_fd);
		fread(vertices[i], sizeof(t_vertex), header.numVertices, loader_fd);
	}
}

static void read_uvcoords(int offset)
{
	fseek(loader_fd, offset, SEEK_SET);
	
	uvs = new t_tcoord[header.numTexCoords];
	fread(uvs, sizeof(t_tcoord), header.numTexCoords, loader_fd);
}

static void read_triangles(int offset)
{
	fseek(loader_fd, offset, SEEK_SET);

	triangles = new t_triangle[header.numTriangles];
	fread(triangles, sizeof(t_triangle), header.numTriangles, loader_fd);
}

static void read_skins(int offset)
{
	fseek(loader_fd, offset, SEEK_SET);
	char buffer[64];
	for(int i=0; i<header.numSkins; i++){
		fread(buffer, sizeof(char), 64, loader_fd);
		//strip off the preciding path, and the trailing ext
		//replacing it with rgb.
			char * start = strrchr(buffer, '/');
			assert ( start != NULL );
			start++;
			/*start[strlen(start)-1] = 'b';
			start[strlen(start)-2] = 'g';
			start[strlen(start)-3] = 'r';*/
			
			strcpy(skins[i], start);
	}
}
static void read_glcommands(int offset)
{
	fseek(loader_fd, offset, SEEK_SET);
}

/* Converts the MD2 structure to one palatable by ssg.
 * Mainly, this involves duplicating vertices so that
 * they can share UVs properly
 * Right now, it does this really naively, (it dups them all)
 */
static ssgEntity * convert_to_ssg()
{
	sgVec3 vert;
	sgVec2 uv;

	
	/**Load all the skins into a state selector
	  */
	int num_skins = header.numSkins;
	if(num_skins == 0)
		num_skins = 1;
	
	ssgStateSelector *states = new ssgStateSelector(num_skins);
	bool stated = false;
  int i;
	
	for(i=0; i<header.numSkins; i++){
		stated = true;

		ssgSimpleState *state = new ssgSimpleState();
		state ->  enable ( GL_TEXTURE_2D );
		//state ->  setTexture( filepath ) ;
		state ->  setTexture( current_options -> createTexture ( skins[i] ) );
		states -> setStep ( i, state );	
	}
	
	/**Activate the first state*/
	if(stated)
		states -> selectStep( 0 );
	else {
		ulSetError(UL_WARNING, "ssgLoadMD2: No skins specified in MD2 file!");
		ssgSimpleState *state = new ssgSimpleState();
		state -> disable ( GL_TEXTURE_2D );
		states -> setStep(0, state);
		states -> selectStep( 0 );
	}
	
	ssgSelector *sequences;
	
	if(header.numFrames == 200)
		 sequences = new ssgSelector(NUM_SEQUENCES);
	else
		 sequences = new ssgSelector(NUM_SEQUENCES - 2);		
	
	int current_sequence_num = 0;
	int current_sequence_frame = 0;
	
	ssgTimedSelector *current_sequence  = new ssgTimedSelector(seq_frames[current_sequence_num]);
	sequences->addKid(current_sequence);
	
	for(i=0; i<header.numFrames; i++){
		ssgVertexArray *vlist	= new ssgVertexArray  ( header.numTriangles * 3 );
		ssgTexCoordArray *tlist = new ssgTexCoordArray( header.numTriangles * 3 );
		
		for(int j=0; j<header.numTriangles; j++){
			for(int k=2; k>-1; k--){
				int vert_index = triangles[j].vertexIndices[k];

				int bx = vertices[i][vert_index].vertex[0];
				int by = vertices[i][vert_index].vertex[1];
				int bz = vertices[i][vert_index].vertex[2];
				
				vert[0] = ((float)bx)*frames[i].scale[0] + frames[i].translate[0];
				vert[1] = ((float)by)*frames[i].scale[1] + frames[i].translate[1];
				vert[2] = ((float)bz)*frames[i].scale[2] + frames[i].translate[2];
			
				uv[0] =     ((float)uvs[triangles[j].textureIndices[k]].s)/header.skinWidth;
				uv[1] = 1 - ((float)uvs[triangles[j].textureIndices[k]].t)/header.skinHeight;	
				
				vlist -> add( vert );
				tlist -> add( uv );
			}
		}
		ssgVtxTable *vtab = new ssgVtxTable ( GL_TRIANGLES, vlist, 0, tlist, 0 ) ;
		vtab -> setState(states);
		
		current_sequence ->addKid(vtab);
		current_sequence_frame++;
		
		if( current_sequence_frame > seq_frames[current_sequence_num] -1){
			current_sequence -> setLimits( 0, seq_frames[current_sequence_num]-1 );
			current_sequence -> setDuration( FRAME_DELAY );
			current_sequence -> setMode ( SSG_ANIM_ONESHOT );
			
			current_sequence_num ++;
			current_sequence_frame = 0;
			current_sequence = new ssgTimedSelector(seq_frames[current_sequence_num]);
			sequences -> addKid( current_sequence );
		}
		
	}
	
	sequences -> selectStep( 0 );
	return sequences;
	
}


ssgEntity * ssgLoadMD2( const char *filename, const ssgLoaderOptions* options)
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  int j = 1 ;
  is_little_endian = *((char *) &j );

  char filepath [ 1024 ] ;
  current_options -> makeModelPath ( filepath, filename ) ;
  
  loader_fd = fopen ( filepath, "rb" );

  if ( loader_fd == NULL ) {
    ulSetError ( UL_WARNING, "ssgLoadMD2: Failed to open '%s' for reading", filepath ) ;
    return NULL ;
  } 


	read_header();
	read_skins(header.offsetSkins);
	read_uvcoords(header.offsetTexCoords);
	read_triangles(header.offsetTriangles);
	read_frames(header.offsetFrames);
	read_glcommands(header.offsetGlCommands);
	

	ssgEntity * model = convert_to_ssg();

	//clean up the memory used by the MD2 structure;
	delete[] uvs;
	delete[] frames;
	delete[] triangles;
	for(int i=0; i<header.numFrames; i++){
		delete[] vertices[i];
	}

	delete[] vertices;

	return model;
}
