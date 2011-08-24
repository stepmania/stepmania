#include "global.h"
#include "RageLog.h"

#include "LuaManager.h"
#include "LuaReference.h"
#include "RageThreads.h"
#include "RageUtil.h"
#include "Foreach.h"
#include "Preference.h"

#include "LuaDriver.h"
#include "LuaDriver_InputModule.h"
#include "LuaDriver_LightsModule.h"
#include "LuaDriver_Peripheral.h"

static Preference<RString> g_sPeripherals( "Peripherals", "" );

DriverPathMap LuaDriver::s_mModulePaths[NUM_ModuleType];
vector<LuaDriver_Peripheral*> LuaDriver::m_vpPeripherals;

static const char* ModuleTypeNames[NUM_ModuleType] =
{
	"InputModule",
	"LightsModule",
	"Peripheral"
};

const RString ModuleTypeToString( ModuleType type )
{
	return ModuleTypeNames[type];
}

ModuleType StringToModuleType( const RString &str )
{
	FOREACH_ENUM( ModuleType, type )
		if(str.compare(ModuleTypeNames[type]) == 0)
			return type;

	return ModuleType_Invalid;
}

/*
 * Global namespace functions
 */

/* If any names from sDrivers are found in mModules, instantiate the
 * corresponding LuaDriver, cast it to Handler, and insert into AddTo. */
template<class Driver, class Handler>
static void AddModules( DriverPathMap &mModules, const RString &sDrivers, vector<Handler*> &AddTo )
{
	LOG->Trace( "LuaDriver::AddModules( %s )", sDrivers.c_str() );

	if( sDrivers.empty() )
		return; // nothing to do

	/* Claim a Lua context to use (later) for loading and ModuleInit. */
	Lua *L = LUA->Get();
	LUA->YieldLua();

	vector<RString> DriversToTry;
	split( sDrivers, ",", DriversToTry, true );

	FOREACH( RString, DriversToTry, s )
	{
		s->MakeLower();
		DriverPathMap::iterator it = mModules.find( *s );

		if( it == mModules.end() )
			continue; // driver not found

		const RString &sName = it->first;
		const RString &sPath = it->second;

		LOG->Trace( "Initializing module %s", sPath.c_str() );

		RString sScript, sError;

		if( !GetFileContents(sPath, sScript) )
			continue;

		// from here on, we start using our context; make sure to
		// re-yield on any code path branching from here.
		LUA->UnyieldLua();

		if( !LuaHelpers::RunScript(L, sScript, "@"+sPath, sError, 0, 1) )
		{
			lua_pop( L, 1 ); // remove the return value
			LUA->YieldLua();

			LOG->Warn( "LuaDriver: error compiling module \"%s\": %s",
				sPath.c_str(), sError.c_str() );

			continue;
		}

		// Set a reference to the returned table
		LuaReference *pTable = new LuaReference;
		pTable->SetFromStack( L );

		ASSERT( lua_gettop(L) == 0 );
		LuaDriver *pDriver = new Driver( sName );

		// pass pTable to pDriver; it takes ownership of the pointer
		if( !pDriver->LoadFromTable(L, pTable) )
		{
			delete pDriver; // deletes pTable for us
			LUA->YieldLua();
			continue;
		}

		/* our LuaDriver is now loaded; try to initialize it */
		bool bInitted = pDriver->ModuleInit( L );

		if( bInitted )
		{
			// dynamic_cast to the inherited handler type
			Handler *pHandler = dynamic_cast<Handler*>( pDriver );
			ASSERT_M( pHandler != NULL, "dynamic cast failed" );

			LOG->Trace( "Lua module initialized: %s", sName.c_str() );
			AddTo.push_back( pHandler );
		}
		else
		{
			LOG->Warn( "Module \"%s\" failed to initialize", sPath.c_str() );

			// deinit and deallocate the driver
			pDriver->ModuleExit( L );
			delete pDriver;
		}

		// re-yield for the next iteration
		LUA->YieldLua();
	}

	LUA->UnyieldLua();
	LUA->Release( L );
}

void LuaDriver::AddInputModules( const RString &sDrivers, vector<InputHandler*> &AddTo )
{
	DriverPathMap &m = s_mModulePaths[ModuleType_Input];
	AddModules<LuaDriver_InputModule>( m, sDrivers, AddTo );
}

void LuaDriver::AddLightsModules( const RString &sDrivers, vector<LightsDriver*> &AddTo )
{
	DriverPathMap &m = s_mModulePaths[ModuleType_Input];
	AddModules<LuaDriver_LightsModule>( m, sDrivers, AddTo );
}

void LuaDriver::LoadPeripherals()
{
/*
	DriverPathMap &m = s_mModulePaths[ModuleType_Peripheral];
	AddModules<LuaDriver_Peripheral>( m, g_sPeripherals, m_vpPeripherals );
*/
}

void LuaDriver::Update( float fDeltaTime )
{
	if( m_vpPeripherals.empty() )
		return;

	Lua *L = LUA->Get();

/*
	for( unsigned i = 0; i < m_vpPeripherals.size(); ++i )
		m_vpPeripherals[i]->ModuleUpdate( L, fDeltaTime );
*/
	LUA->Release( L );
}

