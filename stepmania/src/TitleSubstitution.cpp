#include "stdafx.h"
#include "TitleSubstitution.h"

#include "RageUtil.h"
#include "FontCharAliases.h"

struct TitleTrans
{
	Regex TitleFrom, SubFrom, ArtistFrom;
	CString TitleTo, SubTo, ArtistTo;			/* plain text */

	TitleTrans(CString tf, CString sf, CString af, CString tt, CString st, CString at):
		TitleFrom(tf), SubFrom(sf), ArtistFrom(af),
			TitleTo(tt), SubTo(st), ArtistTo(at) { }

	bool Matches(CString title, CString sub, CString artist);
};

vector<TitleTrans> ttab;

bool TitleTrans::Matches(CString title, CString sub, CString artist)
{
	if(!TitleFrom.Compare(title)) return false; /* no match */
	if(!SubFrom.Compare(sub)) return false; /* no match */
	if(!ArtistFrom.Compare(artist)) return false; /* no match */

	return true;
}

void TitleSubst::AddTrans(const CString &tf, const CString &sf, const CString &af,
		      const CString &tt, const CString &st, const CString &at)
{
	ttab.push_back(new TitleTrans(tf, sf, af, tt, st, at));
}

void TitleSubst::Subst(CString &title, CString &subtitle, CString &artist,
					   CString &ttitle, CString &tsubtitle, CString &tartist)
{
	for(unsigned i = 0; i < ttab.size(); ++i)
	{
		if(!ttab[i]->Matches(title, subtitle, artist))
			continue;

		/* The song matches.  Replace whichever strings aren't empty: */
		if(!ttab[i]->TitleTo.empty())
		{
			ttitle = title;
			title = ttab[i]->TitleTo;
			FontCharAliases::ReplaceMarkers( title );
		}
		if(!ttab[i]->SubTo.empty())
		{
			tsubtitle = subtitle;
			subtitle = ttab[i]->SubTo;
			FontCharAliases::ReplaceMarkers( subtitle );
		}
		if(!ttab[i]->ArtistTo.empty())
		{
			tartist = artist;
			artist = ttab[i]->ArtistTo;
			FontCharAliases::ReplaceMarkers( artist );
		}
	}
}


