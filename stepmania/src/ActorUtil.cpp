#include "global.h"
#include "ActorUtil.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "Model.h"
#include "BGAnimation.h"
#include "IniFile.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "song.h"
#include "GameState.h"
#include "RageTextureManager.h"
#include "SongManager.h"
#include "Course.h"
#include "XmlFile.h"
#include "FontCharAliases.h"
#include "LuaManager.h"
#include "MessageManager.h"

#include "arch/Dialog/Dialog.h"


// Actor registration
static map<CString,CreateActorFn>	*g_pmapRegistrees = NULL;

void ActorUtil::Register( const CString& sClassName, CreateActorFn pfn )
{
	if( g_pmapRegistrees == NULL )
		g_pmapRegistrees = new map<CString,CreateActorFn>;

	map<CString,CreateActorFn>::iterator iter = g_pmapRegistrees->find( sClassName );
	ASSERT_M( iter == g_pmapRegistrees->end(), ssprintf("Actor class '%s' already registered.", sClassName.c_str()) );

	(*g_pmapRegistrees)[sClassName] = pfn;
}

Actor* ActorUtil::Create( const CString& sClassName, const CString& sDir, const XNode* pNode )
{
	map<CString,CreateActorFn>::iterator iter = g_pmapRegistrees->find( sClassName );
	ASSERT_M( iter != g_pmapRegistrees->end(), ssprintf("Actor '%s' is not registered.",sClassName.c_str()) )

	CreateActorFn pfn = iter->second;
	return pfn( sDir, pNode );
}



