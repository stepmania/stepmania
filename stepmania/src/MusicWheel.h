/*
-----------------------------------------------------------------------------
 File: MusicWheel.h

 Desc: A graphic displayed in the MusicWheel during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _MusicWheel_H_
#define _MusicWheel_H_


#include "Sprite.h"
#include "Song.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "Rectangle.h"
#include "TextBanner.h"
#include "RandomSample.h"
#include "GradeDisplay.h"
#include "RageSoundStream.h"
#include "GameTypes.h"
#include "MusicSortDisplay.h"
#include "MusicStatusDisplay.h"
#include "Window.h"	// for WindowMessage

const int NUM_WHEEL_ITEMS_TO_DRAW	=	13;

const float FADE_TIME			=	0.5f;

const WindowMessage	SM_SongChanged		=	WindowMessage(SM_User+47);	// this should be unique!
const WindowMessage SM_PlaySongSample	=	WindowMessage(SM_User+48);	


enum WheelItemType { TYPE_SECTION, TYPE_MUSIC };


struct WheelItemData
{
public:
	WheelItemData();

	void LoadFromSectionName( CString sSectionName );
	void LoadFromSong( Song* pSong );

	WheelItemType	m_WheelItemType;
	D3DXCOLOR		m_colorTint;
	CString			m_sSectionName;
	Song*			m_pSong;
	MusicStatusDisplayType m_MusicStatusDisplayType;
};


class WheelItemDisplay : public Actor,
						 public WheelItemData
{
public:
	WheelItemDisplay();

	virtual void Update( float fDeltaTime );
	virtual void RenderPrimitives();

	void SetDiffuseColor( D3DXCOLOR c );

	CString GetSectionName()
	{
		return m_sSectionName;
	};

	void LoadFromWheelItemData( WheelItemData* pWID );

	// for a section
	Sprite m_sprSectionBackground;
	BitmapText m_textSectionName;

	// for a music
	MusicStatusDisplay	m_MusicStatusDisplay;
	TextBanner			m_Banner;
};




class MusicWheel : public ActorFrame
{
public:
	MusicWheel();
	~MusicWheel();

	virtual void Update( float fDeltaTime );
	virtual void RenderPrimitives();

	virtual void TweenOnScreen();		
	virtual void TweenOffScreen();

	void PrevMusic();
	void NextMusic();
	void NextSort();

	float GetBannerY( float fPosOffsetsFromMiddle );
	float GetBannerBrightness( float fPosOffsetsFromMiddle );
	float GetBannerAlpha( float fPosOffsetsFromMiddle );
	float GetBannerX( float fPosOffsetsFromMiddle );

	bool Select();	// return true if the selected item is a music, otherwise false
	Song* GetSelectedSong() { return GetCurWheelItemDatas()[m_iSelection].m_pSong; };
	CString GetSelectedSection() { return GetCurWheelItemDatas()[m_iSelection].m_sSectionName; };


protected:
	void BuildWheelItemDatas( CArray<WheelItemData, WheelItemData&> &arrayWheelItems, SongSortOrder so );
	void RebuildWheelItemDisplays();
	void SwitchSortOrder();


	Sprite		m_sprSelectionBackground;
	Sprite		m_sprSelectionOverlay;

	MusicSortDisplay	m_MusicSortDisplay;

	SongSortOrder m_SortOrder;

	CArray<WheelItemData, WheelItemData&> m_WheelItemDatas[NUM_SORT_ORDERS];
	CArray<WheelItemData, WheelItemData&> &GetCurWheelItemDatas() { return m_WheelItemDatas[m_SortOrder]; };
	
	WheelItemDisplay m_WheelItemDisplays[NUM_WHEEL_ITEMS_TO_DRAW];
	
	int	m_iSelection;		// index into GetCurWheelItemDatas()
	CString m_sExpandedSectionName;


	enum WheelState { 
		STATE_IDLE, 
		STATE_SWITCHING_MUSIC, 
		STATE_FLYING_OFF_BEFORE_NEXT_SORT, 
		STATE_FLYING_ON_AFTER_NEXT_SORT, 
		STATE_TWEENING_ON_SCREEN, 
		STATE_TWEENING_OFF_SCREEN, 
		STATE_WAITING_OFF_SCREEN
	};
	WheelState m_WheelState;
	float m_fTimeLeftInState;
	float m_fPositionOffsetFromSelection;


	// having sounds here causes a crash in Bass.  What the heck!?!?!
	RandomSample m_soundChangeMusic;
	RandomSample m_soundChangeSort;
	RandomSample m_soundExpand;



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
			sTemp = pSong->GetTitle();
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


	CTypedPtrMap<CMapStringToPtr, CString, D3DXCOLOR*> m_mapGroupNameToColorPtr;


};


#endif
