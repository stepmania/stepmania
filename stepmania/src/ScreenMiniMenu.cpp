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
#include "RageSounds.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"


const float LABEL_X		=	200;
const float ANSWER_X	=	440;
const float SPACING_Y	=	24;

const float ZOOM_SELECTED = 0.7f;
const float ZOOM_NOT_SELECTED = 0.5f;

const RageColor COLOR_ENABLED = RageColor(1,1,1,1);
const RageColor COLOR_DISABLED = RageColor(0.5f,0.5f,0.5f,1);

const ScreenMessage SM_GoToOK		= (ScreenMessage)(SM_User+1);
const ScreenMessage SM_GoToCancel	= (ScreenMessage)(SM_User+2);


int	ScreenMiniMenu::s_iLastLine;
int	ScreenMiniMenu::s_iLastAnswers[MAX_MENU_ROWS];


ScreenMiniMenu::ScreenMiniMenu( Menu* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel ) :
  Screen("ScreenMiniMenu")
{
	m_bIsTransparent = true;	// draw screens below us

	m_SMSendOnOK = SM_SendOnOK;
	m_SMSendOnCancel = SM_SendOnCancel;
	m_Def = *pDef;
	ASSERT( m_Def.rows.size() <= MAX_MENU_ROWS );


	m_Background.LoadFromAniDir( THEME->GetPathToB("ScreenMiniMenu background") );
	this->AddChild( &m_Background );

	
	float fHeightOfAll = (m_Def.rows.size()-1)*SPACING_Y;

	m_textTitle.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textTitle.SetText( m_Def.title );
	m_textTitle.SetXY( CENTER_X, CENTER_Y - fHeightOfAll/2 - 30 );
	m_textTitle.SetZoom( 0.8f );
	this->AddChild( &m_textTitle );

	bool bMarkedFirstEnabledLine = false;
	m_iCurLine = 0;

	float fLongestLabelPlusAnswer = 0;

	for( unsigned i=0; i<m_Def.rows.size(); i++ )
	{
		MenuRow& line = m_Def.rows[i];
		m_iCurAnswers[i] = 0;

		float fY = SCALE( i, 0.f, m_Def.rows.size()-1.f, CENTER_Y-fHeightOfAll/2, CENTER_Y+fHeightOfAll/2 );

		m_textLabel[i].LoadFromFont( THEME->GetPathToF("Common normal") );
		m_textLabel[i].SetText( line.name );
		m_textLabel[i].SetY( fY );
		m_textLabel[i].SetZoom( ZOOM_NOT_SELECTED );
		m_textLabel[i].SetHorizAlign( Actor::align_left );
		m_textLabel[i].SetDiffuse( line.enabled ? COLOR_ENABLED : COLOR_DISABLED );
		this->AddChild( &m_textLabel[i] );

		CString sText = line.choices.empty() ? "" : line.choices[line.defaultChoice];
		m_textAnswer[i].LoadFromFont( THEME->GetPathToF("Common normal") );
 		m_textAnswer[i].SetText( sText );
		m_textAnswer[i].SetY( fY );
		m_textAnswer[i].SetZoom( ZOOM_NOT_SELECTED );
		m_textAnswer[i].SetHorizAlign( Actor::align_right );
		m_textAnswer[i].SetDiffuse( line.enabled ? COLOR_ENABLED : COLOR_DISABLED );
		this->AddChild( &m_textAnswer[i] );

		fLongestLabelPlusAnswer = max( 
			fLongestLabelPlusAnswer, 
			m_textLabel[i].GetWidestLineWidthInSourcePixels() * ZOOM_SELECTED +
			m_textAnswer[i].GetWidestLineWidthInSourcePixels() * ZOOM_SELECTED );

		if( !bMarkedFirstEnabledLine && line.enabled )
		{
			m_iCurLine = i;
			AfterLineChanged();
			bMarkedFirstEnabledLine = true;
		}

		m_iCurAnswers[i] = line.defaultChoice;
	}

	// adjust text spacing based on widest line
	float fLabelX = LABEL_X;
	float fAnswerX = ANSWER_X;
	float fDefaultWidth = ANSWER_X - LABEL_X;
	if( fLongestLabelPlusAnswer+20 > fDefaultWidth )
	{
		float fIncreaseBy = fLongestLabelPlusAnswer - fDefaultWidth + 20;
		fLabelX -= fIncreaseBy/2;
		fAnswerX += fIncreaseBy/2;
	}

	for( unsigned k=0; k<m_Def.rows.size(); k++ )
	{
		m_textLabel[k].SetX( fLabelX );
		m_textAnswer[k].SetX( fAnswerX );
	}

	m_In.Load( THEME->GetPathToB("ScreenMiniMenu in") );
	m_In.StartTransitioning();
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathToB("ScreenMiniMenu out") );
	this->AddChild( &m_Out );
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
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
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
	m_Out.StartTransitioning( SM_GoToOK );

	SOUND->PlayOnce( THEME->GetPathToS("Common start") );

	s_iLastLine = m_iCurLine;
	COPY( s_iLastAnswers, m_iCurAnswers );
}

