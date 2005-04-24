#include "global.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "song.h"
#include "Course.h"
#include "RageUtil.h"
#include "UnlockManager.h"
#include "SongManager.h"
#include "GameState.h"
#include "ProfileManager.h"
#include "ThemeManager.h"
#include "Foreach.h"
#include <float.h>

UnlockManager*	UNLOCKMAN = NULL;	// global and accessable from anywhere in our program

#define UNLOCK_NAMES					THEME->GetMetric ("Unlocks","UnlockNames")
#define UNLOCK(sLineName)				THEME->GetMetric ("Unlocks",ssprintf("Unlock%s",sLineName.c_str()))

static CString UnlockTypeNames[NUM_UNLOCK_TYPES] =
{
	"ArcadePoints",
	"DancePoints",
	"SongPoints",
	"ExtraCleared",
	"ExtraFailed",
	"Toasties",
	"StagesCleared"
};
XToString( UnlockType, NUM_UNLOCK_TYPES );
StringToX( UnlockType );

UnlockManager::UnlockManager()
{
	UNLOCKMAN = this;

	Load();
}

void UnlockManager::UnlockSong( const Song *song )
{
	const UnlockEntry *p = FindSong( song );
	if( !p )
		return;  // does not exist
	if( p->m_iCode == -1 )
		return;

	UnlockCode( p->m_iCode );
}

int UnlockManager::FindCode( const CString &sName ) const
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
		return -1;
	}

	return pEntry->m_iCode;
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
	if( !p )
		return false;

	/* If the song is locked by a code, and it's a roulette code, honor IsLocked. */
	if( p->m_iCode == -1 || m_RouletteCodes.find( p->m_iCode ) == m_RouletteCodes.end() )
		return false;

	return p->IsLocked();
}

bool UnlockManager::ModifierIsLocked( const CString &sOneMod ) const
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindModifier( sOneMod );
	if( p == NULL )
		return false;

	return p->IsLocked();
}


const UnlockEntry *UnlockManager::FindLockEntry( CString songname ) const
{
	FOREACH_CONST( UnlockEntry, m_UnlockEntries, e )
		if( !songname.CompareNoCase(e->m_sName) )
			return &(*e);
	return NULL;
}

