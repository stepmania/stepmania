#include "global.h"
#include "ScreenNameEntryTraditional.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "ScreenRanking.h"
#include "Course.h"
#include "ActorUtil.h"
#include "FontCharAliases.h"
#include "AnnouncerManager.h"
#include "song.h"
#include "Steps.h"
#include "ProfileManager.h"
#include "StageStats.h"
#include "RageDisplay.h"
#include "Foreach.h"
#include "Style.h"


//
// Defines specific to ScreenNameEntryTraditional
//
#define FOV							THEME->GetMetricF(m_sName,"FOV")
#define ALPHABET_GAP_X				THEME->GetMetricF(m_sName,"AlphabetGapX")
#define NUM_ALPHABET_DISPLAYED		THEME->GetMetricI(m_sName,"NumAlphabetDisplayed")
#define MAX_RANKING_NAME_LENGTH		THEME->GetMetricI(m_sName,"MaxRankingNameLength")
#define FEAT_INTERVAL				THEME->GetMetricF(m_sName,"FeatInterval")
#define KEYBOARD_LETTERS			THEME->GetMetric (m_sName,"KeyboardLetters")
#define NEXT_SCREEN					THEME->GetMetric(m_sName,"NextScreen")

#define COMMAND_OPTIONAL( actor, command_name ) \
	if( !actor.GetName().empty() ) \
		COMMAND( actor, command_name );

const ScreenMessage	SM_ChangeDisplayedFeat			= ScreenMessage(SM_User+0);

static const int CHAR_OK = -1;
static const int CHAR_BACK = -2;


void HighScoreWheelItem::Load( int iRankIndex, const HighScore& hs )
{
	SetName( "HighScoreWheelItem" );
	
	m_textRank.SetName( "Rank" );
	m_textRank.LoadFromFont( THEME->GetPathF(m_sName,"rank") );
	m_textRank.SetText( ssprintf("%d", iRankIndex+1) );
	m_textRank.SetShadowLength( 2 );
	this->AddChild( &m_textRank );
	SET_XY_AND_ON_COMMAND( m_textRank );

	m_textName.SetName( "Name" );
	m_textName.LoadFromFont( THEME->GetPathF(m_sName,"name") );
	m_textName.SetText( hs.GetDisplayName() );
	m_textName.SetShadowLength( 2 );
	this->AddChild( &m_textName );
	SET_XY_AND_ON_COMMAND( m_textName );

	m_textScore.SetName( "Score" );
	m_textScore.LoadFromFont( THEME->GetPathF(m_sName,"score") );
	if( PREFSMAN->m_bPercentageScoring )
		m_textScore.SetText( ssprintf("%.2f%%", hs.fPercentDP*100) );
	else
		m_textScore.SetText( ssprintf("%i", hs.iScore) );
	m_textScore.SetShadowLength( 2 );
	this->AddChild( &m_textScore );
	SET_XY_AND_ON_COMMAND( m_textScore );
}

void HighScoreWheelItem::LoadBlank( int iRankIndex )
{
	HighScore hs;
	Load( iRankIndex, hs );
}

void HighScoreWheelItem::ShowFocus()
{
	CString sCommand = "diffuseshift;EffectColor1,1,1,0,1;EffectColor2,0,1,1,1";
	m_textRank.Command( sCommand );
	m_textName.Command( sCommand );
	m_textScore.Command( sCommand );
}

void HighScoreWheel::Load( const HighScoreList& hsl, int iIndexToFocus )
{
	m_Items.resize( PREFSMAN->m_iMaxHighScoresPerListForMachine );
	for( int i=0; i<PREFSMAN->m_iMaxHighScoresPerListForMachine; i++ )
	{
		if( unsigned(i) < hsl.vHighScores.size() )
			m_Items[i].Load( i, hsl.vHighScores[i] );
		else
			m_Items[i].LoadBlank( i );
		this->AddChild( &m_Items[i] );
	}
	m_iIndexToFocus = iIndexToFocus;

	if( m_iIndexToFocus >= 0  &&  m_iIndexToFocus < int(hsl.vHighScores.size()) )
		m_Items[m_iIndexToFocus].ShowFocus();

	ActorScroller::Load( 
		0.2f,	// fSecondsPerItem
		8,		// iNumItemsToDraw
		RageVector3(18,0,0),	// vRotationDegrees
		RageVector3(0,0,90),	// vTranslateTerm0
		RageVector3(0,0,0),		// vTranslateTerm1
		RageVector3(0,0,0)		// vTranslateTerm2
		);
	Scroll();
}

