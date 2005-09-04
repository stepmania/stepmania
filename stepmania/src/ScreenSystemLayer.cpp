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
#include "RageDisplay.h"
#include "RageLog.h"
#include "Command.h"
#include "ScreenDimensions.h"


REGISTER_SCREEN_CLASS( ScreenSystemLayer );
ScreenSystemLayer::ScreenSystemLayer( const CString &sName ) : Screen(sName)
{
	SubscribeToMessage( "RefreshCreditText" );
	SubscribeToMessage( "SystemMessage" );
	SubscribeToMessage( "SystemMessageNoAnimate" );
	SubscribeToMessage( "HideSystemMessage" );

	CREDITS_PRESS_START		.Load(m_sName,"CreditsPressStart");
	CREDITS_INSERT_CARD		.Load(m_sName,"CreditsInsertCard");
	CREDITS_CARD_TOO_LATE	.Load(m_sName,"CreditsCardTooLate");
	CREDITS_CARD_NO_NAME	.Load(m_sName,"CreditsCardNoName");
	CREDITS_CARD_READY		.Load(m_sName,"CreditsCardReady");
	CREDITS_CARD_CHECKING	.Load(m_sName,"CreditsCardChecking");
	CREDITS_CARD_REMOVED	.Load(m_sName,"CreditsCardRemoved");
	CREDITS_FREE_PLAY		.Load(m_sName,"CreditsFreePlay");
	CREDITS_CREDITS			.Load(m_sName,"CreditsCredits");
	CREDITS_NOT_PRESENT		.Load(m_sName,"CreditsNotPresent");
	CREDITS_LOAD_FAILED		.Load(m_sName,"CreditsLoadFailed");
	CREDITS_LOADED_FROM_LAST_GOOD_APPEND.Load(m_sName,"CreditsLoadedFromLastGoodAppend");
	CREDITS_JOIN_ONLY		.Load( m_sName, "CreditsJoinOnly" );
}

ScreenSystemLayer::~ScreenSystemLayer()
{
}

