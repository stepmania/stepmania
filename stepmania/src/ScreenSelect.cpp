#include "global.h"
#include "ScreenSelect.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "ModeChoice.h"
#include "RageDisplay.h"
#include "UnlockSystem.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "LightsManager.h"


#define CHOICE_NAMES			THEME->GetMetric (m_sName,"ChoiceNames")
#define CHOICE( sChoiceName )	THEME->GetMetric (m_sName,ssprintf("Choice%s",sChoiceName.c_str()))
#define NUM_CODES				THEME->GetMetricI(m_sName,"NumCodes")
#define CODE( c )				THEME->GetMetric (m_sName,ssprintf("Code%d",c+1))
#define CODE_ACTION( c )		THEME->GetMetric (m_sName,ssprintf("Code%dAction",c+1))
#define HELP_TEXT				THEME->GetMetric (m_sName,"HelpText")
#define NEXT_SCREEN( c )		THEME->GetMetric (m_sName,ssprintf("NextScreen%d",c+1))

ScreenSelect::ScreenSelect( CString sClassName ) : ScreenWithMenuElements(sClassName)
{
	LOG->Trace( "ScreenSelect::ScreenSelect()" );

	m_sName = sClassName;

	//
	// Load choices
	//
	{
		// Instead of using NUM_CHOICES, use a comma-separated list of choices.  Each
		// element in the list is a choice name.  This level of indirection 
		// makes it easier to add or remove items without having to change a bunch
		// of indices.
		CStringArray asChoiceNames;
		split( CHOICE_NAMES, ",", asChoiceNames, true );

		for( unsigned c=0; c<asChoiceNames.size(); c++ )
		{
			CString sChoiceName = asChoiceNames[c];
			CString sChoice = CHOICE(sChoiceName);

			ModeChoice mc;
			mc.m_sName = sChoiceName;
			mc.Load( c, sChoice );
			m_aModeChoices.push_back( mc );
		
			CString sBGAnimationDir = THEME->GetPath(BGAnimations, m_sName, mc.m_sName, true);	// true="optional"
			if( sBGAnimationDir == "" )
				sBGAnimationDir = THEME->GetPathToB(m_sName+" background");
			BGAnimation *pBGA = new BGAnimation;
			m_vpBGAnimations.push_back( pBGA );
		}
	}

	//
	// Load codes
	//
	for( int c=0; c<NUM_CODES; c++ )
	{
		CodeItem code;
		if( !code.Load( CODE(c) ) )
			continue;

		m_aCodes.push_back( code );
		m_aCodeActions.push_back( CODE_ACTION(c) );
		ModeChoice mc;
		mc.Load( c, CODE_ACTION(c) );
		m_aCodeChoices.push_back( mc );
	}

	if( !m_aModeChoices.size() )
		RageException::Throw( "Screen \"%s\" does not set any choices", m_sName.c_str() );

	// derived classes can override if they want
	LIGHTSMAN->SetLightsMode( LIGHTSMODE_MENU );
}


ScreenSelect::~ScreenSelect()
{
	LOG->Trace( "ScreenSelect::~ScreenSelect()" );
	for( unsigned i=0; i<m_vpBGAnimations.size(); i++ )
		SAFE_DELETE( m_vpBGAnimations[i] );
	m_vpBGAnimations.clear();
}

void ScreenSelect::Update( float fDelta )
{
	if(m_bFirstUpdate)
	{
		/* Don't play sounds during the ctor, since derived classes havn't loaded yet. */
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo(m_sName+" intro") );
		SOUND->PlayMusic( THEME->GetPathToS(m_sName+" music") );
	}

	Screen::Update( fDelta );
	
	// GAMESTATE->m_MasterPlayerNumber is set to PLAYER_INVALID when going Back to 
	// the title screen and this screen is updated after.  TODO: find out why
	if( GAMESTATE->m_MasterPlayerNumber != PLAYER_INVALID )	
	{
		int iSelection = this->GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber);
		m_vpBGAnimations[iSelection]->Update( fDelta );
	}
}