float HighScoreWheel::Scroll()
{
	SetCurrentAndDestinationItem( m_SubActors.size()+5 );
	int iIndexToFocus = max( m_iIndexToFocus, 3 );
	SetDestinationItem( iIndexToFocus );
	return GetTweenTimeLeft();
}

ScreenNameEntryTraditional::ScreenNameEntryTraditional( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenNameEntryTraditional::ScreenNameEntryTraditional()" );

	if( PREFSMAN->m_bScreenTestMode )
	{
		GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;
		GAMESTATE->m_bSideIsJoined[PLAYER_2] = true;
		GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
		GAMESTATE->m_PlayMode = PLAY_MODE_REGULAR;
		GAMESTATE->m_pCurStyle = GAMEMAN->GameAndStringToStyle( GAMEMAN->GetDefaultGame(), "versus" );
		StageStats ss;
		for( int z = 0; z < 3; ++z )
		{
			ss.vpSongs.push_back( SONGMAN->GetRandomSong() );
			ss.iPossibleDancePoints[PLAYER_1] = 100;
			ss.iActualDancePoints[PLAYER_1] = 100;
			ss.iScore[PLAYER_1] = 100;
			ss.iPossibleDancePoints[PLAYER_2] = 100;
			ss.iActualDancePoints[PLAYER_2] = 100;
			ss.iScore[PLAYER_2] = 100;
			ASSERT( ss.vpSongs[0]->GetAllSteps().size() );
			FOREACH_PlayerNumber( p )
			{
				ss.vpSteps[p].push_back( ss.vpSongs[0]->GetAllSteps()[0] );
				GAMESTATE->m_pCurSteps[p] = ss.vpSteps[p][0];
				ss.iPossibleDancePoints[p] = 1000;
				ss.iActualDancePoints[p] = 985;

				HighScore hs;
				hs.grade = GRADE_TIER_3;
				hs.fPercentDP = ss.GetPercentDancePoints(p);
				hs.iScore = ss.iScore[p];
				StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
				int a, b;
				PROFILEMAN->AddStepsScore( ss.vpSongs[0], GAMESTATE->m_pCurSteps[p], p, hs, a, b );
				PROFILEMAN->AddStepsScore( ss.vpSongs[0], GAMESTATE->m_pCurSteps[p], p, hs, a, b );
				PROFILEMAN->AddStepsScore( ss.vpSongs[0], GAMESTATE->m_pCurSteps[p], p, hs, a, b );
				PROFILEMAN->AddStepsScore( ss.vpSongs[0], GAMESTATE->m_pCurSteps[p], p, hs, a, b );
				PROFILEMAN->AddStepsScore( ss.vpSongs[0], GAMESTATE->m_pCurSteps[p], p, hs, a, b );
				PROFILEMAN->AddCategoryScore( st, RANKING_A, p, hs, a, b );
			}

			g_vPlayedStageStats.push_back( ss );
		}

	}


	// Find out if players deserve to enter their name
	FOREACH_PlayerNumber( p )
	{
		vector<GameState::RankingFeat> aFeats;
		GAMESTATE->GetRankingFeats( p, aFeats );
		m_bStillEnteringName[p] = aFeats.size()>0;
		m_CurFeat[p] = 0;
	}

	//
	// init keyboards
	//
	{
		FOREACH_HumanPlayer( p )
		{
			// don't show keyboard if didn't make any high scores
			if( !m_bStillEnteringName[p] )
			{
				m_sprOutOfRanking[p].Load( THEME->GetPathG( "ScreenNameEntryTraditional",ssprintf("OutOfRankingP%i",p+1)) );
				m_sprOutOfRanking[p]->SetName( ssprintf("OutOfRankingP%i",p+1) );
				SET_XY_AND_ON_COMMAND( m_sprOutOfRanking[p] );
				this->AddChild( m_sprOutOfRanking[p] );

				continue;	// skip
			}

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

			m_SelectedChar[p] = 0;
			PositionCharsAndCursor( p );

			// load last used ranking name if any
			const Profile* pProfile = PROFILEMAN->GetProfile(p);
			if( pProfile && !pProfile->m_sLastUsedHighScoreName.empty() )
			{
				m_sSelection[p] = CStringToWstring( pProfile->m_sLastUsedHighScoreName );
				if( (int) m_sSelection[p].size() > MAX_RANKING_NAME_LENGTH )
					m_sSelection[p].erase( MAX_RANKING_NAME_LENGTH );
				ASSERT( (int) m_sSelection[p].size() <= MAX_RANKING_NAME_LENGTH );
				if( m_sSelection[p].size() )
					SelectChar(  p, CHAR_OK );
			}

			UpdateSelectionText( p );

			/* Don't tween to the initial position. */
			unsigned i;
			for( i = 0; i < m_textAlphabet[p].size(); ++i )
				m_textAlphabet[p][i]->FinishTweening();
		}
	}

	//
	// init feat displays
	//
	FOREACH_HumanPlayer( p )
	{
		/* Show feat 0, hide others without tweening.  Run the ON command for
		 * all actors, even if we're going to hide it anyway, so any style commands
		 * are run. */
#define SET_ON( actor ) \
	SET_XY_AND_ON_COMMAND( actor ); \
	if( m_FeatDisplay[p].size()>1 ) \
	{ \
		COMMAND( actor, "On" ); \
		actor.FinishTweening(); \
		COMMAND( actor, "Hide" ); \
		actor.FinishTweening(); \
	}

		m_FeatDisplay[p].reserve( g_vPlayedStageStats.size() );

		for( unsigned i = 0; i < g_vPlayedStageStats.size(); ++i )
		{
			StageStats &ss = g_vPlayedStageStats[i];
			Song* pSong = ss.vpSongs[0];
			Steps* pSteps = ss.vpSteps[p][0];
			Course* pCourse = GAMESTATE->m_pCurCourse;
			Trail* pTrail = GAMESTATE->m_pCurTrail[p];

			int iHighScoreIndex = -1;	// -1 means "out of ranking"
			Grade grade = ss.GetGrade( p );
			int iScore = ss.iScore[p];
			float fPercentDP = ss.GetPercentDancePoints( p );

			// If this is a SHOW_NEVER song, then it's probably a training.
			// Don't show a high score
			if( pSong->m_SelectionDisplay == Song::SHOW_NEVER )
				continue;	// skip

			m_FeatDisplay[p].resize( m_FeatDisplay[p].size()+1 );
			FeatDisplay &display = m_FeatDisplay[p].back();

			const HighScoreList& hsl = 
				GAMESTATE->IsCourseMode() ?
				PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList(pCourse,pTrail) :
				PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps);

			for( unsigned h=0; h<hsl.vHighScores.size(); h++ )
			{
				const HighScore &hs = hsl.vHighScores[h];
				if( hs.sName == RANKING_TO_FILL_IN_MARKER[p]  &&
					hs.fPercentDP == fPercentDP  && 
					hs.iScore == iScore )
				{
					iHighScoreIndex = h;
					break;
				}
			}

 			display.m_Wheel.SetName( ssprintf("WheelP%i",p+1) );
			display.m_Wheel.Load( hsl, iHighScoreIndex );
			SET_ON( display.m_Wheel );
			this->AddChild( &display.m_Wheel );

			CString sBanner;
			if( GAMESTATE->IsCourseMode() )
				sBanner = pCourse->m_sBannerPath;
			else
				sBanner = pSong->GetBannerPath();
			if( !sBanner.empty() )
			{
				display.m_sprBanner.SetName( ssprintf("BannerP%i",p+1) );
				display.m_sprBanner.Load( sBanner );
				SET_ON( display.m_sprBanner );
				this->AddChild( &display.m_sprBanner );
			}

			if( grade != GRADE_NO_DATA )
			{
				display.m_Grade.SetName( ssprintf("GradeP%i",p+1) );
				display.m_Grade.Load( THEME->GetPathToG("ScreenNameEntryTraditional grades") );
				display.m_Grade.SetGrade( p, grade );
				SET_ON( display.m_Grade );
				this->AddChild( &display.m_Grade );
			}

			display.m_Difficulty.SetName( ssprintf("DifficultyP%i",p+1) );
			display.m_Difficulty.Load( THEME->GetPathToG("ScreenNameEntryTraditional difficulty icons") );
			if( GAMESTATE->IsCourseMode() )
				display.m_Difficulty.SetFromTrail( p, pTrail );
			else
				display.m_Difficulty.SetFromSteps( p, pSteps );
			SET_ON( display.m_Difficulty );
			this->AddChild( &display.m_Difficulty );

			display.m_textScore.SetName( "ScreenNameEntryTraditional Percent" );
			display.m_textScore.Load( p, &ss, false );
			display.m_textScore.SetName( ssprintf("ScoreP%i",p+1) );
			SET_ON( display.m_textScore );
			this->AddChild( &display.m_textScore );

//			if( feat.Feat != "" )
//			{
//				display.m_textCategory.SetName( ssprintf("CategoryP%i", p+1) );
//				display.m_textCategory.LoadFromFont( THEME->GetPathToF("ScreenNameEntryTraditional category") );
//				display.m_textCategory.SetText( feat.Feat );
//				SET_ON( display.m_textCategory );
//				this->AddChild( &display.m_textCategory );
//			}

			/* We always show the banner frame (if any), because fading from a graphic to
			 * itself is ugly. */
			display.m_sprBannerFrame.SetName( ssprintf("BannerFrameP%i",p+1) );
			display.m_sprBannerFrame.Load( THEME->GetPathToG(ssprintf("ScreenNameEntryTraditional banner frame p%i",p+1)) );
			SET_XY_AND_ON_COMMAND( display.m_sprBannerFrame );
			this->AddChild( &display.m_sprBannerFrame );
		}
#undef SET_ON
	}

	this->PostScreenMessage( SM_ChangeDisplayedFeat, FEAT_INTERVAL );

	m_soundKey.Load( THEME->GetPathToS("ScreenNameEntryTraditional key") );
	m_soundChange.Load( THEME->GetPathToS("ScreenNameEntryTraditional change",true) );
	m_soundInvalid.Load( THEME->GetPathToS("ScreenNameEntryTraditional invalid",true) );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenNameEntryTraditional music") );

	this->SortByDrawOrder();
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
	FOREACH_PlayerNumber( p )
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
	DISPLAY->CameraPushMatrix();
	DISPLAY->LoadMenuPerspective( FOV, CENTER_X, CENTER_Y );

	Screen::DrawPrimitives();
	
	DISPLAY->CameraPopMatrix();
}

