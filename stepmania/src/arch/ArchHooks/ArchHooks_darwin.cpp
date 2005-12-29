#include "global.h"
#include "ArchHooks_darwin.h"
#include "RageLog.h"
#include "RageThreads.h"
#include "RageTimer.h"
#include "RageUtil.h"
#include "archutils/Darwin/Crash.h"
#include "archutils/Unix/CrashHandler.h"
#include "archutils/Unix/SignalHandler.h"
#include "StepMania.h"
#include "GameLoop.h"
#include "ProductInfo.h"
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/thread_act.h>
#include <mach/mach.h>
#include <mach/host_info.h>
#include <sys/types.h>
#include <sys/time.h>

#if 0 // Time critical stuff
static thread_time_constraint_policy g_oldttcpolicy;
static float g_fStartedTimeCritAt;
#endif

static bool IsFatalSignal( int signal )
{
	switch( signal )
	{
	case SIGINT:
	case SIGTERM:
	case SIGHUP:
		return false;
	default:
		return true;
	}
}

static void DoCleanShutdown( int signal, siginfo_t *si, const ucontext_t *uc )
{
	if( IsFatalSignal(signal) )
		return;

	/* ^C. */
	ArchHooks::SetUserQuit();
}

static void DoCrashSignalHandler( int signal, siginfo_t *si, const ucontext_t *uc )
{
	/* Don't dump a debug file if the user just hit ^C. */
	if( !IsFatalSignal(signal) )
		return;

	CrashSignalHandler( signal, si, uc );
	/* not reached */
}

ArchHooks_darwin::ArchHooks_darwin()
{
    CrashHandlerHandleArgs( g_argc, g_argv );

    /* First, handle non-fatal termination signals. */
    SignalHandler::OnClose( DoCleanShutdown );

    SignalHandler::OnClose( DoCrashSignalHandler );
    //TimeCritMutex = new RageMutex("TimeCritMutex");
	
	// CF*Copy* functions' return values need to be released, CF*Get* functions' do not.
	CFStringRef key = CFSTR( "ApplicationBundlePath" );
	CFStringRef appID = CFSTR( "com." PRODUCT_NAME );
	CFStringRef version = CFSTR( PRODUCT_VER );
	
	CFURLRef path = CFBundleCopyBundleURL( CFBundleGetMainBundle() );
	CFPropertyListRef value = CFURLCopyPath( path );
	CFPropertyListRef old = CFPreferencesCopyAppValue( key, appID );
	CFMutableDictionaryRef newDict = NULL;
	bool changed = false;
	
	if( old && CFGetTypeID(old) != CFDictionaryGetTypeID() )
	{
		CFRelease( old );
		old = NULL;
	}
	
	if( !old )
	{
		newDict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
											 &kCFTypeDictionaryValueCallBacks );
		CFDictionaryAddValue( newDict, version, value );
		changed = true;
	}
	else
	{
		CFTypeRef oldValue;
		CFDictionaryRef dict = CFDictionaryRef( old );
		
		if( !CFDictionaryGetValueIfPresent(dict, version, &oldValue) || !CFEqual(oldValue, value) )
		{
			// The value is either not present or it is but it is different
			newDict = CFDictionaryCreateMutableCopy( kCFAllocatorDefault, 0, dict );
			CFDictionaryAddValue( newDict, version, value );
			changed = true;
		}
		CFRelease( old );
	}
	
	if( changed )
	{
		CFPreferencesSetAppValue( key, newDict, appID );
		if( !CFPreferencesAppSynchronize(appID) )
			LOG->Warn( "Failed to record the run path." );
		CFRelease( newDict );
	}
	CFRelease( value );
	CFRelease( path );
}

ArchHooks_darwin::~ArchHooks_darwin()
{
	//delete TimeCritMutex;
}

void ArchHooks_darwin::DumpDebugInfo()
{
    CString systemVersion;
    OSErr err;
    long code;

    /* Get system version */
    err = Gestalt( gestaltSystemVersion, &code );
    if( err == noErr )
    {
		int major = ( (code >> 12) & 0xF ) * 10 + (code >> 8) & 0xF;
		int minor = (code >> 4) & 0xF;
		int revision = code & 0xF;
		
        systemVersion = ssprintf( "Mac OS X %d.%d.%d", major, minor, revision );
    }
    else
        systemVersion = "Unknown system version";

    /* Get memory */
	long ram;
    long vRam;
	
    err = Gestalt( gestaltLogicalRAMSize, &vRam );
    if (err != noErr)
        vRam = 0;
    err = Gestalt( gestaltPhysicalRAMSize, &ram );
    if( err == noErr )
    {
        vRam -= ram;
        if( vRam < 0 )
            vRam = 0;
        ram >>= 20;
        vRam >>= 20;
    }
    else
    {
        ram = 0;
        vRam = 0;
    }
	
	/* Get processor information */
    CString processor;
    int numProcessors;	
	struct host_basic_info info;
	mach_port_t host = mach_host_self();
	mach_msg_type_number_t size = HOST_BASIC_INFO_COUNT;
	kern_return_t ret = host_info( host, HOST_BASIC_INFO, (host_info_t)&info, &size );
	
	if( ret )
	{
		numProcessors = -1;
		LOG->Warn("Couldn't get host info.");
	}
	else
	{
		char *cpu_type, *cpu_subtype;
		
		numProcessors = info.avail_cpus;
		slot_name( info.cpu_type, info.cpu_subtype, &cpu_type, &cpu_subtype );
		processor = cpu_subtype;
	}

	/* Get processor speed */
	err = Gestalt( gestaltProcClkSpeed, &code );
    if( err != noErr )
        code = 0;
	
	float speed;
	char power;
	
	if( code >= 1000000000 )
	{
		speed = code / 1000000000.0f;
		power = 'G';
	}
	else
	{
		speed = code / 1000000.0f;
		power = 'M';
	}
    
    /* Send all of the information to the log */
    LOG->Info( "Processor: %s (%d)", processor.c_str(), numProcessors );
	LOG->Info( "Clock speed %.2f %cHz", speed, power );
    LOG->Info( "%s", systemVersion.c_str());
    LOG->Info( "Memory: %ld MB total, %ld MB swap", ram, vRam );
}

