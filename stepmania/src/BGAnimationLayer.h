/* BGAnimationLayer - layer elements used by BGAnimation */

#ifndef BGANIMATIONLAYER_H
#define BGANIMATIONLAYER_H

#include "GameConstantsAndTypes.h"
#include "ActorFrame.h"
#include <map>

struct XNode;

class BGAnimationLayer : public ActorFrame
{
public:
	BGAnimationLayer( bool Generic );
	~BGAnimationLayer();
	void Init();
	void Unload();

	void LoadFromAniLayerFile( const CString& sPath );
	void LoadFromNode( const CString& sDir, const XNode* pNode );

	void UpdateInternal( float fDeltaTime );
	bool EarlyAbortDraw() const;

	float GetMaxTweenTimeLeft() const;

protected:
	vector<RageVector3> m_vParticleVelocity;

	enum Type
	{
		TYPE_SPRITE,
		TYPE_PARTICLES,
		TYPE_TILES,
		NUM_TYPES,
	} m_Type;



	//
	// loaded prefs
	//
	
	// common stuff
	bool m_bGeneric;

	CString m_sDrawCond;

	// stretch stuff
	float m_fTexCoordVelocityX;
	float m_fTexCoordVelocityY;

	// particles stuff
	bool  m_bParticlesBounce;

	// tiles stuff
	int m_iNumTilesWide;
	int m_iNumTilesHigh;
	float m_fTilesStartX;
	float m_fTilesStartY;
	float m_fTilesSpacingX;
	float m_fTilesSpacingY;
	float m_fTileVelocityX;
	float m_fTileVelocityY;
};

#endif

/*
 * (c) 2001-2004 Ben Nordstrom, Chris Danford, Glenn Maynard
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
