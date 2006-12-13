#include "global.h"
#include "RageSoundDriver_WDMKS.h"
#include "Foreach.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "archutils/Win32/ErrorStrings.h"

#define _INC_MMREG
#define _NTRTL_ /* Turn off default definition of DEFINE_GUIDEX */
#if !defined(DEFINE_WAVEFORMATEX_GUID)
#define DEFINE_WAVEFORMATEX_GUID(x) (USHORT)(x), 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
#endif

#include <windows.h>
#include <winioctl.h>
#include <ks.h>
#include <mmsystem.h>
#include <ksmedia.h>
#include <setupapi.h>

typedef KSDDKAPI DWORD WINAPI KSCREATEPIN(HANDLE, PKSPIN_CONNECT, ACCESS_MASK, PHANDLE);

#ifndef KSAUDIO_SPEAKER_5POINT1_SURROUND
#define KSAUDIO_SPEAKER_5POINT1_SURROUND \
	(SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT  | SPEAKER_SIDE_RIGHT)
#endif

#ifndef KSAUDIO_SPEAKER_7POINT1_SURROUND
#define KSAUDIO_SPEAKER_7POINT1_SURROUND \
	(SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | \
	 SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT)
#endif

struct WinWdmFilter;

struct WinWdmPin
{
	WinWdmPin( WinWdmFilter *pParentFilter, int iPinId )
	{
		m_hHandle = NULL;
		m_pParentFilter = pParentFilter;
		m_iPinId = iPinId;
	}

	~WinWdmPin()
	{
		Close();
	}

	bool Instantiate( const WAVEFORMATEX *pFormat, RString &sError );
	void Close();

	bool SetState( KSSTATE state, RString &sError );
	KSPIN_CONNECT *MakeFormat( const WAVEFORMATEX *pFormat ) const;
	bool IsFormatSupported( const WAVEFORMATEX *pFormat ) const;

	HANDLE				m_hHandle;
	WinWdmFilter			*m_pParentFilter;
	int				m_iPinId;
	vector<KSDATARANGE_AUDIO>	m_dataRangesItem;
};

enum DeviceSampleFormat
{
	DeviceSampleFormat_Float32,
	DeviceSampleFormat_Int16,
	DeviceSampleFormat_Int24,
	DeviceSampleFormat_Int32,
	NUM_DeviceSampleFormat,
	DeviceSampleFormat_Invalid
};

static int GetBytesPerSample( DeviceSampleFormat sf )
{
	switch( sf )
	{
	case DeviceSampleFormat_Float32: return 4;
	case DeviceSampleFormat_Int16: return 2;
	case DeviceSampleFormat_Int24: return 3;
	case DeviceSampleFormat_Int32: return 4;
	DEFAULT_FAIL(sf);
	}
}

/* The Filter structure
 * A filter has a number of pins and a "friendly name" */
struct WinWdmFilter
{
	/* Filter management functions */
	static WinWdmFilter *Create( const RString &sFilterName, const RString &sFriendlyName, RString &sError );

	WinWdmFilter()
	{
		m_hHandle = NULL;
		m_iUsageCount = 0;
	}

	~WinWdmFilter()
	{
		for( size_t i = 0; i < m_apPins.size(); ++i )
			delete m_apPins[i];
		if( m_hHandle )
			CloseHandle( m_hHandle );
	}

	WinWdmPin *CreatePin( unsigned long iPinId, RString &sError );
	WinWdmPin *InstantiateRenderPin(
			DeviceSampleFormat &PreferredOutputSampleFormat,
			int &iPreferredOutputChannels,
			int &iPreferredSampleRate,
			RString &sError );
	WinWdmPin *InstantiateRenderPin( const WAVEFORMATEX *wfex, RString &sError );
	bool Use( RString &sError );
	void Release();

	HANDLE			m_hHandle;
	vector<WinWdmPin *>	m_apPins;
	RString			m_sFilterName;
	RString			m_sFriendlyName;
	int			m_iUsageCount;
};

static RString GUIDToString( const GUID *pGuid )
{
	return ssprintf("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		pGuid->Data1, pGuid->Data2, pGuid->Data3,
		pGuid->Data4[0], pGuid->Data4[1], pGuid->Data4[2], pGuid->Data4[3],
		pGuid->Data4[4], pGuid->Data4[5], pGuid->Data4[6], pGuid->Data4[7] );
}

static HMODULE      DllKsUser = NULL;
static KSCREATEPIN *FunctionKsCreatePin = NULL;

/* Low level pin/filter access functions */
static bool WdmSyncIoctl(
	HANDLE hHandle, unsigned long ioctlNumber, void *pIn, unsigned long iInSize,
	void *pOut, unsigned long iOutSize, unsigned long *pBytesReturned, RString &sError )
{
	unsigned long iDummyBytesReturned;
	if( pBytesReturned == NULL )
		pBytesReturned = &iDummyBytesReturned;

	OVERLAPPED overlapped;
	memset( &overlapped, 0, sizeof(overlapped) );
	overlapped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if( !overlapped.hEvent )
	{
		sError = werr_ssprintf( GetLastError(), "CreateEvent" );
		return false;
	}
	overlapped.hEvent = (HANDLE)((DWORD_PTR)overlapped.hEvent | 0x1);

	int boolResult = DeviceIoControl( hHandle, ioctlNumber, pIn, iInSize, pOut, iOutSize, pBytesReturned, &overlapped );
	if( !boolResult )
	{
		unsigned long iError = GetLastError();
		if( iError == ERROR_IO_PENDING )
		{
			iError = WaitForSingleObject( overlapped.hEvent, INFINITE );
			if( iError != WAIT_OBJECT_0 )
			{
				ASSERT( iError == WAIT_FAILED );
				sError = werr_ssprintf( GetLastError(), "WaitForSingleObject" );
				CloseHandle( overlapped.hEvent );
				return false;
			}
		}
		else if(( iError == ERROR_INSUFFICIENT_BUFFER || iError == ERROR_MORE_DATA ) &&
			ioctlNumber == IOCTL_KS_PROPERTY && iOutSize == 0 )
		{
			boolResult = TRUE;
		}
		else
		{
			sError = werr_ssprintf( iError, "DeviceIoControl" );
			CloseHandle( overlapped.hEvent );
			return false;
		}
	}
	if( !boolResult )
		*pBytesReturned = 0;

	CloseHandle( overlapped.hEvent );
	return true;
}

