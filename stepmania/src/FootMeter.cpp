#include "stdafx.h"
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



FootMeter::FootMeter()
{
	Load( THEME->GetPathTo(FONT_METER) );

	SetFromNotes( NULL );
}

void FootMeter::SetFromNotes( Notes* pNotes )
{
	if( pNotes != NULL )
	{
		SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		SetNumFeet( pNotes->m_iMeter );
		if( pNotes->m_iMeter >= 10 )
			this->SetEffectGlowing();
		else
			this->SetEffectNone();

		SetDiffuseColor( pNotes->GetColor() );
		this->StopTweening();
		this->SetZoom( 1.1f );
		this->BeginTweening( 0.3f, TWEEN_BOUNCE_BEGIN );
		this->SetTweenZoom( 1 );
	}
	else
	{
		this->SetEffectNone();
		SetDiffuseColor( D3DXCOLOR(0.8f,0.8f,0.8f,1) );
		SetNumFeet( 0 );
	}
}

void FootMeter::SetNumFeet( int iNumFeet )
{
	CString sNewText;
	for( int f=0; f<=8; f++ )
		sNewText += (f<iNumFeet) ? "1" : "0";
	for( f=9; f<=12; f++ )
		if( f<iNumFeet )
			sNewText += "1";

	SetText( sNewText );

}