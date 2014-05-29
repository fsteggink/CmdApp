// CmdApp_internal.h
// FS, 09-07-2007

#ifndef CMDAPP_INTERNAL_H
#define CMDAPP_INTERNAL_H

#include "../../Inc/CmdApp.h"
#include <iostream>
#include <iomanip>
#include <functional>
#include <algorithm>
#include <windows.h>


namespace FSUtils
{


const int FORM_WIDTH = 30;   // Space the short/long form information takes in the help screen


class FindArg_short: public std::binary_function<std::string, CmdArg, bool>
{
public:
	bool operator()(const std::string &shortForm, const CmdArg &arg) const
	{
		if(arg.shortForm.empty())
			return false;

		return (arg.shortForm.compare(shortForm.substr(0, arg.shortForm.length())) == 0);
	}
};


class FindArg_long: public std::binary_function<std::string, CmdArg, bool>
{
public:
	bool operator()(const std::string &longForm, const CmdArg &arg) const
	{
		return (arg.longForm.compare(longForm.substr(0, arg.longForm.length())) == 0);
	}
};


} // namespace FSUtils


#endif // CMDAPP_INTERNAL_H
