#include "global.h"
#include "Player.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "RageTimer.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "InputMapper.h"
#include "SongManager.h"
#include "GameState.h"
#include "ScoreKeeperNormal.h"
#include "RageLog.h"
#include "RageDisplay.h"
#include "ThemeManager.h"
#include "ScoreDisplay.h"
#include "LifeMeter.h"
#include "CombinedLifeMeter.h"
#include "PlayerAI.h"
#include "NoteField.h"
#include "NoteDataUtil.h"
#include "ScreenMessage.h"
#include "ScreenManager.h"
#include "StageStats.h"
#include "ActorUtil.h"
#include "ArrowEffects.h"
#include "Game.h"
#include "NetworkSyncManager.h"	//used for sending timing offset
#include "DancingCharacters.h"
#include "ScreenDimensions.h"
#include "RageSoundManager.h"
#include "ThemeMetric.h"
#include "PlayerState.h"
#include "GameSoundManager.h"
#include "Style.h"
#include "MessageManager.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "StatsManager.h"
#include "song.h"
#include "Steps.h"
#include "GameCommand.h"
#include "LocalizedString.h"
#include "AdjustSync.h"

// Helper class to ensure that each row is only judged once without taking too much memory.
class JudgedRows
{
	char	*m_pRows;
	int 	m_iStart;
	int	m_iOffset;
	int	m_iLen;
	
	void Resize( int iMin )
	{
		char *p = m_pRows;
		int newSize = max( m_iLen*2, iMin );
		m_pRows = new char[newSize];
		int i = 0;
		if( p )
		{
			for( ; i < m_iLen; ++i )
				m_pRows[i] = p[(i+m_iOffset)%m_iLen];
			delete[] p;
		}
		m_iOffset = 0;
		m_iLen = newSize;
		memset( m_pRows + i, 0, newSize - i );
	}
public:
	JudgedRows() : m_pRows(NULL), m_iStart(0), m_iOffset(0), m_iLen(0) { Resize( 32 ); }
	~JudgedRows() { delete[] m_pRows; }
	// Returns true if the row has already been judged.
	bool JudgeRow( int iRow )
	{
		if( iRow < m_iStart )
			return true;
		if( iRow >= m_iStart+m_iLen )
			Resize( iRow+1-m_iStart );
		const bool ret = m_pRows[(iRow-m_iStart+m_iOffset)%m_iLen] != 0;
		m_pRows[(iRow-m_iStart+m_iOffset)%m_iLen] = 1;
		while( m_pRows[m_iOffset] )
		{
			m_pRows[m_iOffset] = 0;
			++m_iStart;
			if( ++m_iOffset >= m_iLen )
				m_iOffset -= m_iLen;
		}
		return ret;
	}
	void Reset( int iStart )
	{
		m_iStart = iStart;
		m_iOffset = 0;
		memset( m_pRows, 0, m_iLen );
	}
};


RString ATTACK_DISPLAY_X_NAME( size_t p, size_t both_sides )	{ return "AttackDisplayXOffset" + (both_sides ? RString("BothSides") : ssprintf("OneSideP%d",int(p+1)) ); }

/* Distance to search for a note in Step(), in seconds. */
// TODO: This should be calculated based on the max size of the current judgment windows.
static const float StepSearchDistance = 1.0f;

void TimingWindowSecondsInit( size_t /*TimingWindow*/ i, RString &sNameOut, float &defaultValueOut )
{
	sNameOut = "TimingWindowSeconds" + TimingWindowToString( (TimingWindow)i );
	switch( i )
	{
	default:	ASSERT(0);
	case TW_W1:	defaultValueOut = 0.0225f;	break;
	case TW_W2:	defaultValueOut = 0.045f;	break;
	case TW_W3:	defaultValueOut = 0.090f;	break;
	case TW_W4:	defaultValueOut = 0.135f;	break;
	case TW_W5:	defaultValueOut = 0.180f;	break;
	case TW_Mine:	defaultValueOut = 0.090f;	break;	// same as great
	case TW_Hold:	defaultValueOut = 0.250f;	break;	// allow enough time to take foot off and put back on
	case TW_Roll:	defaultValueOut = 0.350f;	break;
	case TW_Attack:	defaultValueOut = 0.135f;	break;
	}
}

static Preference<float> m_fTimingWindowScale	( "TimingWindowScale",		1.0f );
static Preference<float> m_fTimingWindowAdd	( "TimingWindowAdd",		0 );
static Preference1D<float> m_fTimingWindowSeconds( TimingWindowSecondsInit, NUM_TimingWindow );
static Preference<float> m_fTimingWindowJump	( "TimingWindowJump",		0.25 );
Preference<float> g_fTimingWindowHopo		( "TimingWindowHopo",		0.25 );		// max time between notes in a hopo chain
Preference<float> g_fTimingWindowStrum		( "TimingWindowStrum",		0.1f );		// max time between strum and when the frets must match
ThemeMetric<float> INITIAL_HOLD_LIFE		( "Player", "InitialHoldLife" );
ThemeMetric<bool> PENALIZE_TAP_SCORE_NONE	( "Player", "PenalizeTapScoreNone" );
ThemeMetric<bool> JUDGE_HOLD_NOTES_ON_SAME_ROW_TOGETHER	( "Player", "JudgeHoldNotesOnSameRowTogether" );
ThemeMetric<bool> HOLD_CHECKPOINTS	( "Player", "HoldCheckpoints" );
ThemeMetric<bool> IMMEDIATE_HOLD_LET_GO	( "Player", "ImmediateHoldLetGo" );
ThemeMetric<bool> REQUIRE_STEP_ON_HOLD_HEADS	( "Player", "RequireStepOnHoldHeads" );


float Player::GetWindowSeconds( TimingWindow tw )
{
	float fSecs = m_fTimingWindowSeconds[tw];
	fSecs *= m_fTimingWindowScale;
	fSecs += m_fTimingWindowAdd;
	return fSecs;
}

Player::Player( NoteData &nd, bool bVisibleParts ) : m_NoteData(nd)
{
	m_bLoaded = false;

	m_pPlayerState = NULL;
	m_pPlayerStageStats = NULL;
	m_fNoteFieldHeight = 0;

	m_pLifeMeter = NULL;
	m_pCombinedLifeMeter = NULL;
	m_pScoreDisplay = NULL;
	m_pSecondaryScoreDisplay = NULL;
	m_pPrimaryScoreKeeper = NULL;
	m_pSecondaryScoreKeeper = NULL;
	m_pInventory = NULL;
	m_pIterNeedsTapJudging = NULL;
	m_pIterNeedsHoldJudging = NULL;
	m_pIterUncrossedRows = NULL;
	m_pIterUnjudgedRows = NULL;
	m_pIterUnjudgedMineRows = NULL;

	m_bPaused = false;

	m_pAttackDisplay = NULL;
	if( bVisibleParts )
	{
		m_pAttackDisplay = new AttackDisplay;
		this->AddChild( m_pAttackDisplay );
	}

	PlayerAI::InitFromDisk();

	m_pNoteField = NULL;
	if( bVisibleParts )
	{
		m_pNoteField = new NoteField;
		m_pNoteField->SetName( "NoteField" );
	}
	m_pJudgedRows = new JudgedRows;
}

Player::~Player()
{
	SAFE_DELETE( m_pAttackDisplay );
	SAFE_DELETE( m_pNoteField );
	for( unsigned i = 0; i < m_vpHoldJudgment.size(); ++i )
		SAFE_DELETE( m_vpHoldJudgment[i] );
	SAFE_DELETE( m_pJudgedRows );
	SAFE_DELETE( m_pIterNeedsTapJudging );
	SAFE_DELETE( m_pIterNeedsHoldJudging );
	SAFE_DELETE( m_pIterUncrossedRows );
	SAFE_DELETE( m_pIterUnjudgedRows );
	SAFE_DELETE( m_pIterUnjudgedMineRows );
}

/* Init() does the expensive stuff: load sounds and note skins.  Load() just loads a NoteData. */
void Player::Init(
	const RString &sType,
	PlayerState* pPlayerState, 
	PlayerStageStats* pPlayerStageStats,
	LifeMeter* pLM, 
	CombinedLifeMeter* pCombinedLM, 
	ScoreDisplay* pScoreDisplay, 
	ScoreDisplay* pSecondaryScoreDisplay, 
	Inventory* pInventory, 
	ScoreKeeper* pPrimaryScoreKeeper, 
	ScoreKeeper* pSecondaryScoreKeeper )
{
	GRAY_ARROWS_Y_STANDARD.Load(			sType, "ReceptorArrowsYStandard" );
	GRAY_ARROWS_Y_REVERSE.Load(			sType, "ReceptorArrowsYReverse" );
	ATTACK_DISPLAY_X.Load(				sType, ATTACK_DISPLAY_X_NAME, NUM_PLAYERS, 2 );
	ATTACK_DISPLAY_Y.Load(				sType, "AttackDisplayY" );
	ATTACK_DISPLAY_Y_REVERSE.Load(			sType, "AttackDisplayYReverse" );
	HOLD_JUDGMENT_Y_STANDARD.Load(			sType, "HoldJudgmentYStandard" );
	HOLD_JUDGMENT_Y_REVERSE.Load(			sType, "HoldJudgmentYReverse" );
	BRIGHT_GHOST_COMBO_THRESHOLD.Load(		sType, "BrightGhostComboThreshold" );
	TAP_JUDGMENTS_UNDER_FIELD.Load(			sType, "TapJudgmentsUnderField" );
	HOLD_JUDGMENTS_UNDER_FIELD.Load(		sType, "HoldJudgmentsUnderField" );
	DRAW_DISTANCE_AFTER_TARGET_PIXELS.Load(		sType, "DrawDistanceAfterTargetsPixels" );
	DRAW_DISTANCE_BEFORE_TARGET_PIXELS.Load(	sType, "DrawDistanceBeforeTargetsPixels" );

	{
		//
		// Init judgment positions
		//
		bool bPlayerUsingBothSides = GAMESTATE->GetCurrentStyle()->m_StyleType==StyleType_OnePlayerTwoSides;
		Actor TempJudgment;
		TempJudgment.SetName( "Judgment" );
		ActorUtil::LoadCommand( TempJudgment, sType, "Transform" );

		Actor TempCombo;
		TempCombo.SetName( "Combo" );
		ActorUtil::LoadCommand( TempCombo, sType, "Transform" );

		int iEnabledPlayerIndex = -1;
		int iNumEnabledPlayers = 0;
		if( GAMESTATE->m_bMultiplayer )
		{
			FOREACH_EnabledMultiPlayer( p )
			{
				if( p == pPlayerState->m_mp )
					iEnabledPlayerIndex = iNumEnabledPlayers;
				iNumEnabledPlayers++;
			}
		}
		else
		{
			FOREACH_EnabledPlayer( p )
			{
				if( p == pPlayerState->m_PlayerNumber )
					iEnabledPlayerIndex = iNumEnabledPlayers;
				iNumEnabledPlayers++;
			}
		}

		if( iNumEnabledPlayers == 0 )	// hack for ScreenHowToPlay where no players are joined
		{
			iEnabledPlayerIndex = 0;
			iNumEnabledPlayers = 1;
		}

		for( int i=0; i<NUM_REVERSE; i++ )
		{
			for( int j=0; j<NUM_CENTERED; j++ )
			{
				Message msg( "Transform" );
				msg.SetParam( "Player", pPlayerState->m_PlayerNumber );
				msg.SetParam( "MultiPlayer", pPlayerState->m_mp );
				msg.SetParam( "iEnabledPlayerIndex", iEnabledPlayerIndex );
				msg.SetParam( "iNumEnabledPlayers", iNumEnabledPlayers );
				msg.SetParam( "bPlayerUsingBothSides", bPlayerUsingBothSides );
				msg.SetParam( "bReverse", !!i );
				msg.SetParam( "bCentered", !!j );

				TempJudgment.HandleMessage( msg );
				m_tsJudgment[i][j] = TempJudgment.DestTweenState();

				TempCombo.HandleMessage( msg );
				m_tsCombo[i][j] = TempCombo.DestTweenState();
			}
		}
	}

	this->SortByDrawOrder();

	m_pPlayerState = pPlayerState;
	m_pPlayerStageStats = pPlayerStageStats;
	m_pLifeMeter = pLM;
	m_pCombinedLifeMeter = pCombinedLM;
	m_pScoreDisplay = pScoreDisplay;
	m_pSecondaryScoreDisplay = pSecondaryScoreDisplay;
	m_pInventory = pInventory;
	m_pPrimaryScoreKeeper = pPrimaryScoreKeeper;
	m_pSecondaryScoreKeeper = pSecondaryScoreKeeper;

	m_iLastSeenCombo      = -1;
	
	// set initial life
	if( m_pLifeMeter && m_pPlayerStageStats )
	{
		float fLife = m_pLifeMeter->GetLife();
		m_pPlayerStageStats->SetLifeRecordAt( fLife, STATSMAN->m_CurStageStats.m_fStepsSeconds );
	}


	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;


	RageSoundLoadParams SoundParams;
	SoundParams.m_bSupportPan = true;
	m_soundMine.Load( THEME->GetPathS(sType,"mine"), true, &SoundParams );

	/* Attacks can be launched in course modes and in battle modes.  They both come
	 * here to play, but allow loading a different sound for different modes. */
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_RAVE:
	case PLAY_MODE_BATTLE:
		m_soundAttackLaunch.Load( THEME->GetPathS(sType,"battle attack launch"), true, &SoundParams );
		m_soundAttackEnding.Load( THEME->GetPathS(sType,"battle attack ending"), true, &SoundParams );
		break;
	default:
		m_soundAttackLaunch.Load( THEME->GetPathS(sType,"course attack launch"), true, &SoundParams );
		m_soundAttackEnding.Load( THEME->GetPathS(sType,"course attack ending"), true, &SoundParams );
		break;
	}


	float fBalance = GameSoundManager::GetPlayerBalance( pn );
	m_soundMine.SetProperty( "Pan", fBalance );
	m_soundAttackLaunch.SetProperty( "Pan", fBalance );
	m_soundAttackEnding.SetProperty( "Pan", fBalance );


	if( HasVisibleParts() )
	{
		LuaThreadVariable var( "Player", LuaReference::Create(m_pPlayerState->m_PlayerNumber) );
		LuaThreadVariable var2( "MultiPlayer", LuaReference::Create(m_pPlayerState->m_mp) );

		m_sprCombo.Load( THEME->GetPathG(sType,"combo") );
		m_sprCombo->SetName( "Combo" );
		m_pActorWithComboPosition = &*m_sprCombo;
		this->AddChild( m_sprCombo );

		m_sprJudgment.Load( THEME->GetPathG(sType,"judgment") );
		m_sprJudgment->SetName( "Judgment" );
		m_pActorWithJudgmentPosition = &*m_sprJudgment;
		this->AddChild( m_sprJudgment );
	}

	// Load HoldJudgments
	m_vpHoldJudgment.resize( GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer );
	for( int i = 0; i < GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer; ++i )
		m_vpHoldJudgment[i] = NULL;

	if( HasVisibleParts() )
	{
		for( int i = 0; i < GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer; ++i )
		{
			HoldJudgment *pJudgment = new HoldJudgment;
			pJudgment->Load( THEME->GetPathG("HoldJudgment","label 1x2") );
			m_vpHoldJudgment[i] = pJudgment;
			this->AddChild( m_vpHoldJudgment[i] );
		}
	}

	m_fNoteFieldHeight = GRAY_ARROWS_Y_REVERSE-GRAY_ARROWS_Y_STANDARD;
	if( m_pNoteField )
	{
		m_pNoteField->Init( m_pPlayerState, m_fNoteFieldHeight );
		ActorUtil::LoadAllCommands( *m_pNoteField, sType );
	}

	m_vbFretIsDown.resize( GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer );
	FOREACH( bool, m_vbFretIsDown, b )
		*b = false;
}

