#ifndef BEGINNERHELPER_H
#define BEGINNERHELPER_H

#include "ActorFrame.h"
#include "Model.h"
#include "Character.h"
#include "Sprite.h"
#include "PlayerNumber.h"
#include "NoteData.h"

class BeginnerHelper : public ActorFrame
{
public:
	enum STEPTYPE
	{
		ST_LEFT=6,
		ST_DOWN=3,
		ST_UP=8,
		ST_RIGHT=4,
		ST_JUMPLR=10,
		ST_JUMPUD=11
	};

	BeginnerHelper();
	~BeginnerHelper();
	
	ActorFrame	m_afDancerSuite;	// So we can easily rotate or whatever without disturbing the Background.

	void FlashOnce();
	void Initialize( int iDancePadType );
	void AddPlayer( int pn, NoteData *pNotes );
	void SetFlash(CString sFilename, float fX, float fY);
	void ShowStepCircle( int pn, int CSTEP );
	void Step( int pn, int CSTEP );
	void TurnFlashOff();
	void TurnFlashOn();

	void Update( float fDeltaTime );
	virtual void DrawPrimitives();

protected:
	NoteData m_NoteData[NUM_PLAYERS];
	Model m_mDancer[NUM_PLAYERS];
	Model m_mDancePad;
	Sprite	m_sFlash;
	Sprite	m_sBackground;
	Sprite	m_sStepCircle[NUM_PLAYERS*2];	// Need two for each player during jumps

	int	iDancePadType;  // 0=none, 1=single, 2=double || This entire class only uses 1 pad.
	bool m_bInitialized;
	bool m_bFlashEnabled;
};
#endif
