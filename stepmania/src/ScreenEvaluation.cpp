#include "global.h"
#include "ScreenEvaluation.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameManager.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "Steps.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "GameSoundManager.h"
#include "ActorUtil.h"
#include "UnlockManager.h"
#include "Course.h"
#include "LightsManager.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "song.h"
#include "StatsManager.h"
#include "Grade.h"
#include "CodeDetector.h"
#include "RageDisplay.h"
#include "StepMania.h"
#include "CryptManager.h"
#include "Style.h"
#include "MemoryCardManager.h"
#include "PlayerState.h"
#include "Command.h"
#include "CommonMetrics.h"
#include "ScoreKeeperMAX2.h"
#include "InputEventPlus.h"

const int NUM_SCORE_DIGITS	=	9;


// metrics that are common to all ScreenEvaluation classes
#define BANNER_WIDTH						THEME->GetMetricF(m_sName,"BannerWidth")
#define BANNER_HEIGHT						THEME->GetMetricF(m_sName,"BannerHeight")
const char* JUDGE_STRING[NUM_JUDGE_LINES] =
{
	"Marvelous", "Perfect", "Great", "Good", "Boo", "Miss", "OK", "MaxCombo", "TotalError"
};
const char* STATS_STRING[NUM_STATS_LINES] =
{
	"Jumps", "Holds", "Mines", "Hands", "Rolls",
};

#define SPIN_GRADES							THEME->GetMetricB(m_sName,"SpinGrades")
#define CHEER_DELAY_SECONDS					THEME->GetMetricF(m_sName,"CheerDelaySeconds")
#define BAR_ACTUAL_MAX_COMMAND				THEME->GetMetricA(m_sName,"BarActualMaxCommand")

// metrics that are specific to classes derived from ScreenEvaluation
#define SHOW_BANNER_AREA					THEME->GetMetricB(m_sName,"ShowBannerArea")
#define SHOW_GRADE_AREA						THEME->GetMetricB(m_sName,"ShowGradeArea")
#define SHOW_POINTS_AREA					THEME->GetMetricB(m_sName,"ShowPointsArea")
#define SHOW_BONUS_AREA						THEME->GetMetricB(m_sName,"ShowBonusArea")
#define SHOW_SURVIVED_AREA					THEME->GetMetricB(m_sName,"ShowSurvivedArea")
#define SHOW_WIN_AREA						THEME->GetMetricB(m_sName,"ShowWinArea")
#define SHOW_JUDGMENT( l )					THEME->GetMetricB(m_sName,ssprintf("Show%s",JUDGE_STRING[l]))
#define SHOW_STAT( s )						THEME->GetMetricB(m_sName,ssprintf("Show%s",STATS_STRING[l]))
#define SHOW_SCORE_AREA						THEME->GetMetricB(m_sName,"ShowScoreArea")
#define SHOW_TOTAL_SCORE_AREA				THEME->GetMetricB(m_sName,"ShowTotalScoreArea")
#define SHOW_TIME_AREA						THEME->GetMetricB(m_sName,"ShowTimeArea")
#define SHOW_PER_DIFFICULTY_AWARD			THEME->GetMetricB(m_sName,"ShowPerDifficultyAward")
#define SHOW_PEAK_COMBO_AWARD				THEME->GetMetricB(m_sName,"ShowPeakComboAward")
#define TYPE								THEME->GetMetric (m_sName,"Type")
#define PASSED_SOUND_TIME					THEME->GetMetricF(m_sName,"PassedSoundTime")
#define FAILED_SOUND_TIME					THEME->GetMetricF(m_sName,"FailedSoundTime")
#define NUM_SEQUENCE_SOUNDS					THEME->GetMetricI(m_sName,"NumSequenceSounds")
#define MAX_COMBO_NUM_DIGITS				THEME->GetMetricI(m_sName,"MaxComboNumDigits")
#define PLAYER_OPTIONS_SEPARATOR			THEME->GetMetric (m_sName,"PlayerOptionsSeparator")


static const int NUM_SHOWN_RADAR_CATEGORIES = 5;

AutoScreenMessage( SM_PlayCheer )
AutoScreenMessage( SM_AddBonus )


REGISTER_SCREEN_CLASS( ScreenEvaluation );
ScreenEvaluation::ScreenEvaluation( CString sClassName ) : ScreenWithMenuElements(sClassName)
{
	LOG->Trace( "ScreenEvaluation::ScreenEvaluation" );

	//
	// debugging
	//
	
	if( PREFSMAN->m_bScreenTestMode )
	{
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_1);
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_2);

		GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );
		GAMESTATE->m_pCurStyle.Set( GAMEMAN->GameAndStringToStyle(GAMEMAN->GetDefaultGame(),"versus") );
		STATSMAN->m_CurStageStats.playMode = GAMESTATE->m_PlayMode;
		STATSMAN->m_CurStageStats.pStyle = GAMESTATE->m_pCurStyle;
		STATSMAN->m_CurStageStats.StageType = StageStats::STAGE_NORMAL;
		GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
		GAMESTATE->m_pCurSong.Set( SONGMAN->GetRandomSong() );
		STATSMAN->m_CurStageStats.vpPlayedSongs.push_back( GAMESTATE->m_pCurSong );
		STATSMAN->m_CurStageStats.vpPossibleSongs.push_back( GAMESTATE->m_pCurSong );
		GAMESTATE->m_pCurCourse.Set( SONGMAN->GetRandomCourse() );
		GAMESTATE->m_iCurrentStageIndex = 0;
		
		FOREACH_PlayerNumber( p )
		{
			if( rand() % 2 )
				GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.m_bTransforms[PlayerOptions::TRANSFORM_ECHO] = true;	// show "disqualified"

			GAMESTATE->m_bSideIsJoined[p] = true;
			GAMESTATE->m_pCurSteps[p].Set( GAMESTATE->m_pCurSong->GetAllSteps()[0] );
			vector<Trail*> apTrails;
			GAMESTATE->m_pCurCourse->GetAllTrails( apTrails );
			if( apTrails.size() )
				GAMESTATE->m_pCurTrail[p].Set( apTrails[0] );
			STATSMAN->m_CurStageStats.m_player[p].vpPlayedSteps.push_back( GAMESTATE->m_pCurSteps[PLAYER_1] );
			STATSMAN->m_CurStageStats.m_player[p].vpPossibleSteps.push_back( GAMESTATE->m_pCurSteps[PLAYER_1] );
			GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.m_fScrollSpeed = 2;
			GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.ChooseRandomModifiers();
		}

		for( float f = 0; f < 100.0f; f += 1.0f )
		{
			float fP1 = fmodf(f/100*4+.3f,1);
			STATSMAN->m_CurStageStats.m_player[PLAYER_1].SetLifeRecordAt( fP1, f );
			STATSMAN->m_CurStageStats.m_player[PLAYER_2].SetLifeRecordAt( 1-fP1, f );
		}
	
		FOREACH_PlayerNumber( p )
		{
			STATSMAN->m_CurStageStats.m_player[p].iActualDancePoints = rand()%3;
			STATSMAN->m_CurStageStats.m_player[p].iPossibleDancePoints = 2;
			STATSMAN->m_CurStageStats.m_player[p].iCurCombo = 0;
			STATSMAN->m_CurStageStats.m_player[p].UpdateComboList( 0, false );
			STATSMAN->m_CurStageStats.m_player[p].iCurCombo = 1;
			STATSMAN->m_CurStageStats.m_player[p].UpdateComboList( 1, false );
			STATSMAN->m_CurStageStats.m_player[p].iCurCombo = 50;
			STATSMAN->m_CurStageStats.m_player[p].UpdateComboList( 25, false );
			STATSMAN->m_CurStageStats.m_player[p].iCurCombo = 250;
			STATSMAN->m_CurStageStats.m_player[p].UpdateComboList( 100, false );
			if( rand()%2 )
			{
				STATSMAN->m_CurStageStats.m_player[p].iCurCombo = rand()%11000;
				STATSMAN->m_CurStageStats.m_player[p].UpdateComboList( 110, false );
			}
			if( rand()%5 == 0 )
			{
				STATSMAN->m_CurStageStats.m_player[p].bFailedEarlier = true;
			}
			STATSMAN->m_CurStageStats.m_player[p].iTapNoteScores[TNS_MARVELOUS] = rand()%3;
			STATSMAN->m_CurStageStats.m_player[p].iTapNoteScores[TNS_PERFECT] = rand()%3;
			STATSMAN->m_CurStageStats.m_player[p].iTapNoteScores[TNS_GREAT] = rand()%3;
			STATSMAN->m_CurStageStats.m_player[p].iPossibleGradePoints = 4*ScoreKeeperMAX2::TapNoteScoreToGradePoints(TNS_MARVELOUS, false);
			STATSMAN->m_CurStageStats.m_player[p].fLifeRemainingSeconds = randomf( 90, 580 );
		}

		STATSMAN->m_vPlayedStageStats.clear();
	}
}


