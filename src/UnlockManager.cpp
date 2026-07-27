#include "global.h"
#include "UnlockManager.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "Song.h"
#include "Course.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "GameState.h"
#include "GameConstantsAndTypes.h" // StepsTypeToString
#include "ProfileManager.h"
#include "Profile.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "CommonMetrics.h"
#include "LuaManager.h"
#include "GameManager.h"
#include "Style.h"

#include <cfloat>
#include <vector>


UnlockManager*	UNLOCKMAN = nullptr;	// global and accessible from anywhere in our program

#define UNLOCK_NAMES		THEME->GetMetric ("UnlockManager","UnlockNames")
#define UNLOCK(x)		THEME->GetMetricR("UnlockManager", ssprintf("Unlock%sCommand",x.c_str()));

static ThemeMetric<bool> AUTO_LOCK_CHALLENGE_STEPS( "UnlockManager", "AutoLockChallengeSteps" );
static ThemeMetric<bool> AUTO_LOCK_EDIT_STEPS( "UnlockManager", "AutoLockEditSteps" );

static const char *UnlockRequirementNames[] =
{
	"ArcadePoints",
	"DancePoints",
	"SongPoints",
	"ExtraCleared",
	"ExtraFailed",
	"Toasties",
	"StagesCleared",
	"NumberUnlocked"
};
XToString( UnlockRequirement );
StringToX( UnlockRequirement );
LuaXType( UnlockRequirement );

static const char *UnlockRewardTypeNames[] =
{
	"Song",
	"Steps",
	"StepsType",
	"Course",
	"Modifier",
};
XToString( UnlockRewardType );
XToLocalizedString( UnlockRewardType );
LuaXType( UnlockRewardType );
LuaFunction( UnlockRewardTypeToLocalizedString, UnlockRewardTypeToLocalizedString(Enum::Check<UnlockRewardType>(L, 1)) );

UnlockManager::UnlockManager()
{
	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "UNLOCKMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}

	UNLOCKMAN = this;

	Load();
}

UnlockManager::~UnlockManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "UNLOCKMAN" );
}

void UnlockManager::UnlockSong( const Song *song )
{
	const UnlockEntry *p = FindSong( song );
	if( !p )
		return;  // does not exist
	if( p->m_sEntryID.empty() )
		return;

	UnlockEntryID( p->m_sEntryID );
}

RString UnlockManager::FindEntryID( const RString &sName ) const
{
	const UnlockEntry *pEntry = nullptr;

	const Song *pSong = SONGMAN->FindSong( sName );
	if( pSong != nullptr )
		pEntry = FindSong( pSong );

	const Course *pCourse = SONGMAN->FindCourse( sName );
	if( pCourse != nullptr )
		pEntry = FindCourse( pCourse );

	if( pEntry == nullptr )
		pEntry = FindModifier( sName );

	if( pEntry == nullptr )
	{
		LuaHelpers::ReportScriptErrorFmt( "Couldn't find locked entry \"%s\"", sName.c_str() );
		return "";
	}

	return pEntry->m_sEntryID;
}

int UnlockManager::CourseIsLocked( const Course *pCourse ) const
{
	int iRet = 0;

	if( PREFSMAN->m_bUseUnlockSystem )
	{
		const UnlockEntry *p = FindCourse( pCourse );
		if( p == nullptr )
			return false;
		if( p->IsLocked() )
			iRet |= LOCKED_LOCK;
	}

	/* If a course uses a song that is disabled, disable the course too. */
	for (CourseEntry const &ce : pCourse->m_vEntries)
	{
		const Song *pSong = ce.songID.ToSong();
		if( pSong == nullptr )
			continue;
		int iSongLock = SongIsLocked( pSong );
		if( iSongLock & LOCKED_DISABLED )
			iRet |= LOCKED_DISABLED;
	}

	return iRet;
}

int UnlockManager::SongIsLocked( const Song *pSong ) const
{
	int iRet = 0;
	if( PREFSMAN->m_bUseUnlockSystem )
	{
		const UnlockEntry *p = FindSong( pSong );
		if( p != nullptr && p->IsLocked() )
		{
			iRet |= LOCKED_LOCK;
			if( !p->m_sEntryID.empty() && m_RouletteCodes.find( p->m_sEntryID ) != m_RouletteCodes.end() )
				iRet |= LOCKED_ROULETTE;
		}
	}
	if( PREFSMAN->m_bHiddenSongs && pSong->m_SelectionDisplay == Song::SHOW_NEVER )
		iRet |= LOCKED_SELECTABLE;

	if( !pSong->m_bEnabled )
		iRet |= LOCKED_DISABLED;

	return iRet;
}

bool UnlockManager::StepsIsLocked( const Song *pSong, const Steps *pSteps ) const
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindSteps( pSong, pSteps );
	if( p == nullptr )
		return false;

	return p->IsLocked();
}

