/*
-----------------------------------------------------------------------------
 File: DifficultyIcon.h

 Desc: A graphic displayed in the DifficultyIcon during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _DifficultyIcon_H_
#define _DifficultyIcon_H_


#include "Sprite.h"
#include "Song.h"
#include "BitmapText.h"
#include "ThemeManager.h"


class DifficultyIcon : public Sprite
{
public:
	DifficultyIcon()
	{
		Load( THEME->GetPathTo(GRAPHIC_STEPS_DESCRIPTION) );
		StopAnimating();

		SetFromDescription( "" );
	};

	void SetFromSteps( Steps &steps )
	{
		SetFromDescription( steps.m_sDescription );
	};

private:

	void SetFromDescription( CString sDescription )
	{
		sDescription.MakeLower();
		if(	sDescription.Find( "basic" ) != -1 )
			SetState( 0 );
		else if( sDescription.Find( "trick" ) != -1 )
			SetState( 1 );
		else if( sDescription.Find( "another" ) != -1  )
			SetState( 2 );
		else if( sDescription.Find( "maniac" ) != -1 )
			SetState( 3 );
		else if( sDescription.Find( "ssr" ) != -1 )
			SetState( 4 );
		else if( sDescription.Find( "battle" ) != -1 )
			SetState( 5 );
		else if( sDescription.Find( "couple" ) != -1 )
			SetState( 6 );
		else
			SetState( 7 );
	};
};

#endif