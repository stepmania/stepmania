#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: DifficultyMeter

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "DifficultyMeter.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "Notes.h"
#include "SongManager.h"


const int NUM_FEET_IN_METER					=	10;
const int GLOW_IF_METER_GREATER_THAN		=	9;


DifficultyMeter::DifficultyMeter()
{
	BitmapText::LoadFromTextureAndChars( THEME->GetPathTo("Graphics","DifficultyMeter bar 2x1"), "10" );

	SetFromNotes( NULL );
}

void DifficultyMeter::SetFromNotes( Notes* pNotes )
{
	if( pNotes != NULL )
	{
		SetDiffuse( RageColor(1,1,1,1) );
		SetMeter( pNotes->GetMeter() );
		if( pNotes->GetMeter() > GLOW_IF_METER_GREATER_THAN )
			this->SetEffectGlowShift();
		else
			this->SetEffectNone();

		SetDiffuse( SONGMAN->GetDifficultyColor(pNotes->GetDifficulty()) );
	}
	else
	{
		this->SetEffectNone();
		SetDiffuse( RageColor(0.8f,0.8f,0.8f,1) );
		SetMeter( 0 );
	}
}

void DifficultyMeter::SetMeter( int iMeter )
{
	CString sNewText;
	int f;
	for( f=0; f<NUM_FEET_IN_METER; f++ )
		sNewText += (f<iMeter) ? "1" : "0";
	for( f=NUM_FEET_IN_METER; f<=13; f++ )
		if( f<iMeter )
			sNewText += "1";

	SetText( sNewText );
}
