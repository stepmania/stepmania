#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSelectDifficulty.cpp

 Desc: Testing the Screen class.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#include "ScreenSelectDifficulty.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "ScreenSelectGroup.h"
#include "ScreenTitleMenu.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "AnnouncerManager.h"
#include "ScreenSelectCourse.h"


const float MORE_ARROWS_X[NUM_PAGES]	=	{ SCREEN_RIGHT-60, SCREEN_WIDTH + SCREEN_LEFT+60};
const float MORE_ARROWS_Y[NUM_PAGES]	=	{ CENTER_Y-150, CENTER_Y-150 };
const float EXPLANATION_X[NUM_PAGES]	=	{ CENTER_X - 150, SCREEN_WIDTH + CENTER_X + 150 };
const float EXPLANATION_Y[NUM_PAGES]	=	{ CENTER_Y - 170, CENTER_Y - 170 };
const float EXPLANATION_OFF_SCREEN_X[NUM_PAGES]	=	{ EXPLANATION_X[0]-400, EXPLANATION_X[1]+400 };
const float EXPLANATION_OFF_SCREEN_Y[NUM_PAGES]	=	{ EXPLANATION_Y[0], EXPLANATION_Y[1] };

const float DIFFICULTY_ITEM_X[NUM_DIFFICULTY_ITEMS] = {
	CENTER_X-200,			// easy
	CENTER_X,				// medium
	CENTER_X+200,			// hard
	SCREEN_WIDTH+CENTER_X,	// Oni, page 2
};
// these sprites are bottom aligned!
const float DIFFICULTY_ITEM_Y[NUM_DIFFICULTY_ITEMS] = {
	CENTER_Y-40,
	CENTER_Y-60,
	CENTER_Y-40,
	CENTER_Y-60,
};

const float DIFFICULTY_ARROW_Y[NUM_DIFFICULTY_ITEMS] = {
	DIFFICULTY_ITEM_Y[0]+205,
	DIFFICULTY_ITEM_Y[1]+205,
	DIFFICULTY_ITEM_Y[2]+205,
	DIFFICULTY_ITEM_Y[3]+205,
};
const float DIFFICULTY_ARROW_X[NUM_DIFFICULTY_ITEMS][NUM_PLAYERS] = {
	{ DIFFICULTY_ITEM_X[0]-40, DIFFICULTY_ITEM_X[0]+40 },
	{ DIFFICULTY_ITEM_X[1]-40, DIFFICULTY_ITEM_X[1]+40 },
	{ DIFFICULTY_ITEM_X[2]-40, DIFFICULTY_ITEM_X[2]+40 },
	{ DIFFICULTY_ITEM_X[3]-40, DIFFICULTY_ITEM_X[3]+40 },
};



const float ARROW_SHADOW_OFFSET = 10;


const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User + 2);
const ScreenMessage SM_StartTweeningOffScreen =	ScreenMessage(SM_User + 3);
const ScreenMessage SM_StartFadingOut		 =	ScreenMessage(SM_User + 4);


