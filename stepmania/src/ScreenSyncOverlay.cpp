#include "global.h"
#include "ScreenSyncOverlay.h"
#include "ScreenDimensions.h"
#include "ScreenManager.h"
#include "GameState.h"
#include "song.h"
#include "PrefsManager.h"
#include "GamePreferences.h"
#include "InputEventPlus.h"
#include "LocalizedString.h"
#include "AdjustSync.h"

static bool IsGameplay()
{
	return SCREENMAN && SCREENMAN->GetTopScreen() && SCREENMAN->GetTopScreen()->GetScreenType() == gameplay;
}

REGISTER_SCREEN_CLASS( ScreenSyncOverlay );

static LocalizedString REVERT_SYNC_CHANGES	( "ScreenSyncOverlay", "Revert sync changes" );
static LocalizedString CURRENT_BPM		( "ScreenSyncOverlay", "Current BPM - smaller/larger" );
static LocalizedString SONG_OFFSET		( "ScreenSyncOverlay", "Song offset - notes earlier/later" );
static LocalizedString MACHINE_OFFSET		( "ScreenSyncOverlay", "Machine offset - notes earlier/later" );
static LocalizedString HOLD_ALT			( "ScreenSyncOverlay", "(hold Alt for smaller increment)" );

void ScreenSyncOverlay::Init()
{
	Screen::Init();
	
	m_quad.SetDiffuse( RageColor(0,0,0,0) );
	m_quad.SetHorizAlign( align_left );
	m_quad.SetXY( SCREEN_CENTER_X+10, SCREEN_TOP+100 );
	this->AddChild( &m_quad );

	m_textHelp.LoadFromFont( THEME->GetPathF("Common", "normal") );
	m_textHelp.SetHorizAlign( align_left );
	m_textHelp.SetXY( SCREEN_CENTER_X+20, SCREEN_TOP+100 );
	m_textHelp.SetDiffuseAlpha( 0 );
	m_textHelp.SetZoom( 0.6f );
	m_textHelp.SetShadowLength( 2 );
	m_textHelp.SetText( 
		REVERT_SYNC_CHANGES.GetValue()+":\n"
		"    F4\n" +
		CURRENT_BPM.GetValue()+":\n"
		"    F9/F10\n" +
		CURRENT_BPM.GetValue()+":\n"
		"    F11/F12\n" +
		MACHINE_OFFSET.GetValue()+":\n"
		"    Shift + F11/F12\n" +
		HOLD_ALT.GetValue() );
	this->AddChild( &m_textHelp );
	
	m_quad.ZoomToWidth( m_textHelp.GetZoomedWidth()+20 ); 
	m_quad.ZoomToHeight( m_textHelp.GetZoomedHeight()+20 ); 

	m_textStatus.LoadFromFont( THEME->GetPathF("Common", "normal") );
	m_textStatus.SetHorizAlign( align_center );
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
		HideHelp();
		return;
	}

	Screen::Update(fDeltaTime);

	// TODO: Only update when changed.
	UpdateText();
}

static Preference<bool>		g_bShowAutoPlayStatus( "ShowAutoPlayStatus", true );

static LocalizedString AUTO_PLAY		( "ScreenSyncOverlay", "AutoPlay" );
static LocalizedString AUTO_PLAY_CPU		( "ScreenSyncOverlay", "AutoPlayCPU" );
static LocalizedString AUTO_SYNC_SONG		( "ScreenSyncOverlay", "AutoSync Song" );
static LocalizedString AUTO_SYNC_MACHINE	( "ScreenSyncOverlay", "AutoSync Machine" );
static LocalizedString AUTO_SYNC_TEMPO		( "ScreenSyncOverlay", "AutoSync Tempo" );
void ScreenSyncOverlay::UpdateText()
{
	vector<RString> vs;

	if( g_bShowAutoPlayStatus )
	{
		switch( GamePreferences::m_AutoPlay.Get() )
		{
		case PC_HUMAN:						break;
		case PC_AUTOPLAY:	vs.push_back(AUTO_PLAY);	break;
		case PC_CPU:		vs.push_back(AUTO_PLAY_CPU);	break;
		default:	ASSERT(0);
		}
	}

	switch( GAMESTATE->m_SongOptions.GetCurrent().m_AutosyncType )
	{
	case SongOptions::AUTOSYNC_OFF:							break;
	case SongOptions::AUTOSYNC_SONG:	vs.push_back(AUTO_SYNC_SONG);		break;
	case SongOptions::AUTOSYNC_MACHINE:	vs.push_back(AUTO_SYNC_MACHINE);	break;
	case SongOptions::AUTOSYNC_TEMPO:	vs.push_back(AUTO_SYNC_TEMPO);		break;
	default:	ASSERT(0);
	}

	if( GAMESTATE->m_pCurSong != NULL  &&  !GAMESTATE->IsCourseMode() )	// sync controls available
	{
		AdjustSync::GetSyncChangeTextGlobal( vs );
		AdjustSync::GetSyncChangeTextSong( vs );
	}	

	m_textStatus.SetText( join("\n",vs) );
}

