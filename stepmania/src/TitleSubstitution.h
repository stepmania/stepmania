#ifndef TITLE_SUBSTITUTION_H
#define TITLE_SUBSTITUTION_H 1

struct TitleTrans;

class TitleSubst
{
	vector<TitleTrans *> ttab;

	void AddTrans(const CString &tf, const CString &sf, const CString &af,
			   const CString &tt, const CString &st, const CString &at, bool translit);
public:
	TitleSubst();
	~TitleSubst();

	void Load(const CString &filename);

	void Subst(CString &title, CString &sub, CString &artist,
					   CString &ttitle, CString &tsub, CString &tartist);
};

#endif
