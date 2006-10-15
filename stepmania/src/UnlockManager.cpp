#include "global.h"
#include "UnlockManager.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "song.h"
#include "Course.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "GameState.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "ThemeManager.h"
#include "Foreach.h"
#include "Steps.h"
#include <float.h>
#include "CommonMetrics.h"
#include "LuaManager.h"

UnlockManager*	UNLOCKMAN = NULL;	// global and accessable from anywhere in our program

#define UNLOCK_NAMES		THEME->GetMetric ("UnlockManager","UnlockNames")
#define UNLOCK(x)		THEME->GetMetricR("UnlockManager", ssprintf("Unlock%sCommand",x.c_str()));

ThemeMetric<bool> AUTO_LOCK_CHALLENGE_STEPS( "UnlockManager", "AutoLockChallengeSteps" );

static const char *UnlockRequirementNames[] =
{
	"ArcadePoints",
	"DancePoints",
	"SongPoints",
	"ExtraCleared",
	"ExtraFailed",
	"Toasties",
	"StagesCleared"
};
XToString2( UnlockRequirement );
StringToX( UnlockRequirement );
LuaXType( UnlockRequirement );

static const char *UnlockRewardTypeNames[] =
{
	"Song",
	"Steps",
	"Course",
	"Modifier",
};
XToString2( UnlockRewardType );
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
	const UnlockEntry *pEntry = NULL;
	
	const Song *pSong = SONGMAN->FindSong( sName );
	if( pSong != NULL )
		pEntry = FindSong( pSong );

	const Course *pCourse = SONGMAN->FindCourse( sName );
	if( pCourse != NULL )
		pEntry = FindCourse( pCourse );
	
	if( pEntry == NULL )
		pEntry = FindModifier( sName );

	if( pEntry == NULL )
	{
		LOG->Warn( "Couldn't find locked entry \"%s\"", sName.c_str() );
		return "";
	}

	return pEntry->m_sEntryID;
}

bool UnlockManager::CourseIsLocked( const Course *course ) const
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindCourse( course );
	if( p == NULL )
		return false;

	return p->IsLocked();
}

bool UnlockManager::SongIsLocked( const Song *song ) const
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindSong( song );
	if( p == NULL )
		return false;

	return p->IsLocked();
}

/* Return true if the song is *only* available in roulette. */
bool UnlockManager::SongIsRouletteOnly( const Song *song ) const
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindSong( song );
	if( p == NULL )
		return false;

	/* If the song is locked by a code, and it's a roulette code, honor IsLocked. */
	if( p->m_sEntryID.empty() || m_RouletteCodes.find( p->m_sEntryID ) == m_RouletteCodes.end() )
		return false;

	return p->IsLocked();
}

bool UnlockManager::StepsIsLocked( const Song *pSong, const Steps *pSteps ) const
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindSteps( pSong, pSteps );
	if( p == NULL )
		return false;

	return p->IsLocked();
}

bool UnlockManager::ModifierIsLocked( const RString &sOneMod ) const
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindModifier( sOneMod );
	if( p == NULL )
		return false;

	return p->IsLocked();
}

const UnlockEntry *UnlockManager::FindSong( const Song *pSong ) const
{
	FOREACH_CONST( UnlockEntry, m_UnlockEntries, e )
		if( e->m_pSong == pSong  &&  e->m_dc == Difficulty_Invalid )
			return &(*e);
	return NULL;
}

const UnlockEntry *UnlockManager::FindSteps( const Song *pSong, const Steps *pSteps ) const
{
	ASSERT( pSong && pSteps );
	FOREACH_CONST( UnlockEntry, m_UnlockEntries, e )
		if( e->m_pSong == pSong  &&  e->m_dc == pSteps->GetDifficulty() )
			return &(*e);
	return NULL;
}

const UnlockEntry *UnlockManager::FindCourse( const Course *pCourse ) const
{
	FOREACH_CONST( UnlockEntry, m_UnlockEntries, e )
		if( e->m_pCourse == pCourse )
			return &(*e);
	return NULL;
}

const UnlockEntry *UnlockManager::FindModifier( const RString &sOneMod ) const
{
	FOREACH_CONST( UnlockEntry, m_UnlockEntries, e )
		if( e->GetModifier().CompareNoCase(sOneMod) == 0 )
			return &(*e);
	return NULL;
}

