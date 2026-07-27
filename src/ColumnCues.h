#ifndef COLUMN_CUES_H
#define COLUMN_CUES_H

#include "GameConstantsAndTypes.h"
#include "NoteTypes.h"
#include "NoteData.h"

/* ColumnCues are used to indicate to the player which column the next note will occur
 after a long gap in the stepchart.
 This info is made available to the theme via lua functions in Steps.cpp
 */
struct ColumnCueColumn
{
	int colNum;
	TapNoteType noteType;
	ColumnCueColumn()
	{
		colNum = 0;
		noteType = TapNoteType_Invalid;
	}
	ColumnCueColumn(int c, TapNoteType n)
	{
		colNum = c;
		noteType = n;
	}
};

struct ColumnCue
{
	float startTime;
	float duration;
	std::vector<ColumnCueColumn> columns;

	ColumnCue()
	{
		startTime = -1;
		duration = -1;
	}

	ColumnCue(float s, float d, std::vector<ColumnCueColumn> c)
	{
		startTime = s;
		duration = d;
		columns.assign(c.begin(), c.end());
	}
	/** @brief Calculates the set of ColumnCues for the given NoteData. Each "cue" is for any note that has a
	 * minimum of minDuration seconds between it and the previous note on that same column.
	*/
	static void CalculateColumnCues(const NoteData &in, std::vector<ColumnCue> &out, float minDuration);
};

#endif
