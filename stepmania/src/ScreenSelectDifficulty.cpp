#include "global.h"
#include "ScreenSelectDifficulty.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameSoundManager.h"
#include "GameManager.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "AnnouncerManager.h"
#include "ModeChoice.h"
#include "ActorUtil.h"

#define NUM_CHOICES_ON_PAGE_1				THEME->GetMetricI(m_sName,"NumChoicesOnPage1")
#define LOCK_INPUT_SECONDS					THEME->GetMetricF(m_sName,"LockInputSeconds")
#define SLEEP_AFTER_CHOICE_SECONDS			THEME->GetMetricF(m_sName,"SleepAfterChoiceSeconds")
#define SLEEP_AFTER_TWEEN_OFF_SECONDS		THEME->GetMetricF(m_sName,"SleepAfterTweenOffSeconds")
#define CURSOR_CHOOSE_COMMAND				THEME->GetMetric (m_sName,"CursorChooseCommand")
#define CURSOR_OFFSET_X_FROM_PICTURE( p )	THEME->GetMetricF(m_sName,ssprintf("CursorP%dOffsetXFromPicture",p+1))
#define CURSOR_OFFSET_Y_FROM_PICTURE( p )	THEME->GetMetricF(m_sName,ssprintf("CursorP%dOffsetYFromPicture",p+1))
#define SHADOW_CHOOSE_COMMAND				THEME->GetMetric (m_sName,"ShadowChooseCommand")
#define SHADOW_LENGTH_X						THEME->GetMetricF(m_sName,"ShadowLengthX")
#define SHADOW_LENGTH_Y						THEME->GetMetricF(m_sName,"ShadowLengthY")
#define OK_CHOOSE_COMMAND					THEME->GetMetric (m_sName,"OKChooseCommand")
#define DISABLED_COLOR						THEME->GetMetricC(m_sName,"DisabledColor")

#define IGNORED_ELEMENT_COMMAND				THEME->GetMetric (m_sName,"IgnoredElementOnCommand")

ScreenSelectDifficulty::ScreenSelectDifficulty( CString sClassName ) : ScreenSelect( sClassName )
{
	m_CurrentPage = PAGE_1;

	FOREACH_PlayerNumber( p )
	{
		m_iChoiceOnPage[p] = 0;
		m_bChosen[p] = false;
	}

	unsigned c; // GCC is bitching again.
	for( c=0; c<m_aModeChoices.size(); c++ )
	{
		if( (int)c < NUM_CHOICES_ON_PAGE_1 )
			m_ModeChoices[PAGE_1].push_back( m_aModeChoices[c] );
		else
			m_ModeChoices[PAGE_2].push_back( m_aModeChoices[c] );
	}

	c = 0;
	for( int page=0; page<NUM_PAGES; page++ )
	{
		for( unsigned choice=0; choice<m_ModeChoices[page].size(); choice++, c++ )
		{
			CString sInfoFile = ssprintf( "%s info%d", m_sName.c_str(), c+1 );
			CString sPictureFile = ssprintf( "%s picture%d", m_sName.c_str(), c+1 );

			m_sprPicture[page][choice].SetName( ssprintf("PicturePage%dChoice%d",page+1,choice+1) );
			m_sprPicture[page][choice].Load( THEME->GetPathToG(sPictureFile) );
			m_framePages.AddChild( &m_sprPicture[page][choice] );

			m_sprInfo[page][choice].SetName( ssprintf("InfoPage%dChoice%d",page+1,choice+1) );
			m_sprInfo[page][choice].Load( THEME->GetPathToG(sInfoFile) );
			m_framePages.AddChild( &m_sprInfo[page][choice] );
		}

		m_sprMore[page].SetName( ssprintf("MorePage%d",page+1) );
		m_sprMore[page].Load( THEME->GetPathToG( ssprintf("%s more page%d", m_sName.c_str(),page+1) ) );
		m_framePages.AddChild( &m_sprMore[page] );

		m_sprExplanation[page].SetName( ssprintf("ExplanationPage%d",page+1) );
		m_sprExplanation[page].Load( THEME->GetPathToG( m_sName + " explanation") );
		m_sprExplanation[page].StopAnimating();
		m_sprExplanation[page].SetState( page );
		m_framePages.AddChild( &m_sprExplanation[page] );
	}


	FOREACH_PlayerNumber( p )
	{
		CLAMP( m_iChoiceOnPage[p], 0, (int)m_ModeChoices[0].size()-1 );
		m_bChosen[p] = false;

		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;

		m_sprShadow[p].SetName( "Shadow" );
		m_sprShadow[p].Load( THEME->GetPathToG( m_sName + " shadow 2x1") );
		m_sprShadow[p].StopAnimating();
		m_sprShadow[p].SetState( p );
		m_sprShadow[p].SetDiffuse( RageColor(0,0,0,0.6f) );
		m_framePages.AddChild( &m_sprShadow[p] );

		m_sprCursor[p].SetName( "Cursor" );
		m_sprCursor[p].Load( THEME->GetPathToG( m_sName + " cursor 2x1") );
		m_sprCursor[p].StopAnimating();
		m_sprCursor[p].SetState( p );
		m_framePages.AddChild( &m_sprCursor[p] );

		m_sprOK[p].SetName( "OK" );
		m_sprOK[p].Load( THEME->GetPathToG( m_sName + " ok 2x1") );
		m_sprOK[p].SetState( p );
		m_sprOK[p].StopAnimating();
		m_sprOK[p].SetDiffuse( RageColor(1,1,1,0) );
		m_framePages.AddChild( &m_sprOK[p] );
	}

	this->AddChild( &m_framePages );
	
	m_soundChange.Load( THEME->GetPathToS( m_sName + " change"), true );
	m_soundDifficult.Load( ANNOUNCER->GetPathTo("select difficulty challenge") );

	m_fLockInputTime = LOCK_INPUT_SECONDS;
	
	this->UpdateSelectableChoices();

	TweenOnScreen();

	this->SortByDrawOrder();
}