void ScreenNameEntryTraditional::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( IsTransitioning() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenNameEntryTraditional::ChangeDisplayedFeat()
{
	CHECKPOINT;
	
	LOG->Trace( "ScreenNameEntryTraditional::ChangeDisplayedFeat" );

	FOREACH_HumanPlayer( pn )
	{
		if( m_FeatDisplay[pn].size() < 2 )
			continue;

		int NewFeat = (m_CurFeat[pn]+1) % m_FeatDisplay[pn].size();
		int OldFeat = m_CurFeat[pn];
		m_CurFeat[pn] = NewFeat;


		COMMAND_OPTIONAL( m_FeatDisplay[pn][OldFeat].m_Wheel, "Hide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][NewFeat].m_Wheel, "Unhide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][OldFeat].m_Grade, "Hide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][NewFeat].m_Grade, "Unhide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][OldFeat].m_Difficulty, "Hide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][NewFeat].m_Difficulty, "Unhide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][OldFeat].m_sprBanner, "Hide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][NewFeat].m_sprBanner, "Unhide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][OldFeat].m_textScore, "Hide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][NewFeat].m_textScore, "Unhide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][OldFeat].m_textCategory, "Hide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][NewFeat].m_textCategory, "Unhide" );

		m_FeatDisplay[pn][NewFeat].m_Wheel.Scroll();
	}
}

