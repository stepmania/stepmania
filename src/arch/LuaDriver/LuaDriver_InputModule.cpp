#include "global.h"
#include "LuaDriver_InputModule.h"

/* InputModules need unique InputDevices assigned to them for ButtonPressed,
 * so we allocate from the joystick device set and track allocations. */

namespace
{
	vector<bool> vbDevicesReserved( NUM_JOYSTICKS, false );

	InputDevice GetFirstFreeInputDevice()
	{
		/* UGLY: I'm not sure if we can tell which devices are used
		 * by other drivers. Start at DEVICE_JOY5 to avoid conflicts. */

		for( int i = (DEVICE_JOY5 - DEVICE_JOY1); i < NUM_JOYSTICKS; ++i )
			if( !vbDevicesReserved[i] )
				return InputDevice(DEVICE_JOY1 + i);

		FAIL_M( "LuaDriver ran out of free InputDevices" );
	}

	void ClaimInputDevice( InputDevice id )
	{
		unsigned idx = id - DEVICE_JOY1;
		DEBUG_ASSERT( !vbDevicesReserved[idx] );
		vbDevicesReserved[idx] = true;
	}

	void ReleaseInputDevice( InputDevice id )
	{
		unsigned idx = id - DEVICE_JOY1;
		DEBUG_ASSERT( vbDevicesReserved[idx] );
		vbDevicesReserved[idx] = false;
	}
}

/*
 * LuaDriver_InputModule functions
 */

LuaDriver_InputModule::LuaDriver_InputModule( const RString &sName ) :
	LuaDriver(sName), InputHandler()
{
	// woop woop
	m_InputDevice = InputDevice_Invalid;
}

LuaDriver_InputModule::~LuaDriver_InputModule()
{
	if( m_InputDevice != InputDevice_Invalid )
		ReleaseInputDevice( m_InputDevice );
}

/* XXX: we need an InputDevice before the input thread is started in
 * LuaDriver::ModuleInit(); reserve one, and release it if init fails. */

bool LuaDriver_InputModule::ModuleInit( Lua *L )
{
	InputDevice id = GetFirstFreeInputDevice();
	ClaimInputDevice( id );

	bool bInitted = LuaDriver::ModuleInit( L );

	if( bInitted )
		m_InputDevice = id;
	else
		ReleaseInputDevice( id );

	return bInitted;
}

void LuaDriver_InputModule::Update()
{
	if( m_bThreaded || !m_bRunning )
		return;

	Lua *L = LUA->Get();
	this->ModuleUpdate( L ); /* reports ButtonPressed events */
	LUA->Release( L );

	this->UpdateTimer();	/* updates polled input timestamps */
}

void LuaDriver_InputModule::ModuleThread()
{
	while( m_bRunning )
	{
		Lua *L = LUA->Get();
		this->ModuleUpdate( L );
		LUA->Release( L );
	}
}

void LuaDriver_InputModule::GetDevicesAndDescriptions( vector<InputDeviceInfo> &vDevicesOut )
{
	if( m_bRunning && m_InputDevice != InputDevice_Invalid )
		vDevicesOut.push_back( InputDeviceInfo(m_InputDevice, m_sDesc) );
}

void LuaDriver_InputModule::ButtonPressed( DeviceButton db, float level )
{
	DeviceInput di( m_InputDevice, db, level );

	/* If we're threaded, update the timstamp */
	if( !m_bThreaded )
		di.ts.Touch();

	InputHandler::ButtonPressed( di );
}
