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


#define FRAME_X				THEME->GetMetricF("ScreenSelectGroup","FrameX")
#define FRAME_Y				THEME->GetMetricF("ScreenSelectGroup","FrameY")
#define BANNER_X			THEME->GetMetricF("ScreenSelectGroup","BannerX")
#define BANNER_Y			THEME->GetMetricF("ScreenSelectGroup","BannerY")
#define BANNER_WIDTH		THEME->GetMetricF("ScreenSelectGroup","BannerWidth")
#define BANNER_HEIGHT		THEME->GetMetricF("ScreenSelectGroup","BannerHeight")
#define NUMBER_X			THEME->GetMetricF("ScreenSelectGroup","NumberX")
#define NUMBER_Y			THEME->GetMetricF("ScreenSelectGroup","NumberY")
#define EXPLANATION_X		THEME->GetMetricF("ScreenSelectGroup","ExplanationX")
#define EXPLANATION_Y		THEME->GetMetricF("ScreenSelectGroup","ExplanationY")
#define CONTENTS_X			THEME->GetMetricF("ScreenSelectGroup","ContentsX")
#define CONTENTS_Y			THEME->GetMetricF("ScreenSelectGroup","ContentsY")
#define HELP_TEXT			THEME->GetMetric("ScreenSelectGroup","HelpText")
#define TIMER_SECONDS		THEME->GetMetricI("ScreenSelectGroup","TimerSeconds")
#define NEXT_SCREEN			THEME->GetMetric("ScreenSelectGroup","NextScreen")


const ScreenMessage SM_StartFadingOut		=	ScreenMessage(SM_User + 3);


ScreenSelectGroup::ScreenSelectGroup()
{
	
	LOG->Trace( "ScreenSelectGroup::ScreenSelectGroup()" );	

	m_Menu.Load(
		THEME->GetPathTo("BGAnimations","select group") , 
		THEME->GetPathTo("Graphics","select group top edge"),
		HELP_TEXT, true, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );

	if(!PREFSMAN->m_bShowSelectGroup)
	{
		GAMESTATE->m_sPreferredGroup = GROUP_ALL_MUSIC;
		m_Menu.ImmedOffScreenToMenu();
		m_bChosen = true;
		this->SendScreenMessage( SM_GoToNextScreen, 0.f );
		return;
	}


	unsigned i;
	int j;

	// The new process by which the group and song lists are formed
	// is bizarre and complex but yields the end result that songs
	// and groups that do not contain any steps for the current
	// style (such as solo) are omitted. Bear with me!
	// -- dro kulix

	// Chris:
	// This is excellent!  I'm going to move the filtering of songs 
	// that can't be played by current style to be the first action.
	// This will simply the code a bit, and fix a weird case that 
	// causes a crash when there are duplicate song names.
	
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

	m_sprExplanation.Load( THEME->GetPathTo("Graphics","select group explanation") );
	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddChild( &m_sprExplanation );

	// these guys get loaded SetSong and TweenToSong
	m_Banner.SetXY( BANNER_X, BANNER_Y );
	m_Banner.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
	this->AddChild( &m_Banner );

	m_sprFrame.Load( THEME->GetPathTo("Graphics","select group info frame") );
	m_sprFrame.SetXY( FRAME_X, FRAME_Y );
	this->AddChild( &m_sprFrame );

	m_textNumber.LoadFromFont( THEME->GetPathTo("Fonts","select group num songs") );
	m_textNumber.SetXY( NUMBER_X, NUMBER_Y );
	m_textNumber.SetHorizAlign( Actor::align_right );
	m_textNumber.TurnShadowOff();
	this->AddChild( &m_textNumber );
	
	m_sprContents.Load( THEME->GetPathTo("Graphics","select group contents header") );
	m_sprContents.SetXY( CONTENTS_X, CONTENTS_Y );
	this->AddChild( &m_sprContents );

	this->AddChild( &m_MusicList );
	
	for( i=0; i < asGroupNames.size(); ++i )
		m_GroupList.AddGroup( asGroupNames[i] );
	m_GroupList.DoneAddingGroups();
	this->AddChild( &m_GroupList );


	m_soundChange.Load( THEME->GetPathTo("Sounds","select group change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select group intro") );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","select group music") );

	m_Menu.TweenOnScreenFromMenu( SM_None );
	TweenOnScreen();
	AfterChange();
	m_GroupList.SetSelection(0);
}


ScreenSelectGroup::~ScreenSelectGroup()
{
	LOG->Trace( "ScreenSelectGroup::~ScreenSelectGroup()" );

}


