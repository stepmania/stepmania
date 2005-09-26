#include "global.h"
#include "WheelBase.h"
#include "RageUtil.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"	// for sending SM_PlayMusicSample
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "RageMath.h"
#include "ThemeManager.h"
#include "RageTextureManager.h"
#include "GameCommand.h"
#include "ActorUtil.h"
#include "Foreach.h"
#include "Style.h"
#include "ThemeMetric.h"
#include "ScreenDimensions.h"


	
const int MAX_WHEEL_SOUND_SPEED = 15;

WheelBase::WheelBase()
{
}

WheelBase::~WheelBase()
{
	FOREACH( WheelItemBase*, m_WheelBaseItems, i )
		SAFE_DELETE( *i );
	m_WheelBaseItems.clear();
	FOREACH( WheelItemBaseData*, m_WheelBaseItemsData, i )
		SAFE_DELETE( *i );
	m_WheelBaseItems.clear();
	m_WheelBaseItemsData.clear();
	m_LastSelection = NULL;
}

void WheelBase::Load( CString sType ) 
{
	LOG->Trace( "WheelBase::Load('%s')", sType.c_str() );

	LoadFromMetrics( sType );
	LoadVariables();

	FOREACH( WheelItemBase*, m_WheelBaseItems, i )
		SAFE_DELETE( *i );
	m_WheelBaseItems.clear();
	for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
	{
		m_WheelBaseItems.push_back( new WheelItemBase );
	}

	m_WheelState = STATE_SELECTING;

	BuildWheelItemsData(m_WheelBaseItemsData);
	RebuildWheelItems();
}

void WheelBase::LoadFromMetrics( CString sType )
{
	SWITCH_SECONDS				.Load(sType,"SwitchSeconds");
	LOCKED_INITIAL_VELOCITY		.Load(sType,"LockedInitialVelocity");
	SCROLL_BAR_X				.Load(sType,"ScrollBarX");
	SCROLL_BAR_HEIGHT			.Load(sType,"ScrollBarHeight");
	ITEM_CURVE_X				.Load(sType,"ItemCurveX");
	USE_LINEAR_WHEEL			.Load(sType,"NoCurving");
	ITEM_SPACING_Y				.Load(sType,"ItemSpacingY");
	WHEEL_3D_RADIUS				.Load(sType,"Wheel3DRadius");
	CIRCLE_PERCENT				.Load(sType,"CirclePercent");
	USE_3D						.Load(sType,"Use3D");
	NUM_WHEEL_ITEMS_TO_DRAW		.Load(sType,"NumWheelItems");
	WHEEL_ITEM_ON_DELAY_CENTER	.Load(sType,"WheelItemOnDelayCenter");
	WHEEL_ITEM_ON_DELAY_OFFSET	.Load(sType,"WheelItemOnDelayOffset");
	WHEEL_ITEM_OFF_DELAY_CENTER	.Load(sType,"WheelItemOffDelayCenter");
	WHEEL_ITEM_OFF_DELAY_OFFSET	.Load(sType,"WheelItemOffDelayOffset");

	m_soundChangeMusic.Load(	THEME->GetPathS(sType,"change"), true );
	m_soundLocked.Load(			THEME->GetPathS(sType,"locked"), true );

	m_sprHighlight.Load( THEME->GetPathG(sType,"highlight") );
	m_sprHighlight->SetName( "Highlight" );
	this->AddChild( m_sprHighlight );
	ActorUtil::OnCommand( m_sprHighlight, sType );

	m_ScrollBar.SetX( SCROLL_BAR_X ); 
	m_ScrollBar.SetBarHeight( SCROLL_BAR_HEIGHT ); 
	this->AddChild( &m_ScrollBar );
}

void WheelBase::LoadVariables()
{
	m_isEmpty = true;
	m_LastSelection = NULL;
	m_iSelection = 0;
	m_fTimeLeftInState = 0;
	m_fPositionOffsetFromSelection = 0;
	m_iSwitchesLeftInSpinDown = 0;
	m_Moving = 0;
}

