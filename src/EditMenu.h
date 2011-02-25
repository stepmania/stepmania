#ifndef EDIT_MENU_H
#define EDIT_MENU_H

#include "ActorFrame.h"
#include "FadingBanner.h"
#include "TextBanner.h"
#include "GameConstantsAndTypes.h"
#include "StepsDisplay.h"
#include "RageSound.h"
#include "EnumHelper.h"
#include "ThemeMetric.h"

enum EditMenuRow 
{ 
	ROW_GROUP, 
	ROW_SONG, 
	ROW_STEPS_TYPE, 
	ROW_STEPS,
	ROW_SOURCE_STEPS_TYPE, 
	ROW_SOURCE_STEPS, 
	ROW_ACTION, 
	NUM_EditMenuRow 
};
/** @brief Loop through each EditMenuRow. */
#define FOREACH_EditMenuRow( r ) FOREACH_ENUM( EditMenuRow, r )
const RString& EditMenuRowToString( EditMenuRow r );
const RString& EditMenuRowToLocalizedString( EditMenuRow r );

/** @brief The different actions one can take on a step. */
enum EditMenuAction
{
	EditMenuAction_Edit, /**< Modify the current step for the Song. */
	EditMenuAction_Delete, /**< Remove the current step from the Song. */
	EditMenuAction_Create, /**< Create a new step for the Song. */
	EditMenuAction_Practice, /**< Practice the current step for the Song. */
	NUM_EditMenuAction, /**< The number of MenuActions available to choose from. */
	EditMenuAction_Invalid
};
/** @brief Loop through each EditMenuAction. */
#define FOREACH_EditMenuAction( ema ) FOREACH_ENUM( EditMenuAction, ema )
const RString& EditMenuActionToString( EditMenuAction ema );
const RString& EditMenuActionToLocalizedString( EditMenuAction ema );

const int NUM_ARROWS = 2;

/**
 * @brief UI on Edit Menu screen. 
 *
 * Create Steps, delete Steps, or launch Steps in editor. */
class EditMenu: public ActorFrame 
{
public:
	EditMenu();
	~EditMenu();
	void Load( const RString &sType );

	bool CanGoUp();
	bool CanGoDown();
	bool CanGoLeft();
	bool CanGoRight();
	bool RowIsSelectable( EditMenuRow row );

	void Up();
	void Down();
	void Left();
	void Right();

	void RefreshAll();

	RString		GetSelectedGroup() const
	{ 
		if( !SHOW_GROUPS.GetValue() ) return GROUP_ALL; 
		ASSERT_M((int)m_iSelection[ROW_GROUP] < (int)m_sGroups.size(),
			 ssprintf("Group selection %d < Number of groups %d", m_iSelection[ROW_GROUP], (int)m_sGroups.size())); 
		return m_sGroups[m_iSelection[ROW_GROUP]]; 
	}
	Song*		GetSelectedSong() const			{ ASSERT(m_iSelection[ROW_SONG]			< (int)m_pSongs.size());	return m_pSongs[m_iSelection[ROW_SONG]]; }
	StepsType	GetSelectedStepsType() const		{ ASSERT(m_iSelection[ROW_STEPS_TYPE]		< (int)m_StepsTypes.size());	return m_StepsTypes[m_iSelection[ROW_STEPS_TYPE]]; }
	Steps*		GetSelectedSteps() const		{ ASSERT(m_iSelection[ROW_STEPS]		< (int)m_vpSteps.size());	return m_vpSteps[m_iSelection[ROW_STEPS]].pSteps; }
	Difficulty	GetSelectedDifficulty() const		{ ASSERT(m_iSelection[ROW_STEPS]		< (int)m_vpSteps.size());	return m_vpSteps[m_iSelection[ROW_STEPS]].dc; }
	StepsType	GetSelectedSourceStepsType() const	{ ASSERT(m_iSelection[ROW_SOURCE_STEPS_TYPE]	< (int)m_StepsTypes.size());	return m_StepsTypes[m_iSelection[ROW_SOURCE_STEPS_TYPE]]; }
	Steps*		GetSelectedSourceSteps() const		{ ASSERT(m_iSelection[ROW_SOURCE_STEPS]		< (int)m_vpSourceSteps.size());	return m_vpSourceSteps[m_iSelection[ROW_SOURCE_STEPS]].pSteps; }
	Difficulty	GetSelectedSourceDifficulty() const	{ ASSERT(m_iSelection[ROW_SOURCE_STEPS]		< (int)m_vpSourceSteps.size());	return m_vpSourceSteps[m_iSelection[ROW_SOURCE_STEPS]].dc; }
	EditMenuAction	GetSelectedAction() const		{ ASSERT(m_iSelection[ROW_ACTION]		< (int)m_Actions.size());	return m_Actions[m_iSelection[ROW_ACTION]]; }

	EditMenuRow GetSelectedRow() const { return m_SelectedRow; }

private:
	struct StepsAndDifficulty;

	void StripLockedStepsAndDifficulty( vector<StepsAndDifficulty> &v );
	void GetSongsToShowForGroup( const RString &sGroup, vector<Song*> &vpSongsOut );
	void GetGroupsToShow( vector<RString> &vsGroupsOut );

	void UpdateArrows();
	AutoActor	m_sprArrows[NUM_ARROWS];

	EditMenuRow m_SelectedRow;
	EditMenuRow GetFirstRow() const { return SHOW_GROUPS.GetValue()? ROW_GROUP:ROW_SONG; }
	int GetRowSize( EditMenuRow er ) const;
	int		m_iSelection[NUM_EditMenuRow];
	BitmapText	m_textLabel[NUM_EditMenuRow];
	BitmapText	m_textValue[NUM_EditMenuRow];

	/** @brief The group's banner. */
	FadingBanner	m_GroupBanner;
	/** @brief The Song's banner. */
	FadingBanner	m_SongBanner;
	TextBanner	m_SongTextBanner;
	StepsDisplay	m_StepsDisplay;
	StepsDisplay	m_StepsDisplaySource;

	struct StepsAndDifficulty
	{
		StepsAndDifficulty( Steps *s, Difficulty d ) { pSteps = s; dc = d; }
		Steps *pSteps;
		Difficulty dc;
	};

	/** @brief The list of groups. */
	vector<RString>			m_sGroups;
	/** @brief The list of Songs in a group. */
	vector<Song*>			m_pSongs;
	vector<StepsType>		m_StepsTypes;
	vector<StepsAndDifficulty>	m_vpSteps;
	vector<StepsAndDifficulty>	m_vpSourceSteps;
	vector<EditMenuAction>		m_Actions;

	void OnRowValueChanged( EditMenuRow row );
	void ChangeToRow( EditMenuRow newRow );

	RageSound m_soundChangeRow;
	RageSound m_soundChangeValue;

	/** @brief A metric to determine if groups are shown. */
	ThemeMetric<bool> SHOW_GROUPS;
	ThemeMetric1D<float> ARROWS_X;
	ThemeMetric<apActorCommands> ARROWS_ENABLED_COMMAND;
	ThemeMetric<apActorCommands> ARROWS_DISABLED_COMMAND;
	ThemeMetric1D<float> ROW_Y;
public:
	ThemeMetric<EditMode> EDIT_MODE;
	ThemeMetric<RString>  TEXT_BANNER_TYPE;
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
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
