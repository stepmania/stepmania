#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: MusicWheel.h

 Desc: A graphic displayed in the MusicWheel during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "MusicWheel.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "GameManager.h"
#include "ScreenDimensions.h"
#include "ThemeManager.h"
#include "RageMusic.h"
#include "WindowManager.h"	// for sending SM_PlayMusicSample
#include "RageHelper.h"




const float SWITCH_MUSIC_TIME	=	0.3f;

const float SORT_ICON_ON_SCREEN_X	=	-395;
const float SORT_ICON_ON_SCREEN_Y	=	-176;

const float SORT_ICON_OFF_SCREEN_X	=	SORT_ICON_ON_SCREEN_X - 200;
const float SORT_ICON_OFF_SCREEN_Y	=	SORT_ICON_ON_SCREEN_Y;




D3DXCOLOR COLOR_BANNER_TINTS[] = { 
	D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1 ),
	D3DXCOLOR( 0.0f, 1.0f, 1.0f, 1 ),
	D3DXCOLOR( 1.0f, 0.0f, 1.0f, 1 ),
	D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1 ),
	D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1 ),
};
const int NUM_BANNER_TINTS = sizeof(COLOR_BANNER_TINTS) / sizeof(D3DXCOLOR);

D3DXCOLOR COLOR_SECTION_TINTS[] = { 
	D3DXCOLOR( 0.9f, 0.0f, 0.2f, 1 ),	// red
	D3DXCOLOR( 0.6f, 0.0f, 0.4f, 1 ),	// pink
	D3DXCOLOR( 0.2f, 0.1f, 0.3f, 1 ),	// purple
	D3DXCOLOR( 0.0f, 0.4f, 0.8f, 1 ),	// sky blue
	D3DXCOLOR( 0.0f, 0.6f, 0.6f, 1 ),	// sea green
	D3DXCOLOR( 0.0f, 0.6f, 0.2f, 1 ),	// green
	D3DXCOLOR( 0.8f, 0.6f, 0.0f, 1 ),	// orange
};
const int NUM_SECTION_TINTS = sizeof(COLOR_SECTION_TINTS) / sizeof(D3DXCOLOR);


D3DXCOLOR COLOR_SECTION_LETTER = D3DXCOLOR(1,1,0.3f,1);


WheelItemData::WheelItemData()
{
	m_pSong = NULL;
	m_MusicStatusDisplayType = TYPE_NONE;
}

void WheelItemData::LoadFromSectionName( const CString &sSectionName )
{
	m_WheelItemType = TYPE_SECTION;
	m_sSectionName = sSectionName;
	m_colorTint = D3DXCOLOR(0.5f,0.5f,0.5f,1);
}


void WheelItemData::LoadFromSong( Song* pSong )
{
	ASSERT( pSong != NULL );
	m_pSong = pSong;
	m_WheelItemType = TYPE_MUSIC;
}


WheelItemDisplay::WheelItemDisplay()
{
	m_MusicStatusDisplay.SetXY( -132, 0 );
	
	m_Banner.SetHorizAlign( align_left );
	m_Banner.SetXY( 15, 0 );

	m_sprSectionBackground.Load( THEME->GetPathTo(GRAPHIC_SECTION_BACKGROUND) );
	m_sprSectionBackground.SetXY( -30, 0 );

	m_textSectionName.Load( THEME->GetPathTo(FONT_BOLD) );
	m_textSectionName.TurnShadowOff();
	m_textSectionName.SetHorizAlign( align_left );
	m_textSectionName.SetVertAlign( align_middle );
	m_textSectionName.SetXY( m_sprSectionBackground.GetX() - m_sprSectionBackground.GetUnzoomedWidth()/2 + 10, 0 );
	m_textSectionName.SetZoom( 1.2f );
	m_textSectionName.SetDiffuseColor( COLOR_SECTION_LETTER );
}


