#ifndef NOTEFIELD_POSITIONING_H
#define NOTEFIELD_POSITIONING_H

#include "PlayerNumber.h"
#include "StyleDef.h"
#include "PlayerNumber.h"
#include "Style.h"
#include "Actor.h"

#include <set>

class NoteFieldPositioning
{
	struct Mode
	{
		Mode();
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

		CString Backdrop;
		Actor m_PositionBackdrop;
	};

	vector<Mode> Modes;

public:
	NoteFieldPositioning(CString fn);

	/* Get the mode number for the given positioning type (for the current
	 * game and style). */
	int GetID(const CString &name) const;
	int GetID(PlayerNumber pn) const;

	void BeginDrawTrack(PlayerNumber pn, int tn);
	void EndDrawTrack(PlayerNumber pn, int tn);

	CString GetBackdropBGA(PlayerNumber pn) const;
	void BeginDrawBackdrop(PlayerNumber pn);
	void EndDrawBackdrop(PlayerNumber pn);

	void GetNamesForCurrentGame(vector<CString> &IDs);
};

#endif
