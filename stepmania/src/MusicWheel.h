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
#include "SoundSet.h"
#include "GradeDisplay.h"
#include "RageSoundStream.h"
#include "MusicSortDisplay.h"
#include "MusicStatusDisplay.h"


const int NUM_WHEEL_ITEMS_TO_DRAW	=	13;

const float FADE_TIME			=	0.5f;



class WheelItem : public ActorFrame
{
public:
	WheelItem() 
	{
		m_pSong = NULL;
	};

	void SetTintColor( D3DXCOLOR c );
	void SetDiffuseColor( D3DXCOLOR c );


	CString GetSectionName()
	{
		return m_textSectionName.GetText();
	};

	void LoadFromSong( Song &song );
	void LoadFromSectionName( CString sSectionName );


	// common
	enum WheelItemType { TYPE_SECTION, TYPE_MUSIC };
	WheelItemType m_WheelItemType;
	D3DXCOLOR m_colorTint;

	// for a section
	Sprite m_sprSectionBackground;
	BitmapText m_textSectionName;

	// for a music
	Song*				m_pSong;
	MusicStatusDisplay	m_MusicStatusDisplay;
	TextBanner			m_Banner;
	GradeDisplay		m_GradeP1;
	GradeDisplay		m_GradeP2;
};




class MusicWheel : public ActorFrame
{
public:
	MusicWheel();

	void Update( float fDeltaTime );
	void RenderPrimitives();

	void PrevMusic();
	void NextMusic();
	void NextSort();

	float GetBannerY( float fPosOffsetsFromMiddle );
	float GetBannerBrightness( float fPosOffsetsFromMiddle );
	float GetBannerAlpha( float fPosOffsetsFromMiddle );
	float GetBannerX( float fPosOffsetsFromMiddle );

	bool Select();	// return true if the selected item is a music, otherwise false
	Song* GetSelectedSong() { return m_WheelItems[m_iSelection].m_pSong; };


protected:
	void RebuildWheelItems();

	void PlayMusicSample();


	Sprite		m_sprSelectionBackground;
	Sprite		m_sprSelectionOverlay;

	MusicSortDisplay	m_MusicSortDisplay;

	
	
	CArray<WheelItem, WheelItem&> m_WheelItems;
	MusicSortOrder m_SortOrder;
	
	int	m_iSelection;
	CString m_sExpandedSectionName;


	enum WheelState { STATE_IDLE, STATE_SWITCHING_TO_PREV_MUSIC, STATE_SWITCHING_TO_NEXT_MUSIC, STATE_FADING_OFF, STATE_FADING_ON };
	WheelState m_WheelState;
	float m_fTimeLeftInState;


	// having sounds here causes a crash in Bass.  What the heck!?!?!
	SoundSet m_soundChangeMusic;
	SoundSet m_soundChangeSort;
	SoundSet m_soundExpand;



	CString GetSectionNameFromSongAndSort( Song* pSong, MusicSortOrder order )
	{
		CString sTemp;

		switch( m_SortOrder )
		{
		case SORT_GROUP:	
			sTemp = pSong->GetGroupName();
			sTemp.MakeUpper();
			if( sTemp.GetLength() > 0 )
				return sTemp;
			else
				return " ";
		case SORT_ARTIST:	
			sTemp = pSong->GetArtist();
			sTemp.MakeUpper();
			if( sTemp.GetLength() > 0 )
				return sTemp[0];
			else
				return " ";
		case SORT_TITLE:	
			sTemp = pSong->GetTitle();
			sTemp.MakeUpper();
			if( sTemp.GetLength() > 0 )
				return sTemp[0];
			else
				return " ";
		case SORT_BPM:
		case SORT_MOST_PLAYED:
		default:
			return " ";
		}
	};


	CTypedPtrMap<CMapStringToPtr, CString, D3DXCOLOR*> m_mapGroupNameToColorPtr;


};


#endif