void WheelItemDisplay::LoadFromWheelItemData( WheelItemData* pWID )
{
	ASSERT( pWID != NULL );
	
	
	// copy all data items
	m_WheelItemType = pWID->m_WheelItemType;
	m_colorTint		= pWID->m_colorTint;
	m_sSectionName	= pWID->m_sSectionName;
	m_pSong			= pWID->m_pSong;
	m_MusicStatusDisplayType = pWID->m_MusicStatusDisplayType;


	// init type specific stuff
	switch( pWID->m_WheelItemType )
	{

	case TYPE_SECTION:
		if( m_sSectionName.GetLength() > 15  && 
			-1 != m_sSectionName.Find(' ', m_sSectionName.GetLength()/3) )	// this is space in the name
		{
			int iIndexSplit = m_sSectionName.Find( ' ', m_sSectionName.GetLength()/3 );
			m_sSectionName.SetAt( iIndexSplit, '\n' );

			m_textSectionName.SetZoom( 0.6f );
			m_textSectionName.SetText( m_sSectionName );
		}
		else	// this is a short name
		{
			m_textSectionName.SetZoom( 1.2f );
			m_textSectionName.SetText( m_sSectionName );
		}
		float fTextWidth, fSpriteWidth;
		fTextWidth = m_textSectionName.GetWidestLineWidthInSourcePixels() * m_textSectionName.GetZoom();
		fSpriteWidth = m_sprSectionBackground.GetUnzoomedWidth() * m_sprSectionBackground.GetZoom() - 20;
		if( fTextWidth > fSpriteWidth  )
			m_textSectionName.SetZoomX( m_textSectionName.GetZoom() * fSpriteWidth / fTextWidth );
		m_sprSectionBackground.SetDiffuseColor( m_colorTint );
		break;
	case TYPE_MUSIC:
		m_Banner.LoadFromSong( m_pSong );
		m_Banner.SetDiffuseColor( m_colorTint );
		m_MusicStatusDisplay.SetType( m_MusicStatusDisplayType );
		break;
	default:
		ASSERT( false );	// invalid type
	}
}





void WheelItemDisplay::SetDiffuseColor( D3DXCOLOR c )
{
	Actor::SetDiffuseColor( c );


	D3DXCOLOR colorTempTint = m_colorTint;
	colorTempTint.a = c.a;
	colorTempTint.r *= c.r;
	colorTempTint.g *= c.g;
	colorTempTint.b *= c.b;

	m_Banner.SetDiffuseColor( colorTempTint );
	m_sprSectionBackground.SetDiffuseColor( colorTempTint );

	D3DXCOLOR colorTempLetter = COLOR_SECTION_LETTER;
	colorTempLetter.a = c.a;
	colorTempLetter.r *= c.r;
	colorTempLetter.g *= c.g;
	colorTempLetter.b *= c.b;

	m_textSectionName.SetDiffuseColor( colorTempLetter );

	D3DXCOLOR colorTempDisplay = D3DXCOLOR( 1, 1 ,1, c.a);

	m_MusicStatusDisplay.SetDiffuseColor( colorTempDisplay );
}

void WheelItemDisplay::Update( float fDeltaTime )
{
	switch( m_WheelItemType )
	{
	case TYPE_SECTION:
		m_sprSectionBackground.Update( fDeltaTime );
		m_textSectionName.Update( fDeltaTime );
		break;
	case TYPE_MUSIC:
		m_MusicStatusDisplay.Update( fDeltaTime );
		m_Banner.Update( fDeltaTime );
		break;
	}
}

void WheelItemDisplay::RenderPrimitives()
{
	switch( m_WheelItemType )
	{
	case TYPE_SECTION:
		m_sprSectionBackground.Draw();
		m_textSectionName.Draw();
		break;
	case TYPE_MUSIC:
		m_MusicStatusDisplay.Draw();
		m_Banner.Draw();
		break;
	}
}



