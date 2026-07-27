/* SongOptions - Options that apply to an entire song (not per-player). */

#ifndef SONG_OPTIONS_H
#define SONG_OPTIONS_H

#include "EnumHelper.h"

#include <vector>


enum AutosyncType
{
	AutosyncType_Off,
	AutosyncType_Song,
	AutosyncType_Machine,
	AutosyncType_Tempo,
	NUM_AutosyncType,
	AutosyncType_Invalid
};
const RString& AutosyncTypeToString( AutosyncType cat );
const RString& AutosyncTypeToLocalizedString( AutosyncType cat );
LuaDeclareType( AutosyncType );

enum SoundEffectType
{
	SoundEffectType_Off,
	SoundEffectType_Speed,
	SoundEffectType_Pitch,
	NUM_SoundEffectType,
	SoundEffectType_Invalid
};
const RString& SoundEffectTypeToString( SoundEffectType cat );
const RString& SoundEffectTypeToLocalizedString( SoundEffectType cat );
LuaDeclareType( SoundEffectType );

class SongOptions
{
public:
	bool m_bAssistClap;
	bool m_bAssistMetronome;
	float m_fMusicRate,	m_SpeedfMusicRate;
	float m_fHaste, m_SpeedfHaste;
	AutosyncType m_AutosyncType;
	SoundEffectType m_SoundEffectType;
	bool m_bStaticBackground;
	bool m_bRandomBGOnly;
	bool m_bSaveScore;
	bool m_bSaveReplay;

	/**
	 * @brief Set up the SongOptions with reasonable defaults.
	 */
	SongOptions(): m_bAssistClap(false),
		m_bAssistMetronome(false), m_fMusicRate(1.0f),
		m_SpeedfMusicRate(1.0f), m_fHaste(0.0f),
		m_SpeedfHaste(1.0f), m_AutosyncType(AutosyncType_Off),
		m_SoundEffectType(SoundEffectType_Off),
		m_bStaticBackground(false), m_bRandomBGOnly(false),
		m_bSaveScore(true), m_bSaveReplay(false) {};
	void Init() { *this = {}; }
	void Approach( const SongOptions& other, float fDeltaSeconds );
	void GetMods( std::vector<RString> &AddTo ) const;
	void GetLocalizedMods( std::vector<RString> &AddTo ) const;
	RString GetString() const;
	RString GetLocalizedString() const;
	void FromString( const RString &sOptions );
	bool FromOneModString( const RString &sOneMod, RString &sErrorDetailOut );	// On error, return false and optionally set sErrorDetailOut

	bool operator==( const SongOptions &other ) const;
	bool operator!=( const SongOptions &other ) const { return !operator==(other); }

	// Lua
	void PushSelf( lua_State *L );
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