ScreenSelectDifficulty::ScreenSelectDifficulty()
{
	LOG->WriteLine( "ScreenSelectDifficulty::ScreenSelectDifficulty()" );

	int p;
	
	m_Menu.Load(
		THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_BACKGROUND) , 
		THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_TOP_EDGE),
		ssprintf("Use %c %c to select, then press START", char(1), char(2))
		);
	this->AddActor( &m_Menu );


	for( int d=0; d<NUM_DIFFICULTY_ITEMS; d++ )
	{
		ThemeElement te_header;
		ThemeElement te_picture;
		switch( d )
		{
		case 0:	te_header = GRAPHIC_SELECT_DIFFICULTY_EASY_HEADER;		te_picture = GRAPHIC_SELECT_DIFFICULTY_EASY_PICTURE;	break;
		case 1: te_header = GRAPHIC_SELECT_DIFFICULTY_MEDIUM_HEADER;	te_picture = GRAPHIC_SELECT_DIFFICULTY_MEDIUM_PICTURE;	break;
		case 2: te_header = GRAPHIC_SELECT_DIFFICULTY_HARD_HEADER;		te_picture = GRAPHIC_SELECT_DIFFICULTY_HARD_PICTURE;	break;
		case 3: te_header = GRAPHIC_SELECT_DIFFICULTY_ONI_HEADER;		te_picture = GRAPHIC_SELECT_DIFFICULTY_ONI_PICTURE;		break;
		case 4: te_header = GRAPHIC_SELECT_DIFFICULTY_ENDLESS_HEADER;	te_picture = GRAPHIC_SELECT_DIFFICULTY_ENDLESS_PICTURE;	break;
		}
		m_sprPicture[d].Load( THEME->GetPathTo(te_picture) );
		m_sprPicture[d].SetXY( DIFFICULTY_ITEM_X[d], DIFFICULTY_ITEM_Y[d] );
		m_sprPicture[d].SetVertAlign( align_bottom );
		m_sprPicture[d].TurnShadowOff();
		m_framePages.AddActor( &m_sprPicture[d] );

		m_sprHeader[d].Load( THEME->GetPathTo(te_header) );
		m_sprHeader[d].SetXY( DIFFICULTY_ITEM_X[d], DIFFICULTY_ITEM_Y[d] );
		m_sprHeader[d].SetVertAlign( align_top );
		m_sprHeader[d].TurnShadowOff();
		m_framePages.AddActor( &m_sprHeader[d] );
	}

	for( p=0; p<NUM_PAGES; p++ )
	{
		m_sprMoreArrows[p].Load( THEME->GetPathTo( p==0 ? GRAPHIC_ARROWS_RIGHT : GRAPHIC_ARROWS_LEFT ) );
		m_sprMoreArrows[p].SetXY( MORE_ARROWS_X[p], MORE_ARROWS_Y[p] );
		m_framePages.AddActor( &m_sprMoreArrows[p] );

		m_sprExplanation[p].Load( THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_EXPLANATION) );
		m_sprExplanation[p].SetXY( EXPLANATION_X[p], EXPLANATION_Y[p] );
		m_sprExplanation[p].StopAnimating();
		m_sprExplanation[p].SetState( p );
		m_framePages.AddActor( &m_sprExplanation[p] );
	}


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_iSelection[p] = 0;
		m_bChosen[p] = false;

		if( !GAMEMAN->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		m_sprArrowShadow[p].Load( THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_ARROWS) );
		m_sprArrowShadow[p].StopAnimating();
		m_sprArrowShadow[p].SetState( p );
		m_sprArrowShadow[p].TurnShadowOff();
		m_sprArrowShadow[p].SetDiffuseColor( D3DXCOLOR(0,0,0,0.6f) );
		m_framePages.AddActor( &m_sprArrowShadow[p] );

		m_sprArrow[p].Load( THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_ARROWS) );
		m_sprArrow[p].StopAnimating();
		m_sprArrow[p].SetState( p );
		m_sprArrow[p].TurnShadowOff();
		m_sprArrow[p].SetDiffuseColor( PlayerToColor((PlayerNumber)p) );
		m_sprArrow[p].SetEffectGlowing();
		m_framePages.AddActor( &m_sprArrow[p] );

		m_sprOK[p].Load( THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_OK) );
		m_framePages.AddActor( &m_sprOK[p] );
	}

	this->AddActor( &m_framePages );
	
	m_soundChange.Load( THEME->GetPathTo(SOUND_SELECT_DIFFICULTY_CHANGE) );
	m_soundSelect.Load( THEME->GetPathTo(SOUND_MENU_START) );

	m_bPlayedChallengeSound = false;
	
	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_DIFFICULTY_INTRO) );


	if( !MUSIC->IsPlaying() )
	{
		MUSIC->Load( THEME->GetPathTo(SOUND_MENU_MUSIC) );
        MUSIC->Play( true );
	}

	m_Menu.TweenOnScreenFromMenu( SM_None );
	TweenOnScreen();
}


ScreenSelectDifficulty::~ScreenSelectDifficulty()
{
	LOG->WriteLine( "ScreenSelectDifficulty::~ScreenSelectDifficulty()" );

}


