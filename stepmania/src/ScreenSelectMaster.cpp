#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectMaster

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenSelectMaster.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "AnnouncerManager.h"
#include "ModeChoice.h"
#include "ActorUtil.h"

#define NUM_ICON_PARTS							THEME->GetMetricI("ScreenSelectMaster","NumIconParts")
#define NUM_PREVIEW_PARTS						THEME->GetMetricI("ScreenSelectMaster","NumPreviewParts")
#define NUM_CURSOR_PARTS						THEME->GetMetricI("ScreenSelectMaster","NumCursorParts")
#define SHARED_PREVIEW_AND_CURSOR				THEME->GetMetricB("ScreenSelectMaster","SharedPreviewAndCursor")
#define NUM_CHOICES_ON_PAGE_1					THEME->GetMetricI("ScreenSelectMaster","NumChoicesOnPage1")
#define CURSOR_OFFSET_X_FROM_ICON( p, part )	THEME->GetMetricF("ScreenSelectMaster",ssprintf("CursorPart%dP%dOffsetXFromIcon",part+1,p+1))
#define CURSOR_OFFSET_Y_FROM_ICON( p, part )	THEME->GetMetricF("ScreenSelectMaster",ssprintf("CursorPart%dP%dOffsetYFromIcon",part+1,p+1))
#define DISABLED_COLOR							THEME->GetMetricC("ScreenSelectMaster","DisabledColor")
#define PRE_SWITCH_PAGE_SECONDS					THEME->GetMetricF("ScreenSelectMaster","PreSwitchPageSeconds")
#define POST_SWITCH_PAGE_SECONDS				THEME->GetMetricF("ScreenSelectMaster","PostSwitchPageSeconds")

const ScreenMessage SM_PlayPostSwitchPage = (ScreenMessage)(SM_User+1);

ScreenSelectMaster::ScreenSelectMaster() : ScreenSelect( "ScreenSelectMaster" )
{
	int p, i;

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_iChoice[p] = 0;
		m_bChosen[p] = false;
	}

	if( SHARED_PREVIEW_AND_CURSOR )
	{
		for( i=0; i<NUM_CURSOR_PARTS; i++ )
		{
			CString sFName = ssprintf("%s Cursor Part%d", m_sName.c_str(),i+1);
			m_sprCursor[i][0].SetName( ssprintf("CursorPart%d",i+1) );
			m_sprCursor[i][0].Load( THEME->GetPathToG(sFName) );
			this->AddChild( &m_sprCursor[i][0] );
		}
	}
	else
	{
		for( i=0; i<NUM_CURSOR_PARTS; i++ )
		{
			for( p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled(p) )
					continue;	// skip
				CString sFName = ssprintf("%s Cursor Part%d P%d", m_sName.c_str(),i+1,p+1);
				m_sprCursor[i][p].SetName( ssprintf("CursorPart%dP%d",i+1,p+1) );
				m_sprCursor[i][p].Load( THEME->GetPathToG(sFName) );
				this->AddChild( &m_sprCursor[i][p] );
			}
		}
	}

	for( unsigned c=0; c<m_aModeChoices.size(); c++ )
	{
		const ModeChoice& mc = m_aModeChoices[c];

		for( i=0; i<NUM_ICON_PARTS; i++ )
		{
			CString sFName = ssprintf("%s Icon Part%d Choice%d", m_sName.c_str(),i+1,c+1);
			m_sprIcon[i][c].SetName( ssprintf("IconPart%dChoice%d",i+1,c+1) );
			m_sprIcon[i][c].Load( THEME->GetPathToG(sFName) );
			this->AddChild( &m_sprIcon[i][c] );
		}

		if( SHARED_PREVIEW_AND_CURSOR )
		{
			for( i=0; i<NUM_PREVIEW_PARTS; i++ )
			{
				CString sFName = ssprintf("%s Preview Part%d Choice%d", m_sName.c_str(),i+1,c+1);
				m_sprPreview[i][c][0].SetName( ssprintf("PreviewPart%d",i+1) );
				m_sprPreview[i][c][0].Load( THEME->GetPathToG(sFName) );
				this->AddChild( &m_sprPreview[i][c][0] );
			}
		}
		else
		{
			for( i=0; i<NUM_PREVIEW_PARTS; i++ )
			{
				for( p=0; p<NUM_PLAYERS; p++ )
				{
					if( !GAMESTATE->IsPlayerEnabled(p) )
						continue;	// skip
					CString sFName = ssprintf("%s Preview Part%d Choice%d P%d", m_sName.c_str(),i+1,c+1,p+1);
					m_sprPreview[i][c][p].SetName( ssprintf("PreviewPart%dP%d",i+1,p+1) );
					m_sprPreview[i][c][p].Load( THEME->GetPathToG(sFName) );
					this->AddChild( &m_sprPreview[i][c][p] );
				}
			}
		}
	}

	for( int page=0; page<NUM_PAGES; page++ )
	{
		m_sprMore[page].SetName( ssprintf("MorePage%d",page+1) );
		m_sprMore[page].Load( THEME->GetPathToG( ssprintf("ScreenSelectMaster more page%d",page+1) ) );
		this->AddChild( &m_sprMore[page] );

		m_sprExplanation[page].SetName( ssprintf("ExplanationPage%d",page+1) );
		m_sprExplanation[page].Load( THEME->GetPathToG( ssprintf("ScreenSelectMaster explanation page%d",page+1) ) );
		this->AddChild( &m_sprExplanation[page] );
	}


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		CLAMP( m_iChoice[p], 0, (int)m_aModeChoices.size()-1 );
		m_bChosen[p] = false;
	}
	
	m_soundChange.Load( THEME->GetPathToS( "ScreenSelectMaster change") );
	m_soundSelect.Load( THEME->GetPathToS( "Common start") );
	m_soundDifficult.Load( ANNOUNCER->GetPathTo("select difficulty challenge") );

	m_fLockInputSecs = TweenOnScreen();
}

