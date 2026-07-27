#include "global.h"
#include "AnnouncerManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageFile.h"

#include <cstring>
#include <vector>


AnnouncerManager*	ANNOUNCER = nullptr; // global and accessible from anywhere in our program


const RString EMPTY_ANNOUNCER_NAME = "Empty";
const RString ANNOUNCERS_DIR  = "Announcers/";

AnnouncerManager::AnnouncerManager()
{
	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "ANNOUNCER" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}
}

AnnouncerManager::~AnnouncerManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "ANNOUNCER" );
}

void AnnouncerManager::GetAnnouncerNames( std::vector<RString>& AddTo )
{
	GetDirListing( ANNOUNCERS_DIR+"*", AddTo, true );

	StripCvsAndSvn( AddTo );
	StripMacResourceForks( AddTo );

	// strip out the empty announcer folder
	for( int i=AddTo.size()-1; i>=0; i-- )
		if( !strcasecmp( AddTo[i], EMPTY_ANNOUNCER_NAME ) )
			AddTo.erase(AddTo.begin()+i, AddTo.begin()+i+1 );
}

bool AnnouncerManager::DoesAnnouncerExist( RString sAnnouncerName )
{
	if( sAnnouncerName == "" )
		return true;

	std::vector<RString> asAnnouncerNames;
	GetAnnouncerNames( asAnnouncerNames );
	for( unsigned i=0; i<asAnnouncerNames.size(); i++ )
		if( 0==strcasecmp(sAnnouncerName, asAnnouncerNames[i]) )
			return true;
	return false;
}

RString AnnouncerManager::GetAnnouncerDirFromName( RString sAnnouncerName )
{
	return ANNOUNCERS_DIR + sAnnouncerName + "/";
}

void AnnouncerManager::SwitchAnnouncer( RString sNewAnnouncerName )
{
	if( !DoesAnnouncerExist(sNewAnnouncerName) )
		m_sCurAnnouncerName = "";
	else
		m_sCurAnnouncerName = sNewAnnouncerName;
}

/* Aliases for announcer paths.  This is for compatibility, so we don't force
 * announcer changes along with everything else.  We could use it to support
 * DWI announcers transparently, too. */
static const char *aliases[][2] = {
	/* ScreenSelectDifficulty compatibility: */
	{ "ScreenSelectDifficulty comment beginner", "select difficulty comment beginner" },
	{ "ScreenSelectDifficulty comment easy", "select difficulty comment easy" },
	{ "ScreenSelectDifficulty comment medium", "select difficulty comment medium" },
	{ "ScreenSelectDifficulty comment hard", "select difficulty comment hard" },
	{ "ScreenSelectDifficulty comment oni", "select difficulty comment oni" },
	{ "ScreenSelectDifficulty comment nonstop", "select difficulty comment nonstop" },
	{ "ScreenSelectDifficulty comment endless", "select difficulty comment endless" },
	{ "ScreenSelectDifficulty intro", "select difficulty intro" },

	/* ScreenSelectStyle compatibility: */
	{ "ScreenSelectStyle intro", "select style intro" },
	{ "ScreenSelectStyle comment single", "select style comment single" },
	{ "ScreenSelectStyle comment double", "select style comment double" },
	{ "ScreenSelectStyle comment solo", "select style comment solo" },
	{ "ScreenSelectStyle comment versus", "select style comment versus" },

	/* Combo compatibility: */
	{ "gameplay combo 100", "gameplay 100 combo" },
	{ "gameplay combo 200", "gameplay 200 combo" },
	{ "gameplay combo 300", "gameplay 300 combo" },
	{ "gameplay combo 400", "gameplay 400 combo" },
	{ "gameplay combo 500", "gameplay 500 combo" },
	{ "gameplay combo 600", "gameplay 600 combo" },
	{ "gameplay combo 700", "gameplay 700 combo" },
	{ "gameplay combo 800", "gameplay 800 combo" },
	{ "gameplay combo 900", "gameplay 900 combo" },
	{ "gameplay combo 1000", "gameplay 1000 combo" },

	{ nullptr, nullptr }
};

