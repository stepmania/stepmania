#include "global.h"
#include "ScreenSelectGroup.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include <map>
#include "ActorUtil.h"
#include "GameState.h"
#include "UnlockSystem.h"
#include "MenuTimer.h"
#include "SongUtil.h"

#define BANNER_WIDTH					THEME->GetMetricF("ScreenSelectGroup","BannerWidth")
#define BANNER_HEIGHT					THEME->GetMetricF("ScreenSelectGroup","BannerHeight")
#define SLEEP_AFTER_TWEEN_OFF_SECONDS	THEME->GetMetricF("ScreenSelectGroup","SleepAfterTweenOffSeconds")


ScreenSelectGroup::ScreenSelectGroup( CString sClassName ) : ScreenWithMenuElements( sClassName )
{	
	LOG->Trace( "ScreenSelectGroup::ScreenSelectGroup()" );	

	if( !PREFSMAN->m_bShowSelectGroup )
	{
		GAMESTATE->m_sPreferredGroup = GROUP_ALL_MUSIC;
		HandleScreenMessage( SM_GoToNextScreen );
		return;
	}


	unsigned i;
	int j;
	
	vector<Song*> aAllSongs;
	SONGMAN->GetSongs( aAllSongs );

	// Filter out Songs that can't be played by the current Style
	for( j=aAllSongs.size()-1; j>=0; j-- )		// foreach Song, back to front
	{
		bool DisplaySong = aAllSongs[j]->NormallyDisplayed();
		
		if( UNLOCKMAN->SongIsLocked( aAllSongs[j] ) )
			DisplaySong = false;
		
		if( aAllSongs[j]->SongCompleteForStyle(GAMESTATE->GetCurrentStyle()) && 
			DisplaySong )
			continue;

		aAllSongs.erase( aAllSongs.begin()+j, aAllSongs.begin()+j+1 );
	}

	// Add all group names to a map.
	map<CString, CString> mapGroupNames;
	for( i=0; i<aAllSongs.size(); i++ )
	{
		const CString& sGroupName = aAllSongs[i]->m_sGroupName;
		mapGroupNames[ sGroupName ] = "";	// group name maps to nothing
	}

	// copy group names into a vector
	vector<CString> asGroupNames;
	asGroupNames.push_back( "ALL MUSIC" );	// special group
	for( map<CString, CString>::const_iterator iter = mapGroupNames.begin(); iter != mapGroupNames.end(); ++iter )
		asGroupNames.push_back( iter->first );

	// Add songs to the MusicList.
	m_MusicList.Load();
	for( unsigned g=0; g < asGroupNames.size(); g++ ) /* for each group */
	{
		vector<Song*> aSongsInGroup;
		/* find all songs */
		for( i=0; i<aAllSongs.size(); i++ )		// foreach Song
		{
			/* group 0 gets all songs */
			if( g != 0 && aAllSongs[i]->m_sGroupName != asGroupNames[g] )
				continue;

			aSongsInGroup.push_back( aAllSongs[i] );
		}

		SongUtil::SortSongPointerArrayByTitle( aSongsInGroup );

		m_MusicList.AddGroup();
		m_MusicList.AddSongsToGroup(aSongsInGroup);
	}

	m_bChosen = false;

	m_sprExplanation.SetName( "Explanation" );
	m_sprExplanation.Load( THEME->GetPathToG("ScreenSelectGroup explanation") );
	this->AddChild( &m_sprExplanation );

	// these guys get loaded SetSong and TweenToSong
	m_Banner.SetName( "Banner" );
	m_Banner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
	this->AddChild( &m_Banner );

	m_sprFrame.SetName( "Frame" );
	m_sprFrame.Load( THEME->GetPathToG("ScreenSelectGroup frame") );
	this->AddChild( &m_sprFrame );

	m_textNumber.SetName( "Number" );
	m_textNumber.LoadFromFont( THEME->GetPathF(m_sName, "num songs") );
	this->AddChild( &m_textNumber );
	
	m_sprContents.SetName( "Contents" );
	m_sprContents.Load( THEME->GetPathToG("ScreenSelectGroup contents") );
	this->AddChild( &m_sprContents );

	m_MusicList.SetName( "MusicList" );
	this->AddChild( &m_MusicList );
	
	m_GroupList.SetName( "GroupList" );
	m_GroupList.Load( asGroupNames );
	this->AddChild( &m_GroupList );


	m_soundChange.Load( THEME->GetPathToS("ScreenSelectGroup change") );

	SOUND->PlayOnceFromAnnouncer( "select group intro" );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenSelectGroup music") );

	AfterChange();
	TweenOnScreen();
	m_GroupList.SetSelection(0);

	this->SortByDrawOrder();
}