void ScreenSelectMaster::Update( float fDelta )
{
	ScreenSelect::Update( fDelta );
	m_fLockInputSecs = max( 0, m_fLockInputSecs-fDelta );
}

void ScreenSelectMaster::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenSelect::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_PlayPostSwitchPage:
		{
			float fSecs = 0;

			if( SHARED_PREVIEW_AND_CURSOR )
			{
				for( int i=0; i<NUM_CURSOR_PARTS; i++ )
				{
					m_sprCursor[i][0].SetXY( GetCursorX((PlayerNumber)0,i), GetCursorY((PlayerNumber)0,i) );
					fSecs = max( fSecs, COMMAND( m_sprCursor[i][0], "PostSwitchPage" ) );
				}
			}
			else
			{
				for( int i=0; i<NUM_CURSOR_PARTS; i++ )
					for( int p=0; p<NUM_PLAYERS; p++ )
						if( GAMESTATE->IsPlayerEnabled(p) )
						{
							m_sprCursor[i][p].SetXY( GetCursorX((PlayerNumber)p,i), GetCursorY((PlayerNumber)p,i) );
							fSecs = max( fSecs, COMMAND( m_sprCursor[i][p], "PostSwitchPage" ) );
						}
			}

			if( SHARED_PREVIEW_AND_CURSOR )
			{
				for( int i=0; i<NUM_PREVIEW_PARTS; i++ )
					fSecs = max( fSecs, COMMAND( m_sprPreview[i][m_iChoice[0]][0], "PostSwitchPage" ) );
			}
			else
			{
				for( int i=0; i<NUM_PREVIEW_PARTS; i++ )
					for( int p=0; p<NUM_PLAYERS; p++ )
						if( GAMESTATE->IsPlayerEnabled(p) )
							fSecs = max( fSecs, COMMAND( m_sprPreview[i][m_iChoice[p]][p], "PostSwitchPage" ) );
			}

			m_fLockInputSecs = POST_SWITCH_PAGE_SECONDS;
		}
		break;
	case SM_BeginFadingOut:
		float fSecs = TweenOffScreen();
		SCREENMAN->PostMessageToTopScreen( SM_AllDoneChoosing, fSecs );	// nofify parent that we're finished
		m_Menu.m_MenuTimer.Stop();
		break;
	}
}

int ScreenSelectMaster::GetSelectionIndex( PlayerNumber pn )
{
	return m_iChoice[pn];
}

