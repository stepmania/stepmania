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
#include "IniFile.h"
#include "ThemeManager.h"
#include "RageUtil_FileDB.h"


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

Actor* MakeActor( CString sPath )
{
	CString sExt = GetExtension(sPath);
	sExt.MakeLower();
	

	if( sExt=="actor" )
	{
		return LoadActor( sPath );
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
		pSprite->Load( sPath );
		return pSprite;
	}
	else if( sExt=="txt" )
	{
		Model* pModel = new Model;
		pModel->LoadMilkshapeAscii( sPath );
		return pModel;
	}

	RageException::Throw("File \"%s\" has unknown type, \"%s\"",
		sPath.c_str(), sExt.c_str() );
}

