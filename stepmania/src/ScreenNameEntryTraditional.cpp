#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenNameEntryTraditional

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenNameEntryTraditional.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "RageSounds.h"
#include "ThemeManager.h"
#include "ScreenRanking.h"
#include "Course.h"
#include "ActorUtil.h"
#include <math.h>


//
// Defines specific to ScreenNameEntryTraditional
//
#define ALPHABET_GAP_X				THEME->GetMetricF("ScreenNameEntryTraditional","AlphabetGapX")
#define NUM_ALPHABET_DISPLAYED		THEME->GetMetricI("ScreenNameEntryTraditional","NumAlphabetDisplayed")

#define MAX_RANKING_NAME_LENGTH		THEME->GetMetricI("ScreenNameEntryTraditional","MaxRankingNameLength")


static const wchar_t NAME_CHARS[] =
{
	L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
};



static const int CHAR_OK = -1;


ScreenNameEntryTraditional::ScreenNameEntryTraditional( CString sClassName ) : Screen( sClassName )
{
	LOG->Trace( "ScreenNameEntryTraditional::ScreenNameEntryTraditional()" );



		// DEBUGGING STUFF
	GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;
	GAMESTATE->m_bSideIsJoined[PLAYER_2] = true;
	GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
	GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;

	int p;

	CStringArray asFeats[NUM_PLAYERS];
	vector<CString*> vpStringsToFill[NUM_PLAYERS];

	// Find out if players deserve to enter their name
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		GAMESTATE->GetRankingFeats( (PlayerNumber)p, asFeats[p], vpStringsToFill[p] );
		m_bStillEnteringName[p] = asFeats[p].size()>0;
	}

	if( !AnyStillEntering() )
	{
		/* Nobody made a high score. */
		HandleScreenMessage( SM_GoToNextScreen );
		return;
	}

	m_Menu.Load( m_sName );
	this->AddChild( &m_Menu );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !m_bStillEnteringName[p] )
			continue;	// skip

		ASSERT( GAMESTATE->IsHumanPlayer(p) );	// they better be enabled if they made a high score!

		m_sprNameFrame[p].SetName( ssprintf("EntryFrameP%i",p+1) );
		m_sprNameFrame[p].Load( THEME->GetPathToG( ssprintf("ScreenNameEntryTraditional name frame p%i",p+1) ) );
		SET_XY_AND_ON_COMMAND( m_sprNameFrame[p] );
		this->AddChild( &m_sprNameFrame[p] );

		m_Keyboard[p].SetName( ssprintf("KeyboardP%i",p+1) );
		SET_XY_AND_ON_COMMAND( m_Keyboard[p] );
		this->AddChild( &m_Keyboard[p] );

		/* Add letters to m_Keyboard. */
		const CString fontpath = THEME->GetPathToF("ScreenNameEntryTraditional letters");
		for( int ch = 0; NAME_CHARS[ch]; ++ch )
		{
			BitmapText *Letter = new BitmapText;
			Letter->LoadFromFont( fontpath );
			Letter->SetText( ssprintf("%lc", NAME_CHARS[ch]) );
			m_textAlphabet[p].push_back( Letter );
			m_Keyboard[p].AddChild( Letter );
			Letter->Command( THEME->GetMetric("ScreenNameEntryTraditional","AlphabetInitCommand") );

			m_AlphabetLetter[p].push_back( NAME_CHARS[ch] );
		}

		/* Add "OK". */
		{
			BitmapText *Letter = new BitmapText;
			Letter->LoadFromFont( fontpath );
			Letter->SetText( "!" ); // XXX: assign a glyph to "OK"
			m_textAlphabet[p].push_back( Letter );
			m_Keyboard[p].AddChild( Letter );

			m_AlphabetLetter[p].push_back( CHAR_OK );
			Letter->Command( THEME->GetMetric("ScreenNameEntryTraditional","OKInitCommand") );
		}

		m_sprCursor[p].SetName( ssprintf("CursorP%i",p+1) );
		m_sprCursor[p].Load( THEME->GetPathToG( ssprintf("ScreenNameEntryTraditional cursor p%i",p+1) ) );
		m_Keyboard[p].AddChild( &m_sprCursor[p] );

		m_textSelection[p].SetName( ssprintf("SelectionP%i",p+1) );
		m_textSelection[p].LoadFromFont( THEME->GetPathToF("ScreenNameEntryTraditional entry") );
		SET_XY_AND_ON_COMMAND( m_textSelection[p] );
		this->AddChild( &m_textSelection[p] );
		UpdateSelectionText( p );

		m_SelectedChar[p] = 0;

		PositionCharsAndCursor( p );
		for( unsigned i = 0; i < m_textAlphabet[p].size(); ++i )
			m_textAlphabet[p][i]->FinishTweening();

		m_Grade[p].SetName( ssprintf("GradeP%i",p+1) );
		m_Grade[p].Load( THEME->GetPathToG("ScreenNameEntryTraditional grades") );
		m_Grade[p].SetGrade( (PlayerNumber)p, GRADE_A/*grade[p]*/ );
		SET_XY_AND_ON_COMMAND( m_Grade[p] );
		this->AddChild( &m_Grade[p] );

		/* XXX: merge this with ScreenNameEntry */
		m_textCategory[p].LoadFromFont( THEME->GetPathToF("ScreenNameEntryTraditional category") );
		m_textCategory[p].SetName( ssprintf("Category", p+1) );
		SET_XY_AND_ON_COMMAND( m_textCategory[p] );
		m_textCategory[p].SetText( join("\n", asFeats[p]) );
		this->AddChild( &m_textCategory[p] );
	}

	m_soundKey.Load( THEME->GetPathToS("ScreenNameEntryTraditional key") );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenNameEntryTraditional music") );
}

