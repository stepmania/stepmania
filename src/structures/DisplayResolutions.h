#ifndef DisplayResolutions_H
#define DisplayResolutions_H

#include <set>
/** @brief The dimensions of the program. */
class DisplayResolution
{
public:
	/** @brief The width of the program. */
	int iWidth;
	/** @brief The height of the program. */
	int iHeight;
	/** @brief Is this display stretched/used for widescreen? */
	bool bStretched;

	/**
	 * @brief Determine if one DisplayResolution is less than the other.
	 * @param other the other DisplayResolution to check.
	 * @return true if this DisplayResolution is less than the other, or false otherwise. */
	bool operator<( const DisplayResolution &other ) const
	{
/** @brief A quick way to compare the two DisplayResolutions. */
#define COMPARE(x) if( x != other.x ) return x < other.x;
		COMPARE( iWidth );
		COMPARE( iHeight );
		COMPARE( bStretched );
#undef COMPARE
		return false;
	}
};
/** @brief The collection of DisplayResolutions available within the program. */
typedef set<DisplayResolution> DisplayResolutions;

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2005
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