void ScreenNameEntryTraditional::HandleScreenMessage( const ScreenMessage SM )
{
	LOG->Trace( "ScreenNameEntryTraditional::HandleScreenMessage( %d )", SM );

	switch( SM )
	{
	case SM_MenuTimer:
		if( !m_Out.IsTransitioning() )
		{
			FOREACH_PlayerNumber( p )
				Finish( p );
			MenuStart( PLAYER_INVALID, IET_FIRST_PRESS );
		}
		break;
	case SM_ChangeDisplayedFeat:
		ChangeDisplayedFeat();
		this->PostScreenMessage( SM_ChangeDisplayedFeat, FEAT_INTERVAL );
		break;

	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
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

	// save last used ranking name
	Profile* pProfile = PROFILEMAN->GetProfile(pn);
	if( pProfile )
		pProfile->m_sLastUsedHighScoreName = selection;

	TrimRight( selection, " " );
	TrimLeft( selection, " " );

	GAMESTATE->StoreRankingName( pn, selection );

	OFF_COMMAND( m_Keyboard[pn] );
	for( int i = 0; i < (int)m_textAlphabet[pn].size(); ++i )
		OFF_COMMAND( m_textAlphabet[pn][i] );
	OFF_COMMAND( m_sprCursor[pn] );

	if( !AnyStillEntering() )
		AllFinished();
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
	if( !AnyStillEntering() )
	{
		AllFinished();
		return;
	}

	if( !m_bStillEnteringName[pn] )
		return;	// ignore
	if( type != IET_FIRST_PRESS )
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
			m_soundInvalid.Play();
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
			m_soundInvalid.Play();
			SelectChar( pn, CHAR_BACK );
			break;
		}
		m_sSelection[pn] += wchar_t(SelectedLetter);
		UpdateSelectionText( pn );
		m_soundKey.Play();

		/* If that filled the string, set the cursor on OK. */
		if( (int) m_sSelection[pn].size() == MAX_RANKING_NAME_LENGTH )
			SelectChar( pn, CHAR_OK );
	}
}