static bool NeedsTapJudging( const TapNote &tn )
{
	switch( tn.type )
	{
	DEFAULT_FAIL( tn.type );
	case TapNote::tap:
	case TapNote::hold_head:
	case TapNote::mine:
	case TapNote::lift:
		if( tn.result.tns == TNS_None )
			return true;
		else
			return false;
	case TapNote::hold_tail:
	case TapNote::attack:
		return false;
	}
}

static bool NeedsHoldJudging( const TapNote &tn )
{
	switch( tn.type )
	{
	DEFAULT_FAIL( tn.type );
	case TapNote::hold_head:
		if( tn.HoldResult.hns == HNS_None )
			return true;
		else
			return false;
	case TapNote::tap:
	case TapNote::hold_tail:
	case TapNote::mine:
	case TapNote::lift:
	case TapNote::attack:
		return false;
	}
}

void Player::Load()
{
	m_bLoaded = true;

	m_LastTapNoteScore = TNS_None;
	// The editor can start playing in the middle of the song.
	const int iNoteRow = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat );
	m_iFirstUncrossedRow     = iNoteRow - 1;
	m_pJudgedRows->Reset( iNoteRow );

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	bool bOniDead = GAMESTATE->m_SongOptions.GetStage().m_LifeType == SongOptions::LIFE_BATTERY  &&  
		(m_pPlayerStageStats == NULL || m_pPlayerStageStats->m_bFailed);

	/* The editor reuses Players ... so we really need to make sure everything
	 * is reset and not tweening.  Perhaps ActorFrame should recurse to subactors;
	 * then we could just this->StopTweening()? -glenn */
	if( m_sprJudgment )
		m_sprJudgment->PlayCommand("Reset");
	if( m_pPlayerStageStats )
	{
		SetCombo( m_pPlayerStageStats->m_iCurCombo, m_pPlayerStageStats->m_iCurMissCombo );	// combo can persist between songs and games
	}
	if( m_pAttackDisplay )
		m_pAttackDisplay->Init( m_pPlayerState );

	/* Don't re-init this; that'll reload graphics.  Add a separate Reset() call
	 * if some ScoreDisplays need it. */
//	if( m_pScore )
//		m_pScore->Init( pn );

	/* Apply transforms. */
	NoteDataUtil::TransformNoteData( m_NoteData, m_pPlayerState->m_PlayerOptions.GetStage(), GAMESTATE->GetCurrentStyle()->m_StepsType );

	const Song* pSong = GAMESTATE->m_pCurSong;
	if( GAMESTATE->m_pCurGame->m_bAllowHopos )
		NoteDataUtil::SetHopoPossibleFlags( pSong, m_NoteData );
	
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_RAVE:
	case PLAY_MODE_BATTLE:
		{
			// ugly, ugly, ugly.  Works only w/ dance.
			// Why does this work only with dance? - Steve
			StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
			NoteDataUtil::TransformNoteData( m_NoteData, m_pPlayerState->m_PlayerOptions.GetStage(), st );
			
			// shuffle either p1 or p2
			static int count = 0;
			switch( count )
			{
			case 0:
			case 3:
				NoteDataUtil::Turn( m_NoteData, st, NoteDataUtil::left);
				break;
			case 1:
			case 2:
				NoteDataUtil::Turn( m_NoteData, st, NoteDataUtil::right);
				break;
			default:
				ASSERT(0);
			}
			count++;
			count %= 4;
		}
		break;
	}

	int iDrawDistanceAfterTargetsPixels = GAMESTATE->IsEditing() ? -100 : DRAW_DISTANCE_AFTER_TARGET_PIXELS;
	int iDrawDistanceBeforeTargetsPixels = GAMESTATE->IsEditing() ? 400 : DRAW_DISTANCE_BEFORE_TARGET_PIXELS;

	float fNoteFieldMiddle = (GRAY_ARROWS_Y_STANDARD+GRAY_ARROWS_Y_REVERSE)/2;
	
	if( m_pNoteField && !bOniDead )
	{
		m_pNoteField->SetY( fNoteFieldMiddle );
		m_pNoteField->Load( &m_NoteData, iDrawDistanceAfterTargetsPixels, iDrawDistanceBeforeTargetsPixels );
	}

	bool bPlayerUsingBothSides = GAMESTATE->GetCurrentStyle()->m_StyleType==StyleType_OnePlayerTwoSides;
	if( m_pAttackDisplay )
		m_pAttackDisplay->SetX( ATTACK_DISPLAY_X.GetValue(pn, bPlayerUsingBothSides) - 40 );
	// set this in Update //m_pAttackDisplay->SetY( bReverse ? ATTACK_DISPLAY_Y_REVERSE : ATTACK_DISPLAY_Y );

	// set this in Update 
	//m_pJudgment->SetX( JUDGMENT_X.GetValue(pn,bPlayerUsingBothSides) );
	//m_pJudgment->SetY( bReverse ? JUDGMENT_Y_REVERSE : JUDGMENT_Y );

	// Need to set Y positions of all these elements in Update since
	// they change depending on PlayerOptions.


	//
	// Load keysounds.  If sounds are already loaded (as in the editor), don't reload them.
	// XXX: the editor will load several duplicate copies (in each NoteField), and each
	// player will load duplicate sounds.  Does this belong somewhere else (perhaps in
	// a separate object, used alongside ScreenGameplay::m_pSoundMusic and ScreenEdit::m_pSoundMusic?)
	// We don't have to load separate copies to set player fade: always make a copy, and set the
	// fade on the copy.
	//
	RString sSongDir = pSong->GetSongDir();
	m_vKeysounds.resize( pSong->m_vsKeysoundFile.size() );

	RageSoundLoadParams SoundParams;
	SoundParams.m_bSupportPan = true;

	float fBalance = GameSoundManager::GetPlayerBalance( pn );
	for( unsigned i=0; i<m_vKeysounds.size(); i++ )
	{
		RString sKeysoundFilePath = sSongDir + pSong->m_vsKeysoundFile[i];
		RageSound& sound = m_vKeysounds[i];
		if( sound.GetLoadedFilePath() != sKeysoundFilePath )
			sound.Load( sKeysoundFilePath, true, &SoundParams );
		sound.SetProperty( "Pan", fBalance );
	}

	if( m_pPlayerStageStats )
		SendComboMessages( m_pPlayerStageStats->m_iCurCombo, m_pPlayerStageStats->m_iCurMissCombo );

	SAFE_DELETE( m_pIterNeedsTapJudging );
	m_pIterNeedsTapJudging = new NoteData::all_tracks_iterator( m_NoteData.GetTapNoteRangeAllTracks(iNoteRow, MAX_NOTE_ROW) );

	SAFE_DELETE( m_pIterNeedsHoldJudging );
	m_pIterNeedsHoldJudging = new NoteData::all_tracks_iterator( m_NoteData.GetTapNoteRangeAllTracks(iNoteRow, MAX_NOTE_ROW ) );

	SAFE_DELETE( m_pIterUncrossedRows );
	m_pIterUncrossedRows = new NoteData::all_tracks_iterator( m_NoteData.GetTapNoteRangeAllTracks(iNoteRow, MAX_NOTE_ROW ) );

	SAFE_DELETE( m_pIterUnjudgedRows );
	m_pIterUnjudgedRows = new NoteData::all_tracks_iterator( m_NoteData.GetTapNoteRangeAllTracks(iNoteRow, MAX_NOTE_ROW ) );

	SAFE_DELETE( m_pIterUnjudgedMineRows );
	m_pIterUnjudgedMineRows = new NoteData::all_tracks_iterator( m_NoteData.GetTapNoteRangeAllTracks(iNoteRow, MAX_NOTE_ROW ) );
}

void Player::SendComboMessages( int iOldCombo, int iOldMissCombo )
{
	const int iCurCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurCombo : 0;
	if( iOldCombo > 50 && iCurCombo < 50 )
	{
		SCREENMAN->PostMessageToTopScreen( SM_ComboStopped, 0 );
	}


	Message msg( "ComboChanged" );
	msg.SetParam( "Player", m_pPlayerState->m_PlayerNumber );
	msg.SetParam( "OldCombo", iOldCombo );
	msg.SetParam( "OldMissCombo", iOldMissCombo );
	if( m_pPlayerState )
		msg.SetParam( "PlayerState", LuaReference::CreateFromPush(*m_pPlayerState) );
	if( m_pPlayerStageStats )
		msg.SetParam( "PlayerStageStats", LuaReference::CreateFromPush(*m_pPlayerStageStats) );
	MESSAGEMAN->Broadcast( msg );
}