bool UnlockManager::StepsTypeIsLocked(const Song *pSong, const Steps *pSteps, const StepsType *pSType) const
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindStepsType( pSong, pSteps, pSType );
	if( p == nullptr )
		return false;

	return p->IsLocked();
}

bool UnlockManager::ModifierIsLocked( const RString &sOneMod ) const
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindModifier( sOneMod );
	if( p == nullptr )
		return false;

	return p->IsLocked();
}

const UnlockEntry *UnlockManager::FindSong( const Song *pSong ) const
{
	for (UnlockEntry const &e : m_UnlockEntries)
		if( e.m_Song.ToSong() == pSong  &&  e.m_dc == Difficulty_Invalid )
			return &e;
	return nullptr;
}

const UnlockEntry *UnlockManager::FindSteps( const Song *pSong, const Steps *pSteps ) const
{
	ASSERT( pSong && pSteps );
	for (UnlockEntry const &e : m_UnlockEntries)
		if( e.m_Song.ToSong() == pSong  &&  e.m_dc == pSteps->GetDifficulty() )
			return &e;
	return nullptr;
}

const UnlockEntry *UnlockManager::FindStepsType(const Song *pSong,
						const Steps *pSteps,
						const StepsType *pSType ) const
{
	ASSERT( pSong && pSteps && pSType );
	for (UnlockEntry const &e : m_UnlockEntries)
	if(e.m_Song.ToSong() == pSong &&
	   e.m_dc == pSteps->GetDifficulty() &&
	   e.m_StepsType == pSteps->m_StepsType)
		return &e;
	return nullptr;
}

const UnlockEntry *UnlockManager::FindCourse( const Course *pCourse ) const
{
	for (UnlockEntry const &e : m_UnlockEntries)
		if( e.m_Course.ToCourse() == pCourse )
			return &e;
	return nullptr;
}

const UnlockEntry *UnlockManager::FindModifier( const RString &sOneMod ) const
{
	for (UnlockEntry const &e : m_UnlockEntries)
		if( e.GetModifier().CompareNoCase(sOneMod) == 0 )
			return &e;
	return nullptr;
}

static float GetArcadePoints( const Profile *pProfile )
{
	float fAP =	0;

	FOREACH_ENUM( Grade, g )
	{
		switch(g)
		{
		case Grade_Tier01:
		case Grade_Tier02:	fAP += 9 * pProfile->m_iNumStagesPassedByGrade[g]; break;
		default:			fAP += 1 * pProfile->m_iNumStagesPassedByGrade[g]; break;

		case Grade_Failed:
		case Grade_NoData:
			;	// no points
			break;
		}
	}

	FOREACH_ENUM( PlayMode, pm )
	{
		switch(pm)
		{
			case PLAY_MODE_NONSTOP:
			case PLAY_MODE_ONI:
			case PLAY_MODE_ENDLESS:
			{
				fAP += pProfile->m_iNumSongsPlayedByPlayMode[pm];
			}
			default: break;
		}
	}

	return fAP;
}

// TODO: Make this more flexible for games with many grade tiers. Lua-ize it? -Wolfman2000
static float GetSongPoints( const Profile *pProfile )
{
	float fSP =	0;

	FOREACH_ENUM( Grade, g )
	{
		switch( g )
		{
		case Grade_Tier01:/*AAAA*/	fSP += 20 * pProfile->m_iNumStagesPassedByGrade[g];	break;
		case Grade_Tier02:/*AAA*/	fSP += 10* pProfile->m_iNumStagesPassedByGrade[g];	break;
		case Grade_Tier03:/*AA*/	fSP += 5* pProfile->m_iNumStagesPassedByGrade[g];	break;
		case Grade_Tier04:/*A*/		fSP += 4* pProfile->m_iNumStagesPassedByGrade[g];	break;
		case Grade_Tier05:/*B*/		fSP += 3* pProfile->m_iNumStagesPassedByGrade[g];	break;
		case Grade_Tier06:/*C*/		fSP += 2* pProfile->m_iNumStagesPassedByGrade[g];	break;
		case Grade_Tier07:/*D*/		fSP += 1* pProfile->m_iNumStagesPassedByGrade[g];	break;
		case Grade_Failed:
		case Grade_NoData:
		default:
			;	// no points
			break;
		}
	}

	FOREACH_ENUM( PlayMode, pm )
	{
		switch(pm)
		{
			case PLAY_MODE_NONSTOP:
			case PLAY_MODE_ONI:
			case PLAY_MODE_ENDLESS:
			{
			fSP += pProfile->m_iNumSongsPlayedByPlayMode[pm];
			}
			default: break;
		}

	}

	return fSP;
}

