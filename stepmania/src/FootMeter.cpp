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
#include "ThemeManager.h"



FootMeter::FootMeter()
{
	Load( THEME->GetPathTo(FONT_METER) );

	SetNumFeet( 0, "" );
}

void FootMeter::SetFromNotes( Notes* pNotes )
{
	if( pNotes != NULL )
	{
		SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		SetNumFeet( pNotes->m_iMeter, pNotes->m_sDescription );
		if( pNotes->m_iMeter >= 10 )
			this->SetEffectGlowing();
		else
			this->SetEffectNone();

		this->StopTweening();
		this->SetZoom( 1.1f );
		this->BeginTweening( 0.3f, TWEEN_BOUNCE_BEGIN );
		this->SetTweenZoom( 1 );
	}
	else
	{
		this->SetEffectNone();
		SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
		SetNumFeet( 0, "" );
	}
}

void FootMeter::SetNumFeet( int iNumFeet, const CString &sDescription )
{
	CString sNewText;
	for( int f=0; f<=8; f++ )
		sNewText += (f<iNumFeet) ? "1" : "0";
	for( f=9; f<=12; f++ )
		if( f<iNumFeet )
			sNewText += "1";

	SetText( sNewText );

	CString sTemp = sDescription;
	sTemp.MakeLower();
	if(	sTemp.Find( "basic" ) != -1 )		SetDiffuseColor( D3DXCOLOR(1,1,0,1) );
	else if( sTemp.Find( "trick" ) != -1 )	SetDiffuseColor( D3DXCOLOR(1,0,0,1) );
	else if( sTemp.Find( "another" ) != -1 )SetDiffuseColor( D3DXCOLOR(1,0,0,1) );
	else if( sTemp.Find( "maniac" ) != -1 )	SetDiffuseColor( D3DXCOLOR(0,1,0,1) );
	else if( sTemp.Find( "ssr" ) != -1 )	SetDiffuseColor( D3DXCOLOR(0,1,0,1) );
	else if( sTemp.Find( "battle" ) != -1 )	SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	else if( sTemp.Find( "couple" ) != -1 )	SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	else									SetDiffuseColor( D3DXCOLOR(0.8f,0.8f,0.8f,1) );
}