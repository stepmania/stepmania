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
#include "FontCharAliases.h"
#include "AnnouncerManager.h"
#include "song.h"
#include "Steps.h"
#include <math.h>
#include "ProfileManager.h"
#include "StageStats.h"


//
// Defines specific to ScreenNameEntryTraditional
//
#define ALPHABET_GAP_X				THEME->GetMetricF(m_sName,"AlphabetGapX")
#define NUM_ALPHABET_DISPLAYED		THEME->GetMetricI(m_sName,"NumAlphabetDisplayed")
#define MAX_RANKING_NAME_LENGTH		THEME->GetMetricI(m_sName,"MaxRankingNameLength")
#define FEAT_INTERVAL				THEME->GetMetricF(m_sName,"FeatInterval")
#define KEYBOARD_LETTERS					THEME->GetMetric (m_sName,"KeyboardLetters")
#define NEXT_SCREEN					THEME->GetMetric(m_sName,"NextScreen")

#define COMMAND_OPTIONAL( actor, command_name ) \
	if( !actor.GetName().empty() ) \
		COMMAND( actor, command_name );




const ScreenMessage	SM_ChangeDisplayedFeat			= ScreenMessage(SM_User+0);

static const int CHAR_OK = -1;
static const int CHAR_BACK = -2;


