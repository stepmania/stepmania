#include "global.h"
#include "ScreenSystemLayer.h"
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


#define CREDITS_PRESS_START						THEME->GetMetric ("ScreenSystemLayer","CreditsPressStart")
#define CREDITS_INSERT_CARD						THEME->GetMetric ("ScreenSystemLayer","CreditsInsertCard")
#define CREDITS_CARD_ERROR						THEME->GetMetric ("ScreenSystemLayer","CreditsCardError")
#define CREDITS_CARD_TOO_LATE					THEME->GetMetric ("ScreenSystemLayer","CreditsCardTooLate")
#define CREDITS_CARD_NO_NAME					THEME->GetMetric ("ScreenSystemLayer","CreditsCardNoName")
#define CREDITS_CARD_READY						THEME->GetMetric ("ScreenSystemLayer","CreditsCardReady")
#define CREDITS_CARD_CHECKING					THEME->GetMetric ("ScreenSystemLayer","CreditsCardChecking")
#define CREDITS_CARD_REMOVED					THEME->GetMetric ("ScreenSystemLayer","CreditsCardRemoved")
#define CREDITS_FREE_PLAY						THEME->GetMetric ("ScreenSystemLayer","CreditsFreePlay")
#define CREDITS_CREDITS							THEME->GetMetric ("ScreenSystemLayer","CreditsCredits")
#define CREDITS_NOT_PRESENT						THEME->GetMetric ("ScreenSystemLayer","CreditsNotPresent")
#define CREDITS_LOAD_FAILED						THEME->GetMetric ("ScreenSystemLayer","CreditsLoadFailed")
#define CREDITS_LOADED_FROM_LAST_GOOD_APPEND	THEME->GetMetric ("ScreenSystemLayer","CreditsLoadedFromLastGoodAppend")
#define CREDITS_JOIN_ONLY						THEME->GetMetricB("ScreenSystemLayer","CreditsJoinOnly")


//REGISTER_SCREEN_CLASS( ScreenSystemLayer );
ScreenSystemLayer::ScreenSystemLayer() : Screen("ScreenSystemLayer")
{
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

	m_SkipBackground.StretchTo(RectF(SKIP_LEFT-8, SKIP_TOP-8,
						SKIP_LEFT+SKIP_WIDTH, SKIP_TOP+SKIP_Y_DIST*NUM_SKIPS_TO_SHOW));
	m_SkipBackground.SetDiffuse( RageColor(0,0,0,0.4f) );
	m_SkipBackground.SetHidden( !PREFSMAN->m_bTimestamping );
	this->AddChild(&m_SkipBackground);

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

	ReloadCreditsText();
	/* This will be done when we set up the first screen, after GAMESTATE->Reset has
	 * been called. */
	// RefreshCreditsMessages();
}

void ScreenSystemLayer::ReloadCreditsText()
{
	m_textMessage.LoadFromFont( THEME->GetPathF("ScreenSystemLayer","message") );
	m_textMessage.SetName( "Message" );
	SET_XY_AND_ON_COMMAND( m_textMessage ); 

 	m_textStats.LoadFromFont( THEME->GetPathF("ScreenSystemLayer","stats") );
	m_textStats.SetName( "Stats" );
	SET_XY_AND_ON_COMMAND( m_textStats ); 

	m_textTime.LoadFromFont( THEME->GetPathF("ScreenSystemLayer","time") );
	m_textTime.SetName( "Time" );
	m_textTime.SetHidden( !PREFSMAN->m_bTimestamping );
	SET_XY_AND_ON_COMMAND( m_textTime ); 

	FOREACH_PlayerNumber( p )
	{
		m_textCredits[p].LoadFromFont( THEME->GetPathF("ScreenManager","credits") );
		m_textCredits[p].SetName( ssprintf("CreditsP%d",p+1) );
		SET_XY_AND_ON_COMMAND( &m_textCredits[p] );
	}
}

void ScreenSystemLayer::SystemMessage( const CString &sMessage )
{
	m_textMessage.SetText( sMessage );
	ActorCommands c = ActorCommands( ParseCommands("finishtweening;diffusealpha,1;addx,-640;linear,0.5;addx,+640;sleep,5;linear,0.5;diffusealpha,0") );
	m_textMessage.RunCommands( c );
}

void ScreenSystemLayer::SystemMessageNoAnimate( const CString &sMessage )
{
	m_textMessage.FinishTweening();
	m_textMessage.SetText( sMessage );
	m_textMessage.SetX( 4 );
	m_textMessage.SetDiffuseAlpha( 1 );
	m_textMessage.BeginTweening( 5 );
	m_textMessage.BeginTweening( 0.5f );
	m_textMessage.SetDiffuse( RageColor(1,1,1,0) );
}

