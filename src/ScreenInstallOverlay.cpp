#include "global.h"
#include "ScreenInstallOverlay.h"
#include "RageFileManager.h"
#include "ScreenManager.h"
#include "Preference.h"
#include "RageLog.h"
#include "FileDownload.h"
#include "json/json.h"
#include "JsonUtil.h"
#include "SpecialFiles.h"
class Song;
#include "SongManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "CommonMetrics.h"
#include "SongManager.h"
#include "CommandLineActions.h"
#include "ScreenDimensions.h"
#include "StepMania.h"
#include "ActorUtil.h"

struct PlayAfterLaunchInfo
{
	RString sSongDir;
	RString sTheme;
	bool bAnySongChanged;
	bool bAnyThemeChanged;

	PlayAfterLaunchInfo()
	{
		bAnySongChanged = false;
		bAnyThemeChanged = false;
	}

	void OverlayWith( const PlayAfterLaunchInfo &other )
	{
		if( !other.sSongDir.empty() ) sSongDir = other.sSongDir;
		if( !other.sTheme.empty() ) sTheme = other.sTheme;
		bAnySongChanged |= other.bAnySongChanged;
		bAnyThemeChanged |= other.bAnyThemeChanged;
	}
};

void InstallSmzipOsArg( const RString &sOsZipFile, PlayAfterLaunchInfo &out );
PlayAfterLaunchInfo DoInstalls( CommandLineActions::CommandLineArgs args );

static void Parse( const RString &sDir, PlayAfterLaunchInfo &out )
{
	vector<RString> vsDirParts;
	split( sDir, "/", vsDirParts, true );
	if( vsDirParts.size() == 3 && vsDirParts[0].EqualsNoCase("Songs") )
		out.sSongDir = "/" + sDir;
	else if( vsDirParts.size() == 2 && vsDirParts[0].EqualsNoCase("Themes") )
		out.sTheme = vsDirParts[1];
}

static const RString TEMP_ZIP_MOUNT_POINT = "/@temp-zip/";
const RString TEMP_OS_MOUNT_POINT = "/@temp-os/";

static void InstallSmzip( const RString &sZipFile, PlayAfterLaunchInfo &out )
{
	if( !FILEMAN->Mount( "zip", sZipFile, TEMP_ZIP_MOUNT_POINT ) )
		FAIL_M("Failed to mount " + sZipFile );

	vector<RString> vsFiles;
	{
		vector<RString> vsRawFiles;
		GetDirListingRecursive( TEMP_ZIP_MOUNT_POINT, "*", vsRawFiles);

		vector<RString> vsPrettyFiles;
		for (RString const &s : vsRawFiles)
		{
			if( GetExtension(s).EqualsNoCase("ctl") )
				continue;

			vsFiles.push_back( s);

			RString s2 = s.Right( s.length() - TEMP_ZIP_MOUNT_POINT.length() );
			vsPrettyFiles.push_back( s2 );
		}
		sort( vsPrettyFiles.begin(), vsPrettyFiles.end() );
	}

	RString sResult = "Success installing " + sZipFile;
	for (RString &tmpFile : vsFiles)
	{
		RString sDestFile = tmpFile;
		sDestFile = sDestFile.Right( sDestFile.length() - TEMP_ZIP_MOUNT_POINT.length() );

		RString sDir, sThrowAway;
		splitpath( sDestFile, sDir, sThrowAway, sThrowAway );

		Parse( sDir, out );
		out.bAnySongChanged = true;

		FILEMAN->CreateDir( sDir );

		if( !FileCopy( tmpFile, sDestFile ) )
		{
			sResult = "Error extracting " + sDestFile;
			break;
		}
	}
	FILEMAN->Unmount( "zip", sZipFile, TEMP_ZIP_MOUNT_POINT );

	SCREENMAN->SystemMessage( sResult );
}

void InstallSmzipOsArg( const RString &sOsZipFile, PlayAfterLaunchInfo &out )
{
	SCREENMAN->SystemMessage("Installing " + sOsZipFile );

	RString sOsDir, sFilename, sExt;
	splitpath( sOsZipFile, sOsDir, sFilename, sExt );

	if( !FILEMAN->Mount( "dir", sOsDir, TEMP_OS_MOUNT_POINT ) )
		FAIL_M("Failed to mount " + sOsDir );
	InstallSmzip( TEMP_OS_MOUNT_POINT + sFilename + sExt, out );

	FILEMAN->Unmount( "dir", sOsDir, TEMP_OS_MOUNT_POINT );
}

struct FileCopyResult
{
	FileCopyResult( RString _sFile, RString _sComment ) : sFile(_sFile), sComment(_sComment) {}
	RString sFile, sComment;
};

#if !defined(WITHOUT_NETWORKING)
Preference<RString> g_sCookie( "Cookie", "" );

