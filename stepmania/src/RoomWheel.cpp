#include "global.h"
#include "RoomWheel.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "ScreenTextEntry.h"

AutoScreenMessage( SM_BackFromRoomName )

void RoomWheel::Load( CString sType ) 
{
	SetName( sType );
	m_offset = 0;
	LOG->Trace( "RoomWheel::Load('%s')", sType.c_str() );

	LoadFromMetrics( sType );
	LoadVariables();

	FOREACH( WheelItemBase*, m_WheelBaseItems, i )
		SAFE_DELETE( *i );

	m_WheelBaseItems.clear();
	for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
		m_WheelBaseItems.push_back( new RoomWheelItem );

	m_roomInfo.SetName("InfoBox");
	m_roomInfo.SetWidth( THEME->GetMetricF(m_sName,"InfoBoxWidth") );
	m_roomInfo.SetHeight( THEME->GetMetricF(m_sName,"InfoBoxHeight") );
	SET_XY( m_roomInfo );
	m_roomInfo.SetHidden( true );
	OFF_COMMAND( m_roomInfo );
	this->AddChild(&m_roomInfo);

	m_WheelState = STATE_SELECTING;
	m_RoomInfoState = LOCKED;

	AddPerminateItem( new RoomWheelData(TYPE_GENERIC, "Create Room", "Create a new game room", THEME->GetMetricC( m_sName, "CreateRoomColor")) );

	BuildWheelItemsData( m_WheelBaseItemsData );
	RebuildWheelItems();
}

RoomWheelData::RoomWheelData( WheelItemType wit, CString sTitle, CString sDesc, RageColor color ):
	WheelItemBaseData( wit, sTitle, color )
{
	m_sDesc = sDesc;
}

RoomWheelItem::RoomWheelItem( CString sType ):
	WheelItemBase(sType)
{
	SetName( sType );
	Load( sType );
}

void RoomWheelItem::Load(CString sType)
{
	DESC_X				.Load(sType,"DescX");
	DESC_Y				.Load(sType,"DescY");
	DESC_WIDTH			.Load(sType,"DescWidth");
	DESC_ON_COMMAND		.Load(sType,"DescOnCommand");


	m_text.SetHorizAlignString("left");
	TEXT_WIDTH		.Load(sType,"TextWidth");
	m_text.SetMaxWidth(TEXT_WIDTH);

	m_Desc.LoadFromFont( THEME->GetPathF("RoomWheel","text") );
	m_Desc.SetHorizAlignString("left");
	m_Desc.SetShadowLength( 0 );
	m_Desc.SetMaxWidth( DESC_WIDTH );
	m_Desc.SetXY( DESC_X, DESC_Y);
	m_Desc.RunCommands( DESC_ON_COMMAND );
	this->AddChild( &m_Desc );
}

void RoomWheel::BuildWheelItemsData( vector<WheelItemBaseData*> &arrayWheelItemDatas )
{
	if( arrayWheelItemDatas.empty() )
	{
		arrayWheelItemDatas.push_back( new RoomWheelData(TYPE_GENERIC, "- EMPTY -", "", RageColor(1,0,0,1)) );
	}
}

void RoomWheel::AddPerminateItem(RoomWheelData* itemdata)
{
	m_offset++;
	AddItem( itemdata );
}

bool RoomWheel::Select()
{
	RetractInfoBox();
	if( m_iSelection > 0 )
		return WheelBase::Select();
	else if( m_iSelection == 0 )
	{
		// Since this is not actually an option outside of this wheel NULL is a good idea.
		m_LastSelection = NULL;
		ScreenTextEntry::TextEntry( SM_BackFromRoomName, "Enter Room Name:", "", 255 );
	}
	return false;
}

void RoomWheelItem::LoadFromWheelItemBaseData( WheelItemBaseData* pWID )
{
	RoomWheelData* tmpdata = (RoomWheelData*) pWID;
	WheelItemBase::LoadFromWheelItemBaseData( pWID );
	m_Desc.SetText( tmpdata->m_sDesc );
	m_Desc.SetDiffuseColor( pWID->m_color );
	m_text.SetDiffuseColor( pWID->m_color );
}

void RoomWheel::Update( float fDeltaTime )
{
	WheelBase::Update(fDeltaTime);

	if ((m_deployDelay.PeekDeltaTime() >= 1.5) && (m_deployDelay.PeekDeltaTime() < (1.5 + 5)))
		DeployInfoBox();
	else if (m_deployDelay.PeekDeltaTime() >= 1.5 + 5)
		RetractInfoBox();
}

void RoomWheel::Move(int n)
{
	if ((n == 0) && (m_iSelection >= m_offset))
	{
		m_RoomInfoState = CLOSED;
		m_deployDelay.Touch();

		if (m_roomInfo.GetHidden())
			m_roomInfo.SetHidden(false);
	}
	else
		RetractInfoBox();

	WheelBase::Move(n);
}

unsigned int RoomWheel::GetNumItems() const
{
	return m_WheelBaseItemsData.size() - m_offset;
}

void RoomWheel::RemoveItem( int index )
{
	WheelBase::RemoveItem(index + m_offset);
}

void RoomWheel::DeployInfoBox()
{
	if (m_RoomInfoState == CLOSED)
	{
		SET_XY_AND_ON_COMMAND( m_roomInfo );
		m_RoomInfoState = OPEN;
	}
}
	
void RoomWheel::RetractInfoBox()
{
	if (m_RoomInfoState == OPEN)
		OFF_COMMAND( m_roomInfo );
	
	m_RoomInfoState = LOCKED;
}

/*
 * (c) 2004 Josh Allen
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
