#ifndef BEGINNER_HELPER_H
#define BEGINNER_HELPER_H

#include "ActorFrame.h"
#include "Model.h"
#include "Character.h"
#include "Sprite.h"
#include "PlayerNumber.h"
#include "NoteData.h"

class BeginnerHelper : public ActorFrame
{
public:
	BeginnerHelper();
	~BeginnerHelper();
	
	void FlashOnce();
	bool Initialize( int iDancePadType );
	bool IsInitialized() { return m_bInitialized; }
	static bool CanUse();
	void AddPlayer( int pn, NoteData *pSteps );
	void SetFlash(CString sFilename, float fX, float fY);
	void ShowStepCircle( int pn, int CSTEP );
	void TurnFlashOff();
	void TurnFlashOn();

	bool	m_bShowBackground;

	void Update( float fDeltaTime );
	virtual void DrawPrimitives();

protected:
	void Step( int pn, int CSTEP );

	NoteData m_NoteData[NUM_PLAYERS];
	bool m_bPlayerEnabled[NUM_PLAYERS];
	Model m_mDancer[NUM_PLAYERS];
	Model m_mDancePad;
	Sprite	m_sFlash;
	Sprite	m_sBackground;
	Sprite	m_sStepCircle[NUM_PLAYERS*2];	// Need two for each player during jumps

	int  m_iLastRowChecked;
	bool m_bInitialized;
	bool m_bFlashEnabled;
};
#endif

/*
 * (c) 2003 Kevin Slaughter
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