static bool WdmGetPropertySimple( HANDLE hHandle, const GUID *pGuidPropertySet, unsigned long iProperty,
	void *pValue, unsigned long iValueSize, void *pInstance, unsigned long iInstanceSize, RString &sError )
{
	unsigned long iPropertySize = sizeof(KSPROPERTY) + iInstanceSize;
	vector<char> buf;
	buf.resize( iPropertySize );
	KSPROPERTY *ksProperty = (KSPROPERTY*) &buf[0];

	memset( ksProperty, 0, sizeof(*ksProperty) );
	ksProperty->Set = *pGuidPropertySet;
	ksProperty->Id = iProperty;
	ksProperty->Flags = KSPROPERTY_TYPE_GET;

	if( pInstance )
		memcpy( &ksProperty[1], pInstance, iInstanceSize );

	return WdmSyncIoctl( hHandle, IOCTL_KS_PROPERTY, ksProperty, iPropertySize, pValue, iValueSize, NULL, sError );
}

static bool WdmSetPropertySimple(
	HANDLE hHandle, const GUID *pGuidPropertySet, unsigned long iProperty,
	void *pValue, unsigned long iValueSize,
	void *instance, unsigned long iInstanceSize, RString &sError )
{
	vector<char> buf;
	unsigned long iPropertySize = sizeof(KSPROPERTY) + iInstanceSize;
	buf.resize( iPropertySize );
	KSPROPERTY *ksProperty = (KSPROPERTY *) &buf[0];

	memset( ksProperty, 0, sizeof(*ksProperty) );
	ksProperty->Set = *pGuidPropertySet;
	ksProperty->Id = iProperty;
	ksProperty->Flags = KSPROPERTY_TYPE_SET;

	if( instance )
		memcpy( ((char*)ksProperty + sizeof(KSPROPERTY)), instance, iInstanceSize );

	return WdmSyncIoctl( hHandle, IOCTL_KS_PROPERTY, ksProperty, iPropertySize, pValue, iValueSize, NULL, sError );
}

static bool WdmGetPinPropertySimple( HANDLE hHandle, unsigned long iPinId, const GUID *pGuidPropertySet, unsigned long iProperty,
	void *pValue, unsigned long iInstanceSize, RString &sError )
{
	KSP_PIN ksPProp;
	ksPProp.Property.Set = *pGuidPropertySet;
	ksPProp.Property.Id = iProperty;
	ksPProp.Property.Flags = KSPROPERTY_TYPE_GET;
	ksPProp.PinId = iPinId;
	ksPProp.Reserved = 0;

	return WdmSyncIoctl( hHandle, IOCTL_KS_PROPERTY, &ksPProp, sizeof(KSP_PIN), pValue, iInstanceSize, NULL, sError );
}

static bool WdmGetPinPropertyMulti(
	HANDLE hHandle,
	unsigned long iPinId,
	const GUID *pGuidPropertySet,
	unsigned long iProperty,
	KSMULTIPLE_ITEM **ksMultipleItem,
	RString &sError )
{
	KSP_PIN ksPProp;

	ksPProp.Property.Set = *pGuidPropertySet;
	ksPProp.Property.Id = iProperty;
	ksPProp.Property.Flags = KSPROPERTY_TYPE_GET;
	ksPProp.PinId = iPinId;
	ksPProp.Reserved = 0;

	unsigned long multipleItemSize = 0;
	if( !WdmSyncIoctl(hHandle, IOCTL_KS_PROPERTY, &ksPProp.Property, sizeof(KSP_PIN), NULL, 0, &multipleItemSize, sError) )
		return false;

	*ksMultipleItem = (KSMULTIPLE_ITEM*) malloc( multipleItemSize );
	ASSERT( *ksMultipleItem );

	if( !WdmSyncIoctl( hHandle, IOCTL_KS_PROPERTY, &ksPProp, sizeof(KSP_PIN), (void*)*ksMultipleItem, multipleItemSize, NULL, sError) )
	{
		free( ksMultipleItem );
		return false;
	}

	return true;
}

/*
 * Create a new pin object belonging to a filter
 * The pin object holds all the configuration information about the pin
 * before it is opened, and then the handle of the pin after is opened
 */
WinWdmPin *WinWdmFilter::CreatePin( unsigned long iPinId, RString &sError )
{
	{
		/* Get the COMMUNICATION property */
		KSPIN_COMMUNICATION communication;
		if( !WdmGetPinPropertySimple(m_hHandle, iPinId, &KSPROPSETID_Pin, KSPROPERTY_PIN_COMMUNICATION,
			&communication, sizeof(KSPIN_COMMUNICATION), sError) )
		{
			sError = "KSPROPERTY_PIN_COMMUNICATION: " + sError;
			return NULL;
		}

		if( communication != KSPIN_COMMUNICATION_SINK && communication != KSPIN_COMMUNICATION_BOTH )
		{
			sError = "Not an audio output device";
			return NULL;
		}
	}

	/* Get dataflow information */
	{
		KSPIN_DATAFLOW dataFlow;
		if( !WdmGetPinPropertySimple(m_hHandle, iPinId, &KSPROPSETID_Pin, KSPROPERTY_PIN_DATAFLOW,
			&dataFlow, sizeof(KSPIN_DATAFLOW), sError) )
		{
			sError = "KSPROPERTY_PIN_DATAFLOW: " + sError;
			return NULL;
		}

		if( dataFlow != KSPIN_DATAFLOW_IN )
		{
			sError = "Not KSPIN_DATAFLOW_IN";
			return NULL;
		}
	}

	/* Get the INTERFACE property list */
	{
		KSMULTIPLE_ITEM *pItem = NULL;
		if( !WdmGetPinPropertyMulti(m_hHandle, iPinId, &KSPROPSETID_Pin, KSPROPERTY_PIN_INTERFACES, &pItem, sError) )
		{
			sError = "KSPROPERTY_PIN_INTERFACES: " + sError;
			return NULL;
		}

		KSIDENTIFIER *identifier = (KSIDENTIFIER *) &pItem[1];

		/* Check that at least one interface is STANDARD_STREAMING */
		sError = "No standard streaming";
		for( unsigned i = 0; i < pItem->Count; i++ )
		{
			if( !memcmp( &identifier[i].Set, &KSINTERFACESETID_Standard, sizeof(GUID) ) &&
				identifier[i].Id == KSINTERFACE_STANDARD_STREAMING )
			{
				sError = "";
				break;
			}
		}

		free( pItem );

		if( sError != "" )
			return NULL;
	}

	/* Get the MEDIUM properties list */
	{
		KSMULTIPLE_ITEM *pItem = NULL;
		if( !WdmGetPinPropertyMulti( m_hHandle, iPinId, &KSPROPSETID_Pin, KSPROPERTY_PIN_MEDIUMS, &pItem, sError) )
		{
			sError = "KSPROPERTY_PIN_MEDIUMS: " + sError;
			return NULL;
		}

		const KSIDENTIFIER *identifier = (KSIDENTIFIER *) &pItem[1];

		/* Check that at least one medium is STANDARD_DEVIO */
		sError = "No STANDARD_DEVIO";
		for( unsigned i = 0; i < pItem->Count; i++ )
		{
			if( !memcmp( &identifier[i].Set, &KSMEDIUMSETID_Standard, sizeof(GUID) ) &&
				identifier[i].Id == KSMEDIUM_STANDARD_DEVIO )
			{
				sError = "";
				break;
			}
		}

		free( pItem );

		if( sError != "" )
			return NULL;
	}

	/* Allocate the new PIN object */
	WinWdmPin *pPin = new WinWdmPin( this, iPinId );

	/* Get DATARANGEs */
	KSMULTIPLE_ITEM *pDataRangesItem;
	if( !WdmGetPinPropertyMulti(m_hHandle, iPinId, &KSPROPSETID_Pin, KSPROPERTY_PIN_DATARANGES, &pDataRangesItem, sError) )
	{
		sError = "KSPROPERTY_PIN_DATARANGES: " + sError;
		goto error;
	}

	KSDATARANGE *pDataRanges = (KSDATARANGE*) (pDataRangesItem + 1);

	/* Find audio DATARANGEs */
	{
		KSDATARANGE *pDataRange = pDataRanges;
		for( unsigned i = 0; i < pDataRangesItem->Count; i++, pDataRange = (KSDATARANGE*)( ((char*)pDataRange) + pDataRange->FormatSize) )
		{
			if( memcmp(&pDataRange->MajorFormat, &KSDATAFORMAT_TYPE_AUDIO, sizeof(GUID)) &&
			    memcmp(&pDataRange->MajorFormat, &KSDATAFORMAT_TYPE_WILDCARD, sizeof(GUID)) )
				continue;

			if( memcmp(&pDataRange->SubFormat, &KSDATAFORMAT_SUBTYPE_PCM, sizeof(GUID) ) &&
			    memcmp(&pDataRange->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(GUID) ) &&
			    memcmp(&pDataRange->SubFormat, &KSDATAFORMAT_SUBTYPE_WILDCARD, sizeof(GUID) ) )
				continue;

			if( memcmp(&pDataRange->Specifier, &KSDATAFORMAT_SPECIFIER_WILDCARD, sizeof(GUID)) &&
			    memcmp(&pDataRange->Specifier, &KSDATAFORMAT_SPECIFIER_WAVEFORMATEX, sizeof(GUID) ))
				continue;

			const KSDATARANGE_AUDIO *pDataRangeAudio = (KSDATARANGE_AUDIO *) pDataRange;
			pPin->m_dataRangesItem.push_back( *pDataRangeAudio );
		}
	}
	free( pDataRangesItem );
	pDataRangesItem = NULL;

	if( pPin->m_dataRangesItem.size() == 0 )
	{
		sError = "Pin has no supported audio data ranges";
		goto error;
	}

	/* Success */
	sError = "";
	CHECKPOINT_M( "Pin created successfully" );
	return pPin;

error:
	/* Error cleanup */
	delete pPin;
	return NULL;
}