void Player::Update( float fDeltaTime )
{
	const RageTimer now;
	// Don't update if we havn't been loaded yet.
	if( !m_bLoaded )
		return;

	//LOG->Trace( "Player::Update(%f)", fDeltaTime );

	if( GAMESTATE->m_pCurSong==NULL )
		return;

	ActorFrame::Update( fDeltaTime );

	const float fSongBeat = GAMESTATE->m_fSongBeat;
	const int iSongRow = BeatToNoteRow( fSongBeat );

	// Optimization: Don't spend time processing the things below that won't show 
	// if the Player doesn't show anything on the screen.
	if( HasVisibleParts() )
	{
		if( m_pPlayerState->m_bAttackBeganThisUpdate )
			m_soundAttackLaunch.Play();
		if( m_pPlayerState->m_bAttackEndedThisUpdate )
			m_soundAttackEnding.Play();


		if( m_pNoteField )
			m_pNoteField->Update( fDeltaTime );

		float fMiniPercent = m_pPlayerState->m_PlayerOptions.GetCurrent().m_fEffects[PlayerOptions::EFFECT_MINI];
		float fTinyPercent = m_pPlayerState->m_PlayerOptions.GetCurrent().m_fEffects[PlayerOptions::EFFECT_TINY];
		float fJudgmentZoom = min( powf(0.5f, fMiniPercent+fTinyPercent), 1.0f );
		
		//
		// Update Y positions
		//
		{
			for( int c=0; c<GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer; c++ )
			{
				float fPercentReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(c);
				float fHoldJudgeYPos = SCALE( fPercentReverse, 0.f, 1.f, HOLD_JUDGMENT_Y_STANDARD, HOLD_JUDGMENT_Y_REVERSE );
	//			float fGrayYPos = SCALE( fPercentReverse, 0.f, 1.f, GRAY_ARROWS_Y_STANDARD, GRAY_ARROWS_Y_REVERSE );

				const float fX = ArrowEffects::GetXPos( m_pPlayerState, c, 0 );
				const float fZ = ArrowEffects::GetZPos( m_pPlayerState, c, 0 );

				m_vpHoldJudgment[c]->SetX( fX );
				m_vpHoldJudgment[c]->SetY( fHoldJudgeYPos );
				m_vpHoldJudgment[c]->SetZ( fZ );
				m_vpHoldJudgment[c]->SetZoom( fJudgmentZoom );
			}
		}

		// NoteField accounts for reverse on its own now.
		//if( m_pNoteField )
		//	m_pNoteField->SetY( fGrayYPos );

		const bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(0) == 1;
		float fPercentCentered = m_pPlayerState->m_PlayerOptions.GetCurrent().m_fScrolls[PlayerOptions::SCROLL_CENTERED];

		if( m_pActorWithJudgmentPosition != NULL )
		{
			const Actor::TweenState &ts1 = m_tsJudgment[bReverse?1:0][0];
			const Actor::TweenState &ts2 = m_tsJudgment[bReverse?1:0][1];
			Actor::TweenState::MakeWeightedAverage( m_pActorWithJudgmentPosition->DestTweenState(), ts1, ts2, fPercentCentered );
		}

		if( m_pActorWithComboPosition != NULL )
		{
			const Actor::TweenState &ts1 = m_tsCombo[bReverse?1:0][0];
			const Actor::TweenState &ts2 = m_tsCombo[bReverse?1:0][1];
			Actor::TweenState::MakeWeightedAverage( m_pActorWithComboPosition->DestTweenState(), ts1, ts2, fPercentCentered );
		}

		float fNoteFieldZoom = 1 - fTinyPercent*0.5f;
		if( m_pNoteField )
			m_pNoteField->SetZoom( fNoteFieldZoom );
		if( m_pActorWithJudgmentPosition != NULL )
			m_pActorWithJudgmentPosition->SetZoom( m_pActorWithJudgmentPosition->GetZoom() * fJudgmentZoom );
		if( m_pActorWithComboPosition != NULL )
			m_pActorWithComboPosition->SetZoom( m_pActorWithComboPosition->GetZoom() * fJudgmentZoom );
	}

	// If we're paused, don't update tap or hold note logic, so hold notes can be released
	// during pause.
	if( m_bPaused )
		return;
	
	//
	// Check for a strum miss
	//
	if( m_pPlayerState->m_fLastStrumMusicSeconds != -1  &&
		m_pPlayerState->m_fLastStrumMusicSeconds + g_fTimingWindowStrum < GAMESTATE->m_fMusicSeconds )
	{
		DoStrumMiss();
	}


	//
	// Check for TapNote misses
	//
	UpdateTapNotesMissedOlderThan( GetMaxStepDistanceSeconds() );



	//
	// update pressed flag
	//
	const int iNumCols = GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer;
	ASSERT_M( iNumCols <= MAX_COLS_PER_PLAYER, ssprintf("%i > %i", iNumCols, MAX_COLS_PER_PLAYER) );
	for( int col=0; col < iNumCols; ++col )
	{
		ASSERT( m_pPlayerState );

		// TODO: Remove use of PlayerNumber.
		GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( col, m_pPlayerState->m_PlayerNumber );

		bool bIsHoldingButton = INPUTMAPPER->IsBeingPressed( GameI );
		// TODO: Make this work for non-human-controlled players
		if( bIsHoldingButton && !GAMESTATE->m_bDemonstrationOrJukebox && m_pPlayerState->m_PlayerController==PC_HUMAN )
			if( m_pNoteField )
				m_pNoteField->SetPressed( col );
	}


	//
	// handle Autoplay for rolls
	//
	if( m_pPlayerState->m_PlayerController != PC_HUMAN )
	{
		for( int iTrack=0; iTrack<m_NoteData.GetNumTracks(); ++iTrack )
		{
			// TODO: Make the CPU miss sometimes.
			int iHeadRow;
			if( !m_NoteData.IsHoldNoteAtRow(iTrack, iSongRow, &iHeadRow) )
				continue;

			const TapNote &tn = m_NoteData.GetTapNote( iTrack, iHeadRow );
			if( tn.type != TapNote::hold_head || tn.subType != TapNote::hold_head_roll )
				continue;
			if( tn.HoldResult.hns != HNS_None )
				continue;
			if( tn.HoldResult.fLife >= 0.5f )
				continue;

			Step( iTrack, iHeadRow, now, false, false );
			if( m_pPlayerState->m_PlayerController == PC_AUTOPLAY )
			{
				STATSMAN->m_CurStageStats.m_bUsedAutoplay = true;
				if( m_pPlayerStageStats )
					m_pPlayerStageStats->m_bDisqualified = true;
			}
		}
	}


	//
	// update HoldNotes logic
	//
	{

		// Fast forward to the first that needs hold judging.
		{
			NoteData::all_tracks_iterator &iter = *m_pIterNeedsHoldJudging;
			while( !iter.IsAtEnd()  &&  iter.Row() <= iSongRow  &&  !NeedsHoldJudging(*iter) )
				++iter;
		}


		vector<TrackRowTapNote> vHoldNotesToGradeTogether;
		int iRowOfLastHoldNote = -1;
		NoteData::all_tracks_iterator iter = *m_pIterNeedsHoldJudging;	// copy
		for( ; !iter.IsAtEnd() &&  iter.Row() <= iSongRow; ++iter )
		{
			TapNote &tn = *iter;
			if( tn.type != TapNote::hold_head )
				continue;

			int iTrack = iter.Track();
			int iRow = iter.Row();
			TrackRowTapNote trtn = { iTrack, iRow, &tn };

			/* All holds must be of the same subType because fLife is handled 
			* in different ways depending on the SubType.  Handle Rolls one at a time 
			 * and don't mix with holds. */
			switch( tn.subType )
			{
			DEFAULT_FAIL( tn.subType );
			case TapNote::hold_head_hold:
				break;
			case TapNote::hold_head_roll:
				{
					vector<TrackRowTapNote> v;
					v.push_back( trtn );
					UpdateHoldNotes( iSongRow, fDeltaTime, v );
				}
				continue;	// don't process this below
			}

			if( iRow != iRowOfLastHoldNote  ||  !JUDGE_HOLD_NOTES_ON_SAME_ROW_TOGETHER )
			{
				if( !vHoldNotesToGradeTogether.empty() )
				{
					UpdateHoldNotes( iSongRow, fDeltaTime, vHoldNotesToGradeTogether );
					vHoldNotesToGradeTogether.clear();
				}
			}
			iRowOfLastHoldNote = iRow;
			vHoldNotesToGradeTogether.push_back( trtn );
		}

		if( !vHoldNotesToGradeTogether.empty() )
		{
			UpdateHoldNotes( iSongRow, fDeltaTime, vHoldNotesToGradeTogether );
			vHoldNotesToGradeTogether.clear();
 		}
	}

	{
		// Why was this originally "BeatToNoteRowNotRounded"?  It should be rounded.  -Chris
		/* We want to send the crossed row message exactly when we cross the row--not
		 * .5 before the row.  Use a very slow song (around 2 BPM) as a test case: without
		 * rounding, autoplay steps early. -glenn */
		const int iRowNow = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat );
		if( iRowNow >= 0 )
		{
			if( GAMESTATE->IsPlayerEnabled(m_pPlayerState) )
				CrossedRows( iRowNow, now );
		}
	}


	// Check for completely judged rows.
	UpdateJudgedRows();

	// process transforms that are waiting to be applied
	ApplyWaitingTransforms();
}

/* Update a group of holds with shared scoring/life.  All of these holds will have the same start row. */
void Player::UpdateHoldNotes( int iSongRow, float fDeltaTime, vector<TrackRowTapNote> &vTN )
{
	ASSERT( !vTN.empty() );

	int iStartRow = vTN[0].iRow;
	int iMaxEndRow = INT_MIN;
	int iFirstTrackWithMaxEndRow = -1;

	TapNote::SubType subType = TapNote::SubType_Invalid;
	FOREACH( TrackRowTapNote, vTN, trtn )
	{
		int iTrack = trtn->iTrack;
		ASSERT( iStartRow == trtn->iRow );
		TapNote &tn = *trtn->pTN;
		int iEndRow = iStartRow + tn.iDuration;
		if( subType == TapNote::SubType_Invalid )
			subType = tn.subType;
		
		/* All holds must be of the same subType because fLife is handled 
		 * in different ways depending on the SubType. */
		ASSERT( tn.subType == subType );
	
		if( iEndRow > iMaxEndRow )
		{
			iMaxEndRow = iEndRow;
			iFirstTrackWithMaxEndRow = iTrack;
		}
	}

	ASSERT( iFirstTrackWithMaxEndRow != -1 );

	FOREACH( TrackRowTapNote, vTN, trtn )
	{
		TapNote &tn = *trtn->pTN;

		// set hold flags so NoteField can do intelligent drawing
		tn.HoldResult.bHeld = false;
		tn.HoldResult.bActive = false;

		int iRow = trtn->iRow;

		// If the song beat is in the range of this hold:
		if( iRow <= iSongRow  &&  iRow <= iMaxEndRow )
			tn.HoldResult.fOverlappedTime += fDeltaTime;
		else
			tn.HoldResult.fOverlappedTime = 0;
	}

	HoldNoteScore hns = vTN[0].pTN->HoldResult.hns;
	float fLife = vTN[0].pTN->HoldResult.fLife;
	if( hns != HNS_None )	// if this HoldNote already has a result
		return;	// we don't need to update the logic for this group

	bool bInitiatedNote = true;
	if( REQUIRE_STEP_ON_HOLD_HEADS )
	{
		FOREACH( TrackRowTapNote, vTN, trtn )
		{
			TapNote &tn = *trtn->pTN;
			TapNoteScore tns = tn.result.tns;

			// TODO: When using JUDGE_HOLD_NOTES_ON_SAME_ROW_TOGETHER, require that the whole row of 
			// taps was hit before activating this group of holds.
			bInitiatedNote &= tns != TNS_None  &&  tns != TNS_Miss;	// did they step on the start of this hold?
		}
	}

	bool bIsHoldingButton = true;
	FOREACH( TrackRowTapNote, vTN, trtn )
	{
		int iTrack = trtn->iTrack;

		// TODO: Remove use of PlayerNumber.
		PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

		if( m_pPlayerState->m_PlayerController != PC_HUMAN )
		{
			// TODO: Make the CPU miss sometimes.
			if( m_pPlayerState->m_PlayerController == PC_AUTOPLAY )
			{
				STATSMAN->m_CurStageStats.m_bUsedAutoplay = true;
				if( m_pPlayerStageStats != NULL )
					m_pPlayerStageStats->m_bDisqualified = true;
			}
		}
		else
		{
			GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( iTrack, pn );
			bIsHoldingButton &= INPUTMAPPER->IsBeingPressed( GameI, m_pPlayerState->m_mp );
		}
	}

	if( bInitiatedNote && fLife != 0 )
	{
		/* This hold note is not judged and we stepped on its head.  Update iLastHeldRow.
		 * Do this even if we're a little beyond the end of the hold note, to make sure
		 * iLastHeldRow is clamped to iEndRow if the hold note is held all the way. */
		FOREACH( TrackRowTapNote, vTN, trtn )
		{
			TapNote &tn = *trtn->pTN;
			int iEndRow = iStartRow + tn.iDuration;

			trtn->pTN->HoldResult.iLastHeldRow = min( iSongRow, iEndRow );
		}
	}

	// If the song beat is in the range of this hold:
	if( iStartRow <= iSongRow  &&  iStartRow <= iMaxEndRow )
	{
		switch( subType )
		{
		case TapNote::hold_head_hold:
			FOREACH( TrackRowTapNote, vTN, trtn )
			{
				TapNote &tn = *trtn->pTN;
			
				// set hold flag so NoteField can do intelligent drawing
				tn.HoldResult.bHeld = bIsHoldingButton && bInitiatedNote;
				tn.HoldResult.bActive = bInitiatedNote;
			}

			if( bInitiatedNote && bIsHoldingButton )
			{
				// Increase life
				fLife = 1;
			}
			else
			{
				// Decrease life
				fLife -= fDeltaTime/GetWindowSeconds(TW_Hold);
				fLife = max( fLife, 0 );	// clamp
			}
			break;
		case TapNote::hold_head_roll:
			FOREACH( TrackRowTapNote, vTN, trtn )
			{
				TapNote &tn = *trtn->pTN;
				tn.HoldResult.bHeld = true;
				tn.HoldResult.bActive = bInitiatedNote;
			}

			// give positive life in Step(), not here.

			// Decrease life
			fLife -= fDeltaTime/GetWindowSeconds(TW_Roll);
			fLife = max( fLife, 0 );	// clamp
			break;
		default:
			ASSERT(0);
		}
	}

	// TODO: Cap the active time passed to the score keeper to the actual start time and end time of the hold.
	if( vTN[0].pTN->HoldResult.bActive ) 
	{
		float fSecondsActiveSinceLastUpdate = fDeltaTime * GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;
		if( m_pPrimaryScoreKeeper )
			m_pPrimaryScoreKeeper->HandleHoldActiveSeconds( fSecondsActiveSinceLastUpdate );
		if( m_pSecondaryScoreKeeper )
			m_pSecondaryScoreKeeper->HandleHoldActiveSeconds( fSecondsActiveSinceLastUpdate );
	}

	/* check for LetGo.  If the head was missed completely, don't count an LetGo. */
	/* Why?  If you never step on the head, then it will be left as HNS_None, which doesn't seem correct. */
	if( IMMEDIATE_HOLD_LET_GO )
	{
		if( bInitiatedNote && fLife == 0 )	// the player has not pressed the button for a long time!
			hns = HNS_LetGo;
	}

	// score hold notes that have passed
	if( iSongRow >= iMaxEndRow )
	{
		bool bLetGoOfHoldNote = false;

		if( HOLD_CHECKPOINTS )
		{
			int iCheckpointsMissed = 0;
			FOREACH( TrackRowTapNote, vTN, v )
				iCheckpointsMissed += v->pTN->HoldResult.iCheckpointsMissed;
			bLetGoOfHoldNote = iCheckpointsMissed > 0;
		}
		else
		{
			bLetGoOfHoldNote = fLife == 0;
		}

		if( bInitiatedNote  &&  !bLetGoOfHoldNote )
		{
			fLife = 1;
			hns = HNS_Held;
			bool bBright = m_pPlayerStageStats && m_pPlayerStageStats->m_iCurCombo>(int)BRIGHT_GHOST_COMBO_THRESHOLD;
			if( m_pNoteField )
			{
				FOREACH( TrackRowTapNote, vTN, trtn )
				{
					int iTrack = trtn->iTrack;
					m_pNoteField->DidHoldNote( iTrack, HNS_Held, bBright );	// bright ghost flash
				}
			}
		}
		else
		{
			hns = HNS_LetGo;
		}
	}
		
	FOREACH( TrackRowTapNote, vTN, trtn )
	{
		TapNote &tn = *trtn->pTN;
		tn.HoldResult.fLife = fLife;
		tn.HoldResult.hns = hns;		
	}

	if( hns != HNS_None )
	{
		TapNote &tn = *vTN[0].pTN;
		HandleHoldScore( tn );
		SetHoldJudgment( tn.result.tns, tn.HoldResult.hns, iFirstTrackWithMaxEndRow );
	}
}