static inline int wrapn( int x, int n )
{
	wrap( x, n );
	return x;
}

void ScreenNameEntryTraditional::PositionCharsAndCursor( int pn )
{
	const int Selected = m_SelectedChar[pn];
	const int NumDisplayed = NUM_ALPHABET_DISPLAYED;

	const int TotalDisplayed = (int)m_textAlphabet[pn].size();
	const int Start = wrapn( Selected - TotalDisplayed/2, TotalDisplayed );

	const int First = -NumDisplayed/2;
	const int Last = NumDisplayed/2;
	for( int i = 0; i < (int)m_textAlphabet[pn].size(); ++i )
	{
		const int Num = wrapn( Start+i, (int) m_textAlphabet[pn].size() );
		BitmapText *bt = m_textAlphabet[pn][Num];

		const int Pos = i - TotalDisplayed/2;
		const bool hidden = ( Pos < First || Pos > Last );
		const int ActualPos = clamp( Pos, First-1, Last+1 );

		bt->Command("stoptweening;accelerate,.12");
		bt->SetX( ActualPos * ALPHABET_GAP_X );
		bt->SetDiffuseAlpha( hidden? 0.0f:1.0f );
	}

	m_sprCursor[pn].SetXY( 0,0 );
}

bool ScreenNameEntryTraditional::AnyStillEntering() const
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( m_bStillEnteringName[p] )
			return true;
	return false;
}

ScreenNameEntryTraditional::~ScreenNameEntryTraditional()
{
	LOG->Trace( "ScreenNameEntryTraditional::~ScreenNameEntryTraditional()" );

	for( int p=0; p<NUM_PLAYERS; ++p )
	{
		for( unsigned i=0; i < m_textAlphabet[p].size(); ++i )
			delete m_textAlphabet[p][i];
	}
}

void ScreenNameEntryTraditional::Update( float fDelta )
{
	Screen::Update(fDelta);
}

