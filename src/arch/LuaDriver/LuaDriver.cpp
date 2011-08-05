#include "global.h"
#include "RageLog.h"
#include "LuaManager.h"
#include "LuaReference.h"
#include "RageThreads.h"
#include "RageUtil.h"
#include "Foreach.h"
#include "LuaDriver.h"

#include "LuaDriver_InputModule.h"
#include "LuaDriver_LightsModule.h"

/*
 * Static class functions
 */

/* Modules that have successfully loaded and are available to use */
LuaDriverMap LuaDriver::s_mpInputModules;
LuaDriverMap LuaDriver::s_mpLightsModules;

/* These map a ModuleType string to a DriverType */
const char* LuaDriver::DriverTypeNames[NUM_DriverType] = { "Input", "Lights", NULL };

/* Inserts drivers from mModules into AddTo if their names are in sDrivers. */
template<class Driver, class Handler>
static void AddModules( LuaDriverMap &mModules, const RString &sDrivers, vector<Handler*> &AddTo )
{
	LOG->Trace( "LuaDriver::AddModules( %s )", sDrivers.c_str() );
	if( sDrivers.empty() )
		return; // nothing to do

	/* Claim a Lua context for ModuleInit, if we get to it */
	Lua *L = LUA->Get();
	LUA->YieldLua();

	vector<RString> DriversToTry;
	split( sDrivers, ",", DriversToTry, true );

	FOREACH( RString, DriversToTry, s )
	{
		s->MakeLower();
		LuaDriverMap::iterator it = mModules.find( *s );

		if( it == mModules.end() )
			continue;

		LOG->Trace( "Found module \"%s\"", s->c_str() );

		Driver *pDriver = dynamic_cast<Driver*>( it->second );
		Handler *pHandler = dynamic_cast<Handler*>( pDriver );

		if( pHandler == NULL )
			FAIL_M( ssprintf("cast failed for \"%s\"", s->c_str()) );

		LUA->UnyieldLua();
		bool bInitted = pDriver->ModuleInit( L );
		LUA->YieldLua();

		/* if init failed, this driver is useless; delete it */
		if( !bInitted )
		{
			LOG->Warn( "Module \"%s\" not usable", s->c_str() );
			mModules.erase( it );

			LUA->UnyieldLua();
			pDriver->ModuleExit( L ); // it's okay if this fails
			LUA->YieldLua();

			delete pDriver;
			continue;
		}

		LOG->Info( "Lua module: %s\n", s->c_str() );

		AddTo.push_back( pHandler );
	}

	LUA->UnyieldLua();
	LUA->Release( L );
}

void LuaDriver::AddInputModules( const RString &sDrivers, vector<InputHandler*> &AddTo )
{
	AddModules<LuaDriver_InputModule>( s_mpInputModules, sDrivers, AddTo );
}

void LuaDriver::AddLightsModules( const RString &sDrivers, vector<LightsDriver*> &AddTo )
{
	AddModules<LuaDriver_LightsModule>( s_mpLightsModules, sDrivers, AddTo );
}

bool LuaDriver::Load( const RString &sPath )
{
	RString sScript, sError;

	if( !GetFileContents(sPath, sScript) )
		return false;

	Lua *L = LUA->Get();

	if( !LuaHelpers::RunScript(L, sScript, "@" + sPath, sError, 0, 1) )
	{
		lua_pop( L, 1 );	// pop the nil return value
		ASSERT( lua_gettop(L) == 0 );
		LUA->Release( L );

		LOG->Warn( "LuaDriver: error compiling module \"%s\": %s", sPath.c_str(), sError.c_str() );
		return false;
	}

	LuaReference *pTable = new LuaReference;
	pTable->SetFromStack( L );
	pTable->PushSelf( L );

	/* get the driver's name */
	lua_pushstring( L, "Name" );
	lua_gettable( L, -2 );
	const char *sName = lua_tostring(L, -1);
	lua_pop( L, 1 );

	if( sName == NULL )
	{
		LOG->Warn( ssprintf("%s needs a Name!", sPath.c_str()) );
		lua_pop( L, 1 );
		return false;
	}

	/* get the driver type from the ModuleType field */
	lua_pushstring( L, "ModuleType" );
	lua_gettable( L, -2 );

	DriverType driverType = DriverType_Invalid;

	if( lua_type(L, -1) == LUA_TSTRING )
	{
		const char *type = lua_tostring( L, -1 );

		for( unsigned i = 0; DriverTypeNames[i] != NULL; ++i )
		{
			if( strcmp(type, DriverTypeNames[i]) == 0 )
			{
				driverType = DriverType(i);
				break;
			}
		}

		if( driverType == DriverType_Invalid )
			FAIL_M( ssprintf("\"%s\": Invalid ModuleType \"%s\"", sPath.c_str(), type) );
	}
	else
	{
		FAIL_M( "ModuleType must be a string" );
	}

	lua_pop( L, 2 ); // pop the result and table

	ASSERT( lua_gettop(L) == 0 );

	LuaDriver *pDriver = NULL;

	switch( driverType )
	{
	case DriverType_Input:
		pDriver = new LuaDriver_InputModule( sName );
		break;
	case DriverType_Lights:
		pDriver = new LuaDriver_LightsModule( sName );
		break;
	default:
		ASSERT(0);
		break;
	}

	/* Load internal data; the driver claims the pTable ref here. */
	bool bLoaded = pDriver->LoadFromTable( L, pTable );
	LUA->Release( L );

	if( bLoaded )
	{
		RString sName = pDriver->GetName();
		sName.MakeLower();

		switch( driverType )
		{
		case DriverType_Input:
			s_mpInputModules[sName] = pDriver;
			break;
		case DriverType_Lights:
			s_mpLightsModules[sName] = pDriver;
			break;
		default:
			ASSERT(0);
			break;
		}
	}

	return bLoaded;
}

/*
 * Object functions
 */

LuaDriver::LuaDriver( const RString &sName ) : m_sName(sName)
{
	m_bRunning = false;
	m_bThreaded = true; // always thread unless explicitly disabled

	m_pThread = NULL;
	m_pDriver = m_pInit = m_pExit = m_pUpdate = NULL;
}

LuaDriver::~LuaDriver()
{
	if( m_bRunning )
	{
		m_bRunning = false;

		LOG->Trace( "Shutting down module \"%s\"...", m_sName.c_str()  );

		/* if the thread is running, let it complete */
		if( m_pThread && m_pThread->IsCreated() )
			m_pThread->Wait();

		Lua *L = LUA->Get();
		this->ModuleExit( L );
		LUA->Release( L );

		SAFE_DELETE( m_pThread );

		LOG->Trace( "Module \"%s\" shut down.", m_sName.c_str() );
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
		LOG->Warn( "LuaDriver %s: %s", m_sName.c_str(), sError.c_str() );
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
