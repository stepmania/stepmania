/* Background - Background behind notes while playing. */

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "Sprite.h"
#include "Quad.h"
#include "ActorFrame.h"
#include "BGAnimation.h"
#include "song.h"
#include <deque>
class DancingCharacters;

class BrightnessOverlay: public ActorFrame
{
public:
	BrightnessOverlay();
	void Update( float fDeltaTime );

	void FadeToActualBrightness();
	void SetActualBrightness();
	void Set( float fBrightness );

private:
	Quad m_quadBGBrightness[NUM_PLAYERS];
	Quad m_quadBGBrightnessFade;
};

class Background : public ActorFrame
{
public:
	Background();
	~Background();

	virtual void LoadFromSong( const Song *pSong );
	virtual void Unload();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void FadeToActualBrightness() { m_Brightness.FadeToActualBrightness(); }
	void SetBrightness( float fBrightness ) { m_Brightness.Set(fBrightness); } /* overrides pref and Cover */
	
	DancingCharacters* GetDancingCharacters() { return m_pDancingCharacters; };

protected:
	const Song *m_pSong;
	void LoadFromAniDir( CString sAniDir );
	void LoadFromRandom( float fFirstBeat, float fLastBeat, const TimingData &timing );
	int FindBGSegmentForBeat( float fBeat ) const;

	bool IsDangerPlayerVisible( PlayerNumber pn );
	bool IsDangerAllVisible();
	bool IsDeadPlayerVisible( PlayerNumber pn );
	void UpdateCurBGChange( float fCurrentTime );
	
	DancingCharacters*	m_pDancingCharacters;

	BGAnimation		m_DangerPlayer[NUM_PLAYERS];
	BGAnimation		m_DangerAll;

	BGAnimation		m_DeadPlayer[NUM_PLAYERS];

	BGAnimation* CreateSongBGA( CString sBGName ) const;
	CString CreateRandomBGA();

	map<CString,BGAnimation*> m_BGAnimations;
	deque<CString> m_RandomBGAnimations;
	vector<BackgroundChange> m_aBGChanges;
	int				m_iCurBGChangeIndex;
	BGAnimation* m_pCurrentBGA;
	BGAnimation* m_pFadingBGA;
	float m_fSecsLeftInFade;
	float m_fLastMusicSeconds;
	Quad m_quadBorder[4];	// l, t, r, b - cover up the edge of animations that might hang outside of the background rectangle

	BrightnessOverlay m_Brightness;
};


#endif

/*
 * (c) 2001-2004 Chris Danford, Ben Nordstrom
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