/* If the pin handle is open, close it */
void WinWdmPin::Close()
{
	if( m_hHandle == NULL )
		return;
	RString sError;
	SetState( KSSTATE_PAUSE, sError );
	SetState( KSSTATE_STOP, sError );
	CloseHandle( m_hHandle );
	m_hHandle = NULL;
	m_pParentFilter->Release();
}

/* Set the state of this (instantiated) pin */
bool WinWdmPin::SetState( KSSTATE state, RString &sError )
{
	ASSERT( m_hHandle != NULL );
	return WdmSetPropertySimple( m_hHandle, &KSPROPSETID_Connection, KSPROPERTY_CONNECTION_STATE,
		&state, sizeof(state), NULL, 0, sError );
}

bool WinWdmPin::Instantiate( const WAVEFORMATEX *pFormat, RString &sError )
{
	if( !IsFormatSupported(pFormat) )
	{
		sError = "format not supported";
		return false;
	}


	if( !m_pParentFilter->Use(sError) )
		return false;

	KSPIN_CONNECT *pPinConnect = MakeFormat( pFormat );
	DWORD iRet = FunctionKsCreatePin( m_pParentFilter->m_hHandle, pPinConnect, GENERIC_WRITE | GENERIC_READ, &m_hHandle );
	free( pPinConnect );
	if( iRet == ERROR_SUCCESS )
		return true;

	sError = werr_ssprintf( iRet, "FunctionKsCreatePin" );
	m_pParentFilter->Release();
	m_hHandle = NULL;
	return false;
}

KSPIN_CONNECT *WinWdmPin::MakeFormat( const WAVEFORMATEX *pFormat ) const
{
	ASSERT( pFormat != NULL );

	unsigned long iWfexSize = sizeof(WAVEFORMATEX) + pFormat->cbSize;
	unsigned long iDataFormatSize = sizeof(KSDATAFORMAT) + iWfexSize;
	unsigned long iSize = sizeof(KSPIN_CONNECT) + iDataFormatSize;

	KSPIN_CONNECT *pPinConnect = (KSPIN_CONNECT *) malloc( iSize );
	ASSERT( pPinConnect != NULL );

	memset( pPinConnect, 0, iSize );
	pPinConnect->PinId				= m_iPinId;
	pPinConnect->Interface.Set			= KSINTERFACESETID_Standard;
	pPinConnect->Interface.Id			= KSINTERFACE_STANDARD_STREAMING;
	pPinConnect->Interface.Flags			= 0;
	pPinConnect->Medium.Set			= KSMEDIUMSETID_Standard;
	pPinConnect->Medium.Id			= KSMEDIUM_TYPE_ANYINSTANCE;
	pPinConnect->Medium.Flags			= 0;
	pPinConnect->PinToHandle			= NULL;
	pPinConnect->Priority.PriorityClass		= KSPRIORITY_NORMAL;
	pPinConnect->Priority.PrioritySubClass	= 1;

	KSDATAFORMAT_WAVEFORMATEX *ksDataFormatWfx = (KSDATAFORMAT_WAVEFORMATEX *)(pPinConnect + 1);
	ksDataFormatWfx->DataFormat.Flags		= 0;
	ksDataFormatWfx->DataFormat.Reserved		= 0;
	ksDataFormatWfx->DataFormat.MajorFormat		= KSDATAFORMAT_TYPE_AUDIO;
	ksDataFormatWfx->DataFormat.SubFormat		= KSDATAFORMAT_SUBTYPE_PCM;
	ksDataFormatWfx->DataFormat.Specifier		= KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;
	ksDataFormatWfx->DataFormat.FormatSize = iDataFormatSize;

	memcpy( &ksDataFormatWfx->WaveFormatEx, pFormat, iWfexSize );
	ksDataFormatWfx->DataFormat.SampleSize = (unsigned short)(pFormat->nChannels * (pFormat->wBitsPerSample / 8));
	return pPinConnect;
}

