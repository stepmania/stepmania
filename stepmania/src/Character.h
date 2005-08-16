/* Character - An persona that defines attacks for use in battle. */

#ifndef Character_H
#define Character_H

#include "GameConstantsAndTypes.h"
#include "RageTexturePreloader.h"
struct lua_State;
typedef lua_State Lua;


class Character
{
public:
//	Character();
	~Character() { }

	bool Load( CString sCharDir );	// return true if success

	CString GetTakingABreakPath() const;
	CString GetCardPath() const;
	CString GetIconPath() const;

	CString GetModelPath() const;
	CString GetRestAnimationPath() const;
	CString GetWarmUpAnimationPath() const;
	CString GetDanceAnimationPath() const;
	CString GetSongSelectIconPath() const;
	CString GetStageIconPath() const;
	bool Has2DElems();

	bool IsDefaultCharacter() const
	{
		return m_sCharacterID.CompareNoCase("default") == 0;
	}

	void DemandGraphics();
	void UndemandGraphics();

	//
	// Lua
	//
	void PushSelf( Lua *L );

	// smart accessor
	const CString &GetDisplayName() { return !m_sDisplayName.empty() ? m_sDisplayName : m_sCharacterID; }

	CString m_sCharDir;
	CString m_sCharacterID;
private:
	CString m_sDisplayName;
public:

	// All the stuff below will be filled in if this character is playable in Rave mode
	bool	m_bUsableInRave;	

	CString	m_sAttacks[NUM_ATTACK_LEVELS][NUM_ATTACKS_PER_LEVEL];
	RageTexturePreloader m_Preload;
};

#endif

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
