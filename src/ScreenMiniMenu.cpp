#include "global.h"
#include "ScreenMiniMenu.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "Foreach.h"
#include "ScreenDimensions.h"
#include "GameState.h"
#include "FontCharAliases.h"
#include "OptionRowHandler.h"
#include "PrefsManager.h"

void PrepareToLoadScreen( const RString &sScreenName );
void FinishedLoadingScreen();

AutoScreenMessage( SM_GoToOK );
AutoScreenMessage( SM_GoToCancel );

bool ScreenMiniMenu::s_bCancelled = false;
int	ScreenMiniMenu::s_iLastRowCode = -1;
vector<int>	ScreenMiniMenu::s_viLastAnswers;

// Hooks for profiling
void PrepareToLoadScreen( const RString &sScreenName ) {}
void FinishedLoadingScreen() {}

// Settings:
namespace
{
	const MenuDef* g_pMenuDef = NULL;
	ScreenMessage g_SendOnOK;
	ScreenMessage g_SendOnCancel;
};

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
	ASSERT( g_pMenuDef != NULL );

	LoadAMenu( g_pMenuDef );
	m_SMSendOnOK = g_SendOnOK;
	m_SMSendOnCancel = g_SendOnCancel;
	g_pMenuDef = NULL;

	ScreenOptions::BeginScreen();

	/* HACK: An OptionRow exits if a screen is set. ScreenMiniMenu is always
	 * pushed, so we don't set screens to load. Set a dummy screen, so
	 * ScreenOptions::GetNextScreenForSelection will know to move on. */
	m_sNextScreen = "xxx";
}

void ScreenMiniMenu::LoadAMenu( const MenuDef* pDef )
{
	m_vMenuRows = pDef->rows;

	s_viLastAnswers.resize( m_vMenuRows.size() );
	// Convert from m_vMenuRows to vector<OptionRowDefinition>
	vector<OptionRowHandler*> vHands;
	for( unsigned r=0; r<m_vMenuRows.size(); r++ )
	{
		const MenuRowDef &mr = m_vMenuRows[r];
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
		ExportOptions( i, vpns );

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