void UnlockManager::GetPoints( const Profile *pProfile, float fScores[NUM_UnlockRequirement] ) const
{
	fScores[UnlockRequirement_ArcadePoints] = GetArcadePoints( pProfile );
	fScores[UnlockRequirement_SongPoints] = GetSongPoints( pProfile );
	fScores[UnlockRequirement_DancePoints] = (float) pProfile->m_iTotalDancePoints;
	fScores[UnlockRequirement_StagesCleared] = (float) pProfile->GetTotalNumSongsPassed();
	//fScores[UnlockRequirement_NumUnlocked] = (float) GetNumUnlocked();
}

/* Return true if all songs and/or courses referenced by an unlock are available. */
bool UnlockEntry::IsValid() const
{
	switch( m_Type )
	{
	case UnlockRewardType_Song:
		return m_Song.IsValid();

	case UnlockRewardType_Steps:
		return m_Song.IsValid() && m_dc != Difficulty_Invalid;

	case UnlockRewardType_Steps_Type:
	{
		return m_Song.IsValid() && m_dc != Difficulty_Invalid && m_StepsType != StepsType_Invalid;
	}

	case UnlockRewardType_Course:
		return m_Course.IsValid();

	case UnlockRewardType_Modifier:
		return true;

	default:
		WARN( ssprintf("%i", m_Type) );
		return false;
	}
}

UnlockEntryStatus UnlockEntry::GetUnlockEntryStatus() const
{
	std::set<RString> &ids = PROFILEMAN->GetMachineProfile()->m_UnlockedEntryIDs;
	if(!m_sEntryID.empty() &&
	   ids.find(m_sEntryID) != ids.end() )
		return UnlockEntryStatus_Unlocked;

	float fScores[NUM_UnlockRequirement];
	UNLOCKMAN->GetPoints( PROFILEMAN->GetMachineProfile(), fScores );

	for( int i = 0; i < NUM_UnlockRequirement; ++i )
	{
		if( i == UnlockRequirement_NumUnlocked )
			continue;

		if( m_fRequirement[i] && fScores[i] >= m_fRequirement[i] )
			return UnlockEntryStatus_RequirementsMet;
	}

	if( m_bRequirePassHardSteps && m_Song.IsValid() )
	{
		Song *pSong = m_Song.ToSong();
		std::vector<Steps*> vp;
		SongUtil::GetSteps(
			pSong,
			vp,
			StepsType_Invalid,
			Difficulty_Hard
			);
		for (Steps const *s : vp)
			if( PROFILEMAN->GetMachineProfile()->HasPassedSteps(pSong, s) )
				return UnlockEntryStatus_Unlocked;
	}

	if (m_bRequirePassChallengeSteps && m_Song.IsValid())
	{
		Song *pSong = m_Song.ToSong();
		std::vector<Steps*> vp;
		SongUtil::GetSteps(pSong,
				   vp,
				   StepsType_Invalid,
				   Difficulty_Challenge);
		for (Steps const *s : vp)
		{
			if (PROFILEMAN->GetMachineProfile()->HasPassedSteps(pSong, s))
				return UnlockEntryStatus_Unlocked;
		}
	}

	return UnlockEntryStatus_RequrementsNotMet;
}

RString UnlockEntry::GetDescription() const
{
	Song *pSong = m_Song.ToSong();
	switch( m_Type )
	{
	default:
		FAIL_M(ssprintf("Invalid UnlockRewardType: %i", m_Type));
	case UnlockRewardType_Song:
		return pSong ? pSong->GetDisplayFullTitle() : RString("");
	case UnlockRewardType_Steps:
	{
		StepsType st = GAMEMAN->GetHowToPlayStyleForGame( GAMESTATE->m_pCurGame )->m_StepsType;	// TODO: Is this the best thing we can do here?
		return (pSong ? pSong->GetDisplayFullTitle() : RString("")) + ", " + CustomDifficultyToLocalizedString( GetCustomDifficulty(st, m_dc, CourseType_Invalid) );
	}
	case UnlockRewardType_Steps_Type:
	{
		RString ret = (pSong ? pSong->GetDisplayFullTitle() : RString(""));
		ret += "," + CustomDifficultyToLocalizedString( GetCustomDifficulty(m_StepsType, m_dc, CourseType_Invalid) );
		return ret + "," + StringConversion::ToString(m_StepsType); // yeah, bit strange.
	}
	case UnlockRewardType_Course:
		return m_Course.IsValid() ? m_Course.ToCourse()->GetDisplayFullTitle() : RString("");
	case UnlockRewardType_Modifier:
		return CommonMetrics::LocalizeOptionItem( GetModifier(), false );
	}
}

