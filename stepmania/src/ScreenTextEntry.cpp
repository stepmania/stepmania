#include "global.h"
#include "ScreenTextEntry.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "FontCharAliases.h"
#include "ScreenDimensions.h"
#include "ActorUtil.h"

const SPECIAL_KEY_WIDTH = 3;	// special keys are 3x as wide as normal keys


const int MAX_KEYS_PER_ROW = 13;

float GetButtonX( int x )
{
	return roundf( SCALE( x, 0, MAX_KEYS_PER_ROW-1, SCREEN_LEFT+100, SCREEN_RIGHT-100 ) );
}

float GetButtonY( KeyboardRow r )
{
	return roundf( SCALE( r, 0, NUM_KEYBOARD_ROWS-1, SCREEN_CENTER_Y, SCREEN_BOTTOM-100 ) );
}

CString ScreenTextEntry::s_sLastAnswer = "";
bool ScreenTextEntry::s_bCancelledLast = false;

/* XXX: Don't let the user use internal-use codepoints (those
 * that resolve to Unicode codepoints above 0xFFFF); those are
 * subject to change and shouldn't be written to .SMs.
 *
 * Handle UTF-8.  Right now, we need to at least be able to backspace
 * a whole UTF-8 character.  Better would be to operate in longchars.
 */
//REGISTER_SCREEN_CLASS( ScreenTextEntry );

ScreenTextEntry::ScreenTextEntry( CString sClassName, CString sQuestion, CString sInitialAnswer, void(*OnOK)(CString sAnswer), void(*OnCancel)(), bool bPassword ) :
  Screen( sClassName )
{
	m_sName = "ScreenTextEntry";

	m_bIsTransparent = true;	// draw screens below us
	m_bPassword = bPassword;

	m_pOnOK = OnOK;
	m_pOnCancel = OnCancel;

	m_sAnswer = CStringToWstring( sInitialAnswer );
	m_bCancelled = false;

	m_sQuestion = sQuestion;
}

void ScreenTextEntry::Init()
{
	m_Background.Load( THEME->GetPathB(m_sName,"background") );
	m_Background->SetDrawOrder( DRAW_ORDER_BEFORE_EVERYTHING );
	this->AddChild( m_Background );
	m_Background->PlayCommand( "On" );

	m_textQuestion.LoadFromFont( THEME->GetPathF(m_sName,"question") );
	m_textQuestion.SetName( "Question" );
	m_textQuestion.SetText( m_sQuestion );
	SET_XY_AND_ON_COMMAND( m_textQuestion );
	this->AddChild( &m_textQuestion );

	m_sprAnswerBox.Load( THEME->GetPathG(m_sName,"AnswerBox") );
	m_sprAnswerBox->SetName( "AnswerBox" );
	SET_XY_AND_ON_COMMAND( m_sprAnswerBox );
	this->AddChild( m_sprAnswerBox );

	m_textAnswer.LoadFromFont( THEME->GetPathF(m_sName,"answer") );
	m_textAnswer.SetName( "Answer" );
	SET_XY_AND_ON_COMMAND( m_textAnswer );
	UpdateAnswerText();
	this->AddChild( &m_textAnswer );


	m_sprCursor.Load( THEME->GetPathG(m_sName,"cursor") );
	m_sprCursor->SetName( "Cursor" );
	ON_COMMAND( m_sprCursor );
	this->AddChild( m_sprCursor );

	m_iFocusX = 0;
	m_iFocusY = (KeyboardRow)0;


	// Fill in m_Keys
	{
		static const char* g_szKeys[NUM_KEYBOARD_CASES][NUM_KEYBOARD_ROWS][MAX_KEYS_PER_ROW+1] =
		{
			{
				{"`","1","2","3","4","5","6","7","8","9","0","-","=",NULL},
				{"q","w","e","r","t","y","u","i","o","p","[","]","\\",NULL},
				{"a","s","d","f","g","h","j","k","l",";","\"",NULL},
				{"z","x","c","v","b","n","m",",",".","/",NULL},
				{"Caps","Space","Backsp","Done",NULL}
			},
			{
				{"~","!","@","#","$","%","^","&","*","(",")","_","+",NULL},
				{"Q","W","E","R","T","Y","U","I","O","P","{","}","|",NULL},
				{"A","S","D","F","G","H","J","K","L",":","'",NULL},
				{"Z","X","C","V","B","N","M","<",">","?",NULL},
				{"CAPS","Space","Backsp","Done",NULL}
			}
		};
		FOREACH_KeyboardCase( k )
		{
			FOREACH_KeyboardRow( r )
			{
				for( int x=0; true; x++ )
				{
					const char *c=g_szKeys[LOWERCASE][r][x];
					if( c == NULL )
						break;
					m_Keys[k][r].push_back( c );
					
				}

				ASSERT_M( m_Keys[k][r].size() == m_Keys[0][r].size(), "uppercase and lowercase sizes don't match" );
			}
		}
	}

	// Init keyboard
	FOREACH_KeyboardRow( r )
	{
		vector<BitmapText*>	&v = m_textKeyboardChars[r];
		const CStringArray &vKeys = m_Keys[LOWERCASE][r];
		for( unsigned x=0; x<vKeys.size(); x++ )
		{
			BitmapText *p = new BitmapText;
			p->LoadFromFont( THEME->GetPathF(m_sName,"keyboard") );
			p->SetXY( GetButtonX(x), GetButtonY(r) );
			v.push_back( p );
			this->AddChild( p );
		}
	}

	UpdateKeyboardText();

	PositionCursor();

	m_In.Load( THEME->GetPathB(m_sName,"in") );
	m_In.StartTransitioning();
	this->AddChild( &m_In );
	
	m_Out.Load( THEME->GetPathB(m_sName,"out") );
	this->AddChild( &m_Out );


	m_sndType.Load( THEME->GetPathS(m_sName,"type"), true );
	m_sndBackspace.Load( THEME->GetPathS(m_sName,"backspace"), true );
}

