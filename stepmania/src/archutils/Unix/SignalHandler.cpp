#include "global.h"

#include "RageLog.h"
#include "SignalHandler.h"

#include <unistd.h>

static vector<SignalHandler::handler> handlers;
SaveSignals *saved_sigs;
static bool initted = false;

static int signals[] = {
	SIGALRM, SIGBUS, SIGFPE, SIGHUP, SIGILL, SIGINT, SIGIOT, SIGPIPE,
	SIGPWR, SIGQUIT, SIGSEGV, SIGTRAP, SIGTERM, SIGVTALRM, SIGXCPU, SIGXFSZ,
	-1
};

/*
 * Store signals and restore them.  This lets us clean up after libraries
 * that like to mess with our signal handler.  (Of course, if we undo
 * signal handlers for libraries, we need to be sure to handle whatever it
 * was cleaning up by hand.)
 */

SaveSignals::SaveSignals()
{
	/* Store the old signal handlers. */
	for(int i = 0; signals[i] != -1; ++i)
	{
        old_handlers.push_back();
		sigaction(signals[i], NULL, &old_handlers.back());
	}
}

SaveSignals::~SaveSignals()
{
	/* Restore the old signal handlers. */
	for(unsigned i = 0; i < old_handlers.size(); ++i)
		sigaction(signals[i], &old_handlers[i], NULL);
}

static void SigHandler(int sig)
{
	LOG->Trace("got sig %i", sig);
	for(unsigned i = 0; i < handlers.size(); ++i)
		handlers[i](sig);

	_exit(1);
}

/* Hook up events to fatal signals, so we can clean up if we're killed. */
void SignalHandler::OnClose(handler h)
{
	if(saved_sigs == NULL)
	{
		saved_sigs = new SaveSignals;

		/* Set up our signal handlers. */
		for(int i = 0; signals[i] != -1; ++i)
		{
			struct sigaction sa;

			sa.sa_handler = SigHandler;
			sa.sa_flags = 0;
			sigemptyset(&sa.sa_mask);

			sigaction(signals[i], &sa, NULL);
		}
	}
	handlers.push_back(h);
}


/* Written by Glenn Myanard.  Public domain, copyrighting a simple signal handler
 * is dumb. */