RString	UnlockEntry::GetBannerFile() const
{
	Song *pSong = m_Song.ToSong();
	switch( m_Type )
	{
	default:
		FAIL_M(ssprintf("Invalid UnlockRewardType: %i", m_Type));
	case UnlockRewardType_Song:
	case UnlockRewardType_Steps:
	case UnlockRewardType_Steps_Type:
		return pSong ? pSong->GetBannerPath() : RString("");
	case UnlockRewardType_Course:
		return m_Course.ToCourse() ? m_Course.ToCourse()->GetBannerPath() : RString("");
	case UnlockRewardType_Modifier:
		return "";
	}
}

RString	UnlockEntry::GetBackgroundFile() const
{
	Song *pSong = m_Song.ToSong();
	switch( m_Type )
	{
	default:
		FAIL_M(ssprintf("Invalid UnlockRewardType: %i", m_Type));
	case UnlockRewardType_Song:
	case UnlockRewardType_Steps:
	case UnlockRewardType_Steps_Type:
		return pSong ? pSong->GetBackgroundPath() : RString("");
	case UnlockRewardType_Course:
		return "";
	case UnlockRewardType_Modifier:
		return "";
	}
}

/////////////////////////////////////////////////////////

void UnlockManager::Load()
{
	LOG->Trace( "UnlockManager::Load()" );

	std::vector<RString> asUnlockNames;
	split( UNLOCK_NAMES, ",", asUnlockNames );

	Lua *L = LUA->Get();
	for( unsigned i = 0; i < asUnlockNames.size(); ++i )
	{
		const RString &sUnlockName = asUnlockNames[i];

		LuaReference cmds = UNLOCK( sUnlockName );

		UnlockEntry current;
		current.m_sEntryID = sUnlockName; // default

		// function
		cmds.PushSelf( L );
		ASSERT( !lua_isnil(L, -1) );

		// 1st parameter
		current.PushSelf( L );

		// call function with 1 argument and 0 results
		RString error= "Lua error in command: ";
		LuaHelpers::RunScriptOnStack(L, error, 1, 0, true);

		if( current.m_bRoulette )
			m_RouletteCodes.insert( current.m_sEntryID );

		m_UnlockEntries.push_back( current );
	}
	LUA->Release(L);

	if( AUTO_LOCK_CHALLENGE_STEPS )
	{
		for (Song const *s : SONGMAN->GetAllSongs())
		{
			// If no hard steps to play to unlock, skip
			if( SongUtil::GetOneSteps(s, StepsType_Invalid, Difficulty_Hard) == nullptr )
				continue;

			// If no challenge steps to unlock, skip
			if( SongUtil::GetOneSteps(s, StepsType_Invalid, Difficulty_Challenge) == nullptr )
				continue;

			UnlockEntry ue;
			ue.m_sEntryID = "_challenge_" + s->GetSongDir();
			ue.m_Type = UnlockRewardType_Steps;
			ue.m_cmd.Load( s->m_sGroupName+"/"+ s->GetTranslitFullTitle()+",expert" );
			ue.m_bRequirePassHardSteps = true;

			m_UnlockEntries.push_back( ue );
		}
	}

	if (AUTO_LOCK_EDIT_STEPS)
	{
		for (Song const *s : SONGMAN->GetAllSongs())
		{
			// no challenge steps to play: skip.
			if (SongUtil::GetOneSteps(s, StepsType_Invalid, Difficulty_Challenge) == nullptr)
				continue;

			// no edit steps to unlock: skip.
			if (SongUtil::GetOneSteps(s, StepsType_Invalid, Difficulty_Edit) == nullptr)
				continue;

			UnlockEntry ue;
			ue.m_sEntryID = "_edit_" + s->GetSongDir();
			ue.m_Type = UnlockRewardType_Steps;
			ue.m_cmd.Load( s->m_sGroupName+"/"+ s->GetTranslitFullTitle()+",edit" );
			ue.m_bRequirePassChallengeSteps = true;

			m_UnlockEntries.push_back(ue);

		}
	}

	// Make sure that we don't have duplicate unlock IDs. This can cause problems
	// with UnlockCelebrate and with codes.
	unsigned size = m_UnlockEntries.size();
	if (size > 1)
	{
		for (unsigned i = 0; i < size - 1; ++i)
		{
			UnlockEntry const &ue1 = m_UnlockEntries[i];
			for (unsigned j = i + 1; j < size; ++j)
			{
				UnlockEntry const &ue2 = m_UnlockEntries[j];
				// at this point, these two are definitely different. Assert.
				ASSERT_M( ue1.m_sEntryID != ue2.m_sEntryID, ssprintf("duplicate unlock entry id %s", ue1.m_sEntryID.c_str()));

			}
		}
	}

	for (UnlockEntry &e : m_UnlockEntries)
	{
		switch( e.m_Type )
		{
		case UnlockRewardType_Song:
			e.m_Song.FromSong( SONGMAN->FindSong( e.m_cmd.GetArg(0).s ) );
			if( !e.m_Song.IsValid() )
				LuaHelpers::ReportScriptErrorFmt( "Unlock: Cannot find song matching \"%s\"", e.m_cmd.GetArg(0).s.c_str() );
			break;
		case UnlockRewardType_Steps:
			e.m_Song.FromSong( SONGMAN->FindSong( e.m_cmd.GetArg(0).s ) );
			if( !e.m_Song.IsValid() )
			{
				LuaHelpers::ReportScriptErrorFmt( "Unlock: Cannot find song matching \"%s\"", e.m_cmd.GetArg(0).s.c_str() );
				break;
			}

			e.m_dc = StringToDifficulty( e.m_cmd.GetArg(1).s );
			if( e.m_dc == Difficulty_Invalid )
			{
				LuaHelpers::ReportScriptErrorFmt( "Unlock: Invalid difficulty \"%s\"", e.m_cmd.GetArg(1).s.c_str() );
				break;
			}

			break;
		case UnlockRewardType_Steps_Type:
		{
			e.m_Song.FromSong( SONGMAN->FindSong( e.m_cmd.GetArg(0).s ) );
			if( !e.m_Song.IsValid() )
			{
				LuaHelpers::ReportScriptErrorFmt( "Unlock: Cannot find song matching \"%s\"", e.m_cmd.GetArg(0).s.c_str() );
				break;
			}

			e.m_dc = StringToDifficulty( e.m_cmd.GetArg(1).s );
			if( e.m_dc == Difficulty_Invalid )
			{
				LuaHelpers::ReportScriptErrorFmt( "Unlock: Invalid difficulty \"%s\"", e.m_cmd.GetArg(1).s.c_str() );
				break;
			}

			e.m_StepsType = GAMEMAN->StringToStepsType(e.m_cmd.GetArg(2).s);
			if (e.m_StepsType == StepsType_Invalid)
			{
				LuaHelpers::ReportScriptErrorFmt( "Unlock: Invalid steps type \"%s\"", e.m_cmd.GetArg(2).s.c_str() );
				break;
			}
			break;
		}
		case UnlockRewardType_Course:
			e.m_Course.FromCourse( SONGMAN->FindCourse(e.m_cmd.GetArg(0).s) );
			if( !e.m_Course.IsValid() )
				LuaHelpers::ReportScriptErrorFmt( "Unlock: Cannot find course matching \"%s\"", e.m_cmd.GetArg(0).s.c_str() );
			break;
		case UnlockRewardType_Modifier:
			// nothing to cache
			break;
		default:
			FAIL_M(ssprintf("Invalid UnlockRewardType: %i", e.m_Type));
		}
	}

	// Log unlocks
	for (UnlockEntry &e : m_UnlockEntries)
	{
		RString str = ssprintf( "Unlock: %s; ", join("\n",e.m_cmd.m_vsArgs).c_str() );
		FOREACH_ENUM( UnlockRequirement, j )
			if( e.m_fRequirement[j] )
				str += ssprintf( "%s = %f; ", UnlockRequirementToString(j).c_str(), e.m_fRequirement[j] );
		if( e.m_bRequirePassHardSteps )
			str += "RequirePassHardSteps; ";
		if (e.m_bRequirePassChallengeSteps)
			str += "RequirePassChallengeSteps; ";

		str += ssprintf( "entryID = %s ", e.m_sEntryID.c_str() );
		str += e.IsLocked()? "locked":"unlocked";
		if( e.m_Song.IsValid() )
			str += ( " (found song)" );
		if( e.m_Course.IsValid() )
			str += ( " (found course)" );
		LOG->Trace( "%s", str.c_str() );
	}

	return;
}


