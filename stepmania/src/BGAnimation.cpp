#include "global.h"
#include "BGAnimation.h"
#include "IniFile.h"
#include "BGAnimationLayer.h"
#include "RageUtil.h"
#include "ActorUtil.h"
#include "LuaHelpers.h"
#include "Foreach.h"


BGAnimation::BGAnimation()
{
}

BGAnimation::~BGAnimation()
{
	Unload();
}

void BGAnimation::Unload()
{
    DeleteAllChildren();
}

static bool CompareLayerNames( const CString& s1, const CString& s2 )
{
	int i1, i2;
	int ret;

	ret = sscanf( s1, "Layer%d", &i1 );
	ASSERT( ret == 1 );
	ret = sscanf( s2, "Layer%d", &i2 );
	ASSERT( ret == 1 );
	return i1 < i2;
}

void BGAnimation::AddLayersFromAniDir( const CString &_sAniDir, vector<Actor*> &layersAddTo, bool bGeneric )
{
	if( _sAniDir.empty() )
		 return;

	CString sAniDir = _sAniDir;
	if( sAniDir.Right(1) != "/" )
		sAniDir += "/";

	ASSERT_M( IsADirectory(sAniDir), sAniDir + " isn't a directory" );

	CString sPathToIni = sAniDir + "BGAnimation.ini";

	IniFile ini;
	ini.ReadFile( sPathToIni );

	{
		CString expr;
		if( ini.GetValue( "BGAnimation", "Condition", expr ) || ini.GetValue( "BGAnimation", "Cond", expr ) )
		{
			if( !Lua::RunExpressionB( expr ) )
				return;
		}
	}

	{
		vector<CString> vsLayerNames;
		FOREACH_CONST_Child( &ini, pLayer )
		{
			if( strncmp(pLayer->m_sName, "Layer", 5) == 0 )
				vsLayerNames.push_back( pLayer->m_sName );
		}

		sort( vsLayerNames.begin(), vsLayerNames.end(), CompareLayerNames );


		FOREACH_CONST( CString, vsLayerNames, s )
		{
			const CString &sLayer = *s;
			const XNode* pKey = ini.GetChild( sLayer );
			ASSERT( pKey );

			CString sImportDir;
			if( pKey->GetAttrValue("Import", sImportDir) )
			{
				CString expr;
				if( pKey->GetAttrValue("Condition",expr) )
				{
					if( !Lua::RunExpressionB( expr ) )
						continue;
				}

				// import a whole BGAnimation
				sImportDir = sAniDir + sImportDir;
				CollapsePath( sImportDir );

				AddLayersFromAniDir( sImportDir, layersAddTo, bGeneric );
			}
			else
			{
				// import as a single layer
				BGAnimationLayer* pLayer = new BGAnimationLayer( bGeneric );
				pLayer->LoadFromNode( sAniDir, *pKey );
				layersAddTo.push_back( pLayer );
			}
		}
	}
}

void BGAnimation::LoadFromAniDir( const CString &_sAniDir, bool bGeneric )
{
	Unload();

	if( _sAniDir.empty() )
		 return;

	CString sAniDir = _sAniDir;
	if( sAniDir.Right(1) != "/" )
		sAniDir += "/";

	ASSERT_M( IsADirectory(sAniDir), sAniDir + " isn't a directory" );

	CString sPathToIni = sAniDir + "BGAnimation.ini";

	if( DoesFileExist(sPathToIni) )
	{
		// This is a new style BGAnimation (using .ini)

		IniFile ini;
		ini.ReadFile( sPathToIni );

		AddLayersFromAniDir( sAniDir, m_SubActors, bGeneric );	// TODO: Check for circular load

		XNode* pBGAnimation = ini.GetChild( "BGAnimation" );
		XNode dummy;
		dummy.m_sName = "BGAnimation";
		if( pBGAnimation == NULL )
			pBGAnimation = &dummy;

		// Ugly: Scroller attributes in BGAnimation.ini files are in an element called 
		// "Scroller", and not in the "BGAnimation" element.  Move the attributes from 
		// Scroller to BGAnimation.
		XNode* pScrollerNode = ini.GetChild( "Scroller" );
		if( pScrollerNode )
		{
			FOREACH_Attr( pScrollerNode, pAttr )
				pBGAnimation->m_attrs.insert( pair<CString,XAttr*>(pAttr->m_sName, pAttr) );
			// Clear the copies in the Scroller node so that we don't double-delete.
			pScrollerNode->m_attrs.clear();	
		}

		LoadFromNode( sAniDir, *pBGAnimation );

	}
	else
	{
		// This is an old style BGAnimation (not using .ini)

		// loading a directory of layers
		CStringArray asImagePaths;
		ASSERT( sAniDir != "" );

		GetDirListing( sAniDir+"*.png", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.jpg", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.gif", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.avi", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.mpg", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.mpeg", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.sprite", asImagePaths, false, true );

		SortCStringArray( asImagePaths );

		for( unsigned i=0; i<asImagePaths.size(); i++ )
		{
			const CString sPath = asImagePaths[i];
			if( Basename(sPath).Left(1) == "_" )
				continue;	// don't directly load files starting with an underscore
			BGAnimationLayer* pLayer = new BGAnimationLayer( bGeneric );
			pLayer->LoadFromAniLayerFile( asImagePaths[i] );
			AddChild( pLayer );
		}
	}
}

void BGAnimation::LoadFromNode( const CString &sDir, const XNode& node )
{
	DEBUG_ASSERT( node.m_sName == "BGAnimation" );

	CString sInitCommand;
	if( node.GetAttrValue( "InitCommand", sInitCommand ) )
	{
		/* There's an InitCommand.  Run it now.  This can be used to eg. change Z to
		 * modify draw order between BGAs in a Foreground.  Most things should be done
		 * in metrics.ini commands, not here. */
		this->RunCommands( ParseCommands(sInitCommand) );
	}

	ActorScroller::LoadFromNode( sDir, &node );

	Command cmd;
	cmd.Load( "PlayCommand,Init" );
	this->RunCommandOnChildren( cmd );

	/* Backwards-compatibility: if a "LengthSeconds" value is present, create a dummy
	 * actor that sleeps for the given length of time.  This will extend GetTweenTimeLeft. */
	float fLengthSeconds = 0;
	if( node.GetAttrValue( "LengthSeconds", fLengthSeconds ) )
	{
		Actor *pActor = new Actor;
		pActor->SetHidden( true );
		pActor->BeginTweening( fLengthSeconds );
		AddChild( pActor );
	}
}

/*
 * (c) 2001-2004 Ben Nordstrom, Chris Danford
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
