#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageTimer.cpp

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "RageTimer.h"
#include "RageLog.h"
#include "DXUtil.h"

 

RageTimer*		TIMER	= NULL;


const float SECS_IN_DAY	=	60*60*24;

RageTimer::RageTimer()
{
	m_fTimeSinceStart = m_fLastDeltaTime = 0;
	DXUtil_Timer( TIMER_START );    // Start the accurate timer	
}

RageTimer::~RageTimer()
{
	DXUtil_Timer( TIMER_STOP );
}

float RageTimer::GetDeltaTime()
{
	m_fLastDeltaTime = DXUtil_Timer( TIMER_GETELAPSEDTIME );
	m_fTimeSinceStart += m_fLastDeltaTime;
	if( m_fTimeSinceStart > SECS_IN_DAY )
		m_fTimeSinceStart = SECS_IN_DAY; 
	return m_fLastDeltaTime;
}

float RageTimer::PeekDeltaTime()
{
	return m_fLastDeltaTime;
}

float RageTimer::GetTimeSinceStart()
{
	return m_fTimeSinceStart;
}

