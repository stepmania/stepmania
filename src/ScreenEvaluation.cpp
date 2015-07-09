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
#include "Course.h"
#include "LightsManager.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "Song.h"
#include "StatsManager.h"
#include "Grade.h"
#include "CodeDetector.h"
#include "RageDisplay.h"
#include "StepMania.h"
#include "CryptManager.h"
#include "MemoryCardManager.h"
#include "PlayerState.h"
#include "CommonMetrics.h"
#include "ScoreKeeperNormal.h"
#include "InputEventPlus.h"

// metrics that are common to all ScreenEvaluation classes
#define BANNER_WIDTH			THEME->GetMetricF(m_sName,"BannerWidth")
#define BANNER_HEIGHT			THEME->GetMetricF(m_sName,"BannerHeight")
static const char *JudgmentLineNames[] =
{
	"W1", "W2", "W3", "W4", "W5", "Miss", "Held", "MaxCombo"
};
XToString( JudgmentLine );
LuaXType( JudgmentLine );
XToLocalizedString( JudgmentLine );
LuaFunction( JudgmentLineToLocalizedString, JudgmentLineToLocalizedString(Enum::Check<JudgmentLine>(L, 1)) );

static const char *DetailLineNames[NUM_DetailLine] =
{
	"NumSteps","Jumps", "Holds", "Mines", "Hands", "Rolls", "Lifts", "Fakes"
};
XToString( DetailLine );
#define DETAILLINE_FORMAT			THEME->GetMetric (m_sName,"DetailLineFormat")

#define CHEER_DELAY_SECONDS			THEME->GetMetricF(m_sName,"CheerDelaySeconds")
#define BAR_ACTUAL_MAX_COMMAND			THEME->GetMetricA(m_sName,"BarActualMaxCommand")

// metrics that are specific to classes derived from ScreenEvaluation
#define SHOW_BANNER_AREA			THEME->GetMetricB(m_sName,"ShowBannerArea")
#define SHOW_GRADE_AREA				THEME->GetMetricB(m_sName,"ShowGradeArea")
#define SHOW_POINTS_AREA			THEME->GetMetricB(m_sName,"ShowPointsArea")
#define SHOW_BONUS_AREA				THEME->GetMetricB(m_sName,"ShowBonusArea")
#define SHOW_SURVIVED_AREA			THEME->GetMetricB(m_sName,"ShowSurvivedArea")
#define SHOW_WIN_AREA				THEME->GetMetricB(m_sName,"ShowWinArea")
#define SHOW_SHARED_JUDGMENT_LINE_LABELS	THEME->GetMetricB(m_sName,"ShowSharedJudgmentLineLabels")
#define SHOW_JUDGMENT_LINE( l )			THEME->GetMetricB(m_sName,"ShowJudgmentLine"+JudgmentLineToString(l))

#define SHOW_DETAIL_AREA			THEME->GetMetricB(m_sName,"ShowDetailArea")
#define SHOW_SCORE_AREA				THEME->GetMetricB(m_sName,"ShowScoreArea")
#define SHOW_TIME_AREA				THEME->GetMetricB(m_sName,"ShowTimeArea")
#define SHOW_RECORDS_AREA			THEME->GetMetricB(m_sName,"ShowRecordsArea")
#define PLAYER_OPTIONS_HIDE_FAIL_TYPE	THEME->GetMetricB(m_sName,"PlayerOptionsHideFailType")
#define PLAYER_OPTIONS_SEPARATOR	THEME->GetMetric (m_sName,"PlayerOptionsSeparator")
#define CHECKPOINTS_WITH_JUDGMENTS	THEME->GetMetricB(m_sName,"CheckpointsWithJudgments")

static const int NUM_SHOWN_RADAR_CATEGORIES = 5;

AutoScreenMessage( SM_PlayCheer );

REGISTER_SCREEN_CLASS( ScreenEvaluation );

ScreenEvaluation::ScreenEvaluation()
{
	GAMESTATE->m_AdjustTokensBySongCostForFinalStageCheck= false;
}

ScreenEvaluation::~ScreenEvaluation()
{
	GAMESTATE->m_AdjustTokensBySongCostForFinalStageCheck= true;
}