class DownloadTask
{
	FileTransfer *m_pTransfer;
	vector<RString> m_vsQueuedPackageUrls;
	RString m_sCurrentPackageTempFile;
	enum
	{
		control,
		packages
	} m_DownloadState;
	PlayAfterLaunchInfo m_playAfterLaunchInfo;
public:
	DownloadTask(const RString &sControlFileUri)
	{
		//SCREENMAN->SystemMessage( "Downloading control file." );
		m_pTransfer = new FileTransfer();
		m_pTransfer->StartDownload( sControlFileUri, "" );
		m_DownloadState = control;
	}
	~DownloadTask()
	{
		SAFE_DELETE(m_pTransfer);
	}
	RString GetStatus()
	{
		if( m_pTransfer == nullptr )
			return "";
		else
			return m_pTransfer->GetStatus();
	}
	bool UpdateAndIsFinished( float fDeltaSeconds, PlayAfterLaunchInfo &playAfterLaunchInfo )
	{
		m_pTransfer->Update( fDeltaSeconds );
		switch( m_DownloadState )
		{
		case control:
			if( m_pTransfer->IsFinished() )
			{
				SCREENMAN->SystemMessage( "Downloading required .smzip" );

				RString sResponse = m_pTransfer->GetResponse();
				SAFE_DELETE( m_pTransfer );

				Json::Value root;
				RString sError;
				if( !JsonUtil::LoadFromString(root, sResponse, sError) )
				{
					SCREENMAN->SystemMessage( sError );
					return true;
				}

				// Parse the JSON response, make a list of all packages need to be downloaded.
				{
					if( root["Cookie"].isString() )
						g_sCookie.Set( root["Cookie"].asString() );
					Json::Value require = root["Require"];
					if( require.isArray() )
					{
						for (Json::Value const &iter : require)
						{
							if( iter["Dir"].isString() )
							{
								RString sDir = iter["Dir"].asString();
								Parse( sDir, m_playAfterLaunchInfo );
								if( DoesFileExist( sDir ) )
									continue;
							}

							RString sUri;
							if( iter["Uri"].isString() )
							{
								sUri = iter["Uri"].asString();
								m_vsQueuedPackageUrls.push_back( sUri );
							}
						}
					}
				}

				/*
				{
					// TODO: Validate that this zip contains files for this version of StepMania

					bool bFileExists = DoesFileExist( SpecialFiles::PACKAGES_DIR + sFilename + sExt );
					if( FileCopy( TEMP_MOUNT_POINT + sFilename + sExt, SpecialFiles::PACKAGES_DIR + sFilename + sExt ) )
						vSucceeded.push_back( FileCopyResult(*s,bFileExists ? "overwrote existing file" : "") );
					else
						vFailed.push_back( FileCopyResult(*s,ssprintf("error copying file to '%s'",sOsDir.c_str())) );

				}
				*/
				m_DownloadState = packages;
				if( !m_vsQueuedPackageUrls.empty() )
				{
					RString sUrl = m_vsQueuedPackageUrls.back();
					m_vsQueuedPackageUrls.pop_back();
					m_sCurrentPackageTempFile = MakeTempFileName(sUrl);
					ASSERT(m_pTransfer == nullptr);
					m_pTransfer = new FileTransfer();
					m_pTransfer->StartDownload( sUrl, m_sCurrentPackageTempFile );
				}
			}
			break;
		case packages:
			{
				if( m_pTransfer->IsFinished() )
				{
					SAFE_DELETE( m_pTransfer );
					InstallSmzip( m_sCurrentPackageTempFile, m_playAfterLaunchInfo );
					FILEMAN->Remove( m_sCurrentPackageTempFile );	// Harmless if this fails because download didn't finish
				}
				if( !m_vsQueuedPackageUrls.empty() )
				{
					RString sUrl = m_vsQueuedPackageUrls.back();
					m_vsQueuedPackageUrls.pop_back();
					m_sCurrentPackageTempFile = MakeTempFileName(sUrl);
					ASSERT(m_pTransfer == nullptr);
					m_pTransfer = new FileTransfer();
					m_pTransfer->StartDownload( sUrl, m_sCurrentPackageTempFile );
				}
			}
			break;
		}
		bool bFinished = m_DownloadState == packages  &&  
			m_vsQueuedPackageUrls.empty() && 
			m_pTransfer == nullptr;
		if( bFinished )
		{
			Message msg( "DownloadFinished" );
			MESSAGEMAN->Broadcast(msg);

			playAfterLaunchInfo = m_playAfterLaunchInfo;
			return true;
		}
		else
		{
			return false;
		}
	}
	static RString MakeTempFileName( RString s )
	{
		return SpecialFiles::CACHE_DIR + "Downloads/" + Basename(s);
	}
};
static vector<DownloadTask*> g_pDownloadTasks;
#endif

