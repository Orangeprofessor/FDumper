#pragma once

#include "../pch.h"
#include "../Win32Thunk.hpp"
#include <string>
#include <map>

#define MSG_HANDLER( n ) virtual INT_PTR n( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )

class Window
{
public:
	typedef INT_PTR(Window::*fnWndProc)(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	typedef std::map<UINT, std::pair<Window*, fnWndProc>> mapWndProc;

public:
	Window(HWND hwnd = NULL)
		: m_hwnd(hwnd)
		, m_subThunk(&Window::SubProc, this) { }

	virtual inline void Attach(HWND hwnd) { m_hwnd = hwnd; }
	virtual inline void Attach(HWND hDlg, UINT id) { m_id = id; m_hwnd = GetDlgItem(hDlg, id); }

	virtual inline HWND hwnd() const { return m_hwnd; }
	virtual inline UINT id()   const { return m_id; }

	virtual inline void enable()  const { EnableWindow(m_hwnd, TRUE); }
	virtual inline void disable() const { EnableWindow(m_hwnd, FALSE); }

	virtual std::wstring text() const
	{
		wchar_t buf[512] = { 0 };
		GetWindowTextW(m_hwnd, buf, ARRAYSIZE(buf));

		return buf;
	}

	virtual inline BOOL text(const std::wstring& text)
	{
		return SetWindowText(m_hwnd, text.c_str());
	}

	virtual inline WNDPROC oldProc() const { return m_oldProc; }

	virtual void Subclass(UINT message, fnWndProc handler, Window* instance = nullptr)
	{
		// Remove old handler
		if (handler == nullptr)
		{
			if (m_subMessages.count(message))
			{
				m_subMessages.erase(message);

				// Remove subproc
				if (m_subMessages.empty())
				{
					SetWindowLongPtrW(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_oldProc));
					m_oldProc = nullptr;
				}

				return;
			}
		}
		else
		{
			m_subMessages[message] = std::make_pair(instance ? instance : this, handler);
			if (!m_oldProc)
				m_oldProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_subThunk.GetThunk())));
		}
	}

private:
	LRESULT SubProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (m_subMessages.count(message))
			return (this->m_subMessages[message].first->*m_subMessages[message].second)(hwnd, message, wParam, lParam);

		return CallWindowProcW(m_oldProc, hwnd, message, wParam, lParam);
	}

protected:
	HWND m_hwnd = NULL;
	UINT m_id = 0;
	WNDPROC m_oldProc = nullptr;
	mapWndProc m_subMessages;
	Win32Thunk<WNDPROC, Window> m_subThunk;
};