void Player::ApplyWaitingTransforms()
{
	for( unsigned j=0; j<m_pPlayerState->m_ModsToApply.size(); j++ )
	{
		const Attack &mod = m_pPlayerState->m_ModsToApply[j];
		PlayerOptions po;
		po.FromString( mod.sModifiers );

		float fStartBeat, fEndBeat;
		mod.GetRealtimeAttackBeats( GAMESTATE->m_pCurSong, m_pPlayerState, fStartBeat, fEndBeat );
		fEndBeat = min( fEndBeat, m_NoteData.GetLastBeat() );

		LOG->Trace( "Applying transform '%s' from %f to %f to '%s'", mod.sModifiers.c_str(), fStartBeat, fEndBeat,
			GAMESTATE->m_pCurSong->GetTranslitMainTitle().c_str() );

		NoteDataUtil::TransformNoteData( m_NoteData, po, GAMESTATE->GetCurrentStyle()->m_StepsType, BeatToNoteRow(fStartBeat), BeatToNoteRow(fEndBeat) );
	}
	m_pPlayerState->m_ModsToApply.clear();
}

void Player::DrawPrimitives()
{
	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	// May have both players in doubles (for battle play); only draw primary player.
	if( GAMESTATE->GetCurrentStyle()->m_StyleType == StyleType_OnePlayerTwoSides  &&
		pn != GAMESTATE->m_MasterPlayerNumber )
		return;


	// Draw these below everything else.
	if( m_pPlayerState->m_PlayerOptions.GetCurrent().m_fBlind == 0 )
		if( m_sprCombo )
			m_sprCombo->Draw();

	if( m_pAttackDisplay )
		m_pAttackDisplay->Draw();

	if( TAP_JUDGMENTS_UNDER_FIELD )
		DrawTapJudgments();

	if( HOLD_JUDGMENTS_UNDER_FIELD )
		DrawHoldJudgments();

	float fTilt = m_pPlayerState->m_PlayerOptions.GetCurrent().m_fPerspectiveTilt;
	float fSkew = m_pPlayerState->m_PlayerOptions.GetCurrent().m_fSkew;
	bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(0)>0.5;


	DISPLAY->CameraPushMatrix();
	DISPLAY->PushMatrix();

	float fCenterY = this->GetY()+(GRAY_ARROWS_Y_STANDARD+GRAY_ARROWS_Y_REVERSE)/2;

	DISPLAY->LoadMenuPerspective( 45, SCREEN_WIDTH, SCREEN_HEIGHT, SCALE(fSkew,0.f,1.f,this->GetX(),SCREEN_CENTER_X), fCenterY );

	if( m_pNoteField )
	{
		float fOriginalY = 	m_pNoteField->GetY();

		float fTiltDegrees = SCALE(fTilt,-1.f,+1.f,+30,-30) * (bReverse?-1:1);

		float fZoom = SCALE( m_pPlayerState->m_PlayerOptions.GetCurrent().m_fEffects[PlayerOptions::EFFECT_TINY], 0.f, 1.f, 1.f, 0.5f );
		if( fTilt > 0 )
			fZoom *= SCALE( fTilt, 0.f, 1.f, 1.f, 0.9f );
		else
			fZoom *= SCALE( fTilt, 0.f, -1.f, 1.f, 0.9f );

		float fYOffset;
		if( fTilt > 0 )
			fYOffset = SCALE( fTilt, 0.f, 1.f, 0.f, -45.f ) * (bReverse?-1:1);
		else
			fYOffset = SCALE( fTilt, 0.f, -1.f, 0.f, -20.f ) * (bReverse?-1:1);

		m_pNoteField->SetY( fOriginalY + fYOffset );
		m_pNoteField->SetZoom( fZoom );
		m_pNoteField->SetRotationX( fTiltDegrees );
		m_pNoteField->Draw();

		m_pNoteField->SetY( fOriginalY );
	}

	DISPLAY->CameraPopMatrix();
	DISPLAY->PopMatrix();


	if( !(bool)TAP_JUDGMENTS_UNDER_FIELD )
		DrawTapJudgments();

	if( !(bool)HOLD_JUDGMENTS_UNDER_FIELD )
		DrawHoldJudgments();
}

void Player::DrawTapJudgments()
{
	if( m_pPlayerState->m_PlayerOptions.GetCurrent().m_fBlind > 0 )
		return;

	if( m_sprJudgment )
		m_sprJudgment->Draw();
}

void Player::DrawHoldJudgments()
{
	if( m_pPlayerState->m_PlayerOptions.GetCurrent().m_fBlind > 0 )
		return;

	for( int c=0; c<m_NoteData.GetNumTracks(); c++ )
		if( m_vpHoldJudgment[c] )
			m_vpHoldJudgment[c]->Draw();
}


void Player::ChangeLife( TapNoteScore tns )
{
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	if( m_pLifeMeter )
		m_pLifeMeter->ChangeLife( tns );
	if( m_pCombinedLifeMeter )
		m_pCombinedLifeMeter->ChangeLife( pn, tns );

	ChangeLifeRecord();

	switch( tns )
	{
	case TNS_None:
	case TNS_Miss:
	case TNS_CheckpointMiss:
	case TNS_HitMine:
		++m_pPlayerState->m_iTapsMissedSinceLastHasteUpdate;
		break;
	default:
		++m_pPlayerState->m_iTapsHitSinceLastHasteUpdate;
		break;
	}
}

void Player::ChangeLife( HoldNoteScore hns, TapNoteScore tns )
{
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	if( m_pLifeMeter )
		m_pLifeMeter->ChangeLife( hns, tns );
	if( m_pCombinedLifeMeter )
		m_pCombinedLifeMeter->ChangeLife( pn, hns, tns );

	ChangeLifeRecord();
}

void Player::ChangeLifeRecord()
{
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	float fLife = -1;
	if( m_pLifeMeter )
	{
		fLife = m_pLifeMeter->GetLife();
	}
	else if( m_pCombinedLifeMeter )
	{
		fLife = GAMESTATE->m_fTugLifePercentP1;
		if( pn == PLAYER_2 )
			fLife = 1.0f - fLife;
	}
	if( fLife != -1 )
		if( m_pPlayerStageStats )
			m_pPlayerStageStats->SetLifeRecordAt( fLife, STATSMAN->m_CurStageStats.m_fStepsSeconds );
}

int Player::GetClosestNoteDirectional( int col, int iStartRow, int iEndRow, bool bAllowGraded, bool bForward ) const
{
	NoteData::const_iterator begin, end;
	m_NoteData.GetTapNoteRange( col, iStartRow, iEndRow, begin, end );

	if( !bForward )
		swap( begin, end );

	while( begin != end )
	{
		if( !bForward )
			--begin;

		/* Is this the row we want? */
		do {
			const TapNote &tn = begin->second;
			if( tn.type == TapNote::empty )
				break;
			if( !bAllowGraded && tn.result.tns != TNS_None )
				break;

			return begin->first;
		} while(0);

		if( bForward )
			++begin;
	}

	return -1;
}

/* Find the closest note to fBeat. */
int Player::GetClosestNote( int col, int iNoteRow, int iMaxRowsAhead, int iMaxRowsBehind, bool bAllowGraded ) const
{
	// Start at iIndexStartLookingAt and search outward.
	int iNextIndex = GetClosestNoteDirectional( col, iNoteRow, iNoteRow+iMaxRowsAhead, bAllowGraded, true );
	int iPrevIndex = GetClosestNoteDirectional( col, iNoteRow-iMaxRowsBehind, iNoteRow, bAllowGraded, false );

	if( iNextIndex == -1 && iPrevIndex == -1 )
		return -1;
	if( iNextIndex == -1 )
		return iPrevIndex;
	if( iPrevIndex == -1 )
		return iNextIndex;

	/* Figure out which row is closer. */
	if( abs(iNoteRow-iNextIndex) > abs(iNoteRow-iPrevIndex) )
		return iPrevIndex;
	else
		return iNextIndex;
}

int Player::GetClosestNonEmptyRowDirectional( int iStartRow, int iEndRow, bool bAllowGraded, bool bForward ) const
{
	if( bForward )
	{
		NoteData::all_tracks_iterator iter = m_NoteData.GetTapNoteRangeAllTracks( iStartRow, iEndRow );

		while( !iter.IsAtEnd() )
		{
			if( NoteDataWithScoring::IsRowCompletelyJudged(m_NoteData, iter.Row(), m_pPlayerState->m_PlayerNumber) )
			{
				++iter;
				continue;
			}
			return iter.Row();
		}
	}
	else
	{
		NoteData::all_tracks_reverse_iterator iter = m_NoteData.GetTapNoteRangeAllTracksReverse( iStartRow, iEndRow );

		while( !iter.IsAtEnd() )
		{
			if( NoteDataWithScoring::IsRowCompletelyJudged(m_NoteData, iter.Row(), m_pPlayerState->m_PlayerNumber) )
			{
				++iter;
				continue;
			}
			return iter.Row();
		}
	}

	return -1;
}

/* Find the closest note to fBeat. */
int Player::GetClosestNonEmptyRow( int iNoteRow, int iMaxRowsAhead, int iMaxRowsBehind, bool bAllowGraded ) const
{
	// Start at iIndexStartLookingAt and search outward.
	int iNextRow = GetClosestNonEmptyRowDirectional( iNoteRow, iNoteRow+iMaxRowsAhead, bAllowGraded, true );
	int iPrevRow = GetClosestNonEmptyRowDirectional( iNoteRow-iMaxRowsBehind, iNoteRow, bAllowGraded, false );

	if( iNextRow == -1 && iPrevRow == -1 )
		return -1;
	if( iNextRow == -1 )
		return iPrevRow;
	if( iPrevRow == -1 )
		return iNextRow;

	/* Figure out which row is closer. */
	if( abs(iNoteRow-iNextRow) > abs(iNoteRow-iPrevRow) )
		return iPrevRow;
	else
		return iNextRow;
}

bool Player::IsOniDead() const
{
	// If we're playing on oni and we've died, do nothing.
	return GAMESTATE->m_SongOptions.GetCurrent().m_LifeType == SongOptions::LIFE_BATTERY && m_pPlayerStageStats  && m_pPlayerStageStats->m_bFailed;
}

void Player::Fret( int col, int row, const RageTimer &tm, bool bHeld, bool bRelease )
{
	if( IsOniDead() )
		return;
	
	DEBUG_ASSERT_M( col >= 0  &&  col <= m_NoteData.GetNumTracks(), ssprintf("%i, %i", col, m_NoteData.GetNumTracks()) );

	m_vbFretIsDown[ col ] = !bRelease;


	// Handle changing fret during a strum
	if( m_pPlayerState->m_fLastStrumMusicSeconds != -1 )
	{
		LOG->Trace( "StrumTry" );
		StepStrumHopo( col, row, tm, bHeld, bRelease, ButtonType_StrumFretsChanged );
	}


	// Handle hammer-ons and pull-offs
	const float fPositionSeconds = GAMESTATE->m_fMusicSeconds - tm.Ago();
	int iHopoCol = -1;
	bool bDoHopo = 
		m_pPlayerState->m_fLastHopoNoteMusicSeconds != -1  &&
		fPositionSeconds <= m_pPlayerState->m_fLastHopoNoteMusicSeconds + g_fTimingWindowHopo;
	if( bDoHopo )
	{
		// do a Hopo:
		//  - on pressed fret is no higher fret is held
		//  - on next lowest held fret when the highest held fret is released
		bool bHigherFretIsDown = false;
		for( int i=col+1; i<m_NoteData.GetNumTracks(); i++ )
		{
			if( m_vbFretIsDown[i] )
			{
				bHigherFretIsDown = true;
				break;
			}
		}
		if( bHigherFretIsDown )
			bDoHopo = false;
	}

	if( bDoHopo )
	{
		if( !bRelease )
		{
			// hammer-on
			iHopoCol = col;
		}
		else
		{
			// pull-off
			// find next lowest fret that is held
			for( int i=col-1; i>=0; i-- )
			{
				if( m_vbFretIsDown[i] )
				{
					iHopoCol = i;
					break;
				}
			}
			if( iHopoCol == -1 )
				bDoHopo = false;
		}
	}

	if( bDoHopo )
		Hopo( iHopoCol, row, tm, bHeld, bRelease );


	// Check if this fret breaks all active holds.
	if( !bRelease )
	{
		const float fSongBeat = GAMESTATE->m_fSongBeat;
		const int iSongRow = BeatToNoteRow( fSongBeat );

		int iMaxHoldCol = -1;
		int iNumColsHeld = 0;

		// Score all active holds to NotHeld
		for( int iTrack=0; iTrack<m_NoteData.GetNumTracks(); ++iTrack )
		{
			// Since this is being called every frame, let's not check the whole array every time.
			// Instead, only check 1 beat back.  Even 1 is overkill.
			const int iStartCheckingAt = max( 0, iSongRow-BeatToNoteRow(1) );
			NoteData::iterator begin, end;
			m_NoteData.GetTapNoteRangeInclusive( iTrack, iStartCheckingAt, iSongRow+1, begin, end );
			for( ; begin != end; ++begin )
			{
				TapNote &tn = begin->second;
				if( tn.HoldResult.bActive )
				{
					iMaxHoldCol = iTrack;
					iNumColsHeld++;
				}
			}
		}

		// Any frets to the right of an active hold will break the hold.
		if( col > iMaxHoldCol  ||  iNumColsHeld >= 2 )
			ScoreAllActiveHoldsLetGo();
	}
}


