/*
-----------------------------------------------------------------------------
 Class: ScreenUnlock

 Desc: See header.

 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Andrew Wong
-----------------------------------------------------------------------------
*/

#include "global.h"
#include "PrefsManager.h"
#include "ScreenUnlock.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "RageLog.h"
#include "UnlockSystem.h"
#include "SongManager.h"
#include "ActorUtil.h"
#include "song.h"
#include "Course.h"

#define NUM_UNLOCKS					THEME->GetMetricI("ScreenUnlock", "NumUnlocks")
#define UNLOCK_TEXT_SCROLL_X		THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollX");
#define UNLOCK_TEXT_SCROLL_START_Y	THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollStartY")
#define UNLOCK_TEXT_SCROLL_END_Y	THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollEndY")
#define UNLOCK_TEXT_SCROLL_ZOOM		THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollZoom")
#define UNLOCK_TEXT_SCROLL_ROWS		THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollRows")
#define UNLOCK_TEXT_SCROLL_MAX_WIDTH THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollMaxWidth")
#define UNLOCK_TEXT_SCROLL_ICON_X		THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollIconX")
#define UNLOCK_TEXT_SCROLL_ICON_SIZE	THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollIconSize")
#define DISPLAYED_SONG(i)			THEME->GetMetric ("ScreenUnlock", ssprintf("Unlock%dSong", i))
#define UNLOCK_TEXT_SCROLL			THEME->GetMetricI("ScreenUnlock", "UnlockTextScroll")
#define TYPE_TO_DISPLAY				THEME->GetMetric("ScreenUnlock", "TypeOfPointsToDisplay")
#define ICON_COMMAND				THEME->GetMetric ("ScreenUnlock", "UnlockIconCommand")
#define TIME_TO_DISPLAY		THEME->GetMetricF("ScreenUnlock", "TimeToDisplay")
#define POINTS_ZOOM			THEME->GetMetricF("ScreenUnlock","PointsZoom")

