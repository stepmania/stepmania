#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenNetSelectBase.h"
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "RageTimer.h"
#include "ActorUtil.h"
#include "Actor.h"
#include "GameSoundManager.h"
#include "MenuTimer.h"
#include "NetworkSyncManager.h"
#include "RageUtil.h"
#include "GameState.h"
#include "InputEventPlus.h"
#include "RageInput.h"
#include "Font.h"
#include "RageDisplay.h"

#define CHAT_TEXT_OUTPUT_WIDTH		THEME->GetMetricF(m_sName,"ChatTextOutputWidth")
#define CHAT_TEXT_INPUT_WIDTH		THEME->GetMetricF(m_sName,"ChatTextInputWidth")
#define SHOW_CHAT_LINES				THEME->GetMetricI(m_sName,"ChatOutputLines")

#define USERS_X						THEME->GetMetricF(m_sName,"UsersX")
#define USERS_Y						THEME->GetMetricF(m_sName,"UsersY")
#define USER_SPACING_X				THEME->GetMetricF(m_sName,"UserSpacingX")
#define USER_ADD_Y					THEME->GetMetricF(m_sName,"UserLine2Y")

AutoScreenMessage( SM_AddToChat );
AutoScreenMessage( SM_UsersUpdate );
AutoScreenMessage( SM_FriendsUpdate );
AutoScreenMessage( SM_SMOnlinePack );

REGISTER_SCREEN_CLASS( ScreenNetSelectBase );

void ScreenNetSelectBase::Init()
{
	ScreenWithMenuElements::Init();

	// Chat boxes
	m_sprChatInputBox.Load( THEME->GetPathG( m_sName, "ChatInputBox" ) );
	m_sprChatInputBox->SetName( "ChatInputBox" );
	LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( m_sprChatInputBox );
	this->AddChild( m_sprChatInputBox );

	m_sprChatOutputBox.Load( THEME->GetPathG( m_sName, "ChatOutputBox" ) );
	m_sprChatOutputBox->SetName( "ChatOutputBox" );
	LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( m_sprChatOutputBox );
	this->AddChild( m_sprChatOutputBox );

	m_textChatInput.LoadFromFont( THEME->GetPathF(m_sName,"chat") );
	m_textChatInput.SetName( "ChatInput" );
	m_textChatInput.SetWrapWidthPixels( (int)(CHAT_TEXT_INPUT_WIDTH) );
	LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( m_textChatInput );
	this->AddChild( &m_textChatInput );

	m_textChatOutput.LoadFromFont( THEME->GetPathF(m_sName,"chat") );
	m_textChatOutput.SetName( "ChatOutput" );
	m_textChatOutput.SetWrapWidthPixels( (int)(CHAT_TEXT_OUTPUT_WIDTH) );
	LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( m_textChatOutput );
	this->AddChild( &m_textChatOutput );

	m_textChatOutput.SetText( NSMAN->m_sChatText );
	m_textChatOutput.SetMaxLines( SHOW_CHAT_LINES, 1 );

	scroll = 0;

	//Display users list
	UpdateUsers();

	return;
}

