#include "global.h"
#include "BGAnimation.h"
#include "IniFile.h"
#include "BGAnimationLayer.h"
#include "RageUtil.h"
#include "ActorUtil.h"
#include "Foreach.h"
#include "Command.h"
#include "LuaManager.h"

BGAnimation::BGAnimation()
{
}

BGAnimation::~BGAnimation()
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

void BGAnimation::AddLayersFromAniDir( const CString &_sAniDir, const XNode *pNode )
{
	const CString& sAniDir = _sAniDir;

	{
		vector<CString> vsLayerNames;
		FOREACH_CONST_Child( pNode, pLayer )
		{
			if( strncmp(pLayer->m_sName, "Layer", 5) == 0 )
				vsLayerNames.push_back( pLayer->m_sName );
		}

		sort( vsLayerNames.begin(), vsLayerNames.end(), CompareLayerNames );


		FOREACH_CONST( CString, vsLayerNames, s )
		{
			const CString &sLayer = *s;
			const XNode* pKey = pNode->GetChild( sLayer );
			ASSERT( pKey );

			CString sImportDir;
			if( pKey->GetAttrValue("Import", sImportDir) )
			{
				CString expr;
				if( pKey->GetAttrValue("Condition",expr) )
				{
					if( !LuaHelpers::RunExpressionB( expr ) )
						continue;
				}

				// import a whole BGAnimation
				sImportDir = sAniDir + sImportDir;
				CollapsePath( sImportDir );

				if( sImportDir.Right(1) != "/" )
					sImportDir += "/";

				ASSERT_M( IsADirectory(sImportDir), sImportDir + " isn't a directory" );

				CString sPathToIni = sImportDir + "BGAnimation.ini";

				IniFile ini2;
				ini2.ReadFile( sPathToIni );

				AddLayersFromAniDir( sImportDir, &ini2 );
			}
			else
			{
				// import as a single layer
				BGAnimationLayer* pLayer = new BGAnimationLayer;
				pLayer->LoadFromNode( sAniDir, pKey );
				this->AddChild( pLayer );
			}
		}
	}
}

void BGAnimation::LoadFromAniDir( const CString &_sAniDir )
{
	DeleteAllChildren();

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

		AddLayersFromAniDir( sAniDir, &ini );	// TODO: Check for circular load

		XNode* pBGAnimation = ini.GetChild( "BGAnimation" );
		XNode dummy;
		dummy.m_sName = "BGAnimation";
		if( pBGAnimation == NULL )
			pBGAnimation = &dummy;

		LoadFromNode( sAniDir, pBGAnimation );
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

		SortCStringArray( asImagePaths );

		for( unsigned i=0; i<asImagePaths.size(); i++ )
		{
			const CString sPath = asImagePaths[i];
			if( Basename(sPath).Left(1) == "_" )
				continue;	// don't directly load files starting with an underscore
			BGAnimationLayer* pLayer = new BGAnimationLayer;
			pLayer->LoadFromAniLayerFile( asImagePaths[i] );
			AddChild( pLayer );
		}
	}
}

void BGAnimation::LoadFromNode( const CString& sDir, const XNode* pNode )
{
	ActorFrame::LoadFromNode( sDir, pNode );

	/* Backwards-compatibility: if a "LengthSeconds" value is present, create a dummy
	 * actor that sleeps for the given length of time.  This will extend GetTweenTimeLeft. */
	float fLengthSeconds = 0;
	if( pNode->GetAttrValue( "LengthSeconds", fLengthSeconds ) )
	{
		Actor *pActor = new Actor;
		pActor->SetName( "BGAnimation dummy" );
		pActor->SetHidden( true );
		apActorCommands ap( new ActorCommands(ssprintf("sleep,%f",fLengthSeconds)) );
		pActor->AddCommand( "On", ap );
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