void ScreenSelectMaster::UpdateSelectableChoices()
{
	/* XXX: If a player joins during the tween-in, this diffuse change
	 * will be undone by the tween.  Hmm. */
	for( unsigned c=0; c<m_aModeChoices.size(); c++ )
	{
		if( GAMESTATE->IsPlayable(m_aModeChoices[c]) )
			for( int i=0; i<NUM_ICON_PARTS; i++ )
				m_sprIcon[i][c].SetDiffuse( RageColor(1,1,1,1) );
		else
			for( int i=0; i<NUM_ICON_PARTS; i++ )
				m_sprIcon[i][c].SetDiffuse( DISABLED_COLOR );
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsHumanPlayer(p) )
		{
			MenuRight( (PlayerNumber)p );
			MenuLeft( (PlayerNumber)p );
		}
	}
}

void ScreenSelectMaster::MenuLeft( PlayerNumber pn )
{
	if( m_fLockInputSecs > 0 )
		return;
	if( m_bChosen[pn] )
		return;
	if( m_iChoice[pn] == 0 )	// can't go left any more
		return;

	if( GetPage(m_iChoice[pn]) != GetPage(m_iChoice[pn]-1) )
		ChangePage( GetPage(m_iChoice[pn]-1) );
	else
		ChangeSelection( pn, m_iChoice[pn]-1 );
}


void ScreenSelectMaster::MenuRight( PlayerNumber pn )
{
	if( m_fLockInputSecs > 0 )
		return;
	if( m_bChosen[pn] )
		return;
	if( m_iChoice[pn] == (int)m_aModeChoices.size()-1 )	// can't go right any more
		return;

	if( GetPage(m_iChoice[pn]) != GetPage(m_iChoice[pn]+1) )
		ChangePage( GetPage(m_iChoice[pn]+1) );
	else
		ChangeSelection( pn, m_iChoice[pn]+1 );
}

void ScreenSelectMaster::ChangePage( Page newPage )
{
	// If anyone has already chosen, don't allow changing of pages
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsHumanPlayer(p) && m_bChosen[p] )
			return;

	float fSecs = 0;

	if( SHARED_PREVIEW_AND_CURSOR )
	{
		for( int i=0; i<NUM_CURSOR_PARTS; i++ )
			fSecs = max( fSecs, COMMAND( m_sprCursor[i][0], "PreSwitchPage" ) );
	}
	else
	{
		for( int i=0; i<NUM_CURSOR_PARTS; i++ )
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsHumanPlayer(p) )
					fSecs = max( fSecs, COMMAND( m_sprCursor[i][p], "PreSwitchPage" ) );
	}

	const CString sIconAndExplanationCommand = ssprintf( "SwitchToPage%d", newPage+1 );

	for( unsigned c=0; c<m_aModeChoices.size(); c++ )
		for( int i=0; i<NUM_ICON_PARTS; i++ )
			fSecs = max( fSecs, COMMAND( m_sprIcon[i][c], sIconAndExplanationCommand ) );

	if( SHARED_PREVIEW_AND_CURSOR )
	{
		for( int i=0; i<NUM_PREVIEW_PARTS; i++ )
			fSecs = max( fSecs, COMMAND( m_sprPreview[i][m_iChoice[p]][0], "PreSwitchPage" ) );
	}
	else
	{
		for( int i=0; i<NUM_PREVIEW_PARTS; i++ )
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					fSecs = max( fSecs, COMMAND( m_sprPreview[i][m_iChoice[p]][p], "PreSwitchPage" ) );
	}

	for( int page=0; page<NUM_PAGES; page++ )
	{
		fSecs = max( fSecs, COMMAND( m_sprExplanation[page], sIconAndExplanationCommand ) );
		fSecs = max( fSecs, COMMAND( m_sprMore[page], sIconAndExplanationCommand ) );
	}



	if( newPage == PAGE_2 )
	{
		// XXX: only play this once (I thought we already did that?)
		// DDR plays it on every change to page 2.  -Chris
		m_soundDifficult.Stop();
		m_soundDifficult.PlayRandom();
	}

	// change both players
	int iNewChoice = (newPage==PAGE_1) ? (NUM_CHOICES_ON_PAGE_1-1) : NUM_CHOICES_ON_PAGE_1;
	for( p=0; p<NUM_PLAYERS; p++ )
		m_iChoice[p] = iNewChoice;

	m_soundChange.Play();

	m_fLockInputSecs = PRE_SWITCH_PAGE_SECONDS;
	this->PostScreenMessage( SM_PlayPostSwitchPage, PRE_SWITCH_PAGE_SECONDS );
}