void ScreenEvaluation::Init()
{
	LOG->Trace( "ScreenEvaluation::Init()" );

	/* Commit stats to the profile, so any profile stats displayed on this screen
	 * include the last game. */
	GAMESTATE->CommitStageStats();

	LIGHTSMAN->SetLightsMode( LIGHTSMODE_MENU );

	m_bFailed = STATSMAN->m_CurStageStats.AllFailed();

	ZERO( m_bSavedScreenshot );


	CString sType = TYPE;
	if( !sType.CompareNoCase("stage") )
		m_Type = stage;
	else if( !sType.CompareNoCase("summary") )
		m_Type = summary;
	else if( !sType.CompareNoCase("course") )
		m_Type = course;
	else
		RageException::Throw("Unknown evaluation type \"%s\"", TYPE.c_str() );
		

	//
	// Figure out which statistics and songs we're going to display
	//
	STATSMAN->CalcAccumStageStats();

	switch( m_Type )
	{
	case summary:
		STATSMAN->GetFinalEvalStageStats( m_StageStats );
		break;
	case stage:
	case course:
		m_StageStats = STATSMAN->m_CurStageStats;
		break;
	default:
		ASSERT(0);
	}

	LOG->Trace( "total error: %i, %i", m_StageStats.m_player[PLAYER_1].iTotalError, m_StageStats.m_player[PLAYER_2].iTotalError );

	// Run this here, so m_StageStats is available to overlays.
	ScreenWithMenuElements::Init();

/*
	//
	// Debugging
	//
	{
		FOREACH_PlayerNumber( p )	// foreach line
			for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )	// foreach line
			{
				m_StageStats.radarPossible[p][r] = 0.5f + r/10.0f;
				m_StageStats.radarActual[p][r] = 0.5f + r/10.0f;
			}
	}
*/

	//
	// Calculate grades
	//
	Grade grade[NUM_PLAYERS];

	FOREACH_PlayerNumber( p )
	{
		if( GAMESTATE->IsPlayerEnabled(p) )
			grade[p] = m_StageStats.m_player[p].GetGrade();
		else
			grade[p] = Grade_Failed;

		if( PREFSMAN->m_ScoringType == PrefsManager::SCORING_5TH )
		{
			const int ScoreBonuses[] = { 10000000, 10000000, 1000000, 100000, 10000, 1000, 100 };
			if( grade[p] < (int) ARRAYSIZE(ScoreBonuses) )
			{
				STATSMAN->m_CurStageStats.m_player[p].iBonus += ScoreBonuses[(int)grade[p] ];
				m_StageStats.m_player[p].iBonus += ScoreBonuses[(int)grade[p] ];
			}
		}
	}


	//
	// update persistent statistics
	//
	RankingCategory rc[NUM_PLAYERS];;
	CommitScores( m_StageStats, m_iPersonalHighScoreIndex, m_iMachineHighScoreIndex, rc, m_pdaToShow, m_pcaToShow );

	m_bTryExtraStage = 
		GAMESTATE->HasEarnedExtraStage()  && 
		m_Type==stage;
 

	//
	// load sounds
	//
	m_soundStart.Load( THEME->GetPathS(m_sName,"start") );

	
	//
	// init banner area
	//
	if( SHOW_BANNER_AREA )
	{
		switch( m_Type )
		{
		case stage:
			{
				m_LargeBanner.LoadFromSong( GAMESTATE->m_pCurSong );
				m_LargeBanner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
				m_LargeBanner.SetName( "LargeBanner" );
				SET_XY_AND_ON_COMMAND( m_LargeBanner );
				this->AddChild( &m_LargeBanner );

				m_sprLargeBannerFrame.Load( THEME->GetPathG(m_sName,"banner frame") );
				m_sprLargeBannerFrame->SetName( "LargeBannerFrame" );
				SET_XY_AND_ON_COMMAND( m_sprLargeBannerFrame );
				this->AddChild( m_sprLargeBannerFrame );
			}
			break;
		case summary:
			{
				for( unsigned i=0; i<m_StageStats.vpPlayedSongs.size(); i++ )
				{
					Song *pSong = m_StageStats.vpPlayedSongs[i];

					m_SmallBanner[i].LoadFromSong( pSong );
					m_SmallBanner[i].ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
					m_SmallBanner[i].SetName( ssprintf("SmallBanner%d",i+1) );
					SET_XY_AND_ON_COMMAND( m_SmallBanner[i] );
					this->AddChild( &m_SmallBanner[i] );

					m_sprSmallBannerFrame[i].Load( THEME->GetPathG(m_sName,"banner frame") );
					m_sprSmallBannerFrame[i]->SetName( ssprintf("SmallBanner%d",i+1) );
					SET_XY_AND_ON_COMMAND( m_sprSmallBannerFrame[i] );
					this->AddChild( m_sprSmallBannerFrame[i] );
				}
			}
			break;
		case course:
			{
				m_LargeBanner.LoadFromCourse( GAMESTATE->m_pCurCourse );
				m_LargeBanner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
				m_LargeBanner.SetName( "LargeBanner" );
				SET_XY_AND_ON_COMMAND( m_LargeBanner );
				this->AddChild( &m_LargeBanner );

				m_sprLargeBannerFrame.Load( THEME->GetPathG(m_sName,"banner frame") );
				m_sprLargeBannerFrame->SetName( "LargeBannerFrame" );
				SET_XY_AND_ON_COMMAND( m_sprLargeBannerFrame );
				this->AddChild( m_sprLargeBannerFrame );
			}
			break;
		default:
			ASSERT(0);
		}
	}

	{
		switch( m_Type )
		{
		case stage:
		case course:
			{
				FOREACH_EnabledPlayer( p )
				{
					m_DifficultyIcon[p].Load( THEME->GetPathG(m_sName,"difficulty icons") );
					switch( m_Type )
					{
					case stage:
						m_DifficultyIcon[p].SetFromSteps( p, GAMESTATE->m_pCurSteps[p] );
						break;
					case course:
						m_DifficultyIcon[p].SetFromTrail( p, GAMESTATE->m_pCurTrail[p] );
						break;
					default:
						ASSERT(0);
					}
					m_DifficultyIcon[p].SetName( ssprintf("DifficultyIconP%d",p+1) );
					SET_XY_AND_ON_COMMAND( m_DifficultyIcon[p] );
					this->AddChild( &m_DifficultyIcon[p] );
					
					m_DifficultyMeter[p].SetName( ssprintf("DifficultyMeterP%d",p+1) );
					m_DifficultyMeter[p].Load( ssprintf("ScreenEvaluation DifficultyMeterP%d",p+1) );
					switch( m_Type )
					{
					case stage:
						m_DifficultyMeter[p].SetFromSteps( GAMESTATE->m_pCurSteps[p] );
						break;
					case course:
						m_DifficultyMeter[p].SetFromTrail( GAMESTATE->m_pCurTrail[p] );
						break;
					default:
						ASSERT(0);
					}
					SET_XY_AND_ON_COMMAND( m_DifficultyMeter[p] );
					this->AddChild( &m_DifficultyMeter[p] );
					
					m_textPlayerOptions[p].LoadFromFont( THEME->GetPathF(m_sName,"PlayerOptions") );
					m_textPlayerOptions[p].SetName( ssprintf("PlayerOptionsP%d",p+1) );
					SET_XY_AND_ON_COMMAND( m_textPlayerOptions[p] );
					vector<CString> v;
					GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.GetThemedMods( v );
					CString sPO = join( PLAYER_OPTIONS_SEPARATOR, v );
					m_textPlayerOptions[p].SetText( sPO );
					this->AddChild( &m_textPlayerOptions[p] );
				}
			}
			break;
		case summary:
			break;
		default:
			ASSERT(0);
		}
	}

	//
	// init grade area
	//
	if( SHOW_GRADE_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprGradeFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("grade frame p%d",p+1)) );
			m_sprGradeFrame[p]->SetName( ssprintf("GradeFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprGradeFrame[p] );
			this->AddChild( m_sprGradeFrame[p] );

			m_Grades[p].Load( THEME->GetPathG(m_sName,"grades") );
			m_Grades[p].SetGrade( p, grade[p] );
			m_Grades[p].SetName( ssprintf("GradeP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_Grades[p] );
			if( SPIN_GRADES )
				m_Grades[p].Spin();
			this->AddChild( &m_Grades[p] );

			m_sprGrade[p].Load( THEME->GetPathG(m_sName,"grade "+GradeToString(grade[p])) );
			m_sprGrade[p]->SetName( ssprintf("GradeP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprGrade[p] );
			this->AddChild( m_sprGrade[p] );
		}
	}

	//
	// init points area
	//
	if( SHOW_POINTS_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprPercentFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("percent frame p%d",p+1)) );
			m_sprPercentFrame[p]->SetName( ssprintf("PercentFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprPercentFrame[p] );
			this->AddChild( m_sprPercentFrame[p] );

			/* Use "ScreenEvaluation Percent" for the [metric set], but position and
			 * tween it with "PercentP1X", etc. */
			m_Percent[p].SetName( ssprintf("PercentP%d",p+1) );
			m_Percent[p].Load( GAMESTATE->m_pPlayerState[p], &STATSMAN->m_CurStageStats.m_player[p], "ScreenEvaluation Percent", true );
			SET_XY_AND_ON_COMMAND( m_Percent[p] );
			this->AddChild( &m_Percent[p] );
		}
	}

	//
	// init bonus area
	//
	if( SHOW_BONUS_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprBonusFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("bonus frame p%d",p+1)) );
			m_sprBonusFrame[p]->SetName( ssprintf("BonusFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprBonusFrame[p] );
			this->AddChild( m_sprBonusFrame[p] );

			for( int r=0; r<NUM_SHOWN_RADAR_CATEGORIES; r++ )	// foreach line
			{
				m_sprPossibleBar[p][r].Load( THEME->GetPathG(m_sName,ssprintf("bar possible p%d",p+1)) );
				m_sprPossibleBar[p][r].SetWidth( m_sprPossibleBar[p][r].GetUnzoomedWidth() * m_StageStats.m_player[p].radarPossible[r] );
				m_sprPossibleBar[p][r].SetName( ssprintf("BarPossible%dP%d",r+1,p+1) );
				SET_XY_AND_ON_COMMAND( m_sprPossibleBar[p][r] );
				this->AddChild( &m_sprPossibleBar[p][r] );

				m_sprActualBar[p][r].Load( THEME->GetPathG(m_sName,ssprintf("bar actual p%d",p+1)) );
				// should be out of the possible bar, not actual (whatever value that is at)
				m_sprActualBar[p][r].SetWidth( m_sprPossibleBar[p][r].GetUnzoomedWidth() * m_StageStats.m_player[p].radarActual[r] );
				
				float value = (float)100 * m_sprActualBar[p][r].GetUnzoomedWidth() / m_sprPossibleBar[p][r].GetUnzoomedWidth();
				LOG->Trace("Radar bar %d of 5 - %f percent", r,  value);
				
				m_sprActualBar[p][r].SetName( ssprintf("BarActual%dP%d",r+1,p+1) );
				SET_XY_AND_ON_COMMAND( m_sprActualBar[p][r] );
				
				// .99999 is fairly close to 1.00, so we use that 
				if( m_StageStats.m_player[p].radarActual[r] > 0.99999f )
					m_sprActualBar[p][r].RunCommands( BAR_ACTUAL_MAX_COMMAND );
				this->AddChild( &m_sprActualBar[p][r] );
			}
		}
	}

	//
	// init survived area
	//
	if( SHOW_SURVIVED_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprSurvivedFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("survived frame p%d",p+1)) );
			m_sprSurvivedFrame[p]->SetName( ssprintf("SurvivedFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprSurvivedFrame[p] );
			this->AddChild( m_sprSurvivedFrame[p] );

			m_textSurvivedNumber[p].LoadFromFont( THEME->GetPathF(m_sName, "stage") );
			m_textSurvivedNumber[p].SetShadowLength( 0 );
			// curewater: edited the "# stages cleared" text so it deducts one if you failed.
			// Should be accurate, but I'm not sure if its "standard" that (bool)true = 1.  (assumption)
			m_textSurvivedNumber[p].SetText( ssprintf("%02d", m_StageStats.m_player[p].iSongsPlayed - (int)m_StageStats.m_player[p].bFailed) );
			m_textSurvivedNumber[p].SetName( ssprintf("SurvivedNumberP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_textSurvivedNumber[p] );
			this->AddChild( &m_textSurvivedNumber[p] );
		}
	}
	
	//
	// init win area
	//
	if( SHOW_WIN_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprWinFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("win frame p%d",p+1)) );
			m_sprWinFrame[p]->SetName( ssprintf("WinFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprWinFrame[p] );
			this->AddChild( m_sprWinFrame[p] );

			m_sprWin[p].Load( THEME->GetPathG(m_sName,ssprintf("win p%d 1x3",p+1)) );
			m_sprWin[p].StopAnimating();
			int iFrame = GAMESTATE->GetStageResult( p );
			m_sprWin[p].SetState( iFrame );
			m_sprWin[p].SetName( ssprintf("WinP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprWin[p] );
			this->AddChild( &m_sprWin[p] );
		}
	}
	
	//
	// init judgment area
	//
	for( int l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
		if( l == 0  && !GAMESTATE->ShowMarvelous() )
			continue;	// skip

		if( SHOW_JUDGMENT(l) )
		{
			m_sprJudgeLabels[l].Load( THEME->GetPathG(m_sName,"judge labels") );
			m_sprJudgeLabels[l].StopAnimating();
			m_sprJudgeLabels[l].SetState( l );
			m_sprJudgeLabels[l].SetName( ssprintf("%sLabel",JUDGE_STRING[l]) );
			SET_XY_AND_ON_COMMAND( m_sprJudgeLabels[l] );
			this->AddChild( &m_sprJudgeLabels[l] );

			FOREACH_EnabledPlayer( p )
			{
				m_textJudgeNumbers[l][p].LoadFromFont( THEME->GetPathF(m_sName, "judge") );
				m_textJudgeNumbers[l][p].SetShadowLength( 0 );
				m_textJudgeNumbers[l][p].RunCommands( PLAYER_COLOR.GetValue(p) );
				m_textJudgeNumbers[l][p].SetName( ssprintf("%sNumberP%d",JUDGE_STRING[l],p+1) );
				SET_XY_AND_ON_COMMAND( m_textJudgeNumbers[l][p] );
				this->AddChild( &m_textJudgeNumbers[l][p] );

				int iValue;
				switch( l )
				{
				case marvelous:	iValue = m_StageStats.m_player[p].iTapNoteScores[TNS_MARVELOUS];	break;
				case perfect:	iValue = m_StageStats.m_player[p].iTapNoteScores[TNS_PERFECT];	break;
				case great:		iValue = m_StageStats.m_player[p].iTapNoteScores[TNS_GREAT];		break;
				case good:		iValue = m_StageStats.m_player[p].iTapNoteScores[TNS_GOOD];		break;
				case boo:		iValue = m_StageStats.m_player[p].iTapNoteScores[TNS_BOO];		break;
				case miss:		iValue = m_StageStats.m_player[p].iTapNoteScores[TNS_MISS];		break;
				case ok:		iValue = m_StageStats.m_player[p].iHoldNoteScores[HNS_OK];		break;
				case max_combo:	iValue = m_StageStats.m_player[p].GetMaxCombo().cnt;				break;
				case error:		iValue = m_StageStats.m_player[p].iTotalError;					break;
				default:	iValue = 0;	ASSERT(0);
				}

				// UGLY... generalize this
				int iNumDigits = (l==max_combo) ? MAX_COMBO_NUM_DIGITS : 4;
				m_textJudgeNumbers[l][p].SetText( ssprintf("%*d",iNumDigits,iValue) );
			}
		}
	}

	for( int l=0; l<NUM_STATS_LINES; l++ ) 
	{
		if( !SHOW_STAT(l) )
			continue;

		m_sprStatsLabel[l].Load( THEME->GetPathG(m_sName,ssprintf("label %s", STATS_STRING[l])) );
		m_sprStatsLabel[l]->StopAnimating();
		m_sprStatsLabel[l]->SetName( ssprintf("%sLabel",STATS_STRING[l]) );
		SET_XY_AND_ON_COMMAND( m_sprStatsLabel[l] );
		this->AddChild( m_sprStatsLabel[l] );

		FOREACH_EnabledPlayer( p )
		{
			m_textStatsText[l][p].LoadFromFont( THEME->GetPathF(m_sName,"stats") );
			m_textStatsText[l][p].RunCommands( PLAYER_COLOR.GetValue(p) );
			m_textStatsText[l][p].SetName( ssprintf("%sTextP%d",STATS_STRING[l],p+1) );
			SET_XY_AND_ON_COMMAND( m_textStatsText[l][p] );
			this->AddChild( &m_textStatsText[l][p] );

			const int indeces[NUM_STATS_LINES] =
			{
				RADAR_NUM_JUMPS, RADAR_NUM_HOLDS, RADAR_NUM_MINES, RADAR_NUM_HANDS, RADAR_NUM_ROLLS
			};
			const int ind = indeces[l];
			const int iActual = (int) roundf(m_StageStats.m_player[p].radarActual[ind]);
			const int iPossible = (int) roundf(m_StageStats.m_player[p].radarPossible[ind]);

			m_textStatsText[l][p].SetText( ssprintf("%3d/%3d",iActual,iPossible) );
		}
	}

	//
	// init score area
	//
	if( SHOW_SCORE_AREA )
	{
		m_sprScoreLabel.Load( THEME->GetPathG(m_sName,"score label") );
		m_sprScoreLabel->SetName( "ScoreLabel" );
		SET_XY_AND_ON_COMMAND( m_sprScoreLabel );
		this->AddChild( m_sprScoreLabel );

		FOREACH_EnabledPlayer( p )
		{
			m_textScore[p].LoadFromFont( THEME->GetPathF(m_sName, "score") );
			m_textScore[p].SetShadowLength( 0 );
			m_textScore[p].RunCommands( PLAYER_COLOR.GetValue(p) );
			m_textScore[p].SetName( ssprintf("ScoreNumberP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_textScore[p] );
			m_textScore[p].SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS, m_StageStats.m_player[p].iScore) );
			this->AddChild( &m_textScore[p] );
		}
	}

	if( SHOW_TOTAL_SCORE_AREA )
	{
		m_sprTotalScoreLabel.Load( THEME->GetPathG(m_sName,"totalscore label") );
		m_sprTotalScoreLabel->SetName( "TotalScoreLabel" );
		SET_XY_AND_ON_COMMAND( m_sprTotalScoreLabel );
		this->AddChild( m_sprTotalScoreLabel );

		FOREACH_EnabledPlayer( p )
		{
			int iTotalScore=0;
			for( unsigned i=0; i<STATSMAN->m_vPlayedStageStats.size(); i++ )
				iTotalScore += STATSMAN->m_vPlayedStageStats[i].m_player[p].iScore;

			//iTotalScore += m_StageStats.m_player[p].iScore;

			m_textTotalScore[p].LoadFromFont( THEME->GetPathF(m_sName, "totalscore") );
			m_textTotalScore[p].SetShadowLength( 0 );
			m_textTotalScore[p].RunCommands( PLAYER_COLOR.GetValue(p) );
			m_textTotalScore[p].SetName( ssprintf("TotalScoreNumberP%d",p+1) );
			m_textTotalScore[p].SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS+2, iTotalScore) );
			SET_XY_AND_ON_COMMAND( m_textTotalScore[p] );

			this->AddChild( &m_textTotalScore[p] );
		}
	}

	//
	// init time area
	//
	if( SHOW_TIME_AREA )
	{
		m_sprTimeLabel.Load( THEME->GetPathG(m_sName,"time label") );
		m_sprTimeLabel->SetName( "TimeLabel" );
		SET_XY_AND_ON_COMMAND( m_sprTimeLabel );
		this->AddChild( m_sprTimeLabel );

		FOREACH_EnabledPlayer( p )
		{
			m_textTime[p].LoadFromFont( THEME->GetPathF(m_sName, "time") );
			m_textTime[p].SetShadowLength( 0 );
			m_textTime[p].RunCommands( PLAYER_COLOR.GetValue(p) );
			m_textTime[p].SetName( ssprintf("TimeNumberP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_textTime[p] );
			m_textTime[p].SetText( SecondsToMMSSMsMs(m_StageStats.m_player[p].fAliveSeconds) );
			this->AddChild( &m_textTime[p] );
		}
	}


	//
	// init extra area
	//
	FOREACH_EnabledPlayer( p )
	{
		if( m_iMachineHighScoreIndex[p] != -1 )
		{
			m_sprMachineRecord[p].Load( THEME->GetPathG( m_sName, ssprintf("MachineRecord %02d",m_iMachineHighScoreIndex[p]+1) ) );
			m_sprMachineRecord[p]->SetName( ssprintf("MachineRecordP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprMachineRecord[p] );
			this->AddChild( m_sprMachineRecord[p] );
		}
		if( m_iPersonalHighScoreIndex[p] != -1 )
		{
			m_sprPersonalRecord[p].Load( THEME->GetPathG( m_sName, ssprintf("PersonalRecord %02d",m_iPersonalHighScoreIndex[p]+1) ) );
			m_sprPersonalRecord[p]->SetName( ssprintf("PersonalRecordP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprPersonalRecord[p] );
			this->AddChild( m_sprPersonalRecord[p] );
		}
		if( SHOW_PER_DIFFICULTY_AWARD && m_pdaToShow[p]!=PER_DIFFICULTY_AWARD_INVALID )
		{
			CString sAward = PerDifficultyAwardToString( m_pdaToShow[p] );

			m_PerDifficultyAward[p].Load( THEME->GetPathG(m_sName,"PerDifficultyAward "+sAward) );
			m_PerDifficultyAward[p]->SetName( ssprintf("PerDifficultyAwardP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_PerDifficultyAward[p] );
			this->AddChild( m_PerDifficultyAward[p] );
		}
		if( SHOW_PEAK_COMBO_AWARD && m_pcaToShow[p]!=PEAK_COMBO_AWARD_INVALID )
		{
			CString sAward = PeakComboAwardToString( m_pcaToShow[p] );

			m_PeakComboAward[p].Load( THEME->GetPathG(m_sName,"PeakComboAward "+sAward) );
			m_PeakComboAward[p]->SetName( ssprintf("PeakComboAwardP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_PeakComboAward[p] );
			this->AddChild( m_PeakComboAward[p] );
		}
	}

	bool bOneHasNewTopRecord = false;
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->IsPlayerEnabled(p) && (m_iMachineHighScoreIndex[p] != -1 || m_iPersonalHighScoreIndex[p] != -1) )
			bOneHasNewTopRecord = true;

	Grade best_grade = Grade_NoData;
	FOREACH_PlayerNumber( p )
		best_grade = min( best_grade, grade[p] ); 
	
	if( PREFSMAN->m_bAllowExtraStage && m_bTryExtraStage )
	{
		m_sprTryExtraStage.Load( THEME->GetPathG(m_sName,GAMESTATE->IsExtraStage()?"try extra2":"try extra1") );
		m_sprTryExtraStage->SetName( "TryExtraStage" );
		SET_XY_AND_ON_COMMAND( m_sprTryExtraStage );
		this->AddChild( m_sprTryExtraStage );

		if( GAMESTATE->IsExtraStage() )
			SOUND->PlayOnce( THEME->GetPathS(m_sName,"try extra2") );
		else
			SOUND->PlayOnce( THEME->GetPathS(m_sName,"try extra1") );
	}
	else if( bOneHasNewTopRecord  &&  ANNOUNCER->HasSoundsFor("evaluation new record") )
	{
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation new record") );
	}
	else
	{	
		switch( m_Type )
		{
		case stage:
			switch( GAMESTATE->m_PlayMode )
			{
			case PLAY_MODE_BATTLE:
				{
					bool bWon = GAMESTATE->GetStageResult(GAMESTATE->m_MasterPlayerNumber) == RESULT_WIN;
					if( bWon )
						SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation win") );
					else
						SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation lose") );
				}
				break;
			default:
				SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation "+GradeToOldString(best_grade)) );
				break;
			}
			break;
		case course:
		case summary:
			SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final "+GradeToOldString(best_grade)) );
			break;
		default:
			ASSERT(0);
		}
	}

	switch( best_grade )
	{
	case Grade_Tier01:
	case Grade_Tier02:	
	case Grade_Tier03:	
		this->PostScreenMessage( SM_PlayCheer, CHEER_DELAY_SECONDS );	
		break;
	}

	this->SortByDrawOrder();

	this->PostScreenMessage( SM_AddBonus, 1.5f );
 }


void ScreenEvaluation::CommitScores(
	const StageStats &m_StageStats, 
	int iPersonalHighScoreIndexOut[NUM_PLAYERS], 
	int iMachineHighScoreIndexOut[NUM_PLAYERS], 
	RankingCategory rcOut[NUM_PLAYERS],
	PerDifficultyAward pdaToShowOut[NUM_PLAYERS],
	PeakComboAward pcaToShowOut[NUM_PLAYERS] )
{
	FOREACH_PlayerNumber( pn )
	{
		iPersonalHighScoreIndexOut[pn] = -1;
		iMachineHighScoreIndexOut[pn] = -1;
		rcOut[pn] = RANKING_INVALID;
		pdaToShowOut[pn] = PER_DIFFICULTY_AWARD_INVALID;
		pcaToShowOut[pn] = PEAK_COMBO_AWARD_INVALID;
	}

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		return; /* don't save scores in battle */
	}

	if( PREFSMAN->m_bScreenTestMode )
	{
		FOREACH_PlayerNumber( pn )
		{
			iPersonalHighScoreIndexOut[pn] = 0;
			iMachineHighScoreIndexOut[pn] = 0;
		}
	}

	// don't save scores if the player chose not to
	if( !GAMESTATE->m_SongOptions.m_bSaveScore )
		return;

	LOG->Trace( "saving stats and high scores" );

	FOREACH_HumanPlayer( p )
	{
		// don't save scores if the player is disqualified
		if( GAMESTATE->IsDisqualified(p) )
			continue;

		Song* pSong = GAMESTATE->m_pCurSong;
		Steps* pSteps = GAMESTATE->m_pCurSteps[p];

		// whether or not to save scores when the stage was failed
		// depends on if this is a course or not ... it's handled
		// below in the switch

		HighScore &hs = m_HighScore[p];
		hs.SetName( RANKING_TO_FILL_IN_MARKER[p] );
		hs.SetGrade( m_StageStats.m_player[p].GetGrade() );
		hs.SetScore( m_StageStats.m_player[p].iScore );
		hs.SetPercentDP( m_StageStats.m_player[p].GetPercentDancePoints() );
		hs.SetSurviveSeconds( m_StageStats.m_player[p].fAliveSeconds );
		hs.SetModifiers( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.GetString() );
		hs.SetDateTime( DateTime::GetNowDateTime() );
		hs.SetPlayerGuid( PROFILEMAN->IsPersistentProfile(p) ? PROFILEMAN->GetProfile(p)->m_sGuid : CString("") );
		hs.SetMachineGuid( PROFILEMAN->GetMachineProfile()->m_sGuid );
		hs.SetProductID( PREFSMAN->m_iProductID );
		FOREACH_TapNoteScore( tns )
			hs.SetTapNoteScore( tns, m_StageStats.m_player[p].iTapNoteScores[tns] );
		FOREACH_HoldNoteScore( hns )
			hs.SetHoldNoteScore( hns, m_StageStats.m_player[p].iHoldNoteScores[hns] );
		hs.SetRadarValues( m_StageStats.m_player[p].radarActual );
		hs.SetLifeRemainingSeconds( m_StageStats.m_player[p].fLifeRemainingSeconds );


		StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;

		switch( m_Type )
		{
		case stage:
			{
				// don't save scores for a failed song
				if( m_StageStats.m_player[p].bFailed )
					continue;

				ASSERT( pSteps );

				PROFILEMAN->AddStepsScore( pSong, pSteps, p, hs, iPersonalHighScoreIndexOut[p], iMachineHighScoreIndexOut[p] );
			}
			break;

		case summary:
			{
				// don't save scores if any stage was failed
				if( m_StageStats.m_player[p].bFailed ) 
					continue;

				int iAverageMeter = m_StageStats.GetAverageMeter(p);
				rcOut[p] = AverageMeterToRankingCategory( iAverageMeter );

				PROFILEMAN->AddCategoryScore( st, rcOut[p], p, hs, iPersonalHighScoreIndexOut[p], iMachineHighScoreIndexOut[p] );
				
				// TRICKY:  Increment play count here, and not on ScreenGameplay like the others.
				PROFILEMAN->IncrementCategoryPlayCount( st, rcOut[p], p );
			}
			break;

		case course:
			{
				Course* pCourse = GAMESTATE->m_pCurCourse;
				ASSERT( pCourse );
				Trail* pTrail = GAMESTATE->m_pCurTrail[p];

				PROFILEMAN->AddCourseScore( pCourse, pTrail, p, hs, iPersonalHighScoreIndexOut[p], iMachineHighScoreIndexOut[p] );
			}
			break;
		default:
			ASSERT(0);
		}
	}

	LOG->Trace( "done saving stats and high scores" );

	// If both players get a machine high score in the same HighScoreList,
	// then one player's score may have bumped the other player.  Look in 
	// the HighScoreList and re-get the high score index.
	FOREACH_HumanPlayer( p )
	{
		if( iMachineHighScoreIndexOut[p] == -1 )	// no record
			continue;	// skip

		HighScore &hs = m_HighScore[p];
		Profile* pProfile = PROFILEMAN->GetMachineProfile();
		StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;

		const HighScoreList *pHSL = NULL;
		switch( m_Type )
		{
		case stage:
			{
				Song* pSong = GAMESTATE->m_pCurSong;
				Steps* pSteps = GAMESTATE->m_pCurSteps[p];
				pHSL = &pProfile->GetStepsHighScoreList( pSong, pSteps );
			}
			break;
		case summary:
			{
				pHSL = &pProfile->GetCategoryHighScoreList( st, rcOut[p] );
			}
			break;
		case course:
			{
				Course* pCourse = GAMESTATE->m_pCurCourse;
				ASSERT( pCourse );
				Trail *pTrail = GAMESTATE->m_pCurTrail[p];
				ASSERT( pTrail );
				pHSL = &pProfile->GetCourseHighScoreList( pCourse, pTrail );
			}
			break;
		default:
			ASSERT(0);
		}
		
		vector<HighScore>::const_iterator iter = find( pHSL->vHighScores.begin(), pHSL->vHighScores.end(), hs );
		if( iter == pHSL->vHighScores.end() )
			iMachineHighScoreIndexOut[p] = -1;
		else
			iMachineHighScoreIndexOut[p] = iter - pHSL->vHighScores.begin();
	}

	
	LOG->Trace( "hand out awards" );
	
	FOREACH_HumanPlayer( p )
	{
		deque<PerDifficultyAward> &vPdas = GAMESTATE->m_vLastPerDifficultyAwards[p];

		LOG->Trace( "per difficulty awards" );

		// per-difficulty awards
		switch( m_Type )
		{
		case stage:
		case course:
			// don't give per-difficutly awards if using easy mods
			if( !GAMESTATE->IsDisqualified(p) )
			{
				if( m_StageStats.m_player[p].FullComboOfScore( TNS_GREAT ) )
					vPdas.push_back( AWARD_FULL_COMBO_GREATS );
				if( m_StageStats.m_player[p].SingleDigitsOfScore( TNS_GREAT ) )
					vPdas.push_back( AWARD_SINGLE_DIGIT_GREATS );
				if( m_StageStats.m_player[p].FullComboOfScore( TNS_PERFECT ) )
					vPdas.push_back( AWARD_FULL_COMBO_PERFECTS );
				if( m_StageStats.m_player[p].SingleDigitsOfScore( TNS_PERFECT ) )
					vPdas.push_back( AWARD_SINGLE_DIGIT_PERFECTS );
				if( m_StageStats.m_player[p].FullComboOfScore( TNS_MARVELOUS ) )
					vPdas.push_back( AWARD_FULL_COMBO_MARVELOUSES );
				
				if( m_StageStats.m_player[p].OneOfScore( TNS_GREAT ) )
					vPdas.push_back( AWARD_ONE_GREAT );
				if( m_StageStats.m_player[p].OneOfScore( TNS_PERFECT ) )
					vPdas.push_back( AWARD_ONE_PERFECT );

				float fPercentGreats = m_StageStats.m_player[p].GetPercentageOfTaps( TNS_GREAT );
				if( fPercentGreats >= 0.8f )
					vPdas.push_back( AWARD_GREATS_80_PERCENT );
				if( fPercentGreats >= 0.9f )
					vPdas.push_back( AWARD_GREATS_90_PERCENT );
				if( fPercentGreats >= 1.f )
					vPdas.push_back( AWARD_GREATS_100_PERCENT );
			}
		}

		// Max one PDA per stage
		if( !vPdas.empty() )
			vPdas.erase( vPdas.begin(), vPdas.end()-1 );
		
		if( !vPdas.empty() )
			pdaToShowOut[p] = vPdas.back();

		LOG->Trace( "done with per difficulty awards" );

		// DO give peak combo awards if using easy mods
		int iComboAtStartOfStage = m_StageStats.m_player[p].GetComboAtStartOfStage();
		int iPeakCombo = m_StageStats.m_player[p].GetMaxCombo().cnt;

		FOREACH_PeakComboAward( pca )
		{
			  int iLevel = 1000 * (pca+1);
			bool bCrossedLevel = iComboAtStartOfStage < iLevel && iPeakCombo >= iLevel;
			LOG->Trace( "pca = %d, iLevel = %d, bCrossedLevel = %d", pca, iLevel, bCrossedLevel );
			if( bCrossedLevel )
			{
				GAMESTATE->m_vLastPeakComboAwards[p].push_back( pca );
			}
		}

		if( !GAMESTATE->m_vLastPeakComboAwards[p].empty() )
			pcaToShowOut[p] = GAMESTATE->m_vLastPeakComboAwards[p].back();

		LOG->Trace( "done with per combo awards" );
	}

	LOG->Trace( "done handing out awards." );
}


void ScreenEvaluation::TweenOursOffScreen()
{
	// large banner area
	OFF_COMMAND( m_LargeBanner );
	OFF_COMMAND( m_sprLargeBannerFrame );
	FOREACH_EnabledPlayer( p )
	{
		OFF_COMMAND( m_DifficultyIcon[p] );
		OFF_COMMAND( m_DifficultyMeter[p] );
		OFF_COMMAND( m_textPlayerOptions[p] );
	}

	// small banner area
	for( int i=0; i<MAX_SONGS_TO_SHOW; i++ )
	{
		OFF_COMMAND( m_SmallBanner[i] );
		OFF_COMMAND( m_sprSmallBannerFrame[i] );
	}

	// grade area
	if( SHOW_GRADE_AREA )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			OFF_COMMAND( m_sprGradeFrame[p] );
			OFF_COMMAND( m_Grades[p] );
			OFF_COMMAND( m_sprGrade[p] );
		}
	}

	// points area
	if( SHOW_POINTS_AREA )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			OFF_COMMAND( m_sprPercentFrame[p] );
			OFF_COMMAND( m_Percent[p] );
		}
	}

	// bonus area
	if( SHOW_BONUS_AREA )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			OFF_COMMAND( m_sprBonusFrame[p] );
			for( int r=0; r<NUM_SHOWN_RADAR_CATEGORIES; r++ )	// foreach line
			{
				OFF_COMMAND( m_sprPossibleBar[p][r] );
				OFF_COMMAND( m_sprActualBar[p][r] );
			}
		}
	}

	// survived area
	if( SHOW_SURVIVED_AREA )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			OFF_COMMAND( m_sprSurvivedFrame[p] );
			OFF_COMMAND( m_textSurvivedNumber[p] );
		}
	}

	// win area
	if( SHOW_WIN_AREA )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			OFF_COMMAND( m_sprWinFrame[p] );
			OFF_COMMAND( m_sprWin[p] );
		}
	}

	// judgement area
	for( int l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
		if( !SHOW_JUDGMENT(l) )
			continue;

		OFF_COMMAND( m_sprJudgeLabels[l] );

		FOREACH_EnabledPlayer( p ) 
			OFF_COMMAND( m_textJudgeNumbers[l][p] );
	}

	// stats area
	for( int l=0; l<NUM_STATS_LINES; l++ ) 
	{
		if( !SHOW_STAT(l) )
			continue;

		OFF_COMMAND( m_sprStatsLabel[l] );

		FOREACH_EnabledPlayer( p ) 
			OFF_COMMAND( m_textStatsText[l][p] );
	}

	// score area
	if( SHOW_SCORE_AREA )
	{
		OFF_COMMAND( m_sprScoreLabel );
		FOREACH_EnabledPlayer( p ) 
			OFF_COMMAND( m_textScore[p] );
	}

	// total score area
	if( SHOW_TOTAL_SCORE_AREA )
	{
		OFF_COMMAND( m_sprTotalScoreLabel );
		FOREACH_EnabledPlayer( p ) 
			OFF_COMMAND( m_textTotalScore[p] );
	}

	// time area
	if( SHOW_TIME_AREA )
	{
		OFF_COMMAND( m_sprTimeLabel );
		FOREACH_EnabledPlayer( p ) 
			OFF_COMMAND( m_textTime[p] );
	}

	// extra area
	FOREACH_EnabledPlayer( p ) 
	{
		OFF_COMMAND( m_sprMachineRecord[p] );
		OFF_COMMAND( m_sprPersonalRecord[p] );
		if( m_PerDifficultyAward[p].IsLoaded() )
			OFF_COMMAND( m_PerDifficultyAward[p] );
		if( m_PeakComboAward[p].IsLoaded() )
			OFF_COMMAND( m_PeakComboAward[p] );
	}
	OFF_COMMAND( m_sprTryExtraStage );
}

