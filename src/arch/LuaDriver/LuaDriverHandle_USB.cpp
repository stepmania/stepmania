#include "global.h"
#include "RageLog.h"
#include "LuaManager.h"
#include "LuaDriverHandle_USB.h"

#include <libusb-1.0/libusb.h>

REGISTER_LUA_DRIVER_HANDLE( USB );

const unsigned USB_API_REVISION_MAJOR = 0;
const unsigned USB_API_REVISION_MINOR = 1;

int LuaDriverHandle_USB::GetRevisionMajor() const { return USB_API_REVISION_MAJOR; }
int LuaDriverHandle_USB::GetRevisionMinor() const { return USB_API_REVISION_MINOR; }

LuaDriverHandle_USB::LuaDriverHandle_USB()
{
	m_iError = libusb_init( &m_pContext );

	if( m_iError != LIBUSB_SUCCESS )
	{
		LOG->Warn( "libusb_init error: %s\n", GetErrorStr(m_iError) );
		return;
	}

	libusb_set_debug( m_pContext, 3 ); // maximum debugging

	m_pHandle = NULL;
}

LuaDriverHandle_USB::~LuaDriverHandle_USB()
{
	libusb_exit( m_pContext );
}

bool LuaDriverHandle_USB::Open( uint16_t iVendorID, uint16_t iProductID )
{
	m_pHandle = libusb_open_device_with_vid_pid( m_pContext, iVendorID, iProductID );
	return m_pHandle != NULL;
}

bool LuaDriverHandle_USB::IsOpen() const
{
	return m_pHandle != NULL;
}

void LuaDriverHandle_USB::Close()
{
	if( !m_pHandle )
		return;

	libusb_reset_device( m_pHandle );
	libusb_close( m_pHandle );
}

/* Enumeration/handling functions */
int LuaDriverHandle_USB::GetConfiguration()
{
	int config;
	m_iError = libusb_get_configuration( m_pHandle, &config );
	return config;
}

bool LuaDriverHandle_USB::SetConfiguration( int config )
{
	m_iError = libusb_set_configuration( m_pHandle, config );
	return m_iError == LIBUSB_SUCCESS;
}

bool LuaDriverHandle_USB::ClaimInterface( int interface )
{
	m_iError = libusb_claim_interface( m_pHandle, interface );
	return m_iError == LIBUSB_SUCCESS;
}

bool LuaDriverHandle_USB::ReleaseInterface( int interface )
{
	m_iError = libusb_release_interface( m_pHandle, interface );

	/* treat an unclaimed interface as success */
	return m_iError == LIBUSB_SUCCESS || m_iError == LIBUSB_ERROR_NOT_FOUND;
}

/* USB I/O functions */

int LuaDriverHandle_USB::ControlTransfer( uint8_t bmReqType, uint8_t bRequest,
	uint16_t wValue, uint16_t wIndex, uint8_t *data, uint16_t wLength,
	unsigned int timeout )
{
	return libusb_control_transfer( m_pHandle, bmReqType, bRequest, wValue, wIndex, data, wLength, timeout );
}

int LuaDriverHandle_USB::BulkTransfer( uint8_t endpoint, uint8_t *data,
	uint32_t length, int* transferred, unsigned int timeout )
{
	return libusb_bulk_transfer( m_pHandle, endpoint, data, length, transferred, timeout );
}

int LuaDriverHandle_USB::InterruptTransfer( uint8_t endpoint, uint8_t *data,
	uint32_t length, int* transferred, unsigned int timeout )
{
	return libusb_interrupt_transfer( m_pHandle, endpoint, data, length, (int*)transferred, timeout );
}

/* Taken from http://libusb.sourceforge.net/api-1.0/group__misc.html */
const char* LuaDriverHandle_USB::GetErrorStr( int error ) const
{
	switch( error )
	{
	case LIBUSB_SUCCESS:			return "Success";
	case LIBUSB_ERROR_IO:			return "Input/output error";
	case LIBUSB_ERROR_INVALID_PARAM:	return "Invalid parameter";
	case LIBUSB_ERROR_ACCESS:		return "Access denied";
	case LIBUSB_ERROR_NO_DEVICE:		return "No such device";
	case LIBUSB_ERROR_NOT_FOUND:		return "Entity not found";
	case LIBUSB_ERROR_BUSY:			return "Resource busy";
	case LIBUSB_ERROR_TIMEOUT:		return "Operation timed out";
	case LIBUSB_ERROR_OVERFLOW:		return "Overflow";
	case LIBUSB_ERROR_PIPE:			return "Pipe error";
	case LIBUSB_ERROR_INTERRUPTED:		return "System call interrupted";
	case LIBUSB_ERROR_NO_MEM:		return "Insufficient memory";
	case LIBUSB_ERROR_NOT_SUPPORTED:	return "Operation not supported";
	case LIBUSB_ERROR_OTHER:		return "Unspecified error";
	default:				return "(nil)";
	}
}