bool ScreenNetSelectBase::Input( const InputEventPlus &input )
{
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return false;

	if( input.type != IET_FIRST_PRESS && input.type != IET_REPEAT )
		return false;

	bool bHoldingCtrl = 
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL)) ||
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL)) ||
		(!NSMAN->useSMserver);	// If we are disconnected, assume no chatting.

	switch( input.DeviceI.button )
	{
	case KEY_PGUP:
		if (!bHoldingCtrl) {
			ShowPreviousMsg();
			break;
		}
		else {
			Scroll(1);
			Scroll(1);
			break;
		}
	case KEY_PGDN:
		if (!bHoldingCtrl) {
			ShowNextMsg();
			break;
		}
		else {
			Scroll(-1);
			Scroll(-1);
			break;
		}
	case KEY_ENTER:
	case KEY_KP_ENTER:
		if (!bHoldingCtrl)
		{
			if (m_sTextInput != "") {
				NSMAN->SendChat(m_sTextInput);
				m_sTextLastestInputs.push_back(m_sTextInput);
				m_sTextLastestInputsIndex = 0;
				if (m_sTextLastestInputs.size() > 10)
					m_sTextLastestInputs.erase(m_sTextLastestInputs.begin());
			}
			m_sTextInput = "";
			UpdateTextInput();
			return true;
		}
		break;
	case KEY_BACK:
		if(!m_sTextInput.empty())
			m_sTextInput = m_sTextInput.erase( m_sTextInput.size()-1 );
		UpdateTextInput();
		break;
	default:
		wchar_t c;
		c = INPUTMAN->DeviceInputToChar(input.DeviceI, true);

		if( (c >= L' ') && (!bHoldingCtrl) )
		{
			if (!enableChatboxInput)
				return true;
			m_sTextInput += WStringToRString(wstring() + c);
			UpdateTextInput();
		}

		// Tricky: If both players are playing, allow the 2 button through to
		// the keymapper. (who? -aj)
		// What purpose does this serve? -aj
		if( c == '2' && GAMESTATE->IsPlayerEnabled( PLAYER_2 ) && GAMESTATE->IsPlayerEnabled( PLAYER_1 ) )
			break;

		if( c >= ' ' )
			return true;
		break;
	}
	return Screen::Input( input );
}

