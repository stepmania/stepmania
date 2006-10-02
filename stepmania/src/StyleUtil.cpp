#include "global.h"
#include "StyleUtil.h"
#include "GameManager.h"
#include "XmlFile.h"
#include "Game.h"
#include "Style.h"


void StyleID::FromStyle( const Style *p )
{
	if( p )
	{
		sGame = p->m_pGame->m_szName;
		sStyle = p->m_szName;
	}
	else
	{
		sGame = "";
		sStyle = "";
	}
}

const Style *StyleID::ToStyle() const
{
	const Game* pGame = GameManager::StringToGameType( sGame );
	if( pGame == NULL )
		return NULL;

	return GAMEMAN->GameAndStringToStyle( pGame, sStyle );
}

XNode* StyleID::CreateNode() const
{
	XNode* pNode = new XNode( "Style" );

	pNode->AppendAttr( "Game", sGame );
	pNode->AppendAttr( "Style", sStyle );

	return pNode;
}

void StyleID::LoadFromNode( const XNode* pNode ) 
{
	Unset();
	ASSERT( pNode->GetName() == "Style" );

	sGame = "";
	pNode->GetAttrValue("Game", sGame);

	sStyle = "";
	pNode->GetAttrValue("Style", sStyle);
}

bool StyleID::IsValid() const
{
	return !sGame.empty() && !sStyle.empty();
}

bool StyleID::operator<( const StyleID &rhs ) const
{
#define COMP(a) if(a<rhs.a) return true; if(a>rhs.a) return false;
	COMP(sGame);
	COMP(sStyle);
#undef COMP
	return false;
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