void WheelBase::GetItemPosition( float fPosOffsetsFromMiddle, float& fX_out, float& fY_out, float& fZ_out, float& fRotationX_out )
{
	if( USE_3D )
	{
		const float curve = CIRCLE_PERCENT*2*PI;
		fRotationX_out = SCALE(fPosOffsetsFromMiddle,-NUM_WHEEL_ITEMS/2.0f,+NUM_WHEEL_ITEMS/2.0f,-curve/2.f,+curve/2.f);
		fX_out = (1-cosf(fPosOffsetsFromMiddle/PI))*ITEM_CURVE_X;
		fY_out = WHEEL_3D_RADIUS*sinf(fRotationX_out);
		fZ_out = -100+WHEEL_3D_RADIUS*cosf(fRotationX_out);
		fRotationX_out *= 180.f/PI;	// to degrees

//		printf( "fRotationX_out = %f\n", fRotationX_out );
	}
	else if(!USE_LINEAR_WHEEL.GetValue())
	{
		fX_out = (1-cosf(fPosOffsetsFromMiddle/PI))*ITEM_CURVE_X;
		fY_out = fPosOffsetsFromMiddle*ITEM_SPACING_Y;
		fZ_out = 0;
		fRotationX_out = 0;

		fX_out = roundf( fX_out );
		fY_out = roundf( fY_out );
		fZ_out = roundf( fZ_out );
	}
	else
	{
		fX_out = fPosOffsetsFromMiddle*ITEM_CURVE_X;
		fY_out = fPosOffsetsFromMiddle*ITEM_SPACING_Y;
		fZ_out = 0;
		fRotationX_out = 0;

		fX_out = roundf( fX_out );
		fY_out = roundf( fY_out );
		fZ_out = roundf( fZ_out );
	}
}

void WheelBase::SetItemPosition( Actor &item, float fPosOffsetsFromMiddle )
{
	float fX, fY, fZ, fRotationX;
	GetItemPosition( fPosOffsetsFromMiddle, fX, fY, fZ, fRotationX );
	item.SetXY( fX, fY );
	item.SetZ( fZ );
	item.SetRotationX( fRotationX );
}

void WheelBase::DrawPrimitives()
{
	// draw outside->inside
	for( int i=0; i<NUM_WHEEL_ITEMS/2; i++ )
		DrawItem( i );
	for( int i=NUM_WHEEL_ITEMS-1; i>=NUM_WHEEL_ITEMS/2; i-- )
		DrawItem( i );

	ActorFrame::DrawPrimitives();
}

void WheelBase::DrawItem( int i )
{
	DrawItem( i, m_WheelBaseItems[i], i - NUM_WHEEL_ITEMS/2 + m_fPositionOffsetFromSelection );
}

void WheelBase::DrawItem( int i, WheelItemBase *display, const float fThisBannerPositionOffsetFromSelection)
{
	if( fabsf(fThisBannerPositionOffsetFromSelection) > NUM_WHEEL_ITEMS_TO_DRAW/2 )
		return;

	switch( m_WheelState )
	{
	case STATE_SELECTING:
	case STATE_LOCKED:
		{
			SetItemPosition( *display, fThisBannerPositionOffsetFromSelection );
		}
		break;
	}

	if( m_WheelState == STATE_LOCKED  &&  i != NUM_WHEEL_ITEMS/2 )
		display->m_fPercentGray = 0.5f;
	else
		display->m_fPercentGray = 0;

	display->Draw();
}

void WheelBase::UpdateScrollbar( unsigned int iSize )
{
	int iTotalNumItems = iSize;
	float fItemAt = m_iSelection - m_fPositionOffsetFromSelection;

	if( NUM_WHEEL_ITEMS >= iTotalNumItems )
	{
		m_ScrollBar.SetPercentage( 0, 1 );
	}
	else
	{
		float fSize = float(NUM_WHEEL_ITEMS) / iTotalNumItems;
		float fCenter = fItemAt / iTotalNumItems;
		fSize *= 0.5f;

		m_ScrollBar.SetPercentage( fCenter - fSize, fCenter + fSize );
	}
}

bool WheelBase::IsSettled() const
{
	if( m_Moving )
		return false;
	if( m_WheelState != STATE_SELECTING && m_WheelState != STATE_LOCKED )
		return false;
	if( m_fPositionOffsetFromSelection != 0 )
		return false;

	return true;
}


