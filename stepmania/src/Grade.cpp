#include "global.h"
#include "Grade.h"
#include "RageUtil.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "EnumHelper.h"
#include "LuaManager.h"
#include "LuaFunctions.h"

LuaFunction( GradeToString,			GradeToString((Grade)IArg(1)) )

RString GradeToLocalizedString( Grade g )
{
	RString s = GradeToString(g);
	if( !THEME->HasString("Grade",s) )
		return "???";
	return THEME->GetString( "Grade",s );
}

RString GradeToOldString( Grade g )
{
	// string is meant to be human readable
	switch( g )
	{
	case Grade_Tier01:	return "AAAA";
	case Grade_Tier02:	return "AAA";
	case Grade_Tier03:	return "AA";
	case Grade_Tier04:	return "A";
	case Grade_Tier05:	return "B";
	case Grade_Tier06:	return "C";
	case Grade_Tier07:	return "D";
	case Grade_Failed:	return "E";
	case Grade_NoData:	return "N";
	default:		return "N";
	}
};

Grade StringToGrade( const RString &sGrade )
{
	RString s = sGrade;
	s.MakeUpper();

	// new style
	int iTier;
	if( sscanf(sGrade.c_str(),"Tier%02d",&iTier) == 1 && iTier >= 0 && iTier < NUM_Grade)
		return (Grade)(iTier-1);
	else if( s == "FAILED" )
		return Grade_Failed;
	else if( s == "NODATA" )
		return Grade_NoData;

	LOG->Warn( "Invalid grade: %s", sGrade.c_str() );
	return Grade_NoData;
};
template<> void StringTo<Grade>( const RString& s, Grade &out ) { out = StringToGrade( s ); }

static void LuaGrade(lua_State* L)
{
	FOREACH_Grade( g )
	{
		RString s = GradeToString(g);
		LUA->SetGlobal( "Grade_"+s, g );
	}
	LUA->SetGlobal( "NUM_Grade", NUM_Grade );
}
REGISTER_WITH_LUA_FUNCTION( LuaGrade );

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
