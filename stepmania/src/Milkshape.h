#ifndef Milkshape_H
#define Milkshape_H
/*
-----------------------------------------------------------------------------
 Class: Milkshape

 Desc: Types defined in msLib.h.  C arrays converted to use std::vector

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

/**********************************************************************
 *
 * Constants
 *
 **********************************************************************/

#define MS_MAX_NAME             32
#define MS_MAX_PATH             256



/**********************************************************************
 *
 * Types
 *
 **********************************************************************/

#ifndef byte
typedef unsigned char byte;
#endif /* byte */

#ifndef word
typedef unsigned short word;
#endif /* word */


typedef struct msVec2 { 
	float v[2];
	operator float* ()				{ return v; };
    operator const float* () const	{ return v; };
} msVec2;
typedef struct msVec3 { 
	float v[3];
	operator float* ()				{ return v; };
    operator const float* () const	{ return v; };
} msVec3;
typedef struct msVec4 { 
	float v[4];
	operator float* ()				{ return v; };
    operator const float* () const	{ return v; };
} msVec4;


/* msFlag */
typedef enum {
    eSelected = 1, eSelected2 = 2, eHidden = 4, eDirty = 8, eAveraged = 16, eUnused = 32
} msFlag;

/* msVertex */
typedef struct msVertex
{
//  byte        nFlags;
    msVec3      Vertex;
    msVec2      uv;
    msVec3      Normal;
    char        nBoneIndex;
} msVertex;

/* msTriangle */
typedef struct
{
//  word        nFlags;
    word        nVertexIndices[3];
//  word        nNormalIndices[3];
//    msVec3      Normal;
//  byte        nSmoothingGroup;
} msTriangle;

/* msMesh */
typedef struct msMesh
{
    byte        nFlags;
    char        szName[MS_MAX_NAME];
    char        nMaterialIndex;
    
    vector<msVertex>   Vertices;

//    vector<msVec3>     Normals;

    vector<msTriangle> Triangles;
} msMesh;

/* msMaterial */
class RageTexture;

typedef struct msMaterial
{
    int         nFlags;
    char        szName[MS_MAX_NAME];
    msVec4      Ambient;
    msVec4      Diffuse;
    msVec4      Specular;
    msVec4      Emissive;
    float       fShininess;
    float       fTransparency;
    char        szDiffuseTexture[MS_MAX_PATH];
    char        szAlphaTexture[MS_MAX_PATH];	// not used in SM.  Use alpha in diffuse texture instead
    int         nName;	// not used in SM.  What is this for?
	RageTexture* pTexture;
} msMaterial;

/* msPositionKey */
typedef struct msPositionKey
{
    float       fTime;
    msVec3      Position;
} msPositionKey;

/* msRotationKey */
typedef struct msRotationKey
{
    float   fTime;
    msVec3  Rotation;
} msRotationKey;

/* msBone */
typedef struct msBone
{
    int             nFlags;
    char            szName[MS_MAX_NAME];
    char            szParentName[MS_MAX_NAME];
    msVec3          Position;
    msVec3          Rotation;

    vector<msPositionKey>  PositionKeys;

    int             nNumRotationKeys;
    int             nNumAllocedRotationKeys;
    vector<msRotationKey>  RotationKeys;
} msBone;

/* msModel */
typedef struct msModel
{
    vector<msMesh>     Meshes;

    vector<msMaterial> Materials;

    vector<msBone>     Bones;

	int FindBoneByName( const char* szName )
	{
		for( unsigned i=0; i<Bones.size(); i++ )
			if( strcmp(Bones[i].szName, szName)==0 )
				return i;
		return -1;
	}

    int         nFrame;
    int         nTotalFrames;

    msVec3      Position;
    msVec3      Rotation;
} msModel;

#endif
