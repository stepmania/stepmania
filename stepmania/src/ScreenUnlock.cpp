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
#define USE_UNLOCKS_DAT		THEME->GetMetricI("ScreenUnlock","UseUnlocksDat")

ScreenUnlock::ScreenUnlock() : ScreenAttract("ScreenUnlock")
{
	LOG->Trace("ScreenUnlock::ScreenUnlock()");

	unsigned NumUnlocks = NUM_UNLOCKS;
	if (GAMESTATE->m_pUnlockingSys->m_SongEntries.size() < NumUnlocks)
		NumUnlocks = GAMESTATE->m_pUnlockingSys->m_SongEntries.size();

	if (!PREFSMAN->m_bUseUnlockSystem || NumUnlocks == 0)
	{
		this->HandleScreenMessage( SM_GoToNextScreen );
		return;
	}

	PointsUntilNextUnlock.LoadFromFont( THEME->GetPathToF("Common normal") );
	PointsUntilNextUnlock.SetHorizAlign( Actor::align_left );

	unsigned i;
	CString IconCommand = ICON_COMMAND;
	for(i=1; i <= NumUnlocks; i++)
	{
		Sprite* entry = new Sprite;

		// new unlock graphic
		entry->Load( THEME->GetPathToG(ssprintf("ScreenUnlock %d icon", i)) );

		// set graphic location
		entry->SetName( ssprintf("Unlock%d",i) );
		SET_XY( *entry );

		// get pertaining songentry
		CString SongTitle = DISPLAYED_SONG(i);
		if (USE_UNLOCKS_DAT == 1)
			if ((unsigned)i <= GAMESTATE->m_pUnlockingSys->m_SongEntries.size() )
				SongTitle = GAMESTATE->m_pUnlockingSys->m_SongEntries[i-1].m_sSongName;
		LOG->Trace("UnlockScreen: Searching for %s", SongTitle.c_str());
		
		SongEntry *pSong = GAMESTATE->m_pUnlockingSys->FindLockEntry( SongTitle );

		if( pSong == NULL)
		{
			LOG->Trace("Can't find song %s", SongTitle.c_str());
			delete entry;   // oops, memory leak
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
		float ScrollingTextX = UNLOCK_TEXT_SCROLL_X;
		float ScrollingTextStartY = UNLOCK_TEXT_SCROLL_START_Y;
		float ScrollingTextEndY = UNLOCK_TEXT_SCROLL_END_Y;
		float ScrollingTextZoom = UNLOCK_TEXT_SCROLL_ZOOM;
		float ScrollingTextRows = UNLOCK_TEXT_SCROLL_ROWS;
		float MaxWidth = UNLOCK_TEXT_SCROLL_MAX_WIDTH;

		float SecondsToScroll = TIME_TO_DISPLAY;
		
		if (SecondsToScroll > 2) SecondsToScroll--;

		float SECS_PER_CYCLE = 0;

		if (UNLOCK_TEXT_SCROLL != 3)
			SECS_PER_CYCLE = (float)SecondsToScroll/(ScrollingTextRows + NumUnlocks);
		else
			SECS_PER_CYCLE = (float)SecondsToScroll/(ScrollingTextRows * 3 + NumUnlocks + 4);

		for(i = 1; i <= NumUnlocks; i++)
		{
			BitmapText* text = new BitmapText;

			text->LoadFromFont( THEME->GetPathToF("ScreenUnlock text") );
			text->SetHorizAlign( Actor::align_left );

			CString DisplayedSong = DISPLAYED_SONG(i);
			if (USE_UNLOCKS_DAT == 1)
				if ((unsigned)i <= GAMESTATE->m_pUnlockingSys->m_SongEntries.size() )
					DisplayedSong = GAMESTATE->m_pUnlockingSys->m_SongEntries[i-1].m_sSongName;
			
			DisplayedSong.MakeUpper();
			SongEntry *pSong = GAMESTATE->m_pUnlockingSys->FindLockEntry(DisplayedSong);
			if ( pSong == NULL )  // no such song
				continue;

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
			else		 // song is missing, might be a course
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

			if (pSong != NULL && pSong->m_pSong != NULL)
			{
				if( pSong->isLocked) // song is locked
				{
					text->SetText("???");
					text->SetZoomX(1);
				} else {             // song is unlocked, change color
					RageColor color = SONGMAN->GetGroupColor(pSong->m_pSong->m_sGroupName);
					text->SetGlobalDiffuseColor(color);
				}
			}

			text->SetXY(ScrollingTextX, ScrollingTextStartY);

			if (UNLOCK_TEXT_SCROLL == 3 && UNLOCK_TEXT_SCROLL_ROWS + i > NumUnlocks)
			{   // special command for last unlocks when extreme-style scrolling is in effect
				float TargetRow = -0.5f + i + UNLOCK_TEXT_SCROLL_ROWS - NumUnlocks;
				float StopOffPoint = ScrollingTextEndY - TargetRow / UNLOCK_TEXT_SCROLL_ROWS * (ScrollingTextEndY - ScrollingTextStartY);
				float FirstCycleTime = (UNLOCK_TEXT_SCROLL_ROWS - TargetRow) * SECS_PER_CYCLE;
				float SecondCycleTime = (6 + TargetRow) * SECS_PER_CYCLE - FirstCycleTime;
				LOG->Trace("Target Row: %f", TargetRow);
				LOG->Trace("command for icon %d: %s", i, ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;linear,%f;y,%f;linear,0.1;diffusealpha,0", SECS_PER_CYCLE * (i - 1), FirstCycleTime, StopOffPoint, SecondCycleTime * 2, ScrollingTextEndY).c_str() );
				text->Command( ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;linear,%f;y,%f;linear,0.1;diffusealpha,0", SECS_PER_CYCLE * (i - 1), FirstCycleTime, StopOffPoint, SecondCycleTime, ScrollingTextEndY) );
			}
			else
				text->Command( ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;linear,0.1;diffusealpha,0", SECS_PER_CYCLE * (i - 1), SECS_PER_CYCLE * (ScrollingTextRows), ScrollingTextEndY) );

			item.push_back(text);

			if (UNLOCK_TEXT_SCROLL >= 2)
			{
				Sprite* IconCount = new Sprite;

				// new unlock graphic
				IconCount->Load( THEME->GetPathToG(ssprintf("ScreenUnlock %d icon", i)) );

				// set graphic location
				IconCount->SetXY( UNLOCK_TEXT_SCROLL_ICON_X, ScrollingTextStartY);

				IconCount->SetHeight(UNLOCK_TEXT_SCROLL_ICON_SIZE);
				IconCount->SetWidth(UNLOCK_TEXT_SCROLL_ICON_SIZE);

				if (UNLOCK_TEXT_SCROLL == 3 && UNLOCK_TEXT_SCROLL_ROWS + i > NumUnlocks)
				{
					float TargetRow = -0.5f + i + UNLOCK_TEXT_SCROLL_ROWS - NumUnlocks;
					float StopOffPoint = ScrollingTextEndY - TargetRow / UNLOCK_TEXT_SCROLL_ROWS * (ScrollingTextEndY - ScrollingTextStartY);
					float FirstCycleTime = (UNLOCK_TEXT_SCROLL_ROWS - TargetRow) * SECS_PER_CYCLE;
					float SecondCycleTime = (6 + TargetRow) * SECS_PER_CYCLE - FirstCycleTime;
					LOG->Trace("Target Row: %f", TargetRow);
					LOG->Trace("command for icon %d: %s", i, ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;linear,%f;y,%f;linear,0.1;diffusealpha,0", SECS_PER_CYCLE * (i - 1), FirstCycleTime, StopOffPoint, SecondCycleTime * 2, ScrollingTextEndY).c_str() );
					IconCount->Command( ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;linear,%f;y,%f;linear,0.1;diffusealpha,0", SECS_PER_CYCLE * (i - 1), FirstCycleTime, StopOffPoint, SecondCycleTime, ScrollingTextEndY) );
				}
				else
					IconCount->Command( ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;linear,0.1;diffusealpha,0", SECS_PER_CYCLE * (i - 1), SECS_PER_CYCLE * (ScrollingTextRows), ScrollingTextEndY) );

				ItemIcons.push_back(IconCount);

				LOG->Trace("Added unlock text %d", i);
					
				if (UNLOCK_TEXT_SCROLL == 3)
				{
					if (!pSong->isLocked)
						LastUnlocks.push_back(i);
				}
			}
		}
	}

	if (UNLOCK_TEXT_SCROLL == 3)
	{
		float ScrollingTextX = UNLOCK_TEXT_SCROLL_X;
		float ScrollingTextStartY = UNLOCK_TEXT_SCROLL_START_Y;
		float ScrollingTextEndY = UNLOCK_TEXT_SCROLL_END_Y;
		float ScrollingTextRows = UNLOCK_TEXT_SCROLL_ROWS;
		float MaxWidth = UNLOCK_TEXT_SCROLL_MAX_WIDTH;
		float SecondsToScroll = TIME_TO_DISPLAY - 1;
		float SECS_PER_CYCLE = (float)SecondsToScroll/(ScrollingTextRows * 3 + NumUnlocks + 4);

		for(i=1; i <= UNLOCK_TEXT_SCROLL_ROWS; i++)
		{
			if (i > LastUnlocks.size())
				continue;

			unsigned NextIcon = LastUnlocks[LastUnlocks.size() - i];

			Sprite* NewIcon = new Sprite;
			BitmapText* NewText = new BitmapText;

			NewText->LoadFromFont( THEME->GetPathToF("ScreenUnlock text") );
			NewText->SetHorizAlign( Actor::align_left );

			CString DisplayedSong = DISPLAYED_SONG(NextIcon);
			if (USE_UNLOCKS_DAT == 1)
			{
				if (NextIcon <= GAMESTATE->m_pUnlockingSys->m_SongEntries.size() )
					DisplayedSong = GAMESTATE->m_pUnlockingSys->m_SongEntries[NextIcon-1].m_sSongName;
			}

			DisplayedSong.MakeUpper();
			SongEntry *pSong = GAMESTATE->m_pUnlockingSys->FindLockEntry(DisplayedSong);

			/* Reset zoom before using SetTextMaxWidth. */
			NewText->SetZoom(UNLOCK_TEXT_SCROLL_ZOOM);

			CString title;
			CString subtitle;

			if (pSong->m_pSong == NULL)
				continue;
			else
			{
				title = pSong->m_pSong->GetDisplayMainTitle();
				subtitle = pSong->m_pSong->GetDisplaySubTitle();
			}

			if( subtitle != "" )
				title = title + "\n" + subtitle;
			NewText->SetTextMaxWidth( MaxWidth, title );

			RageColor color = SONGMAN->GetGroupColor(pSong->m_pSong->m_sGroupName);
			NewText->SetGlobalDiffuseColor(color);

			NewText->SetXY(ScrollingTextX, ScrollingTextStartY);
			NewText->Command( ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;", SECS_PER_CYCLE * (NumUnlocks + 2 * i - 2), SECS_PER_CYCLE * ((ScrollingTextRows - i) * 2 + 1 ), (ScrollingTextStartY + (ScrollingTextEndY - ScrollingTextStartY) * (ScrollingTextRows - i + 0.5) / ScrollingTextRows )) );

			// new unlock graphic
			NewIcon->Load( THEME->GetPathToG(ssprintf("ScreenUnlock %d icon", NextIcon)) );

			// set graphic location
			NewIcon->SetXY( UNLOCK_TEXT_SCROLL_ICON_X, ScrollingTextStartY);

			NewIcon->SetHeight(UNLOCK_TEXT_SCROLL_ICON_SIZE);
			NewIcon->SetWidth(UNLOCK_TEXT_SCROLL_ICON_SIZE);

			NewIcon->Command( ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;", SECS_PER_CYCLE * (NumUnlocks + 2 * i - 2), SECS_PER_CYCLE * ((ScrollingTextRows - i) * 2 + 1 ), (ScrollingTextStartY + (ScrollingTextEndY - ScrollingTextStartY) * (ScrollingTextRows - i + 0.5) / ScrollingTextRows )) );

			ItemIcons.push_back(NewIcon);
			item.push_back(NewText);
		}
	}

	// NOTE: the following two loops require the iterator to 
	// be ints because if you decrement an unsigned when it
	// equals zero, you get the maximum value of an unsigned,
	// which is still greater than 0.  By typecasting it as
	// an integer, you can achieve -1, which exits the loop.

	for(i = item.size() - 1; (int)i >= 0; i--)
		this->AddChild(item[i]);

	for(i = ItemIcons.size() - 1; (int)i >= 0; i--)
		this->AddChild(ItemIcons[i]);

	PointsUntilNextUnlock.SetName( "PointsDisplay" );
	
	CString PointDisplay = TYPE_TO_DISPLAY;
	if (PointDisplay == "DP" || PointDisplay == "Dance")
	{
		CString sDP = ssprintf( "%d", (int)GAMESTATE->m_pUnlockingSys->DancePointsUntilNextUnlock() );
		PointsUntilNextUnlock.SetText( sDP );
	} else if (PointDisplay == "AP" || PointDisplay == "Arcade") {
		CString sAP = ssprintf( "%d", (int)GAMESTATE->m_pUnlockingSys->ArcadePointsUntilNextUnlock() );
		PointsUntilNextUnlock.SetText( sAP );
	} else if (PointDisplay == "SP" || PointDisplay == "Song") {
		CString sSP = ssprintf( "%d", (int)GAMESTATE->m_pUnlockingSys->SongPointsUntilNextUnlock() );
		PointsUntilNextUnlock.SetText( sSP );
	}

	PointsUntilNextUnlock.SetZoom( POINTS_ZOOM );
	SET_XY( PointsUntilNextUnlock );
	this->AddChild( &PointsUntilNextUnlock );

	this->ClearMessageQueue( SM_BeginFadingOut );	// ignore ScreenAttract's SecsToShow

	this->PostScreenMessage( SM_BeginFadingOut, TIME_TO_DISPLAY );
}

ScreenUnlock::~ScreenUnlock()
{
	while (Unlocks.size() > 0)
	{
		Sprite* entry = Unlocks[Unlocks.size()-1];
		SAFE_DELETE(entry);
		Unlocks.pop_back();
	}
	while (item.size() > 0)
	{
		BitmapText* entry = item[item.size()-1];
		SAFE_DELETE(entry);
		item.pop_back();
	}
	while (ItemIcons.size() > 0)
	{
		Sprite* entry = ItemIcons[ItemIcons.size()-1];
		SAFE_DELETE(entry);
		ItemIcons.pop_back();
	}
}
