#include "global.h"
#include "FontCharAliases.h"
#include "Font.h"
#include "RageUtil.h"
#include "RageLog.h"

#include <map>

/* Map from "&foo;" to a UTF-8 string. */
typedef map<CString, wchar_t, StdStringLessNoCase> aliasmap;
static aliasmap CharAliases;
static map<CString,CString> CharAliasRepl;

/* Editing this file in VC6 will be rather ugly, since it contains a lot of UTF-8.
 * Just don't change anything you can't read. :) */

/* If we move this to an INI, group these, so we can display them in a help
 * page off of the TextEntry screen reasonably.
 *
 * [Game Kanji]
 * kakumei1=革
 * [Game Symbols]
 * doublezeta=SPECIAL
 * [Hiragana]
 * [Katakana]
 * [Punctuation]
 *
 * I'm not sure how to handle internal-use character, like Zz.  Whenever we write
 * text to disk, we need to write placeholders (&doublezeta;) for them, and never
 * Unicode characters, since the codepoint is prone to change.  We can't currently
 * write &these; to SMs, due to format limitations.
 */

/* Here's a copy-and-paste for a basic Japanese font page:

　、。〃〄々〆〇〈〉《》「」『』
【】〒〓〔〕〖〗〘〙〚〛〜〝〞〟
〠
〰〱〲〳〴〵〶〷
　ぁあぃいぅうぇえぉおかがきぎく
ぐけげこごさざしじすずせぜそぞた
だちぢっつづてでとどなにぬねのは
ばぱひびぴふぶぷへべぺほぼぽまみ
むめもゃやゅゆょよらりるれろゎわ
ゐゑをんゔ　　　　　　゛゜ゝゞ　
　ァアィイゥウェエォオカガキギク
グケゲコゴサザシジスズセゼソゾタ
ダチヂッツヅテデトドナニヌネノハ
バパヒビピフブプヘベペホボポマミ
ムメモャヤュユョヨラリルレロヮワ
ヰヱヲンヴヵヶヷヸヹヺ・ーヽヾ　

And here's one for a kanji page:

 一ニ三四五六七八
九十
革命祭桜小坂鬼道
弐神様民謡明日青
衝動猫毛亜熱帯爆
弾谷新大見解魔法
扉夜空才

 */

