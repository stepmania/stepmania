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
#include "RageInput.h"
#include "LocalizedString.h"

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

RString ScreenTextEntry::s_sLastAnswer = "";

/* Settings: */
namespace
{
	RString g_sQuestion;
	RString g_sInitialAnswer;
	int g_iMaxInputLength;
	bool(*g_pValidate)(const RString &sAnswer,RString &sErrorOut);
	void(*g_pOnOK)(const RString &sAnswer);
	void(*g_pOnCancel)();
	bool g_bPassword;
	bool (*g_pValidateAppend)(const RString &sAnswerBeforeChar, RString &sAppend);
	RString (*g_pFormatAnswerForDisplay)(const RString &sAnswer);
};

void ScreenTextEntry::SetTextEntrySettings( 
	RString sQuestion, 
	RString sInitialAnswer, 
	int iMaxInputLength,
	bool(*Validate)(const RString &sAnswer,RString &sErrorOut), 
	void(*OnOK)(const RString &sAnswer), 
	void(*OnCancel)(),
	bool bPassword,
	bool (*ValidateAppend)(const RString &sAnswerBeforeChar, RString &sAppend),
	RString (*FormatAnswerForDisplay)(const RString &sAnswer)
	)
{	
	g_sQuestion = sQuestion;
	g_sInitialAnswer = sInitialAnswer;
	g_iMaxInputLength = iMaxInputLength;
	g_pValidate = Validate;
	g_pOnOK = OnOK;
	g_pOnCancel = OnCancel;
	g_pValidateAppend = ValidateAppend;
	g_pFormatAnswerForDisplay = FormatAnswerForDisplay;
}

void ScreenTextEntry::TextEntry( 
	ScreenMessage smSendOnPop, 
	RString sQuestion, 
	RString sInitialAnswer, 
	int iMaxInputLength,
	bool(*Validate)(const RString &sAnswer,RString &sErrorOut), 
	void(*OnOK)(const RString &sAnswer), 
	void(*OnCancel)(),
	bool bPassword,
	bool (*ValidateAppend)(const RString &sAnswerBeforeChar, RString &sAppend),
	RString (*FormatAnswerForDisplay)(const RString &sAnswer)
	)
{	
	g_sQuestion = sQuestion;
	g_sInitialAnswer = sInitialAnswer;
	g_iMaxInputLength = iMaxInputLength;
	g_pValidate = Validate;
	g_pOnOK = OnOK;
	g_pOnCancel = OnCancel;
	g_bPassword = bPassword;
	g_pValidateAppend = ValidateAppend;
	g_pFormatAnswerForDisplay = FormatAnswerForDisplay;

	SCREENMAN->AddNewScreenToTop( "ScreenTextEntry", smSendOnPop );
}