static bool IsStepManiaProtocol(const RString &arg)
{
	// for now, only load from the StepMania domain until the security implications of this feature are better understood.
	//return BeginsWith(arg,"stepmania://beta.stepmania.com/");
	return BeginsWith(arg,"stepmania://");
}

static bool IsPackageFile(const RString &arg)
{
	RString ext = GetExtension(arg);
	return ext.EqualsNoCase("smzip") || ext.EqualsNoCase("zip");
}

PlayAfterLaunchInfo DoInstalls( CommandLineActions::CommandLineArgs args )
{
	PlayAfterLaunchInfo ret;
	for( int i = 0; i<(int)args.argv.size(); i++ )
	{
		RString s = args.argv[i];
		if( IsStepManiaProtocol(s) )
    {
#if !defined(WITHOUT_NETWORKING)
			g_pDownloadTasks.push_back( new DownloadTask(s) );
#else
      // TODO: Figure out a meaningful log message.
#endif
    }
		else if( IsPackageFile(s) )
			InstallSmzipOsArg(s, ret);
	}
	return ret;
}

REGISTER_SCREEN_CLASS( ScreenInstallOverlay );

ScreenInstallOverlay::~ScreenInstallOverlay()
{
}
void ScreenInstallOverlay::Init()
{
	Screen::Init();

	m_textStatus.LoadFromFont( THEME->GetPathF("ScreenInstallOverlay", "status") );
	m_textStatus.SetName("Status");
	ActorUtil::LoadAllCommandsAndSetXY(m_textStatus,"ScreenInstallOverlay");
	this->AddChild( &m_textStatus );
}

bool ScreenInstallOverlay::Input( const InputEventPlus &input )
{
	/*
	if( input.DeviceI.button == g_buttonLogin && input.type == IET_FIRST_PRESS )
	{
		HOOKS->GoToURL("http://www.stepmania.com/launch.php");
		return true;
	}
	*/

	return Screen::Input(input);
}

void ScreenInstallOverlay::Update( float fDeltaTime )
{
	Screen::Update(fDeltaTime);
	PlayAfterLaunchInfo playAfterLaunchInfo;
	while( CommandLineActions::ToProcess.size() > 0 )
	{
		CommandLineActions::CommandLineArgs args = CommandLineActions::ToProcess.back();
		CommandLineActions::ToProcess.pop_back();
 		PlayAfterLaunchInfo pali2 = DoInstalls( args );
		playAfterLaunchInfo.OverlayWith( pali2 );
	}
#if !defined(WITHOUT_NETWORKING)
	for(int i=g_pDownloadTasks.size()-1; i>=0; --i)
	{
		DownloadTask *p = g_pDownloadTasks[i];
		PlayAfterLaunchInfo pali;
		if( p->UpdateAndIsFinished( fDeltaTime, pali) )
		{
			playAfterLaunchInfo.OverlayWith(pali);
			SAFE_DELETE(p);
			g_pDownloadTasks.erase( g_pDownloadTasks.begin()+i );
		}
	}

	{
		vector<RString> vsMessages;
		for (DownloadTask *pDT : g_pDownloadTasks)
		{
			vsMessages.push_back( pDT->GetStatus() );
		}
		m_textStatus.SetText( join("\n", vsMessages) );
	}
#endif
	if( playAfterLaunchInfo.bAnySongChanged )
		SONGMAN->Reload( false, nullptr );

	if( !playAfterLaunchInfo.sSongDir.empty() )
	{
		Song* pSong = nullptr;
		GAMESTATE->Reset();
		RString sInitialScreen;
		if( playAfterLaunchInfo.sSongDir.length() > 0 )
			pSong = SONGMAN->GetSongFromDir( playAfterLaunchInfo.sSongDir );
		if( pSong )
		{
			vector<const Style*> vpStyle;
			GAMEMAN->GetStylesForGame( GAMESTATE->m_pCurGame, vpStyle, false );
			GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );
			GAMESTATE->m_bSideIsJoined[0] = true;
			GAMESTATE->SetMasterPlayerNumber(PLAYER_1);
			GAMESTATE->SetCurrentStyle( vpStyle[0], PLAYER_1 );
			GAMESTATE->m_pCurSong.Set( pSong );
			GAMESTATE->m_pPreferredSong = pSong;
			sInitialScreen = StepMania::GetSelectMusicScreen();
		}
		else
		{
			sInitialScreen = StepMania::GetInitialScreen();
		}

		Screen *curScreen = SCREENMAN->GetTopScreen();
		if(curScreen->GetScreenType() == game_menu || curScreen->GetScreenType() == attract)
			SCREENMAN->SetNewScreen( sInitialScreen );
	}
}

/*
 * (c) 2001-2005 Chris Danford, Glenn Maynard
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