void ScreenSelectDifficulty::Update( float fDelta )
{
	ScreenSelect::Update( fDelta );
	m_fLockInputTime = max( 0, m_fLockInputTime-fDelta );
}

void ScreenSelectDifficulty::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenSelect::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_BeginFadingOut:
		TweenOffScreen();
		SCREENMAN->PostMessageToTopScreen( SM_AllDoneChoosing, SLEEP_AFTER_TWEEN_OFF_SECONDS );	// nofify parent that we're finished
		StopTimer();
		break;
	}
}

int ScreenSelectDifficulty::GetSelectionIndex( PlayerNumber pn )
{
	int index = 0;
	for( int page=0; page<m_CurrentPage; page++ )
		index += m_ModeChoices[page].size();
	index += m_iChoiceOnPage[pn];
	return index;
}

void ScreenSelectDifficulty::UpdateSelectableChoices()
{
	for( int page=0; page<NUM_PAGES; page++ )
	{
		/* XXX: If a player joins during the tween-in, this diffuse change
		 * will be undone by the tween.  Hmm. */
		for( unsigned i=0; i<m_ModeChoices[page].size(); i++ )
		{
			/* If the icon is text, use a dimmer diffuse, or we won't be
			 * able to see the glow. */
			if( m_ModeChoices[page][i].IsPlayable() )
			{
				m_sprInfo[page][i].SetDiffuse( RageColor(1,1,1,1) );
				m_sprPicture[page][i].SetDiffuse( RageColor(1,1,1,1) );
			}
			else
			{
				m_sprInfo[page][i].SetDiffuse( DISABLED_COLOR );
				m_sprPicture[page][i].SetDiffuse( DISABLED_COLOR );
			}
		}
	}

	// I'm not sure why this was here -- but there seem no ill effects
	// of it's removal.
	//
	//FOREACH_HumanPlayer( p )
	//	{
	//		MenuRight( p );
	//		MenuLeft( p );
	//	}
}

static bool BothPlayersModeChoice( const ModeChoice &mc )
{
	switch( mc.m_pm )
	{
	case PLAY_MODE_ONI:
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ENDLESS:
	case PLAY_MODE_RAVE:
		return true;
	}

	return false;
}

