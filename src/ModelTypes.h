/* ModelTypes - Types defined in msLib.h.  C arrays converted to use std::vector */

#ifndef MODEL_TYPES_H
#define MODEL_TYPES_H

#include "RageTypes.h"
#include "RageMatrix.hpp"
#include "RageModelVertex.hpp"
#include "RageVector4.hpp"

struct msTriangle
{
	uint16_t nVertexIndices[3];
};


struct msMesh
{
	std::string			sName;
	char			nMaterialIndex;

	std::vector<Rage::ModelVertex>	Vertices;

	// OPTIMIZATION: If all verts in a mesh are transformed by the same bone,
	// then send the transform to the graphics card for the whole mesh instead
	// of transforming each vertex on the CPU;
	char			m_iBoneIndex;	// -1 = no bone

	std::vector<msTriangle>	Triangles;
};

class RageTexture;

// merge this into Sprite?
class AnimatedTexture
{
public:
	AnimatedTexture();
	~AnimatedTexture();

	void LoadBlank();
	void Load( const std::string &sTexOrIniFile );
	void Unload();
	void Update( float fDelta );

	RageTexture* GetCurrentTexture();

	int GetNumStates() const;
	void SetState( int iNewState );
	float GetAnimationLengthSeconds() const;
	void SetSecondsIntoAnimation( float fSeconds );
	float GetSecondsIntoAnimation() const;
	Rage::Vector2 GetTextureTranslate();

	bool		m_bSphereMapped;
	BlendMode	m_BlendMode;

	bool NeedsNormals() const { return m_bSphereMapped; }

private:
	Rage::Vector2		m_vTexOffset;
	Rage::Vector2		m_vTexVelocity;

	int m_iCurState;
	float m_fSecsIntoFrame;
	struct AnimatedTextureState
	{
		AnimatedTextureState(
			RageTexture* pTexture_,
			float		fDelaySecs_,
			Rage::Vector2	vTranslate_
				     ):
			pTexture(pTexture_), fDelaySecs(fDelaySecs_),
			vTranslate(vTranslate_) {}

		RageTexture* pTexture;
		float		fDelaySecs;
		Rage::Vector2	vTranslate;
	};
	std::vector<AnimatedTextureState> vFrames;
};

struct msMaterial
{
	int		nFlags;
	std::string		sName;
	Rage::Color	Ambient;
	Rage::Color	Diffuse;
	Rage::Color	Specular;
	Rage::Color	Emissive;
	float		fShininess;
	float		fTransparency;

	AnimatedTexture	diffuse;
	AnimatedTexture	alpha;

	bool NeedsNormals() const { return diffuse.NeedsNormals() || alpha.NeedsNormals() ; }
};

struct msPositionKey
{
	float fTime;
	Rage::Vector3 Position;
};

struct msRotationKey
{
	float fTime;
	Rage::Vector4 Rotation;
};

struct msBone
{
	int			nFlags;
	std::string			sName;
	std::string			sParentName;
	Rage::Vector3		Position;
	Rage::Vector3		Rotation;

	std::vector<msPositionKey>	PositionKeys;
	std::vector<msRotationKey>	RotationKeys;
};

struct msAnimation
{
	int FindBoneByName( const std::string &sName ) const
	{
		for( unsigned i=0; i<Bones.size(); i++ )
			if( Bones[i].sName == sName )
				return i;
		return -1;
	}

	bool LoadMilkshapeAsciiBones( std::string sAniName, std::string sPath );

	std::vector<msBone>		Bones;
	int			nTotalFrames;
};

struct myBone_t
{
	Rage::Matrix		m_Relative;
	Rage::Matrix		m_Absolute;
	Rage::Matrix		m_Final;
};

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
