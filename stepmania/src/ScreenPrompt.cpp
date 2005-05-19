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
#include "ActorUtil.h"

PromptAnswer ScreenPrompt::s_LastAnswer = ANSWER_YES;
bool ScreenPrompt::s_bCancelledLast = false;

#define ANSWER_TEXT( elem )		THEME->GetMetric(m_sName,elem+"Text")

//REGISTER_SCREEN_CLASS( ScreenPrompt );
ScreenPrompt::ScreenPrompt( 
	const CString &sScreenName,
	ScreenMessage smSendOnPop,
	CString sText, 
	PromptType type, 
	PromptAnswer defaultAnswer, 
	void(*OnYes)(void*), 
	void(*OnNo)(void*), 
	void* pCallbackData 
	) :
	Screen( sScreenName )
{
	m_bIsTransparent = true;	// draw screens below us

	m_smSendOnPop = smSendOnPop;
	m_sText = sText;
	m_PromptType = type;
	m_Answer = defaultAnswer;
	CLAMP( (int&)m_Answer, 0, m_PromptType );
	m_pOnYes = OnYes;
	m_pOnNo = OnNo;
	m_pCallbackData = pCallbackData;
}

void ScreenPrompt::Init()
{
	Screen::Init();

	m_Background.LoadFromAniDir( THEME->GetPathB(m_sName,"background") );
	m_Background.PlayCommand("On");
	this->AddChild( &m_Background );

	m_textQuestion.LoadFromFont( THEME->GetPathF(m_sName,"question") );
	m_textQuestion.SetName( "Question" );
	m_textQuestion.SetText( m_sText );
	SET_XY_AND_ON_COMMAND( m_textQuestion );
	this->AddChild( &m_textQuestion );

	m_sprCursor.Load( THEME->GetPathG(m_sName,"cursor") );
	m_sprCursor->SetName( "Cursor" );
	ON_COMMAND( m_sprCursor );
	this->AddChild( m_sprCursor );

	for( int i=0; i<=m_PromptType; i++ )
	{
		m_textAnswer[i].LoadFromFont( THEME->GetPathF(m_sName,"answer") );
		CString sElem = ssprintf("Answer%dOf%d", i+1, m_PromptType+1);
		m_textAnswer[i].SetName( sElem );
		m_textAnswer[i].SetText( ANSWER_TEXT(sElem) );
		this->AddChild( &m_textAnswer[i] );
		SET_XY_AND_ON_COMMAND( m_textAnswer[i] );
	}

	PositionCursor();

	m_In.Load( THEME->GetPathB(m_sName,"in") );
	m_In.StartTransitioning();
	this->AddChild( &m_In );
	
	m_Out.Load( THEME->GetPathB(m_sName,"out") );
	this->AddChild( &m_Out );
	
	m_Cancel.Load( THEME->GetPathB(m_sName,"cancel") );
	this->AddChild( &m_Cancel );

	m_sndChange.Load( THEME->GetPathS(m_sName,"change"), true );
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
		if( SCREENMAN->IsStackedScreen(this) )
			SCREENMAN->PopTopScreen( m_smSendOnPop );
		else
			this->HandleScreenMessage( SM_GoToNextScreen );
		break;
	}

	Screen::HandleScreenMessage( SM );
}

void ScreenPrompt::Change( int dir )
{
	m_textAnswer[m_Answer].SetEffectNone();
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
		m_Cancel.StartTransitioning( SM_DoneOpeningWipingRight );
	}
	else
	{
		SCREENMAN->PlayStartSound();
		m_Out.StartTransitioning( SM_DoneOpeningWipingRight );
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
