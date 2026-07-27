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

#include <vector>


/** @brief What type of row is needed for the EditMenu? */
enum EditMenuRow
{
	ROW_GROUP,
	ROW_SONG,
	ROW_STEPS_TYPE,
	ROW_STEPS,
	ROW_SOURCE_STEPS_TYPE,
	ROW_SOURCE_STEPS,
	ROW_ACTION,
	NUM_EditMenuRow /**< The number of EditMenuRows available. */
};
/** @brief Loop through each EditMenuRow. */
#define FOREACH_EditMenuRow( r ) FOREACH_ENUM( EditMenuRow, r )
/**
 * @brief Turn the EditMenuRow into a string.
 * @param r the row.
 * @return the string. */
const RString& EditMenuRowToString( EditMenuRow r );
/**
 * @brief Turn the EditMenuRow into a localized string.
 * @param r the row.
 * @return the localized string. */
const RString& EditMenuRowToLocalizedString( EditMenuRow r );

/** @brief The different actions one can take on a step. */
enum EditMenuAction
{
	EditMenuAction_Edit, /**< Modify the current step for the Song. */
	EditMenuAction_Delete, /**< Remove the current step from the Song. */
	EditMenuAction_Create, /**< Create a new step for the Song. */
	EditMenuAction_Practice, /**< Practice the current step for the Song. */
	EditMenuAction_LoadAutosave,
	NUM_EditMenuAction, /**< The number of MenuActions available to choose from. */
	EditMenuAction_Invalid
};
/** @brief Loop through each EditMenuAction. */
#define FOREACH_EditMenuAction( ema ) FOREACH_ENUM( EditMenuAction, ema )
/**
 * @brief Turn the EditMenuAction into a string.
 * @param ema the action.
 * @return the string. */
const RString& EditMenuActionToString( EditMenuAction ema );
/**
 * @brief Turn the EditMenuAction into a localized string.
 * @param ema the action.
 * @return the localized string. */
const RString& EditMenuActionToLocalizedString( EditMenuAction ema );

/** @brief How many arrows are used for the EditMenu? */
const int NUM_ARROWS = 2;

/**
 * @brief UI on Edit Menu screen.
 *
 * Create Steps, delete Steps, or launch Steps in editor. */
class EditMenu: public ActorFrame
{
public:
	/** @brief Set up the EditMenu. */
	EditMenu();
	/** @brief Destroy the EditMenu. */
	~EditMenu();
	void Load( const RString &sType );

	/** @brief Determine if we can move up.
	 * @return true if we can, false otherwise. */
	bool CanGoUp();
	/** @brief Determine if we can move down.
	 * @return true if we can, false otherwise. */
	bool CanGoDown();
	/** @brief Determine if we can move left.
	 * @return true if we can, false otherwise. */
	bool CanGoLeft();
	/** @brief Determine if we can move right.
	 * @return true if we can, false otherwise. */
	bool CanGoRight();
	/** @brief Determine if the EditMenuRow is selectable.
	 * @param row the row in question.
	 * @return true if it can be selected, false otherwise. */
	bool RowIsSelectable( EditMenuRow row );

	/** @brief Move up to the next selection. */
	void Up();
	/** @brief Move down to the next selection. */
	void Down();
	/** @brief Move left to the next selection. */
	void Left();
	/** @brief Move right to the next selection. */
	void Right();

	void RefreshAll();

	bool SafeToUse();

#define RETURN_IF_INVALID(check, retval) if(check) { return retval; }