TitleSubst::TitleSubst()
{
	/* Ambiguous, so check artist.  Do this early; Riyu will be replaced later: */
	AddTrans("^Candy$", "", "^Luv.*", "CANDY &whitestar;", "", "");
	AddTrans("^Candy$", "", ".*Riyu.*", "CANDY &whiteheart;", "", "");

	/* Make sure this appears after the above "Riyu" match, so it doesn't
		* break it.  I've seen both "Kosaku" and "Kosaka"; I think Kosaka is
		* correct, but handle both. */
	AddTrans("", "", "Riyu Kosak[au]", "", "", "&kosaka1;&kosaka2;&hri;&hyu;");
	AddTrans("", "", "Kosak[au] Riyu", "", "", "&kosaka1;&kosaka2;&hri;&hyu;");

	/* Matsuri Japan is often just "Japan".  There may be other songs by
		* this name, too, so match the artist, too. */
	AddTrans("^Japan$", "", "(Re-Venge)|(RevenG)", "&matsuri; JAPAN", "", "");

	/* Fix up hacked titles: */
	AddTrans("^Max 300$", "", "%", "", "", "&omega;");
	AddTrans("^Bre=kdown$", "", "", "Bre&flipped-a;kdown", "", "");
	AddTrans("^Candy #$", "", "", "Candy &whitestar;", "", "");
	AddTrans("^Candy \\$$", "", "", "Candy &whiteheart;", "", "");
	AddTrans("^} JAPAN$", "", "", "&matsuri; JAPAN", "", "");
	AddTrans("^\\+\\{$", "", "", "&kakumei1;&kakumei2;", "", "");
	AddTrans("^Sweet Sweet \\$ Magic$", "", "", "Sweet Sweet &whiteheart; Magic", "", "");

	/* Special stuff is done.  Titles: */
	AddTrans("^Matsuri Japan$", "", "", "&matsuri; JAPAN", "", "");
	AddTrans("^Kakumei$", "", "", "&kakumei1;&kakumei2;", "", "");
	AddTrans("^Sweet Sweet (Love )?Magic$", "", "", "Sweet Sweet &whiteheart; Magic", "", "");
	AddTrans("^Break ?Down!?$", "", "", "BRE&flipped-a;K DOWN!", "", "");
	/* サナ・モレッテ・ネ・エンテ 
		* People can't decide how they want to spell this, so cope with
		* both l or r, and one or two l/r and t. */
	AddTrans("^Sana Mo((ll?)|(rr?))et(t?)e Ne Ente", "", "", 
		"&ksa;&kna;&kdot;&kmo;&kre;&kq;&kte;&kdot;&kne;&kdot;&ke;&kn;&kte;", "", "");
	AddTrans("^Freckles$", "", "", "&hso;&hba;&hka;&hsu;", "", "");
	AddTrans("^Sobakasu$", "", "", "&hso;&hba;&hka;&hsu;", "", "");

	/* 夜空のムコウ */
	AddTrans("^Yozora no Muko$", "", "", "&yozora1;&yozora2;&hno;&kmu;&kko;&ku;", "", "");

	/* 17才 */
	AddTrans("^17 ?(Sai)?$", "", "", "17&sai;", "", "");

	/* Handle "Mobo Moga", "Mobo * Moga"; spaces optional. */
	AddTrans("^Mobo ?\\*? ?Moga$", "", "", "MOBO&whitestar;MOGA", "", "");

	AddTrans("^Love (Love )?Shine$", "", "", "LOVE &whiteheart; SHINE", "", "");

	/* ロマンスの神様 */
	AddTrans("^God of Romance$", "", "", "&kro;&kma;&kn;&ksu;&hno;&kami;&sama;", "", "");

	/* おどるポンポコリン */
	AddTrans("^Dancing Pompokolin$", "", "", "&ho;&hdo;&hru;&kpo;&kn;&kpo;&kko;&kri;&kn;", "", "");

	/* 青い振動 */
	AddTrans("^Aoi Shoudou$", "", "", "&aoi;&hi;&shoudou1;&shoudou2;", "", "");
	AddTrans("^Blue Impulse$", "", "", "&aoi;&hi;&shoudou1;&shoudou2;", "", "");
	/* Handle a typo: */
	AddTrans("^Aio Shoudou$", "", "", "&aoi;&hi;&shoudou1;&shoudou2;", "", "");

	/* 大見解 */
	AddTrans("^Daikenkai$", "", "", "&ookii;&kenkai1;&kenkai2;", "", "");

	/* ♡LOVE²シュガ→♡ */
	AddTrans("^Love Love Sugar$", "", "", "&whiteheart;LOVE&squared; &ksi;&kyus;&kga;&rightarrow;&whiteheart;", "", "");

	/* 三毛猫ロック */
	AddTrans("^Mikeneko Rock$", "", "", "&num-san;&hair;&neko;&kro;&kq;&kku;", "", "");
	
	/* 桜 */
	AddTrans("^Sakura$", "", "", "&sakura;", "", "");

	/* 魔法の扉, スペース☆マコのテーマ  (handle Door and Doors) */
    AddTrans("^(Doors? of Magic)|(Mahou no Tobira)$", "", "", "&mahou1;&mahou2;&hno;&tobira;", "&ksu;&kpe;&kdash;&ksu;&whitestar;&kma;&kko;&hno;&kti;&kdash;&kmu;", ""); 

    AddTrans("^Senorita$", "", "", "Se&x00F1;orita", "", ""); 
    AddTrans("^Senorita\\(Speedy Mix\\)$", "", "", "Se&x00F1;orita", "Speedy Mix", ""); 
    AddTrans("^La Senorita$", "", "", "La Se&x00F1;orita", "", ""); 
    AddTrans("^La Senorita Virtual$", "", "", "La Se&x00F1;orita Virtual", "", ""); 
	
	/* Subtitles: */
	/* それぞれの明日 (title is Graduation) */
	AddTrans("", "^Each Tomorrow$", "", "", "&hso;&hre;&hzo;&hre;&hno;&aka;&nichi;", "");

	/* Artists: */
	AddTrans("", "", "^Omega$", "", "", "&omega;");
	AddTrans("", "", "^ZZ$", "", "", "&doublezeta;");

	/* 亜熱帯マジ-SKA爆弾 (serious tropical ska bomb? ruh roh) */
	AddTrans("", "", "^Anettai Maji.*Ska (Bakudan|Bukuden)", "", "", "&anettai1;&anettai2;&anettai3;&kma;&kji;-SKA&bakudan1;&bakudan2;");

	/* メキシコ民謡 */
	AddTrans("", "", "^Spanish Folk Music$", "", "", "&kme;&kki;&ksi;&kko;&minyou1;&minyou2;");

	/* 新谷さなえ (Sanae or incorrect Sana) */
	AddTrans("", "", "Sanae? Shintani", "", "", "&shintani1;&shintani2;&hsa;&hna;&he;");

	AddTrans("", "", "dj TAKA feat. ?Noria", "", "", "dj TAKA feat. &hno;&hri;&ha;");

	/* くにたけみゆき */
	AddTrans("", "", "Miyuki Kunitake", "", "", "&hku;&hni;&hta;&hke;&hmi;&hyu;&hki;");
	AddTrans("", "", "Kunitake Miyuki", "", "", "&hku;&hni;&hta;&hke;&hmi;&hyu;&hki;");

	/* Courses: */
	AddTrans("^Kidou$", "", "", "&oni;&michi;", "", "");
	AddTrans("^Demon Road$", "", "", "&oni;&michi;", "", "");
	AddTrans("^Kidou 2$", "", "", "&oni;&michi; 2", "", "");
	AddTrans("^Demon Road 2$", "", "", "&oni;&michi; 2", "", "");
	AddTrans("^Love$", "", "", "Love &whiteheart;", "", "");
	AddTrans("^Love Love$", "", "", "Love &whiteheart;", "", "");
}

TitleSubst::~TitleSubst()
{
	for(unsigned i = 0; i < ttab.size(); ++i)
		delete ttab[i];
}
