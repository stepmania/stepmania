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


REGISTER_SCREEN_CLASS( ScreenSystemLayer );
void ScreenSystemLayer::Init()
{
	SubscribeToMessage( "RefreshCreditText" );
	SubscribeToMessage( "CoinInserted" );
	SubscribeToMessage( "SystemMessage" );
	SubscribeToMessage( "SystemMessageNoAnimate" );
	SubscribeToMessage( "HideSystemMessage" );
	FOREACH_PlayerNumber( pn )
		SubscribeToMessage( enum_add2(Message_SideJoinedP1, pn) );

	CREDITS_PRESS_START		.Load(m_sName,"CreditsPressStart");
	CREDITS_INSERT_CARD		.Load(m_sName,"CreditsInsertCard");
	CREDITS_CARD_TOO_LATE		.Load(m_sName,"CreditsCardTooLate");
	CREDITS_CARD_NO_NAME		.Load(m_sName,"CreditsCardNoName");
	CREDITS_CARD_READY		.Load(m_sName,"CreditsCardReady");
	CREDITS_CARD_CHECKING		.Load(m_sName,"CreditsCardChecking");
	CREDITS_CARD_REMOVED		.Load(m_sName,"CreditsCardRemoved");
	CREDITS_FREE_PLAY		.Load(m_sName,"CreditsFreePlay");
	CREDITS_CREDITS			.Load(m_sName,"CreditsCredits");
	CREDITS_NOT_PRESENT		.Load(m_sName,"CreditsNotPresent");
	CREDITS_LOAD_FAILED		.Load(m_sName,"CreditsLoadFailed");
	CREDITS_LOADED_FROM_LAST_GOOD_APPEND.Load(m_sName,"CreditsLoadedFromLastGoodAppend");
	CREDITS_JOIN_ONLY		.Load( m_sName, "CreditsJoinOnly" );

	Screen::Init();

	this->AddChild(&m_textMessage);
	FOREACH_PlayerNumber( p )
		this->AddChild(&m_textCredits[p]);

	ReloadCreditsText();
	/* This will be done when we set up the first screen, after GAMESTATE->Reset has
	 * been called. */
	// RefreshCreditsMessages();

	m_sprOverlay.Load( THEME->GetPathB("ScreenSystemLayer", "overlay") );
	this->AddChild( m_sprOverlay );
}

void ScreenSystemLayer::ReloadCreditsText()
{
	if( m_sprMessageFrame.IsLoaded() )
		this->RemoveChild( m_sprMessageFrame );
	m_sprMessageFrame.Load( THEME->GetPathG(m_sName,"MessageFrame") );
	this->AddChild( m_sprMessageFrame );
	m_sprMessageFrame->SetName( "MessageFrame" );
	SET_XY_AND_ON_COMMAND( m_sprMessageFrame );
	m_sprMessageFrame->SetVisible( false );

	m_textMessage.LoadFromFont( THEME->GetPathF(m_sName,"message") );
	m_textMessage.SetName( "Message" );
	SET_XY_AND_ON_COMMAND( m_textMessage );
	m_textMessage.SetVisible( false );


	FOREACH_PlayerNumber( p )
	{
		m_textCredits[p].LoadFromFont( THEME->GetPathF("ScreenManager","credits") );
		m_textCredits[p].SetName( ssprintf("CreditsP%d",p+1) );
		SET_XY_AND_ON_COMMAND( &m_textCredits[p] );
	}

	this->SortByDrawOrder();
}

