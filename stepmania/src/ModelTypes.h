#ifndef ModelTypes_H
#define ModelTypes_H
/*
-----------------------------------------------------------------------------
 Class: ModelTypes

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

#include "RageTypes.h"


/* msFlag */
typedef enum {
    eSelected = 1, eSelected2 = 2, eHidden = 4, eDirty = 8, eAveraged = 16, eUnused = 32
} msFlag;

/* msVertex */
/*
typedef struct msVertex
{
//  byte        nFlags;	// we don't care about saving this flag
    RageVector3 Vertex;
    RageVector3 Normal;
    RageVector2 uv;
    char        nBoneIndex;
} msVertex;
*/

/* msTriangle */
typedef struct
{
//  word        nFlags;	// we don't care about saving this flag
    word        nVertexIndices[3];
//  word        nNormalIndices[3];	// we don't care about this.  Use the normals in each vertex
//  msVec3      Normal;	// we don't care about per-triangle normals.  Each vertex has its own
//  byte        nSmoothingGroup;	// we don't care about this, so don't save it
} msTriangle;


/* msMesh */
typedef struct msMesh
{
	msMesh();
	~msMesh();

//  byte        nFlags;	// we don't care about saving this flag
    char        szName[MS_MAX_NAME];
    char        nMaterialIndex;

	vector<RageModelVertex>   Vertices;

//  vector<msVec3>     Normals;	// each vertex holds its own normal

	// OPTIMIZATION: If all verts in a mesh are transformed by the same bone, 
	// then send the transform to the graphics card for the whole mesh instead
	// of transforming each vertex on the CPU;
	char        nBoneIndex;	// -1 = no bone

    vector<msTriangle> Triangles;
} msMesh;

/* msMaterial */
class RageTexture;

// merge this into Sprite?
struct AnimatedTexture
{
	AnimatedTexture();
	~AnimatedTexture();
	
	void Load( CString sTexOrIniFile );
	void Unload();
	void Update( float fDelta );
	
	RageTexture* GetCurrentTexture();
	
	void SetState( int iState );
	int GetNumStates();

	int iCurState;
	float fSecsIntoFrame;
	struct AnimatedTextureState
	{
		RageTexture* pTexture;
		float		fDelaySecs;
	};
	vector<AnimatedTextureState> vFrames;
};

typedef struct msMaterial
{
    int         nFlags;
    char        szName[MS_MAX_NAME];
    RageColor   Ambient;
    RageColor   Diffuse;
    RageColor   Specular;
    RageColor   Emissive;
    float       fShininess;
    float       fTransparency;
    char        szDiffuseTexture[MS_MAX_PATH];
    char        szAlphaTexture[MS_MAX_PATH];	// not used in SM.  Use alpha in diffuse texture instead
    int         nName;	// not used in SM.  What is this for anyway?

	struct Texture
	{
		Texture()
		{
			bSphereMapped = false;
			blendMode = BLEND_NORMAL;
		}

		void Load( CString sFile )
		{
			ani.Load( sFile );
			bSphereMapped = sFile.Find("sphere") != -1;
			if( sFile.Find("add") != -1 )
				blendMode = BLEND_ADD;
			else
				blendMode = BLEND_NORMAL;
		};
		AnimatedTexture ani;
		bool		bSphereMapped;	// true of "sphere" appears in the material name
		BlendMode	blendMode;
	} diffuse, alpha;
} msMaterial;

/* msPositionKey */
typedef struct msPositionKey
{
    float       fTime;
    RageVector3      Position;
} msPositionKey;

/* msRotationKey */
typedef struct msRotationKey
{
    float   fTime;
    RageVector3  Rotation;
} msRotationKey;

/* msBone */
typedef struct msBone
{
    int             nFlags;
    char            szName[MS_MAX_NAME];
    char            szParentName[MS_MAX_NAME];
    RageVector3          Position;
    RageVector3          Rotation;

    vector<msPositionKey>  PositionKeys;

    int             nNumRotationKeys;
    int             nNumAllocedRotationKeys;
    vector<msRotationKey>  RotationKeys;
} msBone;

/* msModel */
//typedef struct msModel
//{
//    vector<msMesh>     Meshes;
//
//    vector<msMaterial> Materials;
//} msModel;

struct msAnimation
{
    vector<msBone>     Bones;

	int FindBoneByName( const char* szName )
	{
		for( unsigned i=0; i<Bones.size(); i++ )
			if( strcmp(Bones[i].szName, szName)==0 )
				return i;
		return -1;
	}

//  int         nFrame;	// not used in SM.  We keep track of this outside this structure.
    int         nTotalFrames;

    RageVector3      Position;
    RageVector3      Rotation;
};

typedef struct
{
	RageMatrix	mRelative;
	RageMatrix	mAbsolute;
	RageMatrix	mRelativeFinal;
	RageMatrix	mFinal;
} myBone_t;

#endif
