#ifndef NOTES_LOADER_H
#define NOTES_LOADER_H

#include "song.h"
#include "RageUtil.h"
#include <set>

typedef int DanceNote;
// MD 10/26/03 - this structure is only correct for DDR - use the lower enum instead
enum {
	DANCE_NOTE_NONE = 0,
	DANCE_NOTE_PAD1_LEFT,
	DANCE_NOTE_PAD1_UPLEFT,
	DANCE_NOTE_PAD1_DOWN,
	DANCE_NOTE_PAD1_UP,
	DANCE_NOTE_PAD1_UPRIGHT,
	DANCE_NOTE_PAD1_RIGHT,
	DANCE_NOTE_PAD2_LEFT,
	DANCE_NOTE_PAD2_UPLEFT,
	DANCE_NOTE_PAD2_DOWN,
	DANCE_NOTE_PAD2_UP,
	DANCE_NOTE_PAD2_UPRIGHT,
	DANCE_NOTE_PAD2_RIGHT
};
enum {
	BMS_NULL_COLUMN = 0,
	BMS_P1_KEY1,
	BMS_P1_KEY2,
	BMS_P1_KEY3,
	BMS_P1_KEY4,
	BMS_P1_KEY5,
	BMS_P1_TURN,
	BMS_P1_KEY6,
	BMS_P1_KEY7,
	BMS_P2_KEY1,
	BMS_P2_KEY2,
	BMS_P2_KEY3,
	BMS_P2_KEY4,
	BMS_P2_KEY5,
	BMS_P2_TURN,
	BMS_P2_KEY6,
	BMS_P2_KEY7,
};

class NotesLoader {
protected:
	virtual void GetApplicableFiles( CString sPath, CStringArray &out )=0;

	set<istring> BlacklistedImages;

public:
	virtual ~NotesLoader() { }
	const set<istring> &GetBlacklistedImages() const { return BlacklistedImages; }
	static void GetMainAndSubTitlesFromFullTitle( const CString sFullTitle, CString &sMainTitleOut, CString &sSubTitleOut );
	virtual bool LoadFromDir( CString sPath, Song &out ) = 0;
	bool Loadable( CString sPath );
};

#endif
