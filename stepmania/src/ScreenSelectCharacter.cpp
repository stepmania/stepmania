#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectCharacter

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelectCharacter.h"
#include "ScreenManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "Character.h"


#define TITLE_ON_COMMAND( p )				THEME->GetMetric ("ScreenSelectCharacter",ssprintf("TitleP%dOnCommand",p+1))
#define TITLE_OFF_COMMAND( p )				THEME->GetMetric ("ScreenSelectCharacter",ssprintf("TitleP%dOffCommand",p+1))
#define CHARACTER_ON_COMMAND( p )			THEME->GetMetric ("ScreenSelectCharacter",ssprintf("CharacterP%dOnCommand",p+1))
#define CHARACTER_OFF_COMMAND( p )			THEME->GetMetric ("ScreenSelectCharacter",ssprintf("CharacterP%dOffCommand",p+1))
#define CHARACTER_ARROWS_ON_COMMAND( p )	THEME->GetMetric ("ScreenSelectCharacter",ssprintf("CharacterArrowsP%dOnCommand",p+1))
#define CHARACTER_ARROWS_OFF_COMMAND( p )	THEME->GetMetric ("ScreenSelectCharacter",ssprintf("CharacterArrowsP%dOffCommand",p+1))
#define EXPLANATION_ON_COMMAND				THEME->GetMetric ("ScreenSelectCharacter","ExplanationOnCommand")
#define EXPLANATION_OFF_COMMAND				THEME->GetMetric ("ScreenSelectCharacter","ExplanationOffCommand")
#define ATTACK_FRAME_ON_COMMAND( p )		THEME->GetMetric ("ScreenSelectCharacter",ssprintf("AttackFrameP%dOnCommand",p+1))
#define ATTACK_FRAME_OFF_COMMAND( p )		THEME->GetMetric ("ScreenSelectCharacter",ssprintf("AttackFrameP%dOffCommand",p+1))
#define ICONS_START_X( p )					THEME->GetMetricF("ScreenSelectCharacter",ssprintf("IconsP%dStartX",p+1))
#define ICONS_START_Y( p )					THEME->GetMetricF("ScreenSelectCharacter",ssprintf("IconsP%dStartY",p+1))
#define ICONS_SPACING_X						THEME->GetMetricF("ScreenSelectCharacter","IconsSpacingX")
#define ICONS_SPACING_Y						THEME->GetMetricF("ScreenSelectCharacter","IconsSpacingY")
#define HELP_TEXT							THEME->GetMetric ("ScreenSelectCharacter","HelpText")
#define TIMER_SECONDS						THEME->GetMetricI("ScreenSelectCharacter","TimerSeconds")
#define NEXT_SCREEN							THEME->GetMetric ("ScreenSelectCharacter","NextScreen")

#define LEVEL_CURSOR_X( p, l )	( ICONS_START_X(p)+ICONS_SPACING_X*((NUM_ATTACKS_PER_LEVEL-1)/2.f) )
#define LEVEL_CURSOR_Y( p, l )	( ICONS_START_Y(p)+ICONS_SPACING_Y*l )

const PlayerNumber	CPU_PLAYER[NUM_PLAYERS] = { PLAYER_2, PLAYER_1 };


