#include "global.h"
#include "ModeChoice.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageDisplay.h"
#include "AnnouncerManager.h"
#include "ProfileManager.h"
#include "StepMania.h"
#include "ScreenManager.h"
#include "SongManager.h"
#include "PrefsManager.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "MemoryCardManager.h"
#include "song.h"
#include "Game.h"
#include "Style.h"

void ModeChoice::Init()
{
	m_sName = "";
	m_bInvalid = true;
	m_iIndex = -1;
	m_pGame = NULL;
	m_pStyle = NULL;
	m_pm = PLAY_MODE_INVALID;
	m_dc = DIFFICULTY_INVALID;
	m_CourseDifficulty = DIFFICULTY_INVALID;
	m_sModifiers = "";
	m_sAnnouncer = "";
	m_sScreen = "";
	m_pSong = NULL;
	m_pSteps = NULL;
	m_pCourse = NULL;
	m_pTrail = NULL;
	m_pCharacter = NULL;
}

bool CompareSongOptions( const SongOptions &so1, const SongOptions &so2 );

bool ModeChoice::DescribesCurrentModeForAllPlayers() const
{
	FOREACH_HumanPlayer( pn )
		if( !DescribesCurrentMode(pn) )
			return false;

	return true;
}

bool ModeChoice::DescribesCurrentMode( PlayerNumber pn ) const
{
	if( m_pGame != NULL && m_pGame != GAMESTATE->m_pCurGame )
		return false;
	if( m_pm != PLAY_MODE_INVALID && GAMESTATE->m_PlayMode != m_pm )
		return false;
	if( m_pStyle && GAMESTATE->m_pCurStyle != m_pStyle )
		return false;
	// HACK: don't compare m_dc if m_pSteps is set.  This causes problems 
	// in ScreenSelectOptionsMaster::ImportOptions if m_PreferredDifficulty 
	// doesn't match the difficulty of m_pCurSteps.
	if( m_pSteps == NULL  &&  m_dc != DIFFICULTY_INVALID )
	{
		// Why is this checking for all players?
		FOREACH_PlayerNumber( pn )
			if( GAMESTATE->IsHumanPlayer(pn) && GAMESTATE->m_PreferredDifficulty[pn] != m_dc )
				return false;
	}
		
	if( m_sAnnouncer != "" && m_sAnnouncer != ANNOUNCER->GetCurAnnouncerName() )
		return false;

	if( m_sModifiers != "" )
	{
		/* Apply modifiers. */
		PlayerOptions po = GAMESTATE->m_PlayerOptions[pn];
		SongOptions so = GAMESTATE->m_SongOptions;
		po.FromString( m_sModifiers );
		so.FromString( m_sModifiers );

		if( po != GAMESTATE->m_PlayerOptions[pn] )
			return false;
		if( so != GAMESTATE->m_SongOptions )
			return false;
	}

	if( m_pSong && GAMESTATE->m_pCurSong != m_pSong )
		return false;
	if( m_pSteps && GAMESTATE->m_pCurSteps[pn] != m_pSteps )
		return false;
	if( m_pCharacter && GAMESTATE->m_pCurCharacters[pn] != m_pCharacter )
		return false;
	if( m_pTrail && GAMESTATE->m_pCurTrail[pn] != m_pTrail )
		return false;

	return true;
}

