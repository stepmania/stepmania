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


// WheelItem stuff
#define ICON_X			THEME->GetMetricF("WheelItemBase","IconX")
#define SONG_NAME_X		THEME->GetMetricF("WheelItemBase","SongNameX")
#define SECTION_NAME_X	THEME->GetMetricF("WheelItemBase","SectionNameX")
#define SECTION_ZOOM	THEME->GetMetricF("WheelItemBase","SectionZoom")
#define ROULETTE_X		THEME->GetMetricF("WheelItemBase","RouletteX")
#define ROULETTE_ZOOM	THEME->GetMetricF("WheelItemBase","RouletteZoom")
#define GRADE_X( p )	THEME->GetMetricF("WheelItemBase",ssprintf("GradeP%dX",p+1))




WheelItemBaseData::WheelItemBaseData( WheelItemType wit, CString sText, RageColor color )
{
	m_Type = wit;
	m_sText = sText;
	m_color = color;
	m_Flags = WheelNotifyIcon::Flags();
}


WheelItemBase::WheelItemBase()
{
//	data = new WheelItemBaseData;
//	data->m_Type = TYPE_GENERIC;
	SetName( "WheelItemBase" );

	m_fPercentGray = 0;

	m_sprBar.Load( THEME->GetPathG("WheelItemBase","bar") );
	m_sprBar.SetXY( 0, 0 );
	m_All.AddChild( &m_sprBar );

	m_text.LoadFromFont( THEME->GetPathF("WheelItemBase","text") );
	m_text.SetShadowLength( 0 );
	m_text.SetVertAlign( align_middle );
	m_text.SetXY( SECTION_NAME_X, 0 );
	m_text.SetZoom( SECTION_ZOOM );
	m_All.AddChild( &m_text );
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
		{
			m_text.SetText( data->m_sText );
			m_text.SetDiffuse( data->m_color );
		}
		break;
	default:
		ASSERT( 0 );	// invalid type
	}
}

void WheelItemBase::Update( float fDeltaTime )
{
	Actor::Update( fDeltaTime );
	
	switch( data->m_Type )
	{
	case TYPE_GENERIC:
		m_sprBar.Update( fDeltaTime );
		m_text.Update( fDeltaTime );
		break;
	default:
		break;//ASSERT(0);
	}
}

void WheelItemBase::DrawPrimitives()
{
	Sprite *bar = NULL;

	switch( data->m_Type )
	{
	case TYPE_GENERIC: 
		bar = &m_sprBar; 
		break;
	default: ASSERT(0);
	}

	bar = &m_sprBar;
	bar->Draw();
	m_text.Draw();
	switch( data->m_Type )
	{
	case TYPE_GENERIC:
		m_text.Draw();
		break;
	default:
		ASSERT(0);
	}

	if( m_fPercentGray > 0 )
	{
		bar->SetGlow( RageColor(0,0,0,m_fPercentGray) );
		bar->SetDiffuse( RageColor(0,0,0,0) );
		bar->Draw();
		bar->SetDiffuse( RageColor(0,0,0,1) );
		bar->SetGlow( RageColor(0,0,0,0) );
	}
}


void WheelItemBase::SetZTestMode( ZTestMode mode )
{
	ActorFrame::SetZTestMode( mode );

	// set all sub-Actors
	m_All.SetZTestMode( mode );
}

void WheelItemBase::SetZWrite( bool b )
{
	ActorFrame::SetZWrite( b );

	// set all sub-Actors
	m_All.SetZWrite( b );
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