ScreenUnlock::ScreenUnlock() : ScreenAttract("ScreenUnlock")
{
	LOG->Trace("ScreenUnlock::ScreenUnlock()");
	PointsUntilNextUnlock.LoadFromFont( THEME->GetPathToF("Common normal") );
	PointsUntilNextUnlock.SetHorizAlign( Actor::align_left );

	// get unlock data first
	CString sDP = ssprintf( "%d", (int)GAMESTATE->m_pUnlockingSys->DancePointsUntilNextUnlock() );
	CString sAP = ssprintf( "%d", (int)GAMESTATE->m_pUnlockingSys->ArcadePointsUntilNextUnlock() );
	CString sSP = ssprintf( "%d", (int)GAMESTATE->m_pUnlockingSys->SongPointsUntilNextUnlock() );
	
	CString IconCommand = ICON_COMMAND;

	for(int i=1; i <= NUM_UNLOCKS; i++)
	{
		Sprite* entry = new Sprite;

		// new unlock graphic
		entry->Load( THEME->GetPathToG(ssprintf("ScreenUnlock %d icon", i)) );

		// set graphic location
		entry->SetName( ssprintf("Unlock%d",i) );
		SET_XY( *entry );

		// get pertaining songentry
		LOG->Trace("UnlockScreen: Searching for %s", DISPLAYED_SONG(i).c_str());
		SongEntry *pSong = GAMESTATE->m_pUnlockingSys->FindLockEntry( DISPLAYED_SONG(i) );

		if( pSong == NULL)
		{
			LOG->Trace("Can't find song");
			continue;
		}

		entry->Command(IconCommand);

		Unlocks.push_back(entry);

		if ( !pSong->isLocked )
			this->AddChild(Unlocks[Unlocks.size() - 1]);
	}

	// scrolling text
	if (UNLOCK_TEXT_SCROLL != 0)
	{
		int NumberUnlocks = NUM_UNLOCKS;
		float ScrollingTextX = UNLOCK_TEXT_SCROLL_X;
		float ScrollingTextStartY = UNLOCK_TEXT_SCROLL_START_Y;
		float ScrollingTextEndY = UNLOCK_TEXT_SCROLL_END_Y;
		float ScrollingTextZoom = UNLOCK_TEXT_SCROLL_ZOOM;
		float ScrollingTextRows = UNLOCK_TEXT_SCROLL_ROWS;
		float MaxWidth = UNLOCK_TEXT_SCROLL_MAX_WIDTH;

		float SecondsToScroll = TIME_TO_DISPLAY;
		
		if (SecondsToScroll > 2) SecondsToScroll--;

		const float SECS_PER_CYCLE = (float)SecondsToScroll/(ScrollingTextRows + NumberUnlocks);

		for(int i = 1; i <= NumberUnlocks; i++)
		{
			BitmapText* text = new BitmapText;

			/* XXX: Don't load _shared directly.  Instead, create a redir in the
			 * Fonts directory, eg. "ScreenUnlock text.redir", that points to it. */
			text->LoadFromFont( THEME->GetPathToF("_shared2") );
			text->SetHorizAlign( Actor::align_left );

			CString DisplayedSong = DISPLAYED_SONG(i);
			DisplayedSong.MakeUpper();
			SongEntry *pSong = GAMESTATE->m_pUnlockingSys->FindLockEntry(DisplayedSong);

			/* Reset zoom before using SetTextMaxWidth. */
			text->SetZoom(ScrollingTextZoom);

			if (pSong && pSong->m_pSong != NULL)
			{
				CString title = pSong->m_pSong->GetDisplayMainTitle();
				CString subtitle = pSong->m_pSong->GetDisplaySubTitle();
				if( subtitle != "" )
					title = title + "\n" + subtitle;
				text->SetTextMaxWidth( MaxWidth, title );
			}
			else					 // song is missing
			{
				Course *crs = SONGMAN->FindCourse( DisplayedSong );
				if (crs != NULL)
				{
					text->SetTextMaxWidth( MaxWidth, crs->m_sName );
					text->Command("Diffuse,0,1,0,1");
				}
				else   // entry isn't a song or course
				{
					text->SetText( "" );
					text->Command("Diffuse,0.5,0,0,1");
				}
			}

			if (pSong != NULL)
			{
				if( pSong->isLocked)
				{
					text->SetText("???");
					// text->SetZoom(ScrollingTextZoom * 1.99); 
				} else {
					RageColor color = SONGMAN->GetGroupColor(pSong->m_pSong->m_sGroupName);
					text->SetGlobalDiffuseColor(color);
				}
			}

			text->SetXY(ScrollingTextX, ScrollingTextStartY);
			text->Command( ssprintf("diffusealpha,0;sleep,%f;linear,0.5;diffusealpha,1;linear,%f;y,%f;linear,0.5;diffusealpha,0", SECS_PER_CYCLE * (i - 1), SECS_PER_CYCLE * (ScrollingTextRows), ScrollingTextEndY) );

			item.push_back(text);
			this->AddChild(item[i-1]);

			if (UNLOCK_TEXT_SCROLL == 2)
			{
				Sprite* IconCount = new Sprite;

				// new unlock graphic
				IconCount->Load( THEME->GetPathToG(ssprintf("ScreenUnlock %d icon", i)) );

				// set graphic location
				IconCount->SetXY( UNLOCK_TEXT_SCROLL_ICON_X, ScrollingTextStartY);

				IconCount->SetHeight(UNLOCK_TEXT_SCROLL_ICON_SIZE);
				IconCount->SetWidth(UNLOCK_TEXT_SCROLL_ICON_SIZE);

				IconCount->Command( ssprintf("diffusealpha,0;sleep,%f;linear,0.5;diffusealpha,1;linear,%f;y,%f;linear,0.5;diffusealpha,0", SECS_PER_CYCLE * (i - 1), SECS_PER_CYCLE * (ScrollingTextRows), ScrollingTextEndY) );

				ItemIcons.push_back(IconCount);

				this->AddChild(ItemIcons[i-1]);
				LOG->Trace("Added unlock text %d", i);
					
			}
		}
	}


	PointsUntilNextUnlock.SetName( "PointsDisplay" );
	
	CString PointDisplay = TYPE_TO_DISPLAY;
	if (PointDisplay == "DP" || PointDisplay == "Dance")
		PointsUntilNextUnlock.SetText( sDP );

	if (PointDisplay == "AP" || PointDisplay == "Arcade")
		PointsUntilNextUnlock.SetText( sAP );

	if (PointDisplay == "SP" || PointDisplay == "Song")
		PointsUntilNextUnlock.SetText( sSP );

	PointsUntilNextUnlock.SetZoom( POINTS_ZOOM );
	SET_XY( PointsUntilNextUnlock );
	this->AddChild( &PointsUntilNextUnlock );

	this->ClearMessageQueue( SM_BeginFadingOut );	// ignore ScreenAttract's SecsToShow

	this->PostScreenMessage( SM_BeginFadingOut, TIME_TO_DISPLAY );
}

