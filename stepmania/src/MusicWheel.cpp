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



void WheelItem::LoadFromSong( Song &song )
{
	m_WheelItemType = TYPE_MUSIC;

	m_MusicStatusDisplay.SetXY( -140, 0 );
	this->AddActor( &m_MusicStatusDisplay );
	
	m_Banner.SetHorizAlign( align_left );
	m_Banner.SetXY( -30, 0 );
	this->AddActor( &m_Banner );

	m_GradeP1.SetZoom( 0.4f );
	m_GradeP1.SetXY( 105, 0 );
	this->AddActor( &m_GradeP1 );

	m_GradeP2.SetZoom( 0.4f );
	m_GradeP2.SetXY( 145, 0 );
	this->AddActor( &m_GradeP2 );


	m_pSong = &song;
	m_Banner.LoadFromSong( song );
	m_MusicStatusDisplay.SetNew( m_pSong->m_iNumTimesPlayed == 0 );
	//m_GradeP1.SetGrade( song.m_TopGrade[PLAYER_1] );
	//m_GradeP2.SetGrade( song.m_TopGrade[PLAYER_2] );
};


void WheelItem::LoadFromSectionName( CString sSectionName )
{
	m_WheelItemType = TYPE_SECTION;

	m_sprSectionBackground.Load( THEME->GetPathTo(GRAPHIC_SECTION_BACKGROUND) );
	m_sprSectionBackground.SetXY( -30, 0 );
	this->AddActor( &m_sprSectionBackground );

	m_textSectionName.Load( THEME->GetPathTo(FONT_OUTLINE) );
	m_textSectionName.TurnShadowOff();
	m_textSectionName.SetText( sSectionName );
	m_textSectionName.SetHorizAlign( align_left );
	m_textSectionName.SetXY( -100, 0 );
	m_textSectionName.SetDiffuseColor( COLOR_SECTION_LETTER );
	m_textSectionName.SetZoom( 1.5f );
	this->AddActor( &m_textSectionName );

};

void WheelItem::SetTintColor( D3DXCOLOR c )
{
	m_colorTint = c;
};

void WheelItem::SetDiffuseColor( D3DXCOLOR c )
{
	ActorFrame::SetDiffuseColor( c );


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

};




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




	m_SortOrder = SORT_GROUP;
	m_sExpandedSectionName = "";

	RebuildWheelItems();
	m_iSelection = 0;

	
	m_WheelState = STATE_IDLE;
	m_fTimeLeftInState = FADE_TIME;
	m_fPositionOffsetFromSelection = 0;


	RageLog( "end of MusicWheel::MusicWheel()" );

}

void MusicWheel::RebuildWheelItems()
{
	CArray<Song*, Song*&> arraySongs;
	arraySongs.Copy( GAMEINFO->m_pSongs );
	
	// sort the songs
	switch( m_SortOrder )
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



	m_WheelItems.RemoveAll();	// clear out the previous wheel items...

	// ...and load new ones
	switch( m_SortOrder )
	{
	case SORT_GROUP:
	case SORT_MOST_PLAYED:
	case SORT_BPM:
		// make WheelItems without sections
		m_WheelItems.SetSize( arraySongs.GetSize() );
		{
			for( int i=0; i< arraySongs.GetSize(); i++ )
			{
				Song* pSong = arraySongs[i];
				m_WheelItems[i].LoadFromSong( *pSong );
				m_WheelItems[i].SetTintColor( *m_mapGroupNameToColorPtr[pSong->GetGroupName()] );
			}
		}
		break;
	case SORT_TITLE:
	case SORT_ARTIST:
		// make WheelItems with sections

		m_WheelItems.SetSize( arraySongs.GetSize()*2 );	// make sure we have enough room for all music and section items

		{
			CString sLastSection = "";
			int iCurWheelItem = 0;
			int iNextSectionTint = 0;
			for( int i=0; i< arraySongs.GetSize(); i++ )
			{
				Song* pSong = arraySongs[i];
				CString sThisSection = GetSectionNameFromSongAndSort( pSong, m_SortOrder );
				if( sThisSection != sLastSection )	// new section, make a section item
				{
					WheelItem &WI = m_WheelItems[iCurWheelItem++];
					WI.LoadFromSectionName( sThisSection );
					WI.SetTintColor( COLOR_SECTION_TINTS[iNextSectionTint++] );
					if( iNextSectionTint >= NUM_SECTION_TINTS )
						iNextSectionTint = 0;
					sLastSection = sThisSection;
				}
				if( sThisSection == m_sExpandedSectionName )	// this song is in the expanded section
				{
					WheelItem &WI = m_WheelItems[iCurWheelItem++];
					WI.LoadFromSong( *pSong );
					WI.SetTintColor( *m_mapGroupNameToColorPtr[pSong->GetGroupName()] );
				}
			}
			m_WheelItems.SetSize( iCurWheelItem );	// make sure we have enough room for all music and section items
		}
		break;
	default:
		ASSERT( true );	// unhandled SORT_ORDER
	}


	if( m_SortOrder == SORT_MOST_PLAYED )
	{
		// init crown icons 
		for( int i=0; i<m_WheelItems.GetSize(); i++ )
		{
			m_WheelItems[i].m_MusicStatusDisplay.SetBlinking( true );
			m_WheelItems[i].m_MusicStatusDisplay.SetRank( i+1 );
		}
	}



	if( m_WheelItems.GetSize() == 0 )
	{
		m_WheelItems.SetSize( 1 );
		m_WheelItems[0].LoadFromSectionName( "No Songs" );
		m_WheelItems[0].SetTintColor( D3DXCOLOR(0.5f,0.5f,0.5f,1) );
	}
}


