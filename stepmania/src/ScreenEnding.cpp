#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenEnding

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/


#include "ScreenEnding.h"
#include "SongManager.h"
#include "RageSounds.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "song.h"
#include "ProfileManager.h"
#include "ActorUtil.h"
#include "GameState.h"
#include "MemoryCardManager.h"
#include "RageLog.h"
#include "StyleDef.h"
#include "GameManager.h"


#define SCROLL_DELAY		THEME->GetMetricF("ScreenEnding","ScrollDelay")
#define SCROLL_SPEED		THEME->GetMetricF("ScreenEnding","ScrollSpeed")
#define TEXT_ZOOM			THEME->GetMetricF("ScreenEnding","TextZoom")



CString GetStatsLineTitle( PlayerNumber pn, int iLine )
{
	static const CString s[NUM_ENDING_STATS_LINES] = {
		"Total %s Points",
		"Total Calories",
		"Total Songs Played",
		"Current Combo",
	};
	StepsType st = GAMESTATE->GetCurrentStyleDef()->m_StepsType;
	return ssprintf( s[iLine], GAMEMAN->NotesTypeToThemedString(st).c_str() );
}

CString GetStatsLineValue( PlayerNumber pn, int iLine )
{
	Profile* pProfile = PROFILEMAN->GetProfile( pn );
	ASSERT( pProfile );

	StepsType st = GAMESTATE->GetCurrentStyleDef()->m_StepsType;
	switch( iLine )
	{
	case 0:		return Commify( pProfile->GetTotalHighScoreDancePointsForStepsType(st) );	
	case 1:		return pProfile->GetDisplayTotalCaloriesBurned();
	case 2:		return Commify( pProfile->GetTotalNumSongsPlayed() );
	case 3:		return Commify( pProfile->m_iCurrentCombo );
	default:	ASSERT(0);	return "";
	}
}


ScreenEnding::ScreenEnding( CString sClassName ) : ScreenAttract( sClassName, false/*dont reset GAMESTATE*/ )
{
	vector<Song*> arraySongs;
	SONGMAN->GetSongs( arraySongs );
	SortSongPointerArrayByTitle( arraySongs );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_bWaitingForRemoveCard[p] = false;

		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		Profile* pProfile = PROFILEMAN->GetProfile( (PlayerNumber)p );

		m_textPlayerName[p].LoadFromFont( THEME->GetPathToF("ScreenEnding player name") );
		m_textPlayerName[p].SetText( pProfile ? pProfile->GetDisplayName() : "NO CARD" );
		m_textPlayerName[p].SetName( ssprintf("PlayerNameP%d",p+1) );
		SET_XY_AND_ON_COMMAND( m_textPlayerName[p] );
		this->AddChild( &m_textPlayerName[p] );

		m_bWaitingForRemoveCard[p] = MEMCARDMAN->GetCardState((PlayerNumber)p)!=MEMORY_CARD_STATE_NO_CARD;

		if( pProfile == NULL )
			continue;	// don't show the stats lines
	
		for( int i=0; i<NUM_ENDING_STATS_LINES; i++ )
		{
			m_textStatsTitle[p][i].LoadFromFont( THEME->GetPathToF("ScreenEnding stats title") );
			m_textStatsTitle[p][i].SetText( GetStatsLineTitle((PlayerNumber)p, i) );
			m_textStatsTitle[p][i].SetName( ssprintf("StatsTitleP%dLine%d",p+1,i+1) );
			SET_XY_AND_ON_COMMAND( m_textStatsTitle[p][i] );
			this->AddChild( &m_textStatsTitle[p][i] );
		
			m_textStatsValue[p][i].LoadFromFont( THEME->GetPathToF("ScreenEnding stats value") );
			m_textStatsValue[p][i].SetText( GetStatsLineValue((PlayerNumber)p, i) );
			m_textStatsValue[p][i].SetName( ssprintf("StatsValueP%dLine%d",p+1,i+1) );
			SET_XY_AND_ON_COMMAND( m_textStatsValue[p][i] );
			this->AddChild( &m_textStatsValue[p][i] );
		}

		m_sprRemoveMemoryCard[p].SetName( ssprintf("RemoveCardP%d",p+1) );
		m_sprRemoveMemoryCard[p].Load( THEME->GetPathToG("ScreenEnding remove card") );
		SET_XY_AND_ON_COMMAND( m_sprRemoveMemoryCard[p] );
		this->AddChild( &m_sprRemoveMemoryCard[p] );
	}

	
	this->MoveToTail( &m_In );		// put it in the back so it covers up the stuff we just added
	this->MoveToTail( &m_Out );		// put it in the back so it covers up the stuff we just added

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("music scroll") );

	// Now that we've read the data from the profile, it's ok to Reset()
	GAMESTATE->Reset();

	// TRICKY: Don't let the timer take us to the next screen until after all memory cards 
	// have been removed.
	this->ClearMessageQueue();

	m_fTimeUntilBeginFadingOut = m_Background.GetLengthSeconds() - m_Out.GetLengthSeconds();
	if( m_fTimeUntilBeginFadingOut < 0 )
	{
		LOG->Warn( "Screen '%s' Out BGAnimation (%f seconds) is longer than Background BGAnimation (%f seconds); background BGA will be truncated",
			m_sName.c_str(), m_Out.GetLengthSeconds(), m_Background.GetLengthSeconds() );
		m_fTimeUntilBeginFadingOut = 0;
	}
}

ScreenEnding::~ScreenEnding()
{
}

void ScreenEnding::Update( float fDeltaTime )
{
	ScreenAttract::Update( fDeltaTime );

	m_fTimeUntilBeginFadingOut = max( 0, m_fTimeUntilBeginFadingOut-fDeltaTime );

	if( !m_In.IsTransitioning() && !m_Out.IsTransitioning() )
	{
		bool bAllRemoved = true;

		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( m_bWaitingForRemoveCard[p] )
			{
				m_bWaitingForRemoveCard[p] = MEMCARDMAN->GetCardState((PlayerNumber)p)!=MEMORY_CARD_STATE_NO_CARD;
				if( m_bWaitingForRemoveCard[p] )
				{
					bAllRemoved = false;
				}
				else
				{
					m_sprRemoveMemoryCard[p].SetHidden( true );
				}
			}
		}

		if( bAllRemoved )
			this->PostScreenMessage( SM_BeginFadingOut, m_fTimeUntilBeginFadingOut );
	}
}	

void ScreenEnding::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenAttract::HandleScreenMessage( SM );
}

