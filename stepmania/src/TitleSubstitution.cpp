#include "global.h"
#include "TitleSubstitution.h"

#include "RageUtil.h"
#include "RageLog.h"
#include "FontCharAliases.h"
#include "RageFile.h"
#include "Foreach.h"
#include "XmlFile.h"

static const CString TRANSLATIONS_PATH = "Data/Translations.xml";
static const CString ERASE_MARKER = "-erase-";

struct TitleTrans
{
	Regex TitleFrom, SubFrom, ArtistFrom;
	TitleFields Replacement;

	/* If this is true, no translit fields will be generated automatically. */
	bool translit;
	TitleTrans() { translit = true; }

	TitleTrans( const TitleFields &tf, bool translit_):
		Replacement(tf), translit(translit_) { }

	bool Matches( const TitleFields &tf, TitleFields &to );

	void LoadFromNode( const XNode* pNode );
};

vector<TitleTrans> ttab;

bool TitleTrans::Matches( const TitleFields &from, TitleFields &to )
{
	if( !TitleFrom.Replace(Replacement.Title, from.Title, to.Title) )
		return false; /* no match */
	if( !SubFrom.Replace(Replacement.Subtitle, from.Subtitle, to.Subtitle) )
		return false; /* no match */
	if( !ArtistFrom.Replace(Replacement.Artist, from.Artist, to.Artist) )
		return false; /* no match */

	return true;
}

void TitleTrans::LoadFromNode( const XNode* pNode )
{
	ASSERT( pNode->m_sName == "Translation" );

	FOREACH_CONST_Attr( pNode, attr )
	{
		/* Surround each regex with ^(...)$, to force all comparisons to default
		 * to being a full-line match.  (Add ".*" manually if this isn't wanted.) */
		const CString &sKeyName = attr->first;
		const CString &sValue = attr->second;
		if(		 sKeyName == "DontTransliterate" )	translit = false;
		else if( sKeyName == "TitleFrom" )			TitleFrom					= "^(" + sValue + ")$";
		else if( sKeyName == "ArtistFrom" )			ArtistFrom					= "^(" + sValue + ")$";
		else if( sKeyName == "SubtitleFrom")		SubFrom						= "^(" + sValue + ")$";
		else if( sKeyName == "TitleTo")				Replacement.Title			= sValue;
		else if( sKeyName == "ArtistTo")			Replacement.Artist			= sValue;
		else if( sKeyName == "SubtitleTo")			Replacement.Subtitle		= sValue;
		else if( sKeyName == "TitleTransTo")		Replacement.TitleTranslit	= sValue;
		else if( sKeyName == "ArtistTransTo")		Replacement.ArtistTranslit	= sValue;
		else if( sKeyName == "SubtitleTransTo")		Replacement.SubtitleTranslit= sValue;
		else
			LOG->Warn( "Unknown TitleSubst tag: \"%s\"", sKeyName.c_str() );
	}
}

void TitleSubst::AddTrans(const TitleTrans &tr)
{
	ASSERT( tr.TitleFrom.IsSet() || tr.SubFrom.IsSet() || tr.ArtistFrom.IsSet() );
	ttab.push_back(new TitleTrans(tr));
}

void TitleSubst::Subst( TitleFields &tf )
{
	FOREACH_CONST( TitleTrans*, ttab, iter )
	{
		TitleTrans* tt = *iter;

		TitleFields to;
		if(!tt->Matches(tf,to))
			continue;

		/* The song matches.  Replace whichever strings aren't empty. */
		if( !tt->Replacement.Title.empty() && tf.Title != tt->Replacement.Title )
		{
			if( tt->translit )
				tf.TitleTranslit = tf.Title;
			tf.Title = (tt->Replacement.Title != ERASE_MARKER)? to.Title : CString();
			FontCharAliases::ReplaceMarkers( tf.Title );
		}
		if( !tt->Replacement.Subtitle.empty() && tf.Subtitle != tt->Replacement.Subtitle )
		{
			if( tt->translit )
				tf.SubtitleTranslit = tf.Subtitle;
			tf.Subtitle = (tt->Replacement.Subtitle != ERASE_MARKER)? to.Subtitle : CString();
			FontCharAliases::ReplaceMarkers( tf.Subtitle );
		}
		if( !tt->Replacement.Artist.empty() && tf.Artist != tt->Replacement.Artist )
		{
			if( tt->translit )
				tf.ArtistTranslit = tf.Artist;
			tf.Artist = (tt->Replacement.Artist != ERASE_MARKER)? to.Artist : CString();
			FontCharAliases::ReplaceMarkers( tf.Artist );
		}

		/* These are used when applying kanji to a field that doesn't have the
		 * correct data.  Should be used sparingly. */
		if( !tt->Replacement.TitleTranslit.empty() )
		{
			tf.TitleTranslit = (tt->Replacement.TitleTranslit != ERASE_MARKER)? tt->Replacement.TitleTranslit : CString();
			FontCharAliases::ReplaceMarkers( tf.TitleTranslit );
		}
		if( !tt->Replacement.SubtitleTranslit.empty() )
		{
			tf.SubtitleTranslit = (tt->Replacement.SubtitleTranslit != ERASE_MARKER)? tt->Replacement.SubtitleTranslit : CString();
			FontCharAliases::ReplaceMarkers( tf.SubtitleTranslit );
		}
		if( !tt->Replacement.ArtistTranslit.empty() )
		{
			tf.ArtistTranslit = (tt->Replacement.ArtistTranslit != ERASE_MARKER)? tt->Replacement.ArtistTranslit : CString();
			FontCharAliases::ReplaceMarkers( tf.ArtistTranslit );
		}

		break;	// Matched once.  Done.
	}
}


TitleSubst::TitleSubst(const CString &section)
{
	Load( TRANSLATIONS_PATH, section);
}

void TitleSubst::Load(const CString &filename, const CString &section)
{
	XNode xml;
	if( !xml.LoadFromFile(filename) )
	{
		// LoadFromFile will show its own error
		//LOG->Trace("Error opening %s: %s", filename.c_str(), f.GetError().c_str() );
		return;
	}

	XNode *pGroup = xml.GetChild( section );
	if( pGroup == NULL )
		return;
	FOREACH_Child( pGroup, child )
	{
		if( child->m_sName != "Translation" )
			continue;

		TitleTrans tr;
		tr.LoadFromNode( child );
		AddTrans(tr);
	}
}

TitleSubst::~TitleSubst()
{
	for(unsigned i = 0; i < ttab.size(); ++i)
		delete ttab[i];
}

/*
 * (c) 2003-2004 Glenn Maynard
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
