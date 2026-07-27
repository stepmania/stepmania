#include "global.h"
#include "ScreenNameEntryTraditional.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "StatsManager.h"
#include "SongManager.h"
#include "Song.h"
#include "Style.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "InputEventPlus.h"
#include "RageInput.h"
#include "RageLog.h"
#include "CommonMetrics.h"

#include <vector>


REGISTER_SCREEN_CLASS( ScreenNameEntryTraditional );

void ScreenNameEntryTraditional::Init()
{
	if( PREFSMAN->m_sTestInitialScreen.Get() == m_sName )
	{
		GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;
		GAMESTATE->m_bSideIsJoined[PLAYER_2] = true;
		GAMESTATE->SetMasterPlayerNumber(PLAYER_1);
		GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );
		GAMESTATE->SetCurrentStyle( GAMEMAN->GameAndStringToStyle( GAMEMAN->GetDefaultGame(),"versus"), GAMESTATE->GetMasterPlayerNumber() );
		for( int z = 0; z < 3; ++z )
		{
			StageStats ss;
			const std::vector<Song*> &apSongs = SONGMAN->GetAllSongs();
			ss.m_vpPlayedSongs.push_back( apSongs[rand()%apSongs.size()] );
			ss.m_vpPossibleSongs = ss.m_vpPlayedSongs;
			ss.m_playMode = GAMESTATE->m_PlayMode;
			ASSERT( ss.m_vpPlayedSongs[0]->GetAllSteps().size() );

			FOREACH_PlayerNumber( p )
			{
				ss.m_player[p].m_pStyle = GAMESTATE->GetCurrentStyle(p);
				StepsType st = GAMESTATE->GetCurrentStyle(p)->m_StepsType;
				Steps *pSteps = ss.m_vpPlayedSongs[0]->GetAllSteps()[0];
				ss.m_player[p].m_iStepsPlayed = 1;
				GAMESTATE->m_pCurSteps[p].Set( pSteps );
				ss.m_player[p].m_iPossibleDancePoints = 100;
				ss.m_player[p].m_iActualDancePoints = 100;
				ss.m_player[p].m_iScore = 100;
				ss.m_player[p].m_iPossibleDancePoints = 1000;
				ss.m_player[p].m_iActualDancePoints = 985;
				ss.m_player[p].m_vpPossibleSteps.push_back( pSteps );

				HighScore hs;
				hs.SetGrade( Grade_Tier03 );
				hs.SetPercentDP( ss.m_player[p].GetPercentDancePoints() );
				hs.SetScore( ss.m_player[p].m_iScore );
				hs.SetDateTime( DateTime::GetNowDateTime() );
				int a, b;
				PROFILEMAN->AddStepsScore( ss.m_vpPlayedSongs[0], pSteps, p, hs, a, b );
				PROFILEMAN->AddStepsScore( ss.m_vpPlayedSongs[0], pSteps, p, hs, a, b );
				PROFILEMAN->AddStepsScore( ss.m_vpPlayedSongs[0], pSteps, p, hs, a, b );
				PROFILEMAN->AddStepsScore( ss.m_vpPlayedSongs[0], pSteps, p, hs, a, b );
				PROFILEMAN->AddStepsScore( ss.m_vpPlayedSongs[0], pSteps, p, hs, a, b );
				PROFILEMAN->AddCategoryScore( st, RANKING_A, p, hs, a, b );
			}

			STATSMAN->m_vPlayedStageStats.push_back( ss );
		}

	}

	MAX_RANKING_NAME_LENGTH.Load( m_sName, "MaxRankingNameLength" );

	ScreenWithMenuElements::Init();
}

void ScreenNameEntryTraditional::BeginScreen()
{
	/* Find out if players are entering their name. */
	FOREACH_PlayerNumber( pn )
	{
		std::vector<GameState::RankingFeat> aFeats;
		GAMESTATE->GetRankingFeats( pn, aFeats );

		bool bNoStagesLeft = GAMESTATE->m_iPlayerStageTokens[pn] <= 0;
		m_bEnteringName[pn] = ( aFeats.size() > 0 ||
				       PROFILEMAN->ProfileFromMemoryCardIsNew(pn) ) && bNoStagesLeft;
		m_bFinalized[pn] = !m_bEnteringName[pn];
	}

	FOREACH_HumanPlayer( pn )
	{
		if( !m_bEnteringName[pn] )
			continue;	// skip

		m_sSelection[pn] = L"";

		// load last used ranking name if any
		const Profile *pProfile = PROFILEMAN->GetProfile(pn);
		if( pProfile && !pProfile->m_sLastUsedHighScoreName.empty() )
		{
			m_sSelection[pn] = RStringToWstring( pProfile->m_sLastUsedHighScoreName );
			if( (int) m_sSelection[pn].size() > MAX_RANKING_NAME_LENGTH )
				m_sSelection[pn].erase( MAX_RANKING_NAME_LENGTH );
		}

		UpdateSelectionText( pn );
	}

	ScreenWithMenuElements::BeginScreen();
}

