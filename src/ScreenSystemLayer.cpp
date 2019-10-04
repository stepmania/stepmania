#include "global.h"
#include "ScreenSystemLayer.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "ActorUtil.h"
#include "GameState.h"
#include "MemoryCardManager.h"
#include "Profile.h"
#include "ProfileManager.h"
#include "RageLog.h"
#include "ScreenDimensions.h"
#include "ThemeMetric.h"
#include "LocalizedString.h"
#include "PlayerState.h"

namespace
{
	LocalizedString CREDITS_PRESS_START("ScreenSystemLayer","CreditsPressStart");
	LocalizedString CREDITS_INSERT_CARD("ScreenSystemLayer","CreditsInsertCard");
	LocalizedString CREDITS_CARD_TOO_LATE("ScreenSystemLayer","CreditsCardTooLate");
	LocalizedString CREDITS_CARD_NO_NAME("ScreenSystemLayer","CreditsCardNoName");
	LocalizedString CREDITS_CARD_READY("ScreenSystemLayer","CreditsCardReady");
	LocalizedString CREDITS_CARD_CHECKING("ScreenSystemLayer","CreditsCardChecking");
	LocalizedString CREDITS_CARD_REMOVED("ScreenSystemLayer","CreditsCardRemoved");
	LocalizedString CREDITS_FREE_PLAY("ScreenSystemLayer","CreditsFreePlay");
	LocalizedString CREDITS_CREDITS("ScreenSystemLayer","CreditsCredits");
	LocalizedString CREDITS_MAX("ScreenSystemLayer","CreditsMax");
	LocalizedString CREDITS_NOT_PRESENT("ScreenSystemLayer","CreditsNotPresent");
	LocalizedString CREDITS_LOAD_FAILED("ScreenSystemLayer","CreditsLoadFailed");
	LocalizedString CREDITS_LOADED_FROM_LAST_GOOD_APPEND("ScreenSystemLayer","CreditsLoadedFromLastGoodAppend");

	ThemeMetric<bool> CREDITS_JOIN_ONLY( "ScreenSystemLayer", "CreditsJoinOnly" );

