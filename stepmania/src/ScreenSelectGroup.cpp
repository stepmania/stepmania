#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSelectGroup.cpp

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelectGroup.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "ScreenSelectMusic.h"
#include "ScreenTitleMenu.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "AnnouncerManager.h"


const float GROUP_INFO_FRAME_X	=	SCREEN_LEFT + 180;
const float GROUP_INFO_FRAME_Y	=	SCREEN_TOP + 160;

const float EXPLANATION_X	=	SCREEN_LEFT + 110;
const float EXPLANATION_Y	=	SCREEN_TOP + 65;

const float BUTTON_X			=	SCREEN_RIGHT - 135;
const float BUTTON_SELECTED_X	=	BUTTON_X - 30;
const float BUTTON_START_Y		=	SCREEN_TOP + 65;
const float BUTTON_GAP_Y		=	26;

const float CONTENTS_X			=	CENTER_X;
const float CONTENTS_START_Y	=	SCREEN_TOP + SCREEN_HEIGHT*3/5 - 15;

const int TITLES_PER_COLUMN = 10;

const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User + 2);
const ScreenMessage SM_StartFadingOut		 =	ScreenMessage(SM_User + 3);


ScreenSelectGroup::ScreenSelectGroup()
{
	LOG->WriteLine( "ScreenSelectGroup::ScreenSelectGroup()" );

	int i;

	SONGMAN->GetGroupNames( m_arrayGroupNames );
	m_arrayGroupNames.InsertAt( 0, "ALL MUSIC" );
	
	m_iSelection = 0;
	m_bChosen = false;

	m_Menu.Load(
		THEME->GetPathTo(GRAPHIC_SELECT_GROUP_BACKGROUND) , 
		THEME->GetPathTo(GRAPHIC_SELECT_GROUP_TOP_EDGE),
		ssprintf("Use %c %c to select, then press NEXT", char(1), char(2))
		);
	this->AddActor( &m_Menu );

	m_sprExplanation.Load( THEME->GetPathTo(GRAPHIC_SELECT_GROUP_EXPLANATION) );
	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddActor( &m_sprExplanation );

	m_GroupInfoFrame.SetXY( GROUP_INFO_FRAME_X, GROUP_INFO_FRAME_Y );
	this->AddActor( &m_GroupInfoFrame );

	m_sprContentsHeader.Load( THEME->GetPathTo(GRAPHIC_SELECT_GROUP_CONTENTS_HEADER) );
	m_sprContentsHeader.SetVertAlign( Actor::align_top );
	m_sprContentsHeader.SetXY( CONTENTS_X, CONTENTS_START_Y );
	this->AddActor( &m_sprContentsHeader );

	for( i=0; i<NUM_CONTENTS_COLUMNS; i++ )
	{
		const float fX = SCREEN_WIDTH * i / (float)NUM_CONTENTS_COLUMNS + 15;
		m_textContents[i].Load( THEME->GetPathTo(FONT_NORMAL) );
		m_textContents[i].SetXY( fX, CONTENTS_START_Y+15 );
		m_textContents[i].SetHorizAlign( Actor::align_left );
		m_textContents[i].SetVertAlign( Actor::align_bottom );
		m_textContents[i].SetZoom( 0.5f );
		m_textContents[i].SetShadowLength( 2 );
		this->AddActor( &m_textContents[i] );
	}
	

	for( i=0; i<min(m_arrayGroupNames.GetSize(), MAX_GROUPS); i++ )
	{
		CString sGroupName = m_arrayGroupNames[i];

		m_sprGroupButton[i].Load( THEME->GetPathTo(GRAPHIC_SELECT_GROUP_BUTTON) );
		m_sprGroupButton[i].SetXY( BUTTON_X, BUTTON_START_Y + BUTTON_GAP_Y*i );
		this->AddActor( &m_sprGroupButton[i] );

		m_textGroup[i].Load( THEME->GetPathTo(FONT_NORMAL) );
		m_textGroup[i].SetXY( BUTTON_X, BUTTON_START_Y + BUTTON_GAP_Y*i );
		m_textGroup[i].SetText( SONGMAN->ShortenGroupName( sGroupName ) );
		m_textGroup[i].SetZoom( 0.8f );
		if( m_textGroup[i].GetWidestLineWidthInSourcePixels()*0.8f > m_sprGroupButton[i].GetUnzoomedWidth() )
			m_textGroup[i].SetZoomX( m_textGroup[i].GetWidestLineWidthInSourcePixels()*0.8f / (float)m_sprGroupButton[i].GetUnzoomedWidth() );
		m_textGroup[i].SetShadowLength( 2 );

		if( i == 0 )	m_textGroup[i].TurnRainbowOn();
		else			m_textGroup[i].SetDiffuseColor( SONGMAN->GetGroupColor(sGroupName) );

		this->AddActor( &m_textGroup[i] );
	}


	m_Fade.SetZ( -2 );
	this->AddActor( &m_Fade );

	m_soundChange.Load( THEME->GetPathTo(SOUND_SELECT_DIFFICULTY_CHANGE) );
	m_soundSelect.Load( THEME->GetPathTo(SOUND_SELECT) );

	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_GROUP_INTRO) );


	if( !MUSIC->IsPlaying() )
	{
		MUSIC->Load( THEME->GetPathTo(SOUND_MENU_MUSIC) );
        MUSIC->Play( true );
	}

	m_Fade.OpenWipingRight();
	TweenOnScreen();
	AfterChange();
}


