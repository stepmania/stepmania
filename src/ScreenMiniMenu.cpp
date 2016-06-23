#include "global.h"
#include "ScreenMiniMenu.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "ScreenDimensions.h"
#include "GameState.h"
#include "FontCharAliases.h"
#include "OptionRowHandler.h"
#include "PrefsManager.h"

using std::vector;

void PrepareToLoadScreen( const std::string &sScreenName );
void FinishedLoadingScreen();

AutoScreenMessage( SM_GoToOK );
AutoScreenMessage( SM_GoToCancel );

bool ScreenMiniMenu::s_bCancelled = false;
int	ScreenMiniMenu::s_iLastRowCode = -1;
vector<int>	ScreenMiniMenu::s_viLastAnswers;

// Hooks for profiling
void PrepareToLoadScreen( const std::string &sScreenName ) {}
void FinishedLoadingScreen() {}

// Settings:
namespace
{
	const MenuDef* g_pMenuDef = nullptr;
	ScreenMessage g_SendOnOK;
	ScreenMessage g_SendOnCancel;
};

#define PUSH(c) do { if( !(c.empty()) ) { choices.push_back(c); } } while(0)

MenuRowDef::MenuRowDef():
	iRowCode(0), sName(""), bEnabled(false),
	pfnEnabled(nullptr), emShowIn(), iDefaultChoice(0),
	choices(), bThemeTitle(false), bThemeItems(false) {}

MenuRowDef::MenuRowDef( int r, std::string n, MenuRowUpdateEnabled pe, EditMode s,
					   bool bTT, bool bTI, int d):
	iRowCode(r), sName(n), bEnabled(true),
	pfnEnabled(pe), emShowIn(s), iDefaultChoice(d),
	choices(), bThemeTitle(bTT), bThemeItems(bTI) {}

MenuRowDef::MenuRowDef(int r, std::string n, bool e, EditMode s,
					   bool bTT, bool bTI, int d, std::vector<std::string> options):
	iRowCode(r), sName(n), bEnabled(e),
	pfnEnabled(nullptr), emShowIn(s), iDefaultChoice(d),
	choices(), bThemeTitle(bTT), bThemeItems(bTI)
{
	for (auto &str: options)
	{
		if (str != "")
		{
			choices.push_back(str);
		}
	}
}

MenuRowDef::MenuRowDef( int r, std::string n, bool e, EditMode s, bool bTT, bool bTI, int d,
	std::string const &c0, std::string const &c1,
	std::string const &c2, std::string const &c3,
	std::string const &c4, std::string const &c5,
	std::string const &c6, std::string const &c7,
	std::string const &c8, std::string const &c9,
	std::string const &c10, std::string const &c11,
	std::string const &c12, std::string const &c13,
	std::string const &c14, std::string const &c15,
	std::string const &c16, std::string const &c17,
	std::string const &c18, std::string const &c19,
	std::string const &c20, std::string const &c21,
	std::string const &c22) :
	iRowCode(r), sName(n), bEnabled(e), pfnEnabled(nullptr),
	emShowIn(s), iDefaultChoice(d), choices(),
	bThemeTitle(bTT), bThemeItems(bTI)
{
	PUSH(c0); PUSH(c1); PUSH(c2); PUSH(c3); PUSH(c4);
	PUSH(c5); PUSH(c6); PUSH(c7); PUSH(c8); PUSH(c9);
	PUSH(c10); PUSH(c11); PUSH(c12); PUSH(c13); PUSH(c14);
	PUSH(c15); PUSH(c16); PUSH(c17); PUSH(c18); PUSH(c19);
	PUSH(c20); PUSH(c21); PUSH(c22);
}

MenuRowDef::MenuRowDef( int r, std::string n, bool e, EditMode s, bool bTT, bool bTI,
					   int d, int low, int high ):
iRowCode(r), sName(n), bEnabled(e),
pfnEnabled(nullptr), emShowIn(s), iDefaultChoice(d),
choices(), bThemeTitle(bTT), bThemeItems(bTI)
{
	for ( int i = low; i <= high; ++i )
	{
		choices.push_back(std::to_string(i).c_str());
	}
}

#undef PUSH