const UnlockEntry *UnlockManager::FindSong( const Song *pSong ) const
{
	FOREACH_CONST( UnlockEntry, m_UnlockEntries, e )
		if( e->m_pSong == pSong )
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

const UnlockEntry *UnlockManager::FindModifier( const CString &sOneMod ) const
{
	FOREACH_CONST( UnlockEntry, m_UnlockEntries, e )
		if( e->m_sName.CompareNoCase(sOneMod) == 0 )
			return &(*e);
	return NULL;
}


UnlockEntry::UnlockEntry()
{
	memset( m_fRequired, 0, sizeof(m_fRequired) );
	m_iCode = -1;

	m_pSong = NULL;
	m_pCourse = NULL;
}

static float GetArcadePoints( const Profile *pProfile )
{
	float fAP =	0;

	FOREACH_Grade(g)
	{
		switch(g)
		{
		case GRADE_TIER01:
		case GRADE_TIER02:	fAP += 9 * pProfile->m_iNumStagesPassedByGrade[g]; break;
		default:			fAP += 1 * pProfile->m_iNumStagesPassedByGrade[g]; break;

		case GRADE_FAILED:
		case GRADE_NO_DATA:
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
		case GRADE_TIER01:/*AAAA*/	fSP += 20 * pProfile->m_iNumStagesPassedByGrade[g];	break;
		case GRADE_TIER02:/*AAA*/	fSP += 10* pProfile->m_iNumStagesPassedByGrade[g];	break;
		case GRADE_TIER03:/*AA*/	fSP += 5* pProfile->m_iNumStagesPassedByGrade[g];	break;
		case GRADE_TIER04:/*A*/		fSP += 4* pProfile->m_iNumStagesPassedByGrade[g];	break;
		case GRADE_TIER05:/*B*/		fSP += 3* pProfile->m_iNumStagesPassedByGrade[g];	break;
		case GRADE_TIER06:/*C*/		fSP += 2* pProfile->m_iNumStagesPassedByGrade[g];	break;
		case GRADE_TIER07:/*D*/		fSP += 1* pProfile->m_iNumStagesPassedByGrade[g];	break;
		case GRADE_FAILED:
		case GRADE_NO_DATA:
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

void UnlockManager::GetPoints( const Profile *pProfile, float fScores[NUM_UNLOCK_TYPES] ) const
{
	fScores[UNLOCK_ARCADE_POINTS] = GetArcadePoints( pProfile );
	fScores[UNLOCK_SONG_POINTS] = GetSongPoints( pProfile );
	fScores[UNLOCK_DANCE_POINTS] = (float) pProfile->m_iTotalDancePoints;
	fScores[UNLOCK_CLEARED] = (float) pProfile->GetTotalNumSongsPassed();
}

bool UnlockEntry::IsLocked() const
{
	float fScores[NUM_UNLOCK_TYPES];
	UNLOCKMAN->GetPoints( PROFILEMAN->GetMachineProfile(), fScores );

	for( int i = 0; i < NUM_UNLOCK_TYPES; ++i )
		if( m_fRequired[i] && fScores[i] >= m_fRequired[i] )
			return false;

	if( m_iCode != -1 && PROFILEMAN->GetMachineProfile()->m_UnlockedSongs.find(m_iCode) != PROFILEMAN->GetMachineProfile()->m_UnlockedSongs.end() )
		return false;

	return true;
}

void UnlockManager::Load()
{
	LOG->Trace( "UnlockManager::Load()" );

	CStringArray asUnlockNames;
	split( UNLOCK_NAMES, ",", asUnlockNames );
	if( asUnlockNames.empty() )
		return;

	for( unsigned i = 0; i < asUnlockNames.size(); ++i )
	{
		const CString &sUnlockName = asUnlockNames[i];
		CString sUnlock = UNLOCK(sUnlockName);

		Commands vCommands;
		ParseCommands( sUnlock, vCommands );

		UnlockEntry current;
		bool bRoulette = false;

		for( unsigned j = 0; j < vCommands.v.size(); ++j )
		{
			const Command &cmd = vCommands.v[j];
			CString sName = cmd.GetName();

			if( sName == "song" )
			{
				current.m_Type = UnlockEntry::TYPE_SONG;
				current.m_sName = (CString) cmd.GetArg(1);
			}
			if( sName == "course" )
			{
				current.m_Type = UnlockEntry::TYPE_COURSE;
				current.m_sName = (CString) cmd.GetArg(1);
			}
			if( sName == "mod" )
			{
				current.m_Type = UnlockEntry::TYPE_MODIFIER;
				current.m_sName = (CString) cmd.GetArg(1);
			}
			else if( sName == "code" )
			{
				// Hack: Lua only has a floating point type, and codes may be big enough
				// that converting them from string to float to int introduces rounding
				// error.  Convert directly to int.
				current.m_iCode = atoi( (CString) cmd.GetArg(1) );
			}
			else if( sName == "roulette" )
			{
				bRoulette = true;
			}
			else
			{
				const UnlockType ut = StringToUnlockType( cmd.GetName() );
				if( ut != UNLOCK_INVALID )
					current.m_fRequired[ut] = cmd.GetArg(1);
			}
		}

		if( bRoulette )
			m_RouletteCodes.insert( current.m_iCode );

		m_UnlockEntries.push_back( current );
	}

	UpdateSongs();

	FOREACH_CONST( UnlockEntry, m_UnlockEntries, e )
	{
		CString str = ssprintf( "Unlock: %s; ", e->m_sName.c_str() );
		FOREACH_UnlockType(j)
			if( e->m_fRequired[j] )
				str += ssprintf( "%s = %f; ", UnlockTypeToString(j).c_str(), e->m_fRequired[j] );

		str += ssprintf( "code = %i ", e->m_iCode );
		str += e->IsLocked()? "locked":"unlocked";
		if( e->m_pSong )
			str += ( " (found song)" );
		if( e->m_pCourse )
			str += ( " (found course)" );
		LOG->Trace( "%s", str.c_str() );
	}
	
	return;
}

float UnlockManager::PointsUntilNextUnlock( UnlockType t ) const
{
	float fScores[NUM_UNLOCK_TYPES];
	UNLOCKMAN->GetPoints( PROFILEMAN->GetMachineProfile(), fScores );

	float fSmallestPoints = FLT_MAX;   // or an arbitrarily large value
	for( unsigned a=0; a<m_UnlockEntries.size(); a++ )
		if( m_UnlockEntries[a].m_fRequired[t] > fScores[t] )
			fSmallestPoints = min( fSmallestPoints, m_UnlockEntries[a].m_fRequired[t] );
	
	if( fSmallestPoints == FLT_MAX )
		return 0;  // no match found
	return fSmallestPoints - fScores[t];
}

/* Update the cached pointers.  Only call this when it's likely to have changed,
 * such as on load, or when a song title changes in the editor. */
void UnlockManager::UpdateSongs()
{
	FOREACH( UnlockEntry, m_UnlockEntries, e )
	{
		switch( e->m_Type )
		{
		case UnlockEntry::TYPE_SONG:
			e->m_pSong = SONGMAN->FindSong( e->m_sName );
			if( e->m_pSong == NULL )
				LOG->Warn( "Unlock: Cannot find song matching \"%s\"", e->m_sName.c_str() );
			break;
		case UnlockEntry::TYPE_COURSE:
			e->m_pCourse = SONGMAN->FindCourse( e->m_sName );
			if( e->m_pCourse == NULL )
				LOG->Warn( "Unlock: Cannot find course matching \"%s\"", e->m_sName.c_str() );
			break;
		case UnlockEntry::TYPE_MODIFIER:
			// nothing to cache
			break;
		default:
			ASSERT(0);
		}
	}
}



void UnlockManager::UnlockCode( int num )
{
	FOREACH_PlayerNumber( pn )
		if( PROFILEMAN->IsUsingProfile(pn) )
			PROFILEMAN->GetProfile(pn)->m_UnlockedSongs.insert( num );

	PROFILEMAN->GetMachineProfile()->m_UnlockedSongs.insert( num );
}

void UnlockManager::PreferUnlockCode( int iCode )
{
	for( unsigned i = 0; i < m_UnlockEntries.size(); ++i )
	{
		UnlockEntry &pEntry = m_UnlockEntries[i];
		if( pEntry.m_iCode != iCode )
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

#include "LuaBinding.h"

template<class T>
class LunaUnlockManager: public Luna<T>
{
public:
	LunaUnlockManager() { LUA->Register( Register ); }

	static int FindCode( T* p, lua_State *L )			{ CString sName = SArg(1); int i = p->FindCode(sName); if( i == -1 ) lua_pushnil(L); else lua_pushnumber(L, i); return 1; }
	static int UnlockCode( T* p, lua_State *L )			{ int iCode = IArg(1); p->UnlockCode(iCode); return 0; }
	static int PreferUnlockCode( T* p, lua_State *L )	{ int iCode = IArg(1); p->PreferUnlockCode(iCode); return 0; }

	static void Register(lua_State *L)
	{
		ADD_METHOD( FindCode )
		ADD_METHOD( UnlockCode )
		ADD_METHOD( PreferUnlockCode )
		Luna<T>::Register( L );

		// Add global singleton if constructed already.  If it's not constructed yet,
		// then we'll register it later when we reinit Lua just before 
		// initializing the display.
		if( UNLOCKMAN )
		{
			lua_pushstring(L, "UNLOCKMAN");
			UNLOCKMAN->PushSelf( LUA->L );
			lua_settable(L, LUA_GLOBALSINDEX);
		}
	}
};

LUA_REGISTER_CLASS( UnlockManager )

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