MusicWheel::MusicWheel() 
{ 
	HELPER.Log( "MusicWheel::MusicWheel()" );

	m_sprSelectionBackground.Load( THEME->GetPathTo(GRAPHIC_MUSIC_SELECTION_HIGHLIGHT) );
	m_sprSelectionBackground.SetXY( 0, 0 );
	m_sprSelectionBackground.SetDiffuseColor( D3DXCOLOR(0,0,0.7f,0.5f) );	// dark transparent blue

	m_sprSelectionOverlay.Load( THEME->GetPathTo(GRAPHIC_MUSIC_SELECTION_HIGHLIGHT) );
	m_sprSelectionOverlay.SetXY( 0, 0 );
	m_sprSelectionOverlay.SetDiffuseColor( D3DXCOLOR(0,0,0,0) );	// invisible
	m_sprSelectionOverlay.SetEffectGlowing( 1.5f, D3DXCOLOR(1,1,1,0.1f), D3DXCOLOR(1,1,1,1) );

	m_MusicSortDisplay.SetZ( -2 );
	m_MusicSortDisplay.SetXY( SORT_ICON_ON_SCREEN_X, SORT_ICON_ON_SCREEN_Y );
	
	this->AddActor( &m_MusicSortDisplay );


	m_soundChangeMusic.Load( THEME->GetPathTo(SOUND_SWITCH_MUSIC) );
	m_soundChangeSort.Load( THEME->GetPathTo(SOUND_SWITCH_SORT) );
	m_soundExpand.Load( THEME->GetPathTo(SOUND_EXPAND) );


	// init m_mapGroupNameToBannerColor

	CArray<Song*, Song*> arraySongs;
	arraySongs.Copy( SONG->m_pSongs );
	SortSongPointerArrayByGroup( arraySongs );
	
	int iNextGroupBannerColor = 0;

	CString sLastGroupNameSeen = "";

	for( int i=0; i<arraySongs.GetSize(); i++ )
	{
		CString sThisGroupName = arraySongs[i]->GetGroupName();
		if( sThisGroupName != sLastGroupNameSeen )
		{
			m_mapGroupNameToColorPtr.SetAt( sThisGroupName, &COLOR_BANNER_TINTS[iNextGroupBannerColor++] );
			if( iNextGroupBannerColor >= NUM_BANNER_TINTS-1 )
				iNextGroupBannerColor = 0;
		}
		sLastGroupNameSeen = sThisGroupName;
	}


	m_SortOrder = PREFS->m_SongSortOrder;
	m_MusicSortDisplay.Set( m_SortOrder );
	m_MusicSortDisplay.SetXY( SORT_ICON_ON_SCREEN_X, SORT_ICON_ON_SCREEN_Y );


	m_sExpandedSectionName = "";

	m_iSelection = 0;

	
	m_WheelState = STATE_IDLE;
	m_fTimeLeftInState = FADE_TIME;
	m_fPositionOffsetFromSelection = 0;


	// build all of the wheel item datas
	for( int so=0; so<NUM_SORT_ORDERS; so++ )
		BuildWheelItemDatas( m_WheelItemDatas[so], SongSortOrder(so) );


	if( SONG->m_pCurSong == NULL && 	// if there is no currently selected song
		SONG->m_pSongs.GetSize() > 0 )		// and there is at least one song
	{
		SONG->m_pCurSong = SONG->m_pSongs[0];	// select the first song
	}



	// find the previously selected song (if any)
	for( i=0; i<GetCurWheelItemDatas().GetSize(); i++ )
	{
		if( GetCurWheelItemDatas()[i].m_pSong != NULL
		 && GetCurWheelItemDatas()[i].m_pSong == SONG->m_pCurSong )
		{
			m_iSelection = i;		// select it
			m_sExpandedSectionName = GetCurWheelItemDatas()[m_iSelection].m_sSectionName;	// make its group the currently expanded group
		}
	}


	// rebuild the WheelItems that appear on screen
	RebuildWheelItemDisplays();

}

