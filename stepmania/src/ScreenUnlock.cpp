#include "global.h"
#include "PrefsManager.h"
#include "ScreenUnlock.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "RageLog.h"
#include "UnlockManager.h"
#include "SongManager.h"
#include "ActorUtil.h"
#include "song.h"
#include "Course.h"

#define UNLOCK_TEXT_SCROLL_X			THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollX");
#define UNLOCK_TEXT_SCROLL_START_Y		THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollStartY")
#define UNLOCK_TEXT_SCROLL_END_Y		THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollEndY")
#define UNLOCK_TEXT_SCROLL_ZOOM			THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollZoom")
#define UNLOCK_TEXT_SCROLL_ROWS			THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollRows")
#define UNLOCK_TEXT_SCROLL_MAX_WIDTH	THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollMaxWidth")
#define UNLOCK_TEXT_SCROLL_ICON_X		THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollIconX")
#define UNLOCK_TEXT_SCROLL_ICON_SIZE	THEME->GetMetricF("ScreenUnlock", "UnlockTextScrollIconSize")
#define UNLOCK_TEXT_SCROLL				THEME->GetMetricI("ScreenUnlock", "UnlockTextScroll")
#define TYPE_TO_DISPLAY					THEME->GetMetric ("ScreenUnlock", "TypeOfPointsToDisplay")
#define ICON_COMMAND					THEME->GetMetricA("ScreenUnlock", "UnlockIconCommand")
#define TIME_TO_DISPLAY					THEME->GetMetricF("ScreenUnlock", "TimeToDisplay")
#define POINTS_ZOOM						THEME->GetMetricF("ScreenUnlock","PointsZoom")

REGISTER_SCREEN_CLASS( ScreenUnlock );