void ScreenEvaluation::Input( const InputEventPlus &input )
{
//	LOG->Trace( "ScreenEvaluation::Input()" );

	if( IsTransitioning() )
		return;

	if( input.GameI.IsValid() )
	{
		PlayerNumber pn = GAMESTATE->GetCurrentStyle()->ControllerToPlayerNumber( input.GameI.controller );
		HighScore &hs = m_HighScore[pn];


		if( CodeDetector::EnteredCode(input.GameI.controller, CODE_SAVE_SCREENSHOT1) ||
			CodeDetector::EnteredCode(input.GameI.controller, CODE_SAVE_SCREENSHOT2) )
		{
			if( !m_bSavedScreenshot[pn]  &&	// only allow one screenshot
				PROFILEMAN->IsPersistentProfile(pn) )
			{
				if( PROFILEMAN->ProfileWasLoadedFromMemoryCard(pn) )
					MEMCARDMAN->MountCard( pn );
			
				Profile* pProfile = PROFILEMAN->GetProfile(pn);
				CString sDir = PROFILEMAN->GetProfileDir((ProfileSlot)pn) + "Screenshots/";
				int iScreenshotIndex = pProfile->GetNextScreenshotIndex();
				CString sFileName = SaveScreenshot( sDir, true, true, iScreenshotIndex );
				CString sPath = sDir+sFileName;
				
				if( !sFileName.empty() )
				{
					Screenshot screenshot;
					screenshot.sFileName = sFileName;
					screenshot.sMD5 = CRYPTMAN->GetMD5( sPath );
					screenshot.highScore = hs;
					pProfile->AddScreenshot( screenshot );
				}

				if( PROFILEMAN->ProfileWasLoadedFromMemoryCard(pn) )
					MEMCARDMAN->UnmountCard( pn );

				m_bSavedScreenshot[pn] = true;
				return;	// handled
			}
		}
	}

	ScreenWithMenuElements::Input( input );
}