void ScreenSelectMaster::ChangeSelection( PlayerNumber pn, int iNewChoice )
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		if( p!=pn && GetCurrentPage()==PAGE_1 )
			continue;	// skip

		const int iOldChoice = m_iChoice[p];

		if( SHARED_PREVIEW_AND_CURSOR )
		{
			for( int i=0; i<NUM_PREVIEW_PARTS; i++ )
			{
				COMMAND( m_sprPreview[i][iOldChoice][0], "LoseFocus" );
				COMMAND( m_sprPreview[i][iNewChoice][0], "GainFocus" );
			}
		}
		else
		{
			for( int i=0; i<NUM_PREVIEW_PARTS; i++ )
			{
				COMMAND( m_sprPreview[i][iOldChoice][p], "LoseFocus" );
				COMMAND( m_sprPreview[i][iNewChoice][p], "GainFocus" );
			}
		}

		m_iChoice[p] = iNewChoice;

		if( SHARED_PREVIEW_AND_CURSOR )
		{
			for( int i=0; i<NUM_CURSOR_PARTS; i++ )
			{
				COMMAND( m_sprCursor[i][0], "Change" );
				m_sprCursor[i][0].SetXY( GetCursorX((PlayerNumber)0,i), GetCursorY((PlayerNumber)0,i) );
			}
		}
		else
		{
			for( int i=0; i<NUM_CURSOR_PARTS; i++ )
			{
				COMMAND( m_sprCursor[i][p], "Change" );
				m_sprCursor[i][p].SetXY( GetCursorX((PlayerNumber)p,i), GetCursorY((PlayerNumber)p,i) );
			}
		}
	}

	m_soundChange.Play();
}

ScreenSelectMaster::Page ScreenSelectMaster::GetPage( int iChoiceIndex )
{
	if( iChoiceIndex < NUM_CHOICES_ON_PAGE_1 )
		return PAGE_1;
	else
		return PAGE_2;
}

ScreenSelectMaster::Page ScreenSelectMaster::GetCurrentPage()
{
	// Both players are guaranteed to be on the same page.
	return GetPage( m_iChoice[GAMESTATE->m_MasterPlayerNumber] );
}


void ScreenSelectMaster::MenuStart( PlayerNumber pn )
{
	if( m_fLockInputSecs > 0 )
		return;
	if( m_bChosen[pn] == true )
		return;
	m_bChosen[pn] = true;

	for( int page=0; page<NUM_PAGES; page++ )
		OFF_COMMAND( m_sprMore[page] );

	const ModeChoice& mc = m_aModeChoices[m_iChoice[pn]];
	/* Don't play sound if we're on the second page and another player
	 * has already selected, since it just played. */
	bool AnotherPlayerSelected = false;
	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
		if(p != pn && m_bChosen[p])
			AnotherPlayerSelected = true;

	if(GetCurrentPage() != PAGE_2 || !AnotherPlayerSelected)
	{
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo(ssprintf("ScreenSelectMaster comment %s",mc.name)) );
		m_soundSelect.Play();
	}

	if( GetCurrentPage() == PAGE_2 )
	{
		// choose this for all the other players too
		for( p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsHumanPlayer(p) && !m_bChosen[p] )
				MenuStart( (PlayerNumber)p );
	}

	float fSecs = 0;

	for( int i=0; i<NUM_CURSOR_PARTS; i++ )
		fSecs = max( fSecs, COMMAND( m_sprCursor[i][pn], "Choose") );

	// check to see if everyone has chosen
	bool bAllDone = true;
	for( p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsHumanPlayer((PlayerNumber)p) )
			bAllDone &= m_bChosen[p];
	if( bAllDone )
		this->PostScreenMessage( SM_BeginFadingOut, fSecs );// tell our owner it's time to move on
}