void ScreenUnlock::Init()
{
	ScreenAttract::Init();

	unsigned iNumUnlocks = UNLOCKMAN->m_UnlockEntries.size();

	if( !PREFSMAN->m_bUseUnlockSystem || iNumUnlocks == 0 )
	{
		this->PostScreenMessage( SM_GoToNextScreen, 0 );
		return;
	}

	unsigned NumUnlocks = UNLOCKMAN->m_UnlockEntries.size();

	PointsUntilNextUnlock.LoadFromFont( THEME->GetPathF("Common","normal") );
	PointsUntilNextUnlock.SetHorizAlign( Actor::align_left );

	apActorCommands IconCommand = ICON_COMMAND;
	for( unsigned i=1; i <= NumUnlocks; i++ )
	{
		// get pertaining UnlockEntry
		const UnlockEntry &entry = UNLOCKMAN->m_UnlockEntries[i-1];
		const Song *pSong = entry.m_pSong;

		if( pSong == NULL)
			continue;

		Sprite* pSpr = new Sprite;

		// new unlock graphic
		pSpr->Load( THEME->GetPathG("ScreenUnlock",ssprintf("%d icon", i)) );

		// set graphic location
		pSpr->SetName( ssprintf("Unlock%d",i) );
		SET_XY( pSpr );

		pSpr->RunCommands(IconCommand);
		Unlocks.push_back(pSpr);

		if ( !entry.IsLocked() )
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

		for(unsigned i = 1; i <= NumUnlocks; i++)
		{
			const UnlockEntry &entry = UNLOCKMAN->m_UnlockEntries[i-1];
			
			BitmapText* text = new BitmapText;

			text->LoadFromFont( THEME->GetPathF("ScreenUnlock","text") );
			text->SetHorizAlign( Actor::align_left );
			text->SetZoom(ScrollingTextZoom);

			switch( entry.m_Type )
			{
			case UnlockEntry::TYPE_SONG:
				{
					const Song *pSong = entry.m_pSong;
					ASSERT( pSong );
		
					RString title = pSong->GetDisplayMainTitle();
					RString subtitle = pSong->GetDisplaySubTitle();
					if( subtitle != "" )
						title = title + "\n" + subtitle;
					text->SetMaxWidth( MaxWidth );
					text->SetText( title );
				}
				break;
			case UnlockEntry::TYPE_COURSE:
				{
					const Course *pCourse = entry.m_pCourse;
					ASSERT( pCourse );

					text->SetMaxWidth( MaxWidth );
					text->SetText( pCourse->GetDisplayFullTitle() );
					text->SetDiffuse( RageColor(0,1,0,1) );
				}
				break;
			default:
				text->SetText( "" );
				text->SetDiffuse( RageColor(0.5,0,0,1) );
				break;
			}

			if( entry.IsLocked() )
			{
				text->SetText("???");
				text->SetZoomX(1);
			}
			else
			{
				// unlocked. change color
				const Song *pSong = entry.m_pSong;
				RageColor color = RageColor(1,1,1,1);
				if( pSong )
					color = SONGMAN->GetSongGroupColor(pSong->m_sGroupName);
				text->SetGlobalDiffuseColor(color);
			}

			text->SetXY( ScrollingTextX, ScrollingTextStartY );

			if (UNLOCK_TEXT_SCROLL == 3 && UNLOCK_TEXT_SCROLL_ROWS + i > NumUnlocks)
			{   // special command for last unlocks when scrolling is in effect
				float TargetRow = -0.5f + i + UNLOCK_TEXT_SCROLL_ROWS - NumUnlocks;
				float StopOffPoint = ScrollingTextEndY - TargetRow / UNLOCK_TEXT_SCROLL_ROWS * (ScrollingTextEndY - ScrollingTextStartY);
				float FirstCycleTime = (UNLOCK_TEXT_SCROLL_ROWS - TargetRow) * SECS_PER_CYCLE;
				float SecondCycleTime = (6 + TargetRow) * SECS_PER_CYCLE - FirstCycleTime;
				LOG->Trace("Target Row: %f", TargetRow);
				LOG->Trace("command for icon %d: %s", i, ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;linear,%f;y,%f;linear,0.1;diffusealpha,0", SECS_PER_CYCLE * (i - 1), FirstCycleTime, StopOffPoint, SecondCycleTime * 2, ScrollingTextEndY).c_str() );
				RString sCommand = ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;linear,%f;y,%f;linear,0.1;diffusealpha,0", SECS_PER_CYCLE * (i - 1), FirstCycleTime, StopOffPoint, SecondCycleTime, ScrollingTextEndY);
				text->RunCommands( ActorCommands(sCommand) );
			}
			else
			{
				RString sCommand = ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;linear,0.1;diffusealpha,0", SECS_PER_CYCLE * (i - 1), SECS_PER_CYCLE * (ScrollingTextRows), ScrollingTextEndY);
				text->RunCommands( ActorCommands(sCommand) );
			}

			item.push_back(text);

			if (UNLOCK_TEXT_SCROLL >= 2)
			{
				Sprite* IconCount = new Sprite;

				// new unlock graphic
				IconCount->Load( THEME->GetPathG("ScreenUnlock",ssprintf("%d icon", i)) );

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
					RString sCommand = ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;linear,%f;y,%f;linear,0.1;diffusealpha,0", SECS_PER_CYCLE * (i - 1), FirstCycleTime, StopOffPoint, SecondCycleTime, ScrollingTextEndY);
					IconCount->RunCommands( ActorCommands(sCommand) );
				}
				else
				{
					RString sCommand = ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;linear,0.1;diffusealpha,0", SECS_PER_CYCLE * (i - 1), SECS_PER_CYCLE * (ScrollingTextRows), ScrollingTextEndY);
					IconCount->RunCommands( ActorCommands(sCommand) );
				}

				ItemIcons.push_back(IconCount);

				LOG->Trace("Added unlock text %d", i);
					
				if (UNLOCK_TEXT_SCROLL == 3)
				{
					if ( !entry.IsLocked() )
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

		for(unsigned i=1; i <= UNLOCK_TEXT_SCROLL_ROWS; i++)
		{
			if (i > LastUnlocks.size())
				continue;

			unsigned NextIcon = LastUnlocks[LastUnlocks.size() - i];

			const UnlockEntry &entry = UNLOCKMAN->m_UnlockEntries[NextIcon-1];
			const Song *pSong = entry.m_pSong;
			if( pSong == NULL )
				continue;

			BitmapText* NewText = new BitmapText;

			NewText->LoadFromFont( THEME->GetPathF("ScreenUnlock","text") );
			NewText->SetHorizAlign( Actor::align_left );

			RString title = pSong->GetDisplayMainTitle();
			RString subtitle = pSong->GetDisplaySubTitle();

			if( subtitle != "" )
				title = title + "\n" + subtitle;
			NewText->SetZoom(UNLOCK_TEXT_SCROLL_ZOOM);
			NewText->SetMaxWidth( MaxWidth );
			NewText->SetText( title );

			RageColor color = SONGMAN->GetSongGroupColor(pSong->m_sGroupName);
			NewText->SetGlobalDiffuseColor(color);

			NewText->SetXY(ScrollingTextX, ScrollingTextStartY);
			{
				RString sCommand = ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;", SECS_PER_CYCLE * (NumUnlocks + 2 * i - 2), SECS_PER_CYCLE * ((ScrollingTextRows - i) * 2 + 1 ), (ScrollingTextStartY + (ScrollingTextEndY - ScrollingTextStartY) * (ScrollingTextRows - i + 0.5) / ScrollingTextRows ));
				NewText->RunCommands( ActorCommands(sCommand) );
			}

			// new unlock graphic
			Sprite* NewIcon = new Sprite;
			NewIcon->Load( THEME->GetPathG("ScreenUnlock",ssprintf("%d icon", NextIcon)) );
			NewIcon->SetXY( UNLOCK_TEXT_SCROLL_ICON_X, ScrollingTextStartY);
			NewIcon->SetHeight(UNLOCK_TEXT_SCROLL_ICON_SIZE);
			NewIcon->SetWidth(UNLOCK_TEXT_SCROLL_ICON_SIZE);
			{
				RString sCommand = ssprintf("diffusealpha,0;sleep,%f;diffusealpha,1;linear,%f;y,%f;", SECS_PER_CYCLE * (NumUnlocks + 2 * i - 2), SECS_PER_CYCLE * ((ScrollingTextRows - i) * 2 + 1 ), (ScrollingTextStartY + (ScrollingTextEndY - ScrollingTextStartY) * (ScrollingTextRows - i + 0.5) / ScrollingTextRows ));
				NewIcon->RunCommands( ActorCommands(sCommand) );
			}

			ItemIcons.push_back(NewIcon);
			item.push_back(NewText);
		}
	}

	// NOTE: the following two loops require the iterator to 
	// be ints because if you decrement an unsigned when it
	// equals zero, you get the maximum value of an unsigned,
	// which is still greater than 0.  By typecasting it as
	// an integer, you can achieve -1, which exits the loop.

	for(int i = item.size() - 1; (int)i >= 0; i--)
		this->AddChild(item[i]);

	for(int i = ItemIcons.size() - 1; (int)i >= 0; i--)
		this->AddChild(ItemIcons[i]);

	PointsUntilNextUnlock.SetName( "PointsDisplay" );
	
	RString PointDisplay = TYPE_TO_DISPLAY;
	if (PointDisplay == "DP" || PointDisplay == "Dance")
	{
		RString sDP = ssprintf( "%d", (int)UNLOCKMAN->DancePointsUntilNextUnlock() );
		PointsUntilNextUnlock.SetText( sDP );
	} else if (PointDisplay == "AP" || PointDisplay == "Arcade") {
		RString sAP = ssprintf( "%d", (int)UNLOCKMAN->ArcadePointsUntilNextUnlock() );
		PointsUntilNextUnlock.SetText( sAP );
	} else if (PointDisplay == "SP" || PointDisplay == "Song") {
		RString sSP = ssprintf( "%d", (int)UNLOCKMAN->SongPointsUntilNextUnlock() );
		PointsUntilNextUnlock.SetText( sSP );
	}

	PointsUntilNextUnlock.SetZoom( POINTS_ZOOM );
	SET_XY( PointsUntilNextUnlock );
	this->AddChild( &PointsUntilNextUnlock );

	this->ClearMessageQueue( SM_BeginFadingOut );	// ignore ScreenAttract's SecsToShow

	this->PostScreenMessage( SM_BeginFadingOut, TIME_TO_DISPLAY );

	this->SortByDrawOrder();
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

/*
 * (c) 2003 Andrew Wong
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
