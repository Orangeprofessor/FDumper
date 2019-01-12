#pragma once

#include "Control.hpp"

namespace ctrl
{
	class ProgressBar : public Control
	{
	public:
		ProgressBar(HWND hwnd = NULL) :
			Control(hwnd) {	}

		virtual void SetTotal(size_t total)
		{
			SendMessage(m_hwnd, PBM_SETRANGE, 0, MAKELPARAM(0, total));
			SendMessage(m_hwnd, PBM_SETSTEP, (WPARAM)1, 0);
		}

		virtual void Reset()
		{
			SendMessage(m_hwnd, PBM_SETRANGE, 0, MAKELPARAM(0, 0));
			SendMessage(m_hwnd, PBM_SETPOS, 0, 0);
		}

		virtual void Step()
		{
			SendMessage(m_hwnd, PBM_STEPIT, 0, 0);
		}
	};
}