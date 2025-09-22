#pragma once
//Contains Definitions for the Functions to be supported by the Adapter class.

#include <afxcoll.h>
#include "PipeConstants.h"

class IWardwizUtilities
{

public:
	// Forbid copying
	IWardwizUtilities(IWardwizUtilities const &) = delete;

	//virtual destructor.
	IWardwizUtilities & operator=(IWardwizUtilities const &) = delete;

	virtual ~IWardwizUtilities() = default;

	virtual BOOL AutorunScanner() = 0;

	virtual BOOL USBVaccinator() = 0;

	virtual BOOL TemporaryFilesCleaner() = 0;

protected :
	// allow construction for child classes only
	IWardwizUtilities()=default;
};
