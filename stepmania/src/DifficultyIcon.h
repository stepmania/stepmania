#pragma once
/*
-----------------------------------------------------------------------------
 Class: DifficultyIcon

 Desc: A graphic displayed in Select Music above the Groove Radar.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "Song.h"
#include "BitmapText.h"
#include "ThemeManager.h"


class DifficultyIcon : public Sprite
{
public:
	DifficultyIcon()
	{
		Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_DIFFICULTY_BANNER_ICONS) );
		StopAnimating();

		SetFromNotes( NULL );
	};

	void SetFromNotes( Notes* pNotes )
	{
		if( pNotes != NULL )
		{
			SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
			SetFromDescription( pNotes->m_sDescription );
		}
		else
		{
			SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
			SetFromDescription( "" );
		}
	};

private:

	void SetFromDescription( const CString &sDescription )
	{
		CString sTemp = sDescription;
		sTemp.MakeLower();
		if(	sTemp.Find( "basic" ) != -1 )			SetState( 0 );
		else if( sTemp.Find( "trick" ) != -1 )		SetState( 1 );
		else if( sTemp.Find( "another" ) != -1  )	SetState( 1 );
		else if( sTemp.Find( "maniac" ) != -1 )		SetState( 2 );
		else if( sTemp.Find( "ssr" ) != -1 )		SetState( 2 );
		else if( sTemp.Find( "battle" ) != -1 )		SetState( 0 );
		else if( sTemp.Find( "couple" ) != -1 )		SetState( 0 );
		else										SetState( 0 );
	};
};

