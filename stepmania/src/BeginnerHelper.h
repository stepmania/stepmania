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
	BeginnerHelper();
	~BeginnerHelper();
	
	void FlashOnce();
	bool Initialize( int iDancePadType );
	bool IsInitialized() { return m_bInitialized; }
	static bool CanUse();
	void AddPlayer( int pn, NoteData *pNotes );
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
