#include "global.h"
#include "ScreenTextEntry.h"
#include "RageUtil.h"
#include "Preference.h"
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "FontCharAliases.h"
#include "ScreenDimensions.h"
#include "ScreenPrompt.h"
#include "ActorUtil.h"
#include "InputEventPlus.h"
#include "RageInput.h"
#include "LocalizedString.h"
#include "RageLog.h"
#include "LuaBinding.h"

static const char* g_szKeys[NUM_KeyboardRow][KEYS_PER_ROW] =
{
	{"A","B","C","D","E","F","G","H","I","J","K","L","M"},
	{"N","O","P","Q","R","S","T","U","V","W","X","Y","Z"},
	{"a","b","c","d","e","f","g","h","i","j","k","l","m"},
	{"n","o","p","q","r","s","t","u","v","w","x","y","z"},
	{"0","1","2","3","4","5","6","7","8","9","", "", "" },
	{"!","@","#","$","%","^","&","(",")","[","]","{","}"},
	{"+","-","=","_",",",".","'","\"",":","", "", "", ""},
	{"","","Space","","","Backspace","","","Cancel","","","Done",""},
};

RString ScreenTextEntry::s_sLastAnswer = "";

// Settings:
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

	// Lua bridge
	LuaReference g_ValidateFunc;
	LuaReference g_OnOKFunc;
	LuaReference g_OnCancelFunc;
	LuaReference g_ValidateAppendFunc;
	LuaReference g_FormatAnswerForDisplayFunc;
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

/* Handle UTF-8. Right now, we need to at least be able to backspace a whole
 * UTF-8 character. Better would be to operate in wchar_t.
 *
 * XXX: Don't allow internal-use codepoints (above 0xFFFF); those are subject to
 * change and shouldn't be written to disk. */
REGISTER_SCREEN_CLASS( ScreenTextEntry );
REGISTER_SCREEN_CLASS( ScreenTextEntryVisual );

void ScreenTextEntry::Init()
{
	ScreenWithMenuElements::Init();

	m_textQuestion.LoadFromFont( THEME->GetPathF(m_sName,"question") );
	m_textQuestion.SetName( "Question" );
	LOAD_ALL_COMMANDS( m_textQuestion );
	this->AddChild( &m_textQuestion );

	m_textAnswer.LoadFromFont( THEME->GetPathF(m_sName,"answer") );
	m_textAnswer.SetName( "Answer" );
	LOAD_ALL_COMMANDS( m_textAnswer );
	this->AddChild( &m_textAnswer );

	m_bShowAnswerCaret = false;
	//m_iCaretLocation = 0;

	m_sndType.Load( THEME->GetPathS(m_sName,"type"), true );
	m_sndBackspace.Load( THEME->GetPathS(m_sName,"backspace"), true );
}

void ScreenTextEntry::BeginScreen()
{
	m_sAnswer = RStringToWstring( g_sInitialAnswer );

	ScreenWithMenuElements::BeginScreen();

	m_textQuestion.SetText( g_sQuestion );
	SET_XY( m_textQuestion );
	SET_XY( m_textAnswer );

	UpdateAnswerText();
}

static LocalizedString ANSWER_CARET	( "ScreenTextEntry", "AnswerCaret" );
static LocalizedString ANSWER_BLANK	( "ScreenTextEntry", "AnswerBlank" );
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

	// Handle caret drawing
	//m_iCaretLocation = s.length()
	if( m_bShowAnswerCaret 	&&  !bAnswerFull )
		s += ANSWER_CARET; // was '_'
	else
	{
		s += ANSWER_BLANK; // was "  "
	}

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