MusicWheel::~MusicWheel()
{
	PREFS->m_SongSortOrder = m_SortOrder;
}

void MusicWheel::BuildWheelItemDatas( CArray<WheelItemData, WheelItemData&> &arrayWheelItemDatas, SongSortOrder so )
{

	///////////////////////////////////
	// Make an array of Song*, then sort them
	///////////////////////////////////
	CArray<Song*, Song*> arraySongs;
	
	// copy only song that have at least one Pattern for the current GameMode
	for( int i=0; i<SONG->m_pSongs.GetSize(); i++ )
	{
		Song* pSong = SONG->m_pSongs[i];

		CArray<Pattern*, Pattern*> arraySteps;
		pSong->GetPatternsThatMatchStyle( GAME->m_DanceStyle, arraySteps );

		if( arraySteps.GetSize() > 0 )
			arraySongs.Add( pSong );
	}


	// sort the SONG
	switch( so )
	{
	case SORT_GROUP:
		SortSongPointerArrayByGroup( arraySongs );
		break;
	case SORT_TITLE:
		SortSongPointerArrayByTitle( arraySongs );
		break;
	case SORT_BPM:
		SortSongPointerArrayByBPM( arraySongs );
		break;
//	case SORT_ARTIST:
//		SortSongPointerArrayByArtist( arraySongs );
//		break;
	case SORT_MOST_PLAYED:
		SortSongPointerArrayByMostPlayed( arraySongs );
		break;
	default:
		ASSERT( false );	// unhandled SORT_ORDER
	}



	///////////////////////////////////
	// Build an array of WheelItemDatas from the sorted list of Song*
	///////////////////////////////////
	arrayWheelItemDatas.RemoveAll();	// clear out the previous wheel items...

	// ...and load new ones
	switch( so )
	{
	case SORT_MOST_PLAYED:
	case SORT_BPM:
		// make WheelItems without sections
		arrayWheelItemDatas.SetSize( arraySongs.GetSize() );
		{
			for( int i=0; i<arraySongs.GetSize(); i++ )
			{
				Song* pSong = arraySongs[i];
				WheelItemData &WID = arrayWheelItemDatas[i];
				WID.LoadFromSong( pSong );
				WID.m_colorTint = *m_mapGroupNameToColorPtr[pSong->GetGroupName()];
				WID.m_sSectionName = "";
			}
		}
		break;
	case SORT_GROUP:
	case SORT_TITLE:
//	case SORT_ARTIST:
		{
		// make WheelItemDatas with sections

		arrayWheelItemDatas.SetSize( arraySongs.GetSize()*2 );	// make sure we have enough room for all music and section items

		CString sLastSection = "";
		int iCurWheelItem = 0;
		int iNextSectionTint = 0;
		for( int i=0; i< arraySongs.GetSize(); i++ )
		{
			Song* pSong = arraySongs[i];
			CString sThisSection = GetSectionNameFromSongAndSort( pSong, so );
			if( sThisSection != sLastSection )	// new section, make a section item
			{
				WheelItemData &WID = arrayWheelItemDatas[iCurWheelItem++];
				WID.LoadFromSectionName( sThisSection );
				WID.m_colorTint = COLOR_SECTION_TINTS[iNextSectionTint++];
				if( iNextSectionTint >= NUM_SECTION_TINTS )
					iNextSectionTint = 0;
				sLastSection = sThisSection;
			}

			WheelItemData &WID = arrayWheelItemDatas[iCurWheelItem++];
			WID.LoadFromSong( pSong );
			WID.m_sSectionName = sThisSection;
			WID.m_colorTint = *m_mapGroupNameToColorPtr[pSong->GetGroupName()];
		}
		arrayWheelItemDatas.SetSize( iCurWheelItem );	// make sure we have enough room for all music and section items
		}
		break;
	default:
		ASSERT( false );	// unhandled SORT_ORDER
	}

	// init crowns
	for( i=0; i<arrayWheelItemDatas.GetSize(); i++ )
	{
		Song* pSong = arrayWheelItemDatas[i].m_pSong;
		if( pSong != NULL )
		{
			MusicStatusDisplayType crown = (pSong->GetNumTimesPlayed()==0) ? TYPE_NEW : TYPE_NONE;
			arrayWheelItemDatas[i].m_MusicStatusDisplayType = crown;
		}		
	}

	if( so == SORT_MOST_PLAYED )
	{
		// init crown icons 
		for( int i=0; i<arrayWheelItemDatas.GetSize() && i<3; i++ )
		{
			MusicStatusDisplayType crown = MusicStatusDisplayType(TYPE_CROWN1 + i);
			arrayWheelItemDatas[i].m_MusicStatusDisplayType = crown;
		}
	}



	if( arrayWheelItemDatas.GetSize() == 0 )
	{
		arrayWheelItemDatas.SetSize( 1 );
		arrayWheelItemDatas[0].LoadFromSectionName( "No SONG" );
		arrayWheelItemDatas[0].m_colorTint = D3DXCOLOR(0.5f,0.5f,0.5f,1);
	}
}

