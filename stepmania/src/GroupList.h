#ifndef GROUPLIST_H
#define GROUPLIST_H

#include "ActorFrame.h"
#include "Sprite.h"
#include "BitmapText.h"

const unsigned MAX_GROUPS_ONSCREEN = 8;

class GroupList: public ActorFrame {
	Sprite			m_sprButton[MAX_GROUPS_ONSCREEN];
	BitmapText		m_screenLabels[MAX_GROUPS_ONSCREEN];
	vector<CString> m_asLabels;

	/* Currently selected label. */
	unsigned m_iSelection;
	/* Label that's currently at the top of the screen. */
	unsigned m_iTop;

	void BeforeChange();
	void AfterChange();
	void SetLabels();

public:
	GroupList();

	void SetSelection(unsigned sel);
	int GetSelection() const { return m_iSelection; }
	CString GetSelectionName() const { return m_asLabels[m_iSelection]; }
	void Up();
	void Down();
	void Load( const CStringArray& asGroupNames );
	void TweenOffScreen();
	void TweenOnScreen();
};

#endif
