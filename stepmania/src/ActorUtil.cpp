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


static Actor* LoadActor( CString sPath )
{
	// TODO: Check for recursive loading
	IniFile ini;
	ini.SetPath( sPath );
	ini.ReadFile();
	
	if( !ini.GetKey("Actor") )
		RageException::Throw( "The actor file '%s' is invalid.", sPath.c_str() );

	CString sFileName;
	ini.GetValue( "Actor", "File", sFileName );
	
	CString sDir = Dirname( sPath );

	/* XXX: How to handle translations?  Maybe we should have one metrics section,
	 * "Text", eg:
	 *
	 * [Text]
	 * SoundVolume=Sound Volume
	 * TextItem=Hello
	 *
	 * and allow "$TextItem$" in .actors to reference that.
	 */
	Actor* pActor = NULL;
	CString text;
	if( ini.GetValue ( "Actor", "Text", text ) )
	{
		/* It's a BitmapText. Note that we could do the actual text setting with metrics,
		 * by adding "text" and "alttext" commands, but right now metrics can't contain
		 * commas or semicolons.  It's useful to be able to refer to fonts in the real
		 * theme font dirs, too. */
		CString alttext;
		ini.GetValue ( "Actor", "AltText", alttext );
		text.Replace( "::", "\n" );
		alttext.Replace( "::", "\n" );

		BitmapText* pBitmapText = new BitmapText;
		pActor = pBitmapText;

		pBitmapText->LoadFromFont( THEME->GetPathToF( sFileName ) );
		pBitmapText->SetText( text, alttext );
	}
	else
	{
		CStringArray asFiles;
		GetDirListing( sDir + sFileName + "*", asFiles );

		if( asFiles.empty() )
			RageException::Throw( "The actor file '%s' references a file '%s' which doesn't exist.", sPath.c_str(), sFileName.c_str() );
		else if( asFiles.size() > 1 )
			RageException::Throw( "The actor file '%s' references a file '%s' which has multiple matches.", sPath.c_str(), sFileName.c_str() );

		CString sNewPath = DerefRedir( sDir + asFiles[0] );

		pActor = MakeActor( sNewPath );
	}

	float f;
	if( ini.GetValue ( "Actor", "BaseRotationXDegrees", f ) )	pActor->SetBaseRotationX( f );
	if( ini.GetValue ( "Actor", "BaseRotationYDegrees", f ) )	pActor->SetBaseRotationY( f );
	if( ini.GetValue ( "Actor", "BaseRotationZDegrees", f ) )	pActor->SetBaseRotationZ( f );
	if( ini.GetValue ( "Actor", "BaseZoomX", f ) )				pActor->SetBaseZoomX( f );
	if( ini.GetValue ( "Actor", "BaseZoomY", f ) )				pActor->SetBaseZoomY( f );
	if( ini.GetValue ( "Actor", "BaseZoomZ", f ) )				pActor->SetBaseZoomZ( f );

	return pActor;
}

Actor* MakeActor( RageTextureID ID )
{
	CString sExt = GetExtension( ID.filename );
	sExt.MakeLower();
	
	if( sExt=="actor" )
	{
		return LoadActor( ID.filename );
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

float UtilCommand( Actor& actor, CString sClassName, CString sCommandName )
{
	// If Actor is hidden, it won't get updated or drawn, so don't bother tweening.
	if( actor.GetHidden() )
		return 0;

	float ret = actor.Command( "playcommand," + sCommandName );

	// HACK:  It's very often that we command things to TweenOffScreen 
	// that we aren't drawing.  We know that an Actor is not being
	// used if its name is blank.  So, do nothing on Actors with a blank name.
	// (Do "playcommand" anyway; BGAs often have no name.)
	if( sCommandName=="Off" )
	{
		if( actor.GetID().empty() )
			return ret;
	} else {
		RAGE_ASSERT_M( !actor.GetID().empty(), ssprintf("!actor.GetID().empty() ('%s', '%s')", sClassName.c_str(), sCommandName.c_str()) );
	}

	return max( ret, actor.Command( THEME->GetMetric(sClassName,actor.GetID()+sCommandName+"Command") ) );
}

