#ifndef NOTEFIELD_POSITIONING_H
#define NOTEFIELD_POSITIONING_H

#include "RageMath.h"
#include "PlayerNumber.h"
#include "StyleDef.h"
#include "PlayerNumber.h"
#include "Style.h"

#include <set>

class NoteFieldPositioning
{
	struct Mode
	{
		Mode();
		bool MatchesCurrentGame() const;

		void BeginDrawTrack(int tn) const;
		void EndDrawTrack(int tn) const;

		CString name;
		set<Style> Styles;

		RageMatrix m_Position[MAX_NOTE_TRACKS];
		/* 0 = no perspective */
		float m_fFov[MAX_NOTE_TRACKS];
		RageMatrix m_PerspPosition[MAX_NOTE_TRACKS];
	};

	vector<Mode> Modes;

public:
	NoteFieldPositioning(CString fn);

	/* Get the mode number for the given positioning type (for the current
	 * game and style). */
	int GetID(const CString &name) const;
	int GetID(PlayerNumber pn) const;

	void BeginDrawTrack(PlayerNumber pn, int tn) const;
	void EndDrawTrack(PlayerNumber pn, int tn) const;

	void GetNamesForCurrentGame(vector<CString> &IDs);
};

#endif
