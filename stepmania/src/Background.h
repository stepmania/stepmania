/*
-----------------------------------------------------------------------------
 File: Background.h

 Desc: Background behind arrows while dancing

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _Background_H_
#define _Background_H_


#include "RageUtil.h"
#include "Sprite.h"
#include "ActorFrame.h"
#include "Song.h"

#include "ParticleSystem.h"



class Background : public ActorFrame
{
public:

	Background();
	virtual ~Background();

	virtual bool LoadFromSong( Song *pSong, bool bDisableVisualizations = false );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void SetDiffuseColor( D3DXCOLOR c )
	{
		m_sprVisualizationOverlay.SetDiffuseColor( c );

		for( int i=0; i < m_backgroundSprites.GetSize(); i++ )
			m_backgroundSprites[i]->SetDiffuseColor(c);

		//m_sprSongBackground.SetDiffuseColor( c );

		m_sprDanger.SetDiffuseColor( c );
		m_sprDangerBackground.SetDiffuseColor( c );
	};
	
	virtual void TurnDangerOn()		{ m_bShowDanger = true; };
	virtual void TurnDangerOff()	{ m_bShowDanger = false; };

	virtual bool IsDangerOn()		{ return m_bShowDanger; };

	virtual void NextEffect();

protected:

	virtual void LoadParticleSprites( CString path );
	virtual void LoadBackgroundSprites( CString path );
	virtual void LoadParticleSystems();


	//CArray<Sprite*,Sprite*> m_backgroundTiles;
	CArray<Sprite*,Sprite*> m_particleSprites;
	Sprite * m_curParticleSprite;

	CArray<ParticleSystem*,ParticleSystem*> m_particleSystems;
	ParticleSystem* m_curPS;

	CArray<Sprite*,Sprite*> m_backgroundSprites;
	Sprite * m_curBackground;

	Sprite m_sprVisualizationOverlay;
	Sprite * m_songBackground;
	
	Sprite m_sprDanger;
	Sprite m_sprDangerBackground;

	bool m_bShowDanger;

	float m_totalTime;
};




#endif