void ModeChoice::Load( int iIndex, CString sChoice )
{
	m_iIndex = iIndex;

	m_bInvalid = false;

	CString sSteps;

	CStringArray asCommands;
	split( sChoice, ";", asCommands );
	for( unsigned i=0; i<asCommands.size(); i++ )
	{
		CString sCommand = asCommands[i];

		CStringArray asBits;
		split( sCommand, ",", asBits );
		
		CString sName = asBits[0];
		asBits.erase(asBits.begin(), asBits.begin()+1);
		CString sValue = join( ",", asBits );

		sName.MakeLower();
		// sValue.MakeLower();

		if( sName == "game" )
		{
			const Game* pGame = GAMEMAN->StringToGameType( sValue );
			if( pGame != NULL )
				m_pGame = pGame;
			else
				m_bInvalid |= true;
		}


		if( sName == "style" )
		{
			const Style* style = GAMEMAN->GameAndStringToStyle( GAMESTATE->m_pCurGame, sValue );
			if( style )
				m_pStyle = style;
			else
				m_bInvalid |= true;
		}

		if( sName == "playmode" )
		{
			PlayMode pm = StringToPlayMode( sValue );
			if( pm != PLAY_MODE_INVALID )
				m_pm = pm;
			else
				m_bInvalid |= true;
		}

		if( sName == "difficulty" )
		{
			Difficulty dc = StringToDifficulty( sValue );
			if( dc != DIFFICULTY_INVALID )
				m_dc = dc;
			else
				m_bInvalid |= true;
		}

		if( sName == "announcer" )
			m_sAnnouncer = sValue;

		if( sName == "name" )
			m_sName = sValue;

		if( sName == "mod" )
		{
			if( m_sModifiers != "" )
				m_sModifiers += ",";
			m_sModifiers += sValue;
		}
		
		if( sName == "song" )
		{
			m_pSong = SONGMAN->FindSong( sValue );
			if( m_pSong == NULL )
			{
				m_sInvalidReason = ssprintf( "Song \"%s\" not found", sValue.c_str() );
				m_bInvalid |= true;
			}
		}

		if( sName == "steps" )
		{
			/* Save the name of the steps, and set this later, since we want to process
			 * any "song" and "style" commands first. */
			sSteps = sValue;
		}

		if( sName == "course" )
		{
			m_pCourse = SONGMAN->FindCourse( sValue );
			if( m_pCourse == NULL )
			{
				m_sInvalidReason = ssprintf( "Course \"%s\" not found", sValue.c_str() );
				m_bInvalid |= true;
			}
		}
		
		if( sName == "screen" )
			m_sScreen = sValue;

		if( sName == "setenv" )
		{
			ASSERT( asBits.size() == 2 );
			m_SetEnv[ asBits[0] ] = asBits[1];
		}
	}

	if( !m_bInvalid && sSteps != "" )
	{
		Song *pSong = (m_pSong != NULL)? m_pSong:GAMESTATE->m_pCurSong;
		const Style *style = m_pStyle ? m_pStyle : GAMESTATE->m_pCurStyle;
		if( pSong == NULL || style == NULL )
			RageException::Throw( "Must set Song and Style to set Steps" );

		Difficulty dc = StringToDifficulty( sSteps );
		if( dc != DIFFICULTY_EDIT )
			m_pSteps = pSong->GetStepsByDifficulty( m_pStyle->m_StepsType, dc );
		else
			m_pSteps = pSong->GetStepsByDescription( m_pStyle->m_StepsType, sSteps );
		if( m_pSteps == NULL )
		{
			m_sInvalidReason = "steps not found";
			m_bInvalid |= true;
		}
	}
}

int GetNumCreditsPaid()
{
	int iNumCreditsPaid = GAMESTATE->GetNumSidesJoined();

	// players other than the first joined for free
	if( PREFSMAN->GetPremium() == PrefsManager::JOINT_PREMIUM )
		iNumCreditsPaid = min( iNumCreditsPaid, 1 );

	return iNumCreditsPaid;
}


int GetCreditsRequiredToPlayStyle( const Style *style )
{
	if( PREFSMAN->GetPremium() == PrefsManager::JOINT_PREMIUM )
		return 1;

	switch( style->m_StyleType )
	{
	case Style::ONE_PLAYER_ONE_CREDIT:
		return 1;
	case Style::TWO_PLAYERS_TWO_CREDITS:
		return 2;
	case Style::ONE_PLAYER_TWO_CREDITS:
		return (PREFSMAN->GetPremium() == PrefsManager::DOUBLES_PREMIUM) ? 1 : 2;
	default:
		ASSERT(0);
		return 1;
	}
}

static bool AreStyleAndPlayModeCompatible( const Style *style, PlayMode pm )
{
	if( style == NULL || pm == PLAY_MODE_INVALID )
		return true;

	switch( pm )
	{
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		// Can't play rave if there isn't enough room for two players.
		// This is correct for dance (ie, no rave for solo and doubles),
		// and should be okay for pump .. not sure about other game types.
		// Techno Motion scales down versus arrows, though, so allow this.
		if( style->m_iColsPerPlayer >= 6 && CString(GAMESTATE->m_pCurGame->m_szName) != "techno" )
			return false;
		
		/* Don't allow battle modes if the style takes both sides. */
		if( style->m_StyleType==Style::ONE_PLAYER_TWO_CREDITS )
			return false;
	}

	return true;
}