	/** @brief Retrieve the currently selected group.
	 * @return the current group. */
	RString	GetSelectedGroup() const
	{
		if( !SHOW_GROUPS.GetValue() ) return GROUP_ALL;
		int groups = static_cast<int>(m_sGroups.size());
		RETURN_IF_INVALID(m_iSelection[ROW_GROUP] >= groups, "");
		return m_sGroups[m_iSelection[ROW_GROUP]];
	}
	/** @brief Retrieve the currently selected song.
	 * @return the current song. */
	Song*	GetSelectedSong() const
	{
		RETURN_IF_INVALID(m_pSongs.empty() ||
			m_iSelection[ROW_SONG] >= (int)m_pSongs.size(), nullptr);
		return m_pSongs[m_iSelection[ROW_SONG]];
	}
	/** @brief Retrieve the currently selected steps type.
	 * @return the current steps type. */
	StepsType GetSelectedStepsType() const
	{
		RETURN_IF_INVALID(m_StepsTypes.empty() ||
			m_iSelection[ROW_STEPS_TYPE] >= (int)m_StepsTypes.size(), StepsType_Invalid);
		return m_StepsTypes[m_iSelection[ROW_STEPS_TYPE]];
	}
	/** @brief Retrieve the currently selected steps.
	 * @return the current steps. */
	Steps*	GetSelectedSteps() const
	{
		RETURN_IF_INVALID(m_vpSteps.empty() ||
			m_iSelection[ROW_STEPS] >= (int)m_vpSteps.size(), nullptr);
		return m_vpSteps[m_iSelection[ROW_STEPS]].pSteps;
	}
	/** @brief Retrieve the currently selected difficulty.
	 * @return the current difficulty. */
	Difficulty GetSelectedDifficulty() const
	{
		RETURN_IF_INVALID(m_vpSteps.empty() ||
			m_iSelection[ROW_STEPS] >= (int)m_vpSteps.size(), Difficulty_Invalid);
		return m_vpSteps[m_iSelection[ROW_STEPS]].dc;
	}
	/** @brief Retrieve the currently selected source steps type.
	 * @return the current source steps type. */
	StepsType GetSelectedSourceStepsType() const
	{
		RETURN_IF_INVALID(m_StepsTypes.empty() ||
			m_iSelection[ROW_SOURCE_STEPS_TYPE] >= (int)m_StepsTypes.size(), StepsType_Invalid);
		return m_StepsTypes[m_iSelection[ROW_SOURCE_STEPS_TYPE]];
	}
	/** @brief Retrieve the currently selected source steps.
	 * @return the current source steps. */
	Steps* GetSelectedSourceSteps() const
	{
		RETURN_IF_INVALID(m_vpSourceSteps.empty() ||
			m_iSelection[ROW_SOURCE_STEPS] >= (int)m_vpSourceSteps.size(), nullptr);
		return m_vpSourceSteps[m_iSelection[ROW_SOURCE_STEPS]].pSteps;
	}
	/** @brief Retrieve the currently selected difficulty.
	 * @return the current difficulty. */
	Difficulty GetSelectedSourceDifficulty() const
	{
		RETURN_IF_INVALID(m_vpSourceSteps.empty() ||
			m_iSelection[ROW_SOURCE_STEPS] >= (int)m_vpSourceSteps.size(), Difficulty_Invalid);
		return m_vpSourceSteps[m_iSelection[ROW_SOURCE_STEPS]].dc;
	}
	/** @brief Retrieve the currently selected action.
	 * @return the current action. */
	EditMenuAction GetSelectedAction() const
	{
		RETURN_IF_INVALID(m_Actions.empty() ||
			m_iSelection[ROW_ACTION]	>= (int)m_Actions.size(), EditMenuAction_Invalid);
		return m_Actions[m_iSelection[ROW_ACTION]];
	}

#undef RETURN_IF_INVALID

	/** @brief Retrieve the currently selected row.
	 * @return the current row. */
	EditMenuRow GetSelectedRow() const { return m_SelectedRow; }

private:
	struct StepsAndDifficulty;

	void StripLockedStepsAndDifficulty( std::vector<StepsAndDifficulty> &v );
	void GetSongsToShowForGroup( const RString &sGroup, std::vector<Song*> &vpSongsOut );
	void GetGroupsToShow( std::vector<RString> &vsGroupsOut );

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
	std::vector<RString>			m_sGroups;
	/** @brief The list of Songs in a group. */
	std::vector<Song*>			m_pSongs;
	std::vector<StepsType>		m_StepsTypes;
	std::vector<StepsAndDifficulty>	m_vpSteps;
	std::vector<StepsAndDifficulty>	m_vpSourceSteps;
	std::vector<EditMenuAction>		m_Actions;

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
