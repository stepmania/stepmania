#include "global.h"
#include "ArchHooks_MacOSX.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "archutils/Unix/CrashHandler.h"
#include "archutils/Unix/SignalHandler.h"
#include "ProductInfo.h"

#include <cstddef>
#include <cstdint>

#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
extern "C" {
#include <mach/mach_time.h>
#include <IOKit/graphics/IOGraphicsLib.h>
}
#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>

#import <AppKit/NSScreen.h>
#import <Foundation/Foundation.h>

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

static bool DoCleanShutdown( int signal, siginfo_t *si, const ucontext_t *uc )
{
	if( IsFatalSignal(signal) )
		return false;

	// ^C.
	ArchHooks::SetUserQuit();
	return true;
}

static bool DoCrashSignalHandler( int signal, siginfo_t *si, const ucontext_t *uc )
{
	// Don't dump a debug file if the user just hit ^C.
	if( !IsFatalSignal(signal) )
		return true;

	CrashHandler::CrashSignalHandler( signal, si, uc );
	return true; // Unreached
}

static bool DoEmergencyShutdown( int signal, siginfo_t *si, const ucontext_t *us )
{
	if( IsFatalSignal(signal) )
		_exit( 1 ); // We ran the crash handler already
	return false;
}

void ArchHooks_MacOSX::Init()
{
	// First, handle non-fatal termination signals.
	SignalHandler::OnClose( DoCleanShutdown );
	CrashHandler::CrashHandlerHandleArgs( g_argc, g_argv );
	CrashHandler::InitializeCrashHandler();
	SignalHandler::OnClose( DoCrashSignalHandler );
	SignalHandler::OnClose( DoEmergencyShutdown );

	// Now that the crash handler is set up, disable crash reporter.
	// Breaks gdb
	// task_set_exception_ports( mach_task_self(), EXC_MASK_ALL, MACH_PORT_NULL, EXCEPTION_DEFAULT, 0 );

	// CF*Copy* functions' return values need to be released, CF*Get* functions' do not.
	CFStringRef key = CFSTR( "ApplicationBundlePath" );

	CFBundleRef bundle = CFBundleGetMainBundle();
	CFStringRef appID = CFBundleGetIdentifier( bundle );
	if( appID == nil)
	{
		// We were probably launched through a symlink. Don't bother hunting down the real path.
		return;
	}
	CFStringRef version = CFStringRef( CFBundleGetValueForInfoDictionaryKey(bundle, kCFBundleVersionKey) );
	CFPropertyListRef old = CFPreferencesCopyAppValue( key, appID );
	CFURLRef path = CFBundleCopyBundleURL( bundle );
	CFPropertyListRef value = CFURLCopyFileSystemPath( path, kCFURLPOSIXPathStyle );
	CFMutableDictionaryRef newDict = nil;

	if( old && CFGetTypeID(old) != CFDictionaryGetTypeID() )
	{
		CFRelease( old );
		old = nil;
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
			CFDictionarySetValue( newDict, version, value );
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

RString ArchHooks_MacOSX::GetArchName() const
{
#if defined(__x86_64__)
	return "macOS (x86_64)";
#elif defined(__aarch64__) || defined(__arm64__)
	return "macOS (arm64)";
#else
#error What arch?
#endif
}

void ArchHooks_MacOSX::DumpDebugInfo()
{
	// Get system version (like 10.x.x)
	RString SystemVersion;
	{
		// http://stackoverflow.com/a/891336
		NSDictionary *version = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
		NSString *productVersion = version[@"ProductVersion"];
		SystemVersion = ssprintf("macOS %s", [productVersion cStringUsingEncoding:[NSString defaultCStringEncoding]]);
	}

	std::size_t size;
#define GET_PARAM( name, var ) (size = sizeof(var), sysctlbyname(name, &var, &size, nil, 0) )
	// Get memory
	float fRam;
	char ramPower;
	{
		std::uint64_t iRam = 0;
		GET_PARAM( "hw.memsize", iRam );

		fRam = float( double(iRam) / 1073741824.0 );
		ramPower = 'G';
	}

	// Get processor information
	int iMaxCPUs = 0;
	int iCPUs = 0;
	float fFreq;
	char freqPower;
	RString sModel("Unknown");
	do {
		char szModel[128];
		std::uint64_t iFreq;

		GET_PARAM( "hw.logicalcpu_max", iMaxCPUs );
		GET_PARAM( "hw.logicalcpu", iCPUs );
		GET_PARAM( "hw.cpufrequency", iFreq );

		fFreq = float( double(iFreq) / 1000000000.0 );
		freqPower = 'G';

		if( GET_PARAM("hw.model", szModel) != 0 )
			break;

		sModel = szModel;

		NSURL* url = [NSURL fileURLWithPath:@"//System/Library/PrivateFrameworks/ServerInformation.framework/Versions/A/Resources/en.lproj/SIMachineAttributes.plist"];
		NSDictionary* machineAttributes = [NSDictionary dictionaryWithContentsOfURL:url];
		if (machineAttributes == nil)
			break;

		NSString* key = [NSString stringWithUTF8String:szModel];
		NSString* val = machineAttributes[key][@"_LOCALIZABLE_"][@"marketingModel"];
		if (val != nil)
			sModel = [val UTF8String];
	} while( false );
#undef GET_PARAM

	// Send all of the information to the log
	LOG->Info( "Model: %s (%d/%d)", sModel.c_str(), iCPUs, iMaxCPUs );
	LOG->Info( "Clock speed %.2f %cHz", fFreq, freqPower );
	LOG->Info( "%s", SystemVersion.c_str());
	LOG->Info( "Memory: %.2f %cB", fRam, ramPower );
}

RString ArchHooks::GetPreferredLanguage()
{
	CFStringRef app = kCFPreferencesCurrentApplication;
	CFTypeRef t = CFPreferencesCopyAppValue( CFSTR("AppleLanguages"), app );
	RString ret = "en";

	if( t == nil)
		return ret;
	if( CFGetTypeID(t) != CFArrayGetTypeID() )
	{
		CFRelease( t );
		return ret;
	}

	CFArrayRef languages = CFArrayRef( t );
	CFStringRef lang;

	if( CFArrayGetCount(languages) > 0 &&
		(lang = (CFStringRef)CFArrayGetValueAtIndex(languages, 0)) != nil)
	{
		// MacRoman agrees with ASCII in the low-order 7 bits.
		const char *str = CFStringGetCStringPtr( lang, kCFStringEncodingMacRoman );
		if( str )
		{
			ret = RString( str, 2 );
			if (ret == "zh")
			{
				ret = RString(str, 7);
				ret[2] = '-';
			}
		}
		else
			LOG->Warn( "Unable to determine system language. Using English." );
	}

	CFRelease( languages );
	return ret;
}

bool ArchHooks_MacOSX::GoToURL( RString sUrl )
{
	CFURLRef url = CFURLCreateWithBytes( kCFAllocatorDefault, (const UInt8*)sUrl.data(),
						 sUrl.length(), kCFStringEncodingUTF8, nil);
	OSStatus result = LSOpenCFURLRef( url, nil);

	CFRelease( url );
	return result == 0;
}

std::int64_t ArchHooks::GetMicrosecondsSinceStart( bool bAccurate )
{
	// http://developer.apple.com/qa/qa2004/qa1398.html
	static double factor = 0.0;

	if( unlikely(factor == 0.0) )
	{
		mach_timebase_info_data_t timeBase;

		mach_timebase_info( &timeBase );
		factor = timeBase.numer / ( 1000.0 * timeBase.denom );
	}
	return std::int64_t( mach_absolute_time() * factor );
}

#include "RageFileManager.h"

void ArchHooks::MountInitialFilesystems( const RString &sDirOfExecutable )
{
	FILEMAN->Mount("dirro", sDirOfExecutable, "/");

	bool portable = DoesFileExist("/Portable.ini");

	NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
	if( resourcePath )
	{
		const char* resourcePathUTF8String = [resourcePath UTF8String];
		FILEMAN->Mount( "dirro", ssprintf("%s/Announcers", resourcePathUTF8String), "/Announcers" );
		FILEMAN->Mount( "dirro", ssprintf("%s/BGAnimations", resourcePathUTF8String), "/BGAnimations" );
		FILEMAN->Mount( "dirro", ssprintf("%s/BackgroundEffects", resourcePathUTF8String), "/BackgroundEffects" );
		FILEMAN->Mount( "dirro", ssprintf("%s/BackgroundTransitions", resourcePathUTF8String), "/BackgroundTransitions" );
		FILEMAN->Mount( "dirro", ssprintf("%s/CDTitles", resourcePathUTF8String), "/CDTitles" );
		FILEMAN->Mount( "dirro", ssprintf("%s/Characters", resourcePathUTF8String), "/Characters" );
		FILEMAN->Mount( "dirro", ssprintf("%s/Courses", resourcePathUTF8String), "/Courses" );
		FILEMAN->Mount( "dirro", ssprintf("%s/NoteSkins", resourcePathUTF8String), "/NoteSkins" );
		FILEMAN->Mount( "dirro", ssprintf("%s/Packages", resourcePathUTF8String), "/Packages" );
		FILEMAN->Mount( "dirro", ssprintf("%s/Songs", resourcePathUTF8String), "/Songs" );
		FILEMAN->Mount( "dirro", ssprintf("%s/RandomMovies", resourcePathUTF8String), "/RandomMovies" );
		FILEMAN->Mount( "dirro", ssprintf("%s/Themes", resourcePathUTF8String), "/Themes" );
		FILEMAN->Mount( "dirro", ssprintf("%s/Data", resourcePathUTF8String), "/Data" );
	}

	CFURLRef dataUrl = CFBundleCopyResourceURL( CFBundleGetMainBundle(), CFSTR("StepMania"), CFSTR("smzip"), nil);
	if( dataUrl )
	{
		char dir[PATH_MAX];

		CFStringRef dataPath = CFURLCopyFileSystemPath( dataUrl, kCFURLPOSIXPathStyle );
		CFStringGetCString( dataPath, dir, PATH_MAX, kCFStringEncodingUTF8 );

		if( strncmp(sDirOfExecutable, dir, sDirOfExecutable.length()) == 0 )
			FILEMAN->Mount( "zip", dir + sDirOfExecutable.length(), "/" );
		CFRelease( dataPath );
		CFRelease( dataUrl );
	}

	if (portable)
	{
		FILEMAN->Mount("dir", sDirOfExecutable + "/Announcers", "/Announcers");
		FILEMAN->Mount("dir", sDirOfExecutable + "/BGAnimations", "/BGAnimations");
		FILEMAN->Mount("dir", sDirOfExecutable + "/BackgroundEffects", "/BackgroundEffects");
		FILEMAN->Mount("dir", sDirOfExecutable + "/BackgroundTransitions", "/BackgroundTransitions");
		FILEMAN->Mount("dir", sDirOfExecutable + "/Cache", "/Cache");
		FILEMAN->Mount("dir", sDirOfExecutable + "/CDTitles", "/CDTitles");
		FILEMAN->Mount("dir", sDirOfExecutable + "/Characters", "/Characters");
		FILEMAN->Mount("dir", sDirOfExecutable + "/Courses", "/Courses");
		FILEMAN->Mount("dir", sDirOfExecutable + "/Downloads", "/Downloads");
		FILEMAN->Mount("dir", sDirOfExecutable + "/Logs", "/Logs");
		FILEMAN->Mount("dir", sDirOfExecutable + "/NoteSkins", "/NoteSkins");
		FILEMAN->Mount("dir", sDirOfExecutable + "/Packages", "/Packages");
		FILEMAN->Mount("dir", sDirOfExecutable + "/Save", "/Save");
		FILEMAN->Mount("dir", sDirOfExecutable + "/Screenshots", "/Screenshots");
		FILEMAN->Mount("dir", sDirOfExecutable + "/Songs", "/Songs");
		FILEMAN->Mount("dir", sDirOfExecutable + "/RandomMovies", "/RandomMovies");
		FILEMAN->Mount("dir", sDirOfExecutable + "/Themes", "/Themes");
	}
}

static std::string PathForDirectory( NSSearchPathDirectory directory )
{
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSURL *url = [fileManager URLForDirectory:directory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:nil];
	if (url == nil)
		FAIL_M( "URLForDirectory() failed." );

	return [url fileSystemRepresentation];
}

void ArchHooks::MountUserFilesystems( const RString &sDirOfExecutable )
{
	// /Save -> ~/Library/Preferences/PRODUCT_ID
	std::string libraryDir = PathForDirectory(NSLibraryDirectory);
	FILEMAN->Mount( "dir", libraryDir + "/Preferences/" PRODUCT_ID, "/Save" );

	// Other stuff -> ~/Library/Application Support/PRODUCT_ID/*
	std::string appSupportDir = PathForDirectory(NSApplicationSupportDirectory);
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/Announcers", "/Announcers" );
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/BGAnimations", "/BGAnimations" );
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/BackgroundEffects", "/BackgroundEffects" );
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/BackgroundTransitions", "/BackgroundTransitions" );
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/CDTitles", "/CDTitles" );
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/Characters", "/Characters" );
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/Courses", "/Courses" );
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/Downloads", "/Downloads" );
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/NoteSkins", "/NoteSkins" );
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/Packages", "/Packages" );
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/Songs", "/Songs" );
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/RandomMovies", "/RandomMovies" );
	FILEMAN->Mount( "dir", appSupportDir + "/" PRODUCT_ID "/Themes", "/Themes" );

	// /Screenshots -> ~/Pictures/PRODUCT_ID Screenshots
	std::string picturesDir = PathForDirectory(NSCachesDirectory);
	FILEMAN->Mount( "dir", picturesDir + "/" PRODUCT_ID " Screenshots", "/Screenshots" );

	// /Cache -> ~/Library/Caches/PRODUCT_ID
	std::string cachesDir = PathForDirectory(NSCachesDirectory);
	FILEMAN->Mount( "dir", cachesDir + "/" PRODUCT_ID, "/Cache" );

	// /Logs -> ~/Library/Logs/PRODUCT_ID
	FILEMAN->Mount( "dir", libraryDir + "/Logs/" PRODUCT_ID, "/Logs" );
}

static inline int GetIntValue( CFTypeRef r )
{
	int ret;

	if( !r || CFGetTypeID(r) != CFNumberGetTypeID() || !CFNumberGetValue(CFNumberRef(r), kCFNumberIntType, &ret) )
		return 0;
	return ret;
}


float ArchHooks_MacOSX::GetDisplayAspectRatio()
{
	NSScreen *screen = [NSScreen mainScreen];
	return screen.frame.size.width / screen.frame.size.height;
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