void Player::Strum( int col, int row, const RageTimer &tm, bool bHeld, bool bRelease )
{
	if( bRelease )
		return;

	if( m_pPlayerState->m_fLastStrumMusicSeconds != -1 )
	{
		DoStrumMiss();
	}

	m_pPlayerState->m_fLastStrumMusicSeconds = GAMESTATE->m_fMusicSeconds;

	StepStrumHopo( col, row, tm, bHeld, bRelease, ButtonType_StrumFretsChanged );
}

void Player::DoTapScoreNone()
{
	Message msg( "ScoreNone" );
	MESSAGEMAN->Broadcast( msg );

	const int iOldCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurCombo : 0;
	const int iOldMissCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurMissCombo : 0;

	/* The only real way to tell if a mine has been scored is if it has disappeared
	* but this only works for hit mines so update the scores for avoided mines here. */
	if( m_pPrimaryScoreKeeper )
		m_pPrimaryScoreKeeper->HandleTapScoreNone();
	if( m_pSecondaryScoreKeeper )
		m_pSecondaryScoreKeeper->HandleTapScoreNone();

	SendComboMessages( iOldCombo, iOldMissCombo );


	if( m_pLifeMeter )
		m_pLifeMeter->HandleTapScoreNone();
	// TODO: Remove use of PlayerNumber
	PlayerNumber pn = PLAYER_INVALID;
	if( m_pCombinedLifeMeter )
		m_pCombinedLifeMeter->HandleTapScoreNone( pn );

	if( PENALIZE_TAP_SCORE_NONE )
	{
		SetJudgment( TNS_Miss, 0 );
		// the ScoreKeeper will subrtract points later.
	}
}

void Player::DoStrumMiss()
{
	m_pPlayerState->m_fLastStrumMusicSeconds = -1;
	DoTapScoreNone();

	ScoreAllActiveHoldsLetGo();
}

void Player::ScoreAllActiveHoldsLetGo()
{
	if( PENALIZE_TAP_SCORE_NONE )
	{
		const float fSongBeat = GAMESTATE->m_fSongBeat;
		const int iSongRow = BeatToNoteRow( fSongBeat );

		// Score all active holds to NotHeld
		for( int iTrack=0; iTrack<m_NoteData.GetNumTracks(); ++iTrack )
		{
			// Since this is being called every frame, let's not check the whole array every time.
			// Instead, only check 1 beat back.  Even 1 is overkill.
			const int iStartCheckingAt = max( 0, iSongRow-BeatToNoteRow(1) );
			NoteData::iterator begin, end;
			m_NoteData.GetTapNoteRangeInclusive( iTrack, iStartCheckingAt, iSongRow+1, begin, end );
			for( ; begin != end; ++begin )
			{
				TapNote &tn = begin->second;
				if( tn.HoldResult.bActive )
				{
					tn.HoldResult.hns = HNS_LetGo;
					tn.HoldResult.fLife = 0;
				
					HandleHoldScore( tn );
					SetHoldJudgment( tn.result.tns, tn.HoldResult.hns, iTrack );
				}
			}
		}
	}
}