float ScreenSelectMaster::TweenOnScreen() 
{
	float fSecs = 0;

	for( unsigned c=0; c<m_aModeChoices.size(); c++ )
	{
		for( int i=0; i<NUM_ICON_PARTS; i++ )
			fSecs = max( fSecs, SET_XY_AND_ON_COMMAND( m_sprIcon[i][c] ) );

		if( SHARED_PREVIEW_AND_CURSOR )
		{
			int p=0;
			for( int i=0; i<NUM_PREVIEW_PARTS; i++ )
			{
				SET_XY( m_sprPreview[i][c][p] );
				if( c==m_iChoice[p] )
					fSecs = max( fSecs, ON_COMMAND( m_sprPreview[i][c][p] ) );
				else
					m_sprPreview[i][c][p].SetDiffuseAlpha(0);
			}
		}
		else
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					for( int i=0; i<NUM_PREVIEW_PARTS; i++ )
					{
						SET_XY( m_sprPreview[i][c][p] );
						if( c==m_iChoice[p] )
							fSecs = max( fSecs, ON_COMMAND( m_sprPreview[i][c][p] ) );
						else
							m_sprPreview[i][c][p].SetDiffuseAlpha(0);
					}
		}
	}

	if( SHARED_PREVIEW_AND_CURSOR )
	{
		for( int i=0; i<NUM_PREVIEW_PARTS; i++ )
			fSecs = max( fSecs, COMMAND( m_sprPreview[i][m_iChoice[0]][0], "GainFocus" ) );
	}
	else
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsPlayerEnabled(p) )
				for( int i=0; i<NUM_PREVIEW_PARTS; i++ )
					fSecs = max( fSecs, COMMAND( m_sprPreview[i][m_iChoice[p]][p], "GainFocus" ) );
	}


	// Need to SetXY of Cursor after Icons since it depends on the Icons' positions.
	if( SHARED_PREVIEW_AND_CURSOR )
	{
		for( int i=0; i<NUM_CURSOR_PARTS; i++ )
		{
			m_sprCursor[i][0].SetXY( GetCursorX((PlayerNumber)0,i), GetCursorY((PlayerNumber)0,i) );
			fSecs = max( fSecs, ON_COMMAND( m_sprCursor[i][0] ) );
		}
	}
	else
	{
		for( int i=0; i<NUM_CURSOR_PARTS; i++ )
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
				{
					m_sprCursor[i][p].SetXY( GetCursorX((PlayerNumber)p,i), GetCursorY((PlayerNumber)p,i) );
					fSecs = max( fSecs, ON_COMMAND( m_sprCursor[i][p] ) );
				}
	}


	fSecs = max( fSecs, SET_XY_AND_ON_COMMAND( m_sprExplanation[GetCurrentPage()] ) );
	fSecs = max( fSecs, SET_XY_AND_ON_COMMAND( m_sprMore[GetCurrentPage()] ) );

	this->SortByZ();

	return fSecs;
}

float ScreenSelectMaster::TweenOffScreen()
{
	float fSecs = 0;

	if( SHARED_PREVIEW_AND_CURSOR )
	{
		for( int i=0; i<NUM_CURSOR_PARTS; i++ )
			fSecs = max( fSecs, OFF_COMMAND( m_sprCursor[i][0] ) );
	}
	else
	{
		for( int i=0; i<NUM_CURSOR_PARTS; i++ )
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					fSecs = max( fSecs, OFF_COMMAND( m_sprCursor[i][p] ) );
	}

	for( unsigned c=0; c<m_aModeChoices.size(); c++ )
	{
		if( GetPage(c) != GetCurrentPage() )
			continue;	// skip

		for( int i=0; i<NUM_ICON_PARTS; i++ )
			fSecs = max( fSecs, OFF_COMMAND( m_sprIcon[i][c] ) );

		if( SHARED_PREVIEW_AND_CURSOR )
		{
			for( int i=0; i<NUM_PREVIEW_PARTS; i++ )
				fSecs = max( fSecs, OFF_COMMAND( m_sprPreview[i][c][0] ) );
		}
		else
		{
			for( int i=0; i<NUM_PREVIEW_PARTS; i++ )
				for( int p=0; p<NUM_PLAYERS; p++ )
					if( GAMESTATE->IsPlayerEnabled(p) )
						fSecs = max( fSecs, OFF_COMMAND( m_sprPreview[i][c][p] ) );
		}
	}

	fSecs = max( fSecs, OFF_COMMAND( m_sprExplanation[GetCurrentPage()] ) );
	fSecs = max( fSecs, OFF_COMMAND( m_sprMore[GetCurrentPage()] ) );

	return fSecs;
}


float ScreenSelectMaster::GetCursorX( PlayerNumber pn, int iPartIndex )
{
	return m_sprIcon[0][m_iChoice[pn]].GetX() + CURSOR_OFFSET_X_FROM_ICON(pn, iPartIndex);
}

float ScreenSelectMaster::GetCursorY( PlayerNumber pn, int iPartIndex )
{
	return m_sprIcon[0][m_iChoice[pn]].GetY() + CURSOR_OFFSET_Y_FROM_ICON(pn, iPartIndex);
}
