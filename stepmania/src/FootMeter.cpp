#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: FootMeter

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "FootMeter.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "Notes.h"
#include "SongManager.h"


const int NUM_FEET_IN_METER					=	10;
const int GLOW_IF_METER_GREATER_THAN		=	9;


FootMeter::FootMeter()
{
	BitmapText::LoadFromTextureAndChars( THEME->GetPathTo("Graphics","select music meter 2x1"), "10" );

	SetFromNotes( NULL );
}

void FootMeter::SetFromNotes( Notes* pNotes )
{
	if( pNotes != NULL )
	{
		SetDiffuse( RageColor(1,1,1,1) );
		SetNumFeet( pNotes->GetMeter() );
		if( pNotes->GetMeter() > GLOW_IF_METER_GREATER_THAN )
			this->SetEffectGlowCamelion();
		else
			this->SetEffectNone();

		SetDiffuse( SONGMAN->GetDifficultyColor(pNotes->GetDifficulty()) );
	}
	else
	{
		this->SetEffectNone();
		SetDiffuse( RageColor(0.8f,0.8f,0.8f,1) );
		SetNumFeet( 0 );
	}
}

void FootMeter::SetNumFeet( int iNumFeet )
{
	CString sNewText;
	int f;
	for( f=0; f<NUM_FEET_IN_METER; f++ )
		sNewText += (f<iNumFeet) ? "1" : "0";
	for( f=NUM_FEET_IN_METER; f<=13; f++ )
		if( f<iNumFeet )
			sNewText += "1";

	SetText( sNewText );
}
