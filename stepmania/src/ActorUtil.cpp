#include "global.h"	// testing updates

/*
-----------------------------------------------------------------------------
 Class: ActorUtil

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorUtil.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "Model.h"
#include "BGAnimation.h"
#include "IniFile.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "RageLog.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "RageFileManager.h"
#include "SongCreditDisplay.h"
#include "song.h"
#include "GameState.h"
#include "RageTextureManager.h"
#include "SongManager.h"
#include "Course.h"


Actor* LoadFromActorFile( CString sIniPath, CString sLayer )
{
	// TODO: Check for recursive loading
	IniFile ini;
	ini.SetPath( sIniPath );
	ini.ReadFile();
	
	if( !ini.GetKey(sLayer) )
		RageException::Throw( "The file '%s' doesn't have layer '%s'.", sIniPath.c_str(), sLayer.c_str() );

	Actor* pActor = NULL;	// fill this in before we return;

	CString sType;
	ini.GetValue( sLayer, "Type", sType );
	CString sFile;
	ini.GetValue( sLayer, "File", sFile );
	CString sDir = Dirname( sIniPath );

	if( sType == "SongCreditDisplay" )
	{
		pActor = new SongCreditDisplay;
	}
	else
	{

		/* XXX: How to handle translations?  Maybe we should have one metrics section,
		 * "Text", eg:
		 *
		 * [Text]
		 * SoundVolume=Sound Volume
		 * TextItem=Hello
		 *
		 * and allow "$TextItem$" in .actors to reference that.
		 */
		CString text;
		if( ini.GetValue ( sLayer, "Text", text ) )
		{
			/* It's a BitmapText. Note that we could do the actual text setting with metrics,
			 * by adding "text" and "alttext" commands, but right now metrics can't contain
			 * commas or semicolons.  It's useful to be able to refer to fonts in the real
			 * theme font dirs, too. */
			CString alttext;
			ini.GetValue ( sLayer, "AltText", alttext );
			text.Replace( "::", "\n" );
			alttext.Replace( "::", "\n" );

			BitmapText* pBitmapText = new BitmapText;

			pBitmapText->LoadFromFont( THEME->GetPathToF( sFile ) );
			pBitmapText->SetText( text, alttext );
			pActor = pBitmapText;
		}
		else
		{

			if( sFile.CompareNoCase("songbackground")==0 )
			{
				Song *pSong = GAMESTATE->m_pCurSong;
				if( pSong && pSong->HasBackground() )
					sFile = pSong->GetBackgroundPath();
				else
					sFile = THEME->GetPathToG("Common fallback background");

				pActor = MakeActor( sFile );
			}
			else if( sFile.CompareNoCase("songbanner")==0 )
			{
				Song *pSong = GAMESTATE->m_pCurSong;
				if( pSong == NULL )
					pSong = SONGMAN->GetRandomSong();

				if( pSong && pSong->HasBanner() )
					sFile = pSong->GetBannerPath();
				else
					sFile = THEME->GetPathToG("Common fallback banner");

				TEXTUREMAN->DisableOddDimensionWarning();
				pActor = MakeActor( sFile );
				TEXTUREMAN->EnableOddDimensionWarning();
			}
			else if( sFile.CompareNoCase("coursebanner")==0 )
			{
				Course *pCourse = GAMESTATE->m_pCurCourse;
				if( pCourse == NULL )
					pCourse = SONGMAN->GetRandomCourse();

				if( pCourse && pCourse->HasBanner() )
					sFile = pCourse->m_sBannerPath;
				else
					sFile = THEME->GetPathToG("Common fallback banner");

				TEXTUREMAN->DisableOddDimensionWarning();
				pActor = MakeActor( sFile );
				TEXTUREMAN->EnableOddDimensionWarning();
			}
			else if( IsAFile(sDir+sFile) || IsADirectory(sDir+sFile) )
			{
				// If we know this is an exact match, don't bother with the
				// GetDirListing below.  The GetDirListing is causing problems with 
				// partial matching BGAnimation directory names.
				CString sNewPath = DerefRedir( sDir+sFile );
				pActor = MakeActor( sNewPath );
			}
			else
			{
				CStringArray asPaths;
				GetDirListing( sDir + sFile + "*", asPaths, false, true );	// return path too

				if( asPaths.empty() )
					RageException::Throw( "The actor file '%s' references a file '%s' which doesn't exist.", sIniPath.c_str(), sFile.c_str() );
				else if( asPaths.size() > 1 )
					RageException::Throw( "The actor file '%s' references a file '%s' which has multiple matches.", sIniPath.c_str(), sFile.c_str() );

				CString sNewPath = DerefRedir( asPaths[0] );
				pActor = MakeActor( sNewPath );
			}
		}
	}

	float f;
	if( ini.GetValue ( sLayer, "BaseRotationXDegrees", f ) )	pActor->SetBaseRotationX( f );
	if( ini.GetValue ( sLayer, "BaseRotationYDegrees", f ) )	pActor->SetBaseRotationY( f );
	if( ini.GetValue ( sLayer, "BaseRotationZDegrees", f ) )	pActor->SetBaseRotationZ( f );
	if( ini.GetValue ( sLayer, "BaseZoomX", f ) )				pActor->SetBaseZoomX( f );
	if( ini.GetValue ( sLayer, "BaseZoomY", f ) )				pActor->SetBaseZoomY( f );
	if( ini.GetValue ( sLayer, "BaseZoomZ", f ) )				pActor->SetBaseZoomZ( f );

	ASSERT( pActor );	// we should have filled this in above
	return pActor;
}