Actor* ActorUtil::LoadFromActorFile( const CString& sAniDir, const XNode* pNode )
{
	ASSERT( pNode );

	{
		CString expr;
		if( pNode->GetAttrValue("Condition",expr) )
		{
			if( !LUA->RunExpressionB(expr) )
				return NULL;
		}
	}


	Actor* pActor = NULL;	// fill this in before we return

	// Element name is the type in XML.
	// Type= is the name in INI.
	CString sType = pNode->m_sName;
	bool bHasType = pNode->GetAttrValue( "Type", sType );

	CString sFile;
	pNode->GetAttrValue( "File", sFile );
	FixSlashesInPlace( sFile );

	CString sText;
	bool bHasText = pNode->GetAttrValue( "Text", sText );

	//
	// backward compatibility hacks
	//
	if( bHasText && !bHasType )
		sType = "BitmapText";
	else if( sFile.CompareNoCase("songbackground") == 0 )
		sType = "SongBackground";
	else if( sFile.CompareNoCase("songbanner") == 0 )
		sType = "SongBanner";
	else if( sFile.CompareNoCase("coursebanner") == 0 )
		sType = "CourseBanner";


	if( sType == "BGAnimation" )
	{
		BGAnimation *p = new BGAnimation;
		p->LoadFromNode( sAniDir, pNode );
		pActor = p;
	}
	else if( 
		sType == "GenreDisplay" ||
		sType == "RollingNumbers" ||
		sType == "ScoreDisplayCalories" )
	{
		pActor = ActorUtil::Create( sType, sAniDir, pNode );
	}
	else if( sType == "ActorFrame" )
	{
		ActorFrame *p = new ActorFrame;
		p->LoadFromNode( sAniDir, pNode );
		pActor = p;
	}
	else if( sType == "BitmapText" )
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
		/* It's a BitmapText. Note that we could do the actual text setting with metrics,
		 * by adding "text" and "alttext" commands, but right now metrics can't contain
		 * commas or semicolons.  It's useful to be able to refer to fonts in the real
		 * theme font dirs, too. */
		CString sAlttext;
		pNode->GetAttrValue("AltText", sAlttext );

		ThemeManager::EvaluateString( sText );
		ThemeManager::EvaluateString( sAlttext );
		
		CString sFont = sFile;	// accept "File" for backward compatibility
		pNode->GetAttrValue("Font", sFont );

		BitmapText* pBitmapText = new BitmapText;

		/* Be careful: if sFile is "", and we don't check it, then we can end up recursively
		 * loading the BGAnimationLayer that we're in. */
		if( sFont == "" )
			RageException::Throw( "A BitmapText in '%s' is missing the Font attribute",
				sAniDir.c_str() );

		pBitmapText->LoadFromFont( THEME->GetPathToF( sFont ) );
		pBitmapText->SetText( sText, sAlttext );
		pActor = pBitmapText;
	}
	else if( sType == "SongBackground" )
	{
		Song *pSong = GAMESTATE->m_pCurSong;
		if( pSong && pSong->HasBackground() )
			sFile = pSong->GetBackgroundPath();
		else
			sFile = THEME->GetPathG("Common","fallback background");

		/* Always load song backgrounds with SongBGTexture.  It sets texture properties;
		 * if we load a background without setting those properties, we'll end up
		 * with duplicates. */
		Sprite* pSprite = new Sprite;
		pSprite->LoadBG( sFile );
		pActor = pSprite;
	}
	else if( sType == "SongBanner" )
	{
		Song *pSong = GAMESTATE->m_pCurSong;
		if( pSong == NULL )
		{
			// probe for a random banner
			for( int i=0; i<300; i++ )
			{
				pSong = SONGMAN->GetRandomSong();
				if( pSong == NULL )
					break;
				if( !pSong->ShowInDemonstrationAndRanking() )
					continue;
				break;
			}
		}

		if( pSong && pSong->HasBanner() )
			sFile = pSong->GetBannerPath();
		else
			sFile = THEME->GetPathG("Common","fallback banner");

		TEXTUREMAN->DisableOddDimensionWarning();

		/* Always load banners with BannerTex.  It sets texture properties;
		 * if we load a background without setting those properties, we'll end up
		 * with duplicates. */
		Sprite* pSprite = new Sprite;
		pSprite->Load( Sprite::SongBannerTexture(sFile) );
		pActor = pSprite;

		TEXTUREMAN->EnableOddDimensionWarning();
	}
	else if( sType == "CourseBanner" )
	{
		Course *pCourse = GAMESTATE->m_pCurCourse;
		if( pCourse == NULL )
		{
			// probe for a random banner
			for( int i=0; i<300; i++ )
			{
				pCourse = SONGMAN->GetRandomCourse();
				if( pCourse == NULL )
					break;
				if( !pCourse->ShowInDemonstrationAndRanking() )
					continue;
				if( pCourse->m_bIsAutogen )
					continue;
				break;
			}
		}

		if( pCourse && pCourse->HasBanner() )
			sFile = pCourse->m_sBannerPath;
		else
			sFile = THEME->GetPathG("Common","fallback banner");

		TEXTUREMAN->DisableOddDimensionWarning();
		Sprite* pSprite = new Sprite;
		pSprite->Load( Sprite::SongBannerTexture(sFile) );
		pActor = pSprite;
		TEXTUREMAN->EnableOddDimensionWarning();
	}
	else // sType is empty or garbage (e.g. "1" // 0==Sprite")
	{
		// automatically figure out the type
retry:
		/* Be careful: if sFile is "", and we don't check it, then we can end up recursively
		 * loading the BGAnimationLayer that we're in. */
		if( sFile == "" )
			RageException::Throw( "The actor file in '%s' is missing the File attribute",
				sAniDir.c_str() );

		/* XXX: We need to do a theme search, since the file we're loading might
		 * be overridden by the theme. */
		CString sNewPath = sAniDir+sFile;
		CollapsePath( sNewPath );

		// If we know this is an exact match, don't bother with the GetDirListing;
		// it's causing problems with partial matching BGAnimation directory names.
		if( !IsAFile(sNewPath) && !IsADirectory(sNewPath) )
		{
			CStringArray asPaths;
			GetDirListing( sNewPath + "*", asPaths, false, true );	// return path too

			if( asPaths.empty() )
			{
				CString sError = ssprintf( "The actor file in '%s' references a file '%s' which doesn't exist.", sAniDir.c_str(), sFile.c_str() );
				switch( Dialog::AbortRetryIgnore( sError, "BROKEN_ACTOR_REFERENCE" ) )
				{
				case Dialog::abort:
					RageException::Throw( sError ); 
					break;
				case Dialog::retry:
					FlushDirCache();
					goto retry;
				case Dialog::ignore:
					asPaths.push_back( sNewPath );
					if( GetExtension(asPaths[0]) == "" )
						asPaths[0] = SetExtension( asPaths[0], "png" );
					break;
				default:
					ASSERT(0);
				}
			}
			else if( asPaths.size() > 1 )
			{
				CString sError = ssprintf( "The actor file in '%s' references a file '%s' which has multiple matches.", sAniDir.c_str(), sFile.c_str() );
				switch( Dialog::AbortRetryIgnore( sError, "DUPLICATE_ACTOR_REFERENCE" ) )
				{
				case Dialog::abort:
					RageException::Throw( sError ); 
					break;
				case Dialog::retry:
					FlushDirCache();
					goto retry;
				case Dialog::ignore:
					asPaths.erase( asPaths.begin()+1, asPaths.end() );
					break;
				default:
					ASSERT(0);
				}
			}

			sNewPath = asPaths[0];
		}

		sNewPath = DerefRedir( sNewPath );

		pActor = ActorUtil::MakeActor( sNewPath );
		if( pActor == NULL )
			return NULL;
	}

	ASSERT( pActor );	// we should have filled this in above

	// TODO: LoadFromNode should be called when we still have a pointer to the derived type.
 	pActor->LoadFromNode( sAniDir, pNode );

	return pActor;
}

