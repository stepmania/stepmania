#include "global.h"
#include "ScreenNameEntryTraditional.h"
#include "SongManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "Course.h"
#include "ActorUtil.h"
#include "FontCharAliases.h"
#include "AnnouncerManager.h"
#include "song.h"
#include "Steps.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "StatsManager.h"
#include "Foreach.h"
#include "Style.h"
#include "ScreenDimensions.h"
#include "Command.h"
#include "InputEventPlus.h"

//
// Defines specific to ScreenNameEntryTraditional
//
static const ThemeMetric<apActorCommands>	ALPHABET_INIT_COMMMAND	("ScreenNameEntryTraditional","AlphabetInitCommand");	// TODO: remove hard coded name
static const ThemeMetric<apActorCommands>	OK_INIT_COMMMAND		("ScreenNameEntryTraditional","OKInitCommand");	// TODO: remove hard coded name

#define COMMAND_OPTIONAL( actor, command_name ) \
	if( !(actor).GetName().empty() ) \
		COMMAND( (actor), command_name );

AutoScreenMessage( SM_ChangeDisplayedFeat )

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
		m_textScore.SetText( PercentageDisplay::FormatPercentScore(hs.GetPercentDP()) );
	else
		m_textScore.SetText( ssprintf("%i", hs.GetScore()) );
	m_textScore.SetShadowLength( 2 );
	this->AddChild( &m_textScore );
	SET_XY_AND_ON_COMMAND( m_textScore );

	m_textDate.SetName( "Date" );
	m_textDate.LoadFromFont( THEME->GetPathF(m_sName,"date") );
	m_textDate.SetText( ssprintf("%02d/%02d", hs.GetDateTime().tm_mon+1, hs.GetDateTime().tm_mday) );
	m_textDate.SetShadowLength( 2 );
	this->AddChild( &m_textDate );
	SET_XY_AND_ON_COMMAND( m_textDate );
}

void HighScoreWheelItem::LoadBlank( int iRankIndex )
{
	HighScore hs;
	Load( iRankIndex, hs );
}

void HighScoreWheelItem::ShowFocus()
{
	ActorCommands c( "diffuseshift;EffectColor1,1,1,0,1;EffectColor2,0,1,1,1" );
	m_textRank.RunCommands( c );
	m_textName.RunCommands( c );
	m_textScore.RunCommands( c );
	m_textDate.RunCommands( c );
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

	CString sTransformFunction = 
		"function(self,offset,itemIndex,numItems) "
		"	local degrees=18*offset; "
		"	local radians=degrees*math.pi/180; "
		"	self:rotationx(degrees); "
		"	self:y(math.sin(radians)*90); "
		"	self:z(math.cos(radians)*90); "
		"end";
	ActorScroller::Load2( 
		10.5,
		false );
	ActorScroller::SetTransformFromExpression( sTransformFunction );
	ActorScroller::SetSecondsPerItem( 0.2f );
	Scroll();
}

float HighScoreWheel::Scroll()
{
	SetCurrentAndDestinationItem( m_SubActors.size()+5.0f );
	int iIndexToFocus = max( m_iIndexToFocus, 3 );
	SetDestinationItem( (float)iIndexToFocus );
	return GetTweenTimeLeft();
}