void ScreenSelectDifficulty::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->WriteLine( "ScreenSelectDifficulty::Input()" );

	if( m_Menu.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelectDifficulty::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectDifficulty::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				MenuStart( (PlayerNumber)p );
		}
		break;
	case SM_GoToPrevState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:
		switch( m_iSelection[PLAYER_1] )
		{
		case 0:
		case 1:
		case 2:	// something on page 1 was chosen
			SCREENMAN->SetNewScreen( new ScreenSelectGroup );
			break;
		case 3:
		case 4:
			PREFSMAN->m_PlayMode = PLAY_MODE_ONI;
			SCREENMAN->SetNewScreen( new ScreenSelectCourse );
			break;
		default:
			ASSERT(0);	// bad selection
		}
		break;
	case SM_StartTweeningOffScreen:
		TweenOffScreen();
		this->SendScreenMessage( SM_StartFadingOut, 0.8f );
		break;
	case SM_StartFadingOut:
		m_Menu.TweenOffScreenToMenu( SM_GoToNextState );
		break;
	}
}

void ScreenSelectDifficulty::MenuLeft( const PlayerNumber p )
{
	if( m_iSelection[p] == 0 )	// can't go left any more
		return;
	if( m_bChosen[p] )
		return;

	ChangeTo( p, m_iSelection[p], m_iSelection[p]-1 );
}


void ScreenSelectDifficulty::MenuRight( const PlayerNumber p )
{
	if( m_iSelection[p] == NUM_DIFFICULTY_ITEMS-1 )	// can't go right any more
		return;
	if( m_bChosen[p] )
		return;

	ChangeTo( p, m_iSelection[p], m_iSelection[p]+1 );
}

void ScreenSelectDifficulty::ChangeTo( const PlayerNumber pn, int iSelectionWas, int iSelectionIs )
{
	bool bChangedPagesFrom1To2 = iSelectionWas < 3  &&  iSelectionIs >= 3;
	bool bChangedPagesFrom2To1 = iSelectionWas >= 3  &&  iSelectionIs < 3;
	bool bChangedPages = bChangedPagesFrom1To2 || bChangedPagesFrom2To1;
	bool bSelectedSomethingOnPage1 = iSelectionIs < 3;
	bool bSelectedSomethingOnPage2 = iSelectionIs >= 3;

	if( bSelectedSomethingOnPage2 )
	{
		// change both players
		for( int p=0; p<NUM_PLAYERS; p++ )
			m_iSelection[p] = iSelectionIs;
	}
	else if( bChangedPagesFrom2To1 )
	{
		// change only the player who pressed the button
		for( int p=0; p<NUM_PLAYERS; p++ )
			m_iSelection[p] = 2;
	}
	else	// moving around in page 1
	{
		// change only the player who pressed the button
		m_iSelection[pn] = iSelectionIs;

	}


	if( !m_bPlayedChallengeSound  &&  bChangedPagesFrom1To2 )
	{
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_DIFFICULTY_CHALLENGE) );
		m_bPlayedChallengeSound = true;
	}

	if( bChangedPagesFrom1To2 || bChangedPagesFrom2To1 )
	{
		m_framePages.BeginTweening( 0.2f );
		m_framePages.SetTweenX( bSelectedSomethingOnPage1 ? 0.0f : -SCREEN_WIDTH );
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( bSelectedSomethingOnPage2 || bChangedPagesFrom2To1 || p==pn )
		{
			m_sprArrow[p].BeginTweening( 0.2f, bChangedPages ? TWEEN_LINEAR : TWEEN_BIAS_BEGIN );
			m_sprArrow[p].SetTweenX( DIFFICULTY_ARROW_X[m_iSelection[p]][p] - ARROW_SHADOW_OFFSET );
			m_sprArrow[p].SetTweenY( DIFFICULTY_ARROW_Y[m_iSelection[p]] - ARROW_SHADOW_OFFSET );

			m_sprArrowShadow[p].BeginTweening( 0.2f, bChangedPages ? TWEEN_LINEAR : TWEEN_BIAS_BEGIN );
			m_sprArrowShadow[p].SetTweenX( DIFFICULTY_ARROW_X[m_iSelection[p]][p] );
			m_sprArrowShadow[p].SetTweenY( DIFFICULTY_ARROW_Y[m_iSelection[p]] );
		}
	}

	m_soundChange.PlayRandom();
}

