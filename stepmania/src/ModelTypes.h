/* ModelTypes - Types defined in msLib.h.  C arrays converted to use std::vector */

#ifndef MODEL_TYPES_H
#define MODEL_TYPES_H

#define MS_MAX_NAME             32

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
class AnimatedTexture
{
public:
	AnimatedTexture();
	~AnimatedTexture();
	
	void Load( CString sTexOrIniFile );
	void Unload();
	void Update( float fDelta );
	
	RageTexture* GetCurrentTexture();
	
	int GetNumStates() const;
	void SetState( int iNewState );
	float GetAnimationLengthSeconds() const;
	void SetSecondsIntoAnimation( float fSeconds );
	float GetSecondsIntoAnimation() const;

	bool		m_bSphereMapped;
	float		m_fTexVelocityX;
	float		m_fTexVelocityY;
	BlendMode	m_BlendMode;

private:

	int m_iCurState;
	float m_fSecsIntoFrame;
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
    CString     sName;
    RageColor   Ambient;
    RageColor   Diffuse;
    RageColor   Specular;
    RageColor   Emissive;
    float       fShininess;
    float       fTransparency;
    int         nName;	// not used in SM.  What is this for anyway?

	AnimatedTexture diffuse;
	AnimatedTexture alpha;
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

/*
 * (c) 2003-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
