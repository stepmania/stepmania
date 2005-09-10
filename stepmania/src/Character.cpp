#include "global.h"
#include "Character.h"
#include "IniFile.h"
#include "RageUtil.h"
#include "RageTextureID.h"

Character::Character()
{
	m_iPreloadRefcount = 0;
}

bool Character::Load( CString sCharDir )
{
	// Save character directory
	if( sCharDir.Right(1) != "/" )
		sCharDir += "/";
	m_sCharDir = sCharDir;


	// save ID
	{
		vector<CString> as;
		split( sCharDir, "/", as );
		m_sCharacterID = as.back();
	}

	{
		CStringArray as;
		GetDirListing( m_sCharDir+"card.png", as, false, true );
		GetDirListing( m_sCharDir+"card.jpg", as, false, true );
		GetDirListing( m_sCharDir+"card.gif", as, false, true );
		GetDirListing( m_sCharDir+"card.bmp", as, false, true );
		if( as.empty() )
			m_sCardPath = "";
		else
			m_sCardPath = as[0];
	}

	{
		CStringArray as;
		GetDirListing( m_sCharDir+"icon.png", as, false, true );
		GetDirListing( m_sCharDir+"icon.jpg", as, false, true );
		GetDirListing( m_sCharDir+"icon.gif", as, false, true );
		GetDirListing( m_sCharDir+"icon.bmp", as, false, true );
		if( as.empty() )
			m_sIconPath = "";
		else
			m_sIconPath = as[0];
	}

	// Save attacks
	IniFile ini;
	if( !ini.ReadFile( sCharDir+"character.ini" ) )
		return false;
	for( int i=0; i<NUM_ATTACK_LEVELS; i++ )
	{
		for( int j=0; j<NUM_ATTACKS_PER_LEVEL; j++ )
			ini.GetValue( "Character", ssprintf("Level%dAttack%d",i+1,j+1), m_sAttacks[i][j] );
	}

	// get display name (if any)
	ini.GetValue( "Character", "DisplayName", m_sDisplayName );

	return true;
}


CString GetRandomFileInDir( CString sDir )
{
	CStringArray asFiles;
	GetDirListing( sDir, asFiles, false, true );
	if( asFiles.empty() )
		return CString();
	else
		return asFiles[rand()%asFiles.size()];
}


CString Character::GetModelPath() const
{
	CString s = m_sCharDir + "model.txt";
	if( DoesFileExist(s) )
		return s; 
	else 
		return CString();
}

CString Character::GetRestAnimationPath() const	{ return DerefRedir(GetRandomFileInDir(m_sCharDir + "Rest/")); }
CString Character::GetWarmUpAnimationPath() const { return DerefRedir(GetRandomFileInDir(m_sCharDir + "WarmUp/")); }
CString Character::GetDanceAnimationPath() const { return DerefRedir(GetRandomFileInDir(m_sCharDir + "Dance/")); }
CString Character::GetTakingABreakPath() const
{
	CStringArray as;
	GetDirListing( m_sCharDir+"break.png", as, false, true );
	GetDirListing( m_sCharDir+"break.jpg", as, false, true );
	GetDirListing( m_sCharDir+"break.gif", as, false, true );
	GetDirListing( m_sCharDir+"break.bmp", as, false, true );
	if( as.empty() )
		return CString();
	else
		return as[0];
}

CString Character::GetSongSelectIconPath() const
{
	CStringArray as;
	// first try and find an icon specific to the select music screen
	// so you can have different icons for music select / char select
	GetDirListing( m_sCharDir+"selectmusicicon.png", as, false, true );
	GetDirListing( m_sCharDir+"selectmusicicon.jpg", as, false, true );
	GetDirListing( m_sCharDir+"selectmusicicon.gif", as, false, true );
	GetDirListing( m_sCharDir+"selectmusicicon.bmp", as, false, true );

	if( as.empty() )
	{
		// if that failed, try using the regular icon
		GetDirListing( m_sCharDir+"icon.png", as, false, true );
		GetDirListing( m_sCharDir+"icon.jpg", as, false, true );
		GetDirListing( m_sCharDir+"icon.gif", as, false, true );
		GetDirListing( m_sCharDir+"icon.bmp", as, false, true );
		if( as.empty() )
			return CString();
		else
			return as[0];
	}
	else
		return as[0];
}

