#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectGroup

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelectGroup.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageSoundManager.h"
#include "ThemeManager.h"
#include <map>


#define FRAME_ON_COMMAND				THEME->GetMetric ("ScreenSelectGroup","FrameOnCommand")
#define FRAME_OFF_COMMAND				THEME->GetMetric ("ScreenSelectGroup","FrameOffCommand")
#define BANNER_ON_COMMAND				THEME->GetMetric ("ScreenSelectGroup","BannerOnCommand")
#define BANNER_OFF_COMMAND				THEME->GetMetric ("ScreenSelectGroup","BannerOffCommand")
#define BANNER_WIDTH					THEME->GetMetricF("ScreenSelectGroup","BannerWidth")
#define BANNER_HEIGHT					THEME->GetMetricF("ScreenSelectGroup","BannerHeight")
#define NUMBER_ON_COMMAND				THEME->GetMetric ("ScreenSelectGroup","NumberOnCommand")
#define NUMBER_OFF_COMMAND				THEME->GetMetric ("ScreenSelectGroup","NumberOffCommand")
#define EXPLANATION_ON_COMMAND			THEME->GetMetric ("ScreenSelectGroup","ExplanationOnCommand")
#define EXPLANATION_OFF_COMMAND			THEME->GetMetric ("ScreenSelectGroup","ExplanationOffCommand")
#define CONTENTS_ON_COMMAND				THEME->GetMetric ("ScreenSelectGroup","ContentsOnCommand")
#define CONTENTS_OFF_COMMAND			THEME->GetMetric ("ScreenSelectGroup","ContentsOffCommand")
#define MUSIC_LIST_ON_COMMAND			THEME->GetMetric ("ScreenSelectGroup","MusicListOnCommand")
#define MUSIC_LIST_OFF_COMMAND			THEME->GetMetric ("ScreenSelectGroup","MusicListOffCommand")
#define GROUP_LIST_ON_COMMAND			THEME->GetMetric ("ScreenSelectGroup","GroupListOnCommand")
#define GROUP_LIST_OFF_COMMAND			THEME->GetMetric ("ScreenSelectGroup","GroupListOnCommand")
#define SLEEP_AFTER_TWEEN_OFF_SECONDS	THEME->GetMetricF("ScreenSelectGroup","SleepAfterTweenOffSeconds")
#define HELP_TEXT						THEME->GetMetric ("ScreenSelectGroup","HelpText")
#define TIMER_SECONDS					THEME->GetMetricI("ScreenSelectGroup","TimerSeconds")
#define NEXT_SCREEN						THEME->GetMetric ("ScreenSelectGroup","NextScreen")


ScreenSelectGroup::ScreenSelectGroup()
{	
	LOG->Trace( "ScreenSelectGroup::ScreenSelectGroup()" );	

	if( !PREFSMAN->m_bShowSelectGroup )
	{
		GAMESTATE->m_sPreferredGroup = GROUP_ALL_MUSIC;
		HandleScreenMessage( SM_GoToNextScreen );
		return;
	}



	m_Menu.Load( "ScreenSelectGroup" );
	this->AddChild( &m_Menu );


	unsigned i;
	int j;

	// The new process by which the group and song lists are formed
	// is bizarre and complex but yields the end result that songs
	// and groups that do not contain any steps for the current
	// style (such as solo) are omitted. Bear with me!
	// -- dro kulix

	
	vector<Song*> aAllSongs;
	SONGMAN->GetSongs( aAllSongs );

	// Filter out Songs that can't be played by the current Style
	for( j=aAllSongs.size()-1; j>=0; j-- )		// foreach Song, back to front
	{
		if( aAllSongs[j]->SongCompleteForStyle(GAMESTATE->GetCurrentStyleDef()) && 
			aAllSongs[j]->NormallyDisplayed() )
			continue;

		aAllSongs.erase( aAllSongs.begin()+j, aAllSongs.begin()+j+1 );
	}

	// Add all group names to a map.
	std::map<CString, CString> mapGroupNames;
	for( i=0; i<aAllSongs.size(); i++ )
	{
		const CString& sGroupName = aAllSongs[i]->m_sGroupName;
		mapGroupNames[ sGroupName ] = "";	// group name maps to nothing
	}

	// copy group names into a vector
	std::vector<CString> asGroupNames;
	asGroupNames.push_back( "ALL MUSIC" );	// special group
	for( std::map<CString, CString>::const_iterator iter = mapGroupNames.begin(); iter != mapGroupNames.end(); ++iter )
		asGroupNames.push_back( iter->first );

	// Add songs to the MusicList.
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

		SortSongPointerArrayByTitle( aSongsInGroup );

		m_MusicList.AddGroup();
		m_MusicList.AddSongsToGroup(aSongsInGroup);
	}

	m_bChosen = false;

	m_sprExplanation.Load( THEME->GetPathTo("Graphics","ScreenSelectGroup explanation") );
	this->AddChild( &m_sprExplanation );

	// these guys get loaded SetSong and TweenToSong
	m_Banner.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
	this->AddChild( &m_Banner );

	m_sprFrame.Load( THEME->GetPathTo("Graphics","ScreenSelectGroup frame") );
	this->AddChild( &m_sprFrame );

	m_textNumber.LoadFromNumbers( THEME->GetPathTo("Numbers","ScreenSelectGroup numbers") );
	this->AddChild( &m_textNumber );
	
	m_sprContents.Load( THEME->GetPathTo("Graphics","ScreenSelectGroup contents") );
	this->AddChild( &m_sprContents );

	this->AddChild( &m_MusicList );
	
	m_GroupList.Load( asGroupNames );
	this->AddChild( &m_GroupList );


	m_soundChange.Load( THEME->GetPathTo("Sounds","ScreenSelectGroup change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","Common start") );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select group intro") );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","ScreenSelectGroup music") );

	AfterChange();
	TweenOnScreen();
	m_GroupList.SetSelection(0);
}


