#ifndef ATTACK_H
#define ATTACK_H

#define ATTACK_STARTS_NOW (-10000.f)

#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"

#include <vector>


class Song;
class PlayerState;
/** @brief An action made against a Player to make things more difficult. */
struct Attack
{
	AttackLevel	level;
	/**
	 * @brief the starting point of this attack.
	 *
	 * If this is -1, then the attack starts now. */
	float fStartSecond;
	/** @brief How long does this attack last? */
	float fSecsRemaining;
	/** @brief The modifiers used for this attack. */
	RString sModifiers;
	bool bOn; // set and used by GAMESTATE
	bool bGlobal; // true for song-wide course mods
	bool bShowInAttackList;

	void MakeBlank()
	{
		level = ATTACK_LEVEL_1;
		fStartSecond = ATTACK_STARTS_NOW;
		fSecsRemaining = 0;
		sModifiers = RString();
		bOn = false;
		bGlobal = false;
		bShowInAttackList = true;
	}
	Attack(): level(ATTACK_LEVEL_1), fStartSecond(ATTACK_STARTS_NOW),
		fSecsRemaining(0), sModifiers(RString()),
		bOn(false), bGlobal(false), bShowInAttackList(true)
		{} // MakeBlank() is effectively called here.
	Attack(
		AttackLevel	level_,
		float fStartSecond_,
		float fSecsRemaining_,
		RString sModifiers_,
		bool bOn_,
		bool bGlobal_,
		bool bShowInAttackList_ = true ):
		level(level_), fStartSecond(fStartSecond_),
		fSecsRemaining(fSecsRemaining_), sModifiers(sModifiers_),
		bOn(bOn_), bGlobal(bGlobal_),
		bShowInAttackList(bShowInAttackList_) {}

	void GetAttackBeats( const Song *pSong, float &fStartBeat, float &fEndBeat ) const;
	void GetRealtimeAttackBeats( const Song *pSong, const PlayerState* pPlayerState, float &fStartBeat, float &fEndBeat ) const;
	/**
	 * @brief Determine if this attack has no modifiers, and is thus blank or empty.
	 * @return true if it is blank/empty, or false otherwise. */
	bool IsBlank() const { return sModifiers.empty(); }
	/**
	 * @brief Determine if two Attacks are equal to each other.
	 * @param rhs the other Attack in question.
	 * @return true if the two Attacks are equal, or false otherwise. */
	bool operator== ( const Attack &rhs ) const;
	bool ContainsTransformOrTurn() const;
	static Attack FromGlobalCourseModifier( const RString &sModifiers );
	RString GetTextDescription() const;

	int GetNumAttacks() const;
};

struct AttackArray : public std::vector<Attack>
{
	/**
	 * @brief Determine if the list of attacks contains a transform or turn mod.
	 * @return true if it does, or false otherwise. */
	bool ContainsTransformOrTurn() const;

	/**
	 * @brief Return a string representation used for simfiles.
	 * @return the string representation. */
	std::vector<RString> ToVectorString() const;

	/**
	 * @brief Adjust the starting time of all attacks.
	 * @param delta the amount to change. */
	void UpdateStartTimes(float delta);
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2003-2004
 * @section LICENSE
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
