#include "global.h"

#include "ScreenOptionsMasterPrefs.h"
#include "PrefsManager.h"

/* "sel" is the selection in the menu. */
static void MoveData( int &sel, int &opt, bool ToSel )
{
	if( ToSel )	sel = opt;
	else		opt = sel;
}

static void MoveData( int &sel, bool &opt, bool ToSel )
{
	if( ToSel )	sel = opt;
	else		opt = !!sel;
}

#define MOVE( name, opt ) \
	static void name( int &sel, bool ToSel ) \
	{ \
		MoveData( sel, opt, ToSel ); \
	}

MOVE( PreloadSounds, PREFSMAN->m_bSoundPreloadAll );
MOVE( ResamplingQuality, PREFSMAN->m_iSoundResampleQuality );

static const ConfOption g_ConfOptions[] =
{
	ConfOption( "Preload\nSounds", PreloadSounds, "NO","YES" ),
	ConfOption( "Resampling\nQuality", ResamplingQuality, "FAST","NORMAL","HIGH QUALITY" ),
	ConfOption( "", NULL )
};

const ConfOption *FindConfOption( CString name )
{
	for( unsigned i = 0; g_ConfOptions[i].name != ""; ++i )
	{
		const ConfOption *opt = &g_ConfOptions[i];

		CString match(opt->name);
		match.Replace("\n", "");
		match.Replace("-", "");

		if( match != name )
			continue;

		return opt;
	}

	return NULL;
}

void ConfOption::MakeOptionsList( CStringArray &out ) const
{
	if( MakeOptionsListCB == NULL )
	{
		out = names;
		return;
	}

	MakeOptionsListCB( out );
}

