#ifndef SCREEN_OPTIONS_MASTER_PREFS_H
#define SCREEN_OPTIONS_MASTER_PREFS_H

static const int MAX_OPTIONS=16;
struct ConfOption
{
	/* Name of this option.  It's helpful to delimit this with newlines, as it'll
	 * be the default display title. */
	CString name;
	void (*MoveData)( int &sel, bool ToSel );
	inline int Get() const { int sel; MoveData( sel, true ); return sel; }
	inline void Put( int sel ) const { MoveData( sel, false ); }
	vector<CString> names;

	ConfOption( const char *n, void (*m)( int &sel, bool in ),
		const char *c0=NULL, const char *c1=NULL, const char *c2=NULL, const char *c3=NULL, const char *c4=NULL, const char *c5=NULL, const char *c6=NULL, const char *c7=NULL, const char *c8=NULL, const char *c9=NULL, const char *c10=NULL, const char *c11=NULL, const char *c12=NULL, const char *c13=NULL, const char *c14=NULL, const char *c15=NULL, const char *c16=NULL, const char *c17=NULL, const char *c18=NULL, const char *c19=NULL )
	{
		name = n;
		MoveData = m;
#define PUSH( c )	if(c) names.push_back(c);
		PUSH(c0);PUSH(c1);PUSH(c2);PUSH(c3);PUSH(c4);PUSH(c5);PUSH(c6);PUSH(c7);PUSH(c8);PUSH(c9);PUSH(c10);PUSH(c11);PUSH(c12);PUSH(c13);PUSH(c14);PUSH(c15);PUSH(c16);PUSH(c17);PUSH(c18);PUSH(c19);
#undef PUSH
	}
};

const ConfOption *FindConfOption( CString name );

#endif
