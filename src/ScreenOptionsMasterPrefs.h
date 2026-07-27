#ifndef SCREEN_OPTIONS_MASTER_PREFS_H
#define SCREEN_OPTIONS_MASTER_PREFS_H

#include "EnumHelper.h"

#include <vector>


static const int MAX_OPTIONS=16;
enum OptEffect
{
	OPT_SAVE_PREFERENCES		=	(1<<0),
	OPT_APPLY_GRAPHICS			=	(1<<1),
	OPT_APPLY_THEME				=	(1<<2),
	OPT_CHANGE_GAME				=	(1<<3),
	OPT_APPLY_SOUND				=	(1<<4),
	OPT_APPLY_SONG				=	(1<<5),
	OPT_APPLY_ASPECT_RATIO		=	(1<<6),
	OPT_APPLY_PROFILES			=	(1<<7),
	NUM_OptEffect				=	8,
	OptEffect_Invalid			=	MAX_OPTIONS+1
};
const RString& OptEffectToString( OptEffect e );
OptEffect StringToOptEffect( const std::string &e );
LuaDeclareType( OptEffect );

struct ConfOption
{
	static ConfOption *Find( RString name );

	// Name of this option.
	RString name;

	// Name of the preference this option affects.
	RString m_sPrefName;

	typedef void (*MoveData_t)( int &sel, bool ToSel, const ConfOption *pConfOption );
	MoveData_t MoveData;
	int m_iEffects;
	bool m_bAllowThemeItems;

	/* For dynamic options, update the options. Since this changes the available
	 * options, this may invalidate the offsets returned by Get() and Put(). */
	void UpdateAvailableOptions();

	/* Return the list of available selections; Get() and Put() use indexes into
	 * this array. UpdateAvailableOptions() should be called before using this. */
	void MakeOptionsList( std::vector<RString> &out ) const;

	inline int Get() const { int sel; MoveData( sel, true, this ); return sel; }
	inline void Put( int sel ) const { MoveData( sel, false, this ); }
	int GetEffects() const;

	ConfOption( const char *n, MoveData_t m,
		const char *c0=nullptr, const char *c1=nullptr, const char *c2=nullptr, const char *c3=nullptr, const char *c4=nullptr, const char *c5=nullptr, const char *c6=nullptr, const char *c7=nullptr, const char *c8=nullptr, const char *c9=nullptr, const char *c10=nullptr, const char *c11=nullptr, const char *c12=nullptr, const char *c13=nullptr, const char *c14=nullptr, const char *c15=nullptr, const char *c16=nullptr, const char *c17=nullptr, const char *c18=nullptr, const char *c19=nullptr )
	{
		name = n;
		m_sPrefName = name; // copy from name (not n), to allow refcounting
		MoveData = m;
		MakeOptionsListCB = nullptr;
		m_iEffects = 0;
		m_bAllowThemeItems = true;
#define PUSH( c )	if(c) names.push_back(c);
		PUSH(c0);PUSH(c1);PUSH(c2);PUSH(c3);PUSH(c4);PUSH(c5);PUSH(c6);PUSH(c7);PUSH(c8);PUSH(c9);PUSH(c10);PUSH(c11);PUSH(c12);PUSH(c13);PUSH(c14);PUSH(c15);PUSH(c16);PUSH(c17);PUSH(c18);PUSH(c19);
	}
	void AddOption( const RString &sName ) { PUSH(sName); }
#undef PUSH

	ConfOption( const char *n, MoveData_t m,
			void (*lst)( std::vector<RString> &out ) )
	{
		name = n;
		MoveData = m;
		MakeOptionsListCB = lst;
		m_iEffects = 0;
		m_bAllowThemeItems = false; // don't theme dynamic choices
	}


// private:
	std::vector<RString> names;
	void (*MakeOptionsListCB)( std::vector<RString> &out );
};

#endif

/**
 * @file
 * @author Glenn Maynard (c) 2003-2004
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