Actor* MakeActor( RageTextureID ID )
{
	CString sExt = GetExtension( ID.filename );
	sExt.MakeLower();
	
	if( sExt=="actor" )
	{
		return LoadFromActorFile( ID.filename );
	}
	else if( sExt=="png" ||
		sExt=="jpg" || 
		sExt=="gif" || 
		sExt=="bmp" || 
		sExt=="avi" || 
		sExt=="mpeg" || 
		sExt=="mpg" ||
		sExt=="sprite" )
	{
		Sprite* pSprite = new Sprite;
		pSprite->Load( ID );
		return pSprite;
	}
	else if( sExt=="txt" )
	{
		Model* pModel = new Model;
		pModel->LoadMilkshapeAscii( ID.filename );
		return pModel;
	}
	/* Do this last, to avoid the IsADirectory in most cases. */
	else if( IsADirectory(ID.filename)  )
	{
		BGAnimation *pBGA = new BGAnimation( true );
		pBGA->LoadFromAniDir( ID.filename );
		return pBGA;
	}
	else 
	RageException::Throw("File \"%s\" has unknown type, \"%s\"",
		ID.filename.c_str(), sExt.c_str() );
}

void UtilSetXY( Actor& actor, CString sClassName )
{
	ASSERT( !actor.GetID().empty() );
	actor.SetXY( THEME->GetMetricF(sClassName,actor.GetID()+"X"), THEME->GetMetricF(sClassName,actor.GetID()+"Y") );
}

void UtilCommand( Actor& actor, CString sClassName, CString sCommandName )
{
	// If Actor is hidden, it won't get updated or drawn, so don't bother tweening.
	/* ... but we might be unhiding it, or setting state for when we unhide it later */
//	if( actor.GetHidden() )
//		return 0;

	actor.Command( "playcommand," + sCommandName );

	// HACK:  It's very often that we command things to TweenOffScreen 
	// that we aren't drawing.  We know that an Actor is not being
	// used if its name is blank.  So, do nothing on Actors with a blank name.
	// (Do "playcommand" anyway; BGAs often have no name.)
	if( sCommandName=="Off" )
	{
		if( actor.GetID().empty() )
			return;
	} else {
		RAGE_ASSERT_M( !actor.GetID().empty(), ssprintf("!actor.GetID().empty() ('%s', '%s')", sClassName.c_str(), sCommandName.c_str()) );
	}

	actor.Command( THEME->GetMetric(sClassName,actor.GetID()+sCommandName+"Command") );
}