void ScreenMiniMenu::MiniMenu( const MenuDef* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel, float fX, float fY )
{
	PrepareToLoadScreen( pDef->sClassName );

	g_pMenuDef = pDef;
	g_SendOnOK = SM_SendOnOK;
	g_SendOnCancel = SM_SendOnCancel;

	SCREENMAN->AddNewScreenToTop( pDef->sClassName );
	Screen *pNewScreen = SCREENMAN->GetTopScreen();
	pNewScreen->SetXY( fX, fY );

	FinishedLoadingScreen();
}

REGISTER_SCREEN_CLASS( ScreenMiniMenu );

void ScreenMiniMenu::Init()
{
	if( PREFSMAN->m_iArcadeOptionsNavigation )
		SetNavigation( NAV_THREE_KEY_MENU );

	ScreenOptions::Init();
}

void ScreenMiniMenu::BeginScreen()
{
	ASSERT( g_pMenuDef != nullptr );

	LoadMenu( g_pMenuDef );
	m_SMSendOnOK = g_SendOnOK;
	m_SMSendOnCancel = g_SendOnCancel;
	g_pMenuDef = nullptr;

	ScreenOptions::BeginScreen();

	/* HACK: An OptionRow exits if a screen is set. ScreenMiniMenu is always
	 * pushed, so we don't set screens to load. Set a dummy screen, so
	 * ScreenOptions::GetNextScreenForSelection will know to move on. */
	m_sNextScreen = "xxx";
}

void ScreenMiniMenu::LoadMenu( const MenuDef* pDef )
{
	m_vMenuRows = pDef->rows;

	s_viLastAnswers.resize( m_vMenuRows.size() );
	// Convert from m_vMenuRows to vector<OptionRowDefinition>
	vector<OptionRowHandler*> vHands;
	for (auto const &mr: m_vMenuRows)
	{
		OptionRowHandler *pHand = OptionRowHandlerUtil::MakeSimple( mr );
		vHands.push_back( pHand );
	}

	ScreenOptions::InitMenu( vHands );
}

void ScreenMiniMenu::AfterChangeValueOrRow( PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueOrRow( pn );

	vector<PlayerNumber> vpns;
	FOREACH_PlayerNumber( p )
		vpns.push_back( p );
	for( unsigned i=0; i<m_pRows.size(); i++ )
	{
		ExportOptions( i, vpns );
	}
	// Changing one option can affect whether other options are available.
	for( unsigned i=0; i<m_pRows.size(); i++ )
	{
		const MenuRowDef &mr = m_vMenuRows[i];
		if( mr.pfnEnabled )
		{
			OptionRow &optrow = *m_pRows[i];
			optrow.GetRowDef().m_vEnabledForPlayers.clear();
			if( mr.pfnEnabled() )
				optrow.GetRowDef().m_vEnabledForPlayers.insert( GAMESTATE->GetMasterPlayerNumber() );
		}
		m_pRows[i]->UpdateEnabledDisabled();
	}
}

void ScreenMiniMenu::ImportOptions( int r, const vector<PlayerNumber> &vpns )
{
	OptionRow &optrow = *m_pRows[r];
	const MenuRowDef &mr = m_vMenuRows[r];
	if( !mr.choices.empty() )
		optrow.SetOneSharedSelection( mr.iDefaultChoice );
}

void ScreenMiniMenu::ExportOptions( int r, const vector<PlayerNumber> &vpns )
{
	if( r == GetCurrentRow() )
		s_iLastRowCode = m_vMenuRows[r].iRowCode;
	s_viLastAnswers.resize( m_vMenuRows.size() );
	s_viLastAnswers[r] = m_pRows[r]->GetOneSharedSelection( true );
}

void ScreenMiniMenu::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToNextScreen )
	{
		s_bCancelled = false;
		SCREENMAN->PopTopScreen( m_SMSendOnOK );
		return;
	}
	else if( SM == SM_GoToPrevScreen )
	{
		s_bCancelled = true;
		SCREENMAN->PopTopScreen( m_SMSendOnCancel );
		return;
	}

	ScreenOptions::HandleScreenMessage( SM );
}

bool ScreenMiniMenu::FocusedItemEndsScreen( PlayerNumber pn ) const
{
	return true;
}


/*
 * (c) 2003-2004 Chris Danford
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
