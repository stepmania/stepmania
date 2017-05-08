#include "global.h"
#include "ScreenInstallOverlay.h"
#include "RageFileManager.h"
#include "ScreenManager.h"
#include "Preference.h"
#include "RageLog.h"
#include "FileDownload.h"
#include "json/value.h"
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

using std::vector;

struct PlayAfterLaunchInfo
{
	std::string sSongDir;
	std::string sTheme;
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

void InstallSmzipOsArg( const std::string &sOsZipFile, PlayAfterLaunchInfo &out );
PlayAfterLaunchInfo DoInstalls( CommandLineActions::CommandLineArgs args );

static void Parse(const std::string &sDir, PlayAfterLaunchInfo &out)
{
	auto vsDirParts = Rage::split(sDir, ",", Rage::EmptyEntries::skip);
	// sanity check
	if (vsDirParts.size() < 1)
	{
		return;
	}
	Rage::ci_ascii_string initialPart{ vsDirParts[0].c_str() };
	if (vsDirParts.size() == 3 && initialPart == "Songs")
	{
		out.sSongDir = "/" + sDir;
	}
	else if (vsDirParts.size() == 2 && initialPart == "Themes")
	{
		out.sTheme = vsDirParts[1];
	}
}

static const std::string TEMP_ZIP_MOUNT_POINT = "/@temp-zip/";
const std::string TEMP_OS_MOUNT_POINT = "/@temp-os/";

static void InstallSmzip( const std::string &sZipFile, PlayAfterLaunchInfo &out )
{
	if( !FILEMAN->Mount( "zip", sZipFile, TEMP_ZIP_MOUNT_POINT ) )
	{
		FAIL_M("Failed to mount " + sZipFile );
	}
	vector<std::string> vsFiles;
	{
		vector<std::string> vsRawFiles;
		GetDirListingRecursive( TEMP_ZIP_MOUNT_POINT, "*", vsRawFiles);

		vector<std::string> vsPrettyFiles;
		Rage::ci_ascii_string ctl{ "ctl" };
		for (auto const &s: vsRawFiles)
		{
			if (ctl == GetExtension(s))
			{
				continue;
			}
			vsFiles.push_back(s);

			std::string s2{ Rage::tail(s, s.size() - TEMP_ZIP_MOUNT_POINT.size()) };
			vsPrettyFiles.push_back( s2 );
		}
		sort( vsPrettyFiles.begin(), vsPrettyFiles.end() );
	}

	std::string sResult = "Success installing " + sZipFile;
	for (auto sSrcFile: vsFiles)
	{
		std::string sDestFile = sSrcFile;
		sDestFile = Rage::tail(sDestFile, sDestFile.size() - TEMP_ZIP_MOUNT_POINT.size());
		
		std::string sDir, sThrowAway;
		splitpath( sDestFile, sDir, sThrowAway, sThrowAway );

		Parse( sDir, out );
		out.bAnySongChanged = true;

		FILEMAN->CreateDir( sDir );

		if( !FileCopy( sSrcFile, sDestFile ) )
		{
			sResult = "Error extracting " + sDestFile;
			break;
		}
	}
	FILEMAN->Unmount( "zip", sZipFile, TEMP_ZIP_MOUNT_POINT );

	SCREENMAN->SystemMessage( sResult );
}

void InstallSmzipOsArg( const std::string &sOsZipFile, PlayAfterLaunchInfo &out )
{
	SCREENMAN->SystemMessage("Installing " + sOsZipFile );

	std::string sOsDir, sFilename, sExt;
	splitpath( sOsZipFile, sOsDir, sFilename, sExt );

	if( !FILEMAN->Mount( "dir", sOsDir, TEMP_OS_MOUNT_POINT ) )
		FAIL_M("Failed to mount " + sOsDir );
	InstallSmzip( TEMP_OS_MOUNT_POINT + sFilename + sExt, out );

	FILEMAN->Unmount( "dir", sOsDir, TEMP_OS_MOUNT_POINT );
}

struct FileCopyResult
{
	FileCopyResult( std::string _sFile, std::string _sComment ) : sFile(_sFile), sComment(_sComment) {}
	std::string sFile, sComment;
};

#if !defined(WITHOUT_NETWORKING)
Preference<std::string> g_sCookie( "Cookie", "" );

class DownloadTask
{
	FileTransfer *m_pTransfer;
	vector<std::string> m_vsQueuedPackageUrls;
	std::string m_sCurrentPackageTempFile;
	enum
	{
		control,
		packages
	} m_DownloadState;
	PlayAfterLaunchInfo m_playAfterLaunchInfo;
public:
	DownloadTask(const std::string &sControlFileUri)
	{
		//SCREENMAN->SystemMessage( "Downloading control file." );
		m_pTransfer = new FileTransfer();
		m_pTransfer->StartDownload( sControlFileUri, "" );
		m_DownloadState = control;
	}
	~DownloadTask()
	{
		Rage::safe_delete(m_pTransfer);
	}
	std::string GetStatus()
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