	RString GetCreditsMessage( PlayerNumber pn )
	{
		if( (bool)CREDITS_JOIN_ONLY && !GAMESTATE->PlayersCanJoin() )
			return RString();

		bool bShowCreditsMessage;
		if( SCREENMAN && SCREENMAN->GetTopScreen() && SCREENMAN->GetTopScreen()->GetScreenType() == system_menu )
			bShowCreditsMessage = true;
		else if( MEMCARDMAN->GetCardLocked(pn) )
			bShowCreditsMessage = !GAMESTATE->IsPlayerEnabled( pn );
		else 
			bShowCreditsMessage = !GAMESTATE->m_bSideIsJoined[pn];
			
		if( !bShowCreditsMessage )
		{
			MemoryCardState mcs = MEMCARDMAN->GetCardState( pn );
			const Profile* pProfile = PROFILEMAN->GetProfile( pn );
			switch( mcs )
			{
			DEFAULT_FAIL( mcs );
			case MemoryCardState_NoCard:
				// this is a local machine profile
				if( PROFILEMAN->LastLoadWasFromLastGood(pn) )
					return pProfile->GetDisplayNameOrHighScoreName() + CREDITS_LOADED_FROM_LAST_GOOD_APPEND.GetValue();
				else if( PROFILEMAN->LastLoadWasTamperedOrCorrupt(pn) )
					return CREDITS_LOAD_FAILED.GetValue();
				// Prefer the name of the profile over the name of the card.
				else if( PROFILEMAN->IsPersistentProfile(pn) )
					return pProfile->GetDisplayNameOrHighScoreName();
				else if( GAMESTATE->PlayersCanJoin() )
					return CREDITS_INSERT_CARD.GetValue();
				else
					return RString();

			case MemoryCardState_Error: 		return THEME->GetString( "ScreenSystemLayer", "CreditsCard" + MEMCARDMAN->GetCardError(pn) );
			case MemoryCardState_TooLate:		return CREDITS_CARD_TOO_LATE.GetValue();
			case MemoryCardState_Checking:		return CREDITS_CARD_CHECKING.GetValue();
			case MemoryCardState_Removed:		return CREDITS_CARD_REMOVED.GetValue();
			case MemoryCardState_Ready:
				{
					// If the profile failed to load and there was no usable backup...
					if( PROFILEMAN->LastLoadWasTamperedOrCorrupt(pn) && !PROFILEMAN->LastLoadWasFromLastGood(pn) )
						return CREDITS_LOAD_FAILED.GetValue();

					// If there is a local profile loaded, prefer it over the name of the memory card.
					if( PROFILEMAN->IsPersistentProfile(pn) )
					{
						RString s = pProfile->GetDisplayNameOrHighScoreName();
						if( s.empty() )
							s = CREDITS_CARD_NO_NAME.GetValue();
						if( PROFILEMAN->LastLoadWasFromLastGood(pn) )
							s += CREDITS_LOADED_FROM_LAST_GOOD_APPEND.GetValue();
						return s;
					}
					else if( !MEMCARDMAN->IsNameAvailable(pn) )
						return CREDITS_CARD_READY.GetValue();
					else if( !MEMCARDMAN->GetName(pn).empty() )
						return MEMCARDMAN->GetName(pn);
					else
						return CREDITS_CARD_NO_NAME.GetValue();
				}
			}
		}
		else // bShowCreditsMessage
		{
			switch( GAMESTATE->GetCoinMode() )
			{
			case CoinMode_Home:
				if( GAMESTATE->PlayersCanJoin() )
					return CREDITS_PRESS_START.GetValue();
				else
					return CREDITS_NOT_PRESENT.GetValue();

			case CoinMode_Pay:
			// GCC is picky and needs this to be bracketed
			{
				int iCredits = GAMESTATE->m_iCoins / PREFSMAN->m_iCoinsPerCredit;
				int iCoins = GAMESTATE->m_iCoins % PREFSMAN->m_iCoinsPerCredit;
				RString sCredits = CREDITS_CREDITS;
				// todo: allow themers to change these strings -aj
				if( iCredits > 0 || PREFSMAN->m_iCoinsPerCredit == 1 )
					sCredits += ssprintf("  %d", iCredits);
				if( PREFSMAN->m_iCoinsPerCredit > 1 )
					sCredits += ssprintf("  %d/%d", iCoins, PREFSMAN->m_iCoinsPerCredit.Get() );
				if( iCredits >= MAX_NUM_CREDITS )
					sCredits += "  " + CREDITS_MAX.GetValue();
				return sCredits;
			}
			default: // CoinMode_Free
				if( GAMESTATE->PlayersCanJoin() )
					return CREDITS_FREE_PLAY.GetValue();
				// TODO: What should be displayed if players can't join in free mode?
				// Probably something like "Please Wait" or "Cannot Join"? -freem
			}
		}
		return RString();
	}

};

// lua start
#include "LuaBinding.h"

namespace
{
	int GetCreditsMessage( lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		RString sText = GetCreditsMessage( pn );
		LuaHelpers::Push( L, sText );
		return 1;
	}

	const luaL_Reg ScreenSystemLayerHelpersTable[] =
	{
		LIST_METHOD( GetCreditsMessage ),
		{ nullptr, nullptr }
	};
}

LUA_REGISTER_NAMESPACE( ScreenSystemLayerHelpers )

REGISTER_SCREEN_CLASS( ScreenSystemLayer );
void ScreenSystemLayer::Init()
{
	Screen::Init();

	m_sprOverlay.Load( THEME->GetPathB(m_sName, "overlay") );
	this->AddChild( m_sprOverlay );
	m_errLayer.Load( THEME->GetPathB(m_sName, "error") );
	this->AddChild( m_errLayer );
}

/*
 * (c) 2001-2005 Chris Danford, Glenn Maynard
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
