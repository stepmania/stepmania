#ifndef DancingCharacters_H
#define DancingCharacters_H
/*
-----------------------------------------------------------------------------
 Class: DancingCharacters

 Desc: A graphic displayed in the DancingCharacters during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Model.h"
#include "ActorFrame.h"
#include "PlayerNumber.h"
#include "BGAnimation.h"
#include "ThemeManager.h"
#include "RageTimer.h"

enum ANIM_STATES_2D
{
	AS2D_IDLE = 0,
	AS2D_MISS,
	AS2D_GOOD,
	AS2D_GREAT,
	AS2D_FEVER,
	AS2D_FAIL,
	AS2D_WIN,
	AS2D_WINFEVER,
	AS2D_IGNORE, // special case -- so that we can timer to idle again.
	AS2D_MAXSTATES // leave at end
};

class DancingCharacters : public ActorFrame
{
public:
	DancingCharacters();

	virtual void LoadNextSong();
 
	virtual void Update( float fDelta );
	virtual void DrawPrimitives();
	bool	m_bDrawDangerLight;
	void Change2DAnimState(int iPlayerNum, int iState);
protected:

	Model	m_Character[NUM_PLAYERS];

	float	m_CameraDistance;
	float	m_CameraPanYStart;
	float	m_CameraPanYEnd;
	float	m_fLookAtHeight;
	float	m_fCameraHeightStart;
	float	m_fCameraHeightEnd;
	float	m_fThisCameraStartBeat;
	float	m_fThisCameraEndBeat;

	bool m_bHas2DElements[NUM_PLAYERS];
	
	bool m_bHasIdleAnim[NUM_PLAYERS];
	BGAnimation m_bgIdle[NUM_PLAYERS];
	bool m_bHasMissAnim[NUM_PLAYERS];
	BGAnimation m_bgMiss[NUM_PLAYERS];
	bool m_bHasGoodAnim[NUM_PLAYERS];
	BGAnimation m_bgGood[NUM_PLAYERS];
	bool m_bHasGreatAnim[NUM_PLAYERS];
	BGAnimation m_bgGreat[NUM_PLAYERS];
	bool m_bHasFeverAnim[NUM_PLAYERS];
	BGAnimation m_bgFever[NUM_PLAYERS];
	bool m_bHasFailAnim[NUM_PLAYERS];
	BGAnimation m_bgFail[NUM_PLAYERS];
	bool m_bHasWinAnim[NUM_PLAYERS];
	BGAnimation m_bgWin[NUM_PLAYERS];
	bool m_bHasWinFeverAnim[NUM_PLAYERS];
	BGAnimation m_bgWinFever[NUM_PLAYERS];
	RageTimer m_2DIdleTimer[NUM_PLAYERS];

	int m_i2DAnimState[NUM_PLAYERS];
};

#endif
