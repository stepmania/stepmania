#include "global.h"

#include "ScreenDownloadMachineStats.h"
#include "ProfileManager.h"
#include "ScreenManager.h"
#include "MemoryCardManager.h"
#include "ThemeManager.h"
#include "PrefsManager.h"

#define NEXT_SCREEN			THEME->GetMetric(m_sName,"NextScreen")

static void SaveMachineStatsToFirstMemCard()
{
	FOREACH_PlayerNumber( pn )
	{
		if( MEMCARDMAN->GetCardState(pn) != MEMORY_CARD_STATE_READY )
			continue;	// skip

		CString sDir = MEM_CARD_MOUNT_POINT[pn];
		sDir += "MachineProfile/";

		bool bOldVal = PREFSMAN->m_bWriteMachineStatsHtml;
		PREFSMAN->m_bWriteMachineStatsHtml = true;
		
		PROFILEMAN->GetMachineProfile()->SaveAllToDir( sDir, PREFSMAN->m_bSignProfileData );
		
		PREFSMAN->m_bWriteMachineStatsHtml = bOldVal;
		SCREENMAN->SystemMessage( ssprintf("Machine stats saved to P%d card.",pn+1) );
		return;
	}

	SCREENMAN->SystemMessage( "Stats not saved - No memory cards ready." );
}

ScreenDownloadMachineStats::ScreenDownloadMachineStats( CString sName ): Screen( sName )
{
	SaveMachineStatsToFirstMemCard();

	SCREENMAN->SetNewScreen( NEXT_SCREEN );
}