static LocalizedString CANT_SYNC_WHILE_PLAYING_A_COURSE	("ScreenSyncOverlay","Can't sync while playing a course.");
static LocalizedString SYNC_CHANGES_REVERTED		("ScreenSyncOverlay","Sync changes reverted.");
bool ScreenSyncOverlay::OverlayInput( const InputEventPlus &input )
{
	if( !IsGameplay() )
		return false;

	if( input.DeviceI.device != DEVICE_KEYBOARD )
		return false;

	enum Action
	{
		RevertSyncChanges,
		ChangeSongBPM,
		ChangeGlobalOffset,
		ChangeSongOffset,
		Action_Invalid
	};
	Action a = Action_Invalid;

	bool bIncrease = true;
	switch( input.DeviceI.button )
	{
	case KEY_F4:	a = RevertSyncChanges; break;
	case KEY_F9:	bIncrease = false; /* fall through */
	case KEY_F10:	a = ChangeSongBPM; break;
	case KEY_F11:	bIncrease = false; /* fall through */
	case KEY_F12:
		if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT)) ||
		    INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT)) )
			a = ChangeGlobalOffset;
		else
			a = ChangeSongOffset;
		break;

	default:
		return false;
	}

	if( GAMESTATE->IsCourseMode() && a != ChangeGlobalOffset )
	{
		SCREENMAN->SystemMessage( CANT_SYNC_WHILE_PLAYING_A_COURSE );
		return true;
	}

	switch( a )
	{
	case RevertSyncChanges:
		switch( input.type )
		{
		case IET_FIRST_PRESS:	break;
		default:	return false;
		}
		SCREENMAN->SystemMessage( SYNC_CHANGES_REVERTED );
		AdjustSync::RevertSyncChanges();
		break;
	case ChangeSongBPM:
		{
			float fDelta = bIncrease? +0.02f:-0.02f;
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RALT)) ||
				INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LALT)) )
			{
				fDelta /= 20;
			}
			switch( input.type )
			{
			case IET_RELEASE:	fDelta *= 0;	break;
			case IET_REPEAT:
				if( INPUTFILTER->GetSecsHeld(input.DeviceI) < 1.0f )
					fDelta *= 0;
				else
					fDelta *= 10;
			}
			if( GAMESTATE->m_pCurSong != NULL )
			{
				BPMSegment& seg = GAMESTATE->m_pCurSong->GetBPMSegmentAtBeat( GAMESTATE->m_fSongBeat );
				seg.m_fBPS += fDelta;
			}
		}
		break;
	case ChangeGlobalOffset:
	case ChangeSongOffset:
		{
			float fDelta = bIncrease? +0.02f:-0.02f;
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RALT)) ||
				INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LALT)) )
			{
				fDelta /= 20; /* 1ms */
			}
			switch( input.type )
			{
			case IET_RELEASE:	fDelta *= 0;	break;
			case IET_REPEAT:
				if( INPUTFILTER->GetSecsHeld(input.DeviceI) < 1.0f )
					fDelta *= 0;
				else
					fDelta *= 10;
			}

			switch( a )
			{
			case ChangeGlobalOffset:
				PREFSMAN->m_fGlobalOffsetSeconds.Set( PREFSMAN->m_fGlobalOffsetSeconds + fDelta );
				break;

			case ChangeSongOffset:
				if( GAMESTATE->m_pCurSong != NULL )
					GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds += fDelta;
				break;
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
	m_quad.BeginTweening( 0.3f, TWEEN_LINEAR );
	m_quad.SetDiffuseAlpha( 0.5f );

	m_textHelp.StopTweening();
	m_textHelp.BeginTweening( 0.3f, TWEEN_LINEAR );
	m_textHelp.SetDiffuseAlpha( 1 );

	m_quad.Sleep( 4 );
	m_quad.BeginTweening( 0.3f, TWEEN_LINEAR );
	m_quad.SetDiffuseAlpha( 0 );

	m_textHelp.Sleep( 4 );
	m_textHelp.BeginTweening( 0.3f, TWEEN_LINEAR );
	m_textHelp.SetDiffuseAlpha( 0 );
}

void ScreenSyncOverlay::HideHelp()
{
	m_quad.FinishTweening();
	m_textHelp.FinishTweening();
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
