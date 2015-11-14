#ifndef Character_H
#define Character_H

#include "GameConstantsAndTypes.h"
#include "RageTexturePreloader.h"
#include "LuaReference.h"
struct lua_State;
typedef lua_State Lua;

/** @brief A persona that defines attacks for use in battle. */
class Character
{
public:
	Character();
	~Character() { }

	bool Load( std::string sCharDir ); // return true if successful

	std::string GetTakingABreakPath() const;
	std::string GetCardPath() const { return m_sCardPath; }
	std::string GetIconPath() const { return m_sIconPath; }

	std::string GetModelPath() const;
	std::string GetRestAnimationPath() const;
	std::string GetWarmUpAnimationPath() const;
	std::string GetDanceAnimationPath() const;
	std::string GetSongSelectIconPath() const;
	std::string GetStageIconPath() const;
	bool Has2DElems();

	bool IsDefaultCharacter() const;

	void DemandGraphics();
	void UndemandGraphics();

	// Lua
	void PushSelf( Lua *L );

	// smart accessor
	const std::string &GetDisplayName() { return !m_sDisplayName.empty() ? m_sDisplayName : m_sCharacterID; }

	std::string m_sCharDir;
	std::string m_sCharacterID;

private:
	std::string m_sDisplayName;
	std::string m_sCardPath;
	std::string m_sIconPath;

public:
	apActorCommands m_cmdInit;

	/**
	 * @brief Is this character playable in the Rave mode?
	 *
	 * All of the variables listed below here will be filled in if true. */
	bool	m_bUsableInRave;

	std::string	m_sAttacks[NUM_ATTACK_LEVELS][NUM_ATTACKS_PER_LEVEL];
	RageTexturePreloader m_Preload;
	int m_iPreloadRefcount;
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
