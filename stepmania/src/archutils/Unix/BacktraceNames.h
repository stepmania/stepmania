#ifndef BACKTRACE_NAMES_H
#define BACKTRACE_NAMES_H

struct BacktraceNames
{
	CString Symbol, File;
	int Address;
	int Offset;
	void FromAddr( const void *p );
	void FromString( CString str );
	void Demangle();
	CString Format() const;
	BacktraceNames(): Address(0), Offset(0) { }
};

#endif

