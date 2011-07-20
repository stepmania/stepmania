#include "global.h"
#include "InGameLoadingWindow.h"
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "ActorUtil.h"

//REGISTER_ACTOR_CLASS( InGameLoadingWindow );

InGameLoadingWindow::InGameLoadingWindow() {
	SetName("InGameLoadingWindow");
	m_Text.SetName("LoadingText");
	m_Text.LoadFromFont( THEME->GetPathF(m_sName, "LoadingText") );
	m_Text.SetXY(0,0);
	AddChild(&m_Text);
}

InGameLoadingWindow::~InGameLoadingWindow() {
	RemoveChild(&m_Text);
}

void InGameLoadingWindow::SetText( RString str )	{
	textChanged=true;
	currentText=str;
}

void InGameLoadingWindow::Update(float delta) {
	if(textChanged) {
		m_Text.SetText( currentText );
		textChanged=false;
	}
}
