/*
-----------------------------------------------------------------------------
 File: MusicStatusDisplay.h

 Desc: A graphic displayed in the MusicStatusDisplay during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

class MusicStatusDisplay;


#ifndef _MusicStatusDisplay_H_
#define _MusicStatusDisplay_H_


#include "Sprite.h"
#include "ThemeManager.h"




class MusicStatusDisplay : public Sprite
{
public:
	MusicStatusDisplay()
	{
		Load( THEME->GetPathTo(GRAPHIC_MUSIC_STATUS_ICONS) );
		StopAnimating();

		m_bIsNew = false;
		m_Rank = NO_CROWN;
		m_bIsBlinking = false;
		m_bDisplayNewIcon = true;

		SetDiffuseColor( D3DXCOLOR(0,0,0,0) );	// invisible
	}
	void SetNew( bool bIsNew )
	{
		m_bIsNew = bIsNew;
		SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	// visible
		SetState( 0 );
	};
	void SetBlinking( bool bIsBlinking )
	{
		m_bIsBlinking = bIsBlinking;
	};
	void SetRank( int i )
	{
		switch( i )
		{
		case 1:		m_Rank = CROWN_1;	SetState( 0 );	SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	break;
		case 2:		m_Rank = CROWN_2;	SetState( 0 );	SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	break;
		case 3:		m_Rank = CROWN_3;	SetState( 0 );	SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	break;
		default:	m_Rank = NO_CROWN;	SetDiffuseColor( D3DXCOLOR(0,0,0,0) );					break;
		}
	};
	virtual void Update( float fDeltaTime )
	{
		Sprite::Update( fDeltaTime );

		if( m_bIsBlinking )
		{
			if( (GetTickCount() % 1000) > 500 )		// show the new icon
			{
				if( m_bIsNew )
				{
					SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
					SetState( 0 );
				}
				else
				{
					SetDiffuseColor( D3DXCOLOR(0,0,0,0) );		// invisible
				}
			}
			else	// show the rank icon
			{
				SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
				switch( m_Rank )
				{
				case CROWN_1:	SetState(1);							break;
				case CROWN_2:	SetState(2);							break;
				case CROWN_3:	SetState(3);							break;
				case NO_CROWN:	SetDiffuseColor( D3DXCOLOR(0,0,0,0) );	break;
				}
			}
		}
	};


protected:

	bool m_bIsNew;
	enum Rank { NO_CROWN, CROWN_1, CROWN_2, CROWN_3 };
	Rank m_Rank;

	bool m_bIsBlinking;	// blink in order to display the new and crown icons
	bool m_bDisplayNewIcon;
};


#endif