void WheelBase::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	UpdateItems( fDeltaTime );

	//Moved to CommonUpdateProcedure, seems to work fine
	//Revert if it happens to break something
	UpdateScrollbar();

	if( m_Moving )
	{
		m_TimeBeforeMovingBegins -= fDeltaTime;
		m_TimeBeforeMovingBegins = max(m_TimeBeforeMovingBegins, 0);
	}

	// update wheel state
	m_fTimeLeftInState -= fDeltaTime;
	if( m_fTimeLeftInState <= 0 )	// time to go to a new state
	{
		UpdateSwitch();
	}

	if( m_WheelState == STATE_LOCKED )
	{
		/* Do this in at most .1 sec chunks, so we don't get weird if we
		 * stop for some reason (and so it behaves the same when being
		 * single stepped). */
		float fTime = fDeltaTime;
		while( fTime > 0 )
		{
			float t = min( fTime, 0.1f );
			fTime -= t;

			m_fPositionOffsetFromSelection = clamp( m_fPositionOffsetFromSelection, -0.3f, +0.3f );

			float fSpringForce = - m_fPositionOffsetFromSelection * LOCKED_INITIAL_VELOCITY;
			m_fLockedWheelVelocity += fSpringForce;

			float fDrag = -m_fLockedWheelVelocity * t*4;
			m_fLockedWheelVelocity += fDrag;

			m_fPositionOffsetFromSelection  += m_fLockedWheelVelocity*t;

			if( fabsf(m_fPositionOffsetFromSelection) < 0.01f  &&  fabsf(m_fLockedWheelVelocity) < 0.01f )
			{
				m_fPositionOffsetFromSelection = 0;
				m_fLockedWheelVelocity = 0;
			}
		}
	}

	if( IsMoving() )
	{
		/* We're automatically moving.  Move linearly, and don't clamp
		 * to the selection. */
		float fSpinSpeed = m_SpinSpeed*m_Moving;
		m_fPositionOffsetFromSelection -= fSpinSpeed*fDeltaTime;

		/* Make sure that we don't go further than 1 away, in case the
		 * speed is very high or we miss a lot of frames. */
		m_fPositionOffsetFromSelection  = clamp(m_fPositionOffsetFromSelection, -1.0f, 1.0f);
		
		/* If it passed the selection, move again. */
		if((m_Moving == -1 && m_fPositionOffsetFromSelection >= 0) ||
		   (m_Moving == 1 && m_fPositionOffsetFromSelection <= 0))
		{
			ChangeMusic( m_Moving );

			if( PREFSMAN->m_iMusicWheelSwitchSpeed < MAX_WHEEL_SOUND_SPEED )
				m_soundChangeMusic.Play();
		}

		if( PREFSMAN->m_iMusicWheelSwitchSpeed >= MAX_WHEEL_SOUND_SPEED &&
			m_MovingSoundTimer.PeekDeltaTime() >= 1.0f / MAX_WHEEL_SOUND_SPEED )
		{
			m_MovingSoundTimer.GetDeltaTime();
			m_soundChangeMusic.Play();
		}
	}
	else
	{
		// "rotate" wheel toward selected song
		float fSpinSpeed = 0.2f + fabsf( m_fPositionOffsetFromSelection ) / SWITCH_SECONDS;

		if( m_fPositionOffsetFromSelection > 0 )
		{
			m_fPositionOffsetFromSelection -= fSpinSpeed*fDeltaTime;
			if( m_fPositionOffsetFromSelection < 0 )
				m_fPositionOffsetFromSelection = 0;
		}
		else if( m_fPositionOffsetFromSelection < 0 )
		{
			m_fPositionOffsetFromSelection += fSpinSpeed*fDeltaTime;
			if( m_fPositionOffsetFromSelection > 0 )
				m_fPositionOffsetFromSelection = 0;
		}
	}
}

void WheelBase::UpdateItems( float fDeltaTime )
{
	for( unsigned i = 0; i < unsigned(NUM_WHEEL_ITEMS); i++)
	{
		m_WheelBaseItems[i]->Update( fDeltaTime );
	}
}

