#include "global.h"
#include "ArchHooks_MacOSX.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "archutils/Unix/CrashHandler.h"
#include "archutils/Unix/SignalHandler.h"
#include "SpecialFiles.h"
#include "ProductInfo.h"
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
#if defined(__i386__)
	return "Mac OS X (i386)";
#elif defined(__x86_64__)
	return "Mac OS X (x86_64)";
#elif defined(__aarch64__) || defined(__arm64__)
	return "macOS (ARM64)";
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
		NSString *productVersion = [version objectForKey:@"ProductVersion"];
		SystemVersion = ssprintf("Mac OS X %s", [productVersion cStringUsingEncoding:[NSString defaultCStringEncoding]]);
	}

	size_t size;
#define GET_PARAM( name, var ) (size = sizeof(var), sysctlbyname(name, &var, &size, nil, 0) )
	// Get memory
	float fRam;
	char ramPower;
	{
		uint64_t iRam = 0;
		GET_PARAM( "hw.memsize", iRam );
		if( iRam >= 1073741824 )
		{
			fRam = float( double(iRam) / 1073741824.0 );
			ramPower = 'G';
		}
		else
		{
			fRam = float( double(iRam) / 1048576.0 );
			ramPower = 'M';
		}
	}

	// Get processor information
	int iMaxCPUs = 0;
	int iCPUs = 0;
	float fFreq;
	char freqPower;
	RString sModel;
	do {
		char szModel[128];
		uint64_t iFreq;

		GET_PARAM( "hw.logicalcpu_max", iMaxCPUs );
		GET_PARAM( "hw.logicalcpu", iCPUs );
		GET_PARAM( "hw.cpufrequency", iFreq );

		if( iFreq >= 1000000000 )
		{
			fFreq = float( double(iFreq) / 1000000000.0 );
			freqPower = 'G';
		}
		else
		{
			fFreq = float( double(iFreq) / 1000000.0 );
			freqPower = 'M';
		}

		if( GET_PARAM("hw.model", szModel) )
		{
			sModel = "Unknown";
			break;
		}
		sModel = szModel;
		CFURLRef urlRef = CFBundleCopyResourceURL( CFBundleGetMainBundle(), CFSTR("Hardware.plist"), nil, nil);

		if( urlRef == nil)
			break;
		CFDataRef dataRef = nil;
		SInt32 error;
		CFURLCreateDataAndPropertiesFromResource( nil, urlRef, &dataRef, nil, nil, &error );
		CFRelease( urlRef );
		if( dataRef == nil)
			break;
		// This also works with binary property lists for some reason.
		CFPropertyListRef plRef = CFPropertyListCreateFromXMLData( nil, dataRef, kCFPropertyListImmutable, nil);
		CFRelease( dataRef );
		if( plRef == nil)
			break;
		if( CFGetTypeID(plRef) != CFDictionaryGetTypeID() )
		{
			CFRelease( plRef );
			break;
		}
		CFStringRef keyRef = CFStringCreateWithCStringNoCopy( nil, szModel, kCFStringEncodingMacRoman, kCFAllocatorNull );
		CFStringRef modelRef = (CFStringRef)CFDictionaryGetValue( (CFDictionaryRef)plRef, keyRef );
		if( modelRef )
			sModel = CFStringGetCStringPtr( modelRef, kCFStringEncodingMacRoman );
		CFRelease( keyRef );
		CFRelease( plRef );
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

int64_t ArchHooks::GetMicrosecondsSinceStart( bool bAccurate )
{
	// http://developer.apple.com/qa/qa2004/qa1398.html
	static double factor = 0.0;

	if( unlikely(factor == 0.0) )
	{
		mach_timebase_info_data_t timeBase;

		mach_timebase_info( &timeBase );
		factor = timeBase.numer / ( 1000.0 * timeBase.denom );
	}
	return int64_t( mach_absolute_time() * factor );
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
	char dir[PATH_MAX];
	CFURLRef dataUrl = CFBundleCopyResourceURL( CFBundleGetMainBundle(), CFSTR("StepMania"), CFSTR("smzip"), nil);

	FILEMAN->Mount( "dir", sDirOfExecutable, "/" );

	if( dataUrl )
	{
		CFStringRef dataPath = CFURLCopyFileSystemPath( dataUrl, kCFURLPOSIXPathStyle );
		CFStringGetCString( dataPath, dir, PATH_MAX, kCFStringEncodingUTF8 );

		if( strncmp(sDirOfExecutable, dir, sDirOfExecutable.length()) == 0 )
			FILEMAN->Mount( "zip", dir + sDirOfExecutable.length(), "/" );
		CFRelease( dataPath );
		CFRelease( dataUrl );
	}
}

void ArchHooks::MountUserFilesystems( const RString &sDirOfExecutable )
{
	char dir[PATH_MAX];

	// /Save -> ~/Library/Preferences/PRODUCT_ID
	PathForFolderType( dir, kPreferencesFolderType );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID, dir), "/Save" );

	// Other stuff -> ~/Library/Application Support/PRODUCT_ID/*
	PathForFolderType( dir, kApplicationSupportFolderType );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/Announcers", dir), "/Announcers" );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/BGAnimations", dir), "/BGAnimations" );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/BackgroundEffects", dir), "/BackgroundEffects" );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/BackgroundTransitions", dir), "/BackgroundTransitions" );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/CDTitles", dir), "/CDTitles" );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/Characters", dir), "/Characters" );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/Courses", dir), "/Courses" );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/NoteSkins", dir), "/NoteSkins" );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/Packages", dir), "/" + SpecialFiles::USER_PACKAGES_DIR );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/Songs", dir), "/Songs" );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/RandomMovies", dir), "/RandomMovies" );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID "/Themes", dir), "/Themes" );

	// /Screenshots -> ~/Pictures/PRODUCT_ID Screenshots
	PathForFolderType( dir, kPictureDocumentsFolderType );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID " Screenshots", dir), "/Screenshots" );

	// /Cache -> ~/Library/Caches/PRODUCT_ID
	PathForFolderType( dir, kCachedDataFolderType );
	FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID, dir), "/Cache" );

	// /Logs -> ~/Library/Logs/PRODUCT_ID
	PathForFolderType( dir, kDomainLibraryFolderType );
	FILEMAN->Mount( "dir", ssprintf("%s/Logs/" PRODUCT_ID, dir), "/Logs" );
    
    NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
    if( resourcePath )
    {
        const char* resourcePathUTF8String = [resourcePath UTF8String];
        FILEMAN->Mount( "dir", ssprintf("%s/Announcers", resourcePathUTF8String), "/Announcers" );
        FILEMAN->Mount( "dir", ssprintf("%s/BGAnimations", resourcePathUTF8String), "/BGAnimations" );
        FILEMAN->Mount( "dir", ssprintf("%s/BackgroundEffects", resourcePathUTF8String), "/BackgroundEffects" );
        FILEMAN->Mount( "dir", ssprintf("%s/BackgroundTransitions", resourcePathUTF8String), "/BackgroundTransitions" );
        FILEMAN->Mount( "dir", ssprintf("%s/CDTitles", resourcePathUTF8String), "/CDTitles" );
        FILEMAN->Mount( "dir", ssprintf("%s/Characters", resourcePathUTF8String), "/Characters" );
        FILEMAN->Mount( "dir", ssprintf("%s/Courses", resourcePathUTF8String), "/Courses" );
        FILEMAN->Mount( "dir", ssprintf("%s/NoteSkins", resourcePathUTF8String), "/NoteSkins" );
        FILEMAN->Mount( "dir", ssprintf("%s/Packages", resourcePathUTF8String), "/" + SpecialFiles::USER_PACKAGES_DIR );
        FILEMAN->Mount( "dir", ssprintf("%s/Songs", resourcePathUTF8String), "/Songs" );
        FILEMAN->Mount( "dir", ssprintf("%s/RandomMovies", resourcePathUTF8String), "/RandomMovies" );
        FILEMAN->Mount( "dir", ssprintf("%s/Themes", resourcePathUTF8String), "/Themes" );
        FILEMAN->Mount( "dir", ssprintf("%s/Data", resourcePathUTF8String), "/Data" );
    }

	// /Desktop -> /Users/<user>/Desktop/PRODUCT_ID
	// PathForFolderType( dir, kDesktopFolderType );
	// FILEMAN->Mount( "dir", ssprintf("%s/" PRODUCT_ID, dir), "/Desktop" );
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
	io_connect_t displayPort = CGDisplayIOServicePort( CGMainDisplayID() );
	CFDictionaryRef dict = IODisplayCreateInfoDictionary( displayPort, 0 );
	int width = GetIntValue( CFDictionaryGetValue(dict, CFSTR(kDisplayHorizontalImageSize)) );
	int height = GetIntValue( CFDictionaryGetValue(dict, CFSTR(kDisplayVerticalImageSize)) );

	CFRelease( dict );

	if( width && height )
		return float(width)/height;
	return 4/3.f;
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
