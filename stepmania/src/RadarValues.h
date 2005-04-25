/* RadarValues - Cached song statistics. */

#ifndef RARAR_VALUES_H
#define RARAR_VALUES_H

#include "GameConstantsAndTypes.h"

#define RADAR_VAL_UNKNOWN -1

struct XNode;

struct RadarValues
{
	union Values
	{
		struct
		{
			float fStream;
			float fVoltage;
			float fAir;
			float fFreeze;
			float fChaos;
			float fNumTapsAndHolds;
			float fNumJumps;
			float fNumHolds;
			float fNumMines;
			float fNumHands;
			float fNumRolls;
		} v;
		float f[NUM_RADAR_CATEGORIES];
	} m_Values;

    operator const float* () const	{ return m_Values.f; };
    operator float* ()				{ return m_Values.f; };

	RadarValues();
	void MakeUnknown();
	void Zero();
	

	RadarValues& operator+=( const RadarValues& other )
	{
		FOREACH_RadarCategory( rc )
			m_Values.f[rc] += other.m_Values.f[rc];
		return *this;
	}
	bool operator==( const RadarValues& other ) const
	{
		FOREACH_RadarCategory( rc )
		{
			if( m_Values.f[rc] != other.m_Values.f[rc] )
				return false;
		}
		return true;
	}
	bool operator!=( const RadarValues& other ) const
	{
		return !operator==( other );
	}

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );

	CString ToString( int iMaxValues = -1 ) const; // default = all
	void FromString( CString sValues );
};


#endif

/*
 * (c) 2001-2004 Chris Danford
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
