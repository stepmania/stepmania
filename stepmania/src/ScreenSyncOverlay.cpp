#include "global.h"
#include "ScreenSyncOverlay.h"
#include "ScreenDimensions.h"
#include "GameState.h"
#include "song.h"
#include "PrefsManager.h"

static bool IsGameplay()
{
	return SCREENMAN && SCREENMAN->GetTopScreen() && SCREENMAN->GetTopScreen()->GetScreenType() == gameplay;
}

REGISTER_SCREEN_CLASS( ScreenSyncOverlay );
ScreenSyncOverlay::ScreenSyncOverlay( const CString &sName ) : Screen(sName)
{
}

ScreenSyncOverlay::~ScreenSyncOverlay()
{
}

void ScreenSyncOverlay::Init()
{
	Screen::Init();
	
	m_quad.SetDiffuse( RageColor(0,0,0,0) );
	m_quad.SetHorizAlign( Actor::align_left );
	m_quad.SetXY( SCREEN_CENTER_X+10, SCREEN_TOP+100 );
	this->AddChild( &m_quad );

	m_textHelp.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textHelp.SetHorizAlign( Actor::align_left );
	m_textHelp.SetXY( SCREEN_CENTER_X+20, SCREEN_TOP+100 );
	m_textHelp.SetDiffuseAlpha( 0 );
	m_textHelp.SetZoom( 0.6f );
	m_textHelp.SetShadowLength( 2 );
	m_textHelp.SetText( 
		"Revert sync changes:\n"
		"    F4\n"
		"Adjust current BPM:\n"
		"    F9/F10 (hold Alt for smaller)\n"
		"Adjust song offset:\n"
		"    F11/F12 (hold Alt for smaller)\n"
		"Adjust machine offset:\n"
		"    Shift + F11/F12 (hold Alt for smaller)" );
	this->AddChild( &m_textHelp );
	
	m_quad.ZoomToWidth( m_textHelp.GetZoomedWidth()+20 ); 
	m_quad.ZoomToHeight( m_textHelp.GetZoomedHeight()+20 ); 

	m_textStatus.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textStatus.SetHorizAlign( Actor::align_center );
	m_textStatus.SetXY( SCREEN_CENTER_X, SCREEN_CENTER_Y+150 );
	m_textStatus.SetZoom( 0.8f );
	m_textStatus.SetShadowLength( 2 );
	this->AddChild( &m_textStatus );
	
	Update( 0 );
}

void ScreenSyncOverlay::Update( float fDeltaTime )
{
	this->SetVisible( IsGameplay() );
	if( !IsGameplay() )
	{
		m_quad.SetDiffuseAlpha( 0 );
		m_textHelp.SetDiffuseAlpha( 0 );
		return;
	}

	Screen::Update(fDeltaTime);

	// TODO: Only update when changed.
	UpdateText();
}

void ScreenSyncOverlay::UpdateText()
{
	CString s;

	switch( PREFSMAN->m_AutoPlay )
	{
	case PC_HUMAN:							break;
	case PC_AUTOPLAY:	s+="AutoPlay\n";	break;
	case PC_CPU:		s+="AutoPlayCPU\n";	break;
	default:	ASSERT(0);
	}

	switch( GAMESTATE->m_SongOptions.m_AutosyncType )
	{
	case SongOptions::AUTOSYNC_OFF:								break;
	case SongOptions::AUTOSYNC_SONG:	s+="AutoSync Song\n";	break;
	case SongOptions::AUTOSYNC_MACHINE:	s+="AutoSync Machine\n";break;
	default:	ASSERT(0);
	}

	if( GAMESTATE->m_pCurSong == NULL )
	{
		m_textStatus.SetText( "" );
		return;
	}
	
	{
		float fOld = GAMESTATE->m_fGlobalOffsetSecondsOriginal;
		float fNew = PREFSMAN->m_fGlobalOffsetSeconds;
		float fDelta = fNew - fOld;

		if( fabs(fDelta) > 0.00001 )
		{
			s += ssprintf( 
				"Machine global offset: %.3fs (%+.3f)\n",
				fNew,
				fDelta );
		}
	}

	{
		float fOld = GAMESTATE->m_pTimingDataOriginal->m_fBeat0OffsetInSeconds;
		float fNew = GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds;
		float fDelta = fNew - fOld;

		if( fabs(fDelta) > 0.00001 )
		{
			s += ssprintf( 
				"Song offset: %.3fs (%+.3f)\n",
				fNew,
				fDelta );
		}
	}

	ASSERT( GAMESTATE->m_pTimingDataOriginal->m_BPMSegments.size() == GAMESTATE->m_pCurSong->m_Timing.m_BPMSegments.size() );

	for( int i=0; i<GAMESTATE->m_pCurSong->m_Timing.m_BPMSegments.size(); i++ )
	{
		float fOld = GAMESTATE->m_pTimingDataOriginal->m_BPMSegments[i].m_fBPS;
		float fNew = GAMESTATE->m_pCurSong->m_Timing.m_BPMSegments[i].m_fBPS;
		float fDelta = fNew - fOld;

		if( fabs(fDelta) > 0.00001 )
		{
			s += ssprintf( 
				"%s tempo segment: %.3f BPS (%+.3f)\n",
				FormatNumberAndSuffix(i+1).c_str(),
				fNew,
				fDelta );
		}
	}	
	

	m_textStatus.SetText( s );
}

