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

#define NUM_WHEEL_ITEMS		((int)ceil(NUM_WHEEL_ITEMS_TO_DRAW+2))

// leaving this one under ScreenSelectMusic because that is the only place it takes effect anyway.
//ThemeMetric<CString> DEFAULT_SORT				("ScreenSelectMusic","DefaultSort");

//static CString SECTION_COLORS_NAME( size_t i )	{ return ssprintf("SectionColor%d",int(i+1)); }
static CString CHOICE_NAME( CString s )			{ return ssprintf("Choice%s",s.c_str()); }
		
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
//	SECTION_COLORS				.Load(sType,SECTION_COLORS_NAME,NUM_SECTION_COLORS);

	FOREACH( WheelItemBase*, m_WheelBaseItems, i )
		SAFE_DELETE( *i );
	m_WheelBaseItems.clear();
	for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
	{
		m_WheelBaseItems.push_back( new WheelItemBase );
	}

	m_WheelState = STATE_SELECTING_GENERIC;

	BuildWheelItemsData(m_WheelBaseItemsData);
	RebuildWheelItems();
}

void WheelBase::LoadFromMetrics( CString sType ) {

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

void WheelBase::LoadVariables() {
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
	if( USE_3D )
	{
//		DISPLAY->PushMatrix();
//		DISPLAY->EnterPerspective(45, false);  //Not needed for 3D

		// construct view and project matrix
//		RageVector3 Up( 0.0f, 1.0f, 0.0f );
//		RageVector3 Eye( SCREEN_CENTER_X, SCREEN_CENTER_Y, 550 );
//		RageVector3 At( SCREEN_CENTER_X, SCREEN_CENTER_Y, 0 );

//		DISPLAY->LoadLookAt(60, Eye, At, Up); //Changed from DISPLAY->LookAt(Eye, At, Up)
	}

	// draw outside->inside
	for( int i=0; i<NUM_WHEEL_ITEMS/2; i++ )
		DrawItem( i );
	for( int i=NUM_WHEEL_ITEMS-1; i>=NUM_WHEEL_ITEMS/2; i-- )
		DrawItem( i );


	ActorFrame::DrawPrimitives();
	
	if( USE_3D )
	{
//		DISPLAY->ExitPerspective(); //Not needed?
//		DISPLAY->PopMatrix();
	}
}


void WheelBase::DrawItem( int i )
{
	WheelItemBase *display = m_WheelBaseItems[i];

	const float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS/2 + m_fPositionOffsetFromSelection;
	if( fabsf(fThisBannerPositionOffsetFromSelection) > NUM_WHEEL_ITEMS_TO_DRAW/2 )
		return;

	switch( m_WheelState )
	{
	case STATE_SELECTING_GENERIC:
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


void WheelBase::UpdateScrollbar()
{
	int total_num_items = m_WheelBaseItemsData.size();
	float item_at=m_iSelection - m_fPositionOffsetFromSelection;

	if(NUM_WHEEL_ITEMS >= total_num_items)
	{
		m_ScrollBar.SetPercentage( 0, 1 );
	} else {
		float size = float(NUM_WHEEL_ITEMS) / total_num_items;
		float center = item_at / total_num_items;
		size *= 0.5f;

		m_ScrollBar.SetPercentage( center - size, center + size );
	}
}

bool WheelBase::IsSettled() const
{
	if( m_Moving )
		return false;
	if( m_WheelState != STATE_SELECTING_GENERIC && m_WheelState != STATE_LOCKED )
		return false;
	if( m_fPositionOffsetFromSelection != 0 )
		return false;

	return true;
}


void WheelBase::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	for( unsigned i = 0; i < unsigned(NUM_WHEEL_ITEMS); i++)
	{
		m_WheelBaseItems[i]->Update( fDeltaTime );
	}
/*
	//Moved to CommonUpdateProcedure, seems to work fine
	//Revert if it happens to break something
	UpdateScrollbar();

	if( m_Moving )
	{
		m_TimeBeforeMovingBegins -= fDeltaTime;
		m_TimeBeforeMovingBegins = max(m_TimeBeforeMovingBegins, 0);
	}
*/
	// update wheel state
	m_fTimeLeftInState -= fDeltaTime;
	if( m_fTimeLeftInState <= 0 )	// time to go to a new state
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
				m_WheelState = STATE_SELECTING_GENERIC;
			}
			break;
		case STATE_TWEENING_OFF_SCREEN:
			m_WheelState = STATE_WAITING_OFF_SCREEN;
			m_fTimeLeftInState = 0;
			break;
		case STATE_SELECTING_GENERIC:
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

	CommonUpdateProcedure(fDeltaTime);
}

void WheelBase::CommonUpdateProcedure(float fDeltaTime) {

	//This bit of code now happens after it's normal execution, but it seems to work fine.
	UpdateScrollbar();

	if( m_Moving )
	{
		m_TimeBeforeMovingBegins -= fDeltaTime;
		m_TimeBeforeMovingBegins = max(m_TimeBeforeMovingBegins, 0);
	}

	//The following code is just fine here.
	if( m_WheelState == STATE_LOCKED )
	{
		/* Do this in at most .1 sec chunks, so we don't get weird if we
		 * stop for some reason (and so it behaves the same when being
		 * single stepped). */
		float tm = fDeltaTime;
		while(tm > 0)
		{
			float t = min(tm, 0.1f);
			tm -= t;

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
			ChangeMusic(m_Moving);

			if(PREFSMAN->m_iMusicWheelSwitchSpeed < MAX_WHEEL_SOUND_SPEED)
				m_soundChangeMusic.Play();
		}

		if(PREFSMAN->m_iMusicWheelSwitchSpeed >= MAX_WHEEL_SOUND_SPEED &&
			m_MovingSoundTimer.PeekDeltaTime() >= 1.0f / MAX_WHEEL_SOUND_SPEED)
		{
			m_MovingSoundTimer.GetDeltaTime();
			m_soundChangeMusic.Play();
		}
	}
	else
	{
		// "rotate" wheel toward selected song
		float fSpinSpeed = 0.2f + fabsf(m_fPositionOffsetFromSelection)/SWITCH_SECONDS;

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

bool WheelBase::Select()	// return true if this selection ends the screen
{
	LOG->Trace( "WheelBase::Select()" );

	m_Moving = 0;

	if (!m_isEmpty)
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

int WheelBase::IsMoving() const
{
	return m_Moving && m_TimeBeforeMovingBegins == 0;
}

void WheelBase::TweenOnScreen(bool changing_sort)
{
	m_WheelState = STATE_TWEENING_ON_SCREEN;

	SetItemPosition( *m_sprHighlight, 0 );

	COMMAND( m_sprHighlight, "StartOn");
	if( changing_sort )
	{
		const float delay = fabsf(NUM_WHEEL_ITEMS/2-WHEEL_ITEM_ON_DELAY_CENTER) * WHEEL_ITEM_ON_DELAY_OFFSET;
		m_sprHighlight->BeginTweening( delay ); // sleep
		COMMAND( m_sprHighlight, "FinishOnSort");
	} else {
		COMMAND( m_sprHighlight, "FinishOn");
	}

	m_ScrollBar.SetX( SCROLL_BAR_X );
	m_ScrollBar.AddX( 30 );
	if(changing_sort)
		m_ScrollBar.BeginTweening( 0.2f );	// sleep
	else
		m_ScrollBar.BeginTweening( 0.7f );	// sleep
	m_ScrollBar.BeginTweening( 0.2f , Actor::TWEEN_ACCELERATE );
	m_ScrollBar.AddX( -30 );

	for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
	{
		WheelItemBase *display = m_WheelBaseItems[i];
		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS/2 + m_fPositionOffsetFromSelection;
		SetItemPosition( *display, fThisBannerPositionOffsetFromSelection );

		COMMAND( display, "StartOn");
		const float delay = fabsf(i-WHEEL_ITEM_ON_DELAY_CENTER) * WHEEL_ITEM_ON_DELAY_OFFSET;
		display->BeginTweening( delay ); // sleep
		COMMAND( display, "FinishOn");
		if( changing_sort )
			display->HurryTweening( 0.25f );
	}

	if( changing_sort )
		HurryTweening( 0.25f );

	m_fTimeLeftInState = GetTweenTimeLeft() + 0.100f;
}
						   
void WheelBase::TweenOffScreen(bool changing_sort)
{
	m_WheelState = STATE_TWEENING_OFF_SCREEN;

	SetItemPosition( *m_sprHighlight, 0 );

	COMMAND( m_sprHighlight, "StartOff");
	if(changing_sort)
	{
		/* When changing sort, tween the overlay with the item in the center;
		 * having it separate looks messy when we're moving fast. */
		const float delay = fabsf(NUM_WHEEL_ITEMS/2-WHEEL_ITEM_ON_DELAY_CENTER) * WHEEL_ITEM_ON_DELAY_OFFSET;
		m_sprHighlight->BeginTweening( delay ); // sleep
		COMMAND( m_sprHighlight, "FinishOffSort");
	} else {
		COMMAND( m_sprHighlight, "FinishOff");
	}
	COMMAND( m_sprHighlight, "FinishOff");

	m_ScrollBar.BeginTweening( 0 );
	m_ScrollBar.BeginTweening( 0.2f, Actor::TWEEN_ACCELERATE );
	m_ScrollBar.SetX( SCROLL_BAR_X+30 );	

	for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
	{
		WheelItemBase *display = m_WheelBaseItems[i];
		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS/2 + m_fPositionOffsetFromSelection;
		SetItemPosition( *display, fThisBannerPositionOffsetFromSelection );

		COMMAND( display, "StartOff");
		const float delay = fabsf(i-WHEEL_ITEM_OFF_DELAY_CENTER) * WHEEL_ITEM_OFF_DELAY_OFFSET;
		display->BeginTweening( delay );	// sleep
		COMMAND( display, "FinishOff");
		if( changing_sort )
			display->HurryTweening( 0.25f );
	}

	if( changing_sort )
		HurryTweening( 0.25f );

	m_fTimeLeftInState = GetTweenTimeLeft() + 0.100f;
}

void WheelBase::Move(int n)
{
	if(n == m_Moving)
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

	/* If we're not selecting, discard this.  We won't ignore it; we'll
	 * get called again every time the key is repeated. */
	/* Still process Move(0) so we sometimes continue moving immediate 
	 * after the sort change finished and before the repeat event causes a 
	 * Move(0). -Chris */
	switch( m_WheelState )
	{
	case STATE_SELECTING_GENERIC:
		break;
	case STATE_FLYING_OFF_BEFORE_NEXT_SORT:
	case STATE_FLYING_ON_AFTER_NEXT_SORT:
		if( n!= 0 )
			return;
		break;
	default:
		return;	// don't continue
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

	m_TimeBeforeMovingBegins = 1/4.0f;
	m_SpinSpeed = float(PREFSMAN->m_iMusicWheelSwitchSpeed);
	m_Moving = n;
	
	if(m_Moving)
		ChangeMusic(m_Moving);
}

void WheelBase::AddItem(WheelItemBaseData* itemdata)
{
	int visible, index;
	m_WheelBaseItemsData.push_back(itemdata);
	visible = FirstVisibleIndex();
	index = m_WheelBaseItemsData.size();

	if (m_isEmpty)
	{
		m_isEmpty = false;
		//Remove the - Empty - field when we add an object from an empty state.
		RemoveItem(0);
	}

	//If the item was shown in the wheel, rebuild the wheel
	if ((0 <= (index - visible)) && ((index - visible) < NUM_WHEEL_ITEMS))
	{
			RebuildWheelItems();
			SCREENMAN->SystemMessageNoAnimate("redraw");
	}
}

void WheelBase::ChangeMusic(int dist)
{
	m_iSelection += dist;
	wrap( m_iSelection, m_WheelBaseItemsData.size() );

	RebuildWheelItems( dist );

	m_fPositionOffsetFromSelection += dist;

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

void WheelBase::RebuildWheelItems( int dist )
{
	// rewind to first index that will be displayed;
	int iFirstVisibleIndex = m_iSelection;
	if( m_iSelection > int(m_WheelBaseItemsData.size()-1) )
		m_iSelection = 0;
		
	// find the first wheel item shown
	iFirstVisibleIndex -= NUM_WHEEL_ITEMS/2;

	ASSERT(m_WheelBaseItemsData.size());
	wrap( iFirstVisibleIndex, m_WheelBaseItemsData.size() );

	// iIndex is now the index of the lowest WheelItem to draw

	if( dist == -999999 )
	{
		LOG->Info("data:%d", m_WheelBaseItemsData.size());
		// Refresh all
		for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
		{
			LOG->Info("i:%d", i);
			int iIndex = iFirstVisibleIndex + i;
			wrap( iIndex, m_WheelBaseItemsData.size() );
			LOG->Info("iIndex:%d", iIndex);


			WheelItemBaseData	*data   = m_WheelBaseItemsData[iIndex];
			WheelItemBase	*display = m_WheelBaseItems[i];
			LOG->Info("DISP");
			display->LoadFromWheelItemBaseData( data );
		}
	}
	else
	{
		// Shift items and refresh only those that have changed.
		CircularShift( m_WheelBaseItems, dist );
		if( dist > 0 )
		{
			for( int i=NUM_WHEEL_ITEMS-dist; i<NUM_WHEEL_ITEMS; i++ )
			{
				int iIndex = iFirstVisibleIndex + i;
				wrap( iIndex, m_WheelBaseItemsData.size() );

				WheelItemBaseData	*data   = m_WheelBaseItemsData[iIndex];
				WheelItemBase	*display = m_WheelBaseItems[i];

				display->LoadFromWheelItemBaseData( data );
			}
		}
		else if( dist < 0 )
		{
			for( int i=0; i<-dist; i++ )
			{
				int iIndex = iFirstVisibleIndex + i;
				wrap( iIndex, m_WheelBaseItemsData.size() );
				WheelItemBaseData	*data   = m_WheelBaseItemsData[iIndex];
				WheelItemBase	*display = m_WheelBaseItems[i];
				display->LoadFromWheelItemBaseData( data );
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
	if (!m_isEmpty)
	{
		if (index < m_WheelBaseItemsData.size())
		{
			vector<WheelItemBaseData *>::iterator i = m_WheelBaseItemsData.begin();
			i += index;

			//If this item's data happened to be last selected, make it NULL.
			if (m_LastSelection == *i)
				m_LastSelection = NULL;

			SAFE_DELETE( *i );
			m_WheelBaseItemsData.erase(i);

			if (m_WheelBaseItemsData.size() < 1)
			{
				m_isEmpty = true;
				BuildWheelItemsData(m_WheelBaseItemsData);
			}

			RebuildWheelItems();
		}
	}
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
