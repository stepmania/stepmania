#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>
       
class SaveSignals
{
	vector<struct sigaction> old_handlers;

public:
	SaveSignals(); /* save signals */
	~SaveSignals(); /* restore signals */
};

namespace SignalHandler
{
	typedef void (*handler)(int);

	void OnClose(handler);
};

#endif

/* Written by Glenn Maynard.  Public domain. */