void ScreenNameEntryTraditional::SelectChar( PlayerNumber pn, int c )
{
	FOREACH( int, m_AlphabetLetter[pn], letter )
	{
		if( *letter == c )	// character found
		{
			m_SelectedChar[pn] = letter - m_AlphabetLetter[pn].begin();
			PositionCharsAndCursor( pn );
			return;
		}
	}
	ASSERT( false );	// character not found
}

void ScreenNameEntryTraditional::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	if( !m_bStillEnteringName[pn] || IsTransitioning()  )
		return;

	--m_SelectedChar[pn];
	wrap( m_SelectedChar[pn], m_textAlphabet[pn].size() );
	PositionCharsAndCursor( pn );
	m_soundChange.Play();
}

void ScreenNameEntryTraditional::MenuRight( PlayerNumber pn, const InputEventType type )
{
	if( !m_bStillEnteringName[pn] || IsTransitioning()  )
		return;

	++m_SelectedChar[pn];
	wrap( m_SelectedChar[pn], m_textAlphabet[pn].size() );
	PositionCharsAndCursor( pn );
	m_soundChange.Play();
}

void ScreenNameEntryTraditional::AllFinished()
{
	StartTransitioning( SM_GoToNextScreen );

	FOREACH_HumanPlayer( pn )
	{
		OFF_COMMAND( m_sprOutOfRanking[pn] );
		OFF_COMMAND( m_sprNameFrame[pn] );
		OFF_COMMAND( m_textSelection[pn] );
	}
}

/*
 * (c) 2001-2004 Glenn Maynard, Chris Danford
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