void ScreenMiniMenu::MenuBack( PlayerNumber pn )
{
	m_Out.StartTransitioning( SM_GoToCancel );
}

void ScreenMiniMenu::BeforeLineChanged()
{
	m_textLabel[m_iCurLine].SetEffectNone();
	m_textAnswer[m_iCurLine].SetEffectNone();
	m_textLabel[m_iCurLine].SetZoom( ZOOM_NOT_SELECTED );
	m_textAnswer[m_iCurLine].SetZoom( ZOOM_NOT_SELECTED );
	SOUND->PlayOnce( THEME->GetPathToS("ScreenMiniMenu row") );
}

void ScreenMiniMenu::AfterLineChanged()
{
	m_textLabel[m_iCurLine].SetEffectGlowShift( 1.0f, RageColor(0,0.5f,0,1), RageColor(0,1,0,1) );
	m_textAnswer[m_iCurLine].SetEffectGlowShift( 1.0f, RageColor(0,0.5f,0,1), RageColor(0,1,0,1) );
	m_textLabel[m_iCurLine].SetZoom( ZOOM_SELECTED );
	m_textAnswer[m_iCurLine].SetZoom( ZOOM_SELECTED );
}

void ScreenMiniMenu::AfterAnswerChanged()
{
	SOUND->PlayOnce( THEME->GetPathToS("ScreenMiniMenu row") );
	int iAnswerInRow = m_iCurAnswers[m_iCurLine];
	CString sAnswerText = m_Def.rows[m_iCurLine].choices[iAnswerInRow];
	m_textAnswer[m_iCurLine].SetText( sAnswerText );
}

int ScreenMiniMenu::GetGoUpSpot()
{
	int i;
	for( i=m_iCurLine-1; i>=0; i-- )
		if( m_Def.rows[i].enabled )
			return i;
	// wrap
	for( i=m_Def.rows.size()-1; i>=0; i-- )
		if( m_Def.rows[i].enabled )
			return i;
	return -1;
}

int ScreenMiniMenu::GetGoDownSpot()
{
	unsigned i;
	for( i=m_iCurLine+1; i<m_Def.rows.size(); i++ )
		if( m_Def.rows[i].enabled )
			return i;
	// wrap
	for( i=0; i<m_Def.rows.size(); i++ )
		if( m_Def.rows[i].enabled )
			return i;
	return -1;
}

bool ScreenMiniMenu::CanGoLeft()
{
	int iNumInCurRow = m_Def.rows[m_iCurLine].choices.size();
	if( iNumInCurRow==0 )
		return false;
	else
		return m_iCurAnswers[m_iCurLine] != 0;
}

bool ScreenMiniMenu::CanGoRight()
{
	int iNumInCurRow = m_Def.rows[m_iCurLine].choices.size();
	if( iNumInCurRow==0 )
		return false;
	else
		return m_iCurAnswers[m_iCurLine] != iNumInCurRow-1;
}