void WheelBase::UpdateSwitch()
{
	switch( m_WheelState )
	{
	case STATE_TWEENING_ON_SCREEN:
		m_fTimeLeftInState = 0;
		if( (GAMESTATE->IsExtraStage() && !PREFSMAN->m_bPickExtraStage) || GAMESTATE->IsExtraStage2() )
		{
			m_WheelState = STATE_LOCKED;
			SCREENMAN->PlayStartSound();
			m_fLockedWheelVelocity = 0;
		}
		else
		{
			m_WheelState = STATE_SELECTING;
		}
		break;
	case STATE_TWEENING_OFF_SCREEN:
		m_WheelState = STATE_WAITING_OFF_SCREEN;
		m_fTimeLeftInState = 0;
		break;
	case STATE_SELECTING:
		m_fTimeLeftInState = 0;
		break;
	case STATE_WAITING_OFF_SCREEN:
		break;
	case STATE_LOCKED:
		break;
	default:
		ASSERT(0);	// all state changes should be handled explicitly
		break;
	}
}

bool WheelBase::Select()	// return true if this selection ends the screen
{
	LOG->Trace( "WheelBase::Select()" );

	m_Moving = 0;

	if( !m_isEmpty )
	{
		switch( m_WheelBaseItemsData[m_iSelection]->m_Type )
		{
		case TYPE_GENERIC:
			m_LastSelection = m_WheelBaseItemsData[m_iSelection];
			return false;
		case TYPE_SECTION:
			{
				CString sThisItemSectionName = m_WheelBaseItemsData[m_iSelection]->m_sText;
				if( m_sExpandedSectionName == sThisItemSectionName )	// already expanded
					m_sExpandedSectionName = "";		// collapse it
				else				// already collapsed
					m_sExpandedSectionName = sThisItemSectionName;	// expand it

				m_soundExpand.Play();
			}
			return false;
		default:
			ASSERT(0);
			return false;
		}
	}
	return false;
}

// return true if this selection ends the screen
WheelItemBaseData* WheelBase::GetItem( unsigned int iIndex )
{
	if( !m_isEmpty && iIndex < m_WheelBaseItemsData.size() )
		return m_WheelBaseItemsData[iIndex];

	return NULL;
}

int WheelBase::IsMoving() const
{
	return m_Moving && m_TimeBeforeMovingBegins == 0;
}

void WheelBase::TweenOnScreen( bool bChangingSort )
{
	m_WheelState = STATE_TWEENING_ON_SCREEN;

	SetItemPosition( *m_sprHighlight, 0 );

	COMMAND( m_sprHighlight, "StartOn");
	if( bChangingSort )
	{
		const float delay = fabsf(NUM_WHEEL_ITEMS/2-WHEEL_ITEM_ON_DELAY_CENTER) * WHEEL_ITEM_ON_DELAY_OFFSET;
		m_sprHighlight->BeginTweening( delay ); // sleep
		COMMAND( m_sprHighlight, "FinishOnSort");
	}
	else
	{
		COMMAND( m_sprHighlight, "FinishOn");
	}

	m_ScrollBar.SetX( SCROLL_BAR_X );
	m_ScrollBar.AddX( 30 );
	if( bChangingSort )
		m_ScrollBar.BeginTweening( 0.2f );	// sleep
	else
		m_ScrollBar.BeginTweening( 0.7f );	// sleep
	m_ScrollBar.BeginTweening( 0.2f , Actor::TWEEN_ACCELERATE );
	m_ScrollBar.AddX( -30 );

	TweenOnScreenUpdateItems( bChangingSort );

	if( bChangingSort )
		HurryTweening( 0.25f );

	m_fTimeLeftInState = GetTweenTimeLeft() + 0.100f;
}

void WheelBase::TweenOnScreenUpdateItems( bool bChangingSort )
{
	for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
	{
		WheelItemBase *display = m_WheelBaseItems[i];
		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS/2 + m_fPositionOffsetFromSelection;
		SetItemPosition( *display, fThisBannerPositionOffsetFromSelection );

		COMMAND( display, "StartOn");
		const float delay = fabsf(i-WHEEL_ITEM_ON_DELAY_CENTER) * WHEEL_ITEM_ON_DELAY_OFFSET;
		display->BeginTweening( delay ); // sleep
		COMMAND( display, "FinishOn");
		if( bChangingSort )
			display->HurryTweening( 0.25f );
	}
}
						   