void ScreenSelectDifficulty::MenuStart( PlayerNumber pn )
{
	if( m_bChosen[pn] == true )
		return;
	m_bChosen[pn] = true;

	m_soundSelect.PlayRandom();
	int iSelection = m_iSelection[pn];

	switch( iSelection )
	{
	case 0:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_DIFFICULTY_COMMENT_EASY) );	break;
	case 1:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_DIFFICULTY_COMMENT_MEDIUM) );	break;
	case 2:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_DIFFICULTY_COMMENT_HARD) );	break;
	case 3:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_DIFFICULTY_COMMENT_ONI) );	break;
	}

	if( iSelection >= 3 )	// chose something on page 2
	{
		// choose this for all the other players too
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( m_bChosen[p] )
				continue;
		
			MenuStart( (PlayerNumber)p );
		}
	}

	m_sprArrow[pn].BeginTweeningQueued( 0.2f );
	m_sprArrow[pn].BeginTweeningQueued( 0.2f );
	m_sprArrow[pn].SetTweenX( DIFFICULTY_ARROW_X[iSelection][pn] );
	m_sprArrow[pn].SetTweenY( DIFFICULTY_ARROW_Y[iSelection] );

	m_sprOK[pn].SetX( DIFFICULTY_ARROW_X[iSelection][pn] );
	m_sprOK[pn].SetY( DIFFICULTY_ARROW_Y[iSelection] );
	m_sprOK[pn].SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	m_sprOK[pn].SetZoom( 2 );

	m_sprOK[pn].BeginTweening( 0.2f );
	m_sprOK[pn].SetTweenZoom( 1 );
	m_sprOK[pn].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );

	m_sprArrowShadow[pn].BeginTweeningQueued( 0.2f );
	m_sprArrowShadow[pn].BeginTweeningQueued( 0.2f );
	m_sprArrowShadow[pn].SetDiffuseColor( D3DXCOLOR(0,0,0,0) );


	// check to see if everyone has chosen
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMEMAN->IsPlayerEnabled((PlayerNumber)p)  &&  m_bChosen[p] == false )
			return;
	}
	this->SendScreenMessage( SM_StartTweeningOffScreen, 0.7f );
}

void ScreenSelectDifficulty::MenuBack( const PlayerNumber p )
{
	m_Menu.TweenOffScreenToBlack( SM_GoToPrevState, true );
}

void ScreenSelectDifficulty::TweenOffScreen()
{
	int p;

	for( p=0; p<NUM_PAGES; p++ )
	{
		m_sprExplanation[p].SetXY( EXPLANATION_X[p], EXPLANATION_Y[p] );
		m_sprExplanation[p].BeginTweening( 0.5, Actor::TWEEN_BOUNCE_BEGIN );
		m_sprExplanation[p].SetTweenXY( EXPLANATION_OFF_SCREEN_X[p], EXPLANATION_OFF_SCREEN_Y[p] );

		m_sprMoreArrows[p].BeginTweening( 0.5 );
		m_sprMoreArrows[p].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		m_sprArrow[p].BeginTweening( 0.3f );
		m_sprArrow[p].SetTweenZoom( 0 );

		m_sprOK[p].BeginTweening( 0.3f );
		m_sprOK[p].SetTweenZoom( 0 );

		m_sprArrowShadow[p].BeginTweeningQueued( 0.3f );
		m_sprArrowShadow[p].SetTweenDiffuseColor( D3DXCOLOR(0,0,0,0) );
	}

	for( int d=0; d<NUM_DIFFICULTY_ITEMS; d++ )
	{
		if( d >= 3 )	// this item is on page 2
			continue;	// don't tween

		const float fPauseTime = d*0.2f;

		// pause
		m_sprHeader[d].BeginTweeningQueued( fPauseTime );

		m_sprPicture[d].BeginTweeningQueued( fPauseTime );

		// roll up
		m_sprHeader[d].BeginTweeningQueued( 0.3f, TWEEN_BOUNCE_BEGIN );

		m_sprPicture[d].BeginTweeningQueued( 0.3f, TWEEN_BOUNCE_BEGIN );
		m_sprPicture[d].SetTweenZoomY( 0 );

		// fly off
		m_sprHeader[d].BeginTweeningQueued( 0.4f, TWEEN_BIAS_END );
		m_sprHeader[d].SetTweenXY( DIFFICULTY_ITEM_X[d]-700, DIFFICULTY_ITEM_Y[d] );

		m_sprPicture[d].BeginTweeningQueued( 0.4f, TWEEN_BIAS_END );
		m_sprPicture[d].SetTweenXY( DIFFICULTY_ITEM_X[d]-700, DIFFICULTY_ITEM_Y[d] );
	}
}

