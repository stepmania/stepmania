#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenTextEntry

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenTextEntry.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "RageMusic.h"
#include "ScreenTitleMenu.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"

const float QUESTION_X	=	CENTER_X;
const float QUESTION_Y	=	CENTER_Y - 60;

const float ANSWER_X	=	CENTER_X;
const float ANSWER_Y	=	CENTER_Y + 120;
const float ANSWER_WIDTH	=	440;
const float ANSWER_HEIGHT	=	30;

ScreenTextEntry::ScreenTextEntry( ScreenMessage SM_SendWhenDone, CString sQuestion, CString sInitialAnswer, void(*OnOK)(CString sAnswer), void(*OnCancel)() )
{
	m_SMSendWhenDone = SM_SendWhenDone;
	m_pOnOK = OnOK;
	m_pOnCancel = OnCancel;
	m_sAnswer = sInitialAnswer;

	m_Fade.SetTransitionTime( 0.5f );
	m_Fade.SetDiffuseColor( D3DXCOLOR(0,0,0,0.7f) );
	m_Fade.SetOpened();
	m_Fade.CloseWipingRight();
	this->AddSubActor( &m_Fade );

	m_textQuestion.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textQuestion.SetText( sQuestion );
	m_textQuestion.SetXY( QUESTION_X, QUESTION_Y );
	this->AddSubActor( &m_textQuestion );

	m_rectAnswerBox.SetDiffuseColor( D3DXCOLOR(0.5f,0.5f,1.0f,0.7f) );
	this->AddSubActor( &m_rectAnswerBox );

	m_rectAnswerBox.SetXY( ANSWER_X, ANSWER_Y );
	m_rectAnswerBox.SetZoomX( ANSWER_WIDTH );
	m_rectAnswerBox.SetZoomY( ANSWER_HEIGHT );
	this->AddSubActor( &m_rectAnswerBox );

	m_textAnswer.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textAnswer.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textAnswer.SetXY( ANSWER_X, ANSWER_Y );
	m_textAnswer.SetText( m_sAnswer );
	this->AddSubActor( &m_textAnswer );

	SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu prompt") );
}

void ScreenTextEntry::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}

void ScreenTextEntry::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenTextEntry::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( m_Fade.IsOpening() )
		return;

	if( type != IET_FIRST_PRESS )
		return;

	switch( DeviceI.button )
	{
	case DIK_ESCAPE:
		m_bCancelled = true;
		MenuStart(PLAYER_1);
		break;
	case DIK_RETURN:
		MenuStart(PLAYER_1);
		break;
	case DIK_BACK:
		m_sAnswer = m_sAnswer.Left( max(0,m_sAnswer.GetLength()-1) );
		m_textAnswer.SetText( m_sAnswer );
		break;
	default:
		char c;
		c = DeviceI.ToChar();
		if( c != '\0' )
			m_sAnswer += c;
		m_textAnswer.SetText( m_sAnswer );
		break;
	}

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenTextEntry::HandleScreenMessage( const ScreenMessage SM )
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

void ScreenTextEntry::MenuLeft( PlayerNumber p )
{
}

void ScreenTextEntry::MenuRight( PlayerNumber p )
{
}

void ScreenTextEntry::MenuStart( PlayerNumber p )
{
	m_Fade.OpenWipingRight( SM_DoneOpeningWipingRight );

	m_textQuestion.BeginTweening( 0.2f );
	m_textQuestion.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );

	m_rectAnswerBox.BeginTweening( 0.2f );
	m_rectAnswerBox.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );

	m_textAnswer.SetEffectNone();

	m_textAnswer.BeginTweening( 0.2f );
	m_textAnswer.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );

	SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu start") );

	if( m_bCancelled )
		if( m_pOnCancel )
			m_pOnCancel();
	else
		if( m_pOnOK )
			m_pOnOK( m_sAnswer );
}

void ScreenTextEntry::MenuBack( PlayerNumber p )
{
	m_bCancelled = true;
	MenuStart(p);
}
