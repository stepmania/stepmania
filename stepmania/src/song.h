/*
-----------------------------------------------------------------------------
 File: Song.h

 Desc: Holds metadata for a song and the song's step data.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _SONG_H_
#define _SONG_H_


#include "Steps.h"
class Steps;	// why is this needed?

#include "GameInfo.h"	// for definition of GameMode
enum GameMode;	// why is this needed?



struct Grade
{
	Grade() { m_GradeType = GRADE_NONE; };
	CString ToString() 
	{
		switch( m_GradeType )
		{
		case GRADE_AAA:		return "AAA";
		case GRADE_AA:		return "AA";
		case GRADE_A:		return "A";
		case GRADE_B:		return "B";
		case GRADE_C:		return "C";
		case GRADE_D:		return "D";
		case GRADE_E:		return "E";
		case GRADE_NONE:	return "N";
		default:			return "N";
		}
	};
	void FromString( CString sGradeString )
	{
		sGradeString.MakeUpper();
		if	   ( sGradeString == "AAA" )	m_GradeType = GRADE_AAA;
		else if( sGradeString == "AA" )		m_GradeType = GRADE_AA;
		else if( sGradeString == "A" )		m_GradeType = GRADE_A;
		else if( sGradeString == "B" )		m_GradeType = GRADE_B;
		else if( sGradeString == "C" )		m_GradeType = GRADE_C;
		else if( sGradeString == "D" )		m_GradeType = GRADE_D;
		else if( sGradeString == "E" )		m_GradeType = GRADE_E;
		else if( sGradeString == "N" )		m_GradeType = GRADE_NONE;
		else								m_GradeType = GRADE_NONE;
	}

	enum GradeType { GRADE_NONE=0, GRADE_E, GRADE_D, GRADE_C, GRADE_B, GRADE_A, GRADE_AA, GRADE_AAA };
	GradeType m_GradeType;
};

struct BPMSegment 
{
	BPMSegment() { m_fStartBeat = m_fBPM = -1; };
	float m_fStartBeat;
	float m_fBPM;
};

struct FreezeSegment 
{
	FreezeSegment() { m_fStartBeat = m_fFreezeSeconds = -1; };
	float m_fStartBeat;
	float m_fFreezeSeconds;
};


class Song
{
public:
	Song();

	bool LoadFromSongDir( CString sDir );
	bool LoadSongInfoFromDWIFile( CString sPath );
	bool LoadSongInfoFromBMSDir( CString sDir );
private:

	void TidyUpData();

public:
	CString GetSongFilePath()	{return m_sSongFilePath; };
	CString GetSongFileDir()	{return m_sSongDir; };
	CString GetGroupName()		{return m_sGroupName; };
	CString GetMusicPath()		{return m_sMusicPath; };
	CString GetBannerPath()		{return m_sBannerPath; };
	CString GetBackgroundPath()	{return m_sBackgroundPath; };
	bool BackgroundIsAMovie()	{return m_sBackgroundPath.Right(3) == "avi" || 
										m_sBackgroundPath.Right(3) == "mpg" ||
										m_sBackgroundPath.Right(4) == "mpeg";	};


	bool HasMusic()			{return m_sMusicPath != ""		&&	DoesFileExist(GetMusicPath()); };
	bool HasBanner()		{return m_sBannerPath != ""		&&  DoesFileExist(GetBannerPath()); };
	bool HasBackground()	{return m_sBackgroundPath != ""	&&  DoesFileExist(GetBackgroundPath()); };


	CString GetTitle()			{return m_sTitle; };
	CString GetArtist()			{return m_sArtist; };
	CString GetCreator()		{return m_sCreator; };
	float GetBeatOffsetInSeconds()	{return m_fOffsetInSeconds; };
	void SetBeatOffsetInSeconds(float fNewOffset)	{m_fOffsetInSeconds = fNewOffset; };
	void GetMinMaxBPM( float &fMinBPM, float &fMaxBPM )
	{
		fMaxBPM = 0;
		fMinBPM = 100000;	// inf
		for( int i=0; i<m_BPMSegments.GetSize(); i++ ) 
		{
			BPMSegment &seg = m_BPMSegments[i];
			fMaxBPM = max( seg.m_fBPM, fMaxBPM );
			fMinBPM = min( seg.m_fBPM, fMinBPM );
		}
	};
	void GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut );
	void GetStepsThatMatchGameMode( GameMode gm, CArray<Steps*, Steps*&>& arrayAddTo );
	void GetNumFeet( GameMode gm, int& iDiffEasy, int& iDiffMedium, int& iDiffHard );
	

public:
	// Song statistics:
	int m_iMaxCombo, m_iTopScore, m_iNumTimesPlayed;
	bool m_bHasBeenLoadedBefore;
	Grade m_TopGrade;


private:
	CString m_sSongFilePath;
	CString m_sSongDir;
	CString m_sGroupName;

	bool	m_bChangedSinceSave;
	CString	m_sTitle;
	CString	m_sArtist;
	CString	m_sCreator;
//	float	m_fBPM;
	float	m_fOffsetInSeconds;

	CString	m_sMusicPath;
	CString	m_sBannerPath;
	CString	m_sBackgroundPath;

	CArray<BPMSegment, BPMSegment&> m_BPMSegments;	// this must be sorted before dancing
	CArray<FreezeSegment, FreezeSegment&> m_FreezeSegments;	// this must be sorted before dancing

public:
	CArray<Steps, Steps&> arraySteps;
};



#endif