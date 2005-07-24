#include "global.h"
#include "ScreenPrompt.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "Style.h"
#include "ThemeManager.h"
#include "ScreenDimensions.h"
#include "ActorUtil.h"

PromptAnswer ScreenPrompt::s_LastAnswer = ANSWER_YES;
bool ScreenPrompt::s_bCancelledLast = false;

#define ANSWER_TEXT( elem )		THEME->GetMetric(m_sName,elem+"Text")

void ScreenPrompt::Prompt( ScreenMessage smSendOnPop, const CString &sText, PromptType type, PromptAnswer defaultAnswer, void(*OnYes)(void*), void(*OnNo)(void*), void* pCallbackData )
{
	// add the new state onto the back of the array
	ScreenPrompt *pNewScreen = new ScreenPrompt( "ScreenPrompt" );
	pNewScreen->Init();
	pNewScreen->Load( sText, type, defaultAnswer, OnYes, OnNo, pCallbackData );
	SCREENMAN->ZeroNextUpdate();
	SCREENMAN->PushScreen( pNewScreen, true, smSendOnPop );
}


//REGISTER_SCREEN_CLASS( ScreenPrompt );
ScreenPrompt::ScreenPrompt( const CString &sScreenName ):
	ScreenWithMenuElements( sScreenName )
{
}

void ScreenPrompt::Init()
{
	ScreenWithMenuElements::Init();

	m_textQuestion.LoadFromFont( THEME->GetPathF(m_sName,"question") );
	m_textQuestion.SetName( "Question" );
	this->AddChild( &m_textQuestion );

	m_sprCursor.Load( THEME->GetPathG(m_sName,"cursor") );
	m_sprCursor->SetName( "Cursor" );
	this->AddChild( m_sprCursor );

	for( int i=0; i<NUM_PROMPT_ANSWERS; i++ )
	{
		m_textAnswer[i].LoadFromFont( THEME->GetPathF(m_sName,"answer") );
		this->AddChild( &m_textAnswer[i] );
	}

	m_sndChange.Load( THEME->GetPathS(m_sName,"change"), true );
}

void ScreenPrompt::Load( 
	CString sText, 
	PromptType type, 
	PromptAnswer defaultAnswer, 
	void (*OnYes)(void*), 
	void (*OnNo)(void*), 
	void* pCallbackData )
{
	m_sText = sText;
	m_PromptType = type;
	m_Answer = defaultAnswer;
	CLAMP( (int&)m_Answer, 0, m_PromptType );
	m_pOnYes = OnYes;
	m_pOnNo = OnNo;
	m_pCallbackData = pCallbackData;


	m_textQuestion.SetText( m_sText );
	SET_XY_AND_ON_COMMAND( m_textQuestion );

	ON_COMMAND( m_sprCursor );

	for( int i=0; i<=m_PromptType; i++ )
	{
		CString sElem = ssprintf("Answer%dOf%d", i+1, m_PromptType+1);
		m_textAnswer[i].SetName( sElem );
		m_textAnswer[i].SetText( ANSWER_TEXT(sElem) );
		SET_XY_AND_ON_COMMAND( m_textAnswer[i] );
	}

	PositionCursor();

	m_In.StartTransitioning();
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

	ScreenWithMenuElements::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenPrompt::Change( int dir )
{
	m_textAnswer[m_Answer].StopEffect();
	m_Answer = (PromptAnswer)(m_Answer+dir);
	ASSERT( m_Answer >= 0  &&  m_Answer < NUM_PROMPT_ANSWERS );  

	PositionCursor();

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
	if( m_Out.IsTransitioning() || m_Cancel.IsTransitioning() )
		return;

	End( false );
}

void ScreenPrompt::MenuBack( PlayerNumber pn )
{
	if( m_Out.IsTransitioning() || m_Cancel.IsTransitioning() )
		return;

	switch( m_PromptType )
	{
	case PROMPT_OK:
	case PROMPT_YES_NO:
		// don't allow cancel
		break;
	case PROMPT_YES_NO_CANCEL:
		End( true );
		break;
	}
}

void ScreenPrompt::End( bool bCancelled )
{
	if( bCancelled )
	{
		m_Cancel.StartTransitioning( SM_GoToNextScreen );
	}
	else
	{
		SCREENMAN->PlayStartSound();
		m_Out.StartTransitioning( SM_GoToNextScreen );
	}

	OFF_COMMAND( m_textQuestion );
	OFF_COMMAND( m_sprCursor );
	for( int i=0; i<=m_PromptType; i++ )
		OFF_COMMAND( m_textAnswer[i] );

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

	s_LastAnswer = bCancelled ? ANSWER_CANCEL : m_Answer;
	s_bCancelledLast = bCancelled;
}

void ScreenPrompt::PositionCursor()
{
	BitmapText &bt = m_textAnswer[m_Answer];
	m_sprCursor->SetXY( bt.GetX(), bt.GetY() );
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