void UnlockManager::Reload()
{
	// clear old data, if any
	m_UnlockEntries.clear();
	m_RouletteCodes.clear();

	Load();
}

float UnlockManager::PointsUntilNextUnlock( UnlockRequirement t ) const
{
	float fScores[NUM_UnlockRequirement];
	ZERO( fScores );
	GetPoints( PROFILEMAN->GetMachineProfile(), fScores );

	float fSmallestPoints = FLT_MAX;   // or an arbitrarily large value
	for( unsigned a=0; a<m_UnlockEntries.size(); a++ )
		if( m_UnlockEntries[a].m_fRequirement[t] > fScores[t] )
			fSmallestPoints = std::min( fSmallestPoints, m_UnlockEntries[a].m_fRequirement[t] );

	if( fSmallestPoints == FLT_MAX )
		return 0;  // no match found
	return fSmallestPoints - fScores[t];
}

void UnlockManager::UnlockEntryID( RString sEntryID )
{
	PROFILEMAN->GetMachineProfile()->m_UnlockedEntryIDs.insert( sEntryID );
	SONGMAN->InvalidateCachedTrails();
}

void UnlockManager::UnlockEntryIndex( int iEntryIndex )
{
	RString sEntryID = m_UnlockEntries[iEntryIndex].m_sEntryID;
	UnlockEntryID( sEntryID );
}

