#ifndef SCREEN_OPTIONS_MASTER_PREFS_H
#define SCREEN_OPTIONS_MASTER_PREFS_H

static const int MAX_OPTIONS=16;
#define OPT_SAVE_PREFERENCES	0x1
#define OPT_APPLY_GRAPHICS		0x2
#define OPT_APPLY_THEME			0x4

struct ConfOption
{
	static const ConfOption *Find( CString name );

	/* Name of this option.  It's helpful to delimit this with newlines, as it'll
	 * be the default display title. */
	CString name;
	typedef void (*MoveData_t)( int &sel, bool ToSel, const CStringArray &choices );
	MoveData_t MoveData;

	/* If MakeOptionsListCB is set, it'll be called by MakeOptionsList; otherwise
	 * the contents of names is returned. */
	void MakeOptionsList( CStringArray &out ) const;

	inline int Get( const CStringArray &choices ) const { int sel; MoveData( sel, true, choices ); return sel; }
	inline void Put( int sel, const CStringArray &choices ) const { MoveData( sel, false, choices ); }
	int GetEffects() const;

	ConfOption( const char *n, void (*m)( int &sel, bool in, const CStringArray &choices  ),
		const char *c0=NULL, const char *c1=NULL, const char *c2=NULL, const char *c3=NULL, const char *c4=NULL, const char *c5=NULL, const char *c6=NULL, const char *c7=NULL, const char *c8=NULL, const char *c9=NULL, const char *c10=NULL, const char *c11=NULL, const char *c12=NULL, const char *c13=NULL, const char *c14=NULL, const char *c15=NULL, const char *c16=NULL, const char *c17=NULL, const char *c18=NULL, const char *c19=NULL )
	{
		name = n;
		MoveData = m;
		MakeOptionsListCB = NULL;
#define PUSH( c )	if(c) names.push_back(c);
		PUSH(c0);PUSH(c1);PUSH(c2);PUSH(c3);PUSH(c4);PUSH(c5);PUSH(c6);PUSH(c7);PUSH(c8);PUSH(c9);PUSH(c10);PUSH(c11);PUSH(c12);PUSH(c13);PUSH(c14);PUSH(c15);PUSH(c16);PUSH(c17);PUSH(c18);PUSH(c19);
#undef PUSH
	}

	ConfOption( const char *n, void (*m)( int &sel, bool in, const CStringArray &choices ),
			void (*lst)( CStringArray &out ) )
	{
		name = n;
		MoveData = m;
		MakeOptionsListCB = lst;
	}


private:
	vector<CString> names;
	void (*MakeOptionsListCB)( CStringArray &out );
};


#endif