void MusicWheel::SwitchSortOrder()
{
	
}

float MusicWheel::GetBannerY( float fPosOffsetsFromMiddle )
{
	return fPosOffsetsFromMiddle*43.2f;
}

float MusicWheel::GetBannerBrightness( float fPosOffsetsFromMiddle )
{
	return 1 - fabsf(fPosOffsetsFromMiddle)*0.11f;
}

float MusicWheel::GetBannerAlpha( float fPosOffsetsFromMiddle )
{
	if( m_WheelState == STATE_FLYING_OFF_BEFORE_NEXT_SORT 
	 || m_WheelState == STATE_TWEENING_OFF_SCREEN  )
	{
		return m_fTimeLeftInState / FADE_TIME;
	}
	else if( m_WheelState == STATE_FLYING_ON_AFTER_NEXT_SORT
		  || m_WheelState == STATE_TWEENING_ON_SCREEN )
	{
		return 1 - (m_fTimeLeftInState / FADE_TIME);
	}
	else if( m_WheelState == STATE_WAITING_OFF_SCREEN )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

float MusicWheel::GetBannerX( float fPosOffsetsFromMiddle )
{	
	float fX = (1-cosf((fPosOffsetsFromMiddle)/3))*95.0f;

	if( m_WheelState == STATE_FLYING_OFF_BEFORE_NEXT_SORT 
	 || m_WheelState == STATE_TWEENING_OFF_SCREEN  )
	{
		float fDistFromCenter = fabsf( fPosOffsetsFromMiddle );
		float fPercentOffScreen = 1- (m_fTimeLeftInState / FADE_TIME);
		float fXLogicalOffset = max( 0, fPercentOffScreen - 1 + (fDistFromCenter+3)/6 );
		fXLogicalOffset = powf( fXLogicalOffset, 1.7f );	// accelerate 
		float fXPixelOffset = fXLogicalOffset * 600;
		fX += fXPixelOffset;
	}
	else if( m_WheelState == STATE_FLYING_ON_AFTER_NEXT_SORT
		  || m_WheelState == STATE_TWEENING_ON_SCREEN )
	{
		float fDistFromCenter = fabsf( fPosOffsetsFromMiddle );
		float fPercentOffScreen = m_fTimeLeftInState / FADE_TIME;
		float fXLogicalOffset = max( 0, fPercentOffScreen - 1 + (fDistFromCenter+3)/6 );
		fXLogicalOffset = powf( fXLogicalOffset, 1.7f );	// accelerate 
		float fXPixelOffset = fXLogicalOffset * 600;
		fX += fXPixelOffset;
	}


	return fX;
}

void MusicWheel::RebuildWheelItemDisplays()
{
	// rewind to first index that will be displayed;
	int iIndex = m_iSelection;
	if( m_iSelection > GetCurWheelItemDatas().GetSize()-1 )
		m_iSelection = 0;

	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW/2; i++ )
	{
		do
		{
			iIndex--;
			if( iIndex < 0 )
				iIndex = GetCurWheelItemDatas().GetSize()-1;
		} 
		while( GetCurWheelItemDatas()[iIndex].m_WheelItemType == TYPE_MUSIC 
			&& GetCurWheelItemDatas()[iIndex].m_sSectionName != ""
			&& GetCurWheelItemDatas()[iIndex].m_sSectionName != m_sExpandedSectionName );
	}

	// iIndex is now the index of the lowest WheelItem to draw
	for( i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemData&    data    = GetCurWheelItemDatas()[iIndex];
		WheelItemDisplay& display = m_WheelItemDisplays[i];

		display.LoadFromWheelItemData( &data );

		// increment iIndex
		do
		{
			iIndex++;
			if( iIndex > GetCurWheelItemDatas().GetSize()-1 )
				iIndex = 0;
		} 
		while( GetCurWheelItemDatas()[iIndex].m_WheelItemType == TYPE_MUSIC 
			&& GetCurWheelItemDatas()[iIndex].m_sSectionName != ""
			&& GetCurWheelItemDatas()[iIndex].m_sSectionName != m_sExpandedSectionName );
	}

}