static LocalizedString INVALID_FLOAT( "ScreenTextEntry", "\"%s\" is an invalid floating point value." );
bool ScreenTextEntry::FloatValidate( const RString &sAnswer, RString &sErrorOut )
{
	float f;
	if( StringToFloat(sAnswer, f) )
		return true;
	sErrorOut = ssprintf( INVALID_FLOAT.GetValue(), sAnswer.c_str() );
	return false;
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
REGISTER_SCREEN_CLASS( ScreenTextEntryVisual );

void ScreenTextEntry::Init()
{
	ScreenWithMenuElements::Init();

	m_textQuestion.LoadFromFont( THEME->GetPathF(m_sName,"question") );
	m_textQuestion.SetName( "Question" );
	this->AddChild( &m_textQuestion );

	m_textAnswer.LoadFromFont( THEME->GetPathF(m_sName,"answer") );
	m_textAnswer.SetName( "Answer" );
	this->AddChild( &m_textAnswer );

	m_bShowAnswerCaret = false;

	m_sndType.Load( THEME->GetPathS(m_sName,"type"), true );
	m_sndBackspace.Load( THEME->GetPathS(m_sName,"backspace"), true );
}

void ScreenTextEntry::BeginScreen()
{
	m_sAnswer = RStringToWstring( g_sInitialAnswer );

	ScreenWithMenuElements::BeginScreen();

	m_textQuestion.SetText( g_sQuestion );
	SET_XY_AND_ON_COMMAND( m_textQuestion );
	SET_XY_AND_ON_COMMAND( m_textAnswer );

	UpdateAnswerText();
}

void ScreenTextEntry::UpdateAnswerText()
{
	RString s;
	if( g_bPassword )
		s = RString( m_sAnswer.size(), '*' );
	else
		s = WStringToRString(m_sAnswer);

	bool bAnswerFull = (int) s.length() >= g_iMaxInputLength;

	if( g_pFormatAnswerForDisplay )
		s = g_pFormatAnswerForDisplay( s );

	if( m_bShowAnswerCaret 	&&  !bAnswerFull )
		s += '_';
	else
		s += "  ";	// assumes that underscore is the width of two spaces

	FontCharAliases::ReplaceMarkers( s );
	m_textAnswer.SetText( s );
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
	if( IsTransitioning() )
		return;

	if( input.DeviceI == DeviceInput(DEVICE_KEYBOARD, KEY_BACK) )
	{
		switch( input.type )
		{
		case IET_FIRST_PRESS:
		case IET_REPEAT:
			BackspaceInAnswer();
			break;
		}
	}
	else if( input.type == IET_FIRST_PRESS )
	{
		wchar_t c = INPUTMAN->DeviceInputToChar(input.DeviceI,true);
		if( c >= ' ' ) 
		{
			TryAppendToAnswer( WStringToRString(wstring()+c) );

			TextEnteredDirectly();
		}
	}

	ScreenWithMenuElements::Input( input );
}

void ScreenTextEntry::TryAppendToAnswer( RString s )
{
	{
		wstring sNewAnswer = m_sAnswer+RStringToWstring(s);
		if( (int)sNewAnswer.length() > g_iMaxInputLength )
		{
			SCREENMAN->PlayInvalidSound();
			return;
		}
	}

	if( g_pValidateAppend  &&  !g_pValidateAppend( WStringToRString(m_sAnswer), s ) )
	{
		SCREENMAN->PlayInvalidSound();
		return;
	}

	wstring sNewAnswer = m_sAnswer+RStringToWstring(s);
	m_sAnswer = sNewAnswer;
	m_sndType.Play();
	UpdateAnswerText();
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

void ScreenTextEntry::MenuStart( const InputEventPlus &input )
{
	if( input.type==IET_FIRST_PRESS )
		End( false );
}

void ScreenTextEntry::TweenOffScreen()
{
	ScreenWithMenuElements::TweenOffScreen();

	OFF_COMMAND( m_textQuestion );
	OFF_COMMAND( m_textAnswer );
}

void ScreenTextEntry::End( bool bCancelled )
{
	if( bCancelled )
	{
		if( g_pOnCancel ) 
			g_pOnCancel();
		
		Cancel( SM_GoToNextScreen );
		TweenOffScreen();
	}
	else
	{
		RString sAnswer = WStringToRString(m_sAnswer);
		RString sError;
		if( g_pValidate != NULL )
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
			RString ret = WStringToRString(m_sAnswer);
			FontCharAliases::ReplaceMarkers(ret);
			g_pOnOK( ret );
		}

		StartTransitioningScreen( SM_GoToNextScreen );
		SCREENMAN->PlayStartSound();
	}

	s_bCancelledLast = bCancelled;
	s_sLastAnswer = bCancelled ? RString("") : WStringToRString(m_sAnswer);
}

void ScreenTextEntry::MenuBack( const InputEventPlus &input )
{
	if( input.type == IET_FIRST_PRESS )
		End( true );
}

void ScreenTextEntryVisual::Init()
{
	ROW_START_X.Load( m_sName, "RowStartX" );
	ROW_START_Y.Load( m_sName, "RowStartY" );
	ROW_END_X.Load( m_sName, "RowEndX" );
	ROW_END_Y.Load( m_sName, "RowEndY" );

	ScreenTextEntry::Init();

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
				pbt = dynamic_cast<BitmapText *>( text.Copy() ); // XXX: Copy() should be covariant
				this->AddChild( pbt );

				RString s = g_szKeys[r][x];
				if( !s.empty()  &&  r == KEYBOARD_ROW_SPECIAL )
					s = THEME->GetString( m_sName, s );
				pbt->SetText( s );
			}
		}
	}

	m_sndChange.Load( THEME->GetPathS(m_sName,"change"), true );
}

ScreenTextEntryVisual::~ScreenTextEntryVisual()
{
	FOREACH_KeyboardRow( r )
		for( int x=0; x<KEYS_PER_ROW; ++x )
			SAFE_DELETE( m_ptextKeys[r][x] );
}

void ScreenTextEntryVisual::BeginScreen()
{
	ScreenTextEntry::BeginScreen();

	ON_COMMAND( m_sprCursor );
	m_iFocusX = 0;
	m_iFocusY = (KeyboardRow)0;

	FOREACH_KeyboardRow( r )
	{
		for( int x=0; x<KEYS_PER_ROW; ++x )
		{
			BitmapText &bt = *m_ptextKeys[r][x];
			float fX = roundf( SCALE( x, 0, KEYS_PER_ROW-1, ROW_START_X, ROW_END_X ) );
			float fY = roundf( SCALE( r, 0, NUM_KEYBOARD_ROWS-1, ROW_START_Y, ROW_END_Y ) );
			bt.SetXY( fX, fY );
		}
	}

	PositionCursor();
}

void ScreenTextEntryVisual::PositionCursor()
{
	BitmapText &bt = *m_ptextKeys[m_iFocusY][m_iFocusX];
	m_sprCursor->SetXY( bt.GetX(), bt.GetY() );
	m_sprCursor->PlayCommand( m_iFocusY == KEYBOARD_ROW_SPECIAL ? "SpecialKey" : "RegularKey" );
}

void ScreenTextEntryVisual::TextEnteredDirectly()
{
	// If the user enters text with a keyboard, jump to DONE, so enter ends the screen.
	m_iFocusY = KEYBOARD_ROW_SPECIAL;
	m_iFocusX = DONE;

	PositionCursor();
}

void ScreenTextEntryVisual::MoveX( int iDir )
{
	RString sKey;
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

void ScreenTextEntryVisual::MoveY( int iDir )
{
	RString sKey;
	do
	{
		m_iFocusY = enum_add2( m_iFocusY,  +iDir );
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

void ScreenTextEntryVisual::MenuStart( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS )
		return;
	if( m_iFocusY == KEYBOARD_ROW_SPECIAL )
	{
		switch( m_iFocusX )
		{
		case SPACEBAR:
			TryAppendToAnswer( " " );
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
		TryAppendToAnswer( g_szKeys[m_iFocusY][m_iFocusX] );
	}
}

void ScreenTextEntryVisual::TweenOffScreen()
{
	ScreenTextEntry::TweenOffScreen();

	OFF_COMMAND( m_sprCursor );
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
