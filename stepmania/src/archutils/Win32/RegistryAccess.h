#ifndef REGISTRY_ACCESS_H
#define REGISTRY_ACCESS_H

#include "windows.h"
bool GetRegValue( const CString &key, const CString &sName, CString &val );
bool GetRegValue( const CString &key, CString sName, int &val );
bool GetRegSubKeys( const CString &key, vector<CString> &lst, const CString &regex = ".*", bool bReturnPathToo = true );

#endif