void ScreenSystemLayer::RefreshCreditsMessages()
{
	// update joined
	FOREACH_PlayerNumber( p )
	{
		CString sCredits;

		bool bShowCreditsMessage;
		if( GAMESTATE->m_bIsOnSystemMenu )
			bShowCreditsMessage = true;
		else if( MEMCARDMAN->GetCardsLocked() )
			bShowCreditsMessage = !GAMESTATE->IsPlayerEnabled( p );	
		else 
			bShowCreditsMessage = !GAMESTATE->m_bSideIsJoined[p];
		
		if( !bShowCreditsMessage )
		{
			MemoryCardState mcs = MEMCARDMAN->GetCardState( p );
			Profile* pProfile = PROFILEMAN->GetProfile( p );
			switch( mcs )
			{
			case MEMORY_CARD_STATE_NO_CARD:
				// this is a local machine profile
				if( PROFILEMAN->LastLoadWasFromLastGood(p) && pProfile )
					sCredits = pProfile->GetDisplayName() + CREDITS_LOADED_FROM_LAST_GOOD_APPEND;
				else if( PROFILEMAN->LastLoadWasTamperedOrCorrupt(p) )
					sCredits = CREDITS_LOAD_FAILED;
				// Prefer the name of the profile over the name of the card.
				else if( pProfile )
					sCredits = pProfile->GetDisplayName();
				else if( GAMESTATE->PlayersCanJoin() )
					sCredits = CREDITS_INSERT_CARD;
				else
					sCredits = "";
				break;
			case MEMORY_CARD_STATE_WRITE_ERROR:
				sCredits = CREDITS_CARD_ERROR;
				break;
			case MEMORY_CARD_STATE_TOO_LATE:
				sCredits = CREDITS_CARD_TOO_LATE;
				break;
			case MEMORY_CARD_STATE_CHECKING:
				sCredits = CREDITS_CARD_CHECKING;
				break;
			case MEMORY_CARD_STATE_REMOVED:
				sCredits = CREDITS_CARD_REMOVED;
				break;
			case MEMORY_CARD_STATE_READY:
				if( PROFILEMAN->LastLoadWasFromLastGood(p) && pProfile )
					sCredits = pProfile->GetDisplayName() + CREDITS_LOADED_FROM_LAST_GOOD_APPEND;
				else if( PROFILEMAN->LastLoadWasTamperedOrCorrupt(p) )
					sCredits = CREDITS_LOAD_FAILED;
				// Prefer the name of the profile over the name of the card.
				else if( pProfile )
					sCredits = pProfile->GetDisplayName();
				else if( !MEMCARDMAN->IsNameAvailable(p) )
					sCredits = CREDITS_CARD_READY;
				else if( !MEMCARDMAN->GetName(p).empty() )
					sCredits = MEMCARDMAN->GetName(p);
				else
					sCredits = CREDITS_CARD_NO_NAME;
				break;
			default:
				FAIL_M( ssprintf("%i",mcs) );
			}
		}
		else // bShowCreditsMessage
		{
			switch( GAMESTATE->GetCoinMode() )
			{
			case COIN_HOME:
				if( GAMESTATE->PlayersCanJoin() )
					sCredits = CREDITS_PRESS_START;
				else
					sCredits = CREDITS_NOT_PRESENT;
				break;
			case COIN_PAY:
				{
					int Credits = GAMESTATE->m_iCoins / PREFSMAN->m_iCoinsPerCredit;
					int Coins = GAMESTATE->m_iCoins % PREFSMAN->m_iCoinsPerCredit;
					sCredits = CREDITS_CREDITS;
					if( Credits > 0 || PREFSMAN->m_iCoinsPerCredit == 1 )
						sCredits += ssprintf("  %d", Credits);
					if( PREFSMAN->m_iCoinsPerCredit > 1 )
						sCredits += ssprintf("  %d/%d", Coins, PREFSMAN->m_iCoinsPerCredit );
				}
				break;
			case COIN_FREE:
				if( GAMESTATE->PlayersCanJoin() )
					sCredits = CREDITS_FREE_PLAY;
				else
					sCredits = CREDITS_NOT_PRESENT;
				break;
			default:
				ASSERT(0);
			}
		}

		if( CREDITS_JOIN_ONLY && !GAMESTATE->PlayersCanJoin() )
			sCredits = "";
		m_textCredits[p].SetText( sCredits );
	}
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

void ScreenSystemLayer::UpdateTimestampAndSkips()
{
	/* Use our own timer, so we ignore `/tab. */
	const float UpdateTime = SkipTimer.GetDeltaTime();

	/* FPS is 0 for a little while after we load a screen; don't report
	 * during this time. Do clear the timer, though, so we don't report
	 * a big "skip" after this period passes. */
	if(DISPLAY->GetFPS())
	{
		/* We want to display skips.  We expect to get updates of about 1.0/FPS ms. */
		const float ExpectedUpdate = 1.0f / DISPLAY->GetFPS();
		
		/* These are thresholds for severity of skips.  The smallest
		 * is slightly above expected, to tolerate normal jitter. */
		const float Thresholds[] = {
			ExpectedUpdate * 2.0f, ExpectedUpdate * 4.0f, 0.1f, -1
		};

		int skip = 0;
		while(Thresholds[skip] != -1 && UpdateTime > Thresholds[skip])
			skip++;

		if(skip)
		{
			CString time(SecondsToMMSSMsMs(RageTimer::GetTimeSinceStart()));

			static const RageColor colors[] = {
				RageColor(0,0,0,0),		  /* unused */
				RageColor(0.2f,0.2f,1,1), /* light blue */
				RageColor(1,1,0,1),		  /* yellow */
				RageColor(1,0.2f,0.2f,1)  /* light red */
			};

			if( PREFSMAN->m_bTimestamping )
				AddTimestampLine( ssprintf("%s: %.0fms (%.0f)", time.c_str(), 1000*UpdateTime, UpdateTime/ExpectedUpdate),
					colors[skip] );
			if( PREFSMAN->m_bLogSkips )
				LOG->Trace( "Frame skip: %.0fms (%.0f)", 1000*UpdateTime, UpdateTime/ExpectedUpdate );
		}
	}

	if( PREFSMAN->m_bTimestamping )
		m_textTime.SetText( SecondsToMMSSMsMs(RageTimer::GetTimeSinceStart()) );
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

	UpdateTimestampAndSkips();
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
