#include "ScreenEvaluation.h"
#include "NetworkSyncManager.h"
#include "Quad.h"
#include "BitmapText.h"
#include "ScreenMessage.h"

class ScreenNetEvaluation: public ScreenEvaluation
{
public:
	ScreenNetEvaluation (const CString& sClassName);
	virtual void Init();

protected:
	virtual void MenuLeft( PlayerNumber pn, const InputEventType type );
	virtual void MenuUp( PlayerNumber pn, const InputEventType type );
	virtual void MenuRight( PlayerNumber pn, const InputEventType type );
	virtual void MenuDown( PlayerNumber pn, const InputEventType type );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void TweenOffScreen( );

	void UpdateStats( );
private:
	Quad			m_rectUsersBG;

	vector<BitmapText>		m_textUsers;
	int				m_iCurrentPlayer;
	int				m_iActivePlayers;
	
	PlayerNumber	m_pActivePlayer;

	bool			m_bHasStats;

	int m_iShowSide;

	void RedoUserTexts();
};