bool ScreenSyncOverlay::OverlayInput( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( !IsGameplay() )
		return false;

	if( DeviceI.device != DEVICE_KEYBOARD )
		return false;

	switch( DeviceI.button )
	{
	case KEY_F4:
		SCREENMAN->SystemMessage( "Sync changes reverted." );
		break;
	case KEY_F9:
	case KEY_F10:
	case KEY_F11:
	case KEY_F12:
		if( GAMESTATE->IsCourseMode() )
		{
			SCREENMAN->SystemMessage( "Can't sync while playing a course." );
			return true;
		}
		break;
	default:
		return false;
	}

	switch( DeviceI.button )
	{
	case KEY_F4:
		GAMESTATE->RevertSyncChanges();
		break;
	case KEY_F9:
	case KEY_F10:
		{
			float fDelta;
			switch( DeviceI.button )
			{
			case KEY_F9:	fDelta = -0.02f;		break;
			case KEY_F10:	fDelta = +0.02f;		break;
			default:	ASSERT(0);
			}
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RALT)) ||
				INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LALT)) )
				fDelta /= 20;
			switch( type )
			{
			case IET_RELEASE:		fDelta *= 0;	break;
			case IET_SLOW_REPEAT:	fDelta *= 0;	break;
			case IET_FAST_REPEAT:	fDelta *= 10;	break;
			}
			if( GAMESTATE->m_pCurSong != NULL )
			{
				BPMSegment& seg = GAMESTATE->m_pCurSong->GetBPMSegmentAtBeat( GAMESTATE->m_fSongBeat );
				seg.m_fBPS += fDelta;
			}
		}
		break;
	case KEY_F11:
	case KEY_F12:
		{
			float fDelta;
			switch( DeviceI.button )
			{
			case KEY_F11:	fDelta = -0.02f;	break;
			case KEY_F12:	fDelta = +0.02f;	break;
			default:	ASSERT(0);
			}
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RALT)) ||
				INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LALT)) )
				fDelta /= 20; /* 1ms */
			switch( type )
			{
			case IET_RELEASE:		fDelta *= 0;	break;
			case IET_SLOW_REPEAT:	fDelta *= 0;	break;
			case IET_FAST_REPEAT:	fDelta *= 10;	break;
			}

			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT)) ||
				INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT)) )
			{
				PREFSMAN->m_fGlobalOffsetSeconds.Set( PREFSMAN->m_fGlobalOffsetSeconds + fDelta );
			}
			else
			{
				if( GAMESTATE->m_pCurSong != NULL )
					GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds += fDelta;
			}
		}
		break;
	default:
		ASSERT(0);
	}

	ShowHelp();
	UpdateText();
	return true;
}

void ScreenSyncOverlay::ShowHelp()
{
	m_quad.StopTweening();
	m_quad.BeginTweening( 0.3f, Actor::TWEEN_LINEAR );
	m_quad.SetDiffuseAlpha( 0.5f );

	m_textHelp.StopTweening();
	m_textHelp.BeginTweening( 0.3f, Actor::TWEEN_LINEAR );
	m_textHelp.SetDiffuseAlpha( 1 );

	if( !GAMESTATE->IsSyncDataChanged() )
	{
		m_quad.Sleep( 4 );
		m_quad.BeginTweening( 0.3f, Actor::TWEEN_LINEAR );
		m_quad.SetDiffuseAlpha( 0 );

		m_textHelp.Sleep( 4 );
		m_textHelp.BeginTweening( 0.3f, Actor::TWEEN_LINEAR );
		m_textHelp.SetDiffuseAlpha( 0 );
	}
}

/*
 * (c) 2001-2005 Chris Danford
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