/* Find an announcer directory with sounds in it.  First search sFolderName,
 * then all aliases above.  Ignore directories that are empty, since we might
 * have "select difficulty intro" with sounds and an empty "ScreenSelectDifficulty
 * intro". */
RString AnnouncerManager::GetPathTo( RString sAnnouncerName, RString sFolderName )
{
	if(sAnnouncerName == "")
		return RString(); /* announcer disabled */

	const RString AnnouncerPath = GetAnnouncerDirFromName(sAnnouncerName);

	if( !DirectoryIsEmpty(AnnouncerPath+sFolderName+"/") )
		return AnnouncerPath+sFolderName+"/";

	/* Search for the announcer folder in the list of aliases. */
	int i;
	for(i = 0; aliases[i][0] != nullptr; ++i)
	{
		if(!sFolderName.EqualsNoCase(aliases[i][0]))
			continue; /* no match */

		if( !DirectoryIsEmpty(AnnouncerPath+aliases[i][1]+"/") )
			return AnnouncerPath+aliases[i][1]+"/";
	}

	/* No announcer directory matched.  In debug, create the directory by
	 * its preferred name. */
#ifdef DEBUG
	LOG->Trace( "The announcer in '%s' is missing the folder '%s'.",
		AnnouncerPath.c_str(), sFolderName.c_str() );
//	MessageBeep( MB_OK );
	RageFile temp;
	temp.Open( AnnouncerPath+sFolderName + "/announcer files go here.txt", RageFile::WRITE );
#endif

	return RString();
}

RString AnnouncerManager::GetPathTo( RString sFolderName )
{
	return GetPathTo(m_sCurAnnouncerName, sFolderName);
}

bool AnnouncerManager::HasSoundsFor( RString sFolderName )
{
	return !DirectoryIsEmpty( GetPathTo(sFolderName) );
}

void AnnouncerManager::NextAnnouncer()
{
	std::vector<RString> as;
	GetAnnouncerNames( as );
	if( as.size()==0 )
		return;

	if( m_sCurAnnouncerName == "" )
		SwitchAnnouncer( as[0] );
	else
	{
		unsigned i;
		for( i=0; i<as.size(); i++ )
			if( as[i].EqualsNoCase(m_sCurAnnouncerName) )
				break;
		if( i==as.size()-1 )
			SwitchAnnouncer( "" );
		else
		{
			int iNewIndex = (i+1)%as.size();
			SwitchAnnouncer( as[iNewIndex] );
		}
	}
}
// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the AnnouncerManager. */
class LunaAnnouncerManager: public Luna<AnnouncerManager>
{
public:
	static int DoesAnnouncerExist( T* p, lua_State *L ) { lua_pushboolean(L, p->DoesAnnouncerExist( SArg(1) )); return 1; }
	static int GetAnnouncerNames( T* p, lua_State *L )
	{
		std::vector<RString> vAnnouncers;
		p->GetAnnouncerNames( vAnnouncers );
		LuaHelpers::CreateTableFromArray(vAnnouncers, L);
		return 1;
	}
	static int GetCurrentAnnouncer( T* p, lua_State *L )
	{
		RString s = p->GetCurAnnouncerName();
		if( s.empty() )
		{
			lua_pushnil(L);
		}
		else
		{
			lua_pushstring(L, s );
		}
		return 1;
	}
	static int SetCurrentAnnouncer( T* p, lua_State *L )
	{
		RString s = SArg(1);
		// only bother switching if the announcer exists. -aj
		if(p->DoesAnnouncerExist(s))
			p->SwitchAnnouncer(s);
		COMMON_RETURN_SELF;
	}

	LunaAnnouncerManager()
	{
		ADD_METHOD( DoesAnnouncerExist );
		ADD_METHOD( GetAnnouncerNames );
		ADD_METHOD( GetCurrentAnnouncer );
		ADD_METHOD( SetCurrentAnnouncer );
	}
};

LUA_REGISTER_CLASS( AnnouncerManager )
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