bool WinWdmPin::IsFormatSupported( const WAVEFORMATEX *pFormat ) const
{
	GUID guid = { DEFINE_WAVEFORMATEX_GUID(pFormat->wFormatTag) };

	if( pFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE )
		guid = ((WAVEFORMATEXTENSIBLE*) pFormat)->SubFormat;

	for( size_t i = 0; i < m_dataRangesItem.size(); i++ )
	{
		const KSDATARANGE_AUDIO *pDataRangeAudio = &m_dataRangesItem[i];
		/* This is an audio or wildcard datarange... */
		if( memcmp(&pDataRangeAudio->DataRange.SubFormat, &KSDATAFORMAT_SUBTYPE_WILDCARD, sizeof(GUID)) &&
		    memcmp(&pDataRangeAudio->DataRange.SubFormat, &guid, sizeof(GUID)) )
			continue;

		if( pDataRangeAudio->MaximumChannels != (ULONG) -1 && pDataRangeAudio->MaximumChannels < pFormat->nChannels )
			continue;
		if( pFormat->wBitsPerSample < pDataRangeAudio->MinimumBitsPerSample || pFormat->wBitsPerSample > pDataRangeAudio->MaximumBitsPerSample )
			continue;
		if( pFormat->nSamplesPerSec < pDataRangeAudio->MinimumSampleFrequency || pFormat->nSamplesPerSec > pDataRangeAudio->MaximumSampleFrequency )
			continue;

		return true;
	}

	return false;
}

/* Create a new filter object. */
WinWdmFilter *WinWdmFilter::Create( const RString &sFilterName, const RString &sFriendlyName, RString &sError )
{
	/* Allocate the new filter object */
	WinWdmFilter *pFilter = new WinWdmFilter;

	pFilter->m_sFilterName = sFilterName;
	pFilter->m_sFriendlyName = sFriendlyName;

	/* Open the filter handle */
	if( !pFilter->Use(sError) )
		goto error;

	/* Get pin count */
	int iNumPins;
	if( !WdmGetPinPropertySimple(pFilter->m_hHandle, 0, &KSPROPSETID_Pin, KSPROPERTY_PIN_CTYPES, &iNumPins, sizeof(iNumPins), sError) )
		goto error;

	/* Create all the pins we can */
	for( int iPinId = 0; iPinId < iNumPins; iPinId++ )
	{
		/* Create the pin with this Id */
		WinWdmPin *pNewPin = pFilter->CreatePin( iPinId, sError );
		if( pNewPin != NULL )
			pFilter->m_apPins.push_back( pNewPin );
	}

	if( pFilter->m_apPins.empty() )
	{
		/* No valid pin was found on this filter so we destroy it */
		sError = "filter has no supported audio pins";
		goto error;
	}

	pFilter->Release();

	sError = "";
	return pFilter;

error:
	/* Error cleanup */
	delete pFilter;
	return NULL;
}

/*
 * Reopen the filter handle if necessary so it can be used
 */
bool WinWdmFilter::Use( RString &sError )
{
	if( m_hHandle == NULL )
	{
		/* Open the filter */
		m_hHandle = CreateFile( m_sFilterName, GENERIC_READ | GENERIC_WRITE, 0,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );

		if( m_hHandle == NULL )
		{
			sError = werr_ssprintf( GetLastError(), "CreateFile(%s)", m_sFilterName.c_str() );
			return false;
		}
	}

	++m_iUsageCount;
	return true;
}

/*
 * Release the filter handle if nobody is using it
 */
void WinWdmFilter::Release()
{
	ASSERT( m_iUsageCount > 0 );

	--m_iUsageCount;
	if( m_iUsageCount == 0 )
	{
		if( m_hHandle != NULL )
		{
			CloseHandle( m_hHandle );
			m_hHandle = NULL;
		}
	}
}

/*
 * Create a render (playback) Pin using the supplied format
 */
WinWdmPin *WinWdmFilter::InstantiateRenderPin( const WAVEFORMATEX *wfex, RString &sError )
{
	for( size_t i = 0; i < m_apPins.size(); ++i )
	{
		WinWdmPin *pPin = m_apPins[i];
		if( pPin->Instantiate(wfex, sError) )
		{
			sError = "";
			return pPin;
		}
	}

	sError = "No pin supports format";
	return NULL;
}

template<typename T, typename U>
void MoveToBeginning( vector<T> &v, const U &item )
{
	vector<T>::iterator it = find( v.begin(), v.end(), item );
	if( it == v.end() )
		return;
	vector<T>::iterator next = it;
	++next;
	copy_backward( v.begin(), it, next );
	*v.begin() = item;
}

static void FillWFEXT( WAVEFORMATEXTENSIBLE* pwfext, DeviceSampleFormat sampleFormat, int sampleRate, int channelCount)
{
	pwfext->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	pwfext->Format.nChannels = channelCount;
	pwfext->Format.nSamplesPerSec = sampleRate;
	switch( channelCount )
	{
	case 1: pwfext->dwChannelMask = KSAUDIO_SPEAKER_MONO; break;
	case 2: pwfext->dwChannelMask = KSAUDIO_SPEAKER_STEREO; break;
	case 4: pwfext->dwChannelMask = KSAUDIO_SPEAKER_QUAD; break;
	case 6: pwfext->dwChannelMask = KSAUDIO_SPEAKER_5POINT1; break; // or KSAUDIO_SPEAKER_5POINT1_SURROUND
	case 8: pwfext->dwChannelMask = KSAUDIO_SPEAKER_7POINT1_SURROUND; break; // or KSAUDIO_SPEAKER_7POINT1
	}

	switch(sampleFormat)
	{
	case DeviceSampleFormat_Float32: pwfext->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT; break;
	case DeviceSampleFormat_Int32: pwfext->SubFormat = KSDATAFORMAT_SUBTYPE_PCM; break;
	case DeviceSampleFormat_Int24: pwfext->SubFormat = KSDATAFORMAT_SUBTYPE_PCM; break;
	case DeviceSampleFormat_Int16: pwfext->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;break;
	}

	pwfext->Format.nBlockAlign = GetBytesPerSample( sampleFormat );
	pwfext->Format.wBitsPerSample = pwfext->Format.nBlockAlign * 8;
	pwfext->Format.nBlockAlign *= channelCount;
	pwfext->Samples.wValidBitsPerSample = pwfext->Format.wBitsPerSample;
	pwfext->Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
	pwfext->Format.nAvgBytesPerSec = pwfext->Format.nSamplesPerSec * pwfext->Format.nBlockAlign;
}

