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
#include "ScreenDimensions.h"
#include "ThemeManager.h"
#include "RageMusic.h"
#include "WindowManager.h"	// for sending SM_PlayMusicSample




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


WheelItem::WheelItem()
{
	m_pSong = NULL;
}


void WheelItem::LoadFromSong( Song* pSong )
{
	ASSERT( pSong != NULL );
	
	m_pSong = pSong;

	m_WheelItemType = TYPE_MUSIC;

	m_MusicStatusDisplay.SetIsNew( m_pSong->GetNumTimesPlayed() == 0 );
	m_MusicStatusDisplay.SetXY( -132, 0 );
	
	m_Banner.LoadFromSong( pSong );
	m_Banner.SetHorizAlign( align_left );
	m_Banner.SetXY( 15, 0 );
}


void WheelItem::LoadFromSectionName( CString sSectionName )
{
	m_WheelItemType = TYPE_SECTION;

	m_sprSectionBackground.Load( THEME->GetPathTo(GRAPHIC_SECTION_BACKGROUND) );
	m_sprSectionBackground.SetXY( -30, 0 );

	m_textSectionName.Load( THEME->GetPathTo(FONT_OUTLINE) );
	m_textSectionName.TurnShadowOff();
	m_textSectionName.SetText( sSectionName );
	m_textSectionName.SetHorizAlign( align_left );
	m_textSectionName.SetXY( -85, 0 );
	m_textSectionName.SetDiffuseColor( COLOR_SECTION_LETTER );
	m_textSectionName.SetZoom( 1.5f );
}


void WheelItem::SetTintColor( D3DXCOLOR c )
{
	m_colorTint = c;
};

void WheelItem::SetDiffuseColor( D3DXCOLOR c )
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


};

void WheelItem::Update( float fDeltaTime )
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

void WheelItem::RenderPrimitives()
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
	RageLog( "MusicWheel::MusicWheel()" );

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

	CArray<Song*, Song*&> arraySongs;
	arraySongs.Copy( GAMEINFO->m_pSongs );
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


	m_SortOrder = GAMEINFO->m_SongSortOrder;
	m_MusicSortDisplay.Set( m_SortOrder );
	m_MusicSortDisplay.SetXY( SORT_ICON_ON_SCREEN_X, SORT_ICON_ON_SCREEN_Y );


	m_sExpandedSectionName = "";

	m_iSelection = 0;

	
	m_WheelState = STATE_IDLE;
	m_fTimeLeftInState = FADE_TIME;
	m_fPositionOffsetFromSelection = 0;


	// find the previously selected song (if any), and select it
	for( i=0; i<GetCurWheelItems().GetSize(); i++ )
	{
		if( GetCurWheelItems()[i].m_pSong != NULL
		 && GetCurWheelItems()[i].m_pSong == GAMEINFO->m_pCurSong )
			m_iSelection = i;
	}


	for( int so=0; so<NUM_SORT_ORDERS; so++ )
		BuildWheelItems( m_WheelItems[so], SongSortOrder(so) );

}

MusicWheel::~MusicWheel()
{
	GAMEINFO->m_SongSortOrder = m_SortOrder;
}

