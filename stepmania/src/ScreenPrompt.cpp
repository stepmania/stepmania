#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenPrompt

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenPrompt.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "RageSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"

const float QUESTION_X	=	CENTER_X;
const float QUESTION_Y	=	CENTER_Y - 60;

const float PROMPT_X	=	CENTER_X;
const float PROMPT_Y	=	CENTER_Y + 120;


ScreenPrompt::ScreenPrompt( ScreenMessage SM_SendWhenDone, CString sText, bool bYesNoPrompt, bool bDefaultAnswer, void(*OnYes)(), void(*OnNo)() ) :
  Screen("ScreenPrompt")
{
	m_bIsTransparent = true;	// draw screens below us

	m_SMSendWhenDone = SM_SendWhenDone;
	m_bYesNoPrompt = bYesNoPrompt;
	m_bAnswer = bDefaultAnswer;
	m_pOnYes = OnYes;
	m_pOnNo = OnNo;


	m_Background.LoadFromAniDir( THEME->GetPathToB("ScreenPrompt background") );
	this->AddChild( &m_Background );

	m_textQuestion.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textQuestion.SetText( sText );
	m_textQuestion.SetXY( QUESTION_X, QUESTION_Y );
	this->AddChild( &m_textQuestion );

	m_rectAnswerBox.SetDiffuse( RageColor(0.5f,0.5f,1.0f,0.7f) );
	this->AddChild( &m_rectAnswerBox );

	m_textAnswer[0].LoadFromFont( THEME->GetPathToF("_shared1") );
	m_textAnswer[1].LoadFromFont( THEME->GetPathToF("_shared1") );
	m_textAnswer[0].SetY( PROMPT_Y );
	m_textAnswer[1].SetY( PROMPT_Y );
	this->AddChild( &m_textAnswer[0] );
	this->AddChild( &m_textAnswer[1] );

	

	if( m_bYesNoPrompt )
	{
		m_textAnswer[0].SetText( "NO" );
		m_textAnswer[1].SetText( "YES" );
		m_textAnswer[0].SetX( PROMPT_X+50 );
		m_textAnswer[1].SetX( PROMPT_X-50 );
	}
	else
	{
		m_textAnswer[0].SetText( "OK" );
		m_textAnswer[0].SetX( PROMPT_X );
	}

	m_rectAnswerBox.SetXY( m_textAnswer[m_bAnswer].GetX(), m_textAnswer[m_bAnswer].GetY() );
	m_rectAnswerBox.SetZoomX( m_textAnswer[m_bAnswer].GetWidestLineWidthInSourcePixels()+10.0f );
	m_rectAnswerBox.SetZoomY( 30 );

	m_textAnswer[m_bAnswer].SetEffectGlowShift();

	m_In.Load( THEME->GetPathToB("ScreenPrompt in") );
	m_In.StartTransitioning();
	this->AddChild( &m_In );
	
	m_Out.Load( THEME->GetPathToB("ScreenPrompt out") );
	this->AddChild( &m_Out );
}

void ScreenPrompt::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}

void ScreenPrompt::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenPrompt::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
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

void ScreenPrompt::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_DoneClosingWipingLeft:
		break;
	case SM_DoneClosingWipingRight:
		break;
	case SM_DoneOpeningWipingLeft:
		break;
	case SM_DoneOpeningWipingRight:
		SCREENMAN->PopTopScreen( m_SMSendWhenDone );
		break;
	}
}

void ScreenPrompt::MenuLeft( PlayerNumber pn )
{
	if( !m_bYesNoPrompt )
		return;

	MenuRight( pn );
}

void ScreenPrompt::MenuRight( PlayerNumber pn )
{
	if( !m_bYesNoPrompt )
		return;

	m_textAnswer[m_bAnswer].SetEffectNone();
	m_bAnswer = !m_bAnswer;
	m_textAnswer[m_bAnswer].SetEffectGlowShift();

	m_rectAnswerBox.StopTweening();
	m_rectAnswerBox.BeginTweening( 0.2f );
	m_rectAnswerBox.SetXY( m_textAnswer[m_bAnswer].GetX(), m_textAnswer[m_bAnswer].GetY() );
	m_rectAnswerBox.SetZoomX( m_textAnswer[m_bAnswer].GetWidestLineWidthInSourcePixels()+10.0f );

	SOUNDMAN->PlayOnce( THEME->GetPathToS("ScreenPrompt change") );
}

void ScreenPrompt::MenuStart( PlayerNumber pn )
{
	m_Out.StartTransitioning( SM_DoneOpeningWipingRight );

	m_textQuestion.BeginTweening( 0.2f );
	m_textQuestion.SetDiffuse( RageColor(1,1,1,0) );

	m_rectAnswerBox.BeginTweening( 0.2f );
	m_rectAnswerBox.SetDiffuse( RageColor(1,1,1,0) );

	m_textAnswer[m_bAnswer].SetEffectNone();

	m_textAnswer[0].BeginTweening( 0.2f );
	m_textAnswer[0].SetDiffuse( RageColor(1,1,1,0) );
	m_textAnswer[1].BeginTweening( 0.2f );
	m_textAnswer[1].SetDiffuse( RageColor(1,1,1,0) );

	SOUNDMAN->PlayOnce( THEME->GetPathToS("Common start") );

	if( m_bAnswer )
	{
		if( m_pOnYes )
			m_pOnYes();
	} else {
		if( m_pOnNo )
			m_pOnNo();
	}
}

void ScreenPrompt::MenuBack( PlayerNumber pn )
{

}
