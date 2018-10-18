#pragma once

#include "wintoastlib.h"

enum Results {
	ToastClicked,					// user clicked on the toast
	ToastDismissed,					// user dismissed the toast
	ToastTimeOut,					// toast timed out
	ToastHided,						// application hid the toast
	ToastNotActivated,				// toast was not activated
	ToastFailed,					// toast failed
	SystemNotSupported,				// system does not support toasts
	UnhandledOption,				// unhandled option
	MultipleTextNotSupported,		// multiple texts were provided
	InitializationFailure,			// toast notification manager initialization failure
	ToastNotLaunched				// toast could not be launched
};


class CFWatcherNotifHandler : public WinToastLib::IWinToastHandler
{
public:
	virtual void toastActivated() const override
	{

	}
	virtual void toastActivated(int actionIndex) const override
	{

	}
	virtual void toastDismissed(WinToastDismissalReason state) const override
	{

	}
	virtual void toastFailed() const override
	{

	}
};

