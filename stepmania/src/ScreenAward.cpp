#include "global.h"
#include "ScreenAward.h"
#include "ScreenManager.h"
#include "AnnouncerManager.h"
#include "RageSounds.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "SongManager.h"
#include "Steps.h"
#include "StyleDef.h"
#include "CodeDetector.h"
#include "ProfileManager.h"
#include "StepMania.h"
#include "CryptManager.h"


#define NEXT_SCREEN		THEME->GetMetric (m_sName,"NextScreen")


ScreenAward::ScreenAward( CString sName ) : ScreenWithMenuElements( sName )
{
	bool bEitherPlayerHasAward = false;
	ZERO( m_bSavedScreenshot );

	FOREACH_PlayerNumber( p )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;

		CString sAward;
		CString sDescription;
		if( !GAMESTATE->m_vLastPerDifficultyAwards[p].empty() )
		{
			PerDifficultyAward pda = GAMESTATE->m_vLastPerDifficultyAwards[p].front();
			GAMESTATE->m_vLastPerDifficultyAwards[p].pop_front();
			sAward = PerDifficultyAwardToString( pda );
			sDescription = 
				PerDifficultyAwardToThemedString( pda ) + "\n" +
				DifficultyToThemedString( GAMESTATE->m_pCurSteps[p]->GetDifficulty() );
		}
		else if( !GAMESTATE->m_vLastPeakComboAwards[p].empty() )
		{
			PeakComboAward pca = GAMESTATE->m_vLastPeakComboAwards[p].front();
			GAMESTATE->m_vLastPeakComboAwards[p].pop_front();
			sAward = PeakComboAwardToString( pca );
			sDescription = PeakComboAwardToThemedString( pca );
		}
		
		if( sAward.empty() )
			continue;	// skip this player

		bEitherPlayerHasAward = true;

		m_Received[p].Load( THEME->GetPathG(m_sName,"received") );
		m_Received[p]->SetName( ssprintf("ReceivedP%d",p+1) );
		SET_XY_AND_ON_COMMAND( m_Received[p] );
		this->AddChild( m_Received[p] );

		m_Trophy[p].Load( THEME->GetPathG(m_sName,"trophy "+sAward) );
		m_Trophy[p]->SetName( ssprintf("TrophyP%d",p+1) );
		SET_XY_AND_ON_COMMAND( m_Trophy[p] );
		this->AddChild( m_Trophy[p] );

		m_textDescription[p].LoadFromFont( THEME->GetPathF(m_sName,"description") );
		m_textDescription[p].SetName( ssprintf("DescriptionP%d",p+1) );
		m_textDescription[p].SetText( sDescription );
		SET_XY_AND_ON_COMMAND( m_textDescription[p] );
		this->AddChild( &m_textDescription[p] );
	}

	if( !bEitherPlayerHasAward )
	{
		HandleScreenMessage( SM_GoToNextScreen );
		return;
	}

	SOUND->PlayMusic( THEME->GetPathToS( m_sName + " music") );
}

void ScreenAward::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenAward::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenAward::Input()" );

	if( IsTransitioning() )
		return;


	if( GameI.IsValid() )
	{
		PlayerNumber pn = GAMESTATE->GetCurrentStyleDef()->ControllerToPlayerNumber( GameI.controller );

		if( CodeDetector::EnteredCode(GameI.controller, CodeDetector::CODE_SAVE_SCREENSHOT) )
		{
			if( !m_bSavedScreenshot[pn]  &&	// only allow one screenshot
				PROFILEMAN->IsUsingProfile(pn) )
			{
				Profile* pProfile = PROFILEMAN->GetProfile(pn);
				CString sDir = PROFILEMAN->GetProfileDir((ProfileSlot)pn) + "Screenshots/";
				int iScreenshotIndex = pProfile->GetNextScreenshotIndex();
				CString sFileName = SaveScreenshot( sDir, true, true, iScreenshotIndex );
				CString sPath = sDir+sFileName;
				
				if( !sFileName.empty() )
				{
					Screenshot screenshot;
					screenshot.sFileName = sFileName;
					screenshot.sMD5 = CRYPTMAN->GetMD5( sPath );
					// FIXME: save HighScore
					pProfile->AddScreenshot( screenshot );
				}

				m_bSavedScreenshot[pn] = true;
				return;	// handled
			}
		}
	}

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenAward::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		MenuStart( PLAYER_INVALID );
		break;
	case SM_GoToNextScreen:
		// TRICKY: Keep looping on this screen until all awards are shown
		bool bMoreAwardsLeft = false;
		FOREACH_PlayerNumber( p )
		{
			if( !GAMESTATE->IsHumanPlayer(p) )
				continue;

			if( !GAMESTATE->m_vLastPerDifficultyAwards[p].empty() ||
				!GAMESTATE->m_vLastPeakComboAwards[p].empty() )
			{
				bMoreAwardsLeft = true;
			}
		}
		if( bMoreAwardsLeft )
			SCREENMAN->SetNewScreen( m_sName );
		else
			SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}

void ScreenAward::MenuStart( PlayerNumber pn )
{
	FOREACH_PlayerNumber( p )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;

		OFF_COMMAND( m_Received[p] );
		OFF_COMMAND( m_Trophy[p] );
		OFF_COMMAND( m_textDescription[p] );
	}

	StartTransitioning( SM_GoToNextScreen );
}

/*
 * (c) 2004 Chris Danford
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