ScreenSelectGroup::~ScreenSelectGroup()
{
	LOG->Trace( "ScreenSelectGroup::~ScreenSelectGroup()" );

}


void ScreenSelectGroup::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectGroup::Input()" );

	if( m_Menu.IsTransitioning() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelectGroup::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectGroup::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_BeginFadingOut:
		m_Menu.StartTransitioning( SM_GoToNextScreen );
		break;
	case SM_MenuTimer:
		MenuStart(PLAYER_1);
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
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

	m_soundSelect.PlayRandom();
	m_Menu.m_MenuTimer.Stop();
	m_bChosen = true;

	GAMESTATE->m_pCurSong = NULL;
	GAMESTATE->m_sPreferredGroup = (m_GroupList.GetSelectionName()=="ALL MUSIC" ? GROUP_ALL_MUSIC : m_GroupList.GetSelectionName() );

	if( GAMESTATE->m_sPreferredGroup == GROUP_ALL_MUSIC )
        SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select group comment all music") );
	else
        SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select group comment general") );


	TweenOffScreen();
	this->SendScreenMessage( SM_BeginFadingOut, SLEEP_AFTER_TWEEN_OFF_SECONDS );
}

void ScreenSelectGroup::MenuBack( PlayerNumber pn )
{
	m_Menu.Back( SM_GoToPrevScreen );
}

void ScreenSelectGroup::TweenOffScreen()
{
	m_sprExplanation.Command( EXPLANATION_OFF_COMMAND );
	m_sprFrame.Command( FRAME_OFF_COMMAND );
	m_Banner.Command( BANNER_OFF_COMMAND );
	m_textNumber.Command( NUMBER_OFF_COMMAND );
	m_sprContents.Command( CONTENTS_OFF_COMMAND );
	m_MusicList.Command( MUSIC_LIST_OFF_COMMAND );
	m_GroupList.Command( GROUP_LIST_OFF_COMMAND );
	m_MusicList.TweenOffScreen();
	m_GroupList.TweenOffScreen();
}

void ScreenSelectGroup::TweenOnScreen() 
{
	m_sprExplanation.Command( EXPLANATION_ON_COMMAND );
	m_sprFrame.Command( FRAME_ON_COMMAND );
	m_Banner.Command( BANNER_ON_COMMAND );
	m_textNumber.Command( NUMBER_ON_COMMAND );
	m_sprContents.Command( CONTENTS_ON_COMMAND );
	m_MusicList.Command( MUSIC_LIST_ON_COMMAND );
	m_GroupList.Command( GROUP_LIST_ON_COMMAND );
	m_MusicList.TweenOnScreen();
	m_GroupList.TweenOnScreen();
}
