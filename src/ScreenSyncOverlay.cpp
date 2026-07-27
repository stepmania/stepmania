#include "global.h"
#include "ScreenSyncOverlay.h"
#include "ScreenDimensions.h"
#include "ScreenManager.h"
#include "GameState.h"
#include "Song.h"
#include "PrefsManager.h"
#include "GamePreferences.h"
#include "InputEventPlus.h"
#include "LocalizedString.h"
#include "AdjustSync.h"
#include "ActorUtil.h"

#include <vector>


static bool IsGameplay()
{
	return SCREENMAN && SCREENMAN->GetTopScreen() && SCREENMAN->GetTopScreen()->GetScreenType() == gameplay;
}

REGISTER_SCREEN_CLASS( ScreenSyncOverlay );

void ScreenSyncOverlay::Init()
{
	Screen::Init();

	m_overlay.Load(THEME->GetPathB(m_sName, "overlay"));
	AddChild(m_overlay);

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

	UpdateText();
}

static LocalizedString AUTO_PLAY		( "ScreenSyncOverlay", "AutoPlay" );
static LocalizedString AUTO_PLAY_CPU		( "ScreenSyncOverlay", "AutoPlayCPU" );
static LocalizedString AUTO_SYNC_SONG		( "ScreenSyncOverlay", "AutoSync Song" );
static LocalizedString AUTO_SYNC_MACHINE	( "ScreenSyncOverlay", "AutoSync Machine" );
static LocalizedString AUTO_SYNC_TEMPO		( "ScreenSyncOverlay", "AutoSync Tempo" );
static LocalizedString OLD_OFFSET	( "ScreenSyncOverlay", "Old offset" );
static LocalizedString NEW_OFFSET	( "ScreenSyncOverlay", "New offset" );
static LocalizedString COLLECTING_SAMPLE( "ScreenSyncOverlay", "Collecting sample" );
static LocalizedString STANDARD_DEVIATION( "ScreenSyncOverlay", "Standard deviation" );
void ScreenSyncOverlay::UpdateText()
{
	// Update Status
	std::vector<RString> vs;

	PlayerController pc = GamePreferences::m_AutoPlay.Get();
	switch( pc )
	{
		case PC_HUMAN:						break;
		case PC_AUTOPLAY:	vs.push_back(AUTO_PLAY);	break;
		case PC_CPU:		vs.push_back(AUTO_PLAY_CPU);	break;
		default:
			FAIL_M(ssprintf("Invalid PlayerController: %i", pc));
	}

	AutosyncType type = GAMESTATE->m_SongOptions.GetCurrent().m_AutosyncType;
	switch( type )
	{
	case AutosyncType_Off:							break;
	case AutosyncType_Song:	vs.push_back(AUTO_SYNC_SONG);		break;
	case AutosyncType_Machine:	vs.push_back(AUTO_SYNC_MACHINE);	break;
	case AutosyncType_Tempo:	vs.push_back(AUTO_SYNC_TEMPO);		break;
	default:
		FAIL_M(ssprintf("Invalid autosync type: %i", type));
	}

	if( GAMESTATE->m_pCurSong != nullptr  &&  !GAMESTATE->IsCourseMode() )	// sync controls available
	{
		AdjustSync::GetSyncChangeTextGlobal( vs );
		AdjustSync::GetSyncChangeTextSong( vs );
	}

	Message set_status("SetStatus");
	set_status.SetParam("text", join("\n",vs));
	m_overlay->HandleMessage(set_status);


	// Update SyncInfo
	bool visible= GAMESTATE->m_SongOptions.GetCurrent().m_AutosyncType != AutosyncType_Off;
	Message set_adjustments("SetAdjustments");
	set_adjustments.SetParam("visible", visible);
	if(visible)
	{
		float fNew = PREFSMAN->m_fGlobalOffsetSeconds;
		float fOld = AdjustSync::s_fGlobalOffsetSecondsOriginal;
		float fStdDev = AdjustSync::s_fStandardDeviation;
		RString s;
		s += OLD_OFFSET.GetValue() + ssprintf( ": %0.3f\n", fOld );
		s += NEW_OFFSET.GetValue() + ssprintf( ": %0.3f\n", fNew );
		s += STANDARD_DEVIATION.GetValue() + ssprintf( ": %0.3f\n", fStdDev );
		s += COLLECTING_SAMPLE.GetValue() + ssprintf( ": %d / %d", AdjustSync::s_iAutosyncOffsetSample+1, AdjustSync::OFFSET_SAMPLE_COUNT );
		set_adjustments.SetParam("text", s);
	}
	else
	{
		set_adjustments.SetParam("text", RString(""));
	}
	m_overlay->HandleMessage(set_adjustments);
}

