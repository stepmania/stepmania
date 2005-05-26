/* Background - Background behind notes while playing. */

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "Sprite.h"
#include "Quad.h"
#include "ActorFrame.h"
#include "BGAnimation.h"
#include "song.h"
#include <deque>
#include <map>

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
	void Init();

	virtual void LoadFromSong( const Song *pSong );
	virtual void Unload();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void FadeToActualBrightness() { m_Brightness.FadeToActualBrightness(); }
	void SetBrightness( float fBrightness ) { m_Brightness.Set(fBrightness); } /* overrides pref and Cover */
	
	DancingCharacters* GetDancingCharacters() { return m_pDancingCharacters; };

protected:
	const Song *m_pSong;
	void LoadFromRandom( float fFirstBeat, float fLastBeat, const TimingData &timing, CString sPreferredSubDir );

	bool IsDangerAllVisible();
	
	bool m_bInitted;
	DancingCharacters*	m_pDancingCharacters;

	class Layer
	{
	public:
		Layer();
		void Unload();

		Actor *CreateSongBGA( const Song *pSong, CString sBGName ) const;
		CString CreateRandomBGA( const Song *pSong, CString sPreferredSubDir );
		int FindBGSegmentForBeat( float fBeat ) const;
		void UpdateCurBGChange( const Song *pSong, float fLastMusicSeconds, float fCurrentTime );

		map<CString,Actor*> m_BGAnimations;
		deque<CString> m_RandomBGAnimations;
		vector<BackgroundChange> m_aBGChanges;
		int				m_iCurBGChangeIndex;
		Actor *m_pCurrentBGA;
		Actor *m_pFadingBGA;
		float m_fSecsLeftInFade;
	};
	Layer m_Layer[NUM_BACKGROUND_LAYERS];

	float m_fLastMusicSeconds;

	BGAnimation		m_DangerPlayer[NUM_PLAYERS];
	BGAnimation		m_DangerAll;

	BGAnimation		m_DeadPlayer[NUM_PLAYERS];
	
	// cover up the edge of animations that might hang outside of the background rectangle
	Quad m_quadBorderLeft, m_quadBorderTop, m_quadBorderRight, m_quadBorderBottom;

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