ScreenNameEntryTraditional::ScreenNameEntryTraditional( CString sClassName ) : Screen( sClassName )
{
	LOG->Trace( "ScreenNameEntryTraditional::ScreenNameEntryTraditional()" );


	if( PREFSMAN->m_bScreenTestMode )
	{
		GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;
		GAMESTATE->m_bSideIsJoined[PLAYER_2] = true;
		GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
		GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;
		GAMESTATE->m_CurStyle = STYLE_DANCE_VERSUS;
		StageStats st;
		for( int z = 0; z < 3; ++z )
		{
			st.pSong = SONGMAN->GetRandomSong();
			ASSERT( st.pSong );
			ASSERT( st.pSong->m_apNotes.size() );
			for( int i = 0; i < 2; ++i )
			{
				GAMESTATE->m_pCurNotes[i] = st.pSteps[i] = st.pSong->m_apNotes[0];
				st.iPossibleDancePoints[i] = 1000;
				st.iActualDancePoints[i] = 985;

				HighScore hs;
				hs.grade = GRADE_TIER_3;
				hs.iScore = 42;
				int a, b;
				PROFILEMAN->AddStepsHighScore( GAMESTATE->m_pCurNotes[i], (PlayerNumber)i, hs, a, b );

				if( i == 0 )
				{
					HighScore hs;
					hs.iScore = 1234567;
					StepsType nt = GAMESTATE->GetCurrentStyleDef()->m_StepsType;
					PROFILEMAN->AddCategoryHighScore( nt, RANKING_A, (PlayerNumber)i, hs, a, b );
				}
			}

			g_vPlayedStageStats.push_back( st );
		}

	}

	int p;

	vector<GameState::RankingFeats> aFeats[NUM_PLAYERS];

	// Find out if players deserve to enter their name
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		GAMESTATE->GetRankingFeats( (PlayerNumber)p, aFeats[p] );
		m_bStillEnteringName[p] = aFeats[p].size()>0;
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
		m_NumFeats[p] = aFeats[p].size();
		m_CurFeat[p] = 0;

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
		const wstring Chars = CStringToWstring(KEYBOARD_LETTERS);
		for( unsigned ch = 0; ch < Chars.size(); ++ch )
		{
			BitmapText *Letter = new BitmapText;
			Letter->SetName( ssprintf("LetterP%i",p+1) );
			Letter->LoadFromFont( fontpath );
			Letter->SetText( ssprintf("%lc", Chars[ch]) );
			m_textAlphabet[p].push_back( Letter );
			m_Keyboard[p].AddChild( Letter );
			Letter->Command( THEME->GetMetric("ScreenNameEntryTraditional","AlphabetInitCommand") );

			m_AlphabetLetter[p].push_back( Chars[ch] );
		}

		/* Add "<-". */
		{
			BitmapText *Letter = new BitmapText;
			Letter->SetName( ssprintf("LetterP%i",p+1) );
			Letter->LoadFromFont( fontpath );
			CString text = "&leftarrow;";
			FontCharAliases::ReplaceMarkers( text );
			Letter->SetText( text );
			m_textAlphabet[p].push_back( Letter );
			m_Keyboard[p].AddChild( Letter );

			m_AlphabetLetter[p].push_back( CHAR_BACK );
			Letter->Command( THEME->GetMetric("ScreenNameEntryTraditional","OKInitCommand") );
		}

		/* Add "OK". */
		{
			BitmapText *Letter = new BitmapText;
			Letter->SetName( ssprintf("LetterP%i",p+1) );
			Letter->LoadFromFont( fontpath );
			CString text = "&ok;";
			FontCharAliases::ReplaceMarkers( text );
			Letter->SetText( text );
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
		unsigned i;
		for( i = 0; i < m_textAlphabet[p].size(); ++i )
			m_textAlphabet[p][i]->FinishTweening();

		/* Show feat 0, hide others without tweening.  Run the ON command for
		 * all actors, even if we're going to hide it anyway, so any style commands
		 * are run. */
#define SET_ON( actor ) \
	SET_XY_AND_ON_COMMAND( actor ); \
	if( i != 0 ) \
	{ \
		COMMAND( actor, "Hide" ); \
		actor.FinishTweening(); \
	}

		for( i = 0; i < aFeats[p].size(); ++i )
		{
			const GameState::RankingFeats &feat = aFeats[p][i];
			if( feat.Banner != "" )
			{
				m_sprBanner[p][i].SetName( ssprintf("BannerP%i",p+1) );
				m_sprBanner[p][i].Load( feat.Banner );
				SET_ON( m_sprBanner[p][i] );
				this->AddChild( &m_sprBanner[p][i] );
			}

			if( feat.grade != GRADE_NO_DATA )
			{
				m_Grade[p][i].SetName( ssprintf("GradeP%i",p+1) );
				m_Grade[p][i].Load( THEME->GetPathToG("ScreenNameEntryTraditional grades") );
				m_Grade[p][i].SetGrade( (PlayerNumber)p, feat.grade );
				SET_ON( m_Grade[p][i] );
				this->AddChild( &m_Grade[p][i] );
			}

			m_textScore[p][i].SetName( ssprintf("ScoreP%i",p+1) );
			m_textScore[p][i].LoadFromNumbers( THEME->GetPathToN("ScreenNameEntryTraditional score") );
			if( PREFSMAN->m_bPercentageScoring )
				m_textScore[p][i].SetText( ssprintf("%.2f%%", feat.fPercentDP*100) );
			else
				m_textScore[p][i].SetText( ssprintf("%i", feat.iScore) );
			SET_ON( m_textScore[p][i] );
			this->AddChild( &m_textScore[p][i] );

			if( feat.Feat != "" )
			{
				m_textCategory[p][i].SetName( ssprintf("CategoryP%i", p+1) );
				m_textCategory[p][i].LoadFromFont( THEME->GetPathToF("ScreenNameEntryTraditional category") );
				m_textCategory[p][i].SetText( feat.Feat );
				SET_ON( m_textCategory[p][i] );
				this->AddChild( &m_textCategory[p][i] );
			}
		}

#undef SET_ON

		/* We always show the banner frame (if any), because fading from a graphic to
		 * itself is ugly. */
		m_sprBannerFrame[p].SetName( ssprintf("BannerFrameP%i",p+1) );
		m_sprBannerFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenNameEntryTraditional banner frame p%i",p+1)) );
		SET_XY_AND_ON_COMMAND( m_sprBannerFrame[p] );
		this->AddChild( &m_sprBannerFrame[p] );
	}

	this->PostScreenMessage( SM_ChangeDisplayedFeat, FEAT_INTERVAL );

	m_soundKey.Load( THEME->GetPathToS("ScreenNameEntryTraditional key") );
	m_soundChange.Load( THEME->GetPathToS("ScreenNameEntryTraditional change",true) );

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

		bt->Command("stoptweening;decelerate,.12");
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
	if( m_bFirstUpdate )
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("name entry") );

	Screen::Update(fDelta);
}