void ScreenSelectDifficulty::MenuLeft( PlayerNumber pn )
{
//	if( m_fLockInputTime > 0 )
//		return;
	if( m_bChosen[pn] )
		return;

	bool AnotherPlayerSelected = false;
	FOREACH_PlayerNumber( p )
		if( p != pn && m_bChosen[p] )
			AnotherPlayerSelected = true;

	int iSwitchToIndex = -1;
	for( int i=m_iChoiceOnPage[pn]-1; i>=0; i-- )
	{
		const ModeChoice &mc = m_ModeChoices[m_CurrentPage][i];
		if( AnotherPlayerSelected && BothPlayersModeChoice(mc) )
			continue;
		if( mc.IsPlayable() )
		{
			iSwitchToIndex = i;
			break;
		}
	}

	if( iSwitchToIndex == -1 )
	{
		if( m_CurrentPage > 0 )
			ChangePage( (Page)(m_CurrentPage-1) );
		return;
	}
	if( ChangeWithinPage( pn, iSwitchToIndex, false ) )
		m_soundChange.Play();
}


void ScreenSelectDifficulty::MenuRight( PlayerNumber pn )
{
//	if( m_fLockInputTime > 0 )
//		return;
	if( m_bChosen[pn] )
		return;

	bool AnotherPlayerSelected = false;
	FOREACH_PlayerNumber( p )
		if( p != pn && m_bChosen[p] )
			AnotherPlayerSelected = true;

	int iSwitchToIndex = -1;
	for( int i=m_iChoiceOnPage[pn]+1; i<(int) m_ModeChoices[m_CurrentPage].size(); i++ )
	{
		const ModeChoice &mc = m_ModeChoices[m_CurrentPage][i];
		if( AnotherPlayerSelected && BothPlayersModeChoice(mc) )
			continue;
		if( mc.IsPlayable() )
		{
			iSwitchToIndex = i;
			break;
		}
	}

	if( iSwitchToIndex == -1 )
	{
		if( m_ModeChoices[m_CurrentPage+1].size()==0 )	// there is no page 2
			return;

		if( m_CurrentPage < NUM_PAGES-1 )
			ChangePage( (Page)(m_CurrentPage+1) );
		return;
	}

	if( ChangeWithinPage( pn, iSwitchToIndex, false ) )
		m_soundChange.Play();
}

void ScreenSelectDifficulty::ChangePage( Page newPage )
{
	// If anyone has already chosen, don't allow changing of pages
	FOREACH_HumanPlayer( p )
		if( m_bChosen[p] )
			return;

	bool bPageIncreasing = newPage > m_CurrentPage;
	m_CurrentPage = newPage;

	if( newPage == PAGE_2 )
	{
		/* XXX: only play this once (I thought we already did that?) */
		m_soundDifficult.Stop();
		m_soundDifficult.PlayRandom();
	}

	// Find the first Playable mode on the new page
	int iSwitchToIndex = -1;
	if( !bPageIncreasing )
	{
		for( int i=m_ModeChoices[newPage].size()-1; i>=0; i-- )
		{
			if( m_ModeChoices[newPage][i].IsPlayable() )
			{
				iSwitchToIndex = i;
				break;
			}
		}
	} else {
		for( unsigned i=0; i<m_ModeChoices[newPage].size(); i++ )
		{
			if( m_ModeChoices[newPage][i].IsPlayable() )
			{
				iSwitchToIndex = i;
				break;
			}
		}
	}
	if( iSwitchToIndex == -1 )
		RageException::Throw( "%s has no selectable choices on page %i", m_sName.c_str(), newPage);

	// change both players
	FOREACH_PlayerNumber( p )
		ChangeWithinPage( p, iSwitchToIndex, true );

	m_soundChange.Play();

	// move frame with choices
	m_framePages.StopTweening();
	m_framePages.BeginTweening( 0.2f );
	m_framePages.SetX( (float)newPage*-SCREEN_WIDTH );
}