void WheelBase::TweenOffScreen( bool bChangingSort )
{
	m_WheelState = STATE_TWEENING_OFF_SCREEN;

	SetItemPosition( *m_sprHighlight, 0 );

	COMMAND( m_sprHighlight, "StartOff");
	if( bChangingSort )
	{
		/* When changing sort, tween the overlay with the item in the center;
		 * having it separate looks messy when we're moving fast. */
		const float delay = fabsf(NUM_WHEEL_ITEMS/2-WHEEL_ITEM_ON_DELAY_CENTER) * WHEEL_ITEM_ON_DELAY_OFFSET;
		m_sprHighlight->BeginTweening( delay ); // sleep
		COMMAND( m_sprHighlight, "FinishOffSort");
	} 
	else
	{
		COMMAND( m_sprHighlight, "FinishOff");
	}

	m_ScrollBar.BeginTweening( 0 );
	m_ScrollBar.BeginTweening( 0.2f, Actor::TWEEN_ACCELERATE );
	m_ScrollBar.SetX( SCROLL_BAR_X+30 );	

	TweenOffScreenUpdateItems( bChangingSort );

	if( bChangingSort )
		HurryTweening( 0.25f );

	m_fTimeLeftInState = GetTweenTimeLeft() + 0.100f;
}

void WheelBase::TweenOffScreenUpdateItems( bool bChangingSort )
{
	for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
	{
		WheelItemBase *display = m_WheelBaseItems[i];
		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS/2 + m_fPositionOffsetFromSelection;
		SetItemPosition( *display, fThisBannerPositionOffsetFromSelection );

		COMMAND( display, "StartOff");
		const float delay = fabsf(i-WHEEL_ITEM_OFF_DELAY_CENTER) * WHEEL_ITEM_OFF_DELAY_OFFSET;
		display->BeginTweening( delay );	// sleep
		COMMAND( display, "FinishOff");
		if( bChangingSort )
			display->HurryTweening( 0.25f );
	}
}

void WheelBase::ChangeMusicUnlessLocked( int n )
{
	if( m_WheelState == STATE_LOCKED )
	{
		if(n)
		{
			int iSign = n/abs(n);
			m_fLockedWheelVelocity = iSign*LOCKED_INITIAL_VELOCITY;
			m_soundLocked.Play();
		}
		return;
	}

	ChangeMusic( n );
}

void WheelBase::Move(int n)
{
	if( n == m_Moving )
		return;

	if( m_WheelState == STATE_LOCKED )
	{
		if(n)
		{
			int iSign = n/abs(n);
			m_fLockedWheelVelocity = iSign*LOCKED_INITIAL_VELOCITY;
			m_soundLocked.Play();
		}
		return;
	}

	if (!MoveSpecific(n))
		return;

	m_TimeBeforeMovingBegins = 1/4.0f;
	m_SpinSpeed = float(PREFSMAN->m_iMusicWheelSwitchSpeed);
	m_Moving = n;
	
	if( m_Moving )
		ChangeMusic(m_Moving);
}

bool WheelBase::MoveSpecific( int n )
{
	/* If we're not selecting, discard this.  We won't ignore it; we'll
	 * get called again every time the key is repeated. */
	/* Still process Move(0) so we sometimes continue moving immediate 
	 * after the sort change finished and before the repeat event causes a 
	 * Move(0). -Chris */
	switch( m_WheelState )
	{
	case STATE_SELECTING:
		break;
	case STATE_FLYING_OFF_BEFORE_NEXT_SORT:
	case STATE_FLYING_ON_AFTER_NEXT_SORT:
		if( n!= 0 )
			return false;
		break;
	default:
		return false;	// don't continue
	}

	if(m_Moving != 0 && n == 0 && m_TimeBeforeMovingBegins == 0)
	{
		/* We were moving, and now we're stopping.  If we're really close to
		 * the selection, move to the next one, so we have a chance to spin down
		 * smoothly. */
		if(fabsf(m_fPositionOffsetFromSelection) < 0.25f )
			ChangeMusic(m_Moving);

		/* Make sure the user always gets an SM_SongChanged when
		 * Moving() is 0, so the final banner, etc. always gets set. */
//		SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );
	}

	return true;
}

void WheelBase::AddItem( WheelItemBaseData* pItemData )
{
	m_WheelBaseItemsData.push_back( pItemData );
	int iVisible = FirstVisibleIndex();
	int iIndex = m_WheelBaseItemsData.size();

	if( m_isEmpty )
	{
		m_isEmpty = false;
		// Remove the - Empty - field when we add an object from an empty state.
		RemoveItem(0);
	}

	// If the item was shown in the wheel, rebuild the wheel
	if( 0 <= iIndex - iVisible && iIndex - iVisible < NUM_WHEEL_ITEMS )
	{
		RebuildWheelItems();
	}
}