				std::string sResponse = m_pTransfer->GetResponse();
				Rage::safe_delete( m_pTransfer );

				Json::Value root;
				std::string sError;
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
						for (auto iter: require)
						{
							if( iter["Dir"].isString() )
							{
								std::string sDir = iter["Dir"].asString();
								Parse( sDir, m_playAfterLaunchInfo );
								if( DoesFileExist( sDir ) )
									continue;
							}

							std::string sUri;
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
						vFailed.push_back( FileCopyResult(*s,fmt::sprintf("error copying file to '%s'",sOsDir.c_str())) );

				}
				*/
				m_DownloadState = packages;
				if( !m_vsQueuedPackageUrls.empty() )
				{
					std::string sUrl = m_vsQueuedPackageUrls.back();
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
					Rage::safe_delete( m_pTransfer );
					InstallSmzip( m_sCurrentPackageTempFile, m_playAfterLaunchInfo );
					FILEMAN->Remove( m_sCurrentPackageTempFile );	// Harmless if this fails because download didn't finish
				}
				if( !m_vsQueuedPackageUrls.empty() )
				{
					std::string sUrl = m_vsQueuedPackageUrls.back();
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
	static std::string MakeTempFileName( std::string s )
	{
		return SpecialFiles::CACHE_DIR + "Downloads/" + Rage::base_name(s);
	}
};
static vector<DownloadTask*> g_pDownloadTasks;
#endif

static bool IsStepManiaProtocol(const std::string &arg)
{
	// for now, only load from the StepMania domain until the security implications of this feature are better understood.
	//return Rage::starts_with(arg,"stepmania://beta.stepmania.com/");
	return Rage::starts_with(arg,"stepmania://");
}

static bool IsPackageFile(const std::string &arg)
{
	Rage::ci_ascii_string ext{ GetExtension(arg).c_str() };
	return ext == "smzip" || ext == "zip";
}

PlayAfterLaunchInfo DoInstalls( CommandLineActions::CommandLineArgs args )
{
	PlayAfterLaunchInfo ret;
	for (auto &s: args.argv)
	{
		if( IsStepManiaProtocol(s) )
		{
#if !defined(WITHOUT_NETWORKING)
			g_pDownloadTasks.push_back( new DownloadTask(s) );
#else
			// TODO: Figure out a meaningful log message.
#endif
		}
		else if( IsPackageFile(s) )
		{
			InstallSmzipOsArg(s, ret);
		}
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
			Rage::safe_delete(p);
			g_pDownloadTasks.erase( g_pDownloadTasks.begin()+i );
		}
	}

	{
		vector<std::string> vsMessages;
		for (auto *pDT: g_pDownloadTasks)
		{
			vsMessages.push_back( pDT->GetStatus() );
		}
		m_textStatus.SetText( Rage::join("\n", vsMessages) );
	}
#endif
	if( playAfterLaunchInfo.bAnySongChanged )
		SONGMAN->Reload( false, nullptr );

	if( !playAfterLaunchInfo.sSongDir.empty() )
	{
		Song* pSong = nullptr;
		GAMESTATE->Reset();
		std::string sInitialScreen;
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
			GAMESTATE->set_curr_song(pSong);
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