void ScreenSelectGroup::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectGroup::Input()" );

	if( IsTransitioning() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelectGroup::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenSelectGroup::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_BeginFadingOut:
		StartTransitioning( SM_GoToNextScreen );
		return;
	}
	Screen::HandleScreenMessage( SM );
}

void ScreenSelectGroup::AfterChange()
{
	int sel = m_GroupList.GetSelection();
	m_MusicList.SetGroupNo(sel);

	CString sSelectedGroupName = m_GroupList.GetSelectionName();
	if( sSelectedGroupName == "ALL MUSIC" )
		m_Banner.LoadAllMusic();
	else 
		m_Banner.LoadFromGroup( sSelectedGroupName );

	const int iNumSongs = m_MusicList.GetNumSongs();
	m_textNumber.SetText( ssprintf("%d", iNumSongs) );
}


void ScreenSelectGroup::MenuLeft( PlayerNumber pn )
{
	MenuUp( pn );
}

void ScreenSelectGroup::MenuRight( PlayerNumber pn )
{
	MenuDown( pn );
}

void ScreenSelectGroup::MenuUp( PlayerNumber pn )
{
	if( m_bChosen )
		return;

	m_GroupList.Up();

	AfterChange();

	m_soundChange.PlayRandom();
}


void ScreenSelectGroup::MenuDown( PlayerNumber pn )
{
	if( m_bChosen )
		return;

	m_GroupList.Down();
	
	AfterChange();

	m_soundChange.PlayRandom();
}

void ScreenSelectGroup::MenuStart( PlayerNumber pn )
{
	if( m_bChosen )
		return;

	SCREENMAN->PlayStartSound();
	m_MenuTimer->Stop();
	m_bChosen = true;

	GAMESTATE->m_pCurSong = NULL;
	GAMESTATE->m_sPreferredGroup = (m_GroupList.GetSelectionName()=="ALL MUSIC" ? GROUP_ALL_MUSIC : m_GroupList.GetSelectionName() );

	if( GAMESTATE->m_sPreferredGroup == GROUP_ALL_MUSIC )
       	SOUND->PlayOnceFromAnnouncer( "select group comment all music" );
	else
        SOUND->PlayOnceFromAnnouncer( "select group comment general" );


	TweenOffScreen();
	this->PostScreenMessage( SM_BeginFadingOut, SLEEP_AFTER_TWEEN_OFF_SECONDS );
}

void ScreenSelectGroup::MenuBack( PlayerNumber pn )
{
	Back( SM_GoToPrevScreen );
}

void ScreenSelectGroup::TweenOffScreen()
{
	OFF_COMMAND( m_sprExplanation );
	OFF_COMMAND( m_sprFrame );
	OFF_COMMAND( m_Banner );
	OFF_COMMAND( m_textNumber );
	OFF_COMMAND( m_sprContents );
	OFF_COMMAND( m_MusicList );
	OFF_COMMAND( m_GroupList );
	m_MusicList.TweenOffScreen();
	m_GroupList.TweenOffScreen();
}

void ScreenSelectGroup::TweenOnScreen() 
{
	SET_XY_AND_ON_COMMAND( m_sprExplanation );
	SET_XY_AND_ON_COMMAND( m_sprFrame );
	SET_XY_AND_ON_COMMAND( m_Banner );
	SET_XY_AND_ON_COMMAND( m_textNumber );
	SET_XY_AND_ON_COMMAND( m_sprContents );
	SET_XY_AND_ON_COMMAND( m_MusicList );
	SET_XY_AND_ON_COMMAND( m_GroupList );
	m_MusicList.TweenOnScreen();
	m_GroupList.TweenOnScreen();
}

/*
 * (c) 2001-2004 Chris Danford
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
