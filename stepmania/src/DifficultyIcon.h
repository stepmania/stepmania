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
#include "Notes.h"
#include "ThemeManager.h"


class DifficultyIcon : public Sprite
{
public:
	DifficultyIcon()
	{
		Load( THEME->GetPathTo("Graphics","select music difficulty icons") );
		StopAnimating();

		SetFromNotes( NULL );
	};

	void SetFromNotes( const Notes* pNotes )
	{
		if( pNotes != NULL )
		{
			SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
			switch( pNotes->m_DifficultyClass )
			{
			case CLASS_EASY:	SetState( 0 );	break;
			case CLASS_MEDIUM:	SetState( 1 );	break;
			case CLASS_HARD:	SetState( 2 );	break;
			}
		}
		else
		{
			SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
			SetState( 0 );
		}
	};

private:

};