bool ModeChoice::IsPlayable( CString *why ) const
{
	if( m_bInvalid )
	{
		if( why )
			*why = m_sInvalidReason;
		return false;
	}

	if ( m_pStyle )
	{
		int iCredits = GAMESTATE->m_iCoins / PREFSMAN->m_iCoinsPerCredit;
		const int iNumCreditsPaid = GetNumCreditsPaid();
		const int iNumCreditsRequired = GetCreditsRequiredToPlayStyle(m_pStyle);
		
		switch( PREFSMAN->GetCoinMode() )
		{
		case COIN_HOME:
		case COIN_FREE:
			iCredits = NUM_PLAYERS; /* not iNumCreditsPaid */
		}
		
		/* With PREFSMAN->m_bDelayedCreditsReconcile disabled, enough credits must be
		 * paid.  (This means that enough sides must be joined.)  Enabled, simply having
		 * enough credits lying in the machine is sufficient; we'll deduct the extra in
		 * Apply(). */
		int iNumCreditsAvailable = iNumCreditsPaid;
		if( PREFSMAN->m_bDelayedCreditsReconcile )
			iNumCreditsAvailable += iCredits;

		if( iNumCreditsAvailable < iNumCreditsRequired )
		{
			if( why )
				*why = ssprintf( "need %i credits, have %i", iNumCreditsRequired, iNumCreditsAvailable );
			return false;
		}

		/* If you've paid too much already, don't allow the mode.  (If we allow this,
		 * the credits will be "refunded" in Apply(), but that's confusing.) */
		/* Do allow the mode if they've already joined in more credits than 
		 * are required.  Otherwise, people who put in two credits to play 
		 * doubles on a doubles-premium machiune will get locked out.
		 * the refund logic isn't that awkward because you never see the 
		 * credits number jump up - the credits display is hidden if both 
		 * sides are joined. -Chris */
		//if( PREFSMAN->m_iCoinMode == COIN_PAY && iNumCreditsPaid > iNumCreditsRequired )
		//{
		//	if( why )
		//		*why = ssprintf( "too many credits paid (%i > %i)", iNumCreditsPaid, iNumCreditsRequired );
		//	return false;
		//}

		/* If both sides are joined, disallow singles modes, since easy to select them
		 * accidentally, instead of versus mode. */
		if( m_pStyle->m_StyleType == Style::ONE_PLAYER_ONE_CREDIT &&
			GAMESTATE->GetNumSidesJoined() > 1 )
		{
			if( why )
				*why = "too many players joined for ONE_PLAYER_ONE_CREDIT";
			return false;
		}
	}

	/* Don't allow a PlayMode that's incompatible with our current Style (if set),
	 * and vice versa. */
	if( m_pm != PLAY_MODE_INVALID || m_pStyle != NULL )
	{
		const PlayMode pm = (m_pm != PLAY_MODE_INVALID) ? m_pm : GAMESTATE->m_PlayMode;
		const Style *style = (m_pStyle != NULL)? m_pStyle: GAMESTATE->m_pCurStyle;
		if( !AreStyleAndPlayModeCompatible( style, pm ) )
		{
			if( why )
				*why = ssprintf("mode %s is incompatible with style %s",
					PlayModeToString(pm).c_str(), style->m_szName );

			return false;
		}
	}

	if( !m_sScreen.CompareNoCase("ScreenEditCoursesMenu") )
	{
		vector<Course*> vCourses;
		SONGMAN->GetAllCourses( vCourses, false );

		if( vCourses.size() == 0 )
		{
			if( why )
				*why = "No courses are installed";
			return false;
		}
	}

	if( !m_sScreen.CompareNoCase("ScreenJukeboxMenu") ||
		!m_sScreen.CompareNoCase("ScreenEditMenu") ||
		!m_sScreen.CompareNoCase("ScreenEditCoursesMenu") )
	{
		if( SONGMAN->GetNumSongs() == 0 )
		{
			if( why )
				*why = "No songs are installed";
			return false;
		}
	}

	return true;
}

void ModeChoice::ApplyToAllPlayers() const
{
	FOREACH_HumanPlayer( pn )
		Apply( pn);

	if( m_sScreen != "" )
		SCREENMAN->SetNewScreen( m_sScreen );
}