ScreenSelectGroup::~ScreenSelectGroup()
{
	LOG->WriteLine( "ScreenSelectGroup::~ScreenSelectGroup()" );

}


void ScreenSelectGroup::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->WriteLine( "ScreenSelectGroup::Input()" );

	if( m_Fade.IsClosing()  ||  m_bChosen )
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
	case SM_GoToPrevState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( new ScreenSelectMusic );
		break;
	case SM_StartFadingOut:
		m_Fade.CloseWipingRight( SM_GoToNextState );
		break;
	}
}

void ScreenSelectGroup::BeforeChange()
{
	int iSel = m_iSelection;

	m_sprGroupButton[iSel].BeginTweening( 0.2f );
	m_sprGroupButton[iSel].SetTweenX( BUTTON_X );
	m_sprGroupButton[iSel].SetEffectNone();

	m_textGroup[iSel].BeginTweening( 0.2f );
	m_textGroup[iSel].SetTweenX( BUTTON_X );
	m_textGroup[iSel].SetEffectNone();
}

void ScreenSelectGroup::AfterChange()
{
	int iSel = m_iSelection;

	m_sprGroupButton[iSel].BeginTweening( 0.2f );
	m_sprGroupButton[iSel].SetTweenX( BUTTON_SELECTED_X );
	m_sprGroupButton[iSel].SetEffectGlowing();

	m_textGroup[iSel].BeginTweening( 0.2f );
	m_textGroup[iSel].SetTweenX( BUTTON_SELECTED_X );
	m_textGroup[iSel].SetEffectGlowing();


	CArray<Song*, Song*> arraySongs;
	const CString sSelectedGroupName = m_arrayGroupNames[m_iSelection];
	
	if( m_iSelection == 0 )		arraySongs.Copy( SONGMAN->m_pSongs );
	else						SONGMAN->GetSongsInGroup( m_arrayGroupNames[m_iSelection], arraySongs );

	for( int i=0; i<NUM_CONTENTS_COLUMNS; i++ )
	{
		CString sText;
		for( int j=i*TITLES_PER_COLUMN; j<(i+1)*TITLES_PER_COLUMN; j++ )
		{
			if( j < arraySongs.GetSize() )
			{
				if( j == NUM_CONTENTS_COLUMNS * TITLES_PER_COLUMN - 1 )
					sText += ssprintf( "(%d more).....", arraySongs.GetSize() - NUM_CONTENTS_COLUMNS * TITLES_PER_COLUMN - 2 );
				else
					sText += arraySongs[j]->GetMainTitle() + "\n";
			}
		}
		m_textContents[i].SetText( sText );
	}

	CString sGroupBannerPath;
	if( 0 == stricmp(sSelectedGroupName, "ALL MUSIC") )
		sGroupBannerPath = THEME->GetPathTo(GRAPHIC_ALL_MUSIC_BANNER);
	else if( SONGMAN->GetGroupBannerPath(sSelectedGroupName) != "" )
		sGroupBannerPath = SONGMAN->GetGroupBannerPath(sSelectedGroupName);
	else
		sGroupBannerPath = THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER);

	// There is too much Z-fighting when we rotate this, so fake a rotation with a squash
	m_GroupInfoFrame.Set( sGroupBannerPath, arraySongs.GetSize() );
}


void ScreenSelectGroup::MenuLeft( PlayerNumber p )
{
	MenuUp( p );
}

void ScreenSelectGroup::MenuRight( PlayerNumber p )
{
	MenuDown( p );
}

void ScreenSelectGroup::MenuUp( PlayerNumber p )
{
	if( m_bChosen )
		return;

	BeforeChange();

	m_iSelection = m_iSelection-1 % m_arrayGroupNames.GetSize();
	if( m_iSelection < 0 )
		m_iSelection += m_arrayGroupNames.GetSize();

	AfterChange();

	m_soundChange.PlayRandom();
}


void ScreenSelectGroup::MenuDown( PlayerNumber p )
{
	if( m_bChosen )
		return;

	BeforeChange();

	m_iSelection = (m_iSelection+1) % m_arrayGroupNames.GetSize();
	
	AfterChange();

	m_soundChange.PlayRandom();
}

void ScreenSelectGroup::MenuStart( PlayerNumber p )
{
	m_soundSelect.PlayRandom();
	m_bChosen = true;

	SONGMAN->m_pCurSong = NULL;
	SONGMAN->m_sPreferredGroup = m_arrayGroupNames[m_iSelection];

	if( 0 == stricmp(SONGMAN->m_sPreferredGroup, "All Music") )
        SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_GROUP_COMMENT_ALL_MUSIC) );
	else
        SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_GROUP_COMMENT_GENERAL) );


	TweenOffScreen();
	this->SendScreenMessage( SM_StartFadingOut, 0.8f );
}

