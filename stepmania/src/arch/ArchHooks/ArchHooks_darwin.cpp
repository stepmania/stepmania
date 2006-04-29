#include "global.h"
#include "ArchHooks_darwin.h"
#include "RageLog.h"
#include "RageThreads.h"
#include "RageTimer.h"
#include "RageUtil.h"
#include "archutils/Darwin/Crash.h"
#include "archutils/Unix/CrashHandler.h"
#include "archutils/Unix/SignalHandler.h"
#include "ProductInfo.h"
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/thread_act.h>
#include <mach/mach.h>
#include <mach/host_info.h>
#include <mach/mach_time.h>
#include <mach/mach_error.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/sysctl.h>

#define REAL_TIME_CRITICAL_SECTION 0

#if REAL_TIME_CRITICAL_SECTION
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

#if defined(CRASH_HANDLER)
static void DoCrashSignalHandler( int signal, siginfo_t *si, const ucontext_t *uc )
{
	/* Don't dump a debug file if the user just hit ^C. */
	if( !IsFatalSignal(signal) )
		return;

	CrashHandler::CrashSignalHandler( signal, si, uc );
	/* not reached */
}
#endif

ArchHooks_darwin::ArchHooks_darwin()
{
	/* First, handle non-fatal termination signals. */
	SignalHandler::OnClose( DoCleanShutdown );
	
#if defined(CRASH_HANDLER)
	CrashHandler::CrashHandlerHandleArgs( g_argc, g_argv );
	SignalHandler::OnClose( DoCrashSignalHandler );
#endif
	
	TimeCritMutex = new RageMutex("TimeCritMutex");
	
	// CF*Copy* functions' return values need to be released, CF*Get* functions' do not.
	CFStringRef key = CFSTR( "ApplicationBundlePath" );
	
	CFBundleRef bundle = CFBundleGetMainBundle();
	CFStringRef appID = CFBundleGetIdentifier( bundle );
	CFStringRef version = CFStringRef( CFBundleGetValueForInfoDictionaryKey(bundle, kCFBundleVersionKey) );
	CFURLRef path = CFBundleCopyBundleURL( bundle );
	CFPropertyListRef value = CFURLCopyPath( path );
	CFPropertyListRef old = CFPreferencesCopyAppValue( key, appID );
	CFMutableDictionaryRef newDict = NULL;
	
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
		}
		CFRelease( old );
	}
	
	if( newDict )
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
	delete TimeCritMutex;
}

void ArchHooks_darwin::DumpDebugInfo()
{
	RString systemVersion;
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
	RString processor;
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
	RString ret = lang;
	
	free(lang);
	return ret;
}

void ArchHooks_darwin::EnterTimeCriticalSection()
{
#if REAL_TIME_CRITICAL_SECTION
	TimeCritMutex->Lock();
	
	mach_msg_type_number_t cnt = THREAD_TIME_CONSTRAINT_POLICY_COUNT;
	boolean_t bDefaults = false;
	kern_return_t ret;

	ret = thread_policy_get( mach_thread_self(), THREAD_TIME_CONSTRAINT_POLICY,
				 (int*)&g_oldttcpolicy, &cnt, &bDefaults );
	
	if( unlikely(ret != KERN_SUCCESS) )
		LOG->Warn( "thread_policy_get(): %s", mach_error_string(ret) );

	/* We want to monopolize the CPU for a very short period of time.  This means that the
	 * period doesn't really matter, and we don't want to be preempted. The thread_policy_set
	 * call will fail unless 50 us <= computation <= constraint <= 50 ms. This isn't
	 * anywhere except in the kernel source: min_rt_quantum and max_rt_quantum in
	 * http://darwinsource.opendarwin.org/10.4/xnu-792/osfmk/kern/sched_prim.c */
	thread_time_constraint_policy ttcpolicy;
	mach_timebase_info_data_t timeBase;
		
	mach_timebase_info( &timeBase );
	
	ttcpolicy.period = 0; // no periodicity
	// http://developer.apple.com/qa/qa2004/qa1398.html
	ttcpolicy.computation = uint32_t( 3000000.0 * timeBase.denom / timeBase.numer ); // 3 ms
	ttcpolicy.constraint = ttcpolicy.computation;
	ttcpolicy.preemptible = 0;
	ret = thread_policy_set( mach_thread_self(), THREAD_TIME_CONSTRAINT_POLICY,
				 (int*)&ttcpolicy, THREAD_TIME_CONSTRAINT_POLICY_COUNT );
	
	if( unlikely(ret != KERN_SUCCESS) )
		LOG->Warn( "thread_policy_set(THREAD_TIME_CONSTRAINT_POLICY): %s", mach_error_string(ret) );

	g_fStartedTimeCritAt = RageTimer::GetTimeSinceStart();
#endif
}