void ScreenNameEntryTraditional::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_MenuTimer )
	{
		if( !m_Out.IsTransitioning() )
		{
			Message msg("MenuTimerExpired");
			this->HandleMessage( msg );
		}
		return;
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

bool ScreenNameEntryTraditional::AnyStillEntering() const
{
	FOREACH_PlayerNumber( pn )
		if( !m_bFinalized[pn] )
			return true;
	return false;
}

bool ScreenNameEntryTraditional::AnyEntering() const
{
	FOREACH_PlayerNumber( pn )
		if( m_bEnteringName[pn] )
			return true;
	return false;
}

bool ScreenNameEntryTraditional::Input( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return false;

	return ScreenWithMenuElements::Input( input );
}

bool ScreenNameEntryTraditional::Finish( PlayerNumber pn )
{
	if( !AnyStillEntering() )
	{
		StartTransitioningScreen( SM_GoToNextScreen );
		return true;
	}

	if( m_bFinalized[pn] )
		return false;
	m_bFinalized[pn] = true;

	UpdateSelectionText( pn ); /* hide NAME_ cursor */
	RString sSelection = WStringToRString( m_sSelection[pn] );

	// save last used ranking name
	Profile* pProfile = PROFILEMAN->GetProfile(pn);
	pProfile->m_sLastUsedHighScoreName = sSelection;

	Trim( sSelection, " " );

	GAMESTATE->StoreRankingName( pn, sSelection );

	{
		Message msg("PlayerFinished");
		msg.SetParam( "PlayerNumber", pn );
		this->HandleMessage( msg );
	}

	if( !AnyStillEntering() )
		StartTransitioningScreen( SM_GoToNextScreen );

	return true;
}

void ScreenNameEntryTraditional::UpdateSelectionText( PlayerNumber pn )
{
	std::wstring sText = m_sSelection[pn];
	if( !m_bFinalized[pn] && (int) sText.size() < MAX_RANKING_NAME_LENGTH )
		sText += L"_";

	Message msg("EntryChanged");
	msg.SetParam( "PlayerNumber", pn );
	msg.SetParam( "Text", WStringToRString(sText) );
	this->HandleMessage( msg );
}

bool ScreenNameEntryTraditional::EnterKey( PlayerNumber pn, wchar_t sLetter )
{
	if( m_bFinalized[pn] )
		return false;	// ignore

	/* If we have room, add a new character. */
	if( (int) m_sSelection[pn].size() == MAX_RANKING_NAME_LENGTH )
	{
		SelectChar( pn, "BACK" );
		return false;
	}

	m_sSelection[pn] += sLetter;
	UpdateSelectionText( pn );

	/* If that filled the string, set the cursor on OK. */
	if( (int) m_sSelection[pn].size() == MAX_RANKING_NAME_LENGTH )
		SelectChar( pn, "ENTER" );
	return true;
}

void ScreenNameEntryTraditional::SelectChar( PlayerNumber pn, const RString &sKey )
{
	Message msg("SelectKey");
	msg.SetParam( "PlayerNumber", pn );
	msg.SetParam( "Key", sKey );
	this->HandleMessage( msg );
}

bool ScreenNameEntryTraditional::Backspace( PlayerNumber pn )
{
	if( m_bFinalized[pn] )
		return false;	// ignore

	if( !m_sSelection[pn].size()  )
		return false;

	m_sSelection[pn].erase( m_sSelection[pn].size()-1, 1 );
	UpdateSelectionText( pn );
	return true;
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ScreenNameEntryTraditional. */
class LunaScreenNameEntryTraditional: public Luna<ScreenNameEntryTraditional>
{
public:
	static int EnterKey( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		RString sKey = SArg(2);
		bool bRet = p->EnterKey( pn, utf8_get_char(sKey) );
		LuaHelpers::Push( L, bRet );
		return 1;
	}

	static int Finish( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		bool bRet = p->Finish( pn );
		LuaHelpers::Push( L, bRet );
		return 1;
	}

	static int Backspace( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		bool bRet = p->Backspace( pn );
		LuaHelpers::Push( L, bRet );
		return 1;
	}

	static int GetEnteringName( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		LuaHelpers::Push( L, p->m_bEnteringName[pn] );
		return 1;
	}

	static int GetFinalized( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		LuaHelpers::Push( L, p->m_bFinalized[pn] );
		return 1;
	}

	static int GetAnyEntering( T* p, lua_State *L )
	{
		LuaHelpers::Push( L, p->AnyEntering() );
		return 1;
	}

	static int GetAnyStillEntering( T* p, lua_State *L )
	{
		LuaHelpers::Push( L, p->AnyStillEntering() );
		return 1;
	}

	static int GetSelection( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		LuaHelpers::Push( L, WStringToRString(p->m_sSelection[pn]) );
		return 1;
	}

	LunaScreenNameEntryTraditional()
	{
		ADD_METHOD( EnterKey );
		ADD_METHOD( Finish );
		ADD_METHOD( Backspace );
		ADD_METHOD( GetEnteringName );
		ADD_METHOD( GetAnyEntering );
		ADD_METHOD( GetFinalized );
		ADD_METHOD( GetAnyStillEntering );
		ADD_METHOD( GetSelection );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenNameEntryTraditional, ScreenWithMenuElements )
// lua end

/*
 * (c) 2001-2007 Glenn Maynard, Chris Danford
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
