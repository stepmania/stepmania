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
	// task_set_exception_ports( mach_task_self(), EXC_MASK_ALL, MACH_PORT_nullptr, EXCEPTION_DEFAULT, 0 );

	// CF*Copy* functions' return values need to be released, CF*Get* functions' do not.
	CFStringRef key = CFSTR( "ApplicationBundlePath" );

	CFBundleRef bundle = CFBundleGetMainBundle();
	CFStringRef appID = CFBundleGetIdentifier( bundle );
	if( appID == nullptr )
	{
		// We were probably launched through a symlink. Don't bother hunting down the real path.
		return;
	}
	CFStringRef version = CFStringRef( CFBundleGetValueForInfoDictionaryKey(bundle, kCFBundleVersionKey) );
	CFPropertyListRef old = CFPreferencesCopyAppValue( key, appID );
	CFURLRef path = CFBundleCopyBundleURL( bundle );
	CFPropertyListRef value = CFURLCopyFileSystemPath( path, kCFURLPOSIXPathStyle );
	CFMutableDictionaryRef newDict = nullptr;

	if( old && CFGetTypeID(old) != CFDictionaryGetTypeID() )
	{
		CFRelease( old );
		old = nullptr;
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

std::string ArchHooks_MacOSX::GetArchName() const
{
#if defined(__i386__)
	return "Mac OS X (i386)";
#elif defined(__x86_64__)
	return "Mac OS X (x86_64)";
#else
#error What arch?
#endif
}

void ArchHooks_MacOSX::DumpDebugInfo()
{
	// Get system version (like 10.x.x)
	std::string SystemVersion;
	{
		// http://stackoverflow.com/a/891336
		NSDictionary *version = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
		NSString *productVersion = [version objectForKey:@"ProductVersion"];
		SystemVersion = fmt::sprintf("Mac OS X %s", [productVersion cStringUsingEncoding:[NSString defaultCStringEncoding]]);
	}

	size_t size;
#define GET_PARAM( name, var ) (size = sizeof(var), sysctlbyname(name, &var, &size, nullptr, 0) )
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
	std::string sModel;
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
		CFURLRef urlRef = CFBundleCopyResourceURL( CFBundleGetMainBundle(), CFSTR("Hardware.plist"), nullptr, nullptr );

		if( urlRef == nullptr )
			break;
		CFDataRef dataRef = nullptr;
		SInt32 error;
		CFURLCreateDataAndPropertiesFromResource( nullptr, urlRef, &dataRef, nullptr, nullptr, &error );
		CFRelease( urlRef );
		if( dataRef == nullptr )
			break;
		// This also works with binary property lists for some reason.
		CFPropertyListRef plRef = CFPropertyListCreateFromXMLData( nullptr, dataRef, kCFPropertyListImmutable, nullptr );
		CFRelease( dataRef );
		if( plRef == nullptr )
			break;
		if( CFGetTypeID(plRef) != CFDictionaryGetTypeID() )
		{
			CFRelease( plRef );
			break;
		}
		CFStringRef keyRef = CFStringCreateWithCStringNoCopy( nullptr, szModel, kCFStringEncodingMacRoman, kCFAllocatorNull );
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

std::string ArchHooks::GetPreferredLanguage()
{
	CFStringRef app = kCFPreferencesCurrentApplication;
	CFTypeRef t = CFPreferencesCopyAppValue( CFSTR("AppleLanguages"), app );
	std::string ret = "en";

	if( t == nullptr )
	{
		return ret;
	}
	if( CFGetTypeID(t) != CFArrayGetTypeID() )
	{
		CFRelease( t );
		return ret;
	}

	CFArrayRef languages = CFArrayRef( t );
	CFStringRef lang;

	if( CFArrayGetCount(languages) > 0 &&
		(lang = (CFStringRef)CFArrayGetValueAtIndex(languages, 0)) != nullptr )
	{
		// MacRoman agrees with ASCII in the low-order 7 bits.
		const char *str = CFStringGetCStringPtr( lang, kCFStringEncodingMacRoman );
		if( str )
		{
			ret = std::string( str, 2 );
		}
		else
		{
			LOG->Warn( "Unable to determine system language. Using English." );
		}
	}

	CFRelease( languages );
	return ret;
}

bool ArchHooks_MacOSX::GoToURL( std::string sUrl )
{
	CFURLRef url = CFURLCreateWithBytes( kCFAllocatorDefault, (const UInt8*)sUrl.data(),
						 sUrl.length(), kCFStringEncodingUTF8, nullptr );
	OSStatus result = LSOpenCFURLRef( url, nullptr );

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
		FAIL_M( fmt::sprintf("FSFindFolder(%lu) failed.", folderType) );
	if( FSRefMakePath(&fs, (UInt8 *)dir, PATH_MAX) )
		FAIL_M( "FSRefMakePath() failed." );
}

void ArchHooks::MountInitialFilesystems( std::string const &sDirOfExecutable )
{
	char dir[PATH_MAX];
	CFURLRef dataUrl = CFBundleCopyResourceURL( CFBundleGetMainBundle(), CFSTR("StepMania"), CFSTR("smzip"), nullptr );

	FILEMAN->Mount( "dir", sDirOfExecutable, "/" );

	if( dataUrl )
	{
		CFStringRef dataPath = CFURLCopyFileSystemPath( dataUrl, kCFURLPOSIXPathStyle );
		CFStringGetCString( dataPath, dir, PATH_MAX, kCFStringEncodingUTF8 );

		if( strncmp(sDirOfExecutable.c_str(), dir, sDirOfExecutable.length()) == 0 )
		{
			FILEMAN->Mount( "zip", dir + sDirOfExecutable.length(), "/" );
		}
		CFRelease( dataPath );
		CFRelease( dataUrl );
	}
}

void ArchHooks::MountUserFilesystems( std::string const &sDirOfExecutable )
{
	// dir is reused several times, so don't change the ordering of any of the
	// directory mountings. -Kyz
	char dir[PATH_MAX];

	// Other stuff -> ~/Library/Application Support/PRODUCT_ID/*
	PathForFolderType( dir, kApplicationSupportFolderType );
	for(auto&& fs_dir : SpecialFiles::USER_CONTENT_DIRS)
	{
		FILEMAN->Mount("dir", fmt::sprintf("%s/" PRODUCT_ID "%s", dir, fs_dir.c_str()), fs_dir);
	}
	// The User Packages dir was the only one where the mount point didn't
	// match the folder name when I changed this to use a loop. -Kyz
	FILEMAN->Mount("dir", fmt::sprintf("%s/Packages", dir), "/" + SpecialFiles::USER_PACKAGES_DIR);

	std::vector<OSType> data_dir_folder_types= {
		// Be sure to keep this in sync with SpecialFiles::USER_DATA_DIRS. -Kyz
		// /Cache -> ~/Library/Caches/PRODUCT_ID
		kCachedDataFolderType,
		// /Logs -> ~/Library/Logs/PRODUCT_ID
		kDomainLibraryFolderType,
		// /Save -> ~/Library/Preferences/PRODUCT_ID
		kPreferencesFolderType,
		// /Screenshots -> ~/Pictures/PRODUCT_ID Screenshots
		kPictureDocumentsFolderType,
	};
	std::vector<std::string> dir_fmt_strs= {
		// I hate having all these special cases. -Kyz
		"%s/" PRODUCT_ID,
		"%s/Logs/" PRODUCT_ID,
		"%s/" PRODUCT_ID,
		"%s/" PRODUCT_ID " Screenshots",
	};
	ASSERT_M(
		data_dir_folder_types.size() == SpecialFiles::USER_DATA_DIRS.size() &&
		data_dir_folder_types.size() == dir_fmt_strs.size(),
		"ArchHooks::MountUserFilesystems in ArchHooks_MacOSX.mm needs to be "
		"updated because SpecialFiles::USER_DATA_DIRS was changed.");
	for(size_t i= 0; i < SpecialFiles::USER_DATA_DIRS.size(); ++i)
	{
		PathForFolderType(dir, data_dir_folder_types[i]);
		FILEMAN->Mount("dir", fmt::sprintf(dir_fmt_strs[i].c_str(), dir), SpecialFiles::USER_DATA_DIRS[i]);
	}
	// /Desktop -> /Users/<user>/Desktop/PRODUCT_ID
	// PathForFolderType( dir, kDesktopFolderType );
	// FILEMAN->Mount( "dir", fmt::sprintf("%s/" PRODUCT_ID, dir), "/Desktop" );
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
