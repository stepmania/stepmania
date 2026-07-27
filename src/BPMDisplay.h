#ifndef BPM_DISPLAY_H
#define BPM_DISPLAY_H

#include "BitmapText.h"
#include "AutoActor.h"
#include "ThemeMetric.h"
#include "LocalizedString.h"

#include <vector>


class Song;
class Steps;
class Course;
struct DisplayBpms;

/** @brief Displays a BPM or a range of BPMs. */
class BPMDisplay : public BitmapText
{
public:
	/** @brief Set up the BPM Display with default values. */
	BPMDisplay();
	/** @brief Copy the BPMDisplay to another. */
	virtual BPMDisplay *Copy() const;
	/** @brief Load the various metrics needed. */
	void Load();
	/**
	 * @brief Update the display as required.
	 * @param fDeltaTime the changed time.
	 */
	virtual void Update( float fDeltaTime );
	void LoadFromNode( const XNode *pNode );
	/**
	 * @brief Use the BPM[s] from a song.
	 * @param pSong the song in question.
	 */
	void SetBpmFromSong( const Song* pSong );
	/**
	 * @brief Use the BPM[s] from a steps.
	 * @param pSteps the steps in question.
	 */
	void SetBpmFromSteps( const Steps* pSteps );
	/**
	 * @brief Use the BPM[s] from a course.
	 * @param pCourse the course in question.
	 */
	void SetBpmFromCourse( const Course* pCourse );
	/**
	 * @brief Use a specified, constant BPM.
	 * @param fBPM the constant BPM.
	 */
	void SetConstantBpm( float fBPM );
	/**
	 * @brief Have the BPMDisplay cycle between various BPMs.
	 */
	void CycleRandomly();
	/** @brief Don't use a BPM at all. */
	void NoBPM();
	/** @brief Have the BPMDisplay use various BPMs. */
	void SetVarious();
	/** @brief Have the GameState determine which BPMs to display. */
	void SetFromGameState();

	// Lua
	virtual void PushSelf( lua_State *L );

protected:
	/**
	 * @brief Retrieve the active BPM on display.
	 * @return the active BPM on display.
	 */
	float GetActiveBPM() const;
	/**
	 * @brief Set the range to be used for the display.
	 * @param bpms the set of BPMs to be used.
	 */
	void SetBPMRange( const DisplayBpms &bpms );

	/** @brief The commands to use when there is no BPM. */
	ThemeMetric<apActorCommands> SET_NO_BPM_COMMAND;
	/** @brief The commands to use when there is a normal BPM. */
	ThemeMetric<apActorCommands> SET_NORMAL_COMMAND;
	/** @brief The commands to use when the BPM can change between 2 or more values. */
	ThemeMetric<apActorCommands> SET_CHANGING_COMMAND;
	/** @brief The commands to use when the BPM is random. */
	ThemeMetric<apActorCommands> SET_RANDOM_COMMAND;
	/** @brief The commands to use if it is an extra stage. */
	ThemeMetric<apActorCommands> SET_EXTRA_COMMAND;
	/** @brief A flag to determine if the BPMs cycle from low to high or just display both. */
	ThemeMetric<bool> CYCLE;
	/** @brief A flag to determine if QUESTIONMARKS_TEXT is shown. */
	ThemeMetric<bool> SHOW_QMARKS;
	/** @brief How often the random BPMs cycle themselves. */
	ThemeMetric<float> RANDOM_CYCLE_SPEED;
	ThemeMetric<float> COURSE_CYCLE_SPEED;
	/** @brief The text used to separate the low and high BPMs. */
	ThemeMetric<RString> SEPARATOR;
	/** @brief The text used when there is no BPM. */
	ThemeMetric<RString> NO_BPM_TEXT;
	/** @brief The text used when there are various BPMs for the song. */
	ThemeMetric<RString> VARIOUS_TEXT;
	/** @brief The text used when it is a random BPM. */
	ThemeMetric<RString> RANDOM_TEXT;
	/** @brief The text used as one possible option for random BPM. */
	ThemeMetric<RString> QUESTIONMARKS_TEXT;
	/** @brief The format string used for the numbers. */
	ThemeMetric<RString> BPM_FORMAT_STRING;

	/** @brief The lowest valued BPM. */
	float m_fBPMFrom;
	/** @brief The highest valued BPM. */
	float m_fBPMTo;
	/** @brief The current BPM index used. */
	int m_iCurrentBPM;
	/** @brief The list of BPMs. */
	std::vector<float> m_BPMS;
	float m_fPercentInState;
	/** @brief How long it takes to cycle the various BPMs. */
	float m_fCycleTime;
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2002
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
 */