void UnlockManager::LockEntryID( RString entryID )
{
	PROFILEMAN->GetMachineProfile()->m_UnlockedEntryIDs.erase( entryID );
	SONGMAN->InvalidateCachedTrails();
}

void UnlockManager::LockEntryIndex( int entryIndex )
{
	RString entryID = m_UnlockEntries[entryIndex].m_sEntryID;
	LockEntryID( entryID );
}

void UnlockManager::PreferUnlockEntryID( RString sUnlockEntryID )
{
	for( unsigned i = 0; i < m_UnlockEntries.size(); ++i )
	{
		UnlockEntry &pEntry = m_UnlockEntries[i];
		if( pEntry.m_sEntryID != sUnlockEntryID )
			continue;

		if( pEntry.m_Song.ToSong() != nullptr )
			GAMESTATE->m_pPreferredSong = pEntry.m_Song.ToSong();
		if( pEntry.m_Course.ToCourse() )
			GAMESTATE->m_pPreferredCourse = pEntry.m_Course.ToCourse();
	}
}

int UnlockManager::GetNumUnlocks() const
{
	return m_UnlockEntries.size();
}

int UnlockManager::GetNumUnlocked() const
{
	return std::count_if(m_UnlockEntries.begin(), m_UnlockEntries.end(), [](UnlockEntry const &ue) {
		return !ue.IsLocked();
	});
}

int UnlockManager::GetUnlockEntryIndexToCelebrate() const
{
	int i = 0;
	for (UnlockEntry const &ue : m_UnlockEntries)
	{
		if( ue.GetUnlockEntryStatus() == UnlockEntryStatus_RequirementsMet )
			return i;
		++i;
	}
	return -1;
}

bool UnlockManager::AnyUnlocksToCelebrate() const
{
	return GetUnlockEntryIndexToCelebrate() != -1;
}

void UnlockManager::GetUnlocksByType( UnlockRewardType t, std::vector<UnlockEntry *> &apEntries )
{
	for (UnlockEntry &entry : m_UnlockEntries)
		if( entry.IsValid() && entry.m_Type == t )
			apEntries.push_back( &entry );
}

void UnlockManager::GetSongsUnlockedByEntryID( std::vector<Song *> &apSongsOut, RString sUnlockEntryID )
{
	std::vector<UnlockEntry *> apEntries;
	GetUnlocksByType( UnlockRewardType_Song, apEntries );

	for (UnlockEntry const *ue : apEntries)
		if( ue->m_sEntryID == sUnlockEntryID )
			apSongsOut.push_back( ue->m_Song.ToSong() );
}

void UnlockManager::GetStepsUnlockedByEntryID( std::vector<Song *> &apSongsOut, std::vector<Difficulty> &apDifficultyOut, RString sUnlockEntryID )
{
	std::vector<UnlockEntry *> apEntries;
	GetUnlocksByType( UnlockRewardType_Steps, apEntries );

	for (UnlockEntry const *entry : apEntries)
	{
		if( entry->m_sEntryID == sUnlockEntryID )
		{
			apSongsOut.push_back( entry->m_Song.ToSong() );
			apDifficultyOut.push_back( entry->m_dc );
		}
	}
}


// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the UnlockEntry. */
class LunaUnlockEntry: public Luna<UnlockEntry>
{
public:
	static int IsLocked( T* p, lua_State *L )		{ lua_pushboolean(L, p->IsLocked() ); return 1; }
	static int GetDescription( T* p, lua_State *L )		{ lua_pushstring(L, p->GetDescription() ); return 1; }
	static int GetUnlockRewardType( T* p, lua_State *L )	{ lua_pushnumber(L, p->m_Type ); return 1; }
	static int GetRequirement( T* p, lua_State *L )		{ UnlockRequirement i = Enum::Check<UnlockRequirement>( L, 1 ); lua_pushnumber(L, p->m_fRequirement[i] ); return 1; }
	static int GetRequirePassHardSteps( T* p, lua_State *L ){ lua_pushboolean(L, p->m_bRequirePassHardSteps); return 1; }
	static int GetRequirePassChallengeSteps( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->m_bRequirePassChallengeSteps);
		return 1;
	}
	static int GetSong( T* p, lua_State *L )
	{
		Song *pSong = p->m_Song.ToSong();
		if( pSong ) { pSong->PushSelf(L); return 1; }
		return 0;
	}
	// Get all of the steps locked based on difficulty (similar to In The Groove 2).
	static int GetStepOfAllTypes( T* p, lua_State *L )
	{
		Song *pSong = p->m_Song.ToSong();
		if (pSong)
		{
			const std::vector<Steps*>& allSteps = pSong->GetAllSteps();
			std::vector<Steps*> toRet;
			for (Steps *step : allSteps)
			{
				if (step->GetDifficulty() == p->m_dc)
				{
					toRet.push_back(step);
				}
			}
			LuaHelpers::CreateTableFromArray<Steps*>( toRet, L );
			return 1;
		}
		return 0;
	}

	// TODO: Add a function to just get all steps.
	static int GetStepByStepsType( T* p, lua_State *L )
	{
		Song *pSong = p->m_Song.ToSong();
		if (pSong)
		{
			const std::vector<Steps*>& allStepsType = pSong->GetStepsByStepsType(p->m_StepsType);
			for (Steps *step : allStepsType)
			{
				if (step->GetDifficulty() == p->m_dc)
				{
					step->PushSelf(L); return 1;
				}
			}
		}
		return 0;
	}

	static int GetCourse( T* p, lua_State *L )
	{
		Course *pCourse = p->m_Course.ToCourse();
		if( pCourse ) { pCourse->PushSelf(L); return 1; }
		return 0;
	}
	static int GetCode( T* p, lua_State *L )
	{
		lua_pushstring( L, p->m_sEntryID );
		return 1;
	}

	// internal
	static int GetArgs( T* p, lua_State *L )
	{
		Command cmd;
		for( int i = 1; i <= lua_gettop(L); ++i )
			cmd.m_vsArgs.push_back( SArg(i) );
		p->m_cmd = cmd;
		return 0;
	}

	static int song( T* p, lua_State *L )	{ GetArgs( p, L ); p->m_Type = UnlockRewardType_Song; return 0; }
	static int steps( T* p, lua_State *L )	{ GetArgs( p, L ); p->m_Type = UnlockRewardType_Steps; return 0; }
	static int steps_type(T* p, lua_State *L) { GetArgs(p, L); p->m_Type = UnlockRewardType_Steps_Type; return 0; }
	static int course( T* p, lua_State *L ) { GetArgs( p, L ); p->m_Type = UnlockRewardType_Course; return 0; }
	static int mod( T* p, lua_State *L )	{ GetArgs( p, L ); p->m_Type = UnlockRewardType_Modifier; return 0; }
	static int code( T* p, lua_State *L )	{ p->m_sEntryID = SArg(1); return 0; }
	static int roulette( T* p, lua_State *L ) { p->m_bRoulette = true; return 0; }
	static int requirepasshardsteps( T* p, lua_State *L ) { p->m_bRequirePassHardSteps = true; return 0; }
	static int requirepasschallengesteps( T* p, lua_State *L ) { p->m_bRequirePassChallengeSteps = true; return 0; }
	static int require( T* p, lua_State *L )
	{
		const UnlockRequirement ut = Enum::Check<UnlockRequirement>( L, 1 );
		if( ut != UnlockRequirement_Invalid )
			p->m_fRequirement[ut] = FArg(2);
		return 0;
	}

	LunaUnlockEntry()
	{
		ADD_METHOD( IsLocked );
		ADD_METHOD( GetCode );
		ADD_METHOD( GetDescription );
		ADD_METHOD( GetUnlockRewardType );
		ADD_METHOD( GetRequirement );
		ADD_METHOD( GetRequirePassHardSteps );
		ADD_METHOD( GetRequirePassChallengeSteps );
		ADD_METHOD( GetSong );
		ADD_METHOD( GetCourse );
		ADD_METHOD( GetStepOfAllTypes );
		ADD_METHOD( GetStepByStepsType );
		ADD_METHOD( song );
		ADD_METHOD( steps );
		ADD_METHOD( steps_type );
		ADD_METHOD( course );
		ADD_METHOD( mod );
		ADD_METHOD( code );
		ADD_METHOD( roulette );
		ADD_METHOD( requirepasshardsteps );
		ADD_METHOD( requirepasschallengesteps );
		ADD_METHOD( require );
	}
};

LUA_REGISTER_CLASS( UnlockEntry )