static LocalizedString CANT_SYNC_WHILE_PLAYING_A_COURSE	("ScreenSyncOverlay","Can't sync while playing a course.");
static LocalizedString SYNC_CHANGES_REVERTED		("ScreenSyncOverlay","Sync changes reverted.");
bool ScreenSyncOverlay::Input( const InputEventPlus &input )
{
	if( !IsGameplay() )
		return Screen::Input(input);

	if( input.DeviceI.device != DEVICE_KEYBOARD )
		return Screen::Input(input);

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
	case KEY_F4:
		a = RevertSyncChanges;
		break;

	case KEY_F9:
		bIncrease = false;
		[[fallthrough]];
	case KEY_F10:
		a = ChangeSongBPM;
		break;

	case KEY_F11:
		bIncrease = false;
		[[fallthrough]];
	case KEY_F12:
		if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT)) ||
		    INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT)) )
			a = ChangeGlobalOffset;
		else
			a = ChangeSongOffset;
		break;

	default:
		return Screen::Input(input);
	}

	if( GAMESTATE->IsCourseMode() && a != ChangeGlobalOffset )
	{
		SCREENMAN->SystemMessage( CANT_SYNC_WHILE_PLAYING_A_COURSE );
		return true;
	}

	// Release the lookup tables being used for the timing data because
	// changing the timing data invalidates them. -Kyz
	if(a != Action_Invalid)
	{
		FOREACH_EnabledPlayer(pn)
		{
			if(GAMESTATE->m_pCurSteps[pn])
			{
				GAMESTATE->m_pCurSteps[pn]->GetTimingData()->ReleaseLookup();
			}
		}
	}

	switch( a )
	{
	case RevertSyncChanges:
		if( input.type != IET_FIRST_PRESS )
			return false;
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
				{
					if( INPUTFILTER->GetSecsHeld(input.DeviceI) < 1.0f )
						fDelta *= 0;
					else
						fDelta *= 10;
					break;
				}
				default: break;
			}
			if( GAMESTATE->m_pCurSong != nullptr )
			{
				TimingData &sTiming = GAMESTATE->m_pCurSong->m_SongTiming;
				BPMSegment * seg = sTiming.GetBPMSegmentAtBeat( GAMESTATE->m_Position.m_fSongBeat );
				seg->SetBPS( seg->GetBPS() + fDelta );
				const std::vector<Steps*>& vpSteps = GAMESTATE->m_pCurSong->GetAllSteps();
				for (Steps *s : vpSteps)
				{
					TimingData &pTiming = s->m_Timing;
					// Empty means it inherits song timing,
					// which has already been updated.
					if( pTiming.empty() )
						continue;
					float second = sTiming.GetElapsedTimeFromBeat(GAMESTATE->m_Position.m_fSongBeat);
					seg = pTiming.GetBPMSegmentAtBeat(pTiming.GetBeatFromElapsedTime(second));
					seg->SetBPS( seg->GetBPS() + fDelta );
				}
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
				{
					if( INPUTFILTER->GetSecsHeld(input.DeviceI) < 1.0f )
						fDelta *= 0;
					else
						fDelta *= 10;
				}
				default: break;
			}

			switch( a )
			{
				case ChangeGlobalOffset:
				{
				PREFSMAN->m_fGlobalOffsetSeconds.Set( PREFSMAN->m_fGlobalOffsetSeconds + fDelta );
				break;
				}

				case ChangeSongOffset:
				{
					if( GAMESTATE->m_pCurSong != nullptr )
					{
						GAMESTATE->m_pCurSong->m_SongTiming.m_fBeat0OffsetInSeconds += fDelta;
						const std::vector<Steps*>& vpSteps = GAMESTATE->m_pCurSong->GetAllSteps();
						for (Steps *s : vpSteps)
						{
							// Empty means it inherits song timing,
							// which has already been updated.
							if( s->m_Timing.empty() )
								continue;
							s->m_Timing.m_fBeat0OffsetInSeconds += fDelta;
						}
					}
					break;
				}
				default: break;
			}
		}
		break;
	default:
		FAIL_M(ssprintf("Invalid sync action choice: %i", a));
	}

	ShowHelp();
	UpdateText();
	return true;
}

void ScreenSyncOverlay::ShowHelp()
{
	m_overlay->PlayCommand("Show");
}

void ScreenSyncOverlay::HideHelp()
{
	m_overlay->PlayCommand("Hide");
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