void ScreenEvaluation::Init()
{
	LOG->Trace( "ScreenEvaluation::Init()" );

	// debugging
	// Only fill StageStats with fake info if we're the InitialScreen
	// (i.e. StageStats not already filled)
	if( PREFSMAN->m_sTestInitialScreen.Get() == m_sName )
	{
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_1);
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_2);

		STATSMAN->m_vPlayedStageStats.clear();
		STATSMAN->m_vPlayedStageStats.push_back( StageStats() );
		StageStats &ss = STATSMAN->m_vPlayedStageStats.back();

		GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );
		GAMESTATE->SetCurrentStyle( GAMEMAN->GameAndStringToStyle(GAMEMAN->GetDefaultGame(),"versus"), PLAYER_INVALID );
		ss.m_playMode = GAMESTATE->m_PlayMode;
		ss.m_Stage = Stage_1st;
		enum_add( ss.m_Stage, rand()%3 );
		ss.m_EarnedExtraStage = (EarnedExtraStage)(rand() % NUM_EarnedExtraStage);
		GAMESTATE->SetMasterPlayerNumber(PLAYER_1);
		GAMESTATE->m_pCurSong.Set( SONGMAN->GetRandomSong() );
		ss.m_vpPlayedSongs.push_back( GAMESTATE->m_pCurSong );
		ss.m_vpPossibleSongs.push_back( GAMESTATE->m_pCurSong );
		GAMESTATE->m_pCurCourse.Set( SONGMAN->GetRandomCourse() );
		GAMESTATE->m_iCurrentStageIndex = 0;
		FOREACH_ENUM( PlayerNumber, p )
			GAMESTATE->m_iPlayerStageTokens[p] = 1;

		FOREACH_PlayerNumber( p )
		{
			ss.m_player[p].m_pStyle = GAMESTATE->GetCurrentStyle(p);
			if( RandomInt(2) )
				PO_GROUP_ASSIGN_N( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions, ModsLevel_Stage, m_bTransforms, PlayerOptions::TRANSFORM_ECHO, true );	// show "disqualified"
			SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Stage, m_fMusicRate, 1.1f );

			GAMESTATE->JoinPlayer( p );
			GAMESTATE->m_pCurSteps[p].Set( GAMESTATE->m_pCurSong->GetAllSteps()[0] );
			if( GAMESTATE->m_pCurCourse )
			{
				vector<Trail*> apTrails;
				GAMESTATE->m_pCurCourse->GetAllTrails( apTrails );
				if( apTrails.size() )
					GAMESTATE->m_pCurTrail[p].Set( apTrails[0] );
			}
			ss.m_player[p].m_vpPossibleSteps.push_back( GAMESTATE->m_pCurSteps[PLAYER_1] );
			ss.m_player[p].m_iStepsPlayed = 1;

			PO_GROUP_ASSIGN( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions, ModsLevel_Stage, m_fScrollSpeed, 2.0f );
			PO_GROUP_CALL( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions, ModsLevel_Stage, ChooseRandomModifiers );
		}

		for( float f = 0; f < 100.0f; f += 1.0f )
		{
			float fP1 = fmodf(f/100*4+.3f,1);
			ss.m_player[PLAYER_1].SetLifeRecordAt( fP1, f );
			ss.m_player[PLAYER_2].SetLifeRecordAt( 1-fP1, f );
		}

		FOREACH_PlayerNumber( p )
		{
			float fSeconds = GAMESTATE->m_pCurSong->GetStepsSeconds();
			ss.m_player[p].m_iActualDancePoints = RandomInt( 3 );
			ss.m_player[p].m_iPossibleDancePoints = 2;
			if( RandomInt(2) )
				ss.m_player[p].m_iCurCombo = RandomInt(15000);
			else
				ss.m_player[p].m_iCurCombo = 0;
			ss.m_player[p].UpdateComboList( 0, true );

			ss.m_player[p].m_iCurCombo += 50;
			ss.m_player[p].UpdateComboList( 0.10f * fSeconds, false );

			ss.m_player[p].m_iCurCombo = 0;
			ss.m_player[p].UpdateComboList( 0.15f * fSeconds, false );
			ss.m_player[p].m_iCurCombo = 1;
			ss.m_player[p].UpdateComboList( 0.25f * fSeconds, false );
			ss.m_player[p].m_iCurCombo = 50;
			ss.m_player[p].UpdateComboList( 0.35f * fSeconds, false );
			ss.m_player[p].m_iCurCombo = 0;
			ss.m_player[p].UpdateComboList( 0.45f * fSeconds, false );
			ss.m_player[p].m_iCurCombo = 1;
			ss.m_player[p].UpdateComboList( 0.50f * fSeconds, false );
			ss.m_player[p].m_iCurCombo = 100;
			ss.m_player[p].UpdateComboList( 1.00f * fSeconds, false );
			if( RandomInt(5) == 0 )
			{
				ss.m_player[p].m_bFailed = true;
			}
			ss.m_player[p].m_iTapNoteScores[TNS_W1] = RandomInt( 3 );
			ss.m_player[p].m_iTapNoteScores[TNS_W2] = RandomInt( 3 );
			ss.m_player[p].m_iTapNoteScores[TNS_W3] = RandomInt( 3 );
			ss.m_player[p].m_iPossibleGradePoints = 4*ScoreKeeperNormal::TapNoteScoreToGradePoints(TNS_W1, false);
			ss.m_player[p].m_fLifeRemainingSeconds = randomf( 90, 580 );
			ss.m_player[p].m_iScore = rand() % (900*1000*1000);
			ss.m_player[p].m_iPersonalHighScoreIndex = (rand() % 3) - 1;
			ss.m_player[p].m_iMachineHighScoreIndex = (rand() % 3) - 1;
			ss.m_player[p].m_PeakComboAward = (PeakComboAward)(rand()%NUM_PeakComboAward);
			ss.m_player[p].m_StageAward = (StageAward)(rand()%NUM_StageAward);

			FOREACH_ENUM( RadarCategory, rc )
			{
				switch( rc )
				{
					case RadarCategory_Stream:
					case RadarCategory_Voltage:
					case RadarCategory_Air:
					case RadarCategory_Freeze:
					case RadarCategory_Chaos:
						ss.m_player[p].m_radarPossible[rc] = randomf( 0, 1 );
						ss.m_player[p].m_radarActual[rc] = randomf( 0, ss.m_player[p].m_radarPossible[rc] );
						break;
					case RadarCategory_TapsAndHolds:
					case RadarCategory_Jumps:
					case RadarCategory_Holds:
					case RadarCategory_Mines:
					case RadarCategory_Hands:
					case RadarCategory_Rolls:
					case RadarCategory_Lifts:
					case RadarCategory_Fakes:
						ss.m_player[p].m_radarPossible[rc] = 1 + (rand() % 200);
						ss.m_player[p].m_radarActual[rc] = rand() % (int)(ss.m_player[p].m_radarPossible[rc]);
						break;
					default: break;
				}

				;	// filled in by ScreenGameplay on start of notes
			}
		}
	}

	ASSERT_M( !STATSMAN->m_vPlayedStageStats.empty(), "PlayerStageStats is empty!" );
	m_pStageStats = &STATSMAN->m_vPlayedStageStats.back();

	ZERO( m_bSavedScreenshot );

	// Figure out which statistics and songs we're going to display
	SUMMARY.Load( m_sName, "Summary" );
	if( SUMMARY )
	{
		STATSMAN->GetFinalEvalStageStats( m_FinalEvalStageStats );
		m_pStageStats = &m_FinalEvalStageStats;
	}

	// update persistent statistics
	if( SUMMARY )
		m_pStageStats->FinalizeScores( true );

	// Run this here, so STATSMAN->m_CurStageStats is available to overlays.
	ScreenWithMenuElements::Init();

	// Calculate grades
	Grade grade[NUM_PLAYERS];

	FOREACH_PlayerNumber( p )
	{
		if( GAMESTATE->IsPlayerEnabled(p) )
			grade[p] = m_pStageStats->m_player[p].GetGrade();
		else
			grade[p] = Grade_Failed;
	}

	// load sounds
	m_soundStart.Load( THEME->GetPathS(m_sName,"start") );

	// init banner area
	if( SHOW_BANNER_AREA )
	{
		if( SUMMARY )
		{
			for( size_t i=0; i<m_pStageStats->m_vpPlayedSongs.size()
						 && i < MAX_SONGS_TO_SHOW; i++ )
			{
				Song *pSong = m_pStageStats->m_vpPlayedSongs[i];

				m_SmallBanner[i].LoadFromSong( pSong );
				m_SmallBanner[i].ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
				m_SmallBanner[i].SetName( ssprintf("SmallBanner%d",i+1) );
				ActorUtil::LoadAllCommands( m_SmallBanner[i], m_sName );
				SET_XY( m_SmallBanner[i] );
				this->AddChild( &m_SmallBanner[i] );

				m_sprSmallBannerFrame[i].Load( THEME->GetPathG(m_sName,"BannerFrame") );
				m_sprSmallBannerFrame[i]->SetName( ssprintf("SmallBanner%d",i+1) );
				ActorUtil::LoadAllCommands( *m_sprSmallBannerFrame[i], m_sName );
				SET_XY( m_sprSmallBannerFrame[i] );
				this->AddChild( m_sprSmallBannerFrame[i] );
			}
		}
		else
		{
			if( GAMESTATE->IsCourseMode() )
				m_LargeBanner.LoadFromCourse( GAMESTATE->m_pCurCourse );
			else
				m_LargeBanner.LoadFromSong( GAMESTATE->m_pCurSong );
			m_LargeBanner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
			m_LargeBanner.SetName( "LargeBanner" );
			ActorUtil::LoadAllCommands( m_LargeBanner, m_sName );
			SET_XY( m_LargeBanner );
			this->AddChild( &m_LargeBanner );

			m_sprLargeBannerFrame.Load( THEME->GetPathG(m_sName,"BannerFrame") );
			m_sprLargeBannerFrame->SetName( "LargeBannerFrame" );
			ActorUtil::LoadAllCommands( *m_sprLargeBannerFrame, m_sName );
			SET_XY( m_sprLargeBannerFrame );
			this->AddChild( m_sprLargeBannerFrame );
		}
	}

	{
		if( !SUMMARY )
		{
			FOREACH_EnabledPlayer( p )
			{
				m_textPlayerOptions[p].LoadFromFont( THEME->GetPathF(m_sName,"PlayerOptions") );
				m_textPlayerOptions[p].SetName( ssprintf("PlayerOptionsP%d",p+1) );
				ActorUtil::LoadAllCommands( m_textPlayerOptions[p], m_sName );
				SET_XY( m_textPlayerOptions[p] );
				vector<RString> v;
				PlayerOptions po = GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.GetPreferred();
				if( PLAYER_OPTIONS_HIDE_FAIL_TYPE )
					po.m_FailType = (FailType)0;	// blank out the fail type so that it won't show in the mods list
				po.GetLocalizedMods( v );
				RString sPO = join( PLAYER_OPTIONS_SEPARATOR, v );
				m_textPlayerOptions[p].SetText( sPO );
				this->AddChild( &m_textPlayerOptions[p] );
			}

			{
				m_textSongOptions.LoadFromFont( THEME->GetPathF(m_sName,"SongOptions") );
				m_textSongOptions.SetName( "SongOptions" );
				ActorUtil::LoadAllCommands( m_textSongOptions, m_sName );
				SET_XY( m_textSongOptions );
				m_textSongOptions.SetText( GAMESTATE->m_SongOptions.GetStage().GetLocalizedString() );
				this->AddChild( &m_textSongOptions );
			}
		}

		// Dairy Queen'd (disqualified)
		FOREACH_EnabledPlayer( p )
		{
			m_sprDisqualified[p].Load( THEME->GetPathG(m_sName,"Disqualified") );
			m_sprDisqualified[p]->SetName( ssprintf("DisqualifiedP%d",p+1) );
			LOAD_ALL_COMMANDS_AND_SET_XY( m_sprDisqualified[p] );
			m_sprDisqualified[p]->SetVisible( m_pStageStats->m_player[p].m_bDisqualified );
			this->AddChild( m_sprDisqualified[p] );
		}
	}

	// init grade area
	if( SHOW_GRADE_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprGradeFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("GradeFrame p%d",p+1)) );
			m_sprGradeFrame[p]->SetName( ssprintf("GradeFrameP%d",p+1) );
			ActorUtil::LoadAllCommands( *m_sprGradeFrame[p], m_sName );
			SET_XY( m_sprGradeFrame[p] );
			this->AddChild( m_sprGradeFrame[p] );

			// TODO: Re-add scrolling grade functionality
			m_Grades[p].Load( "GradeDisplayEval" );
			m_Grades[p].SetGrade( grade[p] );
			m_Grades[p].SetName( ssprintf("GradeP%d",p+1) );
			ActorUtil::LoadAllCommands( m_Grades[p], m_sName );
			SET_XY( m_Grades[p] );
			this->AddChild( &m_Grades[p] );
		}
	}

	// init points area
	if( SHOW_POINTS_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprPercentFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("PercentFrame p%d",p+1)) );
			m_sprPercentFrame[p]->SetName( ssprintf("PercentFrameP%d",p+1) );
			ActorUtil::LoadAllCommands( *m_sprPercentFrame[p], m_sName );
			SET_XY( m_sprPercentFrame[p] );
			this->AddChild( m_sprPercentFrame[p] );

			/* Use "ScreenEvaluation Percent" in the [metrics], but position and
			 * tween it with "PercentP1X", etc. */
			m_Percent[p].SetName( ssprintf("PercentP%d",p+1) );
			m_Percent[p].Load( GAMESTATE->m_pPlayerState[p], &m_pStageStats->m_player[p], "ScreenEvaluation Percent", true );
			ActorUtil::LoadAllCommands( m_Percent[p], m_sName );
			SET_XY( m_Percent[p] );
			this->AddChild( &m_Percent[p] );
		}
	}

	// init bonus area
	if( SHOW_BONUS_AREA )
	{
		// In course mode, we need to make sure the bar doesn't overflow. -aj
		float fDivider = 1.0f;
		if( GAMESTATE->IsCourseMode() )
			fDivider = fDivider / GAMESTATE->m_pCurCourse->m_vEntries.size();

		FOREACH_EnabledPlayer( p )
		{
			m_sprBonusFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("BonusFrame p%d",p+1)) );
			m_sprBonusFrame[p]->SetName( ssprintf("BonusFrameP%d",p+1) );
			ActorUtil::LoadAllCommands( *m_sprBonusFrame[p], m_sName );
			SET_XY( m_sprBonusFrame[p] );
			this->AddChild( m_sprBonusFrame[p] );

			// todo: convert this to use category names instead of numbers? -aj
			for( int r=0; r<NUM_SHOWN_RADAR_CATEGORIES; r++ )	// foreach line
			{
				float possible= m_pStageStats->m_player[p].m_radarPossible[r];
				float actual= m_pStageStats->m_player[p].m_radarActual[r];
				if(possible > 1.0)
				{
					actual /= possible;
					possible /= possible;
				}
				m_sprPossibleBar[p][r].Load( THEME->GetPathG(m_sName,ssprintf("BarPossible p%d",p+1)) );
				m_sprPossibleBar[p][r].SetWidth( m_sprPossibleBar[p][r].GetUnzoomedWidth() * possible * fDivider );
				m_sprPossibleBar[p][r].SetName( ssprintf("BarPossible%dP%d",r+1,p+1) );
				ActorUtil::LoadAllCommands( m_sprPossibleBar[p][r], m_sName );
				SET_XY( m_sprPossibleBar[p][r] );
				this->AddChild( &m_sprPossibleBar[p][r] );

				m_sprActualBar[p][r].Load( THEME->GetPathG(m_sName,ssprintf("BarActual p%d",p+1)) );
				// should be out of the possible bar, not actual (whatever value that is at)
				m_sprActualBar[p][r].SetWidth( m_sprPossibleBar[p][r].GetUnzoomedWidth() * actual * fDivider );

				float value = (float)100 * m_sprActualBar[p][r].GetUnzoomedWidth() / m_sprPossibleBar[p][r].GetUnzoomedWidth();
				LOG->Trace("Radar bar %d of 5 - %f percent", r,  value);

				m_sprActualBar[p][r].SetName( ssprintf("BarActual%dP%d",r+1,p+1) );
				ActorUtil::LoadAllCommands( m_sprActualBar[p][r], m_sName );
				SET_XY( m_sprActualBar[p][r] );

				// .99999 is fairly close to 1.00, so we use that.
				// todo: allow extra commands for AAA/AAAA? -aj
				if( actual > 0.99999f )
					m_sprActualBar[p][r].RunCommands( BAR_ACTUAL_MAX_COMMAND );
				this->AddChild( &m_sprActualBar[p][r] );
			}
		}
	}

	// init survived area
	if( SHOW_SURVIVED_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprSurvivedFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("SurvivedFrame p%d",p+1)) );
			m_sprSurvivedFrame[p]->SetName( ssprintf("SurvivedFrameP%d",p+1) );
			ActorUtil::LoadAllCommands( *m_sprSurvivedFrame[p], m_sName );
			SET_XY( m_sprSurvivedFrame[p] );
			this->AddChild( m_sprSurvivedFrame[p] );

			m_textSurvivedNumber[p].LoadFromFont( THEME->GetPathF(m_sName, "SurvivedNumber") );
			// curewater: edited the "# stages cleared" text so it deducts one if you failed.
			// Should be accurate, but I'm not sure if its "standard" that (bool)true = 1.  (assumption)
			m_textSurvivedNumber[p].SetText( ssprintf("%02d", m_pStageStats->m_player[p].m_iSongsPassed) );
			m_textSurvivedNumber[p].SetName( ssprintf("SurvivedNumberP%d",p+1) );
			ActorUtil::LoadAllCommands( m_textSurvivedNumber[p], m_sName );
			SET_XY( m_textSurvivedNumber[p] );
			this->AddChild( &m_textSurvivedNumber[p] );
		}
	}

	// init win area
	if( SHOW_WIN_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprWinFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("win frame p%d",p+1)) );
			m_sprWinFrame[p]->SetName( ssprintf("WinFrameP%d",p+1) );
			ActorUtil::LoadAllCommands( *m_sprWinFrame[p], m_sName );
			SET_XY( m_sprWinFrame[p] );
			this->AddChild( m_sprWinFrame[p] );

			m_sprWin[p].Load( THEME->GetPathG(m_sName,ssprintf("win p%d 1x3",p+1)) );
			m_sprWin[p].StopAnimating();
			int iFrame = GAMESTATE->GetStageResult( p );
			m_sprWin[p].SetState( iFrame );
			m_sprWin[p].SetName( ssprintf("WinP%d",p+1) );
			ActorUtil::LoadAllCommands( m_sprWin[p], m_sName );
			SET_XY( m_sprWin[p] );
			this->AddChild( &m_sprWin[p] );
		}
	}

	// init judgment area
	ROLLING_NUMBERS_CLASS.Load( m_sName, "RollingNumbersClass" );
	ROLLING_NUMBERS_MAX_COMBO_CLASS.Load( m_sName, "RollingNumbersMaxComboClass" );
	FOREACH_ENUM( JudgmentLine, l )
	{
		if( l == JudgmentLine_W1  && !GAMESTATE->ShowW1() )
			continue; // skip

		if( SHOW_JUDGMENT_LINE(l) )
		{
			if( SHOW_SHARED_JUDGMENT_LINE_LABELS )
			{
				LuaThreadVariable var2( "JudgmentLine", LuaReference::Create(l) );
				m_sprSharedJudgmentLineLabels[l].Load( THEME->GetPathG(m_sName,"JudgmentLabel " + JudgmentLineToString(l)) );
				m_sprSharedJudgmentLineLabels[l]->SetName( JudgmentLineToString(l)+"Label" );
				ActorUtil::LoadAllCommands( m_sprSharedJudgmentLineLabels[l], m_sName );
				SET_XY( m_sprSharedJudgmentLineLabels[l] );
				this->AddChild( m_sprSharedJudgmentLineLabels[l] );
			}

			FOREACH_EnabledPlayer( p )
			{
				m_textJudgmentLineNumber[l][p].LoadFromFont( THEME->GetPathF(m_sName, "JudgmentLineNumber") );
				m_textJudgmentLineNumber[l][p].SetName( JudgmentLineToString(l)+ssprintf("NumberP%d",p+1) );
				if( JudgmentLineToString(l) == "MaxCombo" )
					m_textJudgmentLineNumber[l][p].Load( ROLLING_NUMBERS_MAX_COMBO_CLASS );
				else
					m_textJudgmentLineNumber[l][p].Load( ROLLING_NUMBERS_CLASS );
				ActorUtil::LoadAllCommands( m_textJudgmentLineNumber[l][p], m_sName );
				SET_XY( m_textJudgmentLineNumber[l][p] );
				this->AddChild( &m_textJudgmentLineNumber[l][p] );

				int iValue;
				switch( l )
				{
				/* xxx: This doesn't seem to handle checkpoints correctly.
				 * Something about checkpoints needing to be tied into
				 * the correct judgments instead of just W1/W2. Misses are ok. */
				case JudgmentLine_W1:
					iValue = m_pStageStats->m_player[p].m_iTapNoteScores[TNS_W1];
					if( CHECKPOINTS_WITH_JUDGMENTS && GAMESTATE->ShowW1() )
					{
						iValue += m_pStageStats->m_player[p].m_iTapNoteScores[TNS_CheckpointHit];
					}
					break;
				case JudgmentLine_W2:
					iValue = m_pStageStats->m_player[p].m_iTapNoteScores[TNS_W2];
					if( CHECKPOINTS_WITH_JUDGMENTS && !GAMESTATE->ShowW1() )
					{
						iValue += m_pStageStats->m_player[p].m_iTapNoteScores[TNS_CheckpointHit];
					}
					break;
				case JudgmentLine_W3:		iValue = m_pStageStats->m_player[p].m_iTapNoteScores[TNS_W3];	break;
				case JudgmentLine_W4:		iValue = m_pStageStats->m_player[p].m_iTapNoteScores[TNS_W4];	break;
				case JudgmentLine_W5:		iValue = m_pStageStats->m_player[p].m_iTapNoteScores[TNS_W5];	break;
				case JudgmentLine_Miss:
					iValue = m_pStageStats->m_player[p].m_iTapNoteScores[TNS_Miss];
					if( CHECKPOINTS_WITH_JUDGMENTS )
					{
						iValue += m_pStageStats->m_player[p].m_iTapNoteScores[TNS_CheckpointMiss];
					}
					break;
				case JudgmentLine_Held:		iValue = m_pStageStats->m_player[p].m_iHoldNoteScores[HNS_Held];	break;
				case JudgmentLine_MaxCombo:	iValue = m_pStageStats->m_player[p].GetMaxCombo().m_cnt;		break;
				DEFAULT_FAIL( l );
				}

				m_textJudgmentLineNumber[l][p].SetTargetNumber( iValue );
			}
		}
	}

	// init detail area
	if( SHOW_DETAIL_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprDetailFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("DetailFrame p%d",p+1)) );
			m_sprDetailFrame[p]->SetName( ssprintf("DetailFrameP%d",p+1) );
			ActorUtil::LoadAllCommands( *m_sprDetailFrame[p], m_sName );
			SET_XY( m_sprDetailFrame[p] );
			this->AddChild( m_sprDetailFrame[p] );
		}

		FOREACH_ENUM( DetailLine, l )
		{
			FOREACH_EnabledPlayer( p )
			{
				m_textDetailText[l][p].LoadFromFont( THEME->GetPathF(m_sName,"DetailLineNumber") );
				m_textDetailText[l][p].SetName( DetailLineToString(l)+ssprintf("NumberP%d",p+1) );
				ActorUtil::LoadAllCommands( m_textDetailText[l][p], m_sName );
				SET_XY( m_textDetailText[l][p] );
				this->AddChild( &m_textDetailText[l][p] );

				static const int indices[NUM_DetailLine] =
				{
					RadarCategory_TapsAndHolds, RadarCategory_Jumps, RadarCategory_Holds, RadarCategory_Mines, 
					RadarCategory_Hands, RadarCategory_Rolls, RadarCategory_Lifts, RadarCategory_Fakes
				};
				const int ind = indices[l];
				const int iActual = lrintf(m_pStageStats->m_player[p].m_radarActual[ind]);
				const int iPossible = lrintf(m_pStageStats->m_player[p].m_radarPossible[ind]);

				// todo: check if format string is valid
				// (two integer values in DETAILLINE_FORMAT) -aj
				m_textDetailText[l][p].SetText( ssprintf(DETAILLINE_FORMAT,iActual,iPossible) );
			}
		}
	}

	// init score area
	if( SHOW_SCORE_AREA )
	{
		m_sprScoreLabel.Load( THEME->GetPathG(m_sName,"ScoreLabel") );
		m_sprScoreLabel->SetName( "ScoreLabel" );
		ActorUtil::LoadAllCommands( *m_sprScoreLabel, m_sName );
		SET_XY( m_sprScoreLabel );
		this->AddChild( m_sprScoreLabel );

		FOREACH_EnabledPlayer( p )
		{
			m_textScore[p].LoadFromFont( THEME->GetPathF(m_sName, "ScoreNumber") );
			m_textScore[p].SetName( ssprintf("ScoreNumberP%d",p+1) );
			m_textScore[p].Load( "RollingNumbersEvaluation" );
			ActorUtil::LoadAllCommands( m_textScore[p], m_sName );
			SET_XY( m_textScore[p] );
			m_textScore[p].SetTargetNumber( m_pStageStats->m_player[p].m_iScore );
			this->AddChild( &m_textScore[p] );
		}
	}

	// init time area
	if( SHOW_TIME_AREA )
	{
		m_sprTimeLabel.Load( THEME->GetPathG(m_sName,"TimeLabel") );
		m_sprTimeLabel->SetName( "TimeLabel" );
		ActorUtil::LoadAllCommands( *m_sprTimeLabel, m_sName );
		SET_XY( m_sprTimeLabel );
		this->AddChild( m_sprTimeLabel );

		FOREACH_EnabledPlayer( p )
		{
			m_textTime[p].LoadFromFont( THEME->GetPathF(m_sName, "time") );
			m_textTime[p].SetShadowLength( 0 );
			m_textTime[p].SetName( ssprintf("TimeNumberP%d",p+1) );
			ActorUtil::LoadAllCommands( m_textTime[p], m_sName );
			SET_XY( m_textTime[p] );
			m_textTime[p].SetText( SecondsToMMSSMsMs(m_pStageStats->m_player[p].m_fAliveSeconds) );
			this->AddChild( &m_textTime[p] );
		}
	}

	// init records area
	bool bOneHasNewTopRecord = false;
	bool bOneHasFullW1Combo = false;
	bool bOneHasFullW2Combo = false;
	bool bOneHasFullW3Combo = false;
	bool bOneHasFullW4Combo = false;
	
	FOREACH_PlayerNumber( p )
	{
		if(GAMESTATE->IsPlayerEnabled(p))
		{
			if( (m_pStageStats->m_player[p].m_iMachineHighScoreIndex == 0 ||
				 m_pStageStats->m_player[p].m_iPersonalHighScoreIndex == 0) )
			{
				bOneHasNewTopRecord = true;
			}

			if( m_pStageStats->m_player[p].FullComboOfScore(TNS_W4) )
				bOneHasFullW4Combo = true;

			if( m_pStageStats->m_player[p].FullComboOfScore(TNS_W3) )
				bOneHasFullW3Combo = true;

			if( m_pStageStats->m_player[p].FullComboOfScore(TNS_W2) )
				bOneHasFullW2Combo = true;

			if( m_pStageStats->m_player[p].FullComboOfScore(TNS_W1) )
				bOneHasFullW1Combo = true;
		}
	}

	Grade best_grade = Grade_NoData;
	FOREACH_PlayerNumber( p )
		best_grade = min( best_grade, grade[p] ); 

	if( m_pStageStats->m_EarnedExtraStage != EarnedExtraStage_No )
	{
		SOUND->PlayOnce( THEME->GetPathS(m_sName,"try " + EarnedExtraStageToString(m_pStageStats->m_EarnedExtraStage)) );
	}
	else if( bOneHasNewTopRecord && ANNOUNCER->HasSoundsFor("evaluation new record") )
	{
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation new record") );
	}
	else if( bOneHasFullW4Combo && ANNOUNCER->HasSoundsFor("evaluation full combo W4") )
	{
		SOUND->PlayOnceFromDir(ANNOUNCER->GetPathTo("evaluation full combo W4"));
	}
	else if( (bOneHasFullW1Combo || bOneHasFullW2Combo || bOneHasFullW3Combo) )
	{
		RString sComboType = bOneHasFullW1Combo ? "W1" : ( bOneHasFullW2Combo ? "W2" : "W3" );
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation full combo "+sComboType) );
	}
	else
	{
		if( SUMMARY || GAMESTATE->IsCourseMode() )
		{
			SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final "+GradeToOldString(best_grade)) );
		}
		else
		{
			switch( GAMESTATE->m_PlayMode )
			{
			case PLAY_MODE_BATTLE:
				{
					bool bWon = GAMESTATE->GetStageResult(GAMESTATE->GetMasterPlayerNumber()) == RESULT_WIN;
					RString sResult = bWon ? "win" : "lose";
					SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation "+sResult) );
				}
				break;
			default:
				SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation "+GradeToOldString(best_grade)) );
				break;
			}
		}
	}

	switch( best_grade )
	{
		case Grade_Tier01:
		case Grade_Tier02:
		case Grade_Tier03:
			this->PostScreenMessage( SM_PlayCheer, CHEER_DELAY_SECONDS );
		default:
			break;
	}
}

