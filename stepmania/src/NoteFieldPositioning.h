#ifndef NOTEFIELD_POSITIONING_H
#define NOTEFIELD_POSITIONING_H

#include "PlayerNumber.h"
#include "StyleDef.h"
#include "PlayerNumber.h"
#include "Style.h"
#include "Actor.h"

#include <set>

struct NoteFieldMode
{
	NoteFieldMode();
	bool MatchesCurrentGame() const;

	void BeginDrawTrack(int tn);
	void EndDrawTrack(int tn);

	CString name;
	set<Style> Styles;

	Actor m_Center;
	Actor m_CenterTrack[MAX_NOTE_TRACKS];

	/* 0 = no perspective */
	float m_fFov;
	Actor m_Position;
	Actor m_PositionTrack[MAX_NOTE_TRACKS];

	float m_fFirstPixelToDrawScale, m_fLastPixelToDrawScale;
	CString Backdrop;
	Actor m_PositionBackdrop;
};

class NoteFieldPositioning
{
public:
	NoteFieldPositioning(CString fn);
	void Load(PlayerNumber pn);

	void GetNamesForCurrentGame(vector<CString> &IDs);

private:
	vector<NoteFieldMode> Modes;
	int GetID(const CString &name) const;
};

extern NoteFieldMode g_NoteFieldMode[NUM_PLAYERS];

#endif
