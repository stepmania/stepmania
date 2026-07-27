#include "global.h"
#include "BGAnimation.h"
#include "IniFile.h"
#include "BGAnimationLayer.h"
#include "RageUtil.h"
#include "ActorUtil.h"

#include "LuaManager.h"
#include "PrefsManager.h"

#include <vector>


REGISTER_ACTOR_CLASS(BGAnimation);

BGAnimation::BGAnimation()
{
}

BGAnimation::~BGAnimation()
{
    DeleteAllChildren();
}

static bool CompareLayerNames( const RString& s1, const RString& s2 )
{
	int i1, i2;
	int ret;

	ret = sscanf( s1, "Layer%d", &i1 );
	ASSERT( ret == 1 );
	ret = sscanf( s2, "Layer%d", &i2 );
	ASSERT( ret == 1 );
	return i1 < i2;
}

void BGAnimation::AddLayersFromAniDir( const RString &_sAniDir, const XNode *pNode )
{
	const RString& sAniDir = _sAniDir;

	{
		std::vector<RString> vsLayerNames;
		FOREACH_CONST_Child( pNode, pLayer )
		{
			if( strncmp(pLayer->GetName(), "Layer", 5) == 0 )
				vsLayerNames.push_back( pLayer->GetName() );
		}

		sort( vsLayerNames.begin(), vsLayerNames.end(), CompareLayerNames );


		for (RString const &sLayer : vsLayerNames)
		{
			const XNode* pKey = pNode->GetChild( sLayer );
			ASSERT( pKey != nullptr );

			RString sImportDir;
			if( pKey->GetAttrValue("Import", sImportDir) )
			{
				bool bCond;
				if( pKey->GetAttrValue("Condition",bCond) && !bCond )
					continue;

				// import a whole BGAnimation
				sImportDir = sAniDir + sImportDir;
				CollapsePath( sImportDir );

				if( sImportDir.Right(1) != "/" )
					sImportDir += "/";

				ASSERT_M( IsADirectory(sImportDir), sImportDir + " isn't a directory" );

				RString sPathToIni = sImportDir + "BGAnimation.ini";

				IniFile ini2;
				ini2.ReadFile( sPathToIni );

				AddLayersFromAniDir( sImportDir, &ini2 );
			}
			else
			{
				// import as a single layer
				BGAnimationLayer* bgLayer = new BGAnimationLayer;
				bgLayer->LoadFromNode( pKey );
				this->AddChild( bgLayer );
			}
		}
	}
}

void BGAnimation::LoadFromAniDir( const RString &_sAniDir )
{
	DeleteAllChildren();

	if( _sAniDir.empty() )
		 return;

	RString sAniDir = _sAniDir;
	if( sAniDir.Right(1) != "/" )
		sAniDir += "/";

	ASSERT_M( IsADirectory(sAniDir), sAniDir + " isn't a directory" );

	RString sPathToIni = sAniDir + "BGAnimation.ini";

	if( DoesFileExist(sPathToIni) )
	{
		if( PREFSMAN->m_bQuirksMode )
		{
			// This is a 3.9-style BGAnimation (using .ini)
			IniFile ini;
			ini.ReadFile( sPathToIni );

			AddLayersFromAniDir( sAniDir, &ini ); // TODO: Check for circular load

			XNode* pBGAnimation = ini.GetChild( "BGAnimation" );
			XNode dummy( "BGAnimation" );
			if( pBGAnimation == nullptr )
				pBGAnimation = &dummy;

			LoadFromNode( pBGAnimation );
		}
		else // We don't officially support .ini files anymore.
		{
			XNode dummy( "BGAnimation" );
			XNode *pBG = &dummy;
			LoadFromNode( pBG );
		}
	}
	else
	{
		// This is an 3.0 and before-style BGAnimation (not using .ini)

		// loading a directory of layers
		std::vector<RString> asImagePaths;
		ASSERT( sAniDir != "" );

		GetDirListing( sAniDir+"*.png", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.jpg", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.jpeg", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.gif", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.ogv", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.avi", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.mpg", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.mpeg", asImagePaths, false, true );

		SortRStringArray( asImagePaths );

		for( unsigned i=0; i<asImagePaths.size(); i++ )
		{
			const RString sPath = asImagePaths[i];
			if( Basename(sPath).Left(1) == "_" )
				continue; // don't directly load files starting with an underscore
			BGAnimationLayer* pLayer = new BGAnimationLayer;
			pLayer->LoadFromAniLayerFile( asImagePaths[i] );
			AddChild( pLayer );
		}
	}
}

void BGAnimation::LoadFromNode( const XNode* pNode )
{
	RString sDir;
	if( pNode->GetAttrValue("AniDir", sDir) )
		LoadFromAniDir( sDir );

	ActorFrame::LoadFromNode( pNode );

	/* Backwards-compatibility: if a "LengthSeconds" value is present, create a dummy
	 * actor that sleeps for the given length of time. This will extend GetTweenTimeLeft. */
	float fLengthSeconds = 0;
	if( pNode->GetAttrValue( "LengthSeconds", fLengthSeconds ) )
	{
		Actor *pActor = new Actor;
		pActor->SetName( "BGAnimation dummy" );
		pActor->SetVisible( false );
		apActorCommands ap = ActorUtil::ParseActorCommands( ssprintf("sleep,%f",fLengthSeconds) );
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
