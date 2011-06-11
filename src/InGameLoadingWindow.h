#include "arch/LoadingWindow/LoadingWindow.h"
#include "BitmapText.h"
#include "RageTimer.h"
#include "global.h"
#include "ActorFrame.h"

class InGameLoadingWindow: public LoadingWindow, public ActorFrame {

public:
	InGameLoadingWindow();
	~InGameLoadingWindow();

	void SetText( RString str );
	void Update ( float delta );

private:
	bool textChanged;
	RString currentText;
	BitmapText m_Text;
};

