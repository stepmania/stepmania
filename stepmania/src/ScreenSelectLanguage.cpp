#include "global.h"
#include "ScreenSelectLanguage.h"
#include "PrefsManager.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "InputEventPlus.h"

REGISTER_SCREEN_CLASS( ScreenSelectLanguage );

void ScreenSelectLanguage::Init()
{
	// fill m_aGameCommands before calling Init()
	vector<RString> vs;
	THEME->GetLanguages( vs );
	SortRStringArray( vs, true );

	FOREACH_CONST( RString, vs, s )
	{
		const LanguageInfo *pLI = GetLanguageInfo( *s );

		GameCommand gc;
		gc.m_iIndex = s - vs.begin();
		gc.m_sName = *s;
		gc.m_bInvalid = false;
		if( pLI )
			gc.m_sText = pLI->szNativeName;
		else
			gc.m_sText = *s;
		
		m_aGameCommands.push_back( gc );
	}

	ScreenSelectMaster::Init();
}

RString ScreenSelectLanguage::GetDefaultChoice()
{
	return HOOKS->GetPreferredLanguage();
}

void ScreenSelectLanguage::BeginScreen()
{
	ScreenSelectMaster::BeginScreen();

	if( !PREFSMAN->m_sLanguage.Get().empty() )
		this->HandleScreenMessage( SM_GoToNextScreen );
}

void ScreenSelectLanguage::MenuStart( const InputEventPlus &input )
{
	int iIndex = this->GetSelectionIndex( input.pn );
	RString sLangCode = m_aGameCommands[iIndex].m_sName;
	PREFSMAN->m_sLanguage.Set( sLangCode );
	PREFSMAN->SavePrefsToDisk();
	THEME->SwitchThemeAndLanguage( THEME->GetCurThemeName(), PREFSMAN->m_sLanguage, PREFSMAN->m_bPseudoLocalize );

	m_soundStart.Play();
	this->PostScreenMessage( SM_BeginFadingOut, 0 );
}

void ScreenSelectLanguage::MenuBack( const InputEventPlus &input )
{
	return;	// ignore the press
}

/*
 * (c) 2006 Chris Danford
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
