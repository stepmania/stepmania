// Ehhh....
//
// I think I'll just put this one in the public domain
// (with no warranty as usual).
//
// --Avery

// took some ideas from OpenITG... -aj

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

typedef unsigned long ulong;

int main(void)
{
	FILE *f;
	ulong build=0;
	char strdate[10], strtime[64];
	time_t tm;
	//struct tm *ptm;

	// try to read the last version seen
	if( f = fopen("version.bin","r") )
	{
		fread( &build, sizeof(ulong), 1, f );
		fclose( f );
	}

	// increment the build number and write it
	build++;

	if ( f = fopen("version.bin","wb") )
	{
		fwrite(&build,sizeof(ulong),1,f);
		fclose(f);
	}
//	printf("Incrementing to build %d\n",build);

	// get the current time
	time(&tm);

	/*
	//memcpy(version_time, asctime(localtime(&tm)), sizeof(version_time)-1);
	ptm = localtime(&tm);
	strftime(s, sizeof(s), "%Y%m%d", ptm);
	s[sizeof(s)-1]=0;
	*/

	// print the debug serial date/time
	strftime( strdate, 15, "%Y%m%d", localtime(&tm) );
	strftime( strtime, 64, "%H:%M:%S %Z", localtime(&tm) );
	//memcpy( strtime, asctime(localtime(&tm)), 24 );

	// zero out the newline character
	strtime[sizeof(strtime)-1] = 0;

	// write to verstub
	if ( f = fopen("verstub.cpp","w") )
	{
		fprintf(f,
			"#include \"ver.h\"\n"
			"unsigned long const version_num = %ld;\n"
			"extern char const * const version_date = \"%s\";\n"
			"extern char const * const version_time = \"%s\";\n",
			build, strdate, strtime);
		fclose(f);
	}

	return 0;
}