WinWdmPin *WinWdmFilter::InstantiateRenderPin(
		DeviceSampleFormat &PreferredOutputSampleFormat,
		int &iPreferredOutputChannels,
		int &iPreferredSampleRate,
		RString &sError )
{
	/*
	 * All Preferred settings are hints, and can be ignored if needed.
	 *
	 * Drivers can advertise DataRanges, and reject formats that fit it.  This is documented
	 * for devices that don't support mono sound; it may happen for other cases.
	 *
	 * Supported channel configurations are 8 (7.1), 6 (5.1), 4, 2 (stereo) and 1 (mono).  Prefer
	 * more channels, since some drivers won't send audio to rear speakers in stereo modes.  Sort
	 * the preferred channel count first.
	 */
	vector<int> aChannels;
	aChannels.push_back( 8 );
	aChannels.push_back( 6 );
	aChannels.push_back( 4 );
	aChannels.push_back( 2 );
	aChannels.push_back( 1 );
	MoveToBeginning( aChannels, iPreferredOutputChannels );

	/* Try all sample formats.  Try PreferredOutputSampleFormat first. */
	vector<DeviceSampleFormat> SampleFormats;
	SampleFormats.push_back( DeviceSampleFormat_Int16 );
	SampleFormats.push_back( DeviceSampleFormat_Int24 );
	SampleFormats.push_back( DeviceSampleFormat_Int32 );
	SampleFormats.push_back( DeviceSampleFormat_Float32 );
	MoveToBeginning( SampleFormats, PreferredOutputSampleFormat );

	/*
	 * Some hardware may advertise support for 44.1khz, but actually use poor resampling.
	 * Try to use 48khz before 44.1khz, unless iPreferredSampleRate specifically asks
	 * for 44.1khz.
	 *
	 * Try all samplerates listed in the device's DATARANGES.  Sort iSampleRate first,
	 * then 48k, then 44.1k, then higher sample rates first.
	 */
	vector<int> aSampleRates;
	{
		for( size_t j = 0; j < m_apPins.size(); ++j )
		{
			WinWdmPin *pPin = m_apPins[j];
			FOREACH_CONST( KSDATARANGE_AUDIO, pPin->m_dataRangesItem, range )
			{
				aSampleRates.push_back( range->MinimumSampleFrequency );
				aSampleRates.push_back( range->MaximumSampleFrequency );
			}
		}

		if( iPreferredSampleRate != 0 )
			aSampleRates.push_back( iPreferredSampleRate );
		aSampleRates.push_back( 48000 );
		aSampleRates.push_back( 44100 );

		sort( aSampleRates.begin(), aSampleRates.end() );
		aSampleRates.erase( unique(aSampleRates.begin(), aSampleRates.end()), aSampleRates.end() );
		reverse( aSampleRates.begin(), aSampleRates.end() );

		MoveToBeginning( aSampleRates, 44100 );
		MoveToBeginning( aSampleRates, 48000 );
		if( iPreferredSampleRate != 0 )
			MoveToBeginning( aSampleRates, iPreferredSampleRate );
	}

	/* Try WAVE_FORMAT_EXTENSIBLE, then WAVE_FORMAT_PCM. */
	vector<bool> aTryPCM;
	aTryPCM.push_back( false );
	aTryPCM.push_back( true );

	FOREACH( bool, aTryPCM, bTryPCM )
	{
		FOREACH( int, aSampleRates, iSampleRate )
		{
			FOREACH( int, aChannels, iChannels )
			{
				FOREACH( DeviceSampleFormat, SampleFormats, fmt )
				{
					PreferredOutputSampleFormat = *fmt;
					iPreferredOutputChannels = *iChannels;
					iPreferredSampleRate = *iSampleRate;

					WAVEFORMATEXTENSIBLE wfx;
					FillWFEXT( &wfx, PreferredOutputSampleFormat, iPreferredSampleRate, iPreferredOutputChannels );
					if( *bTryPCM )
					{
						/* Try WAVE_FORMAT_PCM instead of WAVE_FORMAT_EXTENSIBLE. */
						wfx.Format.wFormatTag = WAVE_FORMAT_PCM;
						wfx.Format.cbSize = 0;
						wfx.Samples.wValidBitsPerSample = 0;
						wfx.dwChannelMask = 0;
						wfx.SubFormat = GUID_NULL;
					}

					LOG->Trace( "KS: trying format: %i channels: %i samplerate: %i format: %04x",
						PreferredOutputSampleFormat, iPreferredOutputChannels,
						iPreferredSampleRate, wfx.Format.wFormatTag );
					WinWdmPin *pPlaybackPin = InstantiateRenderPin( (WAVEFORMATEX *) &wfx, sError );

					if( pPlaybackPin != NULL )
					{
						LOG->Trace( "KS: success" );
						return pPlaybackPin;
					}
				}
			}
		}
	}

	sError = "No compatible format found";
	return NULL;
}

static bool GetDevicePath( HANDLE hHandle, SP_DEVICE_INTERFACE_DATA *pInterfaceData, RString &sPath )
{
	unsigned char interfaceDetailsArray[sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + (MAX_PATH * sizeof(WCHAR))];
	const int sizeInterface = sizeof(interfaceDetailsArray);
	SP_DEVICE_INTERFACE_DETAIL_DATA *devInterfaceDetails = (SP_DEVICE_INTERFACE_DETAIL_DATA*) interfaceDetailsArray;
	devInterfaceDetails->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	SP_DEVINFO_DATA devInfoData;
	devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	devInfoData.Reserved = 0;

	if( !SetupDiGetDeviceInterfaceDetail(hHandle, pInterfaceData, devInterfaceDetails, sizeInterface, NULL, &devInfoData) )
		return false;
	sPath = devInterfaceDetails->DevicePath;
	return true;
}

/* Build a list of available filters. */
static bool BuildFilterList( vector<WinWdmFilter*> &aFilters, RString &sError )
{
	const GUID *pCategoryGuid = (GUID*) &KSCATEGORY_RENDER;

	/* Open a handle to search for devices (filters) */
	HDEVINFO hHandle = SetupDiGetClassDevs( pCategoryGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );
	if( hHandle == INVALID_HANDLE_VALUE )
	{
		sError = werr_ssprintf( GetLastError(), "SetupDiGetClassDevs" );
		return false;
	}

	CHECKPOINT_M( "Setup called" );

	/* Create filter objects for each interface */
	for( int device = 0;;device++ )
	{
		SP_DEVICE_INTERFACE_DATA interfaceData;
		interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		interfaceData.Reserved = 0;
		if( !SetupDiEnumDeviceInterfaces(hHandle, NULL, pCategoryGuid, device, &interfaceData) )
			break; /* No more devices */
		if( !interfaceData.Flags || (interfaceData.Flags & SPINT_REMOVED) )
			continue;

		RString sDevicePath;
		if( !GetDevicePath(hHandle, &interfaceData, sDevicePath) )
			continue;

		/* Try to get the friendly name for this interface */
		char szFriendlyName[MAX_PATH] = "(error)";
		DWORD sizeFriendlyName = sizeof(szFriendlyName);
		HKEY hKey = SetupDiOpenDeviceInterfaceRegKey( hHandle, &interfaceData, 0, KEY_QUERY_VALUE );
		if( hKey != INVALID_HANDLE_VALUE )
		{
			DWORD type;
			if( RegQueryValueEx(hKey, "FriendlyName", 0, &type, (BYTE*) szFriendlyName, &sizeFriendlyName) != ERROR_SUCCESS )
				strcpy( szFriendlyName, "(error)" );
			RegCloseKey( hKey );
		}

		WinWdmFilter *pNewFilter = WinWdmFilter::Create( sDevicePath, szFriendlyName, sError );
		if( pNewFilter == NULL )
		{
			LOG->Trace( "Filter \"%s\" not created: %s", szFriendlyName, sError.c_str() );
			continue;
		}

		aFilters.push_back( pNewFilter );
	}

	if( hHandle != NULL )
		SetupDiDestroyDeviceInfoList( hHandle );

	return true;
}

