#include "global.h"
#include "ScreenTextEntry.h"
#include "RageUtil.h"
#include "Preference.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "FontCharAliases.h"
#include "ScreenDimensions.h"
#include "ScreenPrompt.h"
#include "ActorUtil.h"
#include "InputEventPlus.h"


static const char* g_szKeys[NUM_KEYBOARD_ROWS][KEYS_PER_ROW] =
{
	{"A","B","C","D","E","F","G","H","I","J","K","L","M"},
	{"N","O","P","Q","R","S","T","U","V","W","X","Y","Z"},
	{"a","b","c","d","e","f","g","h","i","j","k","l","m"},
	{"n","o","p","q","r","s","t","u","v","w","x","y","z"},
	{"0","1","2","3","4","5","6","7","8","9"},
	{"!","@","#","$","%","^","&","(",")","[","]","{","}"},
	{"+","-","=","_",",",".","'","\"",":"},
	{"","","Space","","","Backspace","","","Cancel","","","Done",""},
};

static Preference<bool> g_bAllowOldKeyboardInput( "AllowOldKeyboardInput",	true );

CString ScreenTextEntry::s_sLastAnswer = "";

/* Settings: */
namespace
{
	CString g_sQuestion;
	CString g_sInitialAnswer;
	int g_iMaxInputLength;
	bool(*g_pValidate)(const CString &sAnswer,CString &sErrorOut);
	void(*g_pOnOK)(const CString &sAnswer);
	void(*g_pOnCancel)();
	bool g_bPassword;
};

void ScreenTextEntry::TextEntry( 
	ScreenMessage smSendOnPop, 
	CString sQuestion, 
	CString sInitialAnswer, 
	int iMaxInputLength,
	bool(*Validate)(const CString &sAnswer,CString &sErrorOut), 
	void(*OnOK)(const CString &sAnswer), 
	void(*OnCancel)(),
	bool bPassword
	)
{	
	g_sQuestion = sQuestion;
	g_sInitialAnswer = sInitialAnswer;
	g_iMaxInputLength = iMaxInputLength;
	g_pValidate = Validate;
	g_pOnOK = OnOK;
	g_pOnCancel = OnCancel;
	g_bPassword = bPassword;

	SCREENMAN->AddNewScreenToTop( "ScreenTextEntry", smSendOnPop );
}

bool ScreenTextEntry::s_bCancelledLast = false;

/*
 * Handle UTF-8.  Right now, we need to at least be able to backspace
 * a whole UTF-8 character.  Better would be to operate in wchar_t.
 *
 * XXX: Don't allow internal-use codepoints (above 0xFFFF); those are
 * subject to change and shouldn't be written to disk.
 */
REGISTER_SCREEN_CLASS( ScreenTextEntry );

ScreenTextEntry::ScreenTextEntry( CString sClassName ) :
	ScreenWithMenuElements( sClassName )
{
}

void ScreenTextEntry::Init()
{
	ScreenWithMenuElements::Init();

	m_textQuestion.LoadFromFont( THEME->GetPathF(m_sName,"question") );
	m_textQuestion.SetName( "Question" );
	this->AddChild( &m_textQuestion );

	m_sprAnswerBox.Load( THEME->GetPathG(m_sName,"AnswerBox") );
	m_sprAnswerBox->SetName( "AnswerBox" );
	this->AddChild( m_sprAnswerBox );

	m_textAnswer.LoadFromFont( THEME->GetPathF(m_sName,"answer") );
	m_textAnswer.SetName( "Answer" );
	this->AddChild( &m_textAnswer );

	m_bShowAnswerCaret = false;

	m_sprCursor.Load( THEME->GetPathG(m_sName,"cursor") );
	m_sprCursor->SetName( "Cursor" );
	this->AddChild( m_sprCursor );

	// Init keyboard
	{
		BitmapText text;
		text.LoadFromFont( THEME->GetPathF(m_sName,"keyboard") );
		text.SetName( "Keys" );
		ActorUtil::LoadAllCommands( text, m_sName );
		text.PlayCommand( "Init" );

		FOREACH_KeyboardRow( r )
		{
			for( int x=0; x<KEYS_PER_ROW; ++x )
			{
				BitmapText *&pbt = m_ptextKeys[r][x];
				pbt = static_cast<BitmapText *>( text.Copy() );
				this->AddChild( pbt );
			}
		}
	}

	m_sndType.Load( THEME->GetPathS(m_sName,"type"), true );
	m_sndBackspace.Load( THEME->GetPathS(m_sName,"backspace"), true );
	m_sndChange.Load( THEME->GetPathS(m_sName,"change"), true );
}

