/*
-----------------------------------------------------------------------------
 File: Background.h

 Desc: Background behind arrows while dancing

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _Background_H_
#define _Background_H_


#include "Sprite.h"
#include "ActorFrame.h"
#include "Song.h"

#include "ParticleSystem.h"



class Background : public ActorFrame
{
public:

	Background();
	virtual bool LoadFromSong( Song *pSong, bool bDisableVisualizations = false );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void SetDiffuseColor( D3DXCOLOR c )
	{
		m_sprVisualizationOverlay.SetDiffuseColor( c );
		m_sprSongBackground.SetDiffuseColor( c );
		m_sprDanger.SetDiffuseColor( c );
		m_sprDangerBackground.SetDiffuseColor( c );
	};
	
	virtual void TurnDangerOn()		{ m_bShowDanger = true; };
	virtual void TurnDangerOff()	{ m_bShowDanger = false; };

	virtual bool IsDangerOn()		{ return m_bShowDanger; };

	virtual void nextEffect() {};

protected:

	virtual void LoadParticleSprites( CString path );


	enum ParticleEffect 
	{
		PE_DROPPING = 0,
		PE_SPIRAL_OUT,
		PE_NUM
	};

	//CArray<Sprite*,Sprite*> m_backgroundTiles;
	CArray<Sprite*,Sprite*> m_particleSprites;

	ParticleSystem* m_pPS;

	Sprite m_sprVisualizationOverlay;
	Sprite m_sprSongBackground;
	
	Sprite m_sprDanger;
	Sprite m_sprDangerBackground;

	bool m_bShowDanger;
};




#endif