void LuaDriver::LoadModulesDir( const RString &sDir )
{
	vector<RString> vsModules;
	GetDirListing( sDir + "/*.lua", vsModules, false, true ); /* nano! */

	for( unsigned i = 0; i < vsModules.size(); ++i )
	{
		const RString &sPath = vsModules[i];

		RString sFilename = GetFileNameWithoutExtension( sPath );
		RString sName, sType;

		LOG->Trace( "Loading file \"%s\" -> %s", sPath.c_str(), sFilename.c_str() );

		// split the file name into its ModuleType and name; the
		// scope will de-allocate type and name when we're done
		{
			char type[16], name[21];
			int num = sscanf( sFilename.c_str(), "%15[^_]_%20[^_]", type, name );

			if( num != 2 )
			{
				LOG->Warn( "Invalid driver name \"%s\"", sName.c_str() );
				continue;
			}

			sType.assign( type );
			sName.assign( name );
		}

		ModuleType type = StringToModuleType( sType );

		if( type == ModuleType_Invalid )
		{
			LOG->Warn( "Invalid ModuleType \"%s\"", sType.c_str() );
			continue;
		}

		// must be lowercase to match case insensitively
		sName.MakeLower();

		// add this module to its appropriate map
		s_mModulePaths[type].insert( pair<RString,RString>(sName, sPath) );
	}
}

/*
 * Object functions
 */

LuaDriver::LuaDriver( const RString &sName ) : m_sName(sName)
{
	m_bRunning = false;
	m_bThreaded = false;

	m_pThread = NULL;
	m_pDriver = m_pInit = m_pExit = m_pUpdate = NULL;
}

LuaDriver::~LuaDriver()
{
	if( m_bRunning )
	{
		Lua *L = LUA->Get();
		this->ModuleExit( L );
		LUA->Release( L );
		m_bRunning = false;
	}

	SAFE_DELETE( m_pThread );
	SAFE_DELETE( m_pExit );
	SAFE_DELETE( m_pUpdate );
	SAFE_DELETE( m_pInit );
	SAFE_DELETE( m_pDriver );
}

bool LuaDriver::ModuleInit( Lua *L )
{
	RString sError;

	/* call the init function, passing the driver table as 'self' */
	m_pInit->PushSelf( L );
	m_pDriver->PushSelf( L );

	bool bSuccess;

	// TODO: error checking
	LuaHelpers::RunScriptOnStack( L, sError, 1, 1 ); // 1 arg, 1 result
	LuaHelpers::Pop( L, bSuccess );

	if( !sError.empty() )
	{
		LOG->Warn( "[LuaDriver::ModuleInit] %s: %s", m_sName.c_str(), sError.c_str() );
		return false;
	}

	m_bRunning = true;

	/* If we're going to be threaded, initialise that now. */
	if( bSuccess && IsThreaded() )
	{
		m_pThread = new RageThread;
		m_pThread->SetName( ssprintf("%s module thread", m_sName.c_str()) );
		m_pThread->Create( ModuleThread_Start, this );
	}

	return bSuccess;
}

void LuaDriver::ModuleExit( Lua *L )
{
	m_bRunning = false;

	LOG->Trace( "Shutting down module \"%s\"...", m_sName.c_str()  );

	/* if the thread is doing something, let it complete */
	if( m_pThread && m_pThread->IsCreated() )
		m_pThread->Wait();

	RString sError;

	/* call the exit function, passing the driver table as 'self' */
	m_pExit->PushSelf( L );
	m_pDriver->PushSelf( L );

	// TODO: error checking
	LuaHelpers::RunScriptOnStack( L, sError, 1, 0 ); // 1 arg, 0 results

	LOG->Trace( "Module \"%s\" shut down.", m_sName.c_str() );
}

void LuaDriver::ModuleUpdate( Lua *L )
{
	RString sError;

	/* call the update function, passing the driver table as 'self' */
	m_pUpdate->PushSelf( L );
	m_pDriver->PushSelf( L );

	LuaHelpers::RunScriptOnStack( L, sError, 1, 0 ); // 1 arg, 0 results

	if( !sError.empty() )
		LOG->Warn( "LuaDriver::ModuleUpdate(%s): %s", m_sName.c_str(), sError.c_str() );
}

bool LuaDriver::LoadFromTable( Lua *L, LuaReference *pTable )
{
	/* claim ownership of the driver table's LuaReference */
	m_pDriver = pTable;

	pTable->PushSelf( L );

	/* Get an optional description */
	lua_pushstring( L, "Desc" );
	lua_gettable( L, -2 );

	if( lua_isstring(L, -1) )
		m_sDesc.assign( lua_tostring(L, -1) );

	lua_pop( L, 1 );

	LuaReference** ppRefs[] = { &m_pInit, &m_pExit, &m_pUpdate, NULL };
	const char *names[] = { "Init", "Exit", "Update", NULL };

	ASSERT( lua_gettop(L) == 1 ); // driver table

	for( unsigned i = 0; ppRefs[i] != NULL && names[i] != NULL; ++i )
	{
		// bring the function described by names[i] to the top
		lua_pushstring( L, names[i] );
		lua_gettable( L, -2 );

		if( lua_type(L, -1) == LUA_TFUNCTION )
		{
			*ppRefs[i] = new LuaReference;
			(*ppRefs[i])->SetFromStack( L );
		}
		else
		{
			LOG->Warn( "LuaDriver::Load(): invalid type for \"%s\" (function expected, got %s)",
				names[i], luaL_typename(L, -1) );

			// pop the result and table
			lua_pop( L, 2 );

			// the module cannot be loaded; clean up and exit
			for( unsigned j = 0; names[j] != NULL; ++j )
				SAFE_DELETE( *ppRefs[i] );
			SAFE_DELETE( m_pDriver );

			return false;
		}
	}

	lua_pop( L, 1 ); // pop the table
	ASSERT( lua_gettop(L) == 0 );

	/* base functions are loaded - now load more specific driver data */
	return this->LoadDerivedFromTable( L, pTable );
}

/*
 * (c) 2011 Mark Cannon
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

