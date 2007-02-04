/* XmlFileUtil - A little graphic to the left of the song's text banner in the MusicWheel. */

#ifndef XML_FILE_UTIL_H
#define XML_FILE_UTIL_H

class RageFileBasic;
class XNode;
struct lua_State;

namespace XmlFileUtil
{
	bool LoadFromFileShowErrors( XNode &xml, const RString &sFile );
	bool LoadFromFileShowErrors( XNode &xml, RageFileBasic &f );

	// Load/Save XML
	void Load( XNode *pNode, const RString &sXml, RString &sErrorOut );
	bool GetXML( const XNode *pNode, RageFileBasic &f, bool bWriteTabs = true );
	RString GetXML( const XNode *pNode );
	bool SaveToFile( const XNode *pNode, const RString &sFile, const RString &sStylesheet = "", bool bWriteTabs = true );
	bool SaveToFile( const XNode *pNode, RageFileBasic &f, const RString &sStylesheet = "", bool bWriteTabs = true );

	void CompileXNodeTree( XNode *pNode, const RString &sFile );
	XNode *XNodeFromTable( lua_State *L );

	void MergeIniUnder( XNode *pFrom, XNode *pTo );
}

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
