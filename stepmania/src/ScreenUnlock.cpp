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
#include "Song.h"

ScreenUnlock::ScreenUnlock() : ScreenAttract("ScreenUnlock")
{
	LOG->Trace("ScreenUnlock::ScreenUnlock()");
	PointsUntilNextUnlock.LoadFromFont( THEME->GetPathToF("Common normal") );
	PointsUntilNextUnlock.SetHorizAlign( Actor::align_left );

	// get unlock data first
	// GAMESTATE->m_pUnlockingSys->LoadFromDATFile("Data\\MemCard.ini");

	CString sDP = ssprintf( "%d", (int)GAMESTATE->m_pUnlockingSys->DancePointsUntilNextUnlock() );
	CString sAP = ssprintf( "%d", (int)GAMESTATE->m_pUnlockingSys->ArcadePointsUntilNextUnlock() );
	CString sSP = ssprintf( "%d", (int)GAMESTATE->m_pUnlockingSys->SongPointsUntilNextUnlock() );

	CString PointDisplay = THEME->GetMetric("ScreenUnlock", "TypeOfPointsToDisplay");
	
	CString IconCommand = THEME->GetMetric("ScreenUnlock", "UnlockIconCommand");

	for(int i=1; i <= THEME->GetMetricI("ScreenUnlock", "NumUnlocks"); i++)
	{
		// new unlock graphic
		Unlocks[i].Load( THEME->GetPathToG(ssprintf("ScreenUnlock %d icon", i)) );

		// set graphic location
		Unlocks[i].SetName( ssprintf("Unlock%d",i) );
		SET_XY( Unlocks[i] );

		// get pertaining songentry
		SongEntry *pSong = GAMESTATE->m_pUnlockingSys->FindSong(
			THEME->GetMetric("ScreenUnlock", 
			ssprintf("Unlock%dSong", i)) );
		
		LOG->Trace("UnlockScreen: Searching for %s", THEME->GetMetric("ScreenUnlock", 
			ssprintf("Unlock%dSong", i)).c_str() );

		if( pSong == NULL)
			continue;

		Unlocks[i].Command(IconCommand);

		if ( !pSong->isLocked )
			this->AddChild(&Unlocks[i]);
	}

	// scrolling text
	if (THEME->GetMetricI("ScreenUnlock", "UnlockTextScroll") != 0)
	{
		int NumberUnlocks = THEME->GetMetricF("ScreenUnlock", "NumUnlocks");
		float ScrollingTextX = THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollX");
		float ScrollingTextStartY = THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollStartY");
		float ScrollingTextEndY = THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollEndY");
		float ScrollingTextZoom = THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollZoom");
		float ScrollingTextRows = THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollRows");
		float MaxWidth = THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollMaxWidth");

		float SecondsToScroll = THEME->GetMetricF("ScreenUnlock", "TimeToDisplay");
		if (SecondsToScroll > 2) SecondsToScroll--;

		const float SECS_PER_CYCLE = (float)SecondsToScroll/(ScrollingTextRows + NumberUnlocks);

		for(int i = 1; i <= NumberUnlocks; i++)
		{
			item[i].LoadFromFont( THEME->GetPathToF("_shared2") );
			item[i].SetHorizAlign( Actor::align_left );

			CString SongText = THEME->GetMetric("ScreenUnlock", 
				ssprintf("Unlock%dSong", i));

			SongText.MakeUpper();
			SongEntry *pSong = GAMESTATE->m_pUnlockingSys->FindSong(SongText);

			if (pSong != NULL && pSong->ActualSong != NULL)
			{
				LOG->Trace("Entry %d, pointer=%d", i, (int)pSong);
				SongText = pSong->ActualSong->GetFullDisplayTitle();
			}
			else if (pSong == NULL)  // song is not in library
				item[i].Command("Diffuse,0,1,0,1");
			else					 // song is mistyped
				item[i].Command("Diffuse,1,0,0,1");

			BreakLine(SongText);

			item[i].SetZoom(ScrollingTextZoom);
			item[i].SetTextMaxWidth(MaxWidth, SongText );

			if (pSong->isLocked)
			{
				item[i].SetText("????????");
				item[i].SetZoom(ScrollingTextZoom * 1.99);
			}

			item[i].SetXY(ScrollingTextX, ScrollingTextStartY);

			CString commands;

			commands = ssprintf("diffusealpha,0;sleep,%f;linear,0.5;diffusealpha,1;linear,%f;y,%f;linear,0.5;diffusealpha,0", SECS_PER_CYCLE * (i - 1), SECS_PER_CYCLE * (ScrollingTextRows), ScrollingTextEndY);

			LOG->Trace("Updated text %d: %s", i, SongText.c_str() );

			item[i].Command(commands);
			this->AddChild(&item[i]);
		}
	}


	PointsUntilNextUnlock.SetName( "PointsDisplay" );
	
	if (PointDisplay == "DP" || PointDisplay == "Dance")
		PointsUntilNextUnlock.SetText( sDP );

	if (PointDisplay == "AP" || PointDisplay == "Arcade")
		PointsUntilNextUnlock.SetText( sAP );

	if (PointDisplay == "SP" || PointDisplay == "Song")
		PointsUntilNextUnlock.SetText( sSP );

	PointsUntilNextUnlock.SetZoom( THEME->GetMetricF("ScreenUnlock","PointsZoom") );
	SET_XY( PointsUntilNextUnlock );
	this->AddChild( &PointsUntilNextUnlock );

	this->ClearMessageQueue( SM_BeginFadingOut );	// ignore ScreenAttract's SecsToShow

	this->PostScreenMessage( SM_BeginFadingOut, 
		THEME->GetMetricF("ScreenUnlock", "TimeToDisplay") );
}


void ScreenUnlock::BreakLine(CString& line)
{
	for(unsigned i = 1; i < line.GetLength(); i++)
		if (line[i] == '~' || line[i] == '(')
			if (line[i-1] == ' ')
			{
				line[i-1] = '\n';
				return;
			}

}