REGISTER_SCREEN_CLASS_NEW( ScreenNameEntryTraditional );
ScreenNameEntryTraditional::ScreenNameEntryTraditional():
	CHANGE_COMMAND("stoptweening;decelerate,.12")
{
	if( PREFSMAN->m_bScreenTestMode )
	{
		GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;
		GAMESTATE->m_bSideIsJoined[PLAYER_2] = true;
		GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
		GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );
		GAMESTATE->m_pCurStyle.Set( GAMEMAN->GameAndStringToStyle( GAMEMAN->GetDefaultGame(),"versus") );
		StageStats ss;
		for( int z = 0; z < 3; ++z )
		{
			ss.vpPlayedSongs.push_back( SONGMAN->GetRandomSong() );
			ss.vpPossibleSongs = ss.vpPlayedSongs;
			ss.pStyle = GAMESTATE->m_pCurStyle;
			ss.playMode = GAMESTATE->m_PlayMode;
			ASSERT( ss.vpPlayedSongs[0]->GetAllSteps().size() );
			StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;

			FOREACH_PlayerNumber( p )
			{
				Steps *pSteps = ss.vpPlayedSongs[0]->GetAllSteps()[0];
				ss.m_player[p].vpPlayedSteps.push_back( pSteps );
				GAMESTATE->m_pCurSteps[p].Set( pSteps );
				ss.m_player[p].iPossibleDancePoints = 100;
				ss.m_player[p].iActualDancePoints = 100;
				ss.m_player[p].iScore = 100;
				ss.m_player[p].iPossibleDancePoints = 1000;
				ss.m_player[p].iActualDancePoints = 985;
				ss.m_player[p].vpPossibleSteps.push_back( pSteps );

				HighScore hs;
				hs.SetGrade( Grade_Tier03 );
				hs.SetPercentDP( ss.m_player[p].GetPercentDancePoints() );
				hs.SetScore( ss.m_player[p].iScore );
				hs.SetDateTime( DateTime::GetNowDateTime() );
				int a, b;
				PROFILEMAN->AddStepsScore( ss.vpPlayedSongs[0], pSteps, p, hs, a, b );
				PROFILEMAN->AddStepsScore( ss.vpPlayedSongs[0], pSteps, p, hs, a, b );
				PROFILEMAN->AddStepsScore( ss.vpPlayedSongs[0], pSteps, p, hs, a, b );
				PROFILEMAN->AddStepsScore( ss.vpPlayedSongs[0], pSteps, p, hs, a, b );
				PROFILEMAN->AddStepsScore( ss.vpPlayedSongs[0], pSteps, p, hs, a, b );
				PROFILEMAN->AddCategoryScore( st, RANKING_A, p, hs, a, b );
			}

			STATSMAN->m_vPlayedStageStats.push_back( ss );
		}

	}
}