void Player::StepStrumHopo( int col, int row, const RageTimer &tm, bool bHeld, bool bRelease, Player::ButtonType pbt )
{
	if( IsOniDead() )
		return;

	// Do everything that depends on a RageTimer here and set your breakpoints somewhere after this block
	const float fLastBeatUpdate = GAMESTATE->m_LastBeatUpdate.Ago();
	const float fPositionSeconds = GAMESTATE->m_fMusicSeconds - tm.Ago();
	const float fTimeSinceStep = tm.Ago();

	switch( pbt )
	{
	DEFAULT_FAIL(pbt);
	case ButtonType_Step:
		break;
	case ButtonType_StrumFretsChanged:
	case ButtonType_Hopo:
		// releasing should hit regular notes, not lifts
		bRelease = false;
		break;
	}

	const float fSongBeat = GAMESTATE->m_pCurSong ? GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds ) : GAMESTATE->m_fSongBeat;
	const int iSongRow = row == -1 ? BeatToNoteRow( fSongBeat ) : row;
	
	if( col != -1 && !bRelease )
	{
		// Update roll life
		// Let's not check the whole array every time.
		// Instead, only check 1 beat back.  Even 1 is overkill.
		// Just update the life here and let Update judge the roll.
		const int iStartCheckingAt = max( 0, iSongRow-BeatToNoteRow(1) );
		NoteData::iterator begin, end;
		m_NoteData.GetTapNoteRangeInclusive( col, iStartCheckingAt, iSongRow+1, begin, end );
		for( ; begin != end; ++begin )
		{
			TapNote &tn = begin->second;
			if( tn.type != TapNote::hold_head )
				continue;

			switch( tn.subType )
			{
			DEFAULT_FAIL( tn.subType );
			case TapNote::hold_head_hold:
				continue;
			case TapNote::hold_head_roll:
				break;
			}

			const int iRow = begin->first;

			HoldNoteScore hns = tn.HoldResult.hns;
			if( hns != HNS_None )	// if this HoldNote already has a result
				continue;	// we don't need to update the logic for this one

			// if they got a bad score or haven't stepped on the corresponding tap yet
			const TapNoteScore tns = tn.result.tns;
			bool bInitiatedNote = true;
			if( REQUIRE_STEP_ON_HOLD_HEADS )
				bInitiatedNote = tns != TNS_None  &&  tns != TNS_Miss;	// did they step on the start?
			else
				bInitiatedNote = true;
			const int iEndRow = iRow + tn.iDuration;

			if( bInitiatedNote && tn.HoldResult.fLife != 0 )
			{
				/* This hold note is not judged and we stepped on its head.  Update iLastHeldRow.
				 * Do this even if we're a little beyond the end of the hold note, to make sure
				 * iLastHeldRow is clamped to iEndRow if the hold note is held all the way. */
				tn.HoldResult.iLastHeldRow = min( iSongRow, iEndRow );
			}

			// If the song beat is in the range of this hold:
			if( iRow <= iSongRow && iRow <= iEndRow )
			{
				if( bInitiatedNote )
				{
					// Increase life
					tn.HoldResult.fLife = 1;

					// increment combo
					const int iOldCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurCombo : 0;
					const int iOldMissCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurMissCombo : 0;

					if( m_pPlayerStageStats )
					{
						m_pPlayerStageStats->m_iCurCombo++;
						m_pPlayerStageStats->m_iCurMissCombo = 0;
					}

					SendComboMessages( iOldCombo, iOldMissCombo );
					if( m_pPlayerStageStats )
						SetCombo( m_pPlayerStageStats->m_iCurCombo, m_pPlayerStageStats->m_iCurMissCombo );

					bool bBright = m_pPlayerStageStats && m_pPlayerStageStats->m_iCurCombo>(int)BRIGHT_GHOST_COMBO_THRESHOLD;
					if( m_pNoteField )
						m_pNoteField->DidHoldNote( col, HNS_Held, bBright );
				}
				break;
			}
		}
	}



	//
	// Count calories for this step, unless we're being called because a button is
	// held over a mine or being released.
	//
	// TODO: Move calorie counting into a ScoreKeeper?
	if( m_pPlayerStageStats && m_pPlayerState && !bHeld && !bRelease )
	{
		// TODO: remove use of PlayerNumber
		PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
		int iWeightPounds = 120;
		Profile *pProfile = PROFILEMAN->GetProfile( pn );
		if( pProfile )
			iWeightPounds = pProfile->GetCalculatedWeightPounds();

		int iNumTracksHeld = 0;
		for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
		{
			GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( t, pn );
			const float fSecsHeld = INPUTMAPPER->GetSecsHeld( GameI );
			if( fSecsHeld > 0  && fSecsHeld < m_fTimingWindowJump )
				iNumTracksHeld++;
		}

		float fCals = 0;
		switch( iNumTracksHeld )
		{
		case 0:
			// autoplay is on, or this is a computer player
			iNumTracksHeld = 1;
			// fall through
		default:
			{
				float fCalsFor100Lbs = SCALE( iNumTracksHeld, 1, 2, 0.023f, 0.077f );
				float fCalsFor200Lbs = SCALE( iNumTracksHeld, 1, 2, 0.041f, 0.133f );
				fCals = SCALE( pProfile->GetCalculatedWeightPounds(), 100.f, 200.f, fCalsFor100Lbs, fCalsFor200Lbs );
			}
			break;
		}

		m_pPlayerStageStats->m_fCaloriesBurned += fCals;
		m_pPlayerStageStats->m_iNumControllerSteps ++;
	}




	//
	// Check for step on a TapNote
	//
	/* XXX: This seems wrong. If a player steps twice quickly and two notes are close together in the same column then
	 * it is possible for the two notes to be graded out of order. Two possible fixes:
	 * 1. Adjust the fSongBeat (or the resulting note row) backward by iStepSearchRows and search forward two iStepSearchRows
	 * lengths, disallowing graded. This doesn't seem right because if a second note has passed, an earlier one should not
	 * be graded.
	 * 2. Clamp the distance searched backward to the previous row graded.
	 * Either option would fundamentally change the grading of two quick notes "jack hammers." Hmm.
	 */
	const int iStepSearchRows = BeatToNoteRow( StepSearchDistance * GAMESTATE->m_fCurBPS * GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate );
	int iRowOfOverlappingNoteOrRow = row;
	if( row == -1 )
	{
		switch( pbt )
		{
		DEFAULT_FAIL(pbt);
		case ButtonType_StrumFretsChanged:
			iRowOfOverlappingNoteOrRow = GetClosestNonEmptyRow( iSongRow, iStepSearchRows, iStepSearchRows, false );
			break;
		case ButtonType_Hopo:
		case ButtonType_Step:
			iRowOfOverlappingNoteOrRow = GetClosestNote( col, iSongRow, iStepSearchRows, iStepSearchRows, false );
			break;
		}
	}
	
	// calculate TapNoteScore
	TapNoteScore score = TNS_None;

	if( iRowOfOverlappingNoteOrRow != -1 )
	{
		// compute the score for this hit
		float fNoteOffset = 0.0f;
		// we need this later if we are autosyncing
		const float fStepBeat = NoteRowToBeat( iRowOfOverlappingNoteOrRow );
		const float fStepSeconds = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat(fStepBeat);

		if( row == -1 )
		{
			/* We actually stepped on the note this long ago: */
			//fTimeSinceStep
		
			/* GAMESTATE->m_fMusicSeconds is the music time as of GAMESTATE->m_LastBeatUpdate. Figure
			 * out what the music time is as of now. */
			const float fCurrentMusicSeconds = GAMESTATE->m_fMusicSeconds + (fLastBeatUpdate*GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate);

			/* ... which means it happened at this point in the music: */
			const float fMusicSeconds = fCurrentMusicSeconds - fTimeSinceStep * GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;

			// The offset from the actual step in seconds:
			fNoteOffset = (fStepSeconds - fMusicSeconds) / GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;	// account for music rate
//			LOG->Trace("step was %.3f ago, music is off by %f: %f vs %f, step was %f off", 
//				fTimeSinceStep, GAMESTATE->m_LastBeatUpdate.Ago()/GAMESTATE->m_SongOptions.m_fMusicRate,
//				fStepSeconds, fMusicSeconds, fNoteOffset );
		}

		const float fSecondsFromExact = fabsf( fNoteOffset );

		TapNote tnDummy = TAP_ORIGINAL_TAP;
		TapNote *pTN = NULL;
		switch( pbt )
		{
		DEFAULT_FAIL(pbt);
		case ButtonType_StrumFretsChanged:
			pTN = &tnDummy;
			break;
		case ButtonType_Hopo:
		case ButtonType_Step:
			NoteData::iterator iter = m_NoteData.FindTapNote( col, iRowOfOverlappingNoteOrRow );
			DEBUG_ASSERT( iter!= m_NoteData.end(col) );
			pTN = &iter->second;
			break;
		}

		switch( m_pPlayerState->m_PlayerController )
		{
		case PC_HUMAN:
			switch( pTN->type )
			{
			case TapNote::mine:
				// Stepped too close to mine?
				if( !bRelease && fSecondsFromExact <= GetWindowSeconds(TW_Mine) )
					score = TNS_HitMine;
				break;

			case TapNote::attack:
				if( !bRelease && fSecondsFromExact <= GetWindowSeconds(TW_Attack) && !pTN->result.bHidden )
					score = TNS_W2; /* sentinel */
				break;

			default:
				if( (pTN->type == TapNote::lift) == bRelease )
				{
					if(	 fSecondsFromExact <= GetWindowSeconds(TW_W1) )	score = TNS_W1;
					else if( fSecondsFromExact <= GetWindowSeconds(TW_W2) )	score = TNS_W2;
					else if( fSecondsFromExact <= GetWindowSeconds(TW_W3) )	score = TNS_W3;
					else if( fSecondsFromExact <= GetWindowSeconds(TW_W4) )	score = TNS_W4;
					else if( fSecondsFromExact <= GetWindowSeconds(TW_W5) )	score = TNS_W5;
				}
				break;
			}
			break;
		
		case PC_CPU:
		case PC_AUTOPLAY:
			score = PlayerAI::GetTapNoteScore( m_pPlayerState );

			/* XXX: This doesn't make sense. Step should only be called in autoplay for hit notes */
#if 0
			// GetTapNoteScore always returns TNS_W1 in autoplay.
			// If the step is far away, don't judge it.
			if( m_pPlayerState->m_PlayerController == PC_AUTOPLAY &&
				fSecondsFromExact > GetWindowSeconds(TW_W5) )
			{
				score = TNS_None;
				break;
			}
#endif

			// TRICKY:  We're asking the AI to judge mines.  consider TNS_W4 and below
			// as "mine was hit" and everything else as "mine was avoided"
			if( pTN->type == TapNote::mine )
			{
				// The CPU hits a lot of mines.  Only consider hitting the 
				// first mine for a row.  We know we're the first mine if 
				// there are are no mines to the left of us.
				for( int t=0; t<col; t++ )
				{
					if( m_NoteData.GetTapNote(t,iRowOfOverlappingNoteOrRow).type == TapNote::mine )	// there's a mine to the left of us
						return;	// avoid
				}

				// The CPU hits a lot of mines.  Make it less likely to hit 
				// mines that don't have a tap note on the same row.
				bool bTapsOnRow = m_NoteData.IsThereATapOrHoldHeadAtRow( iRowOfOverlappingNoteOrRow );
				TapNoteScore get_to_avoid = bTapsOnRow ? TNS_W3 : TNS_W4;

				if( score >= get_to_avoid )
					return;	// avoided
				else
					score = TNS_HitMine;
			}

			if( pTN->type == TapNote::attack && score > TNS_W4 )
				score = TNS_W2; /* sentinel */

			/* AI will generate misses here.  Don't handle a miss like a regular note because
			 * we want the judgment animation to appear delayed.  Instead, return early if
			 * AI generated a miss, and let UpdateTapNotesMissedOlderThan() detect and handle the 
			 * misses. */
			if( score == TNS_Miss )
				return;

			// Put some small, random amount in fNoteOffset so that demonstration 
			// show a mix of late and early.
			fNoteOffset = randomf( -0.1f, 0.1f );

			break;

		default:
			ASSERT(0);
			score = TNS_None;
			break;
		}

		
		switch( pbt )
		{
		DEFAULT_FAIL(pbt);
		case ButtonType_StrumFretsChanged:
			{
				bool bNoteRowMatchesFrets = true;
				int iFirstNoteCol = -1;
				for( int i=0; i<GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer; i++ )
				{
					const TapNote &tn = m_NoteData.GetTapNote( i, iRowOfOverlappingNoteOrRow );
					bool bIsNote = (tn.type != TapNote::empty);
					if( iFirstNoteCol == -1  &&  bIsNote )
						iFirstNoteCol = i;

					// Extra notes to the left (higher up on the string) can be held without penalty.  It's necessary to hold
					// the extra frets or pull-offs.
					if( iFirstNoteCol == -1 )
						continue;

					bool bNoteMatchesFret = m_vbFretIsDown[i] == bIsNote;
					if( !bNoteMatchesFret )
					{
						bNoteRowMatchesFrets = false;
						break;
					}
				}
				ASSERT( iFirstNoteCol != -1 );
				if( !bNoteRowMatchesFrets )
				{
					score = TNS_None;
				}
				else
				{
					int iLastNoteCol = -1;
					for( int i=GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer-1; i>=0; i-- )
					{
						const TapNote &tn = m_NoteData.GetTapNote( i, iRowOfOverlappingNoteOrRow );
						bool bIsNote = (tn.type != TapNote::empty);
						if( bIsNote )
						{
							iLastNoteCol = i;
							break;
						}
					}

					m_pPlayerState->m_fLastHopoNoteMusicSeconds = fStepSeconds;
					m_pPlayerState->m_iLastHopoNoteCol = iLastNoteCol; 
				}
			}
			break;
		case ButtonType_Hopo:
			{
				bool bDidHopo = true;

				{
					// only can hopo on a row with one note
					if( m_NoteData.GetNumTapNotesInRow(iRowOfOverlappingNoteOrRow) != 1 )
					{
						bDidHopo = false;
						goto done_checking_hopo;
					}

					// con't hopo on the same note 2x in a row
					if( col == m_pPlayerState->m_iLastHopoNoteCol )
					{
						bDidHopo = false;
						goto done_checking_hopo;
					}

					const TapNote &tn = m_NoteData.GetTapNote( col, iRowOfOverlappingNoteOrRow );
					ASSERT( tn.type != TapNote::empty );

					int iRowsAgoLastNote = 100000;	// TODO: find more reasonable value based on HOPO_CHAIN_SECONDS?
					NoteData::all_tracks_reverse_iterator iter = m_NoteData.GetTapNoteRangeAllTracksReverse( iRowsAgoLastNote-iRowsAgoLastNote, iRowOfOverlappingNoteOrRow-1 );
					ASSERT( !iter.IsAtEnd() );	// there must have been a note that started the hopo
					if( !NoteDataWithScoring::IsRowCompletelyJudged(m_NoteData, iter.Row(), m_pPlayerState->m_PlayerNumber) )
					{
						bDidHopo = false;
						goto done_checking_hopo;
					}

					const TapNoteResult &lastTNR = NoteDataWithScoring::LastTapNoteWithResult( m_NoteData, iter.Row(), m_pPlayerState->m_PlayerNumber ).result;
					if( lastTNR.tns <= TNS_Miss )
					{
						bDidHopo = false;
						goto done_checking_hopo;
					}
				}
done_checking_hopo:
				if( !bDidHopo )
				{
					score = TNS_None;
				}
				else
				{
					m_pPlayerState->m_fLastHopoNoteMusicSeconds = fStepSeconds;
					m_pPlayerState->m_iLastHopoNoteCol = col;
				}
			}
			break;
		case ButtonType_Step:
			break;
		}


		if( pTN->type == TapNote::attack && score == TNS_W2 )
		{
			score = TNS_None;	// don't score this as anything

			m_soundAttackLaunch.Play();

			// put attack in effect
			Attack attack(
				ATTACK_LEVEL_1,
				-1,	// now
				pTN->fAttackDurationSeconds,
				pTN->sAttackModifiers,
				true,
				false
				);

			// TODO: Remove use of PlayerNumber
			PlayerNumber pnToAttack = OPPOSITE_PLAYER[m_pPlayerState->m_PlayerNumber];
			PlayerState *pPlayerStateToAttack = GAMESTATE->m_pPlayerState[pnToAttack];
			pPlayerStateToAttack->LaunchAttack( attack );

			// remove all TapAttacks on this row
			for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
			{
				const TapNote &tn = m_NoteData.GetTapNote( t, iRowOfOverlappingNoteOrRow );
				if( tn.type == TapNote::attack )
					HideNote( t, iRowOfOverlappingNoteOrRow );
			}
		}

		if( m_pPlayerState->m_PlayerController == PC_HUMAN && score >= TNS_W3 ) 
			AdjustSync::HandleAutosync( fNoteOffset, fStepSeconds );

		// Do game-specific and mode-specific score mapping.
		score = GAMESTATE->GetCurrentGame()->MapTapNoteScore( score );
		if( score == TNS_W1 && !GAMESTATE->ShowW1() )
			score = TNS_W2;


		if( score != TNS_None )
		{
			switch( pbt )
			{
			DEFAULT_FAIL(pbt);
			case ButtonType_StrumFretsChanged:
				for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
				{
					TapNote tn = m_NoteData.GetTapNote( t, iRowOfOverlappingNoteOrRow );
					if( tn.type != TapNote::empty )
					{
						tn.result.tns = score;
						tn.result.fTapNoteOffset = -fNoteOffset;
						m_NoteData.SetTapNote( t, iRowOfOverlappingNoteOrRow, tn );
					}
				}
				break;
			case ButtonType_Hopo:
			case ButtonType_Step:
				pTN->result.tns = score;
				pTN->result.fTapNoteOffset = -fNoteOffset;
				break;
			}
		}

		// TODO: Remove use of PlayerNumber
		PlayerNumber pn = pTN->pn == PLAYER_INVALID ? m_pPlayerState->m_PlayerNumber : pTN->pn;
		m_LastTapNoteScore = score;
		if( GAMESTATE->GetCurrentGame()->m_bCountNotesSeparately )
		{
			if( pTN->type != TapNote::mine )
			{
				const bool bBlind = (m_pPlayerState->m_PlayerOptions.GetCurrent().m_fBlind != 0);
				// XXX This is the wrong combo for shared players. STATSMAN->m_CurStageStats.m_Player[pn] might work but could be wrong.
				const bool bBright = m_pPlayerStageStats && m_pPlayerStageStats->m_iCurCombo > int(BRIGHT_GHOST_COMBO_THRESHOLD) || bBlind;			
				if( m_pNoteField )
					m_pNoteField->DidTapNote( col, score, bBright );
				if( score >= TNS_W3 || bBlind )
					HideNote( col, iRowOfOverlappingNoteOrRow );
			}
		}
		else if( NoteDataWithScoring::IsRowCompletelyJudged(m_NoteData, iRowOfOverlappingNoteOrRow, pn) )
		{
			FlashGhostRow( iRowOfOverlappingNoteOrRow, pn );
		}
	}

	// check for hopo end
	if( score <= TNS_Miss )
	{
		m_pPlayerState->ClearHopoState();
	}

	// check for strum end
	if( score != TNS_None )
	{
		switch( pbt )
		{
		DEFAULT_FAIL(pbt);
		case ButtonType_Step:
			break;
		case ButtonType_StrumFretsChanged:
			m_pPlayerState->m_fLastStrumMusicSeconds = -1;
			break;
		case ButtonType_Hopo:
			break;
		}
	}

	if( score == TNS_None )
	{
		switch( pbt )
		{
		DEFAULT_FAIL(pbt);
		case ButtonType_Step:
			DoTapScoreNone();
			break;
		case ButtonType_StrumFretsChanged:
		case ButtonType_Hopo:
			break;
		}

	}

	{
		/* Search for keyed sounds separately.  Play the nearest note. */
		/* XXX: This isn't quite right. As per the above XXX for iRowOfOverlappingNote, if iRowOfOverlappingNote
		 * is set to a previous note, the keysound could have changed and this would cause the wrong one to play,
		 * in essence playing two sounds in the opposite order. Maybe this should always perform the search. Still,
		 * even that doesn't seem quite right since it would then play the same (new) keysound twice which would
		 * sound wrong even though the notes were judged as being correct, above. Fixing the above problem would
		 * fix this one as well. */
		if( iRowOfOverlappingNoteOrRow == -1 )
			iRowOfOverlappingNoteOrRow = GetClosestNote( col, iSongRow, MAX_NOTE_ROW, MAX_NOTE_ROW, true );
		if( iRowOfOverlappingNoteOrRow != -1 )
		{
			switch( pbt )
			{
			DEFAULT_FAIL(pbt);
			case ButtonType_StrumFretsChanged:
				for( int i=0; i<m_NoteData.GetNumTracks(); i++ )
				{
					const TapNote &tn = m_NoteData.GetTapNote( i, iRowOfOverlappingNoteOrRow );
					if( tn.iKeysoundIndex >= 0 && tn.iKeysoundIndex < (int) m_vKeysounds.size() )
						m_vKeysounds[tn.iKeysoundIndex].Play();
				}
				break;
			case ButtonType_Step:
			case ButtonType_Hopo:
				const TapNote &tn = m_NoteData.GetTapNote( col, iRowOfOverlappingNoteOrRow );
				if( tn.iKeysoundIndex >= 0 && tn.iKeysoundIndex < (int) m_vKeysounds.size() )
					m_vKeysounds[tn.iKeysoundIndex].Play();
				break;
			}
		}
	}
	// XXX:
	if( !bRelease )
	{
		if( m_pNoteField )
		{
			switch( pbt )
			{
			DEFAULT_FAIL(pbt);
			case ButtonType_StrumFretsChanged:
				{
					// only pulse the shortest string fret
					int iLastFret = -1;
					for( int i=0; i<m_NoteData.GetNumTracks(); i++ )
					{
						if( m_vbFretIsDown[i] )
							iLastFret = i;
					}
					if( iLastFret != -1 ) 
						m_pNoteField->Step( iLastFret, score );
				}
				break;
			case ButtonType_Step:
				m_pNoteField->Step( col, score );
				break;
			case ButtonType_Hopo:
				// no animation
				break;
			}
		}
		Message msg( "Step" );
		msg.SetParam( "PlayerNumber", m_pPlayerState->m_PlayerNumber );
		msg.SetParam( "MultiPlayer", m_pPlayerState->m_mp );
		MESSAGEMAN->Broadcast( msg );
	}
}

