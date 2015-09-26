#ifndef RARAR_VALUES_H
#define RARAR_VALUES_H

#include "GameConstantsAndTypes.h"
#include "ThemeMetric.h"

/** @brief Unknown radar values are given a default value. */
#define RADAR_VAL_UNKNOWN -1

class XNode;
struct lua_State;
/** @brief Cached song statistics. */
struct RadarValues
{
private:
	float m_Values[NUM_RadarCategory];
public:
	float operator[](RadarCategory cat) const { return m_Values[cat]; }
	float& operator[](RadarCategory cat) { return m_Values[cat]; }
	float operator[](int cat) const { return m_Values[cat]; }
	float& operator[](int cat) { return m_Values[cat]; }

	RadarValues();
	void MakeUnknown();
	void Zero();

	/**
	 * @brief Add one set of radar values to another.
	 * @param other The other set of radar values to add.
	 * @return the new set of radar values.
	 */
	RadarValues& operator+=( const RadarValues& other )
	{
		FOREACH_ENUM( RadarCategory, rc )
		{
			(*this)[rc] += other[rc];
		}
		return *this;
	}
	/**
	 * @brief Determine if one set of radar values are equal to another.
	 * @param other The otehr set of radar values.
	 * @return true if the two sets are equal, false otherwise.
	 */
	bool operator==( const RadarValues& other ) const
	{
		FOREACH_ENUM( RadarCategory, rc )
		{
			if((*this)[rc] != other[rc])
			{
				return false;
			}
		}
		return true;
	}
	/**
	 * @brief Determine if one set of radar values are not equal to another.
	 * @param other The otehr set of radar values.
	 * @return true if the two sets are not equal, false otherwise.
	 */
	bool operator!=( const RadarValues& other ) const
	{
		return !operator==( other );
	}

	XNode* CreateNode( bool bIncludeSimpleValues, bool bIncludeComplexValues ) const;
	void LoadFromNode( const XNode* pNode );

	RString ToString( int iMaxValues = -1 ) const; // default = all
	void FromString( RString sValues );

	static ThemeMetric<bool> WRITE_SIMPLE_VALIES;
	static ThemeMetric<bool> WRITE_COMPLEX_VALIES;

	// Lua
	void PushSelf( lua_State *L );
};


#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