void ModeChoice::Apply( PlayerNumber pn ) const
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;

	const PlayMode OldPlayMode = GAMESTATE->m_PlayMode;

	if( m_pGame != NULL )
		GAMESTATE->m_pCurGame = m_pGame;
	if( m_pm != PLAY_MODE_INVALID )
		GAMESTATE->m_PlayMode = m_pm;

	if( m_pStyle != NULL )
	{
		GAMESTATE->m_pCurStyle = m_pStyle;

		// It's possible to choose a style that didn't have enough 
		// players joined.  If enough players aren't joined, then 
		// we need to subtract credits for the sides that will be
		// joined as a result of applying this option.
		if( PREFSMAN->m_iCoinMode == COIN_PAY )
		{
			int iNumCreditsRequired = GetCreditsRequiredToPlayStyle(m_pStyle);
			int iNumCreditsPaid = GetNumCreditsPaid();
			int iNumCreditsOwed = iNumCreditsRequired - iNumCreditsPaid;
			GAMESTATE->m_iCoins -= iNumCreditsOwed * PREFSMAN->m_iCoinsPerCredit;
			LOG->Trace( "Deducted %i coins, %i remaining",
					iNumCreditsOwed * PREFSMAN->m_iCoinsPerCredit, GAMESTATE->m_iCoins );
		}


		// If only one side is joined and we picked a style
		// that requires both sides, join the other side.
		switch( m_pStyle->m_StyleType )
		{
		case Style::ONE_PLAYER_ONE_CREDIT:
			break;
		case Style::TWO_PLAYERS_TWO_CREDITS:
		case Style::ONE_PLAYER_TWO_CREDITS:
			{
				FOREACH_PlayerNumber( p )
					GAMESTATE->m_bSideIsJoined[p] = true;
			}
			break;
		default:
			ASSERT(0);
		}
	}
	if( m_dc != DIFFICULTY_INVALID  &&  pn != PLAYER_INVALID )
		GAMESTATE->ChangePreferredDifficulty( pn, m_dc );
	if( m_sAnnouncer != "" )
		ANNOUNCER->SwitchAnnouncer( m_sAnnouncer );
	if( m_sModifiers != "" )
		GAMESTATE->ApplyModifiers( pn, m_sModifiers );
	if( m_pSong )
	{
		GAMESTATE->m_pCurSong = m_pSong;
		GAMESTATE->m_pPreferredSong = m_pSong;
	}
	if( m_pSteps )
		GAMESTATE->m_pCurSteps[pn] = m_pSteps;
	if( m_pCourse )
	{
		GAMESTATE->m_pCurCourse = m_pCourse;
		GAMESTATE->m_pPreferredCourse = m_pCourse;
	}
	if( m_pTrail )
		GAMESTATE->m_pCurTrail[pn] = m_pTrail;
	if( m_CourseDifficulty != DIFFICULTY_INVALID )
		GAMESTATE->ChangePreferredCourseDifficulty( pn, m_CourseDifficulty );
	if( m_pCharacter )
		GAMESTATE->m_pCurCharacters[pn] = m_pCharacter;
	for( map<CString,CString>::const_iterator i = m_SetEnv.begin(); i != m_SetEnv.end(); i++ )
		GAMESTATE->m_mapEnv[ i->first ] = i->second;

	// HACK:  Set life type to BATTERY just once here so it happens once and 
	// we don't override the user's changes if they back out.
	if( GAMESTATE->m_PlayMode == PLAY_MODE_ONI && GAMESTATE->m_PlayMode != OldPlayMode )
		GAMESTATE->m_SongOptions.m_LifeType = SongOptions::LIFE_BATTERY;


	//
	// We know what players are joined at the time we set the Style
	//
	if( m_pStyle != NULL )
	{
		GAMESTATE->PlayersFinalized();
	}
}

bool ModeChoice::IsZero() const
{
	if( m_pGame != NULL ||
		m_pm != PLAY_MODE_INVALID ||
		m_pStyle != NULL ||
		m_dc != DIFFICULTY_INVALID ||
		m_sAnnouncer != "" ||
		m_sModifiers != "" ||
		m_pSong != NULL || 
		m_pSteps != NULL || 
		m_pCourse != NULL || 
		m_pTrail != NULL || 
		m_pCharacter != NULL || 
		m_CourseDifficulty != DIFFICULTY_INVALID )
		return false;

	return true;
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