static bool PaWinWdm_Initialize( RString &sError )
{
	if( DllKsUser == NULL )
	{
		DllKsUser = LoadLibrary( "ksuser.dll" );
		if( DllKsUser == NULL )
		{
			sError = werr_ssprintf( GetLastError(), "LoadLibrary(ksuser.dll)" );
			return false;
		}
	}

	FunctionKsCreatePin = (KSCREATEPIN*) GetProcAddress( DllKsUser, "KsCreatePin" );
	if( FunctionKsCreatePin == NULL )
	{
		sError = "no KsCreatePin in ksuser.dll";
		FreeLibrary( DllKsUser );
		DllKsUser = NULL;
		return false;
	}

	return true;
}

#define MAX_CHUNKS 4
struct WinWdmStream
{
	WinWdmStream()
	{
		memset( this, 0, sizeof(*this) );
		for( int i = 0; i < MAX_CHUNKS; ++i )
			m_Signal[i].hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
		m_pPlaybackPin = NULL;
	}

	~WinWdmStream()
	{
		CloseHandle( m_Signal[0].hEvent );
		CloseHandle( m_Signal[1].hEvent );
		Close();
	}

	bool Open( WinWdmFilter *pFilter,
			int iWriteAheadFrames,
			DeviceSampleFormat PreferredOutputSampleFormat,
			int iPreferredOutputChannels,
			int iSampleRate,
			RString &sError );
	void Close()
	{
		if( m_pPlaybackPin )
			m_pPlaybackPin->Close();
		m_pPlaybackPin = NULL;
		for( int i = 0; i < 2; ++i )
		{
			VirtualFree( m_Packets[i].Data, 0, MEM_RELEASE );
			m_Packets[i].Data = NULL;
		}
	}

	bool SubmitPacket( int iPacket, RString &sError );

	WinWdmPin			*m_pPlaybackPin;
	KSSTREAM_HEADER			m_Packets[MAX_CHUNKS];
	OVERLAPPED			m_Signal[MAX_CHUNKS];

	int				m_iSampleRate;
	int				m_iWriteAheadChunks;
	int				m_iFramesPerChunk;
	int				m_iBytesPerOutputSample;
	int				m_iDeviceOutputChannels;
	DeviceSampleFormat		m_DeviceSampleFormat;
};

bool WinWdmStream::Open( WinWdmFilter *pFilter,
			int iWriteAheadFrames,
			DeviceSampleFormat PreferredOutputSampleFormat,
			int iPreferredOutputChannels,
			int iPreferredSampleRate,
			RString &sError )
{
	/* Instantiate the output pin. */
	m_pPlaybackPin = pFilter->InstantiateRenderPin(
				PreferredOutputSampleFormat,
				iPreferredOutputChannels,
				iPreferredSampleRate,
				sError );

	if( m_pPlaybackPin == NULL )
		goto error;

	m_DeviceSampleFormat = PreferredOutputSampleFormat;
	m_iDeviceOutputChannels = iPreferredOutputChannels;
	m_iSampleRate = iPreferredSampleRate;
	m_iBytesPerOutputSample = GetBytesPerSample( m_DeviceSampleFormat );

	int iFrameSize = 1;
	{
		KSALLOCATOR_FRAMING ksaf;
		KSALLOCATOR_FRAMING_EX ksafex;
		if( WdmGetPropertySimple(m_pPlaybackPin->m_hHandle, &KSPROPSETID_Connection, KSPROPERTY_CONNECTION_ALLOCATORFRAMING,
			&ksaf, sizeof(ksaf), NULL, 0, sError) )
		{
			iFrameSize = ksaf.FrameSize;
		}
		else if( WdmGetPropertySimple(m_pPlaybackPin->m_hHandle, &KSPROPSETID_Connection, KSPROPERTY_CONNECTION_ALLOCATORFRAMING_EX,
				&ksafex, sizeof(ksafex), NULL, 0, sError) )
		{
			iFrameSize = ksafex.FramingItem[0].FramingRange.Range.MinFrameSize;
		}
	}

	iFrameSize /= m_iBytesPerOutputSample * m_iDeviceOutputChannels;

	m_iWriteAheadChunks = 2;

	/* If a writeahead was specified, use it. */
	m_iFramesPerChunk = iWriteAheadFrames / m_iWriteAheadChunks;
	if( m_iFramesPerChunk == 0 )
		m_iFramesPerChunk = iFrameSize;

	LOG->Info( "KS: chunk size: %i; allocator framing: %i (%ims)", m_iFramesPerChunk, iFrameSize, (iFrameSize * 1000) / m_iSampleRate );
	LOG->Info( "KS: %i hz", m_iSampleRate );

	/* Set up chunks. */
	for( int i = 0; i < MAX_CHUNKS; ++i )
	{
		// _aligned_malloc( size, 64 )?
		KSSTREAM_HEADER *p = &m_Packets[i];

		/* Avoid any FileAlignment problems by using VirtualAlloc, which is always page aligned. */
		p->Data = (char *) VirtualAlloc( NULL, m_iFramesPerChunk*m_iBytesPerOutputSample*m_iDeviceOutputChannels, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE );
		ASSERT( p->Data );
		p->FrameExtent = m_iFramesPerChunk*m_iBytesPerOutputSample*m_iDeviceOutputChannels;
		p->DataUsed = m_iFramesPerChunk*m_iBytesPerOutputSample*m_iDeviceOutputChannels;
		p->Size = sizeof(*p);
		p->PresentationTime.Numerator = 1;
		p->PresentationTime.Denominator = 1;
	}

	return true;

error:
	Close();
	return false;
}

bool WinWdmStream::SubmitPacket( int iPacket, RString &sError )
{
	KSSTREAM_HEADER *p = &m_Packets[iPacket];
	int iRet = DeviceIoControl( m_pPlaybackPin->m_hHandle, IOCTL_KS_WRITE_STREAM, NULL, 0,
		p, p->Size, NULL, &m_Signal[iPacket] );
	ASSERT_M( iRet == 0, "DeviceIoControl" );

	DWORD iError = GetLastError();
	if( iError == ERROR_IO_PENDING )
		return true;

	sError = werr_ssprintf(iError, "DeviceIoControl");
	return false;
}