void ScreenNetSelectBase::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToNextScreen )
		SOUND->StopMusic();
	else if( SM == SM_AddToChat )
	{
		m_textChatOutput.SetText( NSMAN->m_sChatText );
		m_textChatOutput.SetMaxLines( SHOW_CHAT_LINES, 1 );
	}
	else if( SM == SM_UsersUpdate )
	{
		UpdateUsers();
	}
	else if (SM == SM_FriendsUpdate)
	{
		MESSAGEMAN->Broadcast("FriendsUpdate");
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

void ScreenNetSelectBase::TweenOffScreen()
{
	OFF_COMMAND( m_sprChatInputBox );
	OFF_COMMAND( m_sprChatOutputBox );
	OFF_COMMAND( m_textChatInput );
	OFF_COMMAND( m_textChatOutput );

	for( unsigned i=0; i<m_textUsers.size(); i++ )
		OFF_COMMAND( m_textUsers[i] );
}

void ScreenNetSelectBase::UpdateTextInput()
{
	m_textChatInput.SetText( m_sTextInput );  
}

void ScreenNetSelectBase::UpdateUsers()
{
	float tX = USERS_X - USER_SPACING_X;
	float tY = USERS_Y;

	for( unsigned i=0; i< m_textUsers.size(); i++)
		this->RemoveChild( &m_textUsers[i] );

	m_textUsers.clear();

	m_textUsers.resize( NSMAN->m_ActivePlayer.size() );

	for( unsigned i=0; i < NSMAN->m_ActivePlayer.size(); i++)
	{
		m_textUsers[i].LoadFromFont( THEME->GetPathF(m_sName,"users") );
		m_textUsers[i].SetHorizAlign( align_center );
		m_textUsers[i].SetVertAlign( align_top );
		m_textUsers[i].SetShadowLength( 0 );
		m_textUsers[i].SetName( "Users" );

		tX += USER_SPACING_X;

		if ( (i % 2) == 1)
			tY = USER_ADD_Y + USERS_Y;
		else
			tY = USERS_Y;
		m_textUsers[i].SetXY( tX, tY );

		ActorUtil::LoadAllCommands( m_textUsers[i], m_sName );
		ActorUtil::OnCommand( m_textUsers[i] );

		m_textUsers[i].SetText( NSMAN->m_PlayerNames[NSMAN->m_ActivePlayer[i]] );
		m_textUsers[i].RunCommands( THEME->GetMetricA( m_sName,
			ssprintf("Users%dCommand", NSMAN->m_PlayerStatus[NSMAN->m_ActivePlayer[i]] ) ) );

		this->AddChild( &m_textUsers[i] );
	}
	if (!usersVisible)
		for (unsigned i = 0; i < NSMAN->m_ActivePlayer.size(); i++)
			m_textUsers[i].SetVisible(false);
	MESSAGEMAN->Broadcast("UsersUpdate");
}

void ScreenNetSelectBase::Scroll(int movescroll)
{
	if (scroll + movescroll >= 0 && scroll + movescroll <= m_textChatOutput.lines - SHOW_CHAT_LINES)
		scroll += movescroll;
	m_textChatOutput.ResetText();
	m_textChatOutput.SetMaxLines(SHOW_CHAT_LINES, 1, scroll);
	return;
}

RString ScreenNetSelectBase::GetPreviousMsg()
{
	m_sTextLastestInputsIndex += 1;
	if (m_sTextLastestInputsIndex <= m_sTextLastestInputs.size() && m_sTextLastestInputsIndex > 0)
		return m_sTextLastestInputs[m_sTextLastestInputs.size() - m_sTextLastestInputsIndex];
	m_sTextLastestInputsIndex = m_sTextLastestInputs.size();
	return m_sTextLastestInputsIndex == 0 ? "" : m_sTextLastestInputs[m_sTextLastestInputs.size() - m_sTextLastestInputsIndex];
}

RString ScreenNetSelectBase::GetNextMsg()
{
	m_sTextLastestInputsIndex -= 1;
	if (m_sTextLastestInputsIndex <= m_sTextLastestInputs.size() && m_sTextLastestInputsIndex > 0)
		return m_sTextLastestInputs[m_sTextLastestInputs.size() - m_sTextLastestInputsIndex];
	m_sTextLastestInputsIndex = 0;
	return "";
}
void ScreenNetSelectBase::ShowPreviousMsg()
{
	SetInputText(GetPreviousMsg());
	return;
}
void ScreenNetSelectBase::ShowNextMsg()
{
	SetInputText(GetNextMsg());
	return;
}
void ScreenNetSelectBase::SetInputText(RString text)
{
	m_sTextInput = text;
	UpdateTextInput();
	return;
}

/** ColorBitmapText ***********************************************************/
void ColorBitmapText::SetText( const RString& _sText, const RString& _sAlternateText, int iWrapWidthPixels )
{
	ASSERT( m_pFont != nullptr );

	RString sNewText = StringWillUseAlternate(_sText,_sAlternateText) ? _sAlternateText : _sText;

	if( iWrapWidthPixels == -1 )	// wrap not specified
		iWrapWidthPixels = m_iWrapWidthPixels;

	if( m_sText == sNewText && iWrapWidthPixels==m_iWrapWidthPixels )
		return;
	m_sText = sNewText;
	m_iWrapWidthPixels = iWrapWidthPixels;

	// Set up the first color.
	m_vColors.clear();
	ColorChange change;
	change.c = RageColor (1, 1, 1, 1);
	change.l = 0;
	m_vColors.push_back( change );

	m_wTextLines.clear();

	RString sCurrentLine = "";
	int		iLineWidth = 0;

	RString sCurrentWord = "";
	int		iWordWidth = 0;
	int		iGlyphsSoFar = 0;

	for( unsigned i = 0; i < m_sText.length(); i++ )
	{
		int iCharsLeft = m_sText.length() - i - 1;

		// First: Check for the special (color) case.

		if( m_sText.length() > 8 && i < m_sText.length() - 9 )
		{
			RString FirstThree = m_sText.substr( i, 3 );
			if( FirstThree.CompareNoCase("|c0") == 0 && iCharsLeft > 8 )
			{
				ColorChange cChange;
				unsigned int r, g, b;
				sscanf( m_sText.substr( i, 9 ).c_str(), "|%*c0%2x%2x%2x", &r, &g, &b );
				cChange.c = RageColor( r/255.f, g/255.f, b/255.f, 1.f );
				cChange.l = iGlyphsSoFar;
				if( iGlyphsSoFar == 0 )
					m_vColors[0] = cChange;
				else
					m_vColors.push_back( cChange );
				i+=8;
				continue;
			}
		}

		int iCharLength = min( utf8_get_char_len(m_sText[i]), iCharsLeft + 1 );
		RString curCharStr = m_sText.substr( i, iCharLength );
		wchar_t curChar = utf8_get_char( curCharStr );
		i += iCharLength - 1;
		int iCharWidth = m_pFont->GetLineWidthInSourcePixels( wstring() + curChar );

		switch( curChar )
		{
			case L' ':
				if( /* iLineWidth == 0 &&*/ iWordWidth == 0 )
					break;
				sCurrentLine += sCurrentWord + " ";
				iLineWidth += iWordWidth + iCharWidth;
				sCurrentWord = "";
				iWordWidth = 0;
				iGlyphsSoFar++;
				break;
			case L'\n':
				if( iLineWidth + iWordWidth > iWrapWidthPixels )
				{
					SimpleAddLine( sCurrentLine, iLineWidth );
					if( iWordWidth > 0 )
						iLineWidth = iWordWidth +	//Add the width of a space
							m_pFont->GetLineWidthInSourcePixels( L" " );
					sCurrentLine = sCurrentWord + " ";
					iWordWidth = 0;
					sCurrentWord = "";
					iGlyphsSoFar++;
				} 
				else
				{
					SimpleAddLine( sCurrentLine + sCurrentWord, iLineWidth + iWordWidth );
					sCurrentLine = "";	iLineWidth = 0;
					sCurrentWord = "";	iWordWidth = 0;
				}
				break;
			default:
				if( iWordWidth + iCharWidth > iWrapWidthPixels && iLineWidth == 0 )
				{
					SimpleAddLine( sCurrentWord, iWordWidth );
					sCurrentWord = curCharStr;  iWordWidth = iCharWidth;
				}
				else if( iWordWidth + iLineWidth + iCharWidth > iWrapWidthPixels )
				{
					SimpleAddLine( sCurrentLine, iLineWidth );
					sCurrentLine = ""; 
					iLineWidth = 0;
					sCurrentWord += curCharStr;
					iWordWidth += iCharWidth;
				}
				else
				{
					sCurrentWord += curCharStr;
					iWordWidth += iCharWidth;
				}
				iGlyphsSoFar++;
				break;
		}
	}

	if( iWordWidth > 0 )
	{
		sCurrentLine += sCurrentWord;
		iLineWidth += iWordWidth;
	}

	if( iLineWidth > 0 )
		SimpleAddLine( sCurrentLine, iLineWidth );

	lines = m_wTextLines.size();

	BuildChars();
	UpdateBaseZoom();
}

void ColorBitmapText::ResetText()
{
	ASSERT(m_pFont != nullptr);

	int iWrapWidthPixels = m_iWrapWidthPixels;

	// Set up the first color.
	m_vColors.clear();
	ColorChange change;
	change.c = RageColor(1, 1, 1, 1);
	change.l = 0;
	m_vColors.push_back(change);

	m_wTextLines.clear();

	RString sCurrentLine = "";
	int		iLineWidth = 0;

	RString sCurrentWord = "";
	int		iWordWidth = 0;
	int		iGlyphsSoFar = 0;

	for (unsigned i = 0; i < m_sText.length(); i++)
	{
		int iCharsLeft = m_sText.length() - i - 1;

		// First: Check for the special (color) case.

		if (m_sText.length() > 8 && i < m_sText.length() - 9)
		{
			RString FirstThree = m_sText.substr(i, 3);
			if (FirstThree.CompareNoCase("|c0") == 0 && iCharsLeft > 8)
			{
				ColorChange cChange;
				unsigned int r, g, b;
				sscanf(m_sText.substr(i, 9).c_str(), "|%*c0%2x%2x%2x", &r, &g, &b);
				cChange.c = RageColor(r / 255.f, g / 255.f, b / 255.f, 1.f);
				cChange.l = iGlyphsSoFar;
				if (iGlyphsSoFar == 0)
					m_vColors[0] = cChange;
				else
					m_vColors.push_back(cChange);
				i += 8;
				continue;
			}
		}

		int iCharLength = min(utf8_get_char_len(m_sText[i]), iCharsLeft + 1);
		RString curCharStr = m_sText.substr(i, iCharLength);
		wchar_t curChar = utf8_get_char(curCharStr);
		i += iCharLength - 1;
		int iCharWidth = m_pFont->GetLineWidthInSourcePixels(wstring() + curChar);

		switch (curChar)
		{
		case L' ':
			if ( /* iLineWidth == 0 &&*/ iWordWidth == 0)
				break;
			sCurrentLine += sCurrentWord + " ";
			iLineWidth += iWordWidth + iCharWidth;
			sCurrentWord = "";
			iWordWidth = 0;
			iGlyphsSoFar++;
			break;
		case L'\n':
			if (iLineWidth + iWordWidth > iWrapWidthPixels)
			{
				SimpleAddLine(sCurrentLine, iLineWidth);
				if (iWordWidth > 0)
					iLineWidth = iWordWidth +	//Add the width of a space
					m_pFont->GetLineWidthInSourcePixels(L" ");
				sCurrentLine = sCurrentWord + " ";
				iWordWidth = 0;
				sCurrentWord = "";
				iGlyphsSoFar++;
			}
			else
			{
				SimpleAddLine(sCurrentLine + sCurrentWord, iLineWidth + iWordWidth);
				sCurrentLine = "";	iLineWidth = 0;
				sCurrentWord = "";	iWordWidth = 0;
			}
			break;
		default:
			if (iWordWidth + iCharWidth > iWrapWidthPixels && iLineWidth == 0)
			{
				SimpleAddLine(sCurrentWord, iWordWidth);
				sCurrentWord = curCharStr;  iWordWidth = iCharWidth;
			}
			else if (iWordWidth + iLineWidth + iCharWidth > iWrapWidthPixels)
			{
				SimpleAddLine(sCurrentLine, iLineWidth);
				sCurrentLine = "";
				iLineWidth = 0;
				sCurrentWord += curCharStr;
				iWordWidth += iCharWidth;
			}
			else
			{
				sCurrentWord += curCharStr;
				iWordWidth += iCharWidth;
			}
			iGlyphsSoFar++;
			break;
		}
	}

	if (iWordWidth > 0)
	{
		sCurrentLine += sCurrentWord;
		iLineWidth += iWordWidth;
	}

	if (iLineWidth > 0)
		SimpleAddLine(sCurrentLine, iLineWidth);
	lines = m_wTextLines.size();
	BuildChars();
	UpdateBaseZoom();
}

void ColorBitmapText::SetMaxLines(int iNumLines, int iDirection, unsigned int &scroll)
{
	iNumLines = max(0, iNumLines);
	iNumLines = min((int)m_wTextLines.size(), iNumLines);
	if (iDirection == 0)
	{
		// Crop all bottom lines
		m_wTextLines.resize(iNumLines);
		m_iLineWidths.resize(iNumLines);
	}
	else
	{
		// Because colors are relative to the beginning, we have to crop them back
		unsigned shift = 0;
		if (scroll >  m_iLineWidths.size() - iNumLines)
			scroll = m_iLineWidths.size() - iNumLines;

		for (unsigned i = 0; i < m_wTextLines.size() - iNumLines - scroll; i++)
			shift += m_wTextLines[i].length();

		// When we're cutting out text, we need to maintain the last
		// color, so our text at the top doesn't become colorless.
		RageColor LastColor;

		for (unsigned i = 0; i < m_vColors.size(); i++)
		{
			m_vColors[i].l -= shift;
			if (m_vColors[i].l < 0)
			{
				LastColor = m_vColors[i].c;
				m_vColors.erase(m_vColors.begin() + i);
				i--;
			}
		}

		// If we already have a color set for the first char
		// do not override it.
		if (m_vColors.size() > 0 && m_vColors[0].l > 0)
		{
			ColorChange tmp;
			tmp.c = LastColor;
			tmp.l = 0;
			m_vColors.insert(m_vColors.begin(), tmp);
		}

		if (scroll == 0 || m_iLineWidths.size() <= iNumLines || scroll > m_iLineWidths.size() - iNumLines) {
			m_wTextLines.erase(m_wTextLines.begin(), m_wTextLines.end() - iNumLines);
			m_iLineWidths.erase(m_iLineWidths.begin(), m_iLineWidths.end() - iNumLines);
		}
		else {
			m_wTextLines.erase(m_wTextLines.begin(), m_wTextLines.end() - iNumLines - scroll);
			m_iLineWidths.erase(m_iLineWidths.begin(), m_iLineWidths.end() - iNumLines - scroll);

			m_wTextLines.erase(m_wTextLines.end() - scroll, m_wTextLines.end());
			m_iLineWidths.erase(m_iLineWidths.begin(), m_iLineWidths.end());
		}
	}
	BuildChars();
}

void ColorBitmapText::SimpleAddLine( const RString &sAddition, const int iWidthPixels) 
{
	m_wTextLines.push_back( RStringToWstring( sAddition ) );
	m_iLineWidths.push_back( iWidthPixels );
}

void ColorBitmapText::DrawPrimitives( )
{
	Actor::SetGlobalRenderStates();	// set Actor-specified render states
	DISPLAY->SetTextureMode( TextureUnit_1, TextureMode_Modulate );

	/* Draw if we're not fully transparent or the zbuffer is enabled */
	if( m_pTempState->diffuse[0].a != 0 )
	{
		// render the shadow
		if( m_fShadowLengthX != 0  ||  m_fShadowLengthY != 0 )
		{
			DISPLAY->PushMatrix();
			DISPLAY->TranslateWorld( m_fShadowLengthX, m_fShadowLengthY, 0 );	// shift by 5 units
			RageColor c = m_ShadowColor;
			c.a *= m_pTempState->diffuse[0].a;
			for( unsigned i=0; i<m_aVertices.size(); i++ )
				m_aVertices[i].c = c;
			DrawChars( true );

			DISPLAY->PopMatrix();
		}

		// render the diffuse pass
		int loc = 0, cur = 0;
		RageColor c = m_pTempState->diffuse[0];

		for( unsigned i=0; i<m_aVertices.size(); i+=4 )
		{
			loc++;
			if( cur < (int)m_vColors.size() )
			{
				if ( loc > m_vColors[cur].l )
				{
					c = m_vColors[cur].c;
					cur++;
				}
			}
			for( unsigned j=0; j<4; j++ )
				m_aVertices[i+j].c = c;
		}

		DrawChars( false );
	}

	// render the glow pass
	if( m_pTempState->glow.a > 0.0001f )
	{
		DISPLAY->SetTextureMode( TextureUnit_1, TextureMode_Glow );

		for( unsigned i=0; i<m_aVertices.size(); i++ )
			m_aVertices[i].c = m_pTempState->glow;
		DrawChars( false );
	}
}

void ColorBitmapText::SetMaxLines( int iNumLines, int iDirection )
{
	iNumLines = max( 0, iNumLines );
	iNumLines = min( (int)m_wTextLines.size(), iNumLines );
	if( iDirection == 0 ) 
	{
		// Crop all bottom lines
		m_wTextLines.resize( iNumLines );
		m_iLineWidths.resize( iNumLines );
	}
	else
	{
		// Because colors are relative to the beginning, we have to crop them back
		unsigned shift = 0;

		for( unsigned i = 0; i < m_wTextLines.size() - iNumLines; i++ )
			shift += m_wTextLines[i].length();

		// When we're cutting out text, we need to maintain the last
		// color, so our text at the top doesn't become colorless.
		RageColor LastColor;

		for( unsigned i = 0; i < m_vColors.size(); i++ )
		{
			m_vColors[i].l -= shift;
			if( m_vColors[i].l < 0 )
			{
				LastColor = m_vColors[i].c;
				m_vColors.erase( m_vColors.begin() + i );
				i--;
			}
		}

		// If we already have a color set for the first char
		// do not override it.
		if( m_vColors.size() > 0 && m_vColors[0].l > 0 )
		{
			ColorChange tmp;
			tmp.c = LastColor;
			tmp.l = 0;
			m_vColors.insert( m_vColors.begin(), tmp );
		}

		m_wTextLines.erase( m_wTextLines.begin(), m_wTextLines.end() - iNumLines );
		m_iLineWidths.erase( m_iLineWidths.begin(), m_iLineWidths.end() - iNumLines );
	}
	BuildChars();
}


void ScreenNetSelectBase::SetChatboxVisible(bool visibility)
{
	m_textChatInput.SetVisible(visibility);
	m_textChatOutput.SetVisible(visibility);
	return;
}
void ScreenNetSelectBase::SetUsersVisible(bool visibility)
{
	usersVisible = visibility;
	for (unsigned int i = 0; i < m_textUsers.size(); i++)
		m_textUsers[i].SetVisible(visibility);
	return;
}

vector<BitmapText>* ScreenNetSelectBase::ToUsers()
{
	return &m_textUsers;
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the PlayerState. */
class LunaScreenNetSelectBase : public Luna<ScreenNetSelectBase>
{
	static int ChatboxInput(T* p, lua_State *L)
	{
		if (!lua_isnil(L, 1))
			p->enableChatboxInput = BArg(1);
		return 1;
	}
	static int UsersVisible(T* p, lua_State *L)
	{
		if (!lua_isnil(L, 1))
			p->SetUsersVisible(BArg(1));
		return 1;
	}
	static int ChatboxVisible(T* p, lua_State *L)
	{
		if (!lua_isnil(L, 1))
			p->SetChatboxVisible(BArg(1));
		return 1;
	}
	static int GetUserQty(T* p, lua_State *L)
	{
		lua_pushnumber(L, p->ToUsers()->size());
		return 1;
	}
	static int GetUser(T* p, lua_State *L)
	{
		if (IArg(1) <= p->ToUsers()->size() && IArg(1) >= 1)
			lua_pushstring(L, (*(p->ToUsers()))[IArg(1) - 1].GetText());
		else
			lua_pushstring(L, "");
		return 1;
	}
	static int GetUserState(T* p, lua_State *L)
	{
		if (IArg(1) <= p->ToUsers()->size() && IArg(1) >= 1)
			lua_pushnumber(L, NSMAN->m_PlayerStatus[NSMAN->m_ActivePlayer[IArg(1) - 1]]);
		else
			lua_pushnumber(L, 0);
		return 1;
	}
	/* 
	static int GetFriendQty(T* p, lua_State *L)
	{
		lua_pushnumber(L, NSMAN->fl_PlayerNames.size());
		return 1;
	}
	static int GetFriendName(T* p, lua_State *L)
	{
		if (IArg(1) <= NSMAN->fl_PlayerNames.size() && IArg(1) >= 1)
			lua_pushstring(L, (NSMAN->fl_PlayerNames[IArg(1) - 1]).c_str());
		else
			lua_pushstring(L, "");
		return 1;
	}
	static int GetFriendState(T* p, lua_State *L)
	{
		if (IArg(1) <= NSMAN->fl_PlayerStates.size() && IArg(1) >= 1)
			lua_pushnumber(L, NSMAN->fl_PlayerStates[IArg(1) - 1]);
		else
			lua_pushnumber(L, 0);
		return 1;
	}
	*/
	static int ScrollChatUp(T* p, lua_State *L)
	{
		p->Scroll(1);
		return 1;
	}
	static int ScrollChatDown(T* p, lua_State *L)
	{
		p->Scroll(-1);
		return 1;
	}
	static int ShowNextMsg(T* p, lua_State *L)
	{
		p->ShowNextMsg();
		return 1;
	}
	static int ShowPreviousMsg(T* p, lua_State *L)
	{
		p->ShowPreviousMsg();
		return 1;
	}
	static int GetChatScroll(T* p, lua_State *L)
	{
		lua_pushnumber(L, p->GetScroll());
		return 1;
	}
	static int GetChatLines(T* p, lua_State *L)
	{
		lua_pushnumber(L, p->GetLines());
		return 1;
	}
public:
	LunaScreenNetSelectBase()
	{
		ADD_METHOD(GetUser);
		ADD_METHOD(UsersVisible);
		ADD_METHOD(ChatboxInput);
		ADD_METHOD(ChatboxVisible);
		ADD_METHOD(GetUserQty);
		ADD_METHOD(GetUserState);
		/*
		ADD_METHOD(GetFriendQty);
		ADD_METHOD(GetFriendState);
		ADD_METHOD(GetFriendName);
		*/
		ADD_METHOD(ScrollChatUp);
		ADD_METHOD(ScrollChatDown);
		ADD_METHOD(ShowNextMsg);
		ADD_METHOD(ShowPreviousMsg);
		ADD_METHOD(GetChatScroll);
		ADD_METHOD(GetChatLines);
	}
};

LUA_REGISTER_DERIVED_CLASS(ScreenNetSelectBase, ScreenWithMenuElements)
// lua end

#endif

/*
 * (c) 2004 Charles Lohr
 * All rights reserved.
 *      Elements from ScreenTextEntry
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