ScreenTextEntry::~ScreenTextEntry()
{
	FOREACH_KeyboardRow( r )
		for( int x=0; x<KEYS_PER_ROW; ++x )
			SAFE_DELETE( m_ptextKeys[r][x] );
}

void ScreenTextEntry::BeginScreen()
{
	m_sAnswer = RStringToWstring( g_sInitialAnswer );

	ScreenWithMenuElements::BeginScreen();

	m_textQuestion.SetText( g_sQuestion );
	SET_XY_AND_ON_COMMAND( m_textQuestion );
	SET_XY_AND_ON_COMMAND( m_sprAnswerBox );
	SET_XY_AND_ON_COMMAND( m_textAnswer );

	UpdateAnswerText();

	ON_COMMAND( m_sprCursor );
	m_iFocusX = 0;
	m_iFocusY = (KeyboardRow)0;

	FOREACH_KeyboardRow( r )
	{
		for( int x=0; x<KEYS_PER_ROW; ++x )
		{
			BitmapText &bt = *m_ptextKeys[r][x];
			float fX = roundf( SCALE( x, 0, KEYS_PER_ROW-1, SCREEN_LEFT+100, SCREEN_RIGHT-100 ) );
			float fY = roundf( SCALE( r, 0, NUM_KEYBOARD_ROWS-1, SCREEN_CENTER_Y-30, SCREEN_BOTTOM-80 ) );
			bt.SetXY( fX, fY );
		}
	}

	UpdateKeyboardText();

	PositionCursor();
}

void ScreenTextEntry::UpdateKeyboardText()
{
	FOREACH_KeyboardRow( r )
	{
		for( int x=0; x<KEYS_PER_ROW; ++x )
		{
			CString s = g_szKeys[r][x];
			if( !s.empty()  &&  r == KEYBOARD_ROW_SPECIAL )
				s = THEME->GetMetric( "ScreenTextEntry", s );
			BitmapText &bt = *m_ptextKeys[r][x];
			bt.SetText( s );
		}
	}
}

void ScreenTextEntry::UpdateAnswerText()
{
	CString s;
	if( g_bPassword )
		s = RString( m_sAnswer.size(), '*' );
	else
		s = WStringToRString(m_sAnswer);

	bool bAnswerFull = (int) s.length() >= g_iMaxInputLength;
	if( m_bShowAnswerCaret 	&&  !bAnswerFull )
		s += '_';
	else
		s += "  ";	// assumes that underscore is the width of two spaces

	FontCharAliases::ReplaceMarkers( s );
	m_textAnswer.SetText( s );
}

void ScreenTextEntry::PositionCursor()
{
	BitmapText &bt = *m_ptextKeys[m_iFocusY][m_iFocusX];
	m_sprCursor->SetXY( bt.GetX(), bt.GetY() );
	m_sprCursor->PlayCommand( m_iFocusY == KEYBOARD_ROW_SPECIAL ? "SpecialKey" : "RegularKey" );
}

void ScreenTextEntry::Update( float fDelta )
{
	ScreenWithMenuElements::Update( fDelta );

	if( m_timerToggleCursor.PeekDeltaTime() > 0.25f )
	{
		m_timerToggleCursor.Touch();
		m_bShowAnswerCaret = !m_bShowAnswerCaret;
		UpdateAnswerText();
	}
}

void ScreenTextEntry::Input( const InputEventPlus &input )
{
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() || m_Cancel.IsTransitioning() )
		return;

	//The user wants to input text traditionally
	if( g_bAllowOldKeyboardInput.Get() && ( input.type == IET_FIRST_PRESS ) )
	{
		if( input.DeviceI.button == KEY_BACK )
		{
			BackspaceInAnswer();
		}
		else if( input.DeviceI.ToChar() >= ' ' ) 
		{
			bool bIsHoldingShift = 
					INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT)) ||
					INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT));
			if ( bIsHoldingShift )
			{

				char c = (char)toupper( input.DeviceI.ToChar() );

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
				case '\'':	c='"';	break;
				case '\\':	c='|';	break;
				case ';':	c=':';	break;
				case ',':	c='<';	break;
				case '.':	c='>';	break;
				case '/':	c='?';	break;
				}

				AppendToAnswer( ssprintf( "%c", c ) );
			}
			else
			{
				AppendToAnswer( ssprintf( "%c", input.DeviceI.ToChar() ) );
			}

			//If the user wishes to select text in traditional way, start should finish text entry
			m_iFocusY = KEYBOARD_ROW_SPECIAL;
			m_iFocusX = DONE;

			UpdateKeyboardText();
			PositionCursor();
		}
	}

	ScreenWithMenuElements::Input( input );
}