float MusicWheel::GetBannerY( float fPosOffsetsFromMiddle )
{
	return roundf( (fPosOffsetsFromMiddle)*43.2f );
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


	return roundf( fX );
}


void MusicWheel::RenderPrimitives()
{
	m_sprSelectionBackground.Draw();


	// rewind to bottom item to draw;
	int iIndex = m_iSelection;
	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW/2; i++ )
	{
		iIndex--;
		if( iIndex < 0 )
			iIndex = m_WheelItems.GetSize()-1;
	}

	// iIndex is now the index of the lowest WheelItem to draw
	for( i=-NUM_WHEEL_ITEMS_TO_DRAW/2; i<NUM_WHEEL_ITEMS_TO_DRAW/2; i++ )
	{
		WheelItem& WI = m_WheelItems[iIndex];

		float fThisBannerPositionOffsetFromSelection = m_fPositionOffsetFromSelection + i;

		float fY = GetBannerY( fThisBannerPositionOffsetFromSelection );
		float fX = GetBannerX( fThisBannerPositionOffsetFromSelection );
		WI.SetXY( fX, fY );

		float fBrightness = GetBannerBrightness( m_fPositionOffsetFromSelection );
		float fAlpha = GetBannerAlpha( m_fPositionOffsetFromSelection );

		WI.SetDiffuseColor( D3DXCOLOR(fBrightness, fBrightness, fBrightness, fAlpha) );
		WI.Draw();


		iIndex++;
		if( iIndex > m_WheelItems.GetSize()-1 )
			iIndex = 0;
	}


	m_sprSelectionOverlay.Draw();

	ActorFrame::RenderPrimitives();
}

void MusicWheel::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_sprSelectionOverlay.Update( fDeltaTime );
	m_sprSelectionBackground.Update( fDeltaTime );



	for( int i=0; i<m_WheelItems.GetSize(); i++ )
	{
		m_WheelItems[i].Update( fDeltaTime );
	}


	// update wheel state
	m_fTimeLeftInState -= fDeltaTime;
	if( m_fTimeLeftInState <= 0 )	// time to go to a new state
	{
		switch( m_WheelState )
		{
		case STATE_SWITCHING_MUSIC:
			m_WheelState = STATE_IDLE;	// now, wait for input
			WM->SendMessageToTopWindow( SM_PlayMusicSample, 0 );
			break;
		case STATE_FLYING_OFF_BEFORE_NEXT_SORT:
			{
			m_WheelState = STATE_FLYING_ON_AFTER_NEXT_SORT;
			m_fTimeLeftInState = FADE_TIME;

			Song* pPrevSelectedSong = m_WheelItems[m_iSelection].m_pSong;

			// change the sort order
			m_SortOrder = MusicSortOrder(m_SortOrder+1);
			if( m_SortOrder > NUM_SORT_ORDERS-1 )
				m_SortOrder = (MusicSortOrder)0;
			m_sExpandedSectionName = GetSectionNameFromSongAndSort( pPrevSelectedSong, m_SortOrder );
			RebuildWheelItems();

			m_MusicSortDisplay.Set( m_SortOrder );
			m_MusicSortDisplay.BeginTweening( FADE_TIME, TWEEN_BIAS_BEGIN );
			m_MusicSortDisplay.SetTweenXY( SORT_ICON_ON_SCREEN_X, SORT_ICON_ON_SCREEN_Y );


			// find the previously selected song, and select it
			for( i=0; i<m_WheelItems.GetSize(); i++ )
			{
				if( m_WheelItems[i].m_pSong == pPrevSelectedSong )
					m_iSelection = i;
			}
			}
			break;
		case STATE_FLYING_ON_AFTER_NEXT_SORT:
			WM->SendMessageToTopWindow( SM_PlayMusicSample, 0 );
			m_WheelState = STATE_IDLE;	// now, wait for input
			break;
		case STATE_TWEENING_ON_SCREEN:
			WM->SendMessageToTopWindow( SM_PlayMusicSample, 0 );
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

	if( fabs(m_fPositionOffsetFromSelection) < 0.05f )
		m_fPositionOffsetFromSelection = 0;
	else
	{
		m_fPositionOffsetFromSelection -= fDeltaTime * m_fPositionOffsetFromSelection*4;	// linear
		//m_fPositionOffsetFromSelection += fDeltaTime * m_fPositionOffsetFromSelection/fabs(m_fPositionOffsetFromSelection);	// constant
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

	m_iSelection--;
	if( m_iSelection < 0 )
		m_iSelection = m_WheelItems.GetSize()-1;

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

	m_iSelection++;
	if( m_iSelection > m_WheelItems.GetSize()-1 )
		m_iSelection = 0;

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
	switch( m_WheelItems[m_iSelection].m_WheelItemType )
	{
	case WheelItem::TYPE_SECTION:
		{
		CString sThisItemSectionName = m_WheelItems[m_iSelection].GetSectionName();
		if( m_sExpandedSectionName == sThisItemSectionName )	// already expanded
			m_sExpandedSectionName = "";		// collapse it
		else				// already collapsed
			m_sExpandedSectionName = sThisItemSectionName;	// expand it

		RebuildWheelItems();


		m_soundExpand.PlayRandom();


		m_iSelection = 0;	// reset in case we can't find the last selected song
		// find the section header and select it
		for( int i=0; i<m_WheelItems.GetSize(); i++ )
		{
			if( m_WheelItems[i].m_WheelItemType == WheelItem::TYPE_SECTION  
				&&  m_WheelItems[i].GetSectionName() == sThisItemSectionName )
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