#include <windows.h>
namespace
{
	void MapChannels( const int16_t *pIn, int16_t *pOut, int iInChannels, int iOutChannels, int iFrames, const int *pChannelMap )
	{
		for( int i = 0; i < iFrames; ++i )
		{
			for( int j = 0; j < iOutChannels; ++j )
			{
				if( pChannelMap[j] == -1 )
					pOut[j] = 0;
				else if( pChannelMap[j] == -2 )
				{
					int iSum = 0;
					for( int k = 0; k < iInChannels; ++k )
						iSum += pIn[k];
					iSum /= iInChannels;
					pOut[j] = iSum;
				}
				else
					pOut[j] = pIn[ pChannelMap[j] ];
			}
			pOut += iOutChannels;
			pIn += iInChannels;
		}
	}

	void MapChannels( const int16_t *pIn, int16_t *pOut, int iInChannels, int iOutChannels, int iFrames )
	{
		static const int i1ChannelMap[] = { -2 };
		static const int i4ChannelMap[] = { 0, 1, 0, 1 };
		static const int i5_1ChannelMap[] = { 0, 1, -1, -2, 0, 1 };
		static const int i7_1ChannelMap[] = { 0, 1, -1, -2, 0, 1, 0, 1 };
		const int *pChannelMap;
		switch( iOutChannels )
		{
		case 1: pChannelMap = i1ChannelMap; break; // KSAUDIO_SPEAKER_MONO
		case 4: pChannelMap = i4ChannelMap; break; // KSAUDIO_SPEAKER_QUAD
		case 6: pChannelMap = i5_1ChannelMap; break; // KSAUDIO_SPEAKER_5POINT1_SURROUND
		case 8: pChannelMap = i7_1ChannelMap; break; // KSAUDIO_SPEAKER_7POINT1_SURROUND
		default: FAIL_M( ssprintf("%i", iOutChannels) );
		}
		MapChannels( pIn, pOut, iInChannels, iOutChannels, iFrames, pChannelMap );
	}

	void MapSampleFormatFromInt16( const int16_t *pIn, void *pOut, int iSamples, DeviceSampleFormat FromFormat )
	{
		switch( FromFormat )
		{
		case DeviceSampleFormat_Float32: // untested
		{
			float *pOutBuf = (float *) pOut;
			for( int i = 0; i < iSamples; ++i )
				pOutBuf[i] = SCALE( pIn[i], -32768, +32767, -1.0f, +1.0f ); // [-32768, 32767] -> [-1,+1]
			break;
		}
		case DeviceSampleFormat_Int24:
		{
			const unsigned char *pInBytes = (unsigned char *) pIn;
			unsigned char *pOutBuf = (unsigned char *) pOut;
			for( int i = 0; i < iSamples; ++i )
			{
				*pOutBuf++ = 0;
				*pOutBuf++ = *pInBytes++;
				*pOutBuf++ = *pInBytes++;
			}
			break;
		}
		case DeviceSampleFormat_Int32:
		{
			int16_t *pOutBuf = (int16_t *) pOut;
			for( int i = 0; i < iSamples; ++i )
			{
				*pOutBuf++ = 0;
				*pOutBuf++ = *pIn++;
			}
			break;
		}
		}
	}
}

void RageSoundDriver_WDMKS::Read( void *pData, int iFrames, int iLastCursorPos, int iCurrentFrame )
{
	/* If we need conversion, read into a temporary buffer.  Otherwise, read directly
	 * into the target buffer. */
	int iChannels = 2;
	if( m_pStream->m_iDeviceOutputChannels == iChannels &&
		m_pStream->m_DeviceSampleFormat == DeviceSampleFormat_Int16 )
	{
		int16_t *pBuf = (int16_t *) pData;
		this->Mix( pBuf, iFrames, iLastCursorPos, iCurrentFrame );
		return;
	}

	int16_t *pBuf = (int16_t *) alloca( iFrames * iChannels * sizeof(int16_t) );
	this->Mix( (int16_t *) pBuf, iFrames, iLastCursorPos, iCurrentFrame );

	/* If the device has other than 2 channels, convert. */
	if( m_pStream->m_iDeviceOutputChannels != iChannels )
	{
		int16_t *pTempBuf = (int16_t *) alloca( iFrames * m_pStream->m_iBytesPerOutputSample * m_pStream->m_iDeviceOutputChannels );
		MapChannels( (int16_t *) pBuf, pTempBuf, iChannels, m_pStream->m_iDeviceOutputChannels, iFrames );
		pBuf = pTempBuf;
	}

	/* If the device format isn't int16_t, convert. */
	if( m_pStream->m_DeviceSampleFormat != DeviceSampleFormat_Int16 )
	{
		int iSamples = iFrames * m_pStream->m_iDeviceOutputChannels;
		void *pTempBuf = alloca( iSamples * m_pStream->m_iBytesPerOutputSample );
		MapSampleFormatFromInt16( (int16_t *) pBuf, pTempBuf, iSamples, m_pStream->m_DeviceSampleFormat );
		pBuf = (int16_t *) pTempBuf;
	}

	memcpy( pData, pBuf, iFrames * m_pStream->m_iDeviceOutputChannels * m_pStream->m_iBytesPerOutputSample );
}

bool RageSoundDriver_WDMKS::Fill( int iPacket, RString &sError )
{
	uint64_t iCurrentFrame = GetPosition();
//	if( iCurrentFrame == m_iLastCursorPos )
//		LOG->Trace( "underrun" );

	Read( m_pStream->m_Packets[iPacket].Data, m_pStream->m_iFramesPerChunk, m_iLastCursorPos, iCurrentFrame );

	/* Increment m_iLastCursorPos. */
	m_iLastCursorPos += m_pStream->m_iFramesPerChunk;

	/* Submit the buffer. */
	return m_pStream->SubmitPacket( iPacket, sError );
}

void RageSoundDriver_WDMKS::MixerThread()
{
	/* I don't trust this driver with THREAD_PRIORITY_TIME_CRITICAL just yet. */
	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) )