ScreenSelectCharacter::ScreenSelectCharacter() : Screen("ScreenSelectCharacter")
{	
	LOG->Trace( "ScreenSelectCharacter::ScreenSelectCharacter()" );	

	if( GAMESTATE->m_pCharacters.empty() )
	{
		HandleScreenMessage( SM_GoToNextScreen );
		return;
	}
	
	
	m_Menu.Load( "ScreenSelectCharacter" );
	this->AddChild( &m_Menu );


	int p;


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_iSelectedCharacter[p] = 0;
		if( GAMESTATE->IsHumanPlayer(p) )
			m_SelectionRow[p] = CHOOSING_HUMAN_CHARACTER;
	}


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprTitle[p].Load( THEME->GetPathToG("ScreenSelectCharacter title 2x2") );
		m_sprTitle[p].SetState( GAMESTATE->IsHumanPlayer(p) ? p : 2+p );
		m_sprTitle[p].StopAnimating();
		m_sprTitle[p].Command( TITLE_ON_COMMAND(p) );

		this->AddChild( &m_sprTitle[p] );

		m_sprCharacter[p].Command( CHARACTER_ON_COMMAND(p) );
		this->AddChild( &m_sprCharacter[p] );

		m_sprCharacterArrows[p].Load( THEME->GetPathToG("ScreenSelectCharacter character arrows") );
		m_sprCharacterArrows[p].Command( CHARACTER_ARROWS_ON_COMMAND(p) );
		this->AddChild( &m_sprCharacterArrows[p] );

		m_sprAttackFrame[p].Load( THEME->GetPathToG("ScreenSelectCharacter attack frame 1x2") );
		m_sprAttackFrame[p].StopAnimating();
		m_sprAttackFrame[p].SetState( p );
		m_sprAttackFrame[p].Command( ATTACK_FRAME_ON_COMMAND(p) );
		this->AddChild( &m_sprAttackFrame[p] );

		for( int i=0; i<NUM_ATTACK_LEVELS; i++ )
			for( int j=0; j<NUM_ATTACKS_PER_LEVEL; j++ )
			{
				float fX = ICONS_START_X(p) + ICONS_SPACING_X*j; 
				float fY = ICONS_START_Y(p) + ICONS_SPACING_Y*i; 
				m_AttackIcons[p][i][j].SetXY( fX, fY );
				this->AddChild( &m_AttackIcons[p][i][j] );
			}
	}

	m_sprExplanation.Load( THEME->GetPathToG("ScreenSelectCharacter explanation") );
	m_sprExplanation.Command( EXPLANATION_ON_COMMAND );
	this->AddChild( &m_sprExplanation );


	m_soundChange.Load( THEME->GetPathToS("ScreenSelectCharacter change") );
	m_soundSelect.Load( THEME->GetPathToS("Common start") );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select group intro") );

	SOUNDMAN->PlayMusic( THEME->GetPathToS("ScreenSelectCharacter music") );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsHumanPlayer(p) )
		{
			AfterRowChange( (PlayerNumber)p );
			AfterValueChange( (PlayerNumber)p );
		}
	}
	TweenOnScreen();
}


ScreenSelectCharacter::~ScreenSelectCharacter()
{
	LOG->Trace( "ScreenSelectCharacter::~ScreenSelectCharacter()" );

}


