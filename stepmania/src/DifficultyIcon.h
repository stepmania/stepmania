/* DifficultyIcon - Graphical representation of the difficulty class. */

#ifndef DIFFICULTY_ICON_H
#define DIFFICULTY_ICON_H

#include "Sprite.h"
#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "Difficulty.h"
class Steps;
class Trail;


class DifficultyIcon : public Sprite
{
public:
	DifficultyIcon();
	virtual bool EarlyAbortDraw() const { return m_bBlank || Sprite::EarlyAbortDraw(); }

	bool Load( RString sFilePath );
	virtual void Load( RageTextureID ID ) { Load( ID.filename ); }
	virtual void LoadFromNode( const XNode* pNode );
	virtual Actor *Copy() const;

	void SetPlayer( PlayerNumber pn );
	void Unset();
	void SetFromSteps( PlayerNumber pn, const Steps* pSteps );
	void SetFromTrail( PlayerNumber pn, const Trail* pTrail );
	void SetFromDifficulty( Difficulty dc );

	// Lua
	void PushSelf( lua_State *L );

protected:
	bool m_bBlank;
	PlayerNumber m_PlayerNumber;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
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