void ScreenSelect::DrawPrimitives()
{
	// GAMESTATE->m_MasterPlayerNumber is set to PLAYER_INVALID when going Back to 
	// the title screen and this screen is updated after.  TODO: find out why
	if( GAMESTATE->m_MasterPlayerNumber != PLAYER_INVALID )	
	{
		int iSelection = this->GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber);
		m_vpBGAnimations[iSelection]->Draw();
	}
	Screen::DrawPrimitives();
}

void ScreenSelect::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenSelect::Input()" );

	if( MenuI.button == MENU_BUTTON_COIN ||
		Screen::JoinInput(DeviceI, type, GameI, MenuI, StyleI) )
	{
		if( type == IET_FIRST_PRESS )
			this->UpdateSelectableChoices();
		return;	// don't let the screen handle the MENU_START press
	}

	if( IsTransitioning() )
		return;

	for( unsigned i = 0; i < m_aCodes.size(); ++i )
	{
		if( !m_aCodes[i].EnteredCode( GameI.controller ) )
			continue;

		const CString &action = m_aCodeActions[i];
		LOG->Trace("entered code for '%s'", action.c_str());
		vector<CString> parts;
		split( action, ";", parts, true );
		for( unsigned j = 0; j < parts.size(); ++j )
		{
			vector<CString> asBits;
			split( parts[j], ",", asBits, true );
			if( !asBits[0].CompareNoCase("unlock") )
				UNLOCKMAN->UnlockCode( atoi(asBits[1]) );
			if( !asBits[0].CompareNoCase("sound") )
				SOUND->PlayOnce( THEME->GetPathToS( asBits[1] ) );
		}

		m_aCodeChoices[i].Apply( MenuI.player );
	}
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelect::FinalizeChoices()
{
	/* At this point, we're tweening out; we can't change the selection.
	 * We don't want to allow players to join if the style will be set,
	 * since that can change the available selection and is likely to
	 * invalidate the choice we've already made.  Hack: apply the style.
	 * (Applying the style may have other side-effects, so it'll be re-applied
	 * in SM_GoToNextScreen.) */
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->IsHumanPlayer(p) )
		{
			const int sel = GetSelectionIndex( p );
			
			if( m_aModeChoices[sel].m_pStyle )
				GAMESTATE->m_pCurStyle = m_aModeChoices[sel].m_pStyle;
		}
	SCREENMAN->RefreshCreditsMessages();
}

void ScreenSelect::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	/* Screen is starting to tween out. */
	case SM_BeginFadingOut:
		FinalizeChoices();
		break;

	/* It's our turn to tween out. */
	case SM_AllDoneChoosing:		
		FinalizeChoices();
		if( !IsTransitioning() )
			StartTransitioning( SM_GoToNextScreen );
		break;
	case SM_GoToNextScreen:
		{
			/* Apply here, not in SM_AllDoneChoosing, because applying can take a very
			 * long time (200+ms), and at SM_AllDoneChoosing, we're still tweening stuff
			 * off-screen. */
			FOREACH_PlayerNumber( p )
				if( GAMESTATE->IsHumanPlayer(p) )
					m_aModeChoices[this->GetSelectionIndex((PlayerNumber)p)].Apply( (PlayerNumber)p );

			const int iSelectionIndex = GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber);
			if( m_aModeChoices[iSelectionIndex].m_sScreen != "" )
				SCREENMAN->SetNewScreen( m_aModeChoices[iSelectionIndex ].m_sScreen );
			else
				SCREENMAN->SetNewScreen( NEXT_SCREEN(iSelectionIndex) );
		}
		return;
	}

	Screen::HandleScreenMessage( SM );
}

void ScreenSelect::MenuBack( PlayerNumber pn )
{
	SOUND->StopMusic();

	Back( SM_GoToPrevScreen );
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