void MusicWheel::RenderPrimitives()
{
	m_sprSelectionBackground.Draw();


	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemDisplay& display = m_WheelItemDisplays[i];

		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS_TO_DRAW/2 + m_fPositionOffsetFromSelection;

		float fY = GetBannerY( fThisBannerPositionOffsetFromSelection );
		float fX = GetBannerX( fThisBannerPositionOffsetFromSelection );
		display.SetXY( fX, fY );

		float fBrightness = GetBannerBrightness( m_fPositionOffsetFromSelection );
		float fAlpha = GetBannerAlpha( m_fPositionOffsetFromSelection );

		display.SetDiffuseColor( D3DXCOLOR(fBrightness, fBrightness, fBrightness, fAlpha) );
		display.Draw();
	}



	m_sprSelectionOverlay.Draw();

	ActorFrame::RenderPrimitives();
}

void MusicWheel::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_sprSelectionOverlay.Update( fDeltaTime );
	m_sprSelectionBackground.Update( fDeltaTime );



	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemDisplay& display = m_WheelItemDisplays[i];

		display.Update( fDeltaTime );
	}



	// update wheel state
	m_fTimeLeftInState -= fDeltaTime;
	if( m_fTimeLeftInState <= 0 )	// time to go to a new state
	{
		switch( m_WheelState )
		{
		case STATE_SWITCHING_MUSIC:
			m_WheelState = STATE_IDLE;	// now, wait for input
			WM->SendMessageToTopWindow( SM_PlaySongSample, 0 );
			break;
		case STATE_FLYING_OFF_BEFORE_NEXT_SORT:
			{
			m_WheelState = STATE_FLYING_ON_AFTER_NEXT_SORT;
			m_fTimeLeftInState = FADE_TIME;

			Song* pPrevSelectedSong = GetCurWheelItemDatas()[m_iSelection].m_pSong;
			CString sPrevSelectedSection = GetCurWheelItemDatas()[m_iSelection].m_sSectionName;

			// change the sort order
			m_SortOrder = SongSortOrder(m_SortOrder+1);
			if( m_SortOrder > NUM_SORT_ORDERS-1 )
				m_SortOrder = (SongSortOrder)0;
			m_sExpandedSectionName = GetSectionNameFromSongAndSort( pPrevSelectedSong, m_SortOrder );
			//RebuildWheelItems();

			m_MusicSortDisplay.Set( m_SortOrder );
			m_MusicSortDisplay.BeginTweening( FADE_TIME, TWEEN_BIAS_BEGIN );
			m_MusicSortDisplay.SetTweenXY( SORT_ICON_ON_SCREEN_X, SORT_ICON_ON_SCREEN_Y );


			m_iSelection = 0;

			if( pPrevSelectedSong != NULL )		// the previous selected item was a song
			{
				// find the previously selected song, and select it
				for( i=0; i<GetCurWheelItemDatas().GetSize(); i++ )
				{
					if( GetCurWheelItemDatas()[i].m_pSong == pPrevSelectedSong )
					{
						m_iSelection = i;
						break;
					}
				}
			}
			else	// the previously selected item was a section
			{
				// find the previously selected song, and select it
				for( i=0; i<GetCurWheelItemDatas().GetSize(); i++ )
				{
					if( GetCurWheelItemDatas()[i].m_sSectionName == sPrevSelectedSection )
					{
						m_iSelection = i;
						break;
					}
				}
			}

			RebuildWheelItemDisplays();

			}
			break;
		case STATE_FLYING_ON_AFTER_NEXT_SORT:
			WM->SendMessageToTopWindow( SM_PlaySongSample, 0 );
			m_WheelState = STATE_IDLE;	// now, wait for input
			break;
		case STATE_TWEENING_ON_SCREEN:
			WM->SendMessageToTopWindow( SM_PlaySongSample, 0 );
			m_WheelState = STATE_IDLE;
			m_fTimeLeftInState = 0;
			break;
		case STATE_TWEENING_OFF_SCREEN:
			m_WheelState = STATE_WAITING_OFF_SCREEN;
			m_fTimeLeftInState = 0;
			break;
		case STATE_IDLE:
			m_fTimeLeftInState = 0;
			break;
		}
	}

	// "rotate" wheel toward selected song

	if( fabsf(m_fPositionOffsetFromSelection) < 0.02f )
		m_fPositionOffsetFromSelection = 0;
	else
	{
		m_fPositionOffsetFromSelection -= fDeltaTime * m_fPositionOffsetFromSelection*4;	// linear
		float fSign = m_fPositionOffsetFromSelection / fabsf(m_fPositionOffsetFromSelection);
		m_fPositionOffsetFromSelection -= fDeltaTime * fSign;	// constant
	}
}