void ScreenSelectGroup::MenuBack( PlayerNumber p )
{
	m_Fade.CloseWipingLeft( SM_GoToPrevState );

	TweenOffScreen();
}

void ScreenSelectGroup::TweenOffScreen()
{
	m_Menu.TweenTopEdgeOffScreen();

	m_sprExplanation.BeginTweeningQueued( 0.8f );
	m_sprExplanation.BeginTweeningQueued( 0.5f, TWEEN_BOUNCE_BEGIN );
	m_sprExplanation.SetTweenX( EXPLANATION_X-400 );

	m_GroupInfoFrame.BeginTweeningQueued( 0.9f );
	m_GroupInfoFrame.BeginTweeningQueued( 0.5f, TWEEN_BOUNCE_BEGIN );
	m_GroupInfoFrame.SetTweenX( GROUP_INFO_FRAME_X-400 );

	m_sprContentsHeader.BeginTweeningQueued( 0.7f );
	m_sprContentsHeader.BeginTweeningQueued( 0.5f, TWEEN_BIAS_END );
	m_sprContentsHeader.SetTweenY( CONTENTS_START_Y+400 );

	int i;

	for( i=0; i<NUM_CONTENTS_COLUMNS; i++ )
	{
		m_textContents[i].BeginTweeningQueued( 0.7f );
		m_textContents[i].BeginTweeningQueued( 0.5f );
		m_textContents[i].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
	}
	

	for( i=0; i<min(m_arrayGroupNames.GetSize(), MAX_GROUPS); i++ )
	{
		if( i == m_iSelection )
			m_sprGroupButton[i].BeginTweeningQueued( 1.0f, TWEEN_BOUNCE_BEGIN );
		else
			m_sprGroupButton[i].BeginTweeningQueued( 0.1f*i, TWEEN_BOUNCE_BEGIN );
		m_sprGroupButton[i].BeginTweeningQueued( 0.2f, TWEEN_BOUNCE_BEGIN );
		m_sprGroupButton[i].SetTweenX( BUTTON_X+400 );

		if( i == m_iSelection )
			m_textGroup[i].BeginTweeningQueued( 1.0f, TWEEN_BOUNCE_BEGIN );
		else
			m_textGroup[i].BeginTweeningQueued( 0.1f*i, TWEEN_BOUNCE_BEGIN );
		m_textGroup[i].BeginTweeningQueued( 0.2f, TWEEN_BOUNCE_BEGIN );
		m_textGroup[i].SetTweenX( BUTTON_X+400 );
	}
}

void ScreenSelectGroup::TweenOnScreen() 
{
	m_Menu.TweenTopEdgeOnScreen();

	m_sprExplanation.SetX( EXPLANATION_X-400 );
	m_sprExplanation.BeginTweening( 0.5f, TWEEN_BOUNCE_END );
	m_sprExplanation.SetTweenX( EXPLANATION_X );

	m_GroupInfoFrame.SetX( GROUP_INFO_FRAME_X-400 );
	m_GroupInfoFrame.BeginTweening( 0.5f, TWEEN_BOUNCE_END );
	m_GroupInfoFrame.SetTweenX( GROUP_INFO_FRAME_X );

	m_sprContentsHeader.SetY( CONTENTS_START_Y+400 );
	m_sprContentsHeader.BeginTweeningQueued( 0.5f, TWEEN_BIAS_END );
	m_sprContentsHeader.BeginTweeningQueued( 0.5f, TWEEN_BIAS_END );
	m_sprContentsHeader.SetTweenY( CONTENTS_START_Y );

	int i;

	for( i=0; i<NUM_CONTENTS_COLUMNS; i++ )
	{
		m_textContents[i].SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
		m_textContents[i].BeginTweeningQueued( 0.5f );
		m_textContents[i].BeginTweeningQueued( 0.5f );
		m_textContents[i].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
	}
	

	for( i=0; i<min(m_arrayGroupNames.GetSize(), MAX_GROUPS); i++ )
	{
		m_sprGroupButton[i].SetX( BUTTON_X+400 );
		m_sprGroupButton[i].BeginTweeningQueued( 0.1f*i, TWEEN_BOUNCE_END );
		m_sprGroupButton[i].BeginTweeningQueued( 0.2f, TWEEN_BOUNCE_END );
		m_sprGroupButton[i].SetTweenX( BUTTON_X );

		m_textGroup[i].SetX( BUTTON_X+400 );
		m_textGroup[i].BeginTweeningQueued( 0.1f*i, TWEEN_BOUNCE_END );
		m_textGroup[i].BeginTweeningQueued( 0.2f, TWEEN_BOUNCE_END );
		m_textGroup[i].SetTweenX( BUTTON_X );
	}
}