void ScreenNameEntryTraditional::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenNameEntryTraditional::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		if( !m_Menu.m_Out.IsTransitioning() )
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				Finish( (PlayerNumber)p );
		}
		break;
	case SM_GoToNextScreen:
		{
			/* Hack: go back to the select course screen in event mode. */
			if( PREFSMAN->m_bEventMode && GAMESTATE->IsCourseMode() )
			{
				SCREENMAN->SetNewScreen( "ScreenSelectCourse" );
				break;
			}

			Grade max_grade = GRADE_E;
			vector<Song*> vSongs;
			StageStats stats;
			GAMESTATE->GetFinalEvalStatsAndSongs( stats, vSongs );

			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsHumanPlayer(p) )
					max_grade = max( max_grade, stats.GetGrade((PlayerNumber)p) );
			if( max_grade >= GRADE_AA )
				SCREENMAN->SetNewScreen( "ScreenCredits" );
			else
				SCREENMAN->SetNewScreen( "ScreenMusicScroll" );
		}
		break;
	}
}

void ScreenNameEntryTraditional::Finish( PlayerNumber pn )
{
	if( !m_bStillEnteringName[pn] )
		return;
	m_bStillEnteringName[pn] = false;

	CString selection = WStringToCString( m_sSelection[pn] );
	TrimRight( selection, " " );
	TrimLeft( selection, " " );

	if( selection == "" )
		selection = DEFAULT_RANKING_NAME;

	GAMESTATE->StoreRankingName( pn, selection );

	if( !AnyStillEntering() && !m_Menu.m_Out.IsTransitioning() )
		m_Menu.StartTransitioning( SM_GoToNextScreen );
}

void ScreenNameEntryTraditional::UpdateSelectionText( int pn )
{
	wstring text = m_sSelection[pn];
	if( (int) text.size() < MAX_RANKING_NAME_LENGTH )
		text += L"_";

	m_textSelection[pn].SetText( WStringToCString(text) );
}

void ScreenNameEntryTraditional::MenuStart( PlayerNumber pn, const InputEventType type )
{
	if( m_Menu.IsTransitioning() )
		return;	
	if( type == IET_RELEASE )
		return;		// ignore

	const int CurrentSelection = m_SelectedChar[pn];
	const int SelectedLetter = m_AlphabetLetter[pn][CurrentSelection];
	switch( SelectedLetter )
	{
	case CHAR_OK:
		m_soundKey.Play();
		Finish( pn );

		break;
	default:
		/* If we have room, add a new character. */
		if( (int) m_sSelection[pn].size() == MAX_RANKING_NAME_LENGTH )
		{
			/* XXX play invalid sound */
			break;
		}
		m_sSelection[pn] += wchar_t(SelectedLetter);
		UpdateSelectionText( pn );
		m_soundKey.Play();

		/* If that filled the string, set the cursor on OK. */
		if( (int) m_sSelection[pn].size() == MAX_RANKING_NAME_LENGTH )
		{
			m_SelectedChar[pn] = 0;
			while( m_AlphabetLetter[pn][m_SelectedChar[pn]] != CHAR_OK )
				++m_SelectedChar[pn];
			ASSERT( m_AlphabetLetter[pn][m_SelectedChar[pn]] == CHAR_OK );
			PositionCharsAndCursor( pn );
		}
	}
}

void ScreenNameEntryTraditional::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	if( m_Menu.IsTransitioning() )
		return;	
	if( type == IET_RELEASE )
		return;		// ignore

	--m_SelectedChar[pn];
	wrap( m_SelectedChar[pn], m_textAlphabet[pn].size() );
	PositionCharsAndCursor( pn );
}

void ScreenNameEntryTraditional::MenuRight( PlayerNumber pn, const InputEventType type )
{
	if( m_Menu.IsTransitioning() )
		return;	
	if( type == IET_RELEASE )
		return;		// ignore

	++m_SelectedChar[pn];
	wrap( m_SelectedChar[pn], m_textAlphabet[pn].size() );
	PositionCharsAndCursor( pn );
}