void Player::UpdateTapNotesMissedOlderThan( float fMissIfOlderThanSeconds )
{
	//LOG->Trace( "Steps::UpdateTapNotesMissedOlderThan(%f)", fMissIfOlderThanThisBeat );
	int iMissIfOlderThanThisRow;
	{
		const float fEarliestTime = GAMESTATE->m_fMusicSeconds - fMissIfOlderThanSeconds;
		bool bFreeze;
		float fMissIfOlderThanThisBeat;
		float fThrowAway;
		GAMESTATE->m_pCurSong->m_Timing.GetBeatAndBPSFromElapsedTime( fEarliestTime, fMissIfOlderThanThisBeat, fThrowAway, bFreeze );

		iMissIfOlderThanThisRow = BeatToNoteRow( fMissIfOlderThanThisBeat );
		if( bFreeze )
		{
			/* If there is a freeze on iMissIfOlderThanThisIndex, include this index too.
			 * Otherwise we won't show misses for tap notes on freezes until the
			 * freeze finishes. */
			iMissIfOlderThanThisRow++;
		}
	}

	NoteData::all_tracks_iterator &iter = *m_pIterNeedsTapJudging;

	for( ; !iter.IsAtEnd() && iter.Row() < iMissIfOlderThanThisRow; ++iter )
	{
		TapNote &tn = *iter;

		if( !NeedsTapJudging(tn) )
			continue;
		
		if( tn.pn != PLAYER_INVALID && tn.pn != m_pPlayerState->m_PlayerNumber )
			continue;
		if( tn.type == TapNote::mine )
		{
			tn.result.tns = TNS_AvoidMine;
			
			/* The only real way to tell if a mine has been scored is if it has disappeared
			 * but this only works for hit mines so update the scores for avoided mines here. */
			if( m_pPrimaryScoreKeeper )
				m_pPrimaryScoreKeeper->HandleTapScore( tn );
			if( m_pSecondaryScoreKeeper )
				m_pSecondaryScoreKeeper->HandleTapScore( tn );
		}
		else
		{
			tn.result.tns = TNS_Miss;
		}
	}
}

void Player::UpdateJudgedRows()
{
	const int iEndRow = BeatToNoteRow( GAMESTATE->m_fSongBeat );
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	bool bAllJudged = true;
	const bool bSeparately = GAMESTATE->GetCurrentGame()->m_bCountNotesSeparately;

	{
	NoteData::all_tracks_iterator iter = *m_pIterUnjudgedRows;
	int iLastSeenRow = -1;
	for( ; !iter.IsAtEnd()  &&  iter.Row() <= iEndRow; ++iter )
	{
		int iRow = iter.Row();

		if( iLastSeenRow != iRow )
		{
			iLastSeenRow = iRow;

			// crossed a not-empty row
			if( !NoteDataWithScoring::IsRowCompletelyJudged(m_NoteData, iRow, pn) )
			{
				bAllJudged = false;
				continue;
			}
			if( bAllJudged )
				*m_pIterUnjudgedRows = iter;
			if( m_pJudgedRows->JudgeRow(iRow) )
				continue;
			const TapNoteResult &lastTNR = NoteDataWithScoring::LastTapNoteWithResult( m_NoteData, iRow, pn ).result;
			
			if( lastTNR.tns < TNS_Miss )
				continue;
			if( bSeparately )
			{
				for( int iTrack = 0; iTrack < m_NoteData.GetNumTracks(); ++iTrack )
				{
					const TapNote &tn = m_NoteData.GetTapNote( iTrack, iRow );
					if( tn.type == TapNote::empty || tn.type == TapNote::mine ) continue;
					if( tn.pn != PLAYER_INVALID && tn.pn != pn ) continue;
					SetJudgment( tn.result.tns, tn.result.fTapNoteOffset );
				}
			}
			else
			{
				SetJudgment( lastTNR.tns, lastTNR.fTapNoteOffset );
			}
			HandleTapRowScore( iRow );
		}
	}
	}

	{
		bAllJudged = true;
		set<RageSound *> setSounds;
		NoteData::all_tracks_iterator iter = *m_pIterUnjudgedMineRows;	// copy
		int iLastSeenRow = -1;
		for( ; !iter.IsAtEnd()  &&  iter.Row() <= iEndRow; ++iter )
		{
			int iRow = iter.Row();
			TapNote &tn = *iter;

			if( iRow != iLastSeenRow )
			{
				iLastSeenRow = iRow;
				if( bAllJudged )
					*m_pIterUnjudgedMineRows = iter;
			}

			bool bMineNotHidden = tn.type == TapNote::mine && !tn.result.bHidden;
			if( !bMineNotHidden )
				continue;

			switch( tn.result.tns )
			{
			DEFAULT_FAIL( tn.result.tns );
			case TNS_None:		
				bAllJudged = false;
				continue;
			case TNS_AvoidMine:
				continue;
			case TNS_HitMine:
				break;
			}
			if( m_pNoteField )
				m_pNoteField->DidTapNote( iter.Track(), tn.result.tns, false );
			
			if( tn.pn != PLAYER_INVALID && tn.pn != pn )
				continue;
			if( tn.iKeysoundIndex >= 0 && tn.iKeysoundIndex < (int) m_vKeysounds.size() )
				setSounds.insert( &m_vKeysounds[tn.iKeysoundIndex] );
			else
				setSounds.insert( &m_soundMine );
			
			ChangeLife( tn.result.tns );
			if( m_pScoreDisplay )
				m_pScoreDisplay->OnJudgment( tn.result.tns );
			if( m_pSecondaryScoreDisplay )
				m_pSecondaryScoreDisplay->OnJudgment( tn.result.tns );
			
			// Make sure hit mines affect the dance points.
			if( m_pPrimaryScoreKeeper )
				m_pPrimaryScoreKeeper->HandleTapScore( tn );
			if( m_pSecondaryScoreKeeper )
				m_pSecondaryScoreKeeper->HandleTapScore( tn );
			tn.result.bHidden = true;
		}

		FOREACHS( RageSound *, setSounds, s )
		{
			/* Only play one copy of each mine sound at a time per player. */
			(*s)->Stop();
			(*s)->Play();
		}
	}
}

void Player::FlashGhostRow( int iRow, PlayerNumber pn )
{
	TapNoteScore lastTNS = NoteDataWithScoring::LastTapNoteWithResult( m_NoteData, iRow, pn ).result.tns;
	const bool bBlind = (m_pPlayerState->m_PlayerOptions.GetCurrent().m_fBlind != 0);
	// XXX This is the wrong combo for shared players. STATSMAN->m_CurStageStats.m_Player[pn] might work but could be wrong.
	const bool bBright = m_pPlayerStageStats && m_pPlayerStageStats->m_iCurCombo > int(BRIGHT_GHOST_COMBO_THRESHOLD) || bBlind;
		
	for( int iTrack = 0; iTrack < m_NoteData.GetNumTracks(); ++iTrack )
	{
		const TapNote &tn = m_NoteData.GetTapNote( iTrack, iRow );
		
		if( tn.type == TapNote::empty || tn.type == TapNote::mine )
			continue;
		if( tn.pn != PLAYER_INVALID && tn.pn != pn )
			continue;
		if( m_pNoteField )
			m_pNoteField->DidTapNote( iTrack, lastTNS, bBright );
		if( lastTNS >= TNS_W3 || bBlind )
			HideNote( iTrack, iRow );
	}
}

void Player::CrossedRows( int iLastRowCrossed, const RageTimer &now )
{
	//LOG->Trace( "Player::CrossedRows   %d    %d", iFirstRowCrossed, iLastRowCrossed );

	NoteData::all_tracks_iterator &iter = *m_pIterUncrossedRows;
	int iLastSeenRow = -1;
	for( ; !iter.IsAtEnd()  &&  iter.Row() <= iLastRowCrossed; ++iter )
	{
		/* Apply InitialHoldLife. */
		TapNote &tn = *iter;
		int iRow = iter.Row();
		int iTrack = iter.Track();
		switch( tn.type )
		{
		case TapNote::hold_head:
			tn.HoldResult.fLife = INITIAL_HOLD_LIFE;
			break;
		case TapNote::mine:
			// Hold the panel while crossing a mine will cause the mine to explode
			// TODO: Remove use of PlayerNumber.
			PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
			GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( iTrack, pn );
			if( PREFSMAN->m_fPadStickSeconds > 0 )
			{
				float fSecsHeld = INPUTMAPPER->GetSecsHeld( GameI, m_pPlayerState->m_mp );
				if( fSecsHeld >= PREFSMAN->m_fPadStickSeconds )
					Step( iTrack, -1, now - PREFSMAN->m_fPadStickSeconds, true, false );
			}
			else
			{
				if( INPUTMAPPER->IsBeingPressed(GameI, m_pPlayerState->m_mp) )
					Step( iTrack, iRow, now, true, false );
			}
			break;
		}


		if( iRow != iLastSeenRow )
		{
			// crossed a not-empty row

			// If we're doing random vanish, randomise notes on the fly.
			if( m_pPlayerState->m_PlayerOptions.GetCurrent().m_fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH]==1 )
				RandomizeNotes( iRow );

			// check to see if there's a note at the crossed row
			if( m_pPlayerState->m_PlayerController != PC_HUMAN )
			{
				if( tn.type != TapNote::empty && tn.result.tns == TNS_None )
				{
					Step( iTrack, iRow, now, false, false );
					if( m_pPlayerState->m_PlayerController == PC_AUTOPLAY )
					{
						STATSMAN->m_CurStageStats.m_bUsedAutoplay = true;
						if( m_pPlayerStageStats )
							m_pPlayerStageStats->m_bDisqualified = true;
					}
				}
			}
		}
	}


	//
	// Update hold checkpoints
	//
	if( HOLD_CHECKPOINTS )
	{
		const int CHECKPOINT_FREQUENCY_ROWS = ROWS_PER_BEAT/2;

		// "the first row after the start of the range that lands on a beat"
		int iFirstCheckpointInRange = ((m_iFirstUncrossedRow+CHECKPOINT_FREQUENCY_ROWS-1)/CHECKPOINT_FREQUENCY_ROWS) * CHECKPOINT_FREQUENCY_ROWS;
		
		// "the last row or first row earlier that lands on a beat"
		int iLastCheckpointInRange = ((iLastRowCrossed)/CHECKPOINT_FREQUENCY_ROWS) * CHECKPOINT_FREQUENCY_ROWS;

		for( int r = iFirstCheckpointInRange; r <= iLastCheckpointInRange; r += CHECKPOINT_FREQUENCY_ROWS )
		{
			//LOG->Trace( "%d...", r );
			vector<int> viColsWithHold;
			int iNumHoldsHeldThisRow = 0;
			int iNumHoldsMissedThisRow = 0;

			// start at r-1 so that we consider holds whose end rows are equal to the checkpoint row
			NoteData::all_tracks_iterator iter = m_NoteData.GetTapNoteRangeAllTracks( r-1, r, true );
			for( ; !iter.IsAtEnd(); ++iter )
			{
				TapNote &tn = *iter;
				if( tn.type != TapNote::hold_head )
					continue;

				int iStartRow = iter.Row();
				int iEndRow = iStartRow + tn.iDuration;
				int iTrack = iter.Track();

				// "the first row after the hold head that lands on a beat"
				int iFirstCheckpointOfHold = ((iStartRow+CHECKPOINT_FREQUENCY_ROWS)/CHECKPOINT_FREQUENCY_ROWS) * CHECKPOINT_FREQUENCY_ROWS;
				
				// "the end row or the first earlier row that lands on a beat"
				int iLastCheckpointOfHold = ((iEndRow)/CHECKPOINT_FREQUENCY_ROWS) * CHECKPOINT_FREQUENCY_ROWS;

				// count the end of the hold as a checkpoint
				bool bHoldOverlapsRow = iFirstCheckpointOfHold <= r  &&   r <= iLastCheckpointOfHold;
				if( !bHoldOverlapsRow )
					continue;

				viColsWithHold.push_back( iTrack );
				if( tn.HoldResult.fLife > 0 )
				{
					++iNumHoldsHeldThisRow;
					++tn.HoldResult.iCheckpointsHit;
				}
				else
				{
					++iNumHoldsMissedThisRow;
					++tn.HoldResult.iCheckpointsMissed;
				}
			}

			if( !viColsWithHold.empty() )
			{
				HandleHoldCheckpoint( r, iNumHoldsHeldThisRow, iNumHoldsMissedThisRow, viColsWithHold );
			}
		}
	}

	m_iFirstUncrossedRow = iLastRowCrossed+1;
}