void ArchHooks_darwin::ExitTimeCriticalSection()
{
#if REAL_TIME_CRITICAL_SECTION
	kern_return_t ret;
	
	ret = thread_policy_set( mach_thread_self(), THREAD_TIME_CONSTRAINT_POLICY,
				 (int*) &g_oldttcpolicy, THREAD_TIME_CONSTRAINT_POLICY_COUNT );
	
	TimeCritMutex->Unlock();
	if( unlikely(ret != KERN_SUCCESS) )
		LOG->Warn( "thread_policy_set(g_oldttcpolicy): %s", mach_error_string(ret) );

	float fTimeCritLen = RageTimer::GetTimeSinceStart() - g_fStartedTimeCritAt;
	if( fTimeCritLen > 0.003f )
		LOG->Warn( "Time-critical section lasted for %f", fTimeCritLen );
#endif
}

bool ArchHooks_darwin::GoToURL( RString sUrl )
{
	CFURLRef url = CFURLCreateWithBytes( kCFAllocatorDefault, (const UInt8*)sUrl.data(),
					     sUrl.length(), kCFStringEncodingUTF8, NULL );
	OSStatus result = LSOpenCFURLRef( url, NULL );
	
	CFRelease( url );
	return result == 0;
}

int64_t ArchHooks::GetMicrosecondsSinceStart( bool bAccurate )
{
	// http://developer.apple.com/qa/qa2004/qa1398.html
	static uint64_t iStartTime = mach_absolute_time();
	static double factor = 0.0;
	
	if( unlikely(factor == 0.0) )
	{
		mach_timebase_info_data_t timeBase;
		
		mach_timebase_info( &timeBase );
		factor = timeBase.numer / ( 1000.0 * timeBase.denom );
	}
	return int64_t( (mach_absolute_time() - iStartTime) * factor );
}

#include "RageFileManager.h"

static void PathForFolderType( char dir[PATH_MAX], OSType folderType )
{
	FSRef fs;

	if( FSFindFolder(kUserDomain, folderType, kDontCreateFolder, &fs) )
		FAIL_M( ssprintf("FSFindFolder(%lu) failed.", folderType) );
	if( FSRefMakePath(&fs, (UInt8 *)dir, PATH_MAX) )
		FAIL_M( "FSRefMakePath() failed." );
}

void ArchHooks::MountInitialFilesystems( const RString &sDirOfExecutable )
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
	
	// /Save -> ~/Library/Application Support/PRODUCT_ID
	PathForFolderType( dir, kPreferencesFolderType );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID, dir), "/Save" );
	
	// /Screenshots -> ~/Documents/PRODUCT_ID Screenshots
	PathForFolderType( dir, kDocumentsFolderType );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID " Screenshots", dir), "/Screenshots" );
	
	// /Cache -> ~/Library/Caches/PRODUCT_ID
	PathForFolderType( dir, kCachedDataFolderType );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID, dir), "/Cache" );

	// /Logs -> ~/Library/Logs/PRODUCT_ID
	PathForFolderType( dir, kDomainLibraryFolderType );
	FILEMAN->Mount( "dir", ssprintf("%s/Logs/" PRODUCT_ID, dir), "/Logs" );
}

/*
 * (c) 2003-2006 Steve Checkoway
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