void ScreenSelectDifficulty::TweenOnScreen() 
{
	int p;

	m_Menu.TweenOnScreenFromMenu( SM_None );

	for( p=0; p<NUM_PAGES; p++ )
	{
		m_sprExplanation[p].SetXY( EXPLANATION_OFF_SCREEN_X[p], EXPLANATION_OFF_SCREEN_Y[p] );
		m_sprExplanation[p].BeginTweening( 0.3f, Actor::TWEEN_BOUNCE_END );
		m_sprExplanation[p].SetTweenXY( EXPLANATION_X[p], EXPLANATION_Y[p] );

		m_sprMoreArrows[p].SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
		m_sprMoreArrows[p].BeginTweening( 0.5 );
		m_sprMoreArrows[p].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		int iSelection = m_iSelection[p];

		m_sprArrow[p].SetX( DIFFICULTY_ARROW_X[iSelection][p] );
		m_sprArrow[p].SetY( DIFFICULTY_ARROW_Y[iSelection] );
		m_sprArrow[p].SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
		m_sprArrow[p].SetRotation( D3DX_PI );
		m_sprArrow[p].SetZoom( 2 );
		m_sprArrow[p].BeginTweening( 0.3f );
		m_sprArrow[p].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
		m_sprArrow[p].SetTweenRotationZ( 0 );
		m_sprArrow[p].SetTweenZoom( 1 );

		m_sprArrowShadow[p].SetX( DIFFICULTY_ARROW_X[iSelection][p] );
		m_sprArrowShadow[p].SetY( DIFFICULTY_ARROW_Y[iSelection] );
		D3DXCOLOR colorOriginal = m_sprArrowShadow[p].GetDiffuseColor();
		m_sprArrowShadow[p].SetDiffuseColor( D3DXCOLOR(0,0,0,0) );
		m_sprArrowShadow[p].BeginTweening( 0.3f );
		m_sprArrowShadow[p].SetTweenDiffuseColor( colorOriginal );
	}

	for( int d=0; d<NUM_DIFFICULTY_ITEMS; d++ )
	{
		const float fPauseTime = d*0.2f;

		bool bIsOnPage1 = m_iSelection[PLAYER_1] >= 3;

		if( bIsOnPage1  &&  d >= 3 )	// they are on page 1, but this item is on page 2
			continue;	// don't tween

		// set off screen
		m_sprHeader[d].SetXY( DIFFICULTY_ITEM_X[d]-700, DIFFICULTY_ITEM_Y[d] );
	
		m_sprPicture[d].SetXY( DIFFICULTY_ITEM_X[d]-700, DIFFICULTY_ITEM_Y[d] );
		m_sprPicture[d].SetZoomY( 0 );

		// pause
		m_sprHeader[d].BeginTweeningQueued( fPauseTime );

		m_sprPicture[d].BeginTweeningQueued( fPauseTime );

		// fly on
		m_sprHeader[d].BeginTweeningQueued( 0.5f, TWEEN_BIAS_BEGIN );
		m_sprHeader[d].SetTweenXY( DIFFICULTY_ITEM_X[d], DIFFICULTY_ITEM_Y[d] );

		m_sprPicture[d].BeginTweeningQueued( 0.5f, TWEEN_BIAS_BEGIN );
		m_sprPicture[d].SetTweenXY( DIFFICULTY_ITEM_X[d], DIFFICULTY_ITEM_Y[d] );

		// roll down
		m_sprHeader[d].BeginTweeningQueued( 0.3f, TWEEN_BOUNCE_END );

		m_sprPicture[d].BeginTweeningQueued( 0.3f, TWEEN_BOUNCE_END );
		m_sprPicture[d].SetTweenZoomY( 1 );

	}
}