void MusicWheel::PrevMusic()
{
	switch( m_WheelState )
	{
	case STATE_IDLE:
	case STATE_SWITCHING_MUSIC:
		break;	// fall through
	default:
		return;	// don't fall through
	}

	MUSIC->Stop();

	// decrement m_iSelection
	do
	{
		m_iSelection--;
		if( m_iSelection < 0 )
			m_iSelection = GetCurWheelItemDatas().GetSize()-1;
	} 
	while( GetCurWheelItemDatas()[m_iSelection].m_WheelItemType == TYPE_MUSIC 
		&& GetCurWheelItemDatas()[m_iSelection].m_sSectionName != ""
		&& GetCurWheelItemDatas()[m_iSelection].m_sSectionName != m_sExpandedSectionName );

	RebuildWheelItemDisplays();

	m_fPositionOffsetFromSelection -= 1;

	m_WheelState = STATE_SWITCHING_MUSIC;
	m_fTimeLeftInState = SWITCH_MUSIC_TIME;
	m_soundChangeMusic.PlayRandom();
}

void MusicWheel::NextMusic()
{
	switch( m_WheelState )
	{
	case STATE_IDLE:
	case STATE_SWITCHING_MUSIC:
		break;	// fall through
	default:
		return;	// don't continue
	}

	MUSIC->Stop();

	// increment m_iSelection
	do
	{
		m_iSelection++;
		if( m_iSelection > GetCurWheelItemDatas().GetSize()-1 )
			m_iSelection = 0;
	} 
	while( GetCurWheelItemDatas()[m_iSelection].m_WheelItemType == TYPE_MUSIC 
		&& GetCurWheelItemDatas()[m_iSelection].m_sSectionName != ""
		&& GetCurWheelItemDatas()[m_iSelection].m_sSectionName != m_sExpandedSectionName );

	RebuildWheelItemDisplays();

	m_fPositionOffsetFromSelection += 1;

	m_WheelState = STATE_SWITCHING_MUSIC;
	m_fTimeLeftInState = SWITCH_MUSIC_TIME;
	m_soundChangeMusic.PlayRandom();
}

