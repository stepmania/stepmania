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
#include "RageUtil_FileDB.h"
#include "RageDisplay.h"
#include "RageLog.h"
#include "arch/ArchHooks/ArchHooks.h"


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
	
	CString sNewPath = Dirname( sPath ) + sFileName;

	sNewPath = DerefRedir( sNewPath );

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
		if( !DoesFileExist(sNewPath) )
			RageException::Throw( "The actor file '%s' references a file '%s' which doesn't exist.", sPath.c_str(), sNewPath.c_str() );

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

void IncorrectActorParametersWarning( const CStringArray &asTokens, int iMaxIndexAccessed, int size )
{
	const CString sError = ssprintf( "Actor::HandleCommand: Wrong number of parameters in command '%s'.  Expected %d but there are %d.",
		join(",",asTokens).c_str(), iMaxIndexAccessed+1, size );
	LOG->Warn( sError );
	if( DISPLAY->IsWindowed() )
		HOOKS->MessageBoxOK( sError );
}

void UtilSetXY( Actor& actor, CString sClassName )
{
	ASSERT( !actor.GetID().empty() );
	actor.SetXY( THEME->GetMetricF(sClassName,actor.GetID()+"X"), THEME->GetMetricF(sClassName,actor.GetID()+"Y") );
}

float UtilCommand( Actor& actor, CString sClassName, CString sCommandName )
{
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
		ASSERT( !actor.GetID().empty() );
	}

	return max( ret, actor.Command( THEME->GetMetric(sClassName,actor.GetID()+sCommandName+"Command") ) );
}