void ScreenEvaluation::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_MenuTimer )
	{
		EndScreen();
	}
	else if( SM == SM_PlayCheer )
	{
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation cheer") );
	}
	else if( SM == SM_AddBonus )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			if( STATSMAN->m_CurStageStats.m_player[p].iBonus == 0 )
				continue;

			if( GAMESTATE->IsCourseMode() )
				continue;

			int increment = STATSMAN->m_CurStageStats.m_player[p].iBonus/10;
			if( increment < 1 )
				increment = min( 1024, STATSMAN->m_CurStageStats.m_player[p].iBonus );

			STATSMAN->m_CurStageStats.m_player[p].iBonus -= increment;
			STATSMAN->m_CurStageStats.m_player[p].iScore += increment;

			if( SHOW_SCORE_AREA )
				m_textScore[p].SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS, STATSMAN->m_CurStageStats.m_player[p].iScore) );
		}
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

void ScreenEvaluation::MenuBack( PlayerNumber pn )
{
	MenuStart( pn );
}

void ScreenEvaluation::MenuStart( PlayerNumber pn )
{
	m_soundStart.Play();

	EndScreen();
}

void ScreenEvaluation::EndScreen()
{
	TweenOursOffScreen();

	FOREACH_PlayerNumber( p )
		m_Grades[p].SettleImmediately();

	if( !GAMESTATE->IsEventMode() )
	{
		switch( m_Type )
		{
		case stage:
			if( !m_bTryExtraStage && (GAMESTATE->IsFinalStage() || GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2()) )
			{
				/* Tween the screen out, but leave the MenuElements where they are.
				 * Play the "swoosh" sound manually (would normally be played by the ME
				 * tween out). */
				TweenOursOffScreen();
			}
			break;
		}
	}
	StartTransitioning( SM_GoToNextScreen );
}

