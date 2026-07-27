#ifndef HIGH_SCORE_H
#define HIGH_SCORE_H

#include "Grade.h"
#include "GameConstantsAndTypes.h"
#include "DateTime.h"
#include "RageUtil_AutoPtr.h"

#include <vector>


class XNode;
struct RadarValues;
struct lua_State;

struct HighScoreImpl;
/** @brief The high score that is earned by a player.
 *
 * This is scoring data that is persisted between sessions. */
struct HighScore
{
	HighScore();

	/**
	 * @brief Retrieve the name of the player that set the high score.
	 * @return the name of the player. */
	RString	GetName() const;
	/**
	 * @brief Retrieve the grade earned from this score.
	 * @return the grade.
	 */
	Grade GetGrade() const;
	/**
	 * @brief Retrieve the score earned.
	 * @return the score. */
	unsigned int GetScore() const;
	/**
	 * @brief Determine if any judgments were tallied during this run.
	 * @return true if no judgments were recorded, false otherwise. */
	bool IsEmpty() const;
	float GetPercentDP() const;
	/**
	 * @brief Determine how many seconds the player had left in Survival mode.
	 * @return the number of seconds left. */
	float GetSurviveSeconds() const;
	float GetSurvivalSeconds() const;
	unsigned int   GetMaxCombo() const;
	StageAward GetStageAward() const;
	PeakComboAward GetPeakComboAward() const;
	/**
	 * @brief Get the modifiers used for this run.
	 * @return the modifiers. */
	RString GetModifiers() const;
	DateTime GetDateTime() const;
	RString GetPlayerGuid() const;
	RString GetMachineGuid() const;
	int GetProductID() const;
	int GetTapNoteScore( TapNoteScore tns ) const;
	int GetHoldNoteScore( HoldNoteScore tns ) const;
	const RadarValues &GetRadarValues() const;
	float GetLifeRemainingSeconds() const;
	/**
	 * @brief Determine if this score was from a situation that would cause disqualification.
	 * @return true if the score would be disqualified, false otherwise. */
	bool GetDisqualified() const;

	/**
	 * @brief Set the name of the Player that earned the score.
	 * @param sName the name of the Player. */
	void SetName( const RString &sName );
	void SetGrade( Grade g );
	void SetScore( unsigned int iScore );
	void SetPercentDP( float f );
	void SetAliveSeconds( float f );
	void SetMaxCombo( unsigned int i );
	void SetStageAward( StageAward a );
	void SetPeakComboAward( PeakComboAward a );
	void SetModifiers( RString s );
	void SetDateTime( DateTime d );
	void SetPlayerGuid( RString s );
	void SetMachineGuid( RString s );
	void SetProductID( int i );
	void SetTapNoteScore( TapNoteScore tns, int i );
	void SetHoldNoteScore( HoldNoteScore tns, int i );
	void SetRadarValues( const RadarValues &rv );
	void SetLifeRemainingSeconds( float f );
	void SetDisqualified( bool b );

	RString *GetNameMutable();
	const RString *GetNameMutable() const { return const_cast<RString *> (const_cast<HighScore *>(this)->GetNameMutable()); }

	void Unset();

	bool operator<(HighScore const& other) const;
	bool operator>(HighScore const& other) const;
	bool operator<=(HighScore const& other) const;
	bool operator>=(HighScore const& other) const;
	bool operator==(HighScore const& other) const;
	bool operator!=(HighScore const& other) const;

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );

	RString GetDisplayName() const;

	// Lua
	void PushSelf( lua_State *L );
private:
	HiddenPtr<HighScoreImpl> m_Impl;
};

/** @brief The list of high scores */
struct HighScoreList
{
public:
	/**
	 * @brief Set up the HighScore List with default values.
	 *
	 * This used to call Init(), but it's better to be explicit here. */
	HighScoreList(): vHighScores(), HighGrade(Grade_NoData),
		iNumTimesPlayed(0), dtLastPlayed() {}

	void Init();

	int GetNumTimesPlayed() const
	{
		return iNumTimesPlayed;
	}
	DateTime GetLastPlayed() const
	{
		ASSERT( iNumTimesPlayed > 0 );	// don't call this unless the song has been played
		return dtLastPlayed;
	}
	const HighScore& GetTopScore() const;

	void AddHighScore( HighScore hs, int &iIndexOut, bool bIsMachine );
	void IncrementPlayCount( DateTime dtLastPlayed );
	void RemoveAllButOneOfEachName();
	void ClampSize( bool bIsMachine );

	void MergeFromOtherHSL(HighScoreList& other, bool is_machine);

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );

	std::vector<HighScore> vHighScores;
	Grade HighGrade;

	// Lua
	void PushSelf( lua_State *L );

private:
	int iNumTimesPlayed;
	DateTime dtLastPlayed;	// meaningless if iNumTimesPlayed == 0

};

/** @brief the picture taken of the high score. */
struct Screenshot
{
	/** @brief the filename of the screen shot. There is no directory part. */
	RString sFileName;
	/** @brief The MD5 hash of the screen shot file above. */
	RString sMD5;
	/** @brief The actual high score in question. */
	HighScore highScore;

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
	bool operator<(Screenshot const& rhs) const
	{
		return highScore.GetDateTime() < rhs.highScore.GetDateTime();
	}

	bool operator==(Screenshot const& rhs) const
	{
		return sFileName == rhs.sFileName;
	}
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2004
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
