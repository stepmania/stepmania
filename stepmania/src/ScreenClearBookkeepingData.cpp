#include "global.h"

#include "ScreenClearBookkeepingData.h"
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "Bookkeeper.h"

#define NEXT_SCREEN			THEME->GetMetric(m_sName,"NextScreen")

ScreenClearBookkeepingData::ScreenClearBookkeepingData( CString sName ): Screen( sName )
{
	BOOKKEEPER->ClearAll();
	BOOKKEEPER->WriteToDisk();

	SCREENMAN->SystemMessage( "Bookkeeping data cleared." );

	SCREENMAN->SetNewScreen( NEXT_SCREEN );
}