void ScreenSelectCharacter::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectCharacter::Input()" );

	if( m_Menu.IsTransitioning() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelectCharacter::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectCharacter::HandleScreenMessage( const ScreenMessage SM )
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

PlayerNumber ScreenSelectCharacter::GetAffectedPlayerNumber( PlayerNumber pn )
{
	switch( m_SelectionRow[pn] )
	{
	case CHOOSING_HUMAN_CHARACTER:
		return pn;
		break;
	case CHOOSING_CPU_CHARACTER:
		return CPU_PLAYER[pn];
		break;
	default:
		ASSERT(0);
	case FINISHED_CHOOSING:
		return pn;
	}
}

void ScreenSelectCharacter::BeforeRowChange( PlayerNumber pn )
{
	PlayerNumber pnAffected = GetAffectedPlayerNumber(pn);
	switch( m_SelectionRow[pn] )
	{
	case CHOOSING_CPU_CHARACTER:
	case CHOOSING_HUMAN_CHARACTER:
		m_sprCharacterArrows[pnAffected].SetEffectNone();
		break;
	}
}

void ScreenSelectCharacter::AfterRowChange( PlayerNumber pn )
{
	PlayerNumber pnAffected = GetAffectedPlayerNumber(pn);
	switch( m_SelectionRow[pn] )
	{
	case CHOOSING_CPU_CHARACTER:
	case CHOOSING_HUMAN_CHARACTER:
		m_sprCharacterArrows[pnAffected].SetEffectGlowShift();
		break;
	}
}

void ScreenSelectCharacter::AfterValueChange( PlayerNumber pn )
{
	PlayerNumber pnAffected = GetAffectedPlayerNumber(pn);
	switch( m_SelectionRow[pn] )
	{
	case CHOOSING_CPU_CHARACTER:
	case CHOOSING_HUMAN_CHARACTER:
		{
			Character* pChar = GAMESTATE->m_pCharacters[ m_iSelectedCharacter[pnAffected] ];
			m_sprCharacter[pnAffected].UnloadTexture();
			m_sprCharacter[pnAffected].Load( pChar->GetCardPath() );

			for( int i=0; i<NUM_ATTACK_LEVELS; i++ )
				for( int j=0; j<NUM_ATTACKS_PER_LEVEL; j++ )
					m_AttackIcons[pnAffected][i][j].Load( pnAffected, pChar->m_sAttacks[i][j] );
		}
		break;
	case FINISHED_CHOOSING:
		;	// do nothing
		break;
	default:
		ASSERT(0);
	}
}


void ScreenSelectCharacter::MenuLeft( PlayerNumber pn )
{
	MenuUp( pn );
}

void ScreenSelectCharacter::MenuRight( PlayerNumber pn )
{
	MenuDown( pn );
}

void ScreenSelectCharacter::MenuUp( PlayerNumber pn )
{
	Move( pn, -1 );
}


void ScreenSelectCharacter::MenuDown( PlayerNumber pn )
{
	Move( pn, +1 );
}

void ScreenSelectCharacter::Move( PlayerNumber pn, int deltaValue )
{
	PlayerNumber pnAffected = GetAffectedPlayerNumber(pn);
	switch( m_SelectionRow[pn] )
	{
	case CHOOSING_CPU_CHARACTER:
	case CHOOSING_HUMAN_CHARACTER:
		m_iSelectedCharacter[pnAffected] = (m_iSelectedCharacter[pnAffected]+deltaValue)+GAMESTATE->m_pCharacters.size();
		m_iSelectedCharacter[pnAffected] %= GAMESTATE->m_pCharacters.size();
		AfterValueChange(pn);
		m_soundChange.PlayRandom();
		break;
	}
}

void ScreenSelectCharacter::MenuStart( PlayerNumber pn )
{
	if( m_SelectionRow[pn] == FINISHED_CHOOSING )
		return;


	// change row
	BeforeRowChange(pn);
	switch( m_SelectionRow[pn] )
	{
	case CHOOSING_HUMAN_CHARACTER:
		m_SelectionRow[pn] = GAMESTATE->AnyPlayersAreCpu() ? CHOOSING_CPU_CHARACTER : FINISHED_CHOOSING;
		break;
	case CHOOSING_CPU_CHARACTER:
		m_SelectionRow[pn] = FINISHED_CHOOSING;
		break;
	}
	AfterRowChange(pn);
	AfterValueChange(pn);
	m_soundSelect.PlayRandom();

	bool bAllAreFinished = true;
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsHumanPlayer(p) )
			bAllAreFinished &= (m_SelectionRow[p] == FINISHED_CHOOSING);

	if( bAllAreFinished )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			Character* pChar = GAMESTATE->m_pCharacters[ m_iSelectedCharacter[p] ];
			GAMESTATE->m_pCurCharacters[p] = pChar;
		}

		m_Menu.m_MenuTimer.Stop();
		TweenOffScreen();
		this->PostScreenMessage( SM_BeginFadingOut, 0 );
	}
}

void ScreenSelectCharacter::MenuBack( PlayerNumber pn )
{
	m_Menu.Back( SM_GoToPrevScreen );
}

void ScreenSelectCharacter::TweenOffScreen()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprCharacter[p].Command( CHARACTER_OFF_COMMAND(p) );
	}
	m_sprExplanation.Command( EXPLANATION_OFF_COMMAND );
}

void ScreenSelectCharacter::TweenOnScreen() 
{
}
