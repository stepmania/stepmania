#define __USE_GNU
#include "global.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Backtrace.h"
#include "BacktraceNames.h"

#include "RageUtil.h"
#include "CrashHandler.h"
#include "CrashHandlerInternal.h"
#include "RageLog.h" /* for RageLog::GetAdditionalLog, etc. only */
#include "ProductInfo.h"

#if defined(DARWIN)
#include "archutils/Darwin/Crash.h"
#endif

#if defined(HAVE_VERSION_INFO)
extern const unsigned version_num;
#endif

const char *g_pCrashHandlerArgv0 = NULL;


static void output_stack_trace( FILE *out, void **BacktracePointers )
{
	if( BacktracePointers[0] == BACKTRACE_METHOD_NOT_AVAILABLE )
	{
		fprintf( out, "No backtrace method available.\n");
		return;
	}

	if( !BacktracePointers[0] )
	{
		fprintf( out, "Backtrace was empty.\n");
		return;
	}

    for( int i = 0; BacktracePointers[i]; ++i)
    {
        BacktraceNames bn;
        bn.FromAddr( BacktracePointers[i] );
        bn.Demangle();

	/* Don't show the main module name. */
        if( bn.File == g_pCrashHandlerArgv0 )
            bn.File = "";

        if( bn.Symbol == "__libc_start_main" )
            break;

        fprintf( out, "%s\n", bn.Format().c_str() );
    }
}

/* Once we get here, we should be * safe to do whatever we want;
* heavyweights like malloc and CString are OK. (Don't crash!) */
static void child_process()
{
    int ret;

    /* 1. Read the backtrace pointers. */
    void *BacktracePointers[BACKTRACE_MAX_SIZE];
    ret = read(3, BacktracePointers, sizeof(void *)*BACKTRACE_MAX_SIZE);

    /* 2. Read the signal. */
    int SignalReceived;
    ret = read(3, &SignalReceived, sizeof(SignalReceived));

    /* 3. Read info. */
    int size;
    ret = read(3, &size, sizeof(size));
    char *Info = new char [size];
    ret = read(3, Info, size);

    /* 4. Read AdditionalLog. */
    ret = read(3, &size, sizeof(size));
    char *AdditionalLog = new char [size];
    ret = read(3, AdditionalLog, size);

    /* 5. Read RecentLogs. */
    int cnt = 0;
    ret = read(3, &cnt, sizeof(cnt));
    char *Recent[1024];
    for( int i = 0; i < cnt; ++i )
    {
        ret = read(3, &size, sizeof(size));
        Recent[i] = new char [size];
        ret = read(3, Recent[i], size);
    }

    /* 6. Read CHECKPOINTs. */
    ret = read(3, &size, sizeof(size));
    char *temp = new char [size];
    ret = read(3, temp, size);
    CStringArray Checkpoints;
    split(temp, "$$", Checkpoints);
    delete [] temp;

    /* 7. Read the crashed thread's name. */
    ret = read(3, &size, sizeof(size));
    temp = new char [size];
    ret = read(3, temp, size);
    const CString CrashedThread(temp);
    delete[] temp;

    /* Wait for the child to either finish cleaning up or die.  XXX:
        * This should time out, in case something deadlocks. */

    char x;
    ret = read(3, &x, sizeof(x));
    if( (ret == -1 && errno != EPIPE) || ret != 0 )
    {
        /* We expect an EOF or EPIPE.  What happened? */
        fprintf(stderr, "Unexpected child read() result: %i (%s)\n", ret, strerror(errno));
        /* keep going */
    }

	const char *home = getenv( "HOME" );
	CString sCrashInfoPath = "/tmp";
	if( home )
		sCrashInfoPath = home;
	sCrashInfoPath += "/crashinfo.txt";

	FILE *CrashDump = fopen( sCrashInfoPath, "w+" );
	if(CrashDump == NULL)
	{
		fprintf( stderr, "Couldn't open " + sCrashInfoPath + ": %s\n", strerror(errno) );
		exit(1);
	}

    fprintf(CrashDump, "%s crash report", PRODUCT_NAME_VER );
#if defined(HAVE_VERSION_INFO)
    fprintf(CrashDump, " (build %u)", version_num);
#endif
    fprintf(CrashDump, "\n");
    fprintf(CrashDump, "--------------------------------------\n");
    fprintf(CrashDump, "\n");

    CString Signal = SignalName( SignalReceived );
    
    fprintf( CrashDump, "Crash reason: %s\n", Signal.c_str() );
    fprintf( CrashDump, "Crashed thread: %s\n\n", CrashedThread.c_str() );

    fprintf(CrashDump, "Checkpoints:\n");
    for (unsigned i=0; i<Checkpoints.size(); ++i)
        fprintf(CrashDump, Checkpoints[i]);
    fprintf(CrashDump, "\n");

    output_stack_trace( CrashDump, BacktracePointers );
    fprintf(CrashDump, "\n");

    fprintf(CrashDump, "Static log:\n");
    fprintf(CrashDump, "%s", Info);
    fprintf(CrashDump, "%s", AdditionalLog);
    fprintf(CrashDump, "\nPartial log:\n");
    for( int i = 0; i < cnt; ++i )
        fprintf(CrashDump, "%s\n", Recent[i] );
    fprintf(CrashDump, "\n");
    fprintf(CrashDump, "-- End of report\n");
    fclose(CrashDump);

#if defined(DARWIN)
    InformUserOfCrash( sCrashInfoPath );
#else
    fprintf(stderr,
            "\n"
            PRODUCT_NAME " has crashed.  Debug information has been output to\n"
            "\n"
            "    " + sCrashInfoPath + "\n"
            "\n"
            "Please report a bug at:\n"
            "\n"
            "    http://sourceforge.net/tracker/?func=add&group_id=37892&atid=421366\n"
            "\n"
            );
#endif

    /* Forcibly kill our parent. */
    //	kill( getppid(), SIGKILL );
}


void CrashHandlerHandleArgs( int argc, char* argv[] )
{
	g_pCrashHandlerArgv0 = argv[0];
	if( argc == 2 && !strcmp(argv[1], CHILD_MAGIC_PARAMETER) )
	{
		child_process();
		exit(0);
	}
}


