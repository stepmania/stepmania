#pragma once
/*
-----------------------------------------------------------------------------
 Class: MusicWheel

 Desc: A wheel with song names used in the Select Music screen.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "Song.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "Quad.h"
#include "TextBanner.h"
#include "RandomSample.h"
#include "GradeDisplay.h"
#include "RageSoundStream.h"
#include "GameConstantsAndTypes.h"
#include "MusicSortDisplay.h"
#include "MusicStatusDisplay.h"
#include "Screen.h"	// for ScreenMessage
#include "ScoreDisplayRolling.h"
#include "ScrollBar.h"


const int NUM_WHEEL_ITEMS_TO_DRAW	=	13;


const ScreenMessage	SM_SongChanged		=	ScreenMessage(SM_User+47);	// this should be unique!
const ScreenMessage SM_PlaySongSample	=	ScreenMessage(SM_User+48);	


enum WheelItemType { TYPE_SECTION, TYPE_SONG, TYPE_ROULETTE };


struct WheelItemData
{
public:
	WheelItemData();

	void Load( WheelItemType wit, Song* pSong, const CString &sSectionName, const D3DXCOLOR color );

	WheelItemType	m_WheelItemType;
	CString			m_sSectionName;
	Song*			m_pSong;
	D3DXCOLOR		m_color;	// either text color or section background color
	MusicStatusDisplayType m_MusicStatusDisplayType;
};


class WheelItemDisplay : public Actor,
						 public WheelItemData
{
public:
	WheelItemDisplay();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	CString GetSectionName() { return m_sSectionName; }

	void LoadFromWheelItemData( WheelItemData* pWID );
	void RefreshGrades();

	// for TYPE_SECTION and TYPE_ROULETTE
	Sprite				m_sprSectionBar;
	// for TYPE_SECTION
	BitmapText			m_textSectionName;
	// for TYPE_ROULETTE
	BitmapText			m_textRoulette;

	// for a TYPE_MUSIC
	Sprite				m_sprSongBar;
	MusicStatusDisplay	m_MusicStatusDisplay;
	TextBanner			m_TextBanner;
	GradeDisplay		m_GradeDisplay[NUM_PLAYERS];
};




class MusicWheel : public ActorFrame
{
public:
	MusicWheel();
	~MusicWheel();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void TweenOnScreen();		
	virtual void TweenOffScreen();

	void PrevMusic();
	void NextMusic();
	void PrevSort();
	void NextSort();
	void NotesChanged( PlayerNumber pn );	// update grade graphics and top score

	float GetBannerX( float fPosOffsetsFromMiddle );
	float GetBannerY( float fPosOffsetsFromMiddle );
	float GetBannerBrightness( float fPosOffsetsFromMiddle );
	float GetBannerAlpha( float fPosOffsetsFromMiddle );

	bool Select();	// return true if the selected item is a music, otherwise false
	WheelItemType	GetSelectedType()	{ return GetCurWheelItemDatas()[m_iSelection].m_WheelItemType; };
	Song*			GetSelectedSong()	{ return GetCurWheelItemDatas()[m_iSelection].m_pSong; };
	CString			GetSelectedSection(){ return GetCurWheelItemDatas()[m_iSelection].m_sSectionName; };


protected:
	void BuildWheelItemDatas( CArray<WheelItemData, WheelItemData&> &arrayWheelItems, SongSortOrder so );
	void RebuildWheelItemDisplays();
	void SwitchSortOrder();


	ScrollBar	m_ScrollBar;

	Sprite		m_sprSelectionOverlay;

	MusicSortDisplay	m_MusicSortDisplay;
	Sprite				m_sprHighScoreFrame[NUM_PLAYERS];
	ScoreDisplayRolling	m_HighScore[NUM_PLAYERS];

	SongSortOrder		m_SortOrder;

	CArray<WheelItemData, WheelItemData&> m_WheelItemDatas[NUM_SORT_ORDERS];
	CArray<WheelItemData, WheelItemData&> &GetCurWheelItemDatas() { return m_WheelItemDatas[m_SortOrder]; };
	
	WheelItemDisplay	m_WheelItemDisplays[NUM_WHEEL_ITEMS_TO_DRAW];
	
	int					m_iSelection;		// index into GetCurWheelItemDatas()
	CString				m_sExpandedSectionName;


	enum WheelState { 
		STATE_IDLE, 
		STATE_SWITCHING_MUSIC, 
		STATE_FLYING_OFF_BEFORE_NEXT_SORT, 
		STATE_FLYING_ON_AFTER_NEXT_SORT, 
		STATE_TWEENING_ON_SCREEN, 
		STATE_TWEENING_OFF_SCREEN, 
		STATE_WAITING_OFF_SCREEN
	};
	WheelState			m_WheelState;
	float				m_fTimeLeftInState;
	float				m_fPositionOffsetFromSelection;


	// having sounds here causes a crash in Bass.  What the heck!?!?!
	RageSoundSample m_soundChangeMusic;
	RageSoundSample m_soundChangeSort;
	RageSoundSample m_soundExpand;



	CString GetSectionNameFromSongAndSort( Song* pSong, SongSortOrder so )
	{
		if( pSong == NULL )
			return "";

		CString sTemp;

		switch( so )
		{
		case SORT_GROUP:	
			sTemp = pSong->GetGroupName();
			return sTemp;
//		case SORT_ARTIST:	
//			sTemp = pSong->GetArtist();
//			sTemp.MakeUpper();
//			sTemp =  (sTemp.GetLength() > 0) ? sTemp.Left(1) : "";
//			if( IsAnInt(sTemp) )
//				sTemp = "NUM";
//			return sTemp;
		case SORT_TITLE:	
			sTemp = pSong->GetMainTitle();
			sTemp.MakeUpper();
			sTemp = (sTemp.GetLength() > 0) ? sTemp.Left(1) : "";
			if( IsAnInt(sTemp) )
				sTemp = "NUM";
			return sTemp;
		case SORT_BPM:
		case SORT_MOST_PLAYED:
		default:
			return "";
		}
	};

};

