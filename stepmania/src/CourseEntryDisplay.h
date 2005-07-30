/* CourseEntryDisplay - Holds course name and banner. */

#ifndef COURSE_ENTRY_DISPLAY_H
#define COURSE_ENTRY_DISPLAY_H

#include "ActorFrame.h"
#include "BitmapText.h"
#include "TextBanner.h"
#include "Sprite.h"
#include "PlayerNumber.h"
struct TrailEntry;
#include "ThemeMetric.h"

class CourseEntryDisplay : public ActorFrame
{
public:
	CourseEntryDisplay();
	virtual Actor *Copy() const;

	void Load();
	void LoadFromNode( const CString& sDir, const XNode* pNode );

	void SetFromTrailEntry( int iCourseEntryIndex, const TrailEntry *te[NUM_PLAYERS] );
	void SetFromGameState( int iCourseEntryIndex );

	// Lua
	void PushSelf( lua_State *L );

private:
	void SetDifficulty( PlayerNumber pn, const CString &text, RageColor c );

	Sprite		m_sprFrame;
	BitmapText	m_textNumber;
	TextBanner	m_TextBanner;
	BitmapText	m_textFoot[NUM_PLAYERS];
	BitmapText	m_textDifficultyNumber[NUM_PLAYERS];
	BitmapText	m_textModifiers;

	ThemeMetric<bool>		SEPARATE_COURSE_METERS;
	ThemeMetric<CString>	TEXT_BANNER_TYPE;
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