bool ScreenSelectDifficulty::ChangeWithinPage( PlayerNumber pn, int iNewChoice, bool bChangingPages )
{
	ASSERT_M( iNewChoice >= 0 && iNewChoice < (int) m_ModeChoices[m_CurrentPage].size(), ssprintf("%i, %i", iNewChoice, (int) m_ModeChoices[m_CurrentPage].size()) );

	bool bAnyChanged = false;
	FOREACH_HumanPlayer( p )
	{
		if( p!=pn && m_CurrentPage==PAGE_1 )
			continue;	// skip
		/* Don't do this.  If we were on page one, with P1 on choice 0, and P2 moves
		 * to the second page, then we're setting choice 0 on the second page; 
		 * m_iChoiceOnPage[p] is going from 0 to 0 (all that's changing is the page). */
//		if( m_iChoiceOnPage[p] == iNewChoice )
//			continue;	// skip

		bAnyChanged = true;
		m_iChoiceOnPage[p] = iNewChoice;

		float fCursorX = GetCursorX( p );
		float fCursorY = GetCursorY( p );

		m_sprCursor[p].StopTweening();
		m_sprCursor[p].BeginTweening( 0.2f, bChangingPages ? TWEEN_LINEAR : TWEEN_DECELERATE );
		m_sprCursor[p].SetX( fCursorX );
		m_sprCursor[p].SetY( fCursorY );

		m_sprShadow[p].StopTweening();
		m_sprShadow[p].BeginTweening( 0.2f, bChangingPages ? TWEEN_LINEAR : TWEEN_DECELERATE );
		m_sprShadow[p].SetX( fCursorX + SHADOW_LENGTH_X );
		m_sprShadow[p].SetY( fCursorY + SHADOW_LENGTH_Y );
	}

	return bAnyChanged;
}

void ScreenSelectDifficulty::MenuStart( PlayerNumber pn )
{
	if( m_fLockInputTime > 0 )
		return;
	if( m_bChosen[pn] )
		return;
	m_bChosen[pn] = true;

	for( int page=0; page<NUM_PAGES; page++ )
		OFF_COMMAND( m_sprMore[page] );

	const ModeChoice& mc = m_ModeChoices[m_CurrentPage][m_iChoiceOnPage[pn]];

	/* Don't play sound if we're recursive, since it just played. */
	static bool bPlaySelect = true;
	if( bPlaySelect )
	{
		SOUND->PlayOnceFromAnnouncer( ssprintf("ScreenSelectDifficulty comment %s",mc.m_sName.c_str()) );
		SCREENMAN->PlayStartSound();
	}

	// courses should be selected for both players at all times
	if( BothPlayersModeChoice(mc) )
	{
		FOREACH_HumanPlayer( p )
		{
			if( m_bChosen[p] || p == pn )
				continue;
	
			// move all cursors to the oni/nonstop selection so it graphically looks as if all players selected the same option.
			ChangeWithinPage( p, m_iChoiceOnPage[pn], false );
			bPlaySelect = false;
			MenuStart( p ); // agree everyone
			bPlaySelect = true;
		}
	}
	else // someone must have chosen arcade style play so oni/nonstop/endless must be disabled
	{
		FOREACH_HumanPlayer( p )
		{
			if( m_bChosen[p] || p == pn )
				continue;

			if( !BothPlayersModeChoice(m_ModeChoices[m_CurrentPage][m_iChoiceOnPage[p]]) )
				continue;

			/* This player is currently on a choice that is no longer available due to
			 * the selection just made. */
			int iSwitchToIndex = -1;
			int i;
			for( i=m_iChoiceOnPage[p]+1; iSwitchToIndex == -1 && i < (int) m_ModeChoices[m_CurrentPage].size(); ++i )
			{
				const ModeChoice &mc = m_ModeChoices[m_CurrentPage][i];
				if( mc.IsPlayable() && !BothPlayersModeChoice(mc) )
					iSwitchToIndex = i;
			}
			if( iSwitchToIndex == -1 ) /* couldn't find a spot looking up; look down */
			{
				for( i=m_iChoiceOnPage[p]-1; iSwitchToIndex == -1 && i >= 0; --i )
				{
					const ModeChoice &mc = m_ModeChoices[m_CurrentPage][i];
					if( mc.IsPlayable() && !BothPlayersModeChoice(mc) )
						iSwitchToIndex = i;
				}
			}
			/* We should always find a place to go--we should at least be able to choose
			 * the same thing pn picked. */
			ASSERT( iSwitchToIndex != -1 );

			// move the cursor
			ChangeWithinPage( p, iSwitchToIndex, false );
		}

		/* If the other player is active and hasn't yet chosen, gray out unselectable options.
		 * Otherwise, don't do this, so we don't gray out stuff when nothing else can be selected
		 * anyway. */
		bool bAnyPlayersLeft = false;
		FOREACH_HumanPlayer( p )
		{
			if( m_bChosen[p] || p == pn )
				continue;
			bAnyPlayersLeft = true;
		}

		if( bAnyPlayersLeft )
		{
			for( unsigned c=0; c<m_ModeChoices[PAGE_1].size(); c++ )
			{
				if( BothPlayersModeChoice(m_ModeChoices[PAGE_1][c]) )
				{
					m_sprPicture[PAGE_1][c].Command( IGNORED_ELEMENT_COMMAND );
					m_sprInfo[PAGE_1][c].Command( IGNORED_ELEMENT_COMMAND );

				//	IGNORED_ELEMENT_COMMAND
				}
			//	m_ModeChoices[PAGE_1].push_back( m_aModeChoices[c] );
			}
		}
	}

	if( m_CurrentPage == PAGE_2 )
	{
		// choose this for all the other players too
		FOREACH_PlayerNumber( p )
		{
			if( m_bChosen[p] )
				continue;
		
			bPlaySelect = false;
			MenuStart( p );
			bPlaySelect = true;
		}
	}


	m_sprCursor[pn].Command( CURSOR_CHOOSE_COMMAND );
	m_sprOK[pn].SetXY( m_sprShadow[pn].GetDestX(), m_sprShadow[pn].GetDestY() );
	m_sprOK[pn].Command( OK_CHOOSE_COMMAND );
	m_sprShadow[pn].Command( SHADOW_CHOOSE_COMMAND );


	// check to see if everyone has chosen
	FOREACH_HumanPlayer( p )
	{
		if( !m_bChosen[p] )
			return;
	}
	this->PostScreenMessage( SM_BeginFadingOut, SLEEP_AFTER_CHOICE_SECONDS );	// tell our owner it's time to move on
}