RString ScreenSystemLayer::GetCreditsMessage( PlayerNumber pn ) const
{
	if( (bool)CREDITS_JOIN_ONLY && !GAMESTATE->PlayersCanJoin() )
		return RString();

	bool bShowCreditsMessage;
	if( SCREENMAN && SCREENMAN->GetTopScreen() && SCREENMAN->GetTopScreen()->GetScreenType() == system_menu )
		bShowCreditsMessage = true;
	else if( MEMCARDMAN->GetCardsLocked() )
		bShowCreditsMessage = !GAMESTATE->IsPlayerEnabled( pn );
	else 
		bShowCreditsMessage = !GAMESTATE->m_bSideIsJoined[pn];
		
	if( !bShowCreditsMessage )
	{
		MemoryCardState mcs = MEMCARDMAN->GetCardState( pn );
		const Profile* pProfile = PROFILEMAN->GetProfile( pn );
		switch( mcs )
		{
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

		case MemoryCardState_Error: 		return THEME->GetMetric( m_sName, "CreditsCard" + MEMCARDMAN->GetCardError(pn) );
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
		default:
			FAIL_M( ssprintf("%i",mcs) );
		}
	}
	else // bShowCreditsMessage
	{
		switch( GAMESTATE->GetCoinMode() )
		{
		case COIN_MODE_HOME:
			if( GAMESTATE->PlayersCanJoin() )
				return CREDITS_PRESS_START.GetValue();
			else
				return CREDITS_NOT_PRESENT.GetValue();

		case COIN_MODE_PAY:
		{
			int Credits = GAMESTATE->m_iCoins / PREFSMAN->m_iCoinsPerCredit;
			int Coins = GAMESTATE->m_iCoins % PREFSMAN->m_iCoinsPerCredit;
			RString sCredits = CREDITS_CREDITS;
			if( Credits > 0 || PREFSMAN->m_iCoinsPerCredit == 1 )
				sCredits += ssprintf("  %d", Credits);
			if( PREFSMAN->m_iCoinsPerCredit > 1 )
				sCredits += ssprintf("  %d/%d", Coins, PREFSMAN->m_iCoinsPerCredit.Get() );
			return sCredits;
		}
		case COIN_MODE_FREE:
			if( GAMESTATE->PlayersCanJoin() )
				return CREDITS_FREE_PLAY.GetValue();
			else
				return CREDITS_NOT_PRESENT.GetValue();

		default:
			ASSERT(0);
		}
	}
}

void ScreenSystemLayer::HandleMessage( const Message &msg )
{
	bool bJoinedMessage = false;
	FOREACH_PlayerNumber( pn )
		if( msg == enum_add2(Message_SideJoinedP1, pn) )
			bJoinedMessage = true;

	if( msg == "RefreshCreditText" || msg == "CoinInserted" || bJoinedMessage )
	{
		// update joined
		FOREACH_PlayerNumber( pn )
			m_textCredits[pn].SetText( GetCreditsMessage(pn) );
	}
	else if( msg == "SystemMessage" )
	{
		m_sprMessageFrame->SetVisible( true );
		m_sprMessageFrame->PlayCommand( "On" );
		m_sprMessageFrame->PlayCommand( "Off" );

		m_textMessage.SetVisible( true );
		m_textMessage.SetText( SCREENMAN->GetCurrentSystemMessage() );
		m_textMessage.PlayCommand( "On" );
		m_textMessage.PlayCommand( "Off" );
	}
	else if( msg == "SystemMessageNoAnimate" )
	{
		m_sprMessageFrame->SetVisible( true );
		m_sprMessageFrame->PlayCommand( "On" );
		m_sprMessageFrame->FinishTweening();
		m_sprMessageFrame->PlayCommand( "Off" );

		m_textMessage.SetVisible( true );
		m_textMessage.SetText( SCREENMAN->GetCurrentSystemMessage() );
		m_textMessage.PlayCommand( "On" );
		m_textMessage.FinishTweening();
		m_textMessage.PlayCommand( "Off" );
	}
	else if( msg == "HideSystemMessage" )
	{
		m_sprMessageFrame->SetVisible( false );

		m_textMessage.SetVisible( false );
	}
	Screen::HandleMessage( msg );
}

void ScreenSystemLayer::Update( float fDeltaTime )
{
	Screen::Update(fDeltaTime);
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
