#include "global.h"
#include "ColumnCues.h"
#include "GameState.h"
#include "TimingData.h"

void ColumnCue::CalculateColumnCues(const NoteData &in, std::vector<ColumnCue> &out, float minDuration)
{
	TimingData *timing = GAMESTATE->GetProcessedTimingData();
	NoteData::all_tracks_const_iterator curr_note = in.GetTapNoteRangeAllTracks(0, MAX_NOTE_ROW);
	int curr_row = -1;

	std::vector<ColumnCue> allColumnCues;
	ColumnCue currentCue = ColumnCue();

	while (!curr_note.IsAtEnd())
	{
		if(curr_note.Row() != curr_row)
		{
			if (currentCue.startTime != -1 )
			{
				allColumnCues.push_back(currentCue);
			}
			currentCue = ColumnCue();
			currentCue.startTime = timing->GetElapsedTimeFromBeat(NoteRowToBeat(curr_note.Row()));
			curr_row = curr_note.Row();
		}

		
		if (curr_note->type == TapNoteType_Tap
			|| curr_note->type == TapNoteType_HoldHead
			|| curr_note->type == TapNoteType_Mine
			|| curr_note->type == TapNoteType_Lift)
		{
			currentCue.columns.push_back(ColumnCueColumn(curr_note.Track() + 1, curr_note->type));
		}
		
		++curr_note;
	}

	// If there's a remaining columnCue from the last row, add it
	if( currentCue.startTime != -1)
	{
		allColumnCues.push_back(currentCue);
	}

	float previousCueTime = 0;
	std::vector<ColumnCue> columnCues;
	for( ColumnCue columnCue : allColumnCues )
	{
		float duration = columnCue.startTime - previousCueTime;
		if( duration > minDuration || previousCueTime == 0)
		{
			columnCues.push_back(ColumnCue(previousCueTime, duration, columnCue.columns));
		}
		previousCueTime = columnCue.startTime;
	}

	out.clear();
	out.assign(columnCues.begin(), columnCues.end());
}
