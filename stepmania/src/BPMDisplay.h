/* BPMDisplay - displays a BPM or a range of BPMs. */

#ifndef BPM_DISPLAY_H
#define BPM_DISPLAY_H

#include "BitmapText.h"
#include "AutoActor.h"
#include "ThemeMetric.h"
#include "LocalizedString.h"
class Song;
class Course;
struct DisplayBpms;

class BPMDisplay : public BitmapText
{
public:
	BPMDisplay();
	virtual BPMDisplay *Copy() const;
	void Load();
	virtual void Update( float fDeltaTime ); 
	void LoadFromNode( const XNode *pNode );

	void SetBpmFromSong( const Song* pSong );
	void SetBpmFromCourse( const Course* pCourse );
	void SetConstantBpm( float fBPM );
	void CycleRandomly();
	void NoBPM();
	void SetVarious();
	void SetFromGameState();

	// Lua
	virtual void PushSelf( lua_State *L );

protected:
	float GetActiveBPM() const;
	void SetBPMRange( const DisplayBpms &bpms );

	ThemeMetric<RageColor> NORMAL_COLOR;
	ThemeMetric<RageColor> CHANGE_COLOR;
	ThemeMetric<RageColor> EXTRA_COLOR;
	ThemeMetric<bool> CYCLE;
	ThemeMetric<RString> SEPARATOR;
	ThemeMetric<RString> NO_BPM_TEXT;
	
	float m_fBPMFrom, m_fBPMTo;
	int m_iCurrentBPM;
	vector<float> m_BPMS;
	float m_fPercentInState;
	float m_fCycleTime;
};

#endif

/*
 * (c) 2001-2002 Chris Danford
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
