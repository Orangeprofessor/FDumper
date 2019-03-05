#pragma once

#include "Control.hpp"

namespace ctrl
{
	class Button : public Control
	{
	public:
		Button(HWND hwnd = NULL)
			: Control(hwnd) {  }

		virtual bool checked() const { return Button_GetCheck(m_hwnd) != BST_UNCHECKED; }
		virtual void checked(bool state) { Button_SetCheck(m_hwnd, state); }

		inline operator bool() { return checked(); }
	};
}