/*
-----------------------------------------------------------------------------
 File: MusicStatusDisplay.h

 Desc: A graphic displayed in the MusicStatusDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

class MusicStatusDisplay;


#ifndef _MusicStatusDisplay_H_
#define _MusicStatusDisplay_H_


#include "Sprite.h"
#include "ThemeManager.h"


enum MusicStatusDisplayType { TYPE_NEW, TYPE_NONE, TYPE_CROWN1, TYPE_CROWN2, TYPE_CROWN3 };

class MusicStatusDisplay : public Sprite
{
public:
	MusicStatusDisplay()
	{
		Load( THEME->GetPathTo(GRAPHIC_MUSIC_STATUS_ICONS) );
		StopAnimating();

		SetType( TYPE_NONE );
	};

	void SetType( MusicStatusDisplayType msdt )
	{
		m_MusicStatusDisplayType = msdt;

		switch( m_MusicStatusDisplayType )
		{
		case TYPE_NEW:
			m_MusicStatusDisplayType = TYPE_NEW;	
			SetState( 0 );	
			SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
			break;
		case TYPE_CROWN1:
			m_MusicStatusDisplayType = TYPE_CROWN1;	
			SetState( 1 );	
			SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	
			break;
		case TYPE_CROWN2:
			m_MusicStatusDisplayType = TYPE_CROWN2;	
			SetState( 2 );	
			SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	
			break;
		case TYPE_CROWN3:
			m_MusicStatusDisplayType = TYPE_CROWN3;	
			SetState( 3 );	
			SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	
			break;
		case TYPE_NONE:
		default:
			m_MusicStatusDisplayType = TYPE_NONE;	
			SetDiffuseColor( D3DXCOLOR(0,0,0,0) );
			break;
		}
	};

	virtual void Update( float fDeltaTime )
	{
		Sprite::Update( fDeltaTime );

	};

	virtual void DrawPrimitives()
	{
		switch( m_MusicStatusDisplayType )
		{
		case TYPE_CROWN1:
		case TYPE_CROWN2:
		case TYPE_CROWN3:
			// blink
			if( (TIMER->GetTimeSinceStart() - (int)TIMER->GetTimeSinceStart()) > 0.5f )		// show the new icon
				return;
			break;
		case TYPE_NONE:
			return;
			break;
		}

		Sprite::DrawPrimitives();
	}

protected:

	enum MusicStatusDisplayType m_MusicStatusDisplayType;
};


#endif