void ScreenSelectGroup::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectGroup::Input()" );

	if( m_Menu.IsClosing()  ||  m_bChosen )
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
	case SM_MenuTimer:
		MenuStart(PLAYER_1);
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	case SM_StartFadingOut:
		m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
		break;
	}
}

void ScreenSelectGroup::AfterChange()
{
	int sel = m_GroupList.GetSelection();
	m_MusicList.SetGroupNo(sel);

	CString sSelectedGroupName = m_GroupList.GetSelectionName();

	CString sGroupBannerPath;
	if( sSelectedGroupName == GROUP_ALL_MUSIC )
		sGroupBannerPath = THEME->GetPathTo("Graphics","all music banner");
	else if( SONGMAN->GetGroupBannerPath(sSelectedGroupName) != "" )
		sGroupBannerPath = SONGMAN->GetGroupBannerPath(sSelectedGroupName);
	else
		sGroupBannerPath = THEME->GetPathTo("Graphics","fallback banner");

	const int iNumSongs = m_MusicList.GetNumSongs();
	m_textNumber.SetText( ssprintf("%d", iNumSongs) );

	m_Banner.LoadFromGroup( sSelectedGroupName );
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
	m_soundSelect.PlayRandom();
	m_bChosen = true;

	GAMESTATE->m_pCurSong = NULL;
	GAMESTATE->m_sPreferredGroup = (m_GroupList.GetSelectionName()=="ALL MUSIC" ? GROUP_ALL_MUSIC : m_GroupList.GetSelectionName() );

	if( GAMESTATE->m_sPreferredGroup == GROUP_ALL_MUSIC )
        SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select group comment all music") );
	else
        SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select group comment general") );


	TweenOffScreen();
	m_Menu.StopTimer();

	this->SendScreenMessage( SM_StartFadingOut, 0.8f );
}

void ScreenSelectGroup::MenuBack( PlayerNumber pn )
{
	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}

void ScreenSelectGroup::TweenOffScreen()
{
	m_sprExplanation.BeginTweening( 0.8f );
	m_sprExplanation.BeginTweening( 0.5f, TWEEN_BOUNCE_BEGIN );
	m_sprExplanation.SetTweenX( EXPLANATION_X-400 );

	Actor* pActorsInGroupInfoFrame[] = { &m_sprFrame, &m_Banner, &m_textNumber };
	const int iNumActorsInGroupInfoFrame = sizeof(pActorsInGroupInfoFrame) / sizeof(Actor*);
	for( int i=0; i<iNumActorsInGroupInfoFrame; i++ )
	{
		pActorsInGroupInfoFrame[i]->BeginTweening( 0.9f );
		pActorsInGroupInfoFrame[i]->BeginTweening( 0.5f, TWEEN_BOUNCE_BEGIN );
		pActorsInGroupInfoFrame[i]->SetTweenX( pActorsInGroupInfoFrame[i]->GetX()-400 );
	}

	m_sprContents.BeginTweening( 0.7f );
	m_sprContents.BeginTweening( 0.5f, TWEEN_BIAS_END );
	m_sprContents.SetTweenY( CONTENTS_Y+400 );
	
	m_MusicList.TweenOffScreen();
	m_GroupList.TweenOffScreen();
}

void ScreenSelectGroup::TweenOnScreen() 
{
	m_sprExplanation.SetX( EXPLANATION_X-400 );
	m_sprExplanation.BeginTweening( 0.5f, TWEEN_BOUNCE_END );
	m_sprExplanation.SetTweenX( EXPLANATION_X );

	Actor* pActorsInGroupInfoFrame[] = { &m_sprFrame, &m_Banner, &m_textNumber };
	const int iNumActorsInGroupInfoFrame = sizeof(pActorsInGroupInfoFrame) / sizeof(Actor*);
	for( int i=0; i<iNumActorsInGroupInfoFrame; i++ )
	{
		float fOriginalX = pActorsInGroupInfoFrame[i]->GetX();
		pActorsInGroupInfoFrame[i]->SetX( fOriginalX-400 );
		pActorsInGroupInfoFrame[i]->BeginTweening( 0.5f, TWEEN_BOUNCE_END );
		pActorsInGroupInfoFrame[i]->SetTweenX( fOriginalX );
	}

	m_sprContents.SetY( CONTENTS_Y+400 );
	m_sprContents.BeginTweening( 0.5f, TWEEN_BIAS_END );	// sleep
	m_sprContents.BeginTweening( 0.5f, TWEEN_BIAS_END );
	m_sprContents.SetTweenY( CONTENTS_Y );

	m_MusicList.TweenOnScreen();
	m_GroupList.TweenOnScreen();
}