void WheelBase::ChangeMusic( int iDist )
{
	m_iSelection += iDist;
	wrap( m_iSelection, m_WheelBaseItemsData.size() );

	RebuildWheelItems( iDist );

	m_fPositionOffsetFromSelection += iDist;

//	SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );

	/* If we're moving automatically, don't play this; it'll be called in Update. */
	if(!IsMoving())
		m_soundChangeMusic.Play();
}

void WheelBase::BuildWheelItemsData( vector<WheelItemBaseData*> &arrayWheelItemDatas )
{
	if( arrayWheelItemDatas.empty() )
	{
		arrayWheelItemDatas.push_back( new WheelItemBaseData(TYPE_GENERIC, "- EMPTY -", RageColor(1,0,0,1)) );
	}
}

void WheelBase::RebuildWheelItems( int iDist )
{
	const vector<WheelItemBaseData *> &data = m_WheelBaseItemsData;
	vector<WheelItemBase *> &items = m_WheelBaseItems;

	// rewind to first index that will be displayed;
	int iFirstVisibleIndex = m_iSelection;
	if( m_iSelection > int(data.size()-1) )
		m_iSelection = 0;
		
	// find the first wheel item shown
	iFirstVisibleIndex -= NUM_WHEEL_ITEMS/2;

	ASSERT(data.size());
	wrap( iFirstVisibleIndex, data.size() );

	// iIndex is now the index of the lowest WheelItem to draw

	if( iDist == INT_MAX )
	{
		// Refresh all
		for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
		{
			int iIndex = iFirstVisibleIndex + i;
			wrap( iIndex, data.size() );

			WheelItemBaseData *pData    = data[iIndex];
			WheelItemBase     *pDisplay = items[i];

			pDisplay->LoadFromWheelItemBaseData( pData );
		}
	}
	else
	{
		// Shift items and refresh only those that have changed.
		CircularShift( items, iDist );
		if( iDist > 0 )
		{
			for( int i=NUM_WHEEL_ITEMS-iDist; i<NUM_WHEEL_ITEMS; i++ )
			{
				int iIndex = iFirstVisibleIndex + i;
				wrap( iIndex, data.size() );

				WheelItemBaseData *pData    = data[iIndex];
				WheelItemBase     *pDisplay = items[i];

				pDisplay->LoadFromWheelItemBaseData( pData );
			}
		}
		else if( iDist < 0 )
		{
			for( int i=0; i < -iDist; i++ )
			{
				int iIndex = iFirstVisibleIndex + i;
				wrap( iIndex, data.size() );

				WheelItemBaseData *pData    = data[iIndex];
				WheelItemBase     *pDisplay = items[i];

				pDisplay->LoadFromWheelItemBaseData( pData );
			}
		}
	}
}

WheelItemBaseData* WheelBase::LastSelected( )
{
	if (m_isEmpty)
		return NULL;
	else
		return m_LastSelection;
}

void WheelBase::RemoveItem( int index )
{
	if( m_isEmpty || index >= (int)m_WheelBaseItemsData.size() )
		return;

	vector<WheelItemBaseData *>::iterator i = m_WheelBaseItemsData.begin();
	i += index;

	// If this item's data happened to be last selected, make it NULL.
	if( m_LastSelection == *i )
		m_LastSelection = NULL;

	SAFE_DELETE( *i );
	m_WheelBaseItemsData.erase(i);

	if( m_WheelBaseItemsData.size() < 1 )
	{
		m_isEmpty = true;
		BuildWheelItemsData(m_WheelBaseItemsData);
	}

	RebuildWheelItems();
}

int WheelBase::FirstVisibleIndex()
{
	// rewind to first index that will be displayed;
	int iFirstVisibleIndex = m_iSelection;
	if( m_iSelection > int(m_WheelBaseItemsData.size()-1) )
		m_iSelection = 0;
	
	// find the first wheel item shown
	iFirstVisibleIndex -= NUM_WHEEL_ITEMS/2;

	wrap( iFirstVisibleIndex, m_WheelBaseItemsData.size() );
	return iFirstVisibleIndex;
}

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez, Glenn Maynard, Josh Allen
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