//	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) )
		LOG->Warn( werr_ssprintf(GetLastError(), "Failed to set sound thread priority") );

	/* Enable priority boosting. */
	SetThreadPriorityBoost( GetCurrentThread(), FALSE );
	
	ASSERT( m_pStream->m_pPlaybackPin );

	/* Some drivers (stock USB audio in XP) misbehave if we go from KSSTATE_STOP to
	 * KSSTATE_RUN.  Always transition through KSSTATE_PAUSE. */
	RString sError;
	if( !m_pStream->m_pPlaybackPin->SetState(KSSTATE_PAUSE, sError) ||
	    !m_pStream->m_pPlaybackPin->SetState(KSSTATE_RUN, sError) )
	    FAIL_M( sError );

	/* Submit initial buffers. */
	for( int i = 0; i < m_pStream->m_iWriteAheadChunks; ++i )
		Fill( i, sError );

	int iNextBufferToSend = m_pStream->m_iWriteAheadChunks;
	iNextBufferToSend %= MAX_CHUNKS;

	int iWaitFor = 0;

	while( !m_bShutdown )
	{
		/* Wait for next output buffer to finish. */
		HANDLE aEventHandles[2] = { m_hSignal, m_pStream->m_Signal[iWaitFor].hEvent };
		unsigned long iWait = WaitForMultipleObjects( 2, aEventHandles, FALSE, 1000 );

		if( iWait == WAIT_FAILED )
		{
			LOG->Warn( werr_ssprintf(GetLastError(), "WaitForMultipleObjects") );
			break;
		}
		if( iWait == WAIT_TIMEOUT )
			continue;

		if( iWait == WAIT_OBJECT_0 )
		{
			/* Abort event */
			ASSERT( m_bShutdown ); /* Should have been set */
			continue;
		}
		++iWaitFor;
		iWaitFor %= MAX_CHUNKS;

		if( !Fill(iNextBufferToSend, sError) )
		{
			LOG->Warn( "Fill(): %s", sError.c_str() );
			break;
		}

		++iNextBufferToSend;
		iNextBufferToSend %= MAX_CHUNKS;
	}

	/* Finished, either normally or aborted */
	m_pStream->m_pPlaybackPin->SetState( KSSTATE_PAUSE, sError );
	m_pStream->m_pPlaybackPin->SetState( KSSTATE_STOP, sError );
}


REGISTER_SOUND_DRIVER_CLASS( WDMKS );

int RageSoundDriver_WDMKS::MixerThread_start( void *p )
{
	((RageSoundDriver_WDMKS *) p)->MixerThread();
	return 0;
}

void RageSoundDriver_WDMKS::SetupDecodingThread()
{
	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) )
		LOG->Warn( werr_ssprintf(GetLastError(), "Failed to set sound thread priority") );
}

int64_t RageSoundDriver_WDMKS::GetPosition() const
{
	KSAUDIO_POSITION pos;

	RString sError;
	WdmGetPropertySimple( m_pStream->m_pPlaybackPin->m_hHandle, &KSPROPSETID_Audio, KSPROPERTY_AUDIO_POSITION,
		&pos, sizeof(pos), NULL, 0, sError );
	ASSERT( sError == "" );

	pos.PlayOffset /= m_pStream->m_iBytesPerOutputSample * m_pStream->m_iDeviceOutputChannels;
	return pos.PlayOffset;
}

RageSoundDriver_WDMKS::RageSoundDriver_WDMKS()
{
	m_pStream = NULL;
	m_pFilter = NULL;
	m_bShutdown = false;
	m_iLastCursorPos = 0;
	m_hSignal = CreateEvent( NULL, FALSE, FALSE, NULL ); /* abort event */
}

RString RageSoundDriver_WDMKS::Init()
{
	RString sError;
	if( !PaWinWdm_Initialize(sError) )
		return sError;

	vector<WinWdmFilter *> apFilters;
	if( !BuildFilterList(apFilters, sError) )
		return "Error building filter list: " + sError;
	if( apFilters.empty() )
		return "No supported audio devices found";

	for( size_t i = 0; i < apFilters.size(); ++i )
	{
		const WinWdmFilter *pFilter = apFilters[i];
		LOG->Trace( "Device #%i: %s", i, pFilter->m_sFriendlyName.c_str() );
		for( size_t j = 0; j < pFilter->m_apPins.size(); ++j )
		{
			WinWdmPin *pPin = pFilter->m_apPins[j];
			LOG->Trace( "  Pin %i", j );
			FOREACH_CONST( KSDATARANGE_AUDIO, pPin->m_dataRangesItem, range )
			{
				RString sSubFormat;
				if( !memcmp(&range->DataRange.SubFormat, &KSDATAFORMAT_SUBTYPE_WILDCARD, sizeof(GUID)) )
					sSubFormat = "WILDCARD";
				else if( !memcmp(&range->DataRange.SubFormat, &KSDATAFORMAT_SUBTYPE_PCM, sizeof(GUID)) )
					sSubFormat = "PCM";
				else if( !memcmp(&range->DataRange.SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(GUID)) )
					sSubFormat = "FLOAT";

				LOG->Trace( "     Range: %i channels, sample %i-%i, %i-%ihz (%s)", 
					range->MaximumChannels,
					range->MinimumBitsPerSample,
					range->MaximumBitsPerSample,
					range->MinimumSampleFrequency,
					range->MaximumSampleFrequency,
					sSubFormat.c_str()
				);
			}
		}
	}

	m_pFilter = apFilters[0];
	for( size_t i=0; i < apFilters.size(); ++i )
	{
		if( apFilters[i] != m_pFilter )
			delete apFilters[i];
	}
	apFilters.clear();

	m_pStream = new WinWdmStream;
	if( !m_pStream->Open( m_pFilter,
		PREFSMAN->m_iSoundWriteAhead,
		DeviceSampleFormat_Int16,
		0, // don't care
		PREFSMAN->m_iSoundPreferredSampleRate,
		sError ) )
	{
		return sError;
	}

	SetDecodeBufferSize( 2048 );
	StartDecodeThread();

	MixingThread.SetName( "Mixer thread" );
	MixingThread.Create( MixerThread_start, this );

	return RString();
}

RageSoundDriver_WDMKS::~RageSoundDriver_WDMKS()
{
	/* Signal the mixing thread to quit. */
	if( MixingThread.IsCreated() )
	{
		m_bShutdown = true;
		SetEvent( m_hSignal ); /* Signal immediately */
		LOG->Trace( "Shutting down mixer thread ..." );
		MixingThread.Wait();
		LOG->Trace( "Mixer thread shut down." );

		delete m_pStream;
	}

	delete m_pFilter;
	CloseHandle( m_hSignal );
}

int RageSoundDriver_WDMKS::GetSampleRate( int iRate ) const
{
	return m_pStream->m_iSampleRate;
}

float RageSoundDriver_WDMKS::GetPlayLatency() const
{
	/* If we have a 1000-byte buffer, and we fill 500 bytes at a time, we
	 * almost always have between 500 and 1000 bytes filled; on average, 750. */
	return (m_pStream->m_iFramesPerChunk + m_pStream->m_iFramesPerChunk/2) * (1.0f / m_pStream->m_iSampleRate);
}

/*
 * Based on the PortAudio Windows WDM-KS interface.
 *
 * Copyright (c) 1999-2004 Andrew Baldwin, Ross Bencina, Phil Burk
 * Copyright (c) 2002-2006 Glenn Maynard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however, 
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also 
 * requested that these non-binding requests be included along with the 
 * license above.
 */