void MusicWheel::BuildWheelItems( CArray<WheelItem, WheelItem&> &arrayWheelItems, SongSortOrder so )
{
	CArray<Song*, Song*&> arraySongs;
	arraySongs.Copy( GAMEINFO->m_pSongs );
	
	// sort the songs
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
	case SORT_ARTIST:
		SortSongPointerArrayByArtist( arraySongs );
		break;
	case SORT_MOST_PLAYED:
		SortSongPointerArrayByMostPlayed( arraySongs );
		break;
	default:
		ASSERT( true );	// unhandled SORT_ORDER
	}

		
	for( int i=0; i<arraySongs.GetSize(); i++ )
	{
		Song* pSong = arraySongs[i];
		int iNumTimesPlayed = pSong->GetNumTimesPlayed();
	}
	arrayWheelItems.RemoveAll();	// clear out the previous wheel items...

	// ...and load new ones
	switch( so )
	{
	case SORT_GROUP:
	case SORT_MOST_PLAYED:
	case SORT_BPM:
		// make WheelItems without sections
		arrayWheelItems.SetSize( arraySongs.GetSize() );
		{
			for( int i=0; i< arraySongs.GetSize(); i++ )
			{
				Song* pSong = arraySongs[i];
				WheelItem &WI = arrayWheelItems[i];
				WI.LoadFromSong( pSong );
				WI.SetTintColor( *m_mapGroupNameToColorPtr[pSong->GetGroupName()] );
				WI.m_sSectionName = "";
			}
		}
		break;
	case SORT_TITLE:
	case SORT_ARTIST:
		// make WheelItems with sections

		arrayWheelItems.SetSize( arraySongs.GetSize()*2 );	// make sure we have enough room for all music and section items

		{
			CString sLastSection = "";
			int iCurWheelItem = 0;
			int iNextSectionTint = 0;
			for( int i=0; i< arraySongs.GetSize(); i++ )
			{
				Song* pSong = arraySongs[i];
				CString sThisSection = GetSectionNameFromSongAndSort( pSong, so );
				if( sThisSection != sLastSection )	// new section, make a section item
				{
					WheelItem &WI = arrayWheelItems[iCurWheelItem++];
					WI.LoadFromSectionName( sThisSection );
					WI.m_sSectionName = sThisSection;
					WI.SetTintColor( COLOR_SECTION_TINTS[iNextSectionTint++] );
					if( iNextSectionTint >= NUM_SECTION_TINTS )
						iNextSectionTint = 0;
					sLastSection = sThisSection;
				}

				WheelItem &WI = arrayWheelItems[iCurWheelItem++];
				WI.LoadFromSong( pSong );
				WI.m_sSectionName = sThisSection;
				WI.SetTintColor( *m_mapGroupNameToColorPtr[pSong->GetGroupName()] );
			}
			arrayWheelItems.SetSize( iCurWheelItem );	// make sure we have enough room for all music and section items
		}
		break;
	default:
		ASSERT( true );	// unhandled SORT_ORDER
	}


	if( so == SORT_MOST_PLAYED )
	{
		// init crown icons 
		for( int i=0; i<arrayWheelItems.GetSize(); i++ )
		{
			arrayWheelItems[i].m_MusicStatusDisplay.SetBlinking( true );
			arrayWheelItems[i].m_MusicStatusDisplay.SetRank( i+1 );
		}
	}



	if( arrayWheelItems.GetSize() == 0 )
	{
		arrayWheelItems.SetSize( 1 );
		arrayWheelItems[0].LoadFromSectionName( "No Songs" );
		arrayWheelItems[0].SetTintColor( D3DXCOLOR(0.5f,0.5f,0.5f,1) );
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
	return 1 - fabs(fPosOffsetsFromMiddle)*0.11f;
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
	float fX = (1-cos((fPosOffsetsFromMiddle)/3))*95.0f;

	if( m_WheelState == STATE_FLYING_OFF_BEFORE_NEXT_SORT 
	 || m_WheelState == STATE_TWEENING_OFF_SCREEN  )
	{
		float fDistFromCenter = fabs( fPosOffsetsFromMiddle );
		float fPercentOffScreen = 1- (m_fTimeLeftInState / FADE_TIME);
		float fXLogicalOffset = max( 0, fPercentOffScreen - 1 + (fDistFromCenter+3)/6 );
		fXLogicalOffset = pow( fXLogicalOffset, 1.7 );	// accelerate 
		float fXPixelOffset = fXLogicalOffset * 600;
		fX += fXPixelOffset;
	}
	else if( m_WheelState == STATE_FLYING_ON_AFTER_NEXT_SORT
		  || m_WheelState == STATE_TWEENING_ON_SCREEN )
	{
		float fDistFromCenter = fabs( fPosOffsetsFromMiddle );
		float fPercentOffScreen = m_fTimeLeftInState / FADE_TIME;
		float fXLogicalOffset = max( 0, fPercentOffScreen - 1 + (fDistFromCenter+3)/6 );
		fXLogicalOffset = pow( fXLogicalOffset, 1.7 );	// accelerate 
		float fXPixelOffset = fXLogicalOffset * 600;
		fX += fXPixelOffset;
	}


	return fX;
}


void MusicWheel::RenderPrimitives()
{
	m_sprSelectionBackground.Draw();


	// rewind to bottom item to draw;
	int iIndex = m_iSelection;
	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW/2; i++ )
	{
		do
		{
			iIndex--;
			if( iIndex < 0 )
				iIndex = GetCurWheelItems().GetSize()-1;
		} 
		while( GetCurWheelItems()[iIndex].m_WheelItemType == WheelItem::TYPE_MUSIC 
			&& GetCurWheelItems()[iIndex].m_sSectionName != ""
			&& GetCurWheelItems()[iIndex].m_sSectionName != m_sExpandedSectionName );


	}

	// iIndex is now the index of the lowest WheelItem to draw
	for( i=-NUM_WHEEL_ITEMS_TO_DRAW/2; i<NUM_WHEEL_ITEMS_TO_DRAW/2; i++ )
	{
		WheelItem& WI = GetCurWheelItems()[iIndex];

		float fThisBannerPositionOffsetFromSelection = m_fPositionOffsetFromSelection + i;

		float fY = GetBannerY( fThisBannerPositionOffsetFromSelection );
		float fX = GetBannerX( fThisBannerPositionOffsetFromSelection );
		WI.SetXY( fX, fY );

		float fBrightness = GetBannerBrightness( m_fPositionOffsetFromSelection );
		float fAlpha = GetBannerAlpha( m_fPositionOffsetFromSelection );

		WI.SetDiffuseColor( D3DXCOLOR(fBrightness, fBrightness, fBrightness, fAlpha) );
		WI.Draw();


		do
		{
			iIndex++;
			if( iIndex > GetCurWheelItems().GetSize()-1 )
				iIndex = 0;
		} 
		while( GetCurWheelItems()[iIndex].m_WheelItemType == WheelItem::TYPE_MUSIC 
			&& GetCurWheelItems()[iIndex].m_sSectionName != ""
			&& GetCurWheelItems()[iIndex].m_sSectionName != m_sExpandedSectionName );

	}


	m_sprSelectionOverlay.Draw();

	ActorFrame::RenderPrimitives();
}