void ScreenTextEntry::MoveX( int iDir )
{
	CString sKey;
	do
	{
		m_iFocusX += iDir;
		wrap( m_iFocusX, KEYS_PER_ROW );

		sKey = g_szKeys[m_iFocusY][m_iFocusX]; 
	}
	while( sKey == "" );

	m_sndChange.Play();
	PositionCursor();
}

void ScreenTextEntry::MoveY( int iDir )
{
	CString sKey;
	do
	{
		m_iFocusY = (KeyboardRow)(m_iFocusY + iDir);
		wrap( (int&)m_iFocusY, NUM_KEYBOARD_ROWS );

		// HACK: Round to nearest option so that we always stop 
		// on KEYBOARD_ROW_SPECIAL.
		if( m_iFocusY == KEYBOARD_ROW_SPECIAL )
		{
			for( int i=0; true; i++ )
			{
				sKey = g_szKeys[m_iFocusY][m_iFocusX]; 
				if( sKey != "" )
					break;

				// UGLY: Probe one space to the left before looking to the right
				m_iFocusX += (i==0) ? -1 : +1;
				wrap( m_iFocusX, KEYS_PER_ROW );
			}
		}

		sKey = g_szKeys[m_iFocusY][m_iFocusX]; 
	}
	while( sKey == "" );

	m_sndChange.Play();
	PositionCursor();
}

void ScreenTextEntry::AppendToAnswer( CString s )
{
	wstring sNewAnswer = m_sAnswer+RStringToWstring(s);
	if( (int)sNewAnswer.length() > g_iMaxInputLength )
	{
		SCREENMAN->PlayInvalidSound();
		return;
	}

	m_sAnswer = sNewAnswer;
	m_sndType.Play();
	UpdateAnswerText();

	UpdateKeyboardText();
}

void ScreenTextEntry::BackspaceInAnswer()
{
	if( m_sAnswer.empty() )
	{
		SCREENMAN->PlayInvalidSound();
		return;
	}
	m_sAnswer.erase( m_sAnswer.end()-1 );
	m_sndBackspace.Play();
	UpdateAnswerText();
}

void ScreenTextEntry::MenuStart( PlayerNumber pn )
{
	if( m_iFocusY == KEYBOARD_ROW_SPECIAL )
	{
		switch( m_iFocusX )
		{
		case SPACEBAR:
			AppendToAnswer( " " );
			break;
		case BACKSPACE:
			BackspaceInAnswer();
			break;
		case CANCEL:
			End( true );
			break;
		case DONE:
			End( false );
			break;
		default:
			break;
		}
	}
	else
	{
		AppendToAnswer( g_szKeys[m_iFocusY][m_iFocusX] );
	}
}

void ScreenTextEntry::End( bool bCancelled )
{
	if( bCancelled )
	{
		if( g_pOnCancel ) 
			g_pOnCancel();
		
		m_Cancel.StartTransitioning( SM_GoToNextScreen );
	}
	else
	{
		CString sAnswer = WStringToRString(m_sAnswer);
		CString sError;
		if ( g_pValidate != NULL )
		{
			bool bValidAnswer = g_pValidate( sAnswer, sError );
			if( !bValidAnswer )
			{
				ScreenPrompt::Prompt( SM_None, sError );
				return;	// don't end this screen.
			}
		}

		if( g_pOnOK )
		{
			CString ret = WStringToRString(m_sAnswer);
			FontCharAliases::ReplaceMarkers(ret);
			g_pOnOK( ret );
		}

		m_Out.StartTransitioning( SM_GoToNextScreen );
		SCREENMAN->PlayStartSound();
	}

	OFF_COMMAND( m_textQuestion );
	OFF_COMMAND( m_sprAnswerBox );
	OFF_COMMAND( m_textAnswer );
	OFF_COMMAND( m_sprCursor );

	s_bCancelledLast = bCancelled;
	s_sLastAnswer = bCancelled ? CString("") : WStringToRString(m_sAnswer);
}

void ScreenTextEntry::MenuBack( PlayerNumber pn )
{
	End( true );
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
