#ifndef GROUPLIST_H
#define GROUPLIST_H

#include "ActorFrame.h"
#include "Sprite.h"
#include "BitmapText.h"

class GroupList: public ActorFrame {
	ActorFrame			 m_Frame;
	vector<Sprite *>	 m_sprButtons;
	vector<BitmapText *> m_textLabels;
	vector<CString>		 m_asLabels;

	vector<bool>		 m_bHidden;

	/* Currently selected label. */
	int m_iSelection;
	/* Label that's currently at the top of the screen. */
	int m_iTop;

	void BeforeChange();
	void AfterChange();
	bool ItemIsOnScreen( int n ) const;
	void ResetTextSize( int item );

public:
	GroupList();
	~GroupList();

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
