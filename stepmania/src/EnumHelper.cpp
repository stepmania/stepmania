#include "global.h"

#include "EnumHelper.h"
#include "ThemeManager.h"

CString GetThemedString( CCStringRef sClass, CCStringRef sValue )
{
	return THEME->GetMetric( sClass, sValue );
}