void MusicWheel::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_sprSelectionOverlay.Update( fDeltaTime );
	m_sprSelectionBackground.Update( fDeltaTime );



	for( int i=0; i<GetCurWheelItems().GetSize(); i++ )
	{
		GetCurWheelItems()[i].Update( fDeltaTime );
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

			Song* pPrevSelectedSong = GetCurWheelItems()[m_iSelection].m_pSong;

			// change the sort order
			m_SortOrder = SongSortOrder(m_SortOrder+1);
			if( m_SortOrder > NUM_SORT_ORDERS-1 )
				m_SortOrder = (SongSortOrder)0;
			m_sExpandedSectionName = GetSectionNameFromSongAndSort( pPrevSelectedSong, m_SortOrder );
			//RebuildWheelItems();

			m_MusicSortDisplay.Set( m_SortOrder );
			m_MusicSortDisplay.BeginTweening( FADE_TIME, TWEEN_BIAS_BEGIN );
			m_MusicSortDisplay.SetTweenXY( SORT_ICON_ON_SCREEN_X, SORT_ICON_ON_SCREEN_Y );


			// find the previously selected song, and select it
			for( i=0; i<GetCurWheelItems().GetSize(); i++ )
			{
				if( GetCurWheelItems()[i].m_pSong == pPrevSelectedSong )
					m_iSelection = i;
			}
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

	if( fabs(m_fPositionOffsetFromSelection) < 0.02f )
		m_fPositionOffsetFromSelection = 0;
	else
	{
		m_fPositionOffsetFromSelection -= fDeltaTime * m_fPositionOffsetFromSelection*4;	// linear
		float fSign = m_fPositionOffsetFromSelection / fabs(m_fPositionOffsetFromSelection);
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

	do
	{
		m_iSelection--;
		if( m_iSelection < 0 )
			m_iSelection = GetCurWheelItems().GetSize()-1;
	} 
	while( GetCurWheelItems()[m_iSelection].m_WheelItemType == WheelItem::TYPE_MUSIC 
		&& GetCurWheelItems()[m_iSelection].m_sSectionName != ""
		&& GetCurWheelItems()[m_iSelection].m_sSectionName != m_sExpandedSectionName );


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

	do
	{
		m_iSelection++;
		if( m_iSelection > GetCurWheelItems().GetSize()-1 )
			m_iSelection = 0;
	} 
	while( GetCurWheelItems()[m_iSelection].m_WheelItemType == WheelItem::TYPE_MUSIC 
		&& GetCurWheelItems()[m_iSelection].m_sSectionName != ""
		&& GetCurWheelItems()[m_iSelection].m_sSectionName != m_sExpandedSectionName );


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
	switch( GetCurWheelItems()[m_iSelection].m_WheelItemType )
	{
	case WheelItem::TYPE_SECTION:
		{
		CString sThisItemSectionName = GetCurWheelItems()[m_iSelection].GetSectionName();
		if( m_sExpandedSectionName == sThisItemSectionName )	// already expanded
			m_sExpandedSectionName = "";		// collapse it
		else				// already collapsed
			m_sExpandedSectionName = sThisItemSectionName;	// expand it

		//RebuildWheelItems();


		m_soundExpand.PlayRandom();


		m_iSelection = 0;	// reset in case we can't find the last selected song
		// find the section header and select it
		for( int i=0; i<GetCurWheelItems().GetSize(); i++ )
		{
			if( GetCurWheelItems()[i].m_WheelItemType == WheelItem::TYPE_SECTION  
				&&  GetCurWheelItems()[i].GetSectionName() == sThisItemSectionName )
			{
				m_iSelection = i;
				break;
			}
		}

		}
		return false;

	case WheelItem::TYPE_MUSIC:
	default:
		
		return true;
	}
}

void MusicWheel::TweenOnScreen() {
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
						   
						   
void MusicWheel::TweenOffScreen(){
	m_WheelState = STATE_TWEENING_OFF_SCREEN;
	m_fTimeLeftInState = FADE_TIME;

	m_sprSelectionBackground.BeginTweening( FADE_TIME );
	m_sprSelectionBackground.SetTweenZoomY( 0 );
	m_sprSelectionOverlay.BeginTweening( FADE_TIME );
	m_sprSelectionOverlay.SetTweenZoomY( 0 );
}