bool ScreenTextEntry::Input( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return false;

	bool bHandled = false;
	if( input.DeviceI == DeviceInput(DEVICE_KEYBOARD, KEY_BACK) )
	{
		switch( input.type )
		{
			case IET_FIRST_PRESS:
			case IET_REPEAT:
				BackspaceInAnswer();
				bHandled = true;
			default:
				break;
		}
	}
	else if( input.type == IET_FIRST_PRESS )
	{
		wchar_t c = INPUTMAN->DeviceInputToChar(input.DeviceI,true);
		if( c >= L' ' ) 
		{
			// todo: handle caps lock -aj
			TryAppendToAnswer( WStringToRString(wstring()+c) );

			TextEnteredDirectly();
			bHandled = true;
		}
	}

	return ScreenWithMenuElements::Input( input ) || bHandled;
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

bool ScreenTextEntry::MenuStart( const InputEventPlus &input )
{
	// HACK: Only allow the screen to end on the Enter key.-aj
	if( input.DeviceI == DeviceInput(DEVICE_KEYBOARD, KEY_ENTER) && input.type==IET_FIRST_PRESS )
	{
		End( false );
		return true;
	}
	return false;
}

void ScreenTextEntry::End( bool bCancelled )
{
	if( bCancelled )
	{
		if( g_pOnCancel ) 
			g_pOnCancel();

		Cancel( SM_GoToNextScreen );
		//TweenOffScreen();
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

bool ScreenTextEntry::MenuBack( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS )
		return false;
	End( true );
	return true;
}

void ScreenTextEntry::TextEntrySettings::FromStack( lua_State *L )
{
	if( lua_type(L, 1) != LUA_TTABLE )
	{
		LOG->Trace("not a table");
		return;
	}

	lua_pushvalue( L, 1 );
	const int iTab = lua_gettop( L );

	// Get ScreenMessage
	lua_getfield( L, iTab, "SendOnPop" );
	const char *pStr = lua_tostring( L, -1 );
	if( pStr == NULL )
		smSendOnPop = SM_None;
	else
		smSendOnPop = ScreenMessageHelpers::ToScreenMessage( pStr );
	lua_settop( L, iTab );

	// Get Question
	lua_getfield( L, iTab, "Question" );
	pStr = lua_tostring( L, -1 );
	if( pStr == NULL )
		RageException::Throw( "\"Question\" entry is not a string." );
	sQuestion = pStr;
	lua_settop( L, iTab );

	// Get Initial Answer
	lua_getfield( L, iTab, "InitialAnswer" );
	pStr = lua_tostring( L, -1 );
	if( pStr == NULL )
		pStr = "";
	sInitialAnswer = pStr;
	lua_settop( L, iTab );

	// Get Max Input Length
	lua_getfield( L, iTab, "MaxInputLength" );
	iMaxInputLength = lua_tointeger( L, -1 );
	lua_settop( L, iTab );

	// Get Password
	lua_getfield( L, iTab, "Password" );
	bPassword = !!lua_toboolean( L, -1 );
	lua_settop( L, iTab );

	// and now the hard part, the functions.
	// Validate
	lua_getfield( L, iTab, "Validate" );
	if( !lua_isfunction( L, -1 ) && !lua_isnil( L, -1 ) )
		RageException::Throw( "\"Validate\" is not a function." );
	Validate.SetFromStack( L );
	lua_settop( L, iTab );

	// OnOK
	lua_getfield( L, iTab, "OnOK" );
	if( !lua_isfunction( L, -1 ) && !lua_isnil( L, -1 ) )
		RageException::Throw( "\"OnOK\" is not a function." );
	OnOK.SetFromStack( L );
	lua_settop( L, iTab );

	// OnCancel
	lua_getfield( L, iTab, "OnCancel" );
	if( !lua_isfunction( L, -1 ) && !lua_isnil( L, -1 ) )
		RageException::Throw( "\"OnCancel\" is not a function." );
	OnCancel.SetFromStack( L );
	lua_settop( L, iTab );

	// ValidateAppend
	lua_getfield( L, iTab, "ValidateAppend" );
	if( !lua_isfunction( L, -1 ) && !lua_isnil( L, -1 ) )
		RageException::Throw( "\"ValidateAppend\" is not a function." );
	ValidateAppend.SetFromStack( L );
	lua_settop( L, iTab );

	// FormatAnswerForDisplay
	lua_getfield( L, iTab, "FormatAnswerForDisplay" );
	if( !lua_isfunction( L, -1 ) && !lua_isnil( L, -1 ) )
		RageException::Throw( "\"FormatAnswerForDisplay\" is not a function." );
	FormatAnswerForDisplay.SetFromStack( L );
	lua_settop( L, iTab );
}

// Lua bridges
static bool ValidateFromLua( const RString &sAnswer, RString &sErrorOut )
{
	Lua *L = LUA->Get();

	if( g_ValidateFunc.IsNil() )
	{
		LUA->Release(L);
		return true;
	}

	g_ValidateFunc.PushSelf( L );

	// Argument 1 (answer):
	lua_pushstring( L, sAnswer );

	// Argument 2 (error out):
	lua_pushstring( L, sErrorOut );

	lua_call( L, 2, 2 ); // call function with 2 arguments and 2 results

	if( !lua_isstring(L, -1) )
		RageException::Throw( "\"Validate\" did not return a string." );

	if( !lua_isboolean(L, -2) )
		RageException::Throw( "\"Validate\" did not return a boolean." );

	RString sErrorFromLua;
	LuaHelpers::Pop( L, sErrorFromLua );
	if( !sErrorFromLua.empty() )
		sErrorOut = sErrorFromLua;

	bool bValidate;
	LuaHelpers::Pop( L, bValidate );

	LUA->Release(L);
	return bValidate;
}

static void OnOKFromLua( const RString &sAnswer )
{
	Lua *L = LUA->Get();

	if( g_OnOKFunc.IsNil() )
	{
		LUA->Release(L);
		return;
	}

	g_OnOKFunc.PushSelf( L );
	// Argument 1 (answer):
	lua_pushstring( L, sAnswer );
	lua_call( L, 1, 0 ); // call function with 1 argument and 0 results

	LUA->Release(L);
}

static void OnCancelFromLua()
{
	Lua *L = LUA->Get();

	if( g_OnCancelFunc.IsNil() )
	{
		LUA->Release(L);
		return;
	}

	g_OnCancelFunc.PushSelf( L );
	lua_call( L, 0, 0 ); // call function with 0 arguments and 0 results

	LUA->Release(L);
}

static bool ValidateAppendFromLua( const RString &sAnswerBeforeChar, RString &sAppend )
{
	Lua *L = LUA->Get();

	if( g_ValidateAppendFunc.IsNil() )
	{
		LUA->Release(L);
		return true;
	}

	g_ValidateAppendFunc.PushSelf( L );

	// Argument 1 (AnswerBeforeChar):
	lua_pushstring( L, sAnswerBeforeChar );

	// Argument 2 (Append):
	lua_pushstring( L, sAppend );
	lua_call( L, 2, 1 ); // call function with 2 arguments and 1 result

	if( !lua_isboolean(L, -1) )
		RageException::Throw( "\"ValidateAppend\" did not return a boolean." );

	bool bAppend;
	LuaHelpers::Pop( L, bAppend );

	LUA->Release(L);
	return bAppend;
}

static RString FormatAnswerForDisplayFromLua( const RString &sAnswer )
{
	Lua *L = LUA->Get();

	if( g_FormatAnswerForDisplayFunc.IsNil() )
	{
		LUA->Release(L);
		return sAnswer;
	}

	g_FormatAnswerForDisplayFunc.PushSelf( L );
	// Argument 1 (Answer):
	lua_pushstring( L, sAnswer );
	lua_call( L, 1, 1 ); // call function with 1 argument and 1 result

	if( !lua_isstring(L, -1) )
		RageException::Throw( "\"FormatAnswerForDisplay\" did not return a string." );

	RString sAnswerFromLua;
	LuaHelpers::Pop( L, sAnswerFromLua );

	LUA->Release(L);
	return sAnswerFromLua;
}

void ScreenTextEntry::LoadFromTextEntrySettings( const TextEntrySettings &settings )
{
	g_ValidateFunc = settings.Validate;
	g_OnOKFunc = settings.OnOK;
	g_OnCancelFunc = settings.OnCancel;
	g_ValidateAppendFunc = settings.ValidateAppend;
	g_FormatAnswerForDisplayFunc = settings.FormatAnswerForDisplay;

	// set functions
	SetTextEntrySettings(
		settings.sQuestion,
		settings.sInitialAnswer,
		settings.iMaxInputLength,
		ValidateFromLua,				// Validate
		OnOKFromLua,					// OnOK
		OnCancelFromLua,				// OnCancel
		settings.bPassword,
		ValidateAppendFromLua,			// ValidateAppend
		FormatAnswerForDisplayFromLua	// FormatAnswerForDisplay
	);

	// Hack: reload screen with new info
	BeginScreen();
}

/** @brief Allow Lua to have access to the ScreenTextEntry. */
class LunaScreenTextEntry: public Luna<ScreenTextEntry>
{
public:
	static int Load( T* p, lua_State *L )
	{
		ScreenTextEntry::TextEntrySettings settings;
		settings.FromStack( L );
		p->LoadFromTextEntrySettings(settings);
		return 0;
	}

	LunaScreenTextEntry()
	{
		ADD_METHOD( Load );
	}
};
LUA_REGISTER_DERIVED_CLASS( ScreenTextEntry, ScreenWithMenuElements )
// lua end

// begin ScreenTextEntryVisual
void ScreenTextEntryVisual::Init()
{
	ROW_START_X.Load( m_sName, "RowStartX" );
	ROW_START_Y.Load( m_sName, "RowStartY" );
	ROW_END_X.Load( m_sName, "RowEndX" );
	ROW_END_Y.Load( m_sName, "RowEndY" );

	ScreenTextEntry::Init();

	m_sprCursor.Load( THEME->GetPathG(m_sName,"cursor") );
	m_sprCursor->SetName( "Cursor" );
	LOAD_ALL_COMMANDS( m_sprCursor );
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
				pbt = text.Copy();
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

	m_iFocusX = 0;
	m_iFocusY = (KeyboardRow)0;

	FOREACH_KeyboardRow( r )
	{
		for( int x=0; x<KEYS_PER_ROW; ++x )
		{
			BitmapText &bt = *m_ptextKeys[r][x];
			float fX = roundf( SCALE( x, 0, KEYS_PER_ROW-1, ROW_START_X, ROW_END_X ) );
			float fY = roundf( SCALE( r, 0, NUM_KeyboardRow-1, ROW_START_Y, ROW_END_Y ) );
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
		wrap( *ConvertValue<int>(&m_iFocusY), NUM_KeyboardRow );

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

bool ScreenTextEntryVisual::MenuLeft( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS )
		return false;
	MoveX(-1);
	return true;
}
bool ScreenTextEntryVisual::MenuRight( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS )
		return false;
	MoveX(+1);
	return true;
}
bool ScreenTextEntryVisual::MenuUp( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS )
		return false;
	MoveY(-1);
	return true;
}
bool ScreenTextEntryVisual::MenuDown( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS )
		return false;
	MoveY(+1);
	return true;
}

bool ScreenTextEntryVisual::MenuStart( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS )
		return false;
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
			return false;
		}
	}
	else
	{
		TryAppendToAnswer( g_szKeys[m_iFocusY][m_iFocusX] );
	}
	return true;
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