#define SET_FIELD(name) \
	{ lua_pushnumber(L, desc->name); lua_setfield(L, -2, #name); }

void LuaDriverHandle_USB::PushDeviceDescriptor( lua_State *L,
	const libusb_device_descriptor *desc, bool bPushSubtables )
{
	lua_newtable( L );
	SET_FIELD( bLength );
	SET_FIELD( bDescriptorType );
	SET_FIELD( bcdUSB );
	SET_FIELD( bDeviceClass );
	SET_FIELD( bDeviceProtocol );
	SET_FIELD( bMaxPacketSize0 );
	SET_FIELD( idVendor );
	SET_FIELD( idProduct );
	SET_FIELD( bcdDevice );
	SET_FIELD( iManufacturer );
	SET_FIELD( iProduct );
	SET_FIELD( iSerialNumber );
	SET_FIELD( bNumConfigurations );

	if( !bPushSubtables )
		return;

	/* Make a sub-table "Configurations" and push our descriptors to it */
	libusb_config_descriptor *config_desc = NULL;
	libusb_device *dev = libusb_get_device(m_pHandle);

	lua_newtable( L );

	for( unsigned n = 0; n < desc->bNumConfigurations; ++n )
	{
		int ret = libusb_get_config_descriptor( dev, n, &config_desc );

		if( ret != LIBUSB_SUCCESS )
		{
			LOG->Warn( "libusb_get_config_descriptor(%d): %s", n, GetErrorStr(ret) );
			continue;
		}

		PushConfigDescriptor( L, config_desc, true );
		lua_rawseti( L, -2, n+1 );

		libusb_free_config_descriptor( config_desc );
	}

	lua_setfield( L, -2, "Configurations" );
}

void LuaDriverHandle_USB::PushConfigDescriptor( lua_State *L,
	const libusb_config_descriptor *desc, bool bPushSubtables )
{

	lua_newtable( L );
	SET_FIELD( bDescriptorType );
	SET_FIELD( bNumInterfaces );
	SET_FIELD( bConfigurationValue );
	SET_FIELD( iConfiguration );
	SET_FIELD( bmAttributes );
	SET_FIELD( MaxPower );

	if( !bPushSubtables )
		return;

	/* Make a sub-table "Interfaces" and push our descriptors to it */
	lua_newtable( L );

	for( unsigned iface = 0; iface < desc->bNumInterfaces; ++iface )
	{
		const libusb_interface *pInterface = &desc->interface[iface];

		/* sub-table under "Interfaces" to hold iface's descriptors */
		lua_newtable( L );

		/* push this interface's altsettings to the subtable */
		for( int n = 0; n < pInterface->num_altsetting; ++n )
		{
			PushInterfaceDescriptor( L, &pInterface->altsetting[n], true );
			lua_rawseti( L, -2, n+1 );
		}

		/* assign the subtable to Interfaces[iface] */
		lua_rawseti( L, -2, iface+1 );
	}

	lua_setfield( L, -2, "Interfaces" );	/* assign sub-table */
}

void LuaDriverHandle_USB::PushInterfaceDescriptor( lua_State *L,
	const libusb_interface_descriptor *desc, bool bPushSubtables )
{
	lua_newtable( L );
	SET_FIELD( bLength );
	SET_FIELD( bDescriptorType );
	SET_FIELD( bInterfaceNumber );
	SET_FIELD( bAlternateSetting );
	SET_FIELD( bNumEndpoints );
	SET_FIELD( bInterfaceClass );
	SET_FIELD( bInterfaceSubClass );
	SET_FIELD( bInterfaceProtocol );
	SET_FIELD( iInterface );

	if( !bPushSubtables )
		return;

	/* make a sub-table "Endpoints" and push our descriptors to it */
	lua_newtable( L );

	for( unsigned n = 0; n < desc->bNumEndpoints; ++n )
	{
		PushEndpointDescriptor( L, &desc->endpoint[n] );
		lua_rawseti( L, -2, n+1 );
	}

	lua_setfield( L, -2, "Endpoints" );
}

void LuaDriverHandle_USB::PushEndpointDescriptor( lua_State *L, const libusb_endpoint_descriptor *desc )
{
	lua_newtable( L );
	SET_FIELD( bDescriptorType );
	SET_FIELD( bEndpointAddress );
	SET_FIELD( bmAttributes );
	SET_FIELD( wMaxPacketSize );
	SET_FIELD( bInterval );
}

#undef SET_FIELD

// lua start

#include "LuaBinding.h"

const unsigned CTL_TRANSFER_BUFFER_SIZE = 64;

class LunaLuaDriverHandle_USB : public Luna<LuaDriverHandle_USB>
{
public:
	static int Open( T *p, lua_State *L )
	{
		uint16_t iVID = IArg(1);
		uint16_t iPID = IArg(2);

		return p->Open( iVID, iPID );
	}

	static int GetDeviceDescriptor( T *p, lua_State *L )
	{
		if( p->m_pHandle == NULL )
		{
			lua_pushnil( L );
			return 1;
		}

		bool bPushSubtables = BArg(1);

		libusb_device *dev = libusb_get_device( p->m_pHandle );
		libusb_device_descriptor desc;
		libusb_get_device_descriptor( dev, &desc );
		p->PushDeviceDescriptor( L, &desc, bPushSubtables );
		return 1;
	}

	static int GetConfiguration( T *p, lua_State *L )
	{
		int config = p->GetConfiguration();

		// if config is 0, the device is unconfigured
		if( config )
			lua_pushnumber( L, config );
		else
			lua_pushnil( L );

		return 1;
	}

	static int SetConfiguration( T *p, lua_State *L )
	{
		lua_pushboolean( L, p->SetConfiguration(IArg(1)) );
		return 1;
	}

	static int ClaimInterface( T *p, lua_State *L )
	{
		lua_pushboolean( L, p->ClaimInterface(IArg(1)) );
		return 1;
	}

	static int ReleaseInterface( T *p, lua_State *L )
	{
		lua_pushboolean( L, p->ReleaseInterface(IArg(1)) );
		return 1;
	}

	/* phew... */
	static int ControlTransfer( T *p, lua_State *L )
	{
		uint8_t bmReqType = IArg(1);
		uint8_t bRequest = IArg(2);
		uint16_t wValue = IArg(3);
		uint16_t wIndex = IArg(4);

		uint8_t *data;
		uint16_t wLength;

		/* If we're getting data from the device, allocate a buffer
		 * to store the incoming data, so we can pass it to Lua. */
		bool bUsingBuffer = bmReqType & LIBUSB_ENDPOINT_OUT;

		if( bUsingBuffer )
		{
			data = new uint8_t[CTL_TRANSFER_BUFFER_SIZE];
			wLength = CTL_TRANSFER_BUFFER_SIZE;
		}
		else
		{
			if( lua_type(L, 5) != LUA_TSTRING )
				data = (uint8_t*)SArg(5); // force an error

			size_t len;
			data = (uint8_t*)lua_tolstring( L, 5, &len );
			wLength = uint16_t(len);
		}

		unsigned int timeout = IArg(6);

		/* Synchronous I/O can take a while; yield while this blocks. */
		LUA->YieldLua();
		int ret = p->ControlTransfer( bmReqType, bRequest,
			wValue, wIndex, data, wLength, timeout );
		LUA->UnyieldLua();

		/* if we encountered an error, set m_iError appropriately */
		if( ret < 0 )
			p->m_iError = ret;

		/* push the result (or error) for the first argument */
		lua_pushnumber( L, ret );

		/* if we received data, push it, else push nil */
		if( bUsingBuffer && ret > 0 )
			lua_pushlstring( L, (const char*)data, ret );
		else
			lua_pushnil( L );

		/* if this was our buffer, de-allocate it */
		if( bUsingBuffer )
			delete[] data;

		return 2;
	}

	LunaLuaDriverHandle_USB()
	{
		/* IsOpen, Close, Revision, Destroy are in the base class */
		ADD_METHOD( Open );
		ADD_METHOD( GetDeviceDescriptor );
		ADD_METHOD( GetConfiguration );
		ADD_METHOD( SetConfiguration );
		ADD_METHOD( ClaimInterface );
		ADD_METHOD( ReleaseInterface );
		ADD_METHOD( ControlTransfer );
	}
};

LUA_REGISTER_DERIVED_CLASS( LuaDriverHandle_USB, LuaDriverHandle );