void ScreenSystemLayer::Init()
{
	Screen::Init();

	this->AddChild(&m_textMessage);
	this->AddChild(&m_textStats);
	this->AddChild(&m_textTime);
	FOREACH_PlayerNumber( p )
		this->AddChild(&m_textCredits[p]);


	/* "Was that a skip?"  This displays a message when an update takes
	 * abnormally long, to quantify skips more precisely, verify them
	 * when they're subtle, and show the time it happened, so you can pinpoint
	 * the time in the log.  Put a dim quad behind it to make it easier to
	 * read. */
	m_LastSkip = 0;
	const float SKIP_LEFT = 320.0f, SKIP_TOP = 60.0f, 
		SKIP_WIDTH = 160.0f, SKIP_Y_DIST = 16.0f;

	m_quadSkipBackground.StretchTo(RectF(SKIP_LEFT-8, SKIP_TOP-8,
						SKIP_LEFT+SKIP_WIDTH, SKIP_TOP+SKIP_Y_DIST*NUM_SKIPS_TO_SHOW));
	m_quadSkipBackground.SetDiffuse( RageColor(0,0,0,0.4f) );
	m_quadSkipBackground.SetHidden( !PREFSMAN->m_bTimestamping );
	this->AddChild(&m_quadSkipBackground);

	for( int i=0; i<NUM_SKIPS_TO_SHOW; i++ )
	{
		/* This is somewhat big.  Let's put it on the right side, where it'll
		 * obscure the 2P side during gameplay; there's nowhere to put it that
		 * doesn't obscure something, and it's just a diagnostic. */
		m_textSkips[i].LoadFromFont( THEME->GetPathF("Common","normal") );
		m_textSkips[i].SetXY( SKIP_LEFT, SKIP_TOP + SKIP_Y_DIST*i );
		m_textSkips[i].SetHorizAlign( Actor::align_left );
		m_textSkips[i].SetVertAlign( Actor::align_top );
		m_textSkips[i].SetZoom( 0.5f );
		m_textSkips[i].SetDiffuse( RageColor(1,1,1,0) );
		m_textSkips[i].SetShadowLength( 0 );
		this->AddChild(&m_textSkips[i]);
	}

	m_quadBrightnessAdd.StretchTo( RectF(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	m_quadBrightnessAdd.SetDiffuse( RageColor(1,1,1,PREFSMAN->m_fBrightnessAdd) );
	m_quadBrightnessAdd.SetHidden( PREFSMAN->m_fBrightnessAdd == 0 );
	m_quadBrightnessAdd.SetBlendMode( BLEND_ADD );
	this->AddChild( &m_quadBrightnessAdd );


	ReloadCreditsText();
	/* This will be done when we set up the first screen, after GAMESTATE->Reset has
	 * been called. */
	// RefreshCreditsMessages();
}

void ScreenSystemLayer::ReloadCreditsText()
{
	if( m_sprMessageFrame.IsLoaded() )
		this->RemoveChild( m_sprMessageFrame );
	m_sprMessageFrame.Load( THEME->GetPathG(m_sName,"MessageFrame") );
	this->AddChild( m_sprMessageFrame );
	m_sprMessageFrame->SetName( "MessageFrame" );
	SET_XY_AND_ON_COMMAND( m_sprMessageFrame );
	m_sprMessageFrame->SetHidden( true );

	m_textMessage.LoadFromFont( THEME->GetPathF(m_sName,"message") );
	m_textMessage.SetName( "Message" );
	SET_XY_AND_ON_COMMAND( m_textMessage );
	m_textMessage.SetHidden( true );

 	m_textStats.LoadFromFont( THEME->GetPathF(m_sName,"stats") );
	m_textStats.SetName( "Stats" );
	SET_XY_AND_ON_COMMAND( m_textStats ); 

	m_textTime.LoadFromFont( THEME->GetPathF(m_sName,"time") );
	m_textTime.SetName( "Time" );
	m_textTime.SetHidden( !PREFSMAN->m_bTimestamping );
	SET_XY_AND_ON_COMMAND( m_textTime ); 

	FOREACH_PlayerNumber( p )
	{
		m_textCredits[p].LoadFromFont( THEME->GetPathF("ScreenManager","credits") );
		m_textCredits[p].SetName( ssprintf("CreditsP%d",p+1) );
		SET_XY_AND_ON_COMMAND( &m_textCredits[p] );
	}

	this->SortByDrawOrder();
}

CString ScreenSystemLayer::GetCreditsMessage( PlayerNumber pn ) const
{
	if( (bool)CREDITS_JOIN_ONLY && !GAMESTATE->PlayersCanJoin() )
		return CString();

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
		case MEMORY_CARD_STATE_NO_CARD:
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
				return CString();

		case MEMORY_CARD_STATE_ERROR: 		return THEME->GetMetric( m_sName, "CreditsCard" + MEMCARDMAN->GetCardError(pn) );
		case MEMORY_CARD_STATE_TOO_LATE:	return CREDITS_CARD_TOO_LATE.GetValue();
		case MEMORY_CARD_STATE_CHECKING:	return CREDITS_CARD_CHECKING.GetValue();
		case MEMORY_CARD_STATE_REMOVED:		return CREDITS_CARD_REMOVED.GetValue();
		case MEMORY_CARD_STATE_READY:
			{
				// If the profile failed to load and there was no usable backup...
				if( PROFILEMAN->LastLoadWasTamperedOrCorrupt(pn) && !PROFILEMAN->LastLoadWasFromLastGood(pn) )
					return CREDITS_LOAD_FAILED.GetValue();

				// If there is a local profile loaded, prefer it over the name of the memory card.
				if( PROFILEMAN->IsPersistentProfile(pn) )
				{
					CString s = pProfile->GetDisplayNameOrHighScoreName();
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
			CString sCredits = CREDITS_CREDITS;
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

void ScreenSystemLayer::HandleMessage( const CString &sMessage )
{
	if( sMessage == "RefreshCreditText" )
	{
		// update joined
		FOREACH_PlayerNumber( pn )
			m_textCredits[pn].SetText( GetCreditsMessage(pn) );
	}
	else if( sMessage == "SystemMessage" )
	{
		m_sprMessageFrame->SetHidden( false );
		m_sprMessageFrame->PlayCommand( "On" );
		m_sprMessageFrame->PlayCommand( "Off" );

		m_textMessage.SetHidden( false );
		m_textMessage.SetText( SCREENMAN->GetCurrentSystemMessage() );
		m_textMessage.PlayCommand( "On" );
		m_textMessage.PlayCommand( "Off" );
	}
	else if( sMessage == "SystemMessageNoAnimate" )
	{
		m_sprMessageFrame->SetHidden( false );
		m_sprMessageFrame->PlayCommand( "On" );
		m_sprMessageFrame->FinishTweening();
		m_sprMessageFrame->PlayCommand( "Off" );

		m_textMessage.SetHidden( false );
		m_textMessage.SetText( SCREENMAN->GetCurrentSystemMessage() );
		m_textMessage.PlayCommand( "On" );
		m_textMessage.FinishTweening();
		m_textMessage.PlayCommand( "Off" );
	}
	else if( sMessage == "HideSystemMessage" )
	{
		m_sprMessageFrame->SetHidden( true );

		m_textMessage.SetHidden( true );
	}
	Screen::HandleMessage( sMessage );
}

void ScreenSystemLayer::AddTimestampLine( const CString &txt, const RageColor &color )
{
	m_textSkips[m_LastSkip].SetText( txt );
	m_textSkips[m_LastSkip].StopTweening();
	m_textSkips[m_LastSkip].SetDiffuse( RageColor(1,1,1,1) );
	m_textSkips[m_LastSkip].BeginTweening( 0.2f );
	m_textSkips[m_LastSkip].SetDiffuse( color );
	m_textSkips[m_LastSkip].BeginTweening( 3.0f );
	m_textSkips[m_LastSkip].BeginTweening( 0.2f );
	m_textSkips[m_LastSkip].SetDiffuse( RageColor(1,1,1,0) );

	m_LastSkip++;
	m_LastSkip %= NUM_SKIPS_TO_SHOW;
}

void ScreenSystemLayer::UpdateSkips()
{
	if( !PREFSMAN->m_bTimestamping && !PREFSMAN->m_bLogSkips )
		return;

	/* Use our own timer, so we ignore `/tab. */
	const float UpdateTime = SkipTimer.GetDeltaTime();

	/* FPS is 0 for a little while after we load a screen; don't report
	 * during this time. Do clear the timer, though, so we don't report
	 * a big "skip" after this period passes. */
	if( !DISPLAY->GetFPS() )
		return;

	/* We want to display skips.  We expect to get updates of about 1.0/FPS ms. */
	const float ExpectedUpdate = 1.0f / DISPLAY->GetFPS();
	
	/* These are thresholds for severity of skips.  The smallest
	 * is slightly above expected, to tolerate normal jitter. */
	const float Thresholds[] =
	{
		ExpectedUpdate * 2.0f, ExpectedUpdate * 4.0f, 0.1f, -1
	};

	int skip = 0;
	while( Thresholds[skip] != -1 && UpdateTime > Thresholds[skip] )
		skip++;

	if( skip )
	{
		CString time(SecondsToMMSSMsMs(RageTimer::GetTimeSinceStartFast()));

		static const RageColor colors[] =
		{
			RageColor(0,0,0,0),		  /* unused */
			RageColor(0.2f,0.2f,1,1), /* light blue */
			RageColor(1,1,0,1),		  /* yellow */
			RageColor(1,0.2f,0.2f,1)  /* light red */
		};

		if( PREFSMAN->m_bTimestamping )
			AddTimestampLine( ssprintf("%s: %.0fms (%.0f)", time.c_str(), 1000*UpdateTime, UpdateTime/ExpectedUpdate), colors[skip] );
		if( PREFSMAN->m_bLogSkips )
			LOG->Trace( "Frame skip: %.0fms (%.0f)", 1000*UpdateTime, UpdateTime/ExpectedUpdate );
	}
}

void ScreenSystemLayer::Update( float fDeltaTime )
{
	Screen::Update(fDeltaTime);

	if( PREFSMAN  &&  (bool)PREFSMAN->m_bShowStats )
	{
		m_textStats.SetDiffuse( RageColor(1,1,1,0.7f) );
		m_textStats.SetText( DISPLAY->GetStats() );
	}
	else
	{
		m_textStats.SetDiffuse( RageColor(1,1,1,0) ); /* hide */
	}

	UpdateSkips();

	if( PREFSMAN->m_bTimestamping )
		m_textTime.SetText( SecondsToMMSSMsMs(RageTimer::GetTimeSinceStartFast()) );
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