void MusicWheel::NextSort()
{
	switch( m_WheelState )
	{
	case STATE_IDLE:
	case STATE_SWITCHING_MUSIC:
	case STATE_FLYING_ON_AFTER_NEXT_SORT:
		break;	// fall through
	default:
		return;	// don't continue
	}

	MUSIC->Stop();

	m_soundChangeSort.PlayRandom();
	m_MusicSortDisplay.BeginTweening( FADE_TIME, TWEEN_BIAS_END );
	m_MusicSortDisplay.SetTweenXY( SORT_ICON_OFF_SCREEN_X, SORT_ICON_OFF_SCREEN_Y );

	m_WheelState = STATE_FLYING_OFF_BEFORE_NEXT_SORT;
	m_fTimeLeftInState = FADE_TIME;
}

bool MusicWheel::Select()
{
	switch( GetCurWheelItemDatas()[m_iSelection].m_WheelItemType )
	{
	case TYPE_SECTION:
		{
		CString sThisItemSectionName = GetCurWheelItemDatas()[m_iSelection].m_sSectionName;
		if( m_sExpandedSectionName == sThisItemSectionName )	// already expanded
			m_sExpandedSectionName = "";		// collapse it
		else				// already collapsed
			m_sExpandedSectionName = sThisItemSectionName;	// expand it

	
		RebuildWheelItemDisplays();


		m_soundExpand.PlayRandom();


		m_iSelection = 0;	// reset in case we can't find the last selected song
		// find the section header and select it
		for( int i=0; i<GetCurWheelItemDatas().GetSize(); i++ )
		{
			if( GetCurWheelItemDatas()[i].m_WheelItemType == TYPE_SECTION  
				&&  GetCurWheelItemDatas()[i].m_sSectionName == sThisItemSectionName )
			{
				m_iSelection = i;
				break;
			}
		}

		}
		return false;

	case TYPE_MUSIC:
	default:
		
		return true;
	}
}

void MusicWheel::TweenOnScreen() 
{
	m_WheelState = STATE_TWEENING_ON_SCREEN;
	m_fTimeLeftInState = FADE_TIME; 

	float fOriginalZoomY = m_sprSelectionBackground.GetZoomY();
	m_sprSelectionBackground.SetZoomY( 0 );
	m_sprSelectionBackground.BeginTweening( FADE_TIME );
	m_sprSelectionBackground.SetTweenZoomY( fOriginalZoomY );

	m_sprSelectionOverlay.SetZoomY( 0 );
	m_sprSelectionOverlay.BeginTweening( FADE_TIME );
	m_sprSelectionOverlay.SetTweenZoomY( fOriginalZoomY );

}
						   
						   
void MusicWheel::TweenOffScreen()
{
	m_WheelState = STATE_TWEENING_OFF_SCREEN;
	m_fTimeLeftInState = FADE_TIME;

	m_sprSelectionBackground.BeginTweening( FADE_TIME );
	m_sprSelectionBackground.SetTweenZoomY( 0 );
	m_sprSelectionOverlay.BeginTweening( FADE_TIME );
	m_sprSelectionOverlay.SetTweenZoomY( 0 );

	m_MusicSortDisplay.BeginTweening( FADE_TIME, TWEEN_BIAS_END );
	m_MusicSortDisplay.SetTweenXY( SORT_ICON_OFF_SCREEN_X, SORT_ICON_OFF_SCREEN_Y );
}