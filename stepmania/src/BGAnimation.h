#ifndef BGANIMATION_H
#define BGANIMATION_H
/*
-----------------------------------------------------------------------------
 Class: BGAnimation

 Desc: Particles that play in the background of ScreenGameplay

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "ActorFrame.h"


class BGAnimationLayer;


class BGAnimation : public ActorFrame
{
public:
	BGAnimation();
	virtual ~BGAnimation();

	void Unload();

	void LoadFromStaticGraphic( CString sPath );
	void LoadFromAniDir( CString sAniDir, CString sSongBGPath="" );
	void LoadFromMovie( CString sMoviePath, bool bLoop, bool bRewind );
	void LoadFromVisualization( CString sMoviePath, CString sSongBGPath );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void SetDiffuse( const RageColor &c );

	void GainingFocus();
	void LosingFocus();

	float GetLengthSeconds() { return m_fLengthSeconds; }

protected:
	vector<BGAnimationLayer*> m_Layers;
	float	m_fLengthSeconds;
};


#endif