static void InitCharAliases()
{
	if(!CharAliases.empty())
		return;

	CharAliases["default"]		= Font::DEFAULT_GLYPH;	/* ? */
	CharAliases["invalid"]		= INVALID_CHAR;			/* 0xFFFD */

	/* The comments here are UTF-8; they won't show up in VC6 (use your
	 * imagination). */

	/* Hiragana: */
	struct alias {
		const char *str;
		wchar_t chr;
	} aliases[] = {
		{ "ha", 	0x3042 }, /* あ */
		{ "hi",		0x3044 }, /* い */
		{ "hu",		0x3046 }, /* う */
		{ "he",		0x3048 }, /* え */
		{ "ho",		0x304a }, /* お */
		{ "hka",	0x304b }, /* か */
		{ "hki",	0x304d }, /* き */
		{ "hku",	0x304f }, /* く */
		{ "hke",	0x3051 }, /* け */
		{ "hko",	0x3053 }, /* こ */
		{ "hga",	0x304c }, /* が */
		{ "hgi",	0x304e }, /* ぎ */
		{ "hgu",	0x3050 }, /* ぐ */
		{ "hge",	0x3052 }, /* げ */
		{ "hgo",	0x3054 }, /* ご */
		{ "hza",	0x3056 }, /* ざ */
		{ "hzi",	0x3058 }, /* じ */
		{ "hzu",	0x305a }, /* ず */
		{ "hze",	0x305c }, /* ぜ */
		{ "hzo",	0x305e }, /* ぞ */
		{ "hta",	0x305f }, /* た */
		{ "hti",	0x3061 }, /* ち */
		{ "htu",	0x3064 }, /* つ */
		{ "hte",	0x3066 }, /* て */
		{ "hto",	0x3068 }, /* と */
		{ "hda",	0x3060 }, /* だ */
		{ "hdi",	0x3062 }, /* ぢ */
		{ "hdu",	0x3065 }, /* づ */
		{ "hde",	0x3067 }, /* で */
		{ "hdo",	0x3069 }, /* ど */
		{ "hna",	0x306a }, /* な */
		{ "hni",	0x306b }, /* に */
		{ "hnu",	0x306c }, /* ぬ */
		{ "hne",	0x306d }, /* ね */
		{ "hno",	0x306e }, /* の */
		{ "hha",	0x306f }, /* は */
		{ "hhi",	0x3072 }, /* ひ */
		{ "hhu",	0x3075 }, /* ふ */
		{ "hhe",	0x3078 }, /* へ */
		{ "hho",	0x307b }, /* ほ */
		{ "hba",	0x3070 }, /* ば */
		{ "hbi",	0x3073 }, /* び */
		{ "hbu",	0x3076 }, /* ぶ */
		{ "hbe",	0x3079 }, /* べ */
		{ "hbo",	0x307c }, /* ぼ */
		{ "hpa",	0x3071 }, /* ぱ */
		{ "hpi",	0x3074 }, /* ぴ */
		{ "hpu",	0x3077 }, /* ぷ */
		{ "hpe",	0x307a }, /* ぺ */
		{ "hpo",	0x307d }, /* ぽ */
		{ "hma",	0x307e }, /* ま */
		{ "hmi",	0x307f }, /* み */
		{ "hmu",	0x3080 }, /* む */
		{ "hme",	0x3081 }, /* め */
		{ "hmo",	0x3082 }, /* も */
		{ "hya",	0x3084 }, /* や */
		{ "hyu",	0x3086 }, /* ゆ */
		{ "hyo",	0x3088 }, /* よ */
		{ "hra",	0x3089 }, /* ら */
		{ "hri",	0x308a }, /* り */
		{ "hru",	0x308b }, /* る */
		{ "hre",	0x308c }, /* れ */
		{ "hro",	0x308d }, /* ろ */
		{ "hwa",	0x308f }, /* わ */
		{ "hwi",	0x3090 }, /* ゐ */
		{ "hwe",	0x3091 }, /* ゑ */
		{ "hwo",	0x3092 }, /* を */
		{ "hn",		0x3093 }, /* ん */
		{ "hvu",	0x3094 }, /* ゔ */
		{ "has",	0x3041 }, /* ぁ */
		{ "his",	0x3043 }, /* ぃ */
		{ "hus",	0x3045 }, /* ぅ */
		{ "hes",	0x3047 }, /* ぇ */
		{ "hos",	0x3049 }, /* ぉ */
		{ "hkas",	0x3095 }, /* ゕ */
		{ "hkes",	0x3096 }, /* ゖ */
		{ "hsa",	0x3055 }, /* さ */
		{ "hsi",	0x3057 }, /* し */
		{ "hsu",	0x3059 }, /* す */
		{ "hse",	0x305b }, /* せ */
		{ "hso",	0x305d }, /* そ */
		{ "hyas",	0x3083 }, /* ゃ */
		{ "hyus",	0x3085 }, /* ゅ */
		{ "hyos",	0x3087 }, /* ょ */
		{ "hwas",	0x308e }, /* ゎ */

		/* Katakana: */
		{ "hq",		0x3063 }, /* っ */
		{ "ka",		0x30a2 }, /* ア */
		{ "ki",		0x30a4 }, /* イ */
		{ "ku",		0x30a6 }, /* ウ */
		{ "ke",		0x30a8 }, /* エ */
		{ "ko",		0x30aa }, /* オ */
		{ "kka",	0x30ab }, /* カ */
		{ "kki",	0x30ad }, /* キ */
		{ "kku",	0x30af }, /* ク */
		{ "kke",	0x30b1 }, /* ケ */
		{ "kko",	0x30b3 }, /* コ */
		{ "kga",	0x30ac }, /* ガ */
		{ "kgi",	0x30ae }, /* ギ */
		{ "kgu",	0x30b0 }, /* グ */
		{ "kge",	0x30b2 }, /* ゲ */
		{ "kgo",	0x30b4 }, /* ゴ */
		{ "kza",	0x30b6 }, /* ザ */
		{ "kzi",	0x30b8 }, /* ジ */
		{ "kji",	0x30b8 }, /* ジ */ /* zi/ji alias */
		{ "kzu",	0x30ba }, /* ズ */
		{ "kze",	0x30bc }, /* ゼ */
		{ "kzo",	0x30be }, /* ゾ */
		{ "kta",	0x30bf }, /* タ */
		{ "kti",	0x30c1 }, /* チ */
		{ "ktu",	0x30c4 }, /* ツ */
		{ "kte",	0x30c6 }, /* テ */
		{ "kto",	0x30c8 }, /* ト */
		{ "kda",	0x30c0 }, /* ダ */
		{ "kdi",	0x30c2 }, /* ヂ */
		{ "kdu",	0x30c5 }, /* ヅ */
		{ "kde",	0x30c7 }, /* デ */
		{ "kdo",	0x30c9 }, /* ド */
		{ "kna",	0x30ca }, /* ナ */
		{ "kni",	0x30cb }, /* ニ */
		{ "knu",	0x30cc }, /* ヌ */
		{ "kne",	0x30cd }, /* ネ */
		{ "kno",	0x30ce }, /* ノ */
		{ "kha",	0x30cf }, /* ハ */
		{ "khi",	0x30d2 }, /* ヒ */
		{ "khu",	0x30d5 }, /* フ */
		{ "khe",	0x30d8 }, /* ヘ */
		{ "kho",	0x30db }, /* ホ */
		{ "kba",	0x30d0 }, /* バ */
		{ "kbi",	0x30d3 }, /* ビ */
		{ "kbu",	0x30d6 }, /* ブ */
		{ "kbe",	0x30d9 }, /* ベ */
		{ "kbo",	0x30dc }, /* ボ */
		{ "kpa",	0x30d1 }, /* パ */
		{ "kpi",	0x30d4 }, /* ピ */
		{ "kpu",	0x30d7 }, /* プ */
		{ "kpe",	0x30da }, /* ペ */
		{ "kpo",	0x30dd }, /* ポ */
		{ "kma",	0x30de }, /* マ */
		{ "kmi",	0x30df }, /* ミ */
		{ "kmu",	0x30e0 }, /* ム */
		{ "kme",	0x30e1 }, /* メ */
		{ "kmo",	0x30e2 }, /* モ */
		{ "kya",	0x30e4 }, /* ヤ */
		{ "kyu",	0x30e6 }, /* ユ */
		{ "kyo",	0x30e8 }, /* ヨ */
		{ "kra",	0x30e9 }, /* ラ */
		{ "kri",	0x30ea }, /* リ */
		{ "kru",	0x30eb }, /* ル */
		{ "kre",	0x30ec }, /* レ */
		{ "kro",	0x30ed }, /* ロ */
		{ "kwa",	0x30ef }, /* ワ */
		{ "kwi",	0x30f0 }, /* ヰ */
		{ "kwe",	0x30f1 }, /* ヱ */
		{ "kwo",	0x30f2 }, /* ヲ */
		{ "kn",		0x30f3 }, /* ン */
		{ "kvu",	0x30f4 }, /* ヴ */
		{ "kas",	0x30a1 }, /* ァ */
		{ "kis",	0x30a3 }, /* ィ */
		{ "kus",	0x30a5 }, /* ゥ */
		{ "kes",	0x30a7 }, /* ェ */
		{ "kos",	0x30a9 }, /* ォ */
		{ "kkas",	0x30f5 }, /* ヵ */
		{ "kkes",	0x30f6 }, /* ヶ */
		{ "ksa",	0x30b5 }, /* サ */
		{ "ksi",	0x30b7 }, /* シ */
		{ "ksu",	0x30b9 }, /* ス */
		{ "kse",	0x30bb }, /* セ */
		{ "kso",	0x30bd }, /* ソ */
		{ "kyas",	0x30e3 }, /* ャ */
		{ "kyus",	0x30e5 }, /* ュ */
		{ "kyos",	0x30e7 }, /* ョ */
		{ "kwas",	0x30ee }, /* ヮ */
		{ "kq",		0x30c3 }, /* ッ */

		{ "kdot",	0x30FB }, /* ・ */
		{ "kdash",	0x30FC }, /* ー */

		{ "nbsp",	0x00a0 }, /* Non-breaking space */

		/* Symbols: */
		{ "delta",	0x0394 }, /* Δ */
		{ "sigma",	0x03a3 }, /* Σ */
		{ "omega",	0x03a9 }, /* Ω */
		{ "angle",	0x2220 }, /* ∠ */
		{ "whiteheart",	0x2661 }, /* ♡ */
		{ "blackstar",	0x2605 }, /* ★ */
		{ "whitestar",	0x2606 }, /* ☆ */
		{ "flipped-a",	0x2200 }, /* ∀ */
		{ "squared",	0x00b2 }, /* ² */
		{ "cubed",	0x00b3 }, /* ³ */
		{ "oq",		0x201c }, /* “ */
		{ "cq",		0x201d }, /* ” */
		{ "leftarrow",	0x2190 }, /* ← */
		{ "uparrow",	0x2191 }, /* ↑ */
		{ "rightarrow",	0x2192 }, /* → */
		{ "downarrow",	0x2193 }, /* ↓ */
		{ "doublezeta",	0xE009 },
		{ "planet",		0xE00A },
		{ "4thnote",	0x2669 }, /* ♩ */
		{ "8thnote",	0x266A }, /* ♪ */
		{ "b8thnote",	0x266B }, /* ♫ */
		{ "b16thnote",	0x266C }, /* ♬ */
		{ "flat",		0x266D }, /* ♭ */
		{ "natural",	0x266E }, /* ♮ */
		{ "sharp",		0x266F }, /* ♯ */

		/* These are internal-use glyphs; they don't have real Unicode codepoints. */
		{ "up",			0xE000 },
		{ "down",		0xE001 },
		{ "left",		0xE002 },
		{ "right",		0xE003 },
		{ "menuup",		0xE004 },
		{ "menudown",	0xE005 },
		{ "menuleft",	0xE006 },
		{ "menuright",	0xE007 },
		{ "start",		0xE008 },
		{ "back",		0xE009 },
		{ "ok",			0xE00A },
		{ "nextrow",	0xE00B },
		{ "select",		0xE00C },
		/* PlayStation-style controller */
		{ "auxx",		0xE010 },
		{ "auxtriangle",0xE011 },
		{ "auxsquare",	0xE012 },
		{ "auxcircle",	0xE013 },
		{ "auxl1",		0xE014 },
		{ "auxl2",		0xE015 },
		{ "auxl3",		0xE016 },
		{ "auxr1",		0xE017 },
		{ "auxr2",		0xE018 },
		{ "auxr3",		0xE019 },
		{ "auxselect",	0xE01A },
		{ "auxstart",	0xE01B },

		{ NULL, 	0 }
	};

	for( unsigned n = 0; aliases[n].str; ++n )
		CharAliases[aliases[n].str] = aliases[n].chr;

	for(aliasmap::const_iterator i = CharAliases.begin(); i != CharAliases.end(); ++i)
	{
		CString from = i->first;
		CString to = WcharToUTF8(i->second);
		from.MakeLower();
		CharAliasRepl[from] = to;
	}
}

/* Replace all &markers; and &#NNNN;s with UTF-8. */
void FontCharAliases::ReplaceMarkers( CString &sText )
{
	InitCharAliases();
	ReplaceEntityText( sText, CharAliasRepl );
	Replace_Unicode_Markers(sText);
}

/* Replace all &markers; and &#NNNN;s with UTF-8. */
bool FontCharAliases::GetChar( CString &codepoint, wchar_t &ch )
{
	InitCharAliases();
	aliasmap::const_iterator i = CharAliases.find(codepoint);
	if(i == CharAliases.end())
		return false;
	
	ch = i->second;
	return true;
}

/*
 * (c) 2003 Glenn Maynard
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