void ScreenNameEntryTraditional::Init()
{
	ALPHABET_GAP_X.Load( m_sName, "AlphabetGapX" );
	NUM_ALPHABET_DISPLAYED.Load( m_sName, "NumAlphabetDisplayed" );
	MAX_RANKING_NAME_LENGTH.Load( m_sName, "MaxRankingNameLength" );
	FEAT_INTERVAL.Load( m_sName, "FeatInterval" );
	KEYBOARD_LETTERS.Load( m_sName, "KeyboardLetters" );

	ScreenWithMenuElements::Init();

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
				m_sprOutOfRanking[p].Load( THEME->GetPathG( m_sName,ssprintf("OutOfRankingP%i",p+1)) );
				m_sprOutOfRanking[p]->SetName( ssprintf("OutOfRankingP%i",p+1) );
				SET_XY_AND_ON_COMMAND( m_sprOutOfRanking[p] );
				this->AddChild( m_sprOutOfRanking[p] );

				continue;	// skip
			}

			m_sprNameFrame[p].SetName( ssprintf("EntryFrameP%i",p+1) );
			m_sprNameFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("name frame p%i",p+1)) );
			SET_XY_AND_ON_COMMAND( m_sprNameFrame[p] );
			this->AddChild( &m_sprNameFrame[p] );

			m_Keyboard[p].SetName( ssprintf("KeyboardP%i",p+1) );
			SET_XY_AND_ON_COMMAND( m_Keyboard[p] );
			this->AddChild( &m_Keyboard[p] );

			/* Add letters to m_Keyboard. */
			const CString fontpath = THEME->GetPathF(m_sName,"letters");
			const wstring Chars = RStringToWstring(KEYBOARD_LETTERS);
			for( unsigned ch = 0; ch < Chars.size(); ++ch )
			{
				BitmapText *Letter = new BitmapText;
				Letter->SetName( ssprintf("LetterP%i",p+1) );
				Letter->LoadFromFont( fontpath );
				Letter->SetText( ssprintf("%lc", Chars[ch]) );
				m_textAlphabet[p].push_back( Letter );
				m_Keyboard[p].AddChild( Letter );
				Letter->RunCommands( ALPHABET_INIT_COMMMAND );

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
				Letter->RunCommands( OK_INIT_COMMMAND );
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
				Letter->RunCommands( OK_INIT_COMMMAND );
			}

			m_sprCursor[p].SetName( ssprintf("CursorP%i",p+1) );
			m_sprCursor[p].Load( THEME->GetPathG(m_sName,ssprintf("cursor p%i",p+1)) );
			m_Keyboard[p].AddChild( &m_sprCursor[p] );

			m_textSelection[p].SetName( ssprintf("SelectionP%i",p+1) );
			m_textSelection[p].LoadFromFont( THEME->GetPathF(m_sName,"entry") );
			SET_XY_AND_ON_COMMAND( m_textSelection[p] );
			this->AddChild( &m_textSelection[p] );

			m_SelectedChar[p] = 0;
			PositionCharsAndCursor( p );

			// load last used ranking name if any
			const Profile* pProfile = PROFILEMAN->GetProfile(p);
			if( pProfile && !pProfile->m_sLastUsedHighScoreName.empty() )
			{
				m_sSelection[p] = RStringToWstring( pProfile->m_sLastUsedHighScoreName );
				if( (int) m_sSelection[p].size() > MAX_RANKING_NAME_LENGTH )
					m_sSelection[p].erase( MAX_RANKING_NAME_LENGTH );
				ASSERT( (int) m_sSelection[p].size() <= MAX_RANKING_NAME_LENGTH );
				if( m_sSelection[p].size() )
					SelectChar(  p, CHAR_OK );
			}

			UpdateSelectionText( p );

			/* Don't tween to the initial position. */
			for( unsigned i = 0; i < m_textAlphabet[p].size(); ++i )
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
		(actor).FinishTweening(); \
		COMMAND( actor, "Hide" ); \
		(actor).FinishTweening(); \
	}

		m_FeatDisplay[p].reserve( STATSMAN->m_vPlayedStageStats.size() );

		for( unsigned i = 0; i < STATSMAN->m_vPlayedStageStats.size(); ++i )
		{
			StageStats &ss = STATSMAN->m_vPlayedStageStats[i];
			Song* pSong = ss.vpPlayedSongs[0];
			Steps* pSteps = ss.m_player[p].vpPlayedSteps[0];
			Course* pCourse = GAMESTATE->m_pCurCourse;
			Trail* pTrail = GAMESTATE->m_pCurTrail[p];

			int iHighScoreIndex = -1;	// -1 means "out of ranking"
			Grade grade = ss.m_player[p].GetGrade();
			int iScore = ss.m_player[p].iScore;
			float fPercentDP = ss.m_player[p].GetPercentDancePoints();

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

			for( int h=0; h<(int)hsl.vHighScores.size() && h<PREFSMAN->m_iMaxHighScoresPerListForMachine; h++ )
			{
				const HighScore &hs = hsl.vHighScores[h];
				if( hs.GetName() == RANKING_TO_FILL_IN_MARKER[p]  &&
					hs.GetPercentDP() == fPercentDP  && 
					hs.GetScore() == iScore )
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

			if( grade != Grade_NoData )
			{
				display.m_Grade.SetName( ssprintf("GradeP%i",p+1) );
				display.m_Grade.Load( THEME->GetPathG(m_sName,"grades") );
				display.m_Grade.SetGrade( p, grade );
				SET_ON( display.m_Grade );
				this->AddChild( &display.m_Grade );
			}

			display.m_DifficultyIcon.Load( THEME->GetPathG(m_sName,"DifficultyIcon") );
			if( GAMESTATE->IsCourseMode() )
				display.m_DifficultyIcon.SetFromTrail( p, pTrail );
			else
				display.m_DifficultyIcon.SetFromSteps( p, pSteps );
			display.m_DifficultyIcon.SetName( ssprintf("DifficultyIconP%i",p+1) );
			SET_ON( display.m_DifficultyIcon );
			this->AddChild( &display.m_DifficultyIcon );

			display.m_DifficultyMeter.Load( m_sName+ssprintf(" DifficultyMeterP%d",p+1) );
			if( GAMESTATE->IsCourseMode() )
				display.m_DifficultyMeter.SetFromTrail( pTrail );
			else
				display.m_DifficultyMeter.SetFromSteps( pSteps );
			display.m_DifficultyMeter.SetName( ssprintf("DifficultyMeterP%i",p+1) );
			SET_ON( display.m_DifficultyMeter );
			this->AddChild( &display.m_DifficultyMeter );

			display.m_textScore.Load( GAMESTATE->m_pPlayerState[p], &ss.m_player[p], "ScreenNameEntryTraditional Percent", false );
			display.m_textScore.SetName( ssprintf("ScoreP%i",p+1) );
			SET_ON( display.m_textScore );
			this->AddChild( &display.m_textScore );

//			if( feat.Feat != "" )
//			{
//				display.m_textCategory.SetName( ssprintf("CategoryP%i", p+1) );
//				display.m_textCategory.LoadFromFont( THEME->GetPathF(m_sName, "category") );
//				display.m_textCategory.SetText( feat.Feat );
//				SET_ON( display.m_textCategory );
//				this->AddChild( &display.m_textCategory );
//			}

			/* We always show the banner frame (if any), because fading from a graphic to
			 * itself is ugly. */
			display.m_sprBannerFrame.SetName( ssprintf("BannerFrameP%i",p+1) );
			display.m_sprBannerFrame.Load( THEME->GetPathG(m_sName,ssprintf("banner frame p%i",p+1)) );
			SET_XY_AND_ON_COMMAND( display.m_sprBannerFrame );
			this->AddChild( &display.m_sprBannerFrame );
		}
#undef SET_ON
	}

	this->PostScreenMessage( SM_ChangeDisplayedFeat, FEAT_INTERVAL );

	m_soundKey.Load( THEME->GetPathS(m_sName,"key") );
	m_soundChange.Load( THEME->GetPathS(m_sName,"change",true) );
	m_soundInvalid.Load( THEME->GetPathS(m_sName,"invalid",true) );

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

		bt->RunCommands( CHANGE_COMMAND );
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

	ScreenWithMenuElements::Update(fDelta);
}

