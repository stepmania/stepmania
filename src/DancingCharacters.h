#ifndef DancingCharacters_H
#define DancingCharacters_H

#include "ActorFrame.h"
#include "PlayerNumber.h"
#include "ThemeManager.h"
#include "RageTimer.h"
#include "AutoActor.h"
class Model;

/** @brief The different animation states for the dancer. */
enum ANIM_STATES_2D
{
	AS2D_IDLE = 0, /**< The dancer is idle. */
	AS2D_MISS, /**< The dancer just missed a note. */
	AS2D_GOOD, /**< The dancer is doing a good job. */
	AS2D_GREAT, /**< The dancer is doing a great job. */
	AS2D_FEVER, /**< The dancer is on fire! */
	AS2D_FAIL, /**< The dancer has failed. */
	AS2D_WIN, /**< The dancer has won. */
	AS2D_WINFEVER, /**< The dancer has won while on fire. */
	AS2D_IGNORE, /**< This is a special case -- so that we can timer to idle again. */
	AS2D_MAXSTATES /**< A count of the maximum number of states. */
};

/** @brief Characters that react to how the players are doing. */
class DancingCharacters : public ActorFrame
{
public:
	DancingCharacters();
	virtual ~DancingCharacters();

	void LoadNextSong();
 
	virtual void Update( float fDelta );
	virtual void DrawPrimitives();
	bool	m_bDrawDangerLight;
	void Change2DAnimState( PlayerNumber pn, int iState );
protected:

	Model	*m_pCharacter[NUM_PLAYERS];

	/** @brief How far away is the camera from the dancer? */
	float	m_CameraDistance;
	float	m_CameraPanYStart;
	float	m_CameraPanYEnd;
	float	m_fLookAtHeight;
	float	m_fCameraHeightStart;
	float	m_fCameraHeightEnd;
	float	m_fThisCameraStartBeat;
	float	m_fThisCameraEndBeat;

	bool m_bHas2DElements[NUM_PLAYERS];
	
	AutoActor m_bgIdle[NUM_PLAYERS];
	AutoActor m_bgMiss[NUM_PLAYERS];
	AutoActor m_bgGood[NUM_PLAYERS];
	AutoActor m_bgGreat[NUM_PLAYERS];
	AutoActor m_bgFever[NUM_PLAYERS];
	AutoActor m_bgFail[NUM_PLAYERS];
	AutoActor m_bgWin[NUM_PLAYERS];
	AutoActor m_bgWinFever[NUM_PLAYERS];
	RageTimer m_2DIdleTimer[NUM_PLAYERS];

	int m_i2DAnimState[NUM_PLAYERS];
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
 * 
 * (c) 2016- Electromuis, Anton Grootes
 * This branch of https://github.com/stepmania/stepmania
 * will from here on out be released as GPL v3 (wich converts from the previous MIT license)
 */
