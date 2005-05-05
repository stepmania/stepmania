#include "global.h"
#include "WheelItemBase.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"	// for sending SM_PlayMusicSample
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "song.h"
#include "Course.h"
#include "ProfileManager.h"
#include "ActorUtil.h"
#include "ThemeMetric.h"

WheelItemBaseData::WheelItemBaseData( WheelItemType wit, CString sText, RageColor color )
{
	m_Type = wit;
	m_sText = sText;
	m_color = color;
	m_Flags = WheelNotifyIcon::Flags();
}


WheelItemBase::WheelItemBase(CString sType)
{
	SetName( sType );
	m_pBar = NULL;
	Load(sType);
}

void WheelItemBase::Load( CString sType )
{
	TEXT_X			.Load(sType,"TextX");
	TEXT_Y			.Load(sType,"TextY");
	TEXT_ON_COMMAND	.Load(sType,"TextOnCommand");

	m_fPercentGray = 0;

	m_sprBar.Load( THEME->GetPathG(sType,"bar") );
	m_sprBar.SetXY( 0, 0 );
	this->AddChild( &m_sprBar );
	m_pBar = &m_sprBar;

	m_text.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_text.SetShadowLength( 0 );
	m_text.SetXY( TEXT_X, TEXT_Y );
	m_text.RunCommands( TEXT_ON_COMMAND );
	this->AddChild( &m_text );
}

void WheelItemBase::LoadFromWheelItemBaseData( WheelItemBaseData* pWID )
{
	ASSERT( pWID != NULL );
	
	data = pWID;

	m_text.SetText(pWID->m_sText);
	m_Type = pWID->m_Type;
	m_color	= pWID->m_color;

	// init type specific stuff
	switch( pWID->m_Type )
	{
	case TYPE_GENERIC:
		m_text.SetText( data->m_sText );
		m_text.SetDiffuse( data->m_color );
		break;
	default:
		ASSERT( 0 );	// invalid type
	}
}

void WheelItemBase::DrawGrayBar( Actor& bar )
{
	if( m_fPercentGray == 0 )
		return;

	bar.SetGlow( RageColor(0,0,0,m_fPercentGray) );
	bar.SetDiffuse( RageColor(0,0,0,0) );
	bar.Draw();
	bar.SetDiffuse( RageColor(0,0,0,1) );
	bar.SetGlow( RageColor(0,0,0,0) );
}

void WheelItemBase::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();

	if( m_pBar != NULL )
		DrawGrayBar( *m_pBar );
}
  
/*
 * (c) 2001-2004 Chris Danford, Chris Gomez, Glenn Maynard, Josh Allen
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