void ScreenNameEntryTraditional::Input( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	ScreenWithMenuElements::Input( input );
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
		COMMAND_OPTIONAL( m_FeatDisplay[pn][OldFeat].m_DifficultyIcon, "Hide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][NewFeat].m_DifficultyIcon, "Unhide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][OldFeat].m_DifficultyMeter, "Hide" );
		COMMAND_OPTIONAL( m_FeatDisplay[pn][NewFeat].m_DifficultyMeter, "Unhide" );
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

	if( SM == SM_MenuTimer )
	{
		if( !m_Out.IsTransitioning() )
		{
			FOREACH_PlayerNumber( p )
				Finish( p );
			HandleStart( PLAYER_INVALID );
		}
	}
	else if( SM == SM_ChangeDisplayedFeat )
	{
		ChangeDisplayedFeat();
		this->PostScreenMessage( SM_ChangeDisplayedFeat, FEAT_INTERVAL );
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

void ScreenNameEntryTraditional::Finish( PlayerNumber pn )
{
	if( !m_bStillEnteringName[pn] )
		return;
	m_bStillEnteringName[pn] = false;

	UpdateSelectionText( pn ); /* hide NAME_ cursor */

	CString selection = WStringToRString( m_sSelection[pn] );

	// save last used ranking name
	Profile* pProfile = PROFILEMAN->GetProfile(pn);
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

	m_textSelection[pn].SetText( WStringToRString(text) );
}

void ScreenNameEntryTraditional::MenuStart( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS )
		return;		// ignore

	HandleStart( input.MenuI.player );
}

void ScreenNameEntryTraditional::HandleStart( PlayerNumber pn )
{
	/* The screen may have started out with nobody entering, in which case we're
	 * just showing scores and the first Start press moves on. */
	if( !AnyStillEntering() )
	{
		AllFinished();
		return;
	}

	if( !m_bStillEnteringName[pn] )
		return;	// ignore

	const int CurrentSelection = m_SelectedChar[pn];
	const int SelectedLetter = m_AlphabetLetter[pn][CurrentSelection];
	switch( SelectedLetter )
	{
	case CHAR_OK:
		m_soundKey.Play();
		Finish( pn );
		break;

	case CHAR_BACK:
		Backspace( pn );
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

void ScreenNameEntryTraditional::MenuSelect( const InputEventPlus &input )
{
	PlayerNumber pn = input.MenuI.player;

	if( !m_bStillEnteringName[pn] )
		return;	// ignore
	if( input.type != IET_FIRST_PRESS )
		return;		// ignore

	Backspace( pn );
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

void ScreenNameEntryTraditional::Backspace( PlayerNumber pn )
{
	if( !m_sSelection[pn].size()  )
	{
		m_soundInvalid.Play();
		return;
	}

	m_sSelection[pn].erase( m_sSelection[pn].size()-1, 1 );
	UpdateSelectionText( pn );
	m_soundKey.Play();
}

void ScreenNameEntryTraditional::MenuLeft( const InputEventPlus &input )
{
	PlayerNumber pn = input.MenuI.player;
	if( !m_bStillEnteringName[pn] || IsTransitioning()  )
		return;

	--m_SelectedChar[pn];
	wrap( m_SelectedChar[pn], m_textAlphabet[pn].size() );
	PositionCharsAndCursor( pn );
	m_soundChange.Play();
}

void ScreenNameEntryTraditional::MenuRight( const InputEventPlus &input )
{
	PlayerNumber pn = input.MenuI.player;
	if( !m_bStillEnteringName[pn] || IsTransitioning()  )
		return;

	++m_SelectedChar[pn];
	wrap( m_SelectedChar[pn], m_textAlphabet[pn].size() );
	PositionCharsAndCursor( pn );
	m_soundChange.Play();
}

void ScreenNameEntryTraditional::AllFinished()
{
	StartTransitioningScreen( SM_GoToNextScreen );

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
