#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenPrompt.h

 Desc: Area for testing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#include "ScreenPrompt.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "RageMusic.h"
#include "ScreenTitleMenu.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"

ScreenPrompt::ScreenPrompt( CString sText, PromptType pt, bool* pbAnswer )
{

	static bool bThrowAway;

	if( pbAnswer == NULL )
		pbAnswer = &bThrowAway;


	m_PromptType = pt;
	m_pbAnswer = pbAnswer;

	ASSERT( !(pt == PROMPT_YES_NO  &&  pt == NULL) );

	m_Fade.SetTransitionTime( 0.5f );
	m_Fade.SetColor( D3DXCOLOR(0,0,0,0.7f) );
	m_Fade.SetOpened();
	m_Fade.CloseWipingRight();
	m_Fade.SetZ(-2);
	this->AddSubActor( &m_Fade );

	CStringArray arrayTextLines;
	split( sText, "\n", arrayTextLines );
	for( int i=0; i<arrayTextLines.GetSize(); i++ )
	{
		m_textQuestion[i].Load( THEME->GetPathTo(FONT_HEADER1) );
		m_textQuestion[i].SetText( arrayTextLines[i] );
		m_textQuestion[i].SetXY( CENTER_X, CENTER_Y-50 + i*27 - arrayTextLines.GetSize()*27/2 );
		m_textQuestion[i].SetZ(-2);
		this->AddSubActor( &m_textQuestion[i] );
	}

	m_rectAnswerBox.SetDiffuseColor( D3DXCOLOR(0.5f,0.5f,1.0f,0.7f) );
	m_rectAnswerBox.SetXY( m_textAnswer[*m_pbAnswer].GetX(), m_textAnswer[*m_pbAnswer].GetY() );
	m_rectAnswerBox.SetZoomX( m_textAnswer[*m_pbAnswer].GetWidestLineWidthInSourcePixels()+10.0f );
	m_rectAnswerBox.SetZoomY( 30 );
	m_rectAnswerBox.SetZ(-2);
	this->AddSubActor( &m_rectAnswerBox );

	m_textAnswer[0].Load( THEME->GetPathTo(FONT_HEADER1) );
	m_textAnswer[1].Load( THEME->GetPathTo(FONT_HEADER1) );
	m_textAnswer[0].SetY( CENTER_Y+120 );
	m_textAnswer[1].SetY( CENTER_Y+120 );
	m_textAnswer[0].SetZ(-2);
	m_textAnswer[1].SetZ(-2);
	this->AddSubActor( &m_textAnswer[0] );
	this->AddSubActor( &m_textAnswer[1] );

	

	switch( m_PromptType )
	{
	case PROMPT_OK:
		m_textAnswer[0].SetText( "OK" );
		m_textAnswer[0].SetX( CENTER_X );
		break;
	case PROMPT_YES_NO:
		m_textAnswer[0].SetText( "NO" );
		m_textAnswer[1].SetText( "YES" );
		m_textAnswer[0].SetX( CENTER_X+50 );
		m_textAnswer[1].SetX( CENTER_X-50 );
		break;
	}

	m_rectAnswerBox.SetXY( m_textAnswer[*m_pbAnswer].GetX(), m_textAnswer[*m_pbAnswer].GetY() );
	m_rectAnswerBox.SetZoomX( m_textAnswer[*m_pbAnswer].GetWidestLineWidthInSourcePixels()+10.0f );

	m_textAnswer[*m_pbAnswer].SetEffectGlowing();


	m_soundSelect.Load( THEME->GetPathTo(SOUND_MENU_START) );
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
	if( m_Fade.IsOpening() )
		return;

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

void ScreenPrompt::MenuLeft( const PlayerNumber p )
{
	if( m_PromptType == PROMPT_OK )
		return;

	MenuRight( p );
}

void ScreenPrompt::MenuRight( const PlayerNumber p )
{
	if( m_PromptType == PROMPT_OK )
		return;

	m_textAnswer[*m_pbAnswer].SetEffectNone();
	*m_pbAnswer = !*m_pbAnswer;
	m_textAnswer[*m_pbAnswer].SetEffectGlowing();

	m_rectAnswerBox.BeginTweening( 0.2f );
	m_rectAnswerBox.SetTweenXY( m_textAnswer[*m_pbAnswer].GetX(), m_textAnswer[*m_pbAnswer].GetY() );
	m_rectAnswerBox.SetTweenZoomX( m_textAnswer[*m_pbAnswer].GetWidestLineWidthInSourcePixels()+10.0f );

}

void ScreenPrompt::MenuStart( const PlayerNumber p )
{
	m_Fade.OpenWipingRight( SM_DoneOpeningWipingRight );

	m_textTitle.BeginTweening( 0.2f );
	m_textTitle.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );

	for( int i=0; i<NUM_QUESTION_LINES; i++ )
	{
		m_textQuestion[i].BeginTweening( 0.2f );
		m_textQuestion[i].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
	}

	m_rectAnswerBox.BeginTweening( 0.2f );
	m_rectAnswerBox.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );

	m_textAnswer[*m_pbAnswer].SetEffectNone();

	m_textAnswer[0].BeginTweening( 0.2f );
	m_textAnswer[0].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
	m_textAnswer[1].BeginTweening( 0.2f );
	m_textAnswer[1].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );


	m_soundSelect.PlayRandom();
}

void ScreenPrompt::MenuBack( const PlayerNumber p )
{

}
