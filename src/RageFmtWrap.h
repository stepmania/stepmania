#ifndef RAGE_FMT_WRAP_H
#define RAGE_FMT_WRAP_H

// Wanted to put rage_fmt_wrapper in RageUtil, but RageUtil.h is included
// by stuff like dialog drivers, and gtk module, which don't have a reason to
// include LuaManager.  -Kyz

template<typename strish>
void log_fmt_error(strish const& msg)
{
	LuaHelpers::ReportScriptErrorFmt("Format error in metric %s :: %s.", msg.GetGroup().c_str(), msg.GetName().c_str());
}

template<typename strish, typename... Args>
	std::string rage_fmt_wrapper(strish const& msg, Args const& ...args)
{
	try
	{
		return fmt::sprintf(msg.GetValue(), args...);
	}
	catch(...)
	{
		log_fmt_error(msg);
	}
	return "";
}


#endif