// In archutils/darwin/PreferredLanguage.m
extern "C"
{
	extern char *GetPreferredLanguage();
}

RString ArchHooks::GetPreferredLanguage()
{
	char *lang = ::GetPreferredLanguage();
	CString ret = lang;
	
	free(lang);
	return ret.ToUpper();
}

void ArchHooks_darwin::EnterTimeCriticalSection()
{
#if 0
	TimeCritMutex->Lock();

	int mib[] = { CTL_HW, HW_BUS_FREQ };
	int miblen = ARRAYSIZE( mib );
	int bus_speed;
	size_t len = sizeof (bus_speed);
	if( sysctl( mib, miblen, &bus_speed, &len, NULL, 0 ) == -1 )
	{
		LOG->Warn( "sysctl(HW_BUS_FREQ): %s", strerror(errno) );
		return;
	}

	mach_msg_type_number_t cnt = THREAD_TIME_CONSTRAINT_POLICY_COUNT;
	boolean_t bDefaults = false;
	thread_policy_get( mach_thread_self(), THREAD_TIME_CONSTRAINT_POLICY, (int*)&g_oldttcpolicy, &cnt, &bDefaults );

	/* We want to monopolize the CPU for a very short period of time.  This means that the
	 * period doesn't really matter, and we don't want to be preempted.  Set the period
	 * very high (~1 second), so that if we ever lose the CPU when we shouldn't, we can
	 * detect it and log it in ExitTimeCriticalSection(). */
	thread_time_constraint_policy ttcpolicy;
	ttcpolicy.period = bus_speed;
	ttcpolicy.computation = ttcpolicy.constraint = bus_speed/60;
	ttcpolicy.preemptible = 0;
	thread_policy_set( mach_thread_self(), THREAD_TIME_CONSTRAINT_POLICY,
		(int*)&ttcpolicy, THREAD_TIME_CONSTRAINT_POLICY_COUNT );

	g_fStartedTimeCritAt = RageTimer::GetTimeSinceStart();
#endif
}

void ArchHooks_darwin::ExitTimeCriticalSection()
{
#if 0
	thread_policy_set( mach_thread_self(), THREAD_TIME_CONSTRAINT_POLICY,
		(int*) &g_oldttcpolicy, THREAD_TIME_CONSTRAINT_POLICY_COUNT );
	TimeCritMutex->Unlock();

	float fTimeCritLen = RageTimer::GetTimeSinceStart() - g_fStartedTimeCritAt;
	if( fTimeCritLen > 0.1f )
		LOG->Warn( "Time-critical section lasted for %f", fTimeCritLen );
#endif
}

static int64_t GetMicrosecondsSinceEpoch()
{
	struct timeval tv;
	gettimeofday( &tv, NULL );

	return int64_t(tv.tv_sec) * 1000000 + int64_t(tv.tv_usec);
}

int64_t ArchHooks::GetMicrosecondsSinceStart( bool bAccurate )
{
	static int64_t iStartTime = GetMicrosecondsSinceEpoch();
	int64_t ret = GetMicrosecondsSinceEpoch() - iStartTime;
	if( bAccurate )
		ret = FixupTimeIfBackwards( ret );
	return ret;
}

#include "RageFileManager.h"

void ArchHooks::MountInitialFilesystems( const CString &sDirOfExecutable )
{
	CFBundleRef bundle = CFBundleGetMainBundle();
	CFURLRef bundleURL = CFBundleCopyBundleURL( bundle );
	CFURLRef dirURL = CFURLCreateCopyDeletingLastPathComponent( kCFAllocatorDefault, bundleURL );
	char dir[PATH_MAX];
	
	if( !CFURLGetFileSystemRepresentation(dirURL, true, (UInt8 *)dir, sizeof(dir)) )
		FAIL_M( "CFURLGetFileSystemRepresentation() failed." );
	CFRelease( bundleURL );
	CFRelease( dirURL );
	FILEMAN->Mount( "dir", dir, "/" );
	
	FSRef fs; // This does not need to be "released" by the file manager
	
	// This returns the absolute path for ~/Library/Application Support
	if( FSFindFolder(kUserDomain, kApplicationSupportFolderType, kDontCreateFolder, &fs) )
		FAIL_M( "FSFindFolder() failed." );
	if( FSRefMakePath(&fs, (UInt8 *)dir, sizeof(dir)) )
		FAIL_M( "FSRefMakePath() failed." );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/Save", dir), "/Save" );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/Screenshots", dir), "/Screenshots" );
	/* The Cache directory should probably go in ~/Library/Caches/bundle_identifier/Cache but
	 * why bother we're already using ~/Library/Application Support/PRODUCT_ID. */
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/Cache", dir), "/Cache" );
}

/*
 * (c) 2003-2005 Steve Checkoway
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