ScreenTextEntry::~ScreenTextEntry()
{
	for( int i=0; i<NUM_KEYBOARD_ROWS; i++ )
	{
		FOREACH( BitmapText*, m_textKeyboardChars[i], p )
			SAFE_DELETE(*p);
		m_textKeyboardChars[i].clear();
	}
}

void ScreenTextEntry::UpdateKeyboardText()
{
	FOREACH_KeyboardRow( r )
	{
		vector<BitmapText*>	&v = m_textKeyboardChars[r];
		const CStringArray &vKeys = m_Keys[LOWERCASE][r];
		for( unsigned x=0; x<vKeys.size(); x++ )
		{
			BitmapText *p = v[x];
			p->SetText( vKeys[x] );
		}
	}
}

void ScreenTextEntry::UpdateAnswerText()
{
	CString txt = WStringToCString(m_sAnswer);
	if( m_bPassword )
	{
		int len = txt.GetLength();
		txt = "";
		for( int i=0; i<len; ++i )
			txt += "*";
	}
	FontCharAliases::ReplaceMarkers(txt);
	m_textAnswer.SetText( txt );
}

void ScreenTextEntry::PositionCursor()
{
	BitmapText *p = m_textKeyboardChars[m_iFocusY][m_iFocusX];
	m_sprCursor->SetXY( p->GetX(), p->GetY() );
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
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return;

	if( type != IET_FIRST_PRESS )
		return;

	if( !MenuI.IsValid() )
	{
		switch( DeviceI.button )
		{
		case KEY_ESC:
			m_bCancelled = true;
			End();
			break;
		case KEY_ENTER:
		case KEY_KP_ENTER:
			End();
			break;
		case KEY_BACK:
			if(!m_sAnswer.empty())
				m_sAnswer = m_sAnswer.erase( m_sAnswer.size()-1 );
			UpdateAnswerText();
			break;
		default:
			char c = DeviceI.ToChar();

			bool bHoldingShift = 
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT));

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
		
			if( c >= ' ' )
			{
				m_sAnswer += c;
				UpdateAnswerText();
			}
			break;
		}

		return;	// don't let Screen::Input handle
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
		SCREENMAN->PopTopScreen();
		break;
	}
}

void ScreenTextEntry::MoveX( int iDir )
{
	m_iFocusX += iDir;
	wrap( m_iFocusX, m_Keys[LOWERCASE][m_iFocusY].size() );

	PositionCursor();
}

void ScreenTextEntry::MoveY( int iDir )
{
	m_iFocusY = (KeyboardRow)(m_iFocusY + iDir);
	wrap( (int&)m_iFocusY, NUM_KEYBOARD_ROWS );
	CLAMP( m_iFocusX, 0, (int)m_Keys[LOWERCASE][m_iFocusY].size()-1 );

	PositionCursor();
}

void ScreenTextEntry::MenuStart( PlayerNumber pn )
{
	if( m_iFocusY == KEYBOARD_ROW_SPECIAL )
	{
		switch( m_iFocusX )
		{
		case CAPS:
			break;
		case SPACEBAR:
			m_sAnswer += CStringToWstring( " " );
			m_sndType.Play();
			UpdateAnswerText();
			break;
		case BACKSPACE:
			m_sAnswer.erase( m_sAnswer.end()-1 );
			m_sndBackspace.Play();
			UpdateAnswerText();
			break;
		case DONE:
			End();
			break;
		default:
			break;
		}
	}
	else
	{
		m_sAnswer += CStringToWstring( m_Keys[LOWERCASE][m_iFocusY][m_iFocusX] );
		m_sndType.Play();
		UpdateAnswerText();
	}
}

void ScreenTextEntry::End()
{
	m_Out.StartTransitioning( SM_DoneOpeningWipingRight );

	m_Background->PlayCommand("Off");

	OFF_COMMAND( m_textQuestion );
	OFF_COMMAND( m_sprAnswerBox );
	OFF_COMMAND( m_textAnswer );
	OFF_COMMAND( m_sprCursor );

	SCREENMAN->PlayStartSound();

	if( m_bCancelled )
	{
		if( m_pOnCancel ) 
			m_pOnCancel();

	} 
	else 
	{
		if( m_pOnOK )
		{
			CString ret = WStringToCString(m_sAnswer);
			FontCharAliases::ReplaceMarkers(ret);
			m_pOnOK( ret );
		}
	}

	s_bCancelledLast = m_bCancelled;
	s_sLastAnswer = m_bCancelled ? CString("") : WStringToCString(m_sAnswer);
}

void ScreenTextEntry::MenuBack( PlayerNumber pn )
{
	m_bCancelled = true;
	End();
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
