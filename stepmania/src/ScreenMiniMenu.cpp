#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenMiniMenu

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenMiniMenu.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "RageSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"


const float LABEL_X		=	200;
const float ANSWER_X	=	440;
const float SPACING_Y	=	26;

const ScreenMessage SM_GoToOK		= (ScreenMessage)(SM_User+1);
const ScreenMessage SM_GoToCancel	= (ScreenMessage)(SM_User+2);


int	ScreenMiniMenu::s_iLastLine;
int	ScreenMiniMenu::s_iLastAnswers[MAX_MINI_MENU_LINES];


ScreenMiniMenu::ScreenMiniMenu( MiniMenuDefinition* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel )
{
	m_SMSendOnOK = SM_SendOnOK;
	m_SMSendOnCancel = SM_SendOnCancel;
	m_Def = *pDef;
	ASSERT( m_Def.iNumLines <= MAX_MINI_MENU_LINES );


	m_Fade.SetTransitionTime( 0.5f );
	m_Fade.SetDiffuse( RageColor(0,0,0,0.7f) );
	m_Fade.SetOpened();
	m_Fade.CloseWipingRight();
	this->AddChild( &m_Fade );

	
	float fHeightOfAll = (m_Def.iNumLines-1)*SPACING_Y;

	m_textTitle.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textTitle.SetText( m_Def.szTitle );
	m_textTitle.SetXY( CENTER_X, CENTER_Y - fHeightOfAll/2 - 30 );
	m_textTitle.SetZoom( 0.8f );
	this->AddChild( &m_textTitle );

	bool bMarkedFirstEnabledLine = false;
	
	for( int i=0; i<m_Def.iNumLines; i++ )
	{
		MiniMenuDefinition::MiniMenuLine& line = m_Def.lines[i];
		m_iCurAnswers[i] = 0;

		float fY = SCALE( i, 0.f, m_Def.iNumLines-1.f, CENTER_Y-fHeightOfAll/2, CENTER_Y+fHeightOfAll/2 );

		m_textLabel[i].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_textLabel[i].SetText( line.szLabel );
		m_textLabel[i].SetXY( LABEL_X, fY );
		m_textLabel[i].SetZoom( 0.5f );
		m_textLabel[i].SetHorizAlign( Actor::align_left );
		m_textLabel[i].SetDiffuse( line.bEnabled ? RageColor(1,1,1,1) : RageColor(0.5f,0.5f,0.5f,1) );
		this->AddChild( &m_textLabel[i] );

		m_textAnswer[i].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
 		m_textAnswer[i].SetText( line.szOptionsText[0] );
		m_textAnswer[i].SetXY( ANSWER_X, fY );
		m_textAnswer[i].SetZoom( 0.5f );
		m_textAnswer[i].SetHorizAlign( Actor::align_right );
		m_textAnswer[i].SetDiffuse( line.bEnabled ? RageColor(1,1,1,1) : RageColor(0.5f,0.5f,0.5f,1) );
		this->AddChild( &m_textAnswer[i] );

		if( !bMarkedFirstEnabledLine && line.bEnabled )
		{
			m_iCurLine = i;
			AfterLineChanged();
			bMarkedFirstEnabledLine = true;
		}

		m_iCurAnswers[i] = line.iDefaultOption;
	}

	SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","menu prompt") );
}

void ScreenMiniMenu::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}

void ScreenMiniMenu::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenMiniMenu::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( m_Fade.IsOpening() )
		return;

	if( DeviceI.device==DEVICE_KEYBOARD && type==IET_FIRST_PRESS )
	{
		switch( DeviceI.button )
		{
		case SDLK_LEFT:
			this->MenuLeft( StyleI.player );
			return;
		case SDLK_RIGHT:
			this->MenuRight( StyleI.player );
			return;
		}
	}

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenMiniMenu::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToOK:
		SCREENMAN->PopTopScreen( m_SMSendOnOK );
		break;
	case SM_GoToCancel:
		SCREENMAN->PopTopScreen( m_SMSendOnCancel );
		break;
	}
}

void ScreenMiniMenu::MenuUp( PlayerNumber pn )
{
	if( GetGoUpSpot() != -1 )
	{
		BeforeLineChanged();
		m_iCurLine = GetGoUpSpot();
		AfterLineChanged();
	}
}

void ScreenMiniMenu::MenuDown( PlayerNumber pn )
{
	if( GetGoDownSpot() != -1 )
	{
		BeforeLineChanged();
		m_iCurLine = GetGoDownSpot();
		AfterLineChanged();
	}
}

void ScreenMiniMenu::MenuLeft( PlayerNumber pn )
{
	if( CanGoLeft() )
	{
		m_iCurAnswers[m_iCurLine]--;
		AfterAnswerChanged();
	}
}

void ScreenMiniMenu::MenuRight( PlayerNumber pn )
{
	if( CanGoRight() )
	{
		m_iCurAnswers[m_iCurLine]++;
		AfterAnswerChanged();
	}
}

void ScreenMiniMenu::MenuStart( PlayerNumber pn )
{
	m_Fade.OpenWipingRight( SM_GoToOK );

	SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","menu start") );

	s_iLastLine = m_iCurLine;
	COPY( s_iLastAnswers, m_iCurAnswers );
}

void ScreenMiniMenu::MenuBack( PlayerNumber pn )
{
	m_Fade.OpenWipingRight( SM_GoToCancel );
}

void ScreenMiniMenu::BeforeLineChanged()
{
	m_textLabel[m_iCurLine].SetEffectNone();
	m_textAnswer[m_iCurLine].SetEffectNone();
	m_textLabel[m_iCurLine].SetZoom( 0.5f );
	m_textAnswer[m_iCurLine].SetZoom( 0.5f );
}

void ScreenMiniMenu::AfterLineChanged()
{
	m_textLabel[m_iCurLine].SetEffectGlowShift( 1.0f, RageColor(0,0.5f,0,1), RageColor(0,1,0,1) );
	m_textAnswer[m_iCurLine].SetEffectGlowShift( 1.0f, RageColor(0,0.5f,0,1), RageColor(0,1,0,1) );
	m_textLabel[m_iCurLine].SetZoom( 0.7f );
	m_textAnswer[m_iCurLine].SetZoom( 0.7f );
	SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","mini menu row") );
}

void ScreenMiniMenu::AfterAnswerChanged()
{
	SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","mini menu row") );
	int iAnswerInRow = m_iCurAnswers[m_iCurLine];
	CString sAnswerText = m_Def.lines[m_iCurLine].szOptionsText[iAnswerInRow];
	m_textAnswer[m_iCurLine].SetText( sAnswerText );
}

int ScreenMiniMenu::GetGoUpSpot()
{
	for( int i=m_iCurLine-1; i>=0; i-- )
		if( m_Def.lines[i].bEnabled )
			return i;
	return -1;
}

int ScreenMiniMenu::GetGoDownSpot()
{
	for( int i=m_iCurLine+1; i<m_Def.iNumLines; i++ )
		if( m_Def.lines[i].bEnabled )
			return i;
	return -1;
}

bool ScreenMiniMenu::CanGoLeft()
{
	return m_iCurAnswers[m_iCurLine] != 0;
}

bool ScreenMiniMenu::CanGoRight()
{
	int iNumInCurRow = m_Def.lines[m_iCurLine].iNumOptions;
	return m_iCurAnswers[m_iCurLine] != iNumInCurRow-1;
}