bool ScreenEvaluation::Input( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return false;

	if( input.GameI.IsValid() )
	{
		if( CodeDetector::EnteredCode(input.GameI.controller, CODE_SAVE_SCREENSHOT1) ||
			CodeDetector::EnteredCode(input.GameI.controller, CODE_SAVE_SCREENSHOT2) )
		{
			PlayerNumber pn = input.pn;
			if( !m_bSavedScreenshot[pn]  &&	// only allow one screenshot
				PROFILEMAN->IsPersistentProfile(pn) )
			{
				if( PROFILEMAN->ProfileWasLoadedFromMemoryCard(pn) )
					MEMCARDMAN->MountCard( pn );

				Profile* pProfile = PROFILEMAN->GetProfile(pn);
				RString sDir = PROFILEMAN->GetProfileDir((ProfileSlot)pn) + "Screenshots/";
				RString sFileName = StepMania::SaveScreenshot( sDir, true, true, "", "" );

				if( !sFileName.empty() )
				{
					RString sPath = sDir+sFileName;

					const HighScore &hs = m_pStageStats->m_player[pn].m_HighScore;
					Screenshot screenshot;
					screenshot.sFileName = sFileName;
					screenshot.sMD5 = BinaryToHex( CRYPTMAN->GetMD5ForFile(sPath) );
					screenshot.highScore = hs;
					pProfile->AddScreenshot( screenshot );
				}

				if( PROFILEMAN->ProfileWasLoadedFromMemoryCard(pn) )
					MEMCARDMAN->UnmountCard( pn );

				m_bSavedScreenshot[pn] = true;
				return true;	// handled
			}
		}
	}

	return ScreenWithMenuElements::Input( input );
}

void ScreenEvaluation::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_PlayCheer )
	{
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation cheer") );
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

bool ScreenEvaluation::MenuBack( const InputEventPlus &input )
{
	return MenuStart( input );
}

bool ScreenEvaluation::MenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return false;

	m_soundStart.Play(true);

	HandleMenuStart();
	return true;
}

void ScreenEvaluation::HandleMenuStart()
{
	StartTransitioningScreen( SM_GoToNextScreen );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ScreenEvaluation. */ 
class LunaScreenEvaluation: public Luna<ScreenEvaluation>
{
public:
	static int GetStageStats( T* p, lua_State *L ) { LuaHelpers::Push( L, p->GetStageStats() ); return 1; }
	LunaScreenEvaluation()
	{
		ADD_METHOD( GetStageStats );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenEvaluation, ScreenWithMenuElements )

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
