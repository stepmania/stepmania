#ifndef SCREEN_OPTIONS_MASTER_H
#define SCREEN_OPTIONS_MASTER_H

#include "ScreenOptions.h"
#include "ModeChoice.h"

struct ConfOption;

class ScreenOptionsMaster: public ScreenOptions
{
public:
	ScreenOptionsMaster( CString sName );
	virtual ~ScreenOptionsMaster();

private:

	enum OptionRowType
	{
		ROW_LIST, /* list of custom settings */
		ROW_STEP, /* list of steps for the current song or course */
		ROW_CHARACTER, /* list of characters */
		ROW_CONFIG,	/* global pref */
		ROW_SAVE_TO_PROFILE, /* save new options to profile? */
		NUM_OPTION_ROW_TYPES
	};

	struct OptionRowHandler
	{
		OptionRowHandler() { opt = NULL; }

		OptionRowType type;

		/* ROW_LIST: */
		vector<ModeChoice> ListEntries;
		ModeChoice Default;

		/* ROW_CONFIG: */
		const ConfOption *opt;
	};

	CString m_NextScreen;

	vector<OptionRowHandler> OptionRowHandlers;
	OptionRowData *m_OptionRowAlloc;

	int ExportOption( const OptionRowData &row, const OptionRowHandler &hand, int pn, const vector<bool> &vbSelected );
	void ImportOption( const OptionRowData &row, const OptionRowHandler &hand, int pn, int rowno, vector<bool> &vbSelectedOut );
	void SetList( OptionRowData &row, OptionRowHandler &hand, CString param, CString &TitleOut );
	void SetStep( OptionRowData &row, OptionRowHandler &hand );
	void SetConf( OptionRowData &row, OptionRowHandler &hand, CString param, CString &TitleOut );
	void SetCharacter( OptionRowData &row, OptionRowHandler &hand );
	void SetSaveToProfile( OptionRowData &row, OptionRowHandler &hand );

protected:
	virtual void ImportOptions();
	virtual void ExportOptions();
	virtual void ImportOptionsForPlayer( PlayerNumber pn ); // used by ScreenPlayerOptions

	virtual void GoToNextState();
	virtual void GoToPrevState();

	virtual void RefreshIcons();
};


#endif