CString Character::GetStageIconPath() const
{
	CStringArray as;
	// first try and find an icon specific to the select music screen
	// so you can have different icons for music select / char select
	GetDirListing( m_sCharDir+"stageicon.png", as, false, true );
	GetDirListing( m_sCharDir+"stageicon.jpg", as, false, true );
	GetDirListing( m_sCharDir+"stageicon.gif", as, false, true );
	GetDirListing( m_sCharDir+"stageicon.bmp", as, false, true );

	if( as.empty() )
	{
		// if that failed, try using the regular icon
		GetDirListing( m_sCharDir+"card.png", as, false, true );
		GetDirListing( m_sCharDir+"card.jpg", as, false, true );
		GetDirListing( m_sCharDir+"card.gif", as, false, true );
		GetDirListing( m_sCharDir+"card.bmp", as, false, true );
		if( as.empty() )
			return CString();
		else
			return as[0];
	}
	else
		return as[0];
}

bool Character::Has2DElems()
{
	if( DoesFileExist(m_sCharDir + "2DFail/BGAnimation.ini") ) // check 2D Idle BGAnim exists
		return true;
	if( DoesFileExist(m_sCharDir + "2DFever/BGAnimation.ini") ) // check 2D Idle BGAnim exists
		return true;
	if( DoesFileExist(m_sCharDir + "2DGood/BGAnimation.ini") ) // check 2D Idle BGAnim exists
		return true;	
	if( DoesFileExist(m_sCharDir + "2DMiss/BGAnimation.ini") ) // check 2D Idle BGAnim exists
		return true;
	if( DoesFileExist(m_sCharDir + "2DWin/BGAnimation.ini") ) // check 2D Idle BGAnim exists
		return true;
	if( DoesFileExist(m_sCharDir + "2DWinFever/BGAnimation.ini") ) // check 2D Idle BGAnim exists
		return true;
	if( DoesFileExist(m_sCharDir + "2DGreat/BGAnimation.ini") ) // check 2D Idle BGAnim exists
		return true;
	if( DoesFileExist(m_sCharDir + "2DIdle/BGAnimation.ini") ) // check 2D Idle BGAnim exists
		return true;
	return false;
}

void Character::DemandGraphics()
{
	++m_iPreloadRefcount;
	if( m_iPreloadRefcount == 1 )
	{
		CString s = GetIconPath();
		if( !s.empty() )
			m_Preload.Load( s );
	}
}

void Character::UndemandGraphics()
{
	--m_iPreloadRefcount;
	if( m_iPreloadRefcount == 0 )
		m_Preload.UnloadAll();
}

// lua start
#include "LuaBinding.h"

class LunaCharacter: public Luna<Character>
{
public:
	LunaCharacter() { LUA->Register( Register ); }

	static int GetCardPath( T* p, lua_State *L )			{ lua_pushstring(L, p->GetCardPath() ); return 1; }
	static int GetIconPath( T* p, lua_State *L )			{ lua_pushstring(L, p->GetIconPath() ); return 1; }
	static int GetSongSelectIconPath( T* p, lua_State *L )	{ lua_pushstring(L, p->GetSongSelectIconPath() ); return 1; }
	static int GetStageIconPath( T* p, lua_State *L )		{ lua_pushstring(L, p->GetStageIconPath() ); return 1; }

	CString GetCardPath() const;
	CString GetIconPath() const;
	
	static void Register( Lua *L )
	{
		ADD_METHOD( GetCardPath );
		ADD_METHOD( GetIconPath );
		ADD_METHOD( GetSongSelectIconPath );
		ADD_METHOD( GetStageIconPath );

		Luna<T>::Register( L );
	}
};

LUA_REGISTER_CLASS( Character )
// lua end

/*
 * (c) 2003 Chris Danford
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