void ScreenNameEntryTraditional::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenNameEntryTraditional::ChangeDisplayedFeat()
{
	for( int pn=0; pn<NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsHumanPlayer(pn) )
			continue;
		if( m_NumFeats[pn] < 2 )
			continue;

		int NewFeat = (m_CurFeat[pn]+1) % m_NumFeats[pn];
		int OldFeat = m_CurFeat[pn];
		m_CurFeat[pn] = NewFeat;

		COMMAND_OPTIONAL( m_Grade[pn][OldFeat], "Hide" );
		COMMAND_OPTIONAL( m_Grade[pn][NewFeat], "Unhide" );
		COMMAND_OPTIONAL( m_sprBanner[pn][OldFeat], "Hide" );
		COMMAND_OPTIONAL( m_sprBanner[pn][NewFeat], "Unhide" );
		COMMAND_OPTIONAL( m_textScore[pn][OldFeat], "Hide" );
		COMMAND_OPTIONAL( m_textScore[pn][NewFeat], "Unhide" );
		COMMAND_OPTIONAL( m_textCategory[pn][OldFeat], "Hide" );
		COMMAND_OPTIONAL( m_textCategory[pn][NewFeat], "Unhide" );
	}
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
	case SM_ChangeDisplayedFeat:
		ChangeDisplayedFeat();
		this->PostScreenMessage( SM_ChangeDisplayedFeat, FEAT_INTERVAL );
		break;

	case SM_GoToNextScreen:
		{
			/* Hack: go back to the select course screen in event mode. */
			if( PREFSMAN->m_bEventMode && GAMESTATE->IsCourseMode() )
			{
				SCREENMAN->SetNewScreen( "ScreenSelectCourse" );
				break;
			}

			Grade max_grade = GRADE_FAILED;
			vector<Song*> vSongs;
			StageStats stats;
			GAMESTATE->GetFinalEvalStatsAndSongs( stats, vSongs );

			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsHumanPlayer(p) )
					max_grade = min( max_grade, stats.GetGrade((PlayerNumber)p) );

			SCREENMAN->SetNewScreen( NEXT_SCREEN );
		}
		break;
	}
}

void ScreenNameEntryTraditional::Finish( PlayerNumber pn )
{
	if( !m_bStillEnteringName[pn] )
		return;
	m_bStillEnteringName[pn] = false;

	UpdateSelectionText( pn ); /* hide NAME_ cursor */

	CString selection = WStringToCString( m_sSelection[pn] );
	TrimRight( selection, " " );
	TrimLeft( selection, " " );

	if( selection == "" )
		selection = DEFAULT_RANKING_NAME;

	GAMESTATE->StoreRankingName( pn, selection );

	// save last used ranking name
	Profile* pProfile = PROFILEMAN->GetProfile(pn);
	if( pProfile )
		pProfile->m_sLastUsedHighScoreName = selection;

	OFF_COMMAND( m_Keyboard[pn] );
	for( int i = 0; i < (int)m_textAlphabet[pn].size(); ++i )
		OFF_COMMAND( m_textAlphabet[pn][i] );
	OFF_COMMAND( m_sprCursor[pn] );

	if( !AnyStillEntering() && !m_Menu.m_Out.IsTransitioning() )
		m_Menu.StartTransitioning( SM_GoToNextScreen );
}

void ScreenNameEntryTraditional::UpdateSelectionText( int pn )
{
	wstring text = m_sSelection[pn];
	if( m_bStillEnteringName[pn] && (int) text.size() < MAX_RANKING_NAME_LENGTH )
		text += L"_";

	m_textSelection[pn].SetText( WStringToCString(text) );
}

void ScreenNameEntryTraditional::MenuStart( PlayerNumber pn, const InputEventType type )
{
	if( !m_bStillEnteringName[pn] || m_Menu.IsTransitioning()  )
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

	case CHAR_BACK:
		if( !m_sSelection[pn].size()  )
		{
			/* XXX play invalid sound */
			break;
		}

		m_sSelection[pn].erase( m_sSelection[pn].size()-1, 1 );
		UpdateSelectionText( pn );
		m_soundKey.Play();

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
	if( !m_bStillEnteringName[pn] || m_Menu.IsTransitioning()  )
		return;
	if( type == IET_RELEASE )
		return;		// ignore

	--m_SelectedChar[pn];
	wrap( m_SelectedChar[pn], m_textAlphabet[pn].size() );
	PositionCharsAndCursor( pn );
	m_soundChange.Play();
}

void ScreenNameEntryTraditional::MenuRight( PlayerNumber pn, const InputEventType type )
{
	if( !m_bStillEnteringName[pn] || m_Menu.IsTransitioning()  )
		return;
	if( type == IET_RELEASE )
		return;		// ignore

	++m_SelectedChar[pn];
	wrap( m_SelectedChar[pn], m_textAlphabet[pn].size() );
	PositionCharsAndCursor( pn );
	m_soundChange.Play();
}
