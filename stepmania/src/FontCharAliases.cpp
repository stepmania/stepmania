#include "stdafx.h"
#include "FontCharAliases.h"
#include "Font.h"
#include "RageUtil.h"
#include "RageLog.h"

#include <map>

/* Map from "&foo;" to a UTF-8 string. */
typedef map<CString, longchar, StdStringLessNoCase> aliasmap;
static aliasmap CharAliases;
static map<CString,CString> CharAliasRepl;

/* If we move this to an INI, group these, so we can display them in a help
 * page off of the TextEntry screen reasonably.  Allow putting the actual
 * character on the same line (as below).  Perhaps warn if they don't match.
 *
 * [Game Kanji]
 * kakumei1=0x9769 (革)
 * [Game Symbols]
 * [Hiragana]
 * [Katakana]
 * [Punctuation]
 *
 * This will need some mechanism for assigning internal numbers.
 */


static void InitCharAliases()
{
	if(!CharAliases.empty())
		return;

	CharAliases["default"]		= Font::DEFAULT_GLYPH;	/* ? */
	CharAliases["invalid"]		= INVALID_CHAR;			/* 0xFFFF */

	/* The comments here are UTF-8; they won't show up in VC6 (use your
	 * imagination).
	 *
	 * Kanji:
	 *
	 * I've chosen semi-arbitrary names for these kanji.  Almost all have other
	 * readings.  Some of the names are just words that use it (大きい -> ookii).
	 * A character can have more than one alias, but keep aliases separate so
	 * a list of all named kanji can be made without having to weed dupes. Avoid
	 * really short names; eg. let's save "me" for め if we make hiragana aliases. */
	CharAliases["kakumei1"]		= 0x9769; /* 革 */
	CharAliases["kakumei2"]		= 0x547D; /* 命 */
	CharAliases["matsuri"]		= 0x796D; /* 祭 */
	CharAliases["sakura"]		= 0x685C; /* 桜 */
	CharAliases["kosaka1"]		= 0x5C0F; /* 小 */
	CharAliases["kosaka2"]		= 0x5742; /* 坂 */
	CharAliases["oni"]			= 0x9B3C; /* 鬼 */
	CharAliases["michi"]		= 0x9053; /* 道 */
	CharAliases["futatsu"]		= 0x5F10; /* 弐 */
	CharAliases["kami"]			= 0x795E; /* 神 */
	CharAliases["sama"]			= 0x69D8; /* 様 */
	CharAliases["minyou1"]		= 0x6C11; /* 民 */
	CharAliases["minyou2"]		= 0x8B21; /* 謡 */
	CharAliases["aka"]			= 0x660E; /* 明 */
	CharAliases["nichi"]		= 0x65E5; /* 日 */
	CharAliases["aoi"]			= 0x9752; /* 青 */
	CharAliases["shoudou1"]		= 0x885D; /* 衝 */
	CharAliases["shoudou2"]		= 0x52D5; /* 動 */
	CharAliases["neko"]			= 0x732B; /* 猫 */
	CharAliases["hair"]			= 0x6BDB; /* 毛 */
	CharAliases["anettai1"]		= 0x5E2F; /* 亜 */
	CharAliases["anettai2"]		= 0x71B1; /* 熱 */
	CharAliases["anettai3"]		= 0x4E9C; /* 帯 */
	CharAliases["bakudan1"]		= 0x7206; /* 爆 */
	CharAliases["bakudan2"]		= 0x5F3E; /* 弾 */
	CharAliases["shintani1"]	= 0x65B0; /* 新 */
	CharAliases["shintani2"]	= 0x8C37; /* 谷 */
	CharAliases["ookii"]		= 0x5927; /* 大 */
	CharAliases["kenkai1"]		= 0x898B; /* 見 */
	CharAliases["kenkai2"]		= 0x89E3; /* 解 */
	CharAliases["mahou1"]		= 0x9B54; /* 魔 */
	CharAliases["mahou2"]		= 0x6CD5; /* 法 */
	CharAliases["tobira"]		= 0x6249; /* 扉 */
	CharAliases["yozora1"]		= 0x7A7A; /* 夜 */
	CharAliases["yozora2"]		= 0x6249; /* 空 */
	
	CharAliases["num-ichi"]		= 0x4E00; /* 一 */
	CharAliases["num-ni"]		= 0x4E01; /* 二 */
	CharAliases["num-san"]		= 0x4E02; /* 三 */
	CharAliases["num-yon"]		= 0x4E03; /* 四 */
	CharAliases["num-go"]		= 0x4E04; /* 五 */
	CharAliases["num-roku"]		= 0x4E05; /* 六 */
	CharAliases["num-nana"]		= 0x4E06; /* 七 */
	CharAliases["num-hachi"]	= 0x4E07; /* 八 */
	CharAliases["num-kyuu"]		= 0x4E08; /* 九 */
	CharAliases["num-juu"]		= 0x4E09; /* 十 */

	/* Hiragana: */
	CharAliases["ha"]	= 0x3042; /* あ */
	CharAliases["hi"]	= 0x3044; /* い */
	CharAliases["hu"]	= 0x3046; /* う */
	CharAliases["he"]	= 0x3048; /* え */
	CharAliases["ho"]	= 0x304a; /* お */
	CharAliases["hka"]	= 0x304b; /* か */
	CharAliases["hki"]	= 0x304d; /* き */
	CharAliases["hku"]	= 0x304f; /* く */
	CharAliases["hke"]	= 0x3051; /* け */
	CharAliases["hko"]	= 0x3053; /* こ */
	CharAliases["hga"]	= 0x304c; /* が */
	CharAliases["hgi"]	= 0x304e; /* ぎ */
	CharAliases["hgu"]	= 0x3050; /* ぐ */
	CharAliases["hge"]	= 0x3052; /* げ */
	CharAliases["hgo"]	= 0x3054; /* ご */
	CharAliases["hza"]	= 0x3056; /* ざ */
	CharAliases["hzi"]	= 0x3058; /* じ */
	CharAliases["hzu"]	= 0x305a; /* ず */
	CharAliases["hze"]	= 0x305c; /* ぜ */
	CharAliases["hzo"]	= 0x305e; /* ぞ */
	CharAliases["hta"]	= 0x305f; /* た */
	CharAliases["hti"]	= 0x3061; /* ち */
	CharAliases["htu"]	= 0x3064; /* つ */
	CharAliases["hte"]	= 0x3066; /* て */
	CharAliases["hto"]	= 0x3068; /* と */
	CharAliases["hda"]	= 0x3060; /* だ */
	CharAliases["hdi"]	= 0x3062; /* ぢ */
	CharAliases["hdu"]	= 0x3065; /* づ */
	CharAliases["hde"]	= 0x3067; /* で */
	CharAliases["hdo"]	= 0x3069; /* ど */
	CharAliases["hna"]	= 0x306a; /* な */
	CharAliases["hni"]	= 0x306b; /* に */
	CharAliases["hnu"]	= 0x306c; /* ぬ */
	CharAliases["hne"]	= 0x306d; /* ね */
	CharAliases["hno"]	= 0x306e; /* の */
	CharAliases["hha"]	= 0x306f; /* は */
	CharAliases["hhi"]	= 0x3072; /* ひ */
	CharAliases["hhu"]	= 0x3075; /* ふ */
	CharAliases["hhe"]	= 0x3078; /* へ */
	CharAliases["hho"]	= 0x307b; /* ほ */
	CharAliases["hba"]	= 0x3070; /* ば */
	CharAliases["hbi"]	= 0x3073; /* び */
	CharAliases["hbu"]	= 0x3076; /* ぶ */
	CharAliases["hbe"]	= 0x3079; /* べ */
	CharAliases["hbo"]	= 0x307c; /* ぼ */
	CharAliases["hpa"]	= 0x3071; /* ぱ */
	CharAliases["hpi"]	= 0x3074; /* ぴ */
	CharAliases["hpu"]	= 0x3077; /* ぷ */
	CharAliases["hpe"]	= 0x307a; /* ぺ */
	CharAliases["hpo"]	= 0x307d; /* ぽ */
	CharAliases["hma"]	= 0x307e; /* ま */
	CharAliases["hmi"]	= 0x307f; /* み */
	CharAliases["hmu"]	= 0x3080; /* む */
	CharAliases["hme"]	= 0x3081; /* め */
	CharAliases["hmo"]	= 0x3082; /* も */
	CharAliases["hya"]	= 0x3084; /* や */
	CharAliases["hyu"]	= 0x3086; /* ゆ */
	CharAliases["hyo"]	= 0x3088; /* よ */
	CharAliases["hra"]	= 0x3089; /* ら */
	CharAliases["hri"]	= 0x308a; /* り */
	CharAliases["hru"]	= 0x308b; /* る */
	CharAliases["hre"]	= 0x308c; /* れ */
	CharAliases["hro"]	= 0x308d; /* ろ */
	CharAliases["hwa"]	= 0x308f; /* わ */
	CharAliases["hwi"]	= 0x3090; /* ゐ */
	CharAliases["hwe"]	= 0x3091; /* ゑ */
	CharAliases["hwo"]	= 0x3092; /* を */
	CharAliases["hn"]	= 0x3093; /* ん */
	CharAliases["hvu"]	= 0x3094; /* ゔ */
	CharAliases["has"]	= 0x3041; /* ぁ */
	CharAliases["his"]	= 0x3043; /* ぃ */
	CharAliases["hus"]	= 0x3045; /* ぅ */
	CharAliases["hes"]	= 0x3047; /* ぇ */
	CharAliases["hos"]	= 0x3049; /* ぉ */
	CharAliases["hkas"]	= 0x3095; /* ゕ */
	CharAliases["hkes"]	= 0x3096; /* ゖ */
	CharAliases["hsa"]	= 0x3055; /* さ */
	CharAliases["hsi"]	= 0x3057; /* し */
	CharAliases["hsu"]	= 0x3059; /* す */
	CharAliases["hse"]	= 0x305b; /* せ */
	CharAliases["hso"]	= 0x305d; /* そ */
	CharAliases["hyas"]	= 0x3083; /* ゃ */
	CharAliases["hyus"]	= 0x3085; /* ゅ */
	CharAliases["hyos"]	= 0x3087; /* ょ */
	CharAliases["hwas"]	= 0x308e; /* ゎ */

	/* Katakana: */
	CharAliases["hq"]	= 0x3063; /* っ */
	CharAliases["ka"]	= 0x30a2; /* ア */
	CharAliases["ki"]	= 0x30a4; /* イ */
	CharAliases["ku"]	= 0x30a6; /* ウ */
	CharAliases["ke"]	= 0x30a8; /* エ */
	CharAliases["ko"]	= 0x30aa; /* オ */
	CharAliases["kka"]	= 0x30ab; /* カ */
	CharAliases["kki"]	= 0x30ad; /* キ */
	CharAliases["kku"]	= 0x30af; /* ク */
	CharAliases["kke"]	= 0x30b1; /* ケ */
	CharAliases["kko"]	= 0x30b3; /* コ */
	CharAliases["kga"]	= 0x30ac; /* ガ */
	CharAliases["kgi"]	= 0x30ae; /* ギ */
	CharAliases["kgu"]	= 0x30b0; /* グ */
	CharAliases["kge"]	= 0x30b2; /* ゲ */
	CharAliases["kgo"]	= 0x30b4; /* ゴ */
	CharAliases["kza"]	= 0x30b6; /* ザ */
	CharAliases["kzi"]	= 0x30b8; /* ジ */
	CharAliases["kji"]	= 0x30b8; /* ジ */ /* zi/ji alias */
	CharAliases["kzu"]	= 0x30ba; /* ズ */
	CharAliases["kze"]	= 0x30bc; /* ゼ */
	CharAliases["kzo"]	= 0x30be; /* ゾ */
	CharAliases["kta"]	= 0x30bf; /* タ */
	CharAliases["kti"]	= 0x30c1; /* チ */
	CharAliases["ktu"]	= 0x30c4; /* ツ */
	CharAliases["kte"]	= 0x30c6; /* テ */
	CharAliases["kto"]	= 0x30c8; /* ト */
	CharAliases["kda"]	= 0x30c0; /* ダ */
	CharAliases["kdi"]	= 0x30c2; /* ヂ */
	CharAliases["kdu"]	= 0x30c5; /* ヅ */
	CharAliases["kde"]	= 0x30c7; /* デ */
	CharAliases["kdo"]	= 0x30c9; /* ド */
	CharAliases["kna"]	= 0x30ca; /* ナ */
	CharAliases["kni"]	= 0x30cb; /* ニ */
	CharAliases["knu"]	= 0x30cc; /* ヌ */
	CharAliases["kne"]	= 0x30cd; /* ネ */
	CharAliases["kno"]	= 0x30ce; /* ノ */
	CharAliases["kha"]	= 0x30cf; /* ハ */
	CharAliases["khi"]	= 0x30d2; /* ヒ */
	CharAliases["khu"]	= 0x30d5; /* フ */
	CharAliases["khe"]	= 0x30d8; /* ヘ */
	CharAliases["kho"]	= 0x30db; /* ホ */
	CharAliases["kba"]	= 0x30d0; /* バ */
	CharAliases["kbi"]	= 0x30d3; /* ビ */
	CharAliases["kbu"]	= 0x30d6; /* ブ */
	CharAliases["kbe"]	= 0x30d9; /* ベ */
	CharAliases["kbo"]	= 0x30dc; /* ボ */
	CharAliases["kpa"]	= 0x30d1; /* パ */
	CharAliases["kpi"]	= 0x30d4; /* ピ */
	CharAliases["kpu"]	= 0x30d7; /* プ */
	CharAliases["kpe"]	= 0x30da; /* ペ */
	CharAliases["kpo"]	= 0x30dd; /* ポ */
	CharAliases["kma"]	= 0x30de; /* マ */
	CharAliases["kmi"]	= 0x30df; /* ミ */
	CharAliases["kmu"]	= 0x30e0; /* ム */
	CharAliases["kme"]	= 0x30e1; /* メ */
	CharAliases["kmo"]	= 0x30e2; /* モ */
	CharAliases["kya"]	= 0x30e4; /* ヤ */
	CharAliases["kyu"]	= 0x30e6; /* ユ */
	CharAliases["kyo"]	= 0x30e8; /* ヨ */
	CharAliases["kra"]	= 0x30e9; /* ラ */
	CharAliases["kri"]	= 0x30ea; /* リ */
	CharAliases["kru"]	= 0x30eb; /* ル */
	CharAliases["kre"]	= 0x30ec; /* レ */
	CharAliases["kro"]	= 0x30ed; /* ロ */
	CharAliases["kwa"]	= 0x30ef; /* ワ */
	CharAliases["kwi"]	= 0x30f0; /* ヰ */
	CharAliases["kwe"]	= 0x30f1; /* ヱ */
	CharAliases["kwo"]	= 0x30f2; /* ヲ */
	CharAliases["kn"]	= 0x30f3; /* ン */
	CharAliases["kvu"]	= 0x30f4; /* ヴ */
	CharAliases["kas"]	= 0x30a1; /* ァ */
	CharAliases["kis"]	= 0x30a3; /* ィ */
	CharAliases["kus"]	= 0x30a5; /* ゥ */
	CharAliases["kes"]	= 0x30a7; /* ェ */
	CharAliases["kos"]	= 0x30a9; /* ォ */
	CharAliases["kkas"]	= 0x30f5; /* ヵ */
	CharAliases["kkes"]	= 0x30f6; /* ヶ */
	CharAliases["ksa"]	= 0x30b5; /* サ */
	CharAliases["ksi"]	= 0x30b7; /* シ */
	CharAliases["ksu"]	= 0x30b9; /* ス */
	CharAliases["kse"]	= 0x30bb; /* セ */
	CharAliases["kso"]	= 0x30bd; /* ソ */
	CharAliases["kyas"]	= 0x30e3; /* ャ */
	CharAliases["kyus"]	= 0x30e5; /* ュ */
	CharAliases["kyos"]	= 0x30e7; /* ョ */
	CharAliases["kwas"]	= 0x30ee; /* ヮ */
	CharAliases["kq"]	= 0x30c3; /* ッ */

	CharAliases["kdot"]	= 0x30FB; /* ・ */
	CharAliases["kdash"]= 0x30FC; /* ー */
	
	/* Symbols: */
	CharAliases["omega"]		= 0x03a9; /* Ω */
	CharAliases["whiteheart"]	= 0x2661; /* ♡ */
	CharAliases["blackstar"]	= 0x2605; /* ★ */
	CharAliases["whitestar"]	= 0x2606; /* ☆ */
	CharAliases["flipped-a"]	= 0x2200; /* ∀ */
	CharAliases["squared"]		= 0x00b2; /* ² */
	CharAliases["cubed"]		= 0x00b3; /* ³ */
	CharAliases["oq"]			= 0x201c; /* “ */
	CharAliases["cq"]			= 0x201d; /* ” */
	CharAliases["leftarrow"]	= 0x2190; /* ← */
	CharAliases["uparrow"]		= 0x2191; /* ↑ */
	CharAliases["rightarrow"]	= 0x2192; /* → */
	CharAliases["downarrow"]	= 0x2193; /* ↓ */

	/* These are internal-use glyphs; they don't have real Unicode codepoints. */
	CharAliases["up"]			= 0xE000;
	CharAliases["down"]			= 0xE001;
	CharAliases["left"]			= 0xE002;
	CharAliases["right"]		= 0xE003;
	CharAliases["menuup"]		= 0xE004;
	CharAliases["menudown"]		= 0xE005;
	CharAliases["menuleft"]		= 0xE006;
	CharAliases["menuright"]	= 0xE007;
	CharAliases["start"]		= 0xE008;
	CharAliases["zz"]			= 0xE009;

	for(aliasmap::const_iterator i = CharAliases.begin(); i != CharAliases.end(); ++i)
	{
		CString from = ssprintf("&%s;", i->first.GetString());
		CString to = LcharToUTF8(i->second);
		from.MakeUpper();
		CharAliasRepl[from] = to;
		LOG->Trace("from '%s' to '%s'", from.GetString(), to.GetString());
	}
}

/* Replace all &markers; and &#NNNN;s with UTF-8.  I'm not really
 * sure where to put this; this is used elsewhere, too. */
void FontCharAliases::ReplaceMarkers( CString &sText )
{
	InitCharAliases();
	ReplaceText(sText, CharAliasRepl);
	Replace_Unicode_Markers(sText);
}

/* Replace all &markers; and &#NNNN;s with UTF-8. */
longchar FontCharAliases::GetChar( CString &codepoint )
{
	InitCharAliases();
	aliasmap::const_iterator i = CharAliases.find(codepoint);
	if(i == CharAliases.end())
		return longchar(-1);
	
	return i->second;
}

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