static float GetArcadePoints( const Profile *pProfile )
{
	float fAP =	0;

	FOREACH_Grade(g)
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

	FOREACH_PlayMode(pm)
	{
		switch(pm)
		{
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			fAP += pProfile->m_iNumSongsPlayedByPlayMode[pm];
			break;
		}

	}

	return fAP;
}

static float GetSongPoints( const Profile *pProfile )
{
	float fSP =	0;

	FOREACH_Grade(g)
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
			;	// no points
			break;
		}
	}

	FOREACH_PlayMode(pm)
	{
		switch(pm)
		{
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			fSP += pProfile->m_iNumSongsPlayedByPlayMode[pm];
			break;
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
}

/* Return true if all songs and/or courses referenced by an unlock are available. */
bool UnlockEntry::IsValid() const
{
	switch( m_Type )
	{
	case UnlockRewardType_Song:
		return m_pSong != NULL;

	case UnlockRewardType_Steps:
		return m_pSong != NULL && m_dc != Difficulty_Invalid;

	case UnlockRewardType_Course:
		return m_pCourse != NULL;

	case UnlockRewardType_Modifier:
		return true;

	default:
		WARN( ssprintf("%i", m_Type) );
		return false;
	}
}

UnlockEntryStatus UnlockEntry::GetUnlockEntryStatus() const
{
	if( !m_sEntryID.empty() && PROFILEMAN->GetMachineProfile()->m_UnlockedEntryIDs.find(m_sEntryID) != PROFILEMAN->GetMachineProfile()->m_UnlockedEntryIDs.end() )
		return UnlockEntryStatus_Unlocked;

	float fScores[NUM_UnlockRequirement];
	UNLOCKMAN->GetPoints( PROFILEMAN->GetMachineProfile(), fScores );

	for( int i = 0; i < NUM_UnlockRequirement; ++i )
		if( m_fRequirement[i] && fScores[i] >= m_fRequirement[i] )
			return UnlockEntryStatus_RequirementsMet;

	if( m_bRequirePassHardSteps && m_pSong )
	{
		vector<Steps*> vp;
		SongUtil::GetSteps(
			m_pSong,
			vp, 
			StepsType_Invalid, 
			DIFFICULTY_HARD
			);
		FOREACH_CONST( Steps*, vp, s )
			if( PROFILEMAN->GetMachineProfile()->HasPassedSteps( m_pSong, *s ) )
				return UnlockEntryStatus_RequirementsMet;
	}


	return UnlockEntryStatus_RequrementsNotMet;
}

RString UnlockEntry::GetDescription() const
{
	switch( m_Type )
	{
	default:
		ASSERT(0);
		return "";
	case UnlockRewardType_Song:
		return m_pSong ? m_pSong->GetDisplayFullTitle() : "";
	case UnlockRewardType_Steps:
		return (m_pSong ? m_pSong->GetDisplayFullTitle() : "") + ", " + DifficultyToLocalizedString( m_dc );
	case UnlockRewardType_Course:
		return m_pCourse ? m_pCourse->GetDisplayFullTitle() : "";
	case UnlockRewardType_Modifier:
		return CommonMetrics::LocalizeOptionItem( GetModifier(), false );
	}
}

RString	UnlockEntry::GetBannerFile() const
{
	switch( m_Type )
	{
	default:
		ASSERT(0);
		return "";
	case UnlockRewardType_Song:
	case UnlockRewardType_Steps:
		return m_pSong ? m_pSong->GetBannerPath() : "";
	case UnlockRewardType_Course:
		return m_pCourse ? m_pCourse->m_sBannerPath : "";
	case UnlockRewardType_Modifier:
		return "";
	}	
}

RString	UnlockEntry::GetBackgroundFile() const
{
	switch( m_Type )
	{
	default:
		ASSERT(0);
		return "";
	case UnlockRewardType_Song:
	case UnlockRewardType_Steps:
		return m_pSong ? m_pSong->GetBackgroundPath() : "";
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

	vector<RString> asUnlockNames;
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
		lua_call( L, 1, 0 ); 

		if( current.m_bRoulette )
			m_RouletteCodes.insert( current.m_sEntryID );

		m_UnlockEntries.push_back( current );
	}
	LUA->Release(L);

	if( AUTO_LOCK_CHALLENGE_STEPS )
	{
		FOREACH_CONST( Song*, SONGMAN->GetAllSongs(), s )
		{
			// If no hard steps to play to unlock, skip
			if( SongUtil::GetOneSteps(*s, StepsType_Invalid, DIFFICULTY_HARD) == NULL )
				continue;
			
			// If no challenge steps to unlock, skip
			if( SongUtil::GetOneSteps(*s, StepsType_Invalid, DIFFICULTY_CHALLENGE) == NULL )
				continue;

			if( SONGMAN->WasLoadedFromAdditionalSongs(*s) )
				continue;
				
			UnlockEntry ue;			
			ue.m_sEntryID = "_challenge_" + (*s)->GetSongDir();
			ue.m_Type = UnlockRewardType_Steps;
			ue.m_cmd.Load( (*s)->m_sGroupName+"/"+(*s)->GetTranslitFullTitle()+",expert" );
			ue.m_bRequirePassHardSteps = true;

			m_UnlockEntries.push_back( ue );
		}
	}

	// Make sure that we don't have duplicate unlock IDs.  This can cause problems 
	// with UnlockCelebrate and with codes.
	FOREACH_CONST( UnlockEntry, m_UnlockEntries, ue )
		FOREACH_CONST( UnlockEntry, m_UnlockEntries, ue2 )
			if( ue != ue2 )
				ASSERT_M( ue->m_sEntryID != ue2->m_sEntryID, ssprintf("duplicate unlock entry id %s",ue->m_sEntryID.c_str()) );

	UpdateCachedPointers();

	//
	// Log unlocks
	//
	FOREACH_CONST( UnlockEntry, m_UnlockEntries, e )
	{
		RString str = ssprintf( "Unlock: %s; ", join("\n",e->m_cmd.m_vsArgs).c_str() );
		FOREACH_ENUM( UnlockRequirement, j )
			if( e->m_fRequirement[j] )
				str += ssprintf( "%s = %f; ", UnlockRequirementToString(j).c_str(), e->m_fRequirement[j] );
		if( e->m_bRequirePassHardSteps )
			str += "RequirePassHardSteps; ";

		str += ssprintf( "entryID = %s ", e->m_sEntryID.c_str() );
		str += e->IsLocked()? "locked":"unlocked";
		if( e->m_pSong )
			str += ( " (found song)" );
		if( e->m_pCourse )
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
	UNLOCKMAN->GetPoints( PROFILEMAN->GetMachineProfile(), fScores );

	float fSmallestPoints = FLT_MAX;   // or an arbitrarily large value
	for( unsigned a=0; a<m_UnlockEntries.size(); a++ )
		if( m_UnlockEntries[a].m_fRequirement[t] > fScores[t] )
			fSmallestPoints = min( fSmallestPoints, m_UnlockEntries[a].m_fRequirement[t] );
	
	if( fSmallestPoints == FLT_MAX )
		return 0;  // no match found
	return fSmallestPoints - fScores[t];
}

/* Update the cached pointers.  Only call this when it's likely to have changed,
 * such as on load, or when a song title changes in the editor. */
void UnlockManager::UpdateCachedPointers()
{
	FOREACH( UnlockEntry, m_UnlockEntries, e )
	{
		switch( e->m_Type )
		{
		case UnlockRewardType_Song:
			e->m_pSong = SONGMAN->FindSong( e->m_cmd.GetArg(0).s );
			if( e->m_pSong == NULL )
				LOG->Warn( "Unlock: Cannot find song matching \"%s\"", e->m_cmd.GetArg(1).s.c_str() );
			break;
		case UnlockRewardType_Steps:
			e->m_pSong = SONGMAN->FindSong( e->m_cmd.GetArg(0).s );
			if( e->m_pSong == NULL )
			{
				LOG->Warn( "Unlock: Cannot find song matching \"%s\"", e->m_cmd.GetArg(1).s.c_str() );
				break;
			}

			e->m_dc = StringToDifficulty( e->m_cmd.GetArg(1).s );
			if( e->m_dc == Difficulty_Invalid )
			{
				LOG->Warn( "Unlock: Invalid difficulty \"%s\"", e->m_cmd.GetArg(2).s.c_str() );
				break;
			}

			break;
		case UnlockRewardType_Course:
			e->m_pCourse = SONGMAN->FindCourse( e->m_cmd.GetArg(0).s );
			if( e->m_pCourse == NULL )
				LOG->Warn( "Unlock: Cannot find course matching \"%s\"", e->m_cmd.GetArg(1).s.c_str() );
			break;
		case UnlockRewardType_Modifier:
			// nothing to cache
			break;
		default:
			ASSERT(0);
		}
	}
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

void UnlockManager::PreferUnlockEntryID( RString sUnlockEntryID )
{
	for( unsigned i = 0; i < m_UnlockEntries.size(); ++i )
	{
		UnlockEntry &pEntry = m_UnlockEntries[i];
		if( pEntry.m_sEntryID != sUnlockEntryID )
			continue;

		if( pEntry.m_pSong != NULL )
			GAMESTATE->m_pPreferredSong = pEntry.m_pSong;
		if( pEntry.m_pCourse != NULL )
			GAMESTATE->m_pPreferredCourse = pEntry.m_pCourse;
	}
}

int UnlockManager::GetNumUnlocks() const
{
	return m_UnlockEntries.size();
}

int UnlockManager::GetNumUnlocked() const
{
	int count = 0;
	FOREACH_CONST( UnlockEntry, m_UnlockEntries, ue )
	{
		if( ue->GetUnlockEntryStatus() == UnlockEntryStatus_Unlocked )
			count++;
	}
	return count;
}

int UnlockManager::GetUnlockEntryIndexToCelebrate() const
{
	FOREACH_CONST( UnlockEntry, m_UnlockEntries, ue )
	{
		if( ue->GetUnlockEntryStatus() == UnlockEntryStatus_RequirementsMet )
			return ue - m_UnlockEntries.begin();
	}
	return -1;
}

bool UnlockManager::AnyUnlocksToCelebrate() const
{
	return GetUnlockEntryIndexToCelebrate() != -1;
}

void UnlockManager::GetUnlocksByType( UnlockRewardType t, vector<UnlockEntry *> &apEntries )
{
	for( unsigned i = 0; i < m_UnlockEntries.size(); ++i )
		if( m_UnlockEntries[i].IsValid() && m_UnlockEntries[i].m_Type == t )
			apEntries.push_back( &m_UnlockEntries[i] );
}

void UnlockManager::GetSongsUnlockedByEntryID( vector<Song *> &apSongsOut, RString sUnlockEntryID )
{
	vector<UnlockEntry *> apEntries;
	GetUnlocksByType( UnlockRewardType_Song, apEntries );

	for( unsigned i = 0; i < apEntries.size(); ++i )
		if( apEntries[i]->m_sEntryID == sUnlockEntryID )
			apSongsOut.push_back( apEntries[i]->m_pSong );
}

void UnlockManager::GetStepsUnlockedByEntryID( vector<Song *> &apSongsOut, vector<Difficulty> &apDifficultyOut, RString sUnlockEntryID )
{
	vector<UnlockEntry *> apEntries;
	GetUnlocksByType( UnlockRewardType_Steps, apEntries );

	for( unsigned i = 0; i < apEntries.size(); ++i )
	{
		if( apEntries[i]->m_sEntryID == sUnlockEntryID )
		{
			apSongsOut.push_back( apEntries[i]->m_pSong );
			apDifficultyOut.push_back( apEntries[i]->m_dc );
		}
	}
}


// lua start
#include "LuaBinding.h"

class LunaUnlockEntry: public Luna<UnlockEntry>
{
public:
	static int IsLocked( T* p, lua_State *L )		{ lua_pushboolean(L, p->IsLocked() ); return 1; }
	static int GetDescription( T* p, lua_State *L )		{ lua_pushstring(L, p->GetDescription() ); return 1; }
	static int GetUnlockRewardType( T* p, lua_State *L )	{ lua_pushnumber(L, p->m_Type ); return 1; }
	static int GetRequirement( T* p, lua_State *L )		{ UnlockRequirement i = Enum::Check<UnlockRequirement>( L, 1 ); lua_pushnumber(L, p->m_fRequirement[i] ); return 1; }
	static int GetRequirePassHardSteps( T* p, lua_State *L ){ lua_pushboolean(L, p->m_bRequirePassHardSteps); return 1; }

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
	static int course( T* p, lua_State *L ) { GetArgs( p, L ); p->m_Type = UnlockRewardType_Course; return 0; }
	static int mod( T* p, lua_State *L )	{ GetArgs( p, L ); p->m_Type = UnlockRewardType_Modifier; return 0; }
	static int code( T* p, lua_State *L )	{ p->m_sEntryID = SArg(1); return 0; }
	static int roulette( T* p, lua_State *L ) { p->m_bRoulette = true; return 0; }
	static int requirepasshardsteps( T* p, lua_State *L ) { p->m_bRequirePassHardSteps = true; return 0; }
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
		ADD_METHOD( GetDescription );
		ADD_METHOD( GetUnlockRewardType );
		ADD_METHOD( GetRequirement );
		ADD_METHOD( GetRequirePassHardSteps );
		ADD_METHOD( GetRequirePassHardSteps );
		ADD_METHOD( song );
		ADD_METHOD( steps );
		ADD_METHOD( course );
		ADD_METHOD( mod );
		ADD_METHOD( code );
		ADD_METHOD( roulette );
		ADD_METHOD( requirepasshardsteps );
		ADD_METHOD( require );
	}
};

LUA_REGISTER_CLASS( UnlockEntry )

class LunaUnlockManager: public Luna<UnlockManager>
{
public:
	static int FindEntryID( T* p, lua_State *L )			{ RString sName = SArg(1); RString s = p->FindEntryID(sName); if( s.empty() ) lua_pushnil(L); else lua_pushstring(L, s); return 1; }
	static int UnlockEntryID( T* p, lua_State *L )			{ RString sUnlockEntryID = SArg(1); p->UnlockEntryID(sUnlockEntryID); return 0; }
	static int UnlockEntryIndex( T* p, lua_State *L )		{ int iUnlockEntryID = IArg(1); p->UnlockEntryIndex(iUnlockEntryID); return 0; }
	static int PreferUnlockEntryID( T* p, lua_State *L )		{ RString sUnlockEntryID = SArg(1); p->PreferUnlockEntryID(sUnlockEntryID); return 0; }
	static int GetNumUnlocks( T* p, lua_State *L )			{ lua_pushnumber( L, p->GetNumUnlocks() ); return 1; }
	static int GetNumUnlocked( T* p, lua_State *L )			{ lua_pushnumber( L, p->GetNumUnlocked() ); return 1; }
	static int AnyUnlocksToCelebrate( T* p, lua_State *L )		{ lua_pushboolean( L, p->AnyUnlocksToCelebrate() ); return 1; }
	static int GetUnlockEntry( T* p, lua_State *L )			{ int iIndex = IArg(1); p->m_UnlockEntries[iIndex].PushSelf(L); return 1; }
	static int GetSongsUnlockedByEntryID( T* p, lua_State *L )
	{
		vector<Song *> apSongs;
		UNLOCKMAN->GetSongsUnlockedByEntryID( apSongs, SArg(1) );
		LuaHelpers::CreateTableFromArray( apSongs, L );
		return 1;
	}

	static int GetStepsUnlockedByEntryID( T* p, lua_State *L )
	{
		// Return the song each steps are associated with, too.
		vector<Song *> apSongs;
		vector<Difficulty> apDifficulty;
		UNLOCKMAN->GetStepsUnlockedByEntryID( apSongs, apDifficulty, SArg(1) );
		LuaHelpers::CreateTableFromArray( apSongs, L );
		LuaHelpers::CreateTableFromArray( apDifficulty, L );
		return 2;
	}

	LunaUnlockManager()
	{
		ADD_METHOD( FindEntryID );
		ADD_METHOD( UnlockEntryID );
		ADD_METHOD( UnlockEntryIndex );
		ADD_METHOD( PreferUnlockEntryID );
		ADD_METHOD( GetNumUnlocks );
		ADD_METHOD( GetNumUnlocked );
		ADD_METHOD( AnyUnlocksToCelebrate );
		ADD_METHOD( GetUnlockEntry );
		ADD_METHOD( GetSongsUnlockedByEntryID );
		ADD_METHOD( GetStepsUnlockedByEntryID );
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
