#include "global.h"

#include "ScreenResetToDefaults.h"
#include "ProfileManager.h"
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "PrefsManager.h"

#define NEXT_SCREEN			THEME->GetMetric(m_sName,"NextScreen")

ScreenResetToDefaults::ScreenResetToDefaults( CString sName ): Screen( sName )
{
	PREFSMAN->ResetToFactoryDefaults();

	SCREENMAN->SystemMessage( "All options reset to factory defaults." );

	SCREENMAN->SetNewScreen( NEXT_SCREEN );
}

