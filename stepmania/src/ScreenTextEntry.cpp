#include "global.h"
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
#include "RageSoundManager.h"
#include "ScreenTitleMenu.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "FontCharAliases.h"

const float QUESTION_X	=	CENTER_X;
const float QUESTION_Y	=	CENTER_Y - 60;

const float ANSWER_X	=	CENTER_X;
const float ANSWER_Y	=	CENTER_Y + 120;
const float ANSWER_WIDTH	=	440;
const float ANSWER_HEIGHT	=	30;

/* XXX: Don't let the user use internal-use codepoints (those
 * that resolve to Unicode codepoints above 0xFFFF); those are
 * subject to change and shouldn't be written to .SMs.
 *
 * Handle UTF-8.  Right now, we need to at least be able to backspace
 * a whole UTF-8 character.  Better would be to operate in longchars.
 */
ScreenTextEntry::ScreenTextEntry( ScreenMessage SM_SendWhenDone, CString sQuestion, CString sInitialAnswer, void(*OnOK)(CString sAnswer), void(*OnCancel)() )
{
	m_SMSendWhenDone = SM_SendWhenDone;
	m_pOnOK = OnOK;
	m_pOnCancel = OnCancel;

	m_sAnswer = CStringToWstring(sInitialAnswer);
	m_bCancelled = false;

	m_Fade.SetTransitionTime( 0.5f );
	m_Fade.SetDiffuse( RageColor(0,0,0,0.7f) );
	m_Fade.SetOpened();
	m_Fade.CloseWipingRight();
	this->AddChild( &m_Fade );

	m_textQuestion.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textQuestion.SetText( sQuestion );
	m_textQuestion.SetXY( QUESTION_X, QUESTION_Y );
	this->AddChild( &m_textQuestion );

	m_rectAnswerBox.SetDiffuse( RageColor(0.5f,0.5f,1.0f,0.7f) );
	this->AddChild( &m_rectAnswerBox );

	m_rectAnswerBox.SetXY( ANSWER_X, ANSWER_Y );
	m_rectAnswerBox.SetZoomX( ANSWER_WIDTH );
	m_rectAnswerBox.SetZoomY( ANSWER_HEIGHT );
	this->AddChild( &m_rectAnswerBox );

	m_textAnswer.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textAnswer.SetXY( ANSWER_X, ANSWER_Y );
	UpdateText();
	this->AddChild( &m_textAnswer );

	SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","menu prompt") );
}

void ScreenTextEntry::UpdateText()
{
	CString txt = WStringToCString(m_sAnswer);
	FontCharAliases::ReplaceMarkers(txt);
	m_textAnswer.SetText( txt );
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
	case SDLK_ESCAPE:
		m_bCancelled = true;
		MenuStart(PLAYER_1);
		break;
	case SDLK_RETURN:
		MenuStart(PLAYER_1);
		break;
	case SDLK_BACKSPACE:
		if(!m_sAnswer.empty())
			m_sAnswer = m_sAnswer.erase( m_sAnswer.size()-1 );
		UpdateText();
		break;
	default:
		char c;
		c = DeviceI.ToChar();

		bool bHoldingShift = 
			INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, SDLK_LSHIFT)) ||
			INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, SDLK_RSHIFT));

		// International keyboards often have other keys mapped to shifted keys, and always 
		// using a US layout is a bit gimped.  This is better than nothing though.
		if( bHoldingShift )
		{
			c = (char)toupper(c);

			switch( c )
			{
			case '`':	c='~';	break;
			case '1':	c='!';	break;
			case '2':	c='@';	break;
			case '3':	c='#';	break;
			case '4':	c='$';	break;
			case '5':	c='%';	break;
			case '6':	c='^';	break;
			case '7':	c='&';	break;
			case '8':	c='*';	break;
			case '9':	c='(';	break;
			case '0':	c=')';	break;
			case '-':	c='_';	break;
			case '=':	c='+';	break;
			case '[':	c='{';	break;
			case ']':	c='}';	break;
			case '\\':	c='|';	break;
			case ';':	c=':';	break;
			case '\'':	c='"';	break;
			case ',':	c='<';	break;
			case '.':	c='>';	break;
			case '/':	c='?';	break;
			}
		}

		if( c != '\0' )
		{
			m_sAnswer += c;
			UpdateText();
		}
		break;
	}
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

void ScreenTextEntry::MenuLeft( PlayerNumber pn )
{
}

void ScreenTextEntry::MenuRight( PlayerNumber pn )
{
}

void ScreenTextEntry::MenuStart( PlayerNumber pn )
{
	m_Fade.OpenWipingRight( SM_DoneOpeningWipingRight );

	m_textQuestion.BeginTweening( 0.2f );
	m_textQuestion.SetTweenDiffuse( RageColor(1,1,1,0) );

	m_rectAnswerBox.BeginTweening( 0.2f );
	m_rectAnswerBox.SetTweenDiffuse( RageColor(1,1,1,0) );

	m_textAnswer.SetEffectNone();

	m_textAnswer.BeginTweening( 0.2f );
	m_textAnswer.SetTweenDiffuse( RageColor(1,1,1,0) );

	SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","menu start") );

	if( m_bCancelled ) {
		if( m_pOnCancel ) m_pOnCancel();
	} else {
		if( m_pOnOK )
		{
			CString ret = WStringToCString(m_sAnswer);
			FontCharAliases::ReplaceMarkers(ret);
			m_pOnOK( ret );
		}
	}
}

void ScreenTextEntry::MenuBack( PlayerNumber pn )
{
	m_bCancelled = true;
	MenuStart(pn);
}