/** @brief Allow Lua to have access to the UnlockManager. */
class LunaUnlockManager: public Luna<UnlockManager>
{
public:
	static int GetPointsUntilNextUnlock( T* p, lua_State *L )
	{
		const UnlockRequirement ut = Enum::Check<UnlockRequirement>( L, 1 );
		lua_pushnumber( L, p->PointsUntilNextUnlock(ut) );
		return 1;
	}
	static int FindEntryID( T* p, lua_State *L )			{ RString sName = SArg(1); RString s = p->FindEntryID(sName); if( s.empty() ) lua_pushnil(L); else lua_pushstring(L, s); return 1; }
	static int UnlockEntryID( T* p, lua_State *L )			{ RString sUnlockEntryID = SArg(1); p->UnlockEntryID(sUnlockEntryID); COMMON_RETURN_SELF; }
	static int UnlockEntryIndex( T* p, lua_State *L )		{ int iUnlockEntryID = IArg(1); p->UnlockEntryIndex(iUnlockEntryID); COMMON_RETURN_SELF; }
	static int LockEntryID( T * p, lua_State * L)
	{
		RString entryID = SArg(1);
		p->LockEntryID( entryID );
		COMMON_RETURN_SELF;
	}
	static int LockEntryIndex( T * p, lua_State * L)
	{
		int entryIndex = IArg(1);
		p->LockEntryIndex( entryIndex );
		COMMON_RETURN_SELF;
	}
	static int PreferUnlockEntryID( T* p, lua_State *L )		{ RString sUnlockEntryID = SArg(1); p->PreferUnlockEntryID(sUnlockEntryID); COMMON_RETURN_SELF; }
	static int GetNumUnlocks( T* p, lua_State *L )			{ lua_pushnumber( L, p->GetNumUnlocks() ); return 1; }
	static int GetNumUnlocked( T* p, lua_State *L )			{ lua_pushnumber( L, p->GetNumUnlocked() ); return 1; }
	static int GetUnlockEntryIndexToCelebrate( T* p, lua_State *L )	{ lua_pushnumber( L, p->GetUnlockEntryIndexToCelebrate() ); return 1; }
	static int AnyUnlocksToCelebrate( T* p, lua_State *L )		{ lua_pushboolean( L, p->AnyUnlocksToCelebrate() ); return 1; }
	static int GetUnlockEntry( T* p, lua_State *L )			{ unsigned iIndex = IArg(1); if( iIndex >= p->m_UnlockEntries.size() ) return 0; p->m_UnlockEntries[iIndex].PushSelf(L); return 1; }
	static int GetSongsUnlockedByEntryID( T* p, lua_State *L )
	{
		std::vector<Song *> apSongs;
		UNLOCKMAN->GetSongsUnlockedByEntryID( apSongs, SArg(1) );
		LuaHelpers::CreateTableFromArray( apSongs, L );
		return 1;
	}

	static int GetStepsUnlockedByEntryID( T* p, lua_State *L )
	{
		// Return the Song each Steps are associated with, too.
		std::vector<Song *> apSongs;
		std::vector<Difficulty> apDifficulty;
		UNLOCKMAN->GetStepsUnlockedByEntryID( apSongs, apDifficulty, SArg(1) );
		LuaHelpers::CreateTableFromArray( apSongs, L );
		LuaHelpers::CreateTableFromArray( apDifficulty, L );
		return 2;
	}

	static int GetPoints( T* p, lua_State *L ) {
		float fScores[NUM_UnlockRequirement];
		UNLOCKMAN->GetPoints( PROFILEMAN->GetMachineProfile(), fScores );
		lua_pushnumber( L, fScores[Enum::Check<UnlockRequirement>(L, 1)] );
		return 1;
	}

	static int GetPointsForProfile( T* p, lua_State *L ) {
		float fScores[NUM_UnlockRequirement];
		UNLOCKMAN->GetPoints( Luna<Profile>::check(L,1), fScores );
		lua_pushnumber( L, fScores[Enum::Check<UnlockRequirement>(L, 2)] );
		return 1;
	}

	static int IsSongLocked( T* p, lua_State *L )
	{
		Song *pSong = Luna<Song>::check(L,1);
		lua_pushnumber( L, UNLOCKMAN->SongIsLocked(pSong));
		return 1;
	}

	LunaUnlockManager()
	{
		ADD_METHOD( AnyUnlocksToCelebrate );
		ADD_METHOD( FindEntryID );
		ADD_METHOD( GetNumUnlocks );
		ADD_METHOD( GetNumUnlocked );
		ADD_METHOD( GetPoints );
		ADD_METHOD( GetPointsForProfile );
		ADD_METHOD( GetPointsUntilNextUnlock );
		ADD_METHOD( GetSongsUnlockedByEntryID );
		ADD_METHOD( GetStepsUnlockedByEntryID );
		ADD_METHOD( GetUnlockEntry );
		ADD_METHOD( GetUnlockEntryIndexToCelebrate );
		ADD_METHOD( PreferUnlockEntryID );
		ADD_METHOD( UnlockEntryID );
		ADD_METHOD( UnlockEntryIndex );
		ADD_METHOD( LockEntryID );
		ADD_METHOD( LockEntryIndex );
		ADD_METHOD( IsSongLocked );
		//ADD_METHOD( UnlockSong );
		//ADD_METHOD( GetUnlocksByType );
	}
};

LUA_REGISTER_CLASS( UnlockManager )
// lua end

/*
 * (c) 2001-2004 Kevin Slaughter, Andrew Wong, Glenn Maynard
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
