#include "global.h"
#include "ScreenPrompt.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "Style.h"
#include "ThemeManager.h"
#include "ScreenDimensions.h"

#define QUESTION_X	(SCREEN_CENTER_X)
#define QUESTION_Y	(SCREEN_CENTER_Y - 60)

#define PROMPT_X	(SCREEN_CENTER_X)
#define PROMPT_Y	(SCREEN_CENTER_Y + 120)

PromptAnswer ScreenPrompt::s_LastAnswer = ANSWER_YES;

//REGISTER_SCREEN_CLASS( ScreenPrompt );
ScreenPrompt::ScreenPrompt( 
	CString sText, 
	PromptType type, 
	PromptAnswer defaultAnswer, 
	void(*OnYes)(void*), 
	void(*OnNo)(void*), 
	void* pCallbackData 
	) :
  Screen("ScreenPrompt")
{
	m_bIsTransparent = true;	// draw screens below us

	m_PromptType = type;
	m_Answer = defaultAnswer;
	m_pOnYes = OnYes;
	m_pOnNo = OnNo;
	m_pCallbackData = pCallbackData;
	m_sText = sText;
}

void ScreenPrompt::Init()
{
	Screen::Init();

	m_Background.LoadFromAniDir( THEME->GetPathB("ScreenPrompt","background") );
	m_Background.PlayCommand("On");
	this->AddChild( &m_Background );

	m_textQuestion.LoadFromFont( THEME->GetPathF("ScreenPrompt","question") );
	m_textQuestion.SetText( m_sText );
	m_textQuestion.SetXY( QUESTION_X, QUESTION_Y );
	this->AddChild( &m_textQuestion );

	m_rectAnswerBox.SetDiffuse( RageColor(0.5f,0.5f,1.0f,0.7f) );
	this->AddChild( &m_rectAnswerBox );

	for( int i=0; i<NUM_PROMPT_ANSWERS; i++ )
	{
		m_textAnswer[i].LoadFromFont( THEME->GetPathF("ScreenPrompt","answer") );
		m_textAnswer[i].SetY( PROMPT_Y );
		this->AddChild( &m_textAnswer[i] );
	}
	
	switch( m_PromptType )
	{
	case PROMPT_OK:
		m_textAnswer[ANSWER_YES].	SetText( "OK" );
		m_textAnswer[ANSWER_YES].	SetX( PROMPT_X );
		break;
	case PROMPT_YES_NO:
		m_textAnswer[ANSWER_YES].	SetText( "YES" );
		m_textAnswer[ANSWER_NO].	SetText( "NO" );
		m_textAnswer[ANSWER_YES].	SetX( PROMPT_X-50 );
		m_textAnswer[ANSWER_NO].	SetX( PROMPT_X+50 );
		break;
	case PROMPT_YES_NO_CANCEL:
		m_textAnswer[ANSWER_YES].	SetText( "YES" );
		m_textAnswer[ANSWER_NO].	SetText( "NO" );
		m_textAnswer[ANSWER_CANCEL].SetText( "CANCEL" );
		m_textAnswer[ANSWER_YES].	SetX( PROMPT_X-120 );
		m_textAnswer[ANSWER_NO].	SetX( PROMPT_X-20 );
		m_textAnswer[ANSWER_CANCEL].SetX( PROMPT_X+150 );
		break;
	}

	m_rectAnswerBox.SetXY( m_textAnswer[m_Answer].GetX(), m_textAnswer[m_Answer].GetY() );
	m_rectAnswerBox.SetZoomX( m_textAnswer[m_Answer].GetUnzoomedWidth()+10.0f );
	m_rectAnswerBox.SetZoomY( 30 );

	m_textAnswer[m_Answer].SetEffectGlowShift();

	m_In.Load( THEME->GetPathB("ScreenPrompt","in") );
	m_In.StartTransitioning();
	this->AddChild( &m_In );
	
	m_Out.Load( THEME->GetPathB("ScreenPrompt","out") );
	this->AddChild( &m_Out );

	m_sndChange.Load( THEME->GetPathS("ScreenPrompt","change"), true );
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
		PlayerNumber pn;
		if ( GAMESTATE->GetCurrentStyle() == NULL )
			pn = (PlayerNumber)GameI.controller;
		else
			pn = GAMESTATE->GetCurrentStyle()->ControllerToPlayerNumber( GameI.controller );
		switch( DeviceI.button )
		{
		case KEY_LEFT:
			this->MenuLeft( pn );
			return;
		case KEY_RIGHT:
			this->MenuRight( pn );
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
		SCREENMAN->PopTopScreen();
		break;
	}
}

void ScreenPrompt::Change( int dir )
{
	m_textAnswer[m_Answer].SetEffectNone();
	m_Answer = (PromptAnswer)(m_Answer+dir);
	ASSERT( m_Answer >= 0  &&  m_Answer < NUM_PROMPT_ANSWERS );  
	m_textAnswer[m_Answer].SetEffectGlowShift();

	m_rectAnswerBox.StopTweening();
	m_rectAnswerBox.BeginTweening( 0.2f );
	m_rectAnswerBox.SetXY( m_textAnswer[m_Answer].GetX(), m_textAnswer[m_Answer].GetY() );
	m_rectAnswerBox.SetZoomX( m_textAnswer[m_Answer].GetUnzoomedWidth()+10.0f );

	m_sndChange.Play();
}

void ScreenPrompt::MenuLeft( PlayerNumber pn )
{
	if( CanGoLeft() )
		Change( -1 );
}

void ScreenPrompt::MenuRight( PlayerNumber pn )
{
	if( CanGoRight() )
		Change( +1 );
}

void ScreenPrompt::MenuStart( PlayerNumber pn )
{
	m_Out.StartTransitioning( SM_DoneOpeningWipingRight );

	m_textQuestion.BeginTweening( 0.2f );
	m_textQuestion.SetDiffuse( RageColor(1,1,1,0) );

	m_rectAnswerBox.BeginTweening( 0.2f );
	m_rectAnswerBox.SetDiffuse( RageColor(1,1,1,0) );

	m_textAnswer[m_Answer].SetEffectNone();

	for( int i=0; i<NUM_PROMPT_ANSWERS; i++ )
	{
		m_textAnswer[i].BeginTweening( 0.2f );
		m_textAnswer[i].SetDiffuse( RageColor(1,1,1,0) );
	}

	SCREENMAN->PlayStartSound();

	switch( m_Answer )
	{
	case ANSWER_YES:
		if( m_pOnYes )
			m_pOnYes(m_pCallbackData);
		break;
	case ANSWER_NO:
		if( m_pOnNo )
			m_pOnNo(m_pCallbackData);
		break;
	}

	s_LastAnswer = m_Answer;
}

void ScreenPrompt::MenuBack( PlayerNumber pn )
{

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