void ScreenSelectDifficulty::TweenOnScreen() 
{
	for( int page=0; page<NUM_PAGES; page++ )
	{
		SET_XY_AND_ON_COMMAND( m_sprExplanation[page] );
		SET_XY_AND_ON_COMMAND( m_sprMore[page] );

		for( unsigned c=0; c<m_ModeChoices[page].size(); c++ )
		{			
			SET_XY_AND_ON_COMMAND( m_sprInfo[page][c] );
			SET_XY_AND_ON_COMMAND( m_sprPicture[page][c] );
		}
	}

	FOREACH_HumanPlayer( p )
	{
		float fCursorX = GetCursorX( p );
		float fCursorY = GetCursorY( p );

		m_sprCursor[p].SetXY( fCursorX, fCursorY );
		ON_COMMAND( m_sprCursor[p] );

		m_sprShadow[p].SetXY( fCursorX + SHADOW_LENGTH_X, fCursorY + SHADOW_LENGTH_Y );
		ON_COMMAND( m_sprShadow[p] );
	}
}

void ScreenSelectDifficulty::TweenOffScreen()
{	
	const int page = m_CurrentPage;

	OFF_COMMAND( m_sprExplanation[page] );
	OFF_COMMAND( m_sprMore[page] );

	FOREACH_HumanPlayer( p )
	{
		OFF_COMMAND( m_sprCursor[p] );
		OFF_COMMAND( m_sprOK[p] );
		OFF_COMMAND( m_sprShadow[p] );
	}

	for( unsigned c=0; c<m_ModeChoices[page].size(); c++ )
	{
		OFF_COMMAND( m_sprInfo[page][c] );
		OFF_COMMAND( m_sprPicture[page][c] );
	}
}


float ScreenSelectDifficulty::GetCursorX( PlayerNumber pn )
{
	return m_sprPicture[m_CurrentPage][m_iChoiceOnPage[pn]].GetX() + CURSOR_OFFSET_X_FROM_PICTURE(pn);
}

float ScreenSelectDifficulty::GetCursorY( PlayerNumber pn )
{
	return m_sprPicture[m_CurrentPage][m_iChoiceOnPage[pn]].GetY() + CURSOR_OFFSET_Y_FROM_PICTURE(pn);
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