void Player::RandomizeNotes( int iNoteRow )
{
	// change the row to look ahead from based upon their speed mod
	/* This is incorrect: if m_fScrollSpeed is 0.5, we'll never change
	 * any odd rows, and if it's 2, we'll shuffle each row twice. */
	int iNewNoteRow = iNoteRow + ROWS_PER_BEAT*2;
	iNewNoteRow = int( iNewNoteRow / m_pPlayerState->m_PlayerOptions.GetCurrent().m_fScrollSpeed );

	int iNumOfTracks = m_NoteData.GetNumTracks();
	for( int t=0; t+1 < iNumOfTracks; t++ )
	{
		const int iSwapWith = RandomInt( iNumOfTracks );

		/* Only swap a tap and an empty. */
		NoteData::iterator iter = m_NoteData.FindTapNote( t, iNewNoteRow );
		if( iter == m_NoteData.end(t) || iter->second.type != TapNote::tap )
			continue;

		// Make sure this is empty.
		if( m_NoteData.FindTapNote(iSwapWith, iNewNoteRow) != m_NoteData.end(iSwapWith) )
			continue;

		/* Make sure the destination row isn't in the middle of a hold. */
		if( m_NoteData.IsHoldNoteAtRow(iSwapWith, iNoteRow) )
			continue;
		
		m_NoteData.SetTapNote( iSwapWith, iNewNoteRow, iter->second );
		m_NoteData.RemoveTapNote( t, iter );
	}
}

void Player::HandleTapRowScore( unsigned row )
{
	bool bNoCheating = true;
#ifdef DEBUG
	bNoCheating = false;
#endif

	if( GAMESTATE->m_bDemonstrationOrJukebox )
		bNoCheating = false;
	// don't accumulate points if AutoPlay is on.
	if( bNoCheating && m_pPlayerState->m_PlayerController == PC_AUTOPLAY )
		return;

	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	TapNoteScore scoreOfLastTap = NoteDataWithScoring::LastTapNoteWithResult(m_NoteData, row, pn).result.tns;
	const int iOldCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurCombo : 0;
	const int iOldMissCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurMissCombo : 0;

	if( scoreOfLastTap == TNS_Miss )
		m_LastTapNoteScore = TNS_Miss;

	for( int track = 0; track < m_NoteData.GetNumTracks(); ++track )
	{
		const TapNote &tn = m_NoteData.GetTapNote( track, row );
		// Mines cannot be handled here.
		if( tn.type == TapNote::empty || tn.type == TapNote::mine )
			continue;
		if( tn.pn != PLAYER_INVALID && tn.pn != pn )
			continue;
		if( m_pPrimaryScoreKeeper )
			m_pPrimaryScoreKeeper->HandleTapScore( tn );
		if( m_pSecondaryScoreKeeper )
			m_pSecondaryScoreKeeper->HandleTapScore( tn );
	}		

	if( m_pPrimaryScoreKeeper != NULL )
		m_pPrimaryScoreKeeper->HandleTapRowScore( m_NoteData, row );
	if( m_pSecondaryScoreKeeper != NULL )
		m_pSecondaryScoreKeeper->HandleTapRowScore( m_NoteData, row );

	const int iCurCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurCombo : 0;
	const int iCurMissCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurMissCombo : 0;

	SendComboMessages( iOldCombo, iOldMissCombo );

	if( m_pPlayerStageStats )
	{
		SetCombo( iCurCombo, iCurMissCombo );
	}

#define CROSSED( x ) (iOldCombo<x && iCurCombo>=x)
	if ( CROSSED(100) )	
		SCREENMAN->PostMessageToTopScreen( SM_100Combo, 0 );
	else if( CROSSED(200) )	
		SCREENMAN->PostMessageToTopScreen( SM_200Combo, 0 );
	else if( CROSSED(300) )	
		SCREENMAN->PostMessageToTopScreen( SM_300Combo, 0 );
	else if( CROSSED(400) )	
		SCREENMAN->PostMessageToTopScreen( SM_400Combo, 0 );
	else if( CROSSED(500) )	
		SCREENMAN->PostMessageToTopScreen( SM_500Combo, 0 );
	else if( CROSSED(600) )	
		SCREENMAN->PostMessageToTopScreen( SM_600Combo, 0 );
	else if( CROSSED(700) )	
		SCREENMAN->PostMessageToTopScreen( SM_700Combo, 0 );
	else if( CROSSED(800) )	
		SCREENMAN->PostMessageToTopScreen( SM_800Combo, 0 );
	else if( CROSSED(900) )	
		SCREENMAN->PostMessageToTopScreen( SM_900Combo, 0 );
	else if( CROSSED(1000))	
		SCREENMAN->PostMessageToTopScreen( SM_1000Combo, 0 );
	else if( (iOldCombo / 100) < (iCurCombo / 100) && iCurCombo > 1000 )
		SCREENMAN->PostMessageToTopScreen( SM_ComboContinuing, 0 );
#undef CROSSED

	// new max combo
	if( m_pPlayerStageStats )
		m_pPlayerStageStats->m_iMaxCombo = max(m_pPlayerStageStats->m_iMaxCombo, iCurCombo);

	/*
	 * Use the real current beat, not the beat we've been passed.  That's because we
	 * want to record the current life/combo to the current time; eg. if it's a MISS,
	 * the beat we're registering is in the past, but the life is changing now.
	 *
	 * We need to include time from previous songs in a course, so we can't use
	 * GAMESTATE->m_fMusicSeconds.  Use fStepsSeconds instead.
	 */
	if( m_pPlayerStageStats )
		m_pPlayerStageStats->UpdateComboList( STATSMAN->m_CurStageStats.m_fStepsSeconds, false );

	if( m_pScoreDisplay )
	{
		if( m_pPlayerStageStats )
			m_pScoreDisplay->SetScore( m_pPlayerStageStats->m_iScore );
		m_pScoreDisplay->OnJudgment( scoreOfLastTap );
	}
	if( m_pSecondaryScoreDisplay )
	{
		if( m_pPlayerStageStats )
			m_pSecondaryScoreDisplay->SetScore( m_pPlayerStageStats->m_iScore );
		m_pSecondaryScoreDisplay->OnJudgment( scoreOfLastTap );
	}

	ChangeLife( scoreOfLastTap );
}

void Player::HandleHoldCheckpoint( int iRow, int iNumHoldsHeldThisRow, int iNumHoldsMissedThisRow, const vector<int> &viColsWithHold )
{
	const int iOldCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurCombo : 0;
	const int iOldMissCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurMissCombo : 0;

	if( m_pPrimaryScoreKeeper )
		m_pPrimaryScoreKeeper->HandleHoldCheckpointScore( m_NoteData, iRow, iNumHoldsHeldThisRow, iNumHoldsMissedThisRow );
	if( m_pSecondaryScoreKeeper )
		m_pSecondaryScoreKeeper->HandleHoldCheckpointScore( m_NoteData, iRow, iNumHoldsHeldThisRow, iNumHoldsMissedThisRow );

	if( iNumHoldsMissedThisRow == 0 )
	{
		FOREACH_CONST( int, viColsWithHold, i )
		{
			bool bBright = m_pPlayerStageStats && m_pPlayerStageStats->m_iCurCombo>(int)BRIGHT_GHOST_COMBO_THRESHOLD;
			if( m_pNoteField )
				m_pNoteField->DidHoldNote( *i, HNS_Held, bBright );
		}
	}

	SendComboMessages( iOldCombo, iOldMissCombo );

	if( m_pPlayerStageStats )
	{
		SetCombo( m_pPlayerStageStats->m_iCurCombo, m_pPlayerStageStats->m_iCurMissCombo );
	}

	if( m_pPlayerStageStats )
		m_pPlayerStageStats->UpdateComboList( STATSMAN->m_CurStageStats.m_fStepsSeconds, false );

	ChangeLife( iNumHoldsMissedThisRow == 0? TNS_CheckpointHit:TNS_CheckpointMiss );

	SetJudgment( iNumHoldsMissedThisRow == 0? TNS_CheckpointHit:TNS_CheckpointMiss, 0 );
}

void Player::HandleHoldScore( const TapNote &tn )
{
	HoldNoteScore holdScore = tn.HoldResult.hns;
	TapNoteScore tapScore = tn.result.tns;
	bool bNoCheating = true;
#ifdef DEBUG
	bNoCheating = false;
#endif

	if( GAMESTATE->m_bDemonstrationOrJukebox )
		bNoCheating = false;
	// don't accumulate points if AutoPlay is on.
	if( bNoCheating && m_pPlayerState->m_PlayerController == PC_AUTOPLAY )
		return;

	if( m_pPrimaryScoreKeeper )
		m_pPrimaryScoreKeeper->HandleHoldScore( tn );
	if( m_pSecondaryScoreKeeper )
		m_pSecondaryScoreKeeper->HandleHoldScore( tn );

	if( m_pScoreDisplay )
	{
		if( m_pPlayerStageStats ) 
			m_pScoreDisplay->SetScore( m_pPlayerStageStats->m_iScore );
		m_pScoreDisplay->OnJudgment( holdScore, tapScore );
	}
	if( m_pSecondaryScoreDisplay )
	{
		if( m_pPlayerStageStats ) 
			m_pSecondaryScoreDisplay->SetScore( m_pPlayerStageStats->m_iScore );
		m_pSecondaryScoreDisplay->OnJudgment( holdScore, tapScore );
	}

	ChangeLife( holdScore, tapScore );
}

float Player::GetMaxStepDistanceSeconds()
{
	float fMax = 0;
	fMax = max( fMax, GetWindowSeconds(TW_W5) );
	fMax = max( fMax, GetWindowSeconds(TW_W4) );
	fMax = max( fMax, GetWindowSeconds(TW_W3) );
	fMax = max( fMax, GetWindowSeconds(TW_W2) );
	fMax = max( fMax, GetWindowSeconds(TW_W1) );
	return GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate * fMax;
}

void Player::FadeToFail()
{
	if( m_pNoteField )
		m_pNoteField->FadeToFail();

	// clear miss combo
	SetCombo( 0, 0 );
}

void Player::CacheAllUsedNoteSkins()
{
	if( m_pNoteField )
		m_pNoteField->CacheAllUsedNoteSkins();
}

void Player::SetJudgment( TapNoteScore tns, float fTapNoteOffset )
{
	Message msg("Judgment");
	msg.SetParam( "Player", m_pPlayerState->m_PlayerNumber );
	msg.SetParam( "MultiPlayer", m_pPlayerState->m_mp );
	msg.SetParam( "TapNoteScore", tns );
	msg.SetParam( "Early", fTapNoteOffset < 0.0f );
	msg.SetParam( "TapNoteOffset", fTapNoteOffset );
	MESSAGEMAN->Broadcast( msg );
}

void Player::SetHoldJudgment( TapNoteScore tns, HoldNoteScore hns, int iTrack )
{
	ASSERT( iTrack < (int)m_vpHoldJudgment.size() );
	if( m_vpHoldJudgment[iTrack] )
		m_vpHoldJudgment[iTrack]->SetHoldJudgment( hns );

	Message msg("Judgment");
	msg.SetParam( "Player", m_pPlayerState->m_PlayerNumber );
	msg.SetParam( "MultiPlayer", m_pPlayerState->m_mp );
	msg.SetParam( "TapNoteScore", tns );
	msg.SetParam( "HoldNoteScore", hns );
	MESSAGEMAN->Broadcast( msg );
}

void Player::SetCombo( int iCombo, int iMisses )
{
	if( m_iLastSeenCombo == -1 )	// first update, don't set bIsMilestone=true
		m_iLastSeenCombo = iCombo;

	bool b100Milestone = false;
	bool b1000Milestone = false;
	for( int i=m_iLastSeenCombo+1; i<=iCombo; i++ )
	{
		if( i < 600 )
			b100Milestone |= ((i % 100) == 0);
		else
			b1000Milestone |= ((i % 200) == 0);
	}
	m_iLastSeenCombo = iCombo;

	if( b100Milestone )
		this->PlayCommand( "100Milestone" );
	if( b1000Milestone )
		this->PlayCommand( "1000Milestone" );

	// don't show a colored combo until 1/4 of the way through the song
	bool bPastMidpoint = GAMESTATE->GetCourseSongIndex()>0 ||
		GAMESTATE->m_fMusicSeconds > GAMESTATE->m_pCurSong->m_fMusicLengthSeconds/4;

	Message msg("Combo");
	if( iCombo )
		msg.SetParam( "Combo", iCombo );
	if( iMisses )
		msg.SetParam( "Misses", iMisses );
	if( bPastMidpoint && m_pPlayerStageStats->FullComboOfScore(TNS_W1) )
		msg.SetParam( "FullComboW1", true );
	if( bPastMidpoint && m_pPlayerStageStats->FullComboOfScore(TNS_W2) )
		msg.SetParam( "FullComboW2", true );
	if( bPastMidpoint && m_pPlayerStageStats->FullComboOfScore(TNS_W3) )
		msg.SetParam( "FullComboW3", true );
	this->HandleMessage( msg );
}

// lua start
#include "LuaBinding.h"

class LunaPlayer: public Luna<Player>
{
public:
	static int SetActorWithJudgmentPosition( T* p, lua_State *L )	{ Actor *pActor = Luna<Actor>::check(L, 1); p->SetActorWithJudgmentPosition(pActor); return 0; }
	static int SetActorWithComboPosition( T* p, lua_State *L )	{ Actor *pActor = Luna<Actor>::check(L, 1); p->SetActorWithComboPosition(pActor); return 0; }

	LunaPlayer()
	{
		ADD_METHOD( SetActorWithJudgmentPosition );
		ADD_METHOD( SetActorWithComboPosition );
	}
};

LUA_REGISTER_DERIVED_CLASS( Player, ActorFrame )
// lua end

/*
 * (c) 2001-2006 Chris Danford, Steve Checkoway
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
