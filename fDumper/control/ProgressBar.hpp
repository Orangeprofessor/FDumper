#pragma once

#include "Control.hpp"

namespace ctrl
{
	class ProgressBar : Control
	{
	public:
		ProgressBar(HWND hwnd = NULL) :
			Control(hwnd) {	}

		virtual void SetTotal(size_t total)
		{

		}
	};
}