Actor* ActorUtil::MakeActor( const RageTextureID &ID )
{
	CString sExt = GetExtension( ID.filename );
	sExt.MakeLower();
	
	if( sExt=="xml" )
	{
		XNode xml;
		PARSEINFO pi;
		if( !xml.LoadFromFile( ID.filename, &pi ) )
			RageException::Throw( pi.error_string );
		CString sDir = Dirname( ID.filename );
		return LoadFromActorFile( sDir, &xml );
	}
	else if( sExt=="actor" )
	{
		// TODO: Check for recursive loading
		IniFile ini;
		if( !ini.ReadFile( ID.filename ) )
			RageException::Throw( "%s", ini.GetError().c_str() );
	
		CString sDir = Dirname( ID.filename );

		const XNode* pNode = ini.GetChild( "Actor" );
		if( pNode == NULL )
			RageException::Throw( "The file '%s' doesn't have layer 'Actor'.", ID.filename.c_str() );

		return LoadFromActorFile( sDir, pNode );
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
	else if( sExt=="txt" ||
		sExt=="model" )
	{
		Model* pModel = new Model;
		pModel->Load( ID.filename );
		return pModel;
	}
	/* Do this last, to avoid the IsADirectory in most cases. */
	else if( IsADirectory(ID.filename)  )
	{
		CString sDir = ID.filename;
		if( sDir.Right(1) != "/" )
			sDir += '/';
		CString sIni = sDir + "BGAnimation.ini";
		CString sXml = sDir + "default.xml";

		if( DoesFileExist(sXml) )
		{
			XNode xml;
			PARSEINFO pi;
			if( !xml.LoadFromFile( sXml, &pi ) )
				RageException::Throw( pi.error_string );
			return LoadFromActorFile( sDir, &xml );
		}
		else
		{
			BGAnimation *pBGA = new BGAnimation;
			pBGA->LoadFromAniDir( sDir );
			return pBGA;
		}
	}
	else 
	{
		RageException::Throw("File \"%s\" has unknown type, \"%s\"",
			ID.filename.c_str(), sExt.c_str() );
	}
}

void ActorUtil::SetXY( Actor& actor, const CString &sScreenName )
{
	ASSERT( !actor.GetID().empty() );
	actor.SetXY( THEME->GetMetricF(sScreenName,actor.GetID()+"X"), THEME->GetMetricF(sScreenName,actor.GetID()+"Y") );
}

void ActorUtil::RunCommand( Actor& actor, const CString &sScreenName, const CString &sCommandName )
{
	actor.PlayCommand( sCommandName );

	// HACK:  It's very often that we command things to TweenOffScreen 
	// that we aren't drawing.  We know that an Actor is not being
	// used if its name is blank.  So, do nothing on Actors with a blank name.
	// (Do "playcommand" anyway; BGAs often have no name.)
	if( sCommandName=="Off" )
	{
		if( actor.GetID().empty() )
			return;
	}
	else
	{
		ASSERT_M( !actor.GetID().empty(), ssprintf("!actor.GetID().empty() ('%s', '%s')",
												   sScreenName.c_str(), sCommandName.c_str()) );
	}

	actor.RunCommands( THEME->GetMetricA(sScreenName,actor.GetID()+sCommandName+"Command") );
}

/*
 * (c) 2003-2004 Chris Danford
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