// lua start
#include "LuaBinding.h"

class LunaScreenEvaluation: public Luna<ScreenEvaluation>
{
public:
	LunaScreenEvaluation() { LUA->Register( Register ); }
	static int GetEvalStageStats( T* p, lua_State *L ) { p->m_StageStats.PushSelf( L ); return 1; }
	static int GetPersonalHighScoreIndex( T* p, lua_State *L ) { lua_pushnumber( L, p->m_iPersonalHighScoreIndex[IArg(1)] ); return 1; }
	static int GetMachineHighScoreIndex( T* p, lua_State *L ) { lua_pushnumber( L, p->m_iMachineHighScoreIndex[IArg(1)] ); return 1; }
	static int GetPerDifficultyAward( T* p, lua_State *L ) { lua_pushnumber( L, p->m_pdaToShow[IArg(1)] ); return 1; }
	static int GetPeakComboAward( T* p, lua_State *L ) { lua_pushnumber( L, p->m_pcaToShow[IArg(1)] ); return 1; }

	static void Register( Lua *L )
	{
		ADD_METHOD( GetEvalStageStats )
		ADD_METHOD( GetPersonalHighScoreIndex )
		ADD_METHOD( GetMachineHighScoreIndex )
		ADD_METHOD( GetPerDifficultyAward )
		ADD_METHOD( GetPeakComboAward )
		Luna<T>::Register( L );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenEvaluation, ScreenWithMenuElements );
// lua end

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
