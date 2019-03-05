#pragma once

#include "window.hpp"

/// <summary>
/// Generic dialog class
/// </summary>
class Dialog : public Window
{
public:
	typedef INT_PTR(Dialog::*fnDlgProc)(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	using mapMsgProc = std::map<UINT, fnDlgProc>;
	using mapCtrlProc = std::map<WORD, fnDlgProc>;
	using mapNotifyProc = std::map<std::pair<UINT, UINT_PTR>, fnDlgProc>;

public:
	Dialog(int dialogID)
		: _dialogID(dialogID)
	{
		m_messages[WM_INITDIALOG] = &Dialog::OnInit;
		m_messages[WM_COMMAND] = &Dialog::OnCommand;
		m_messages[WM_CLOSE] = &Dialog::OnClose;
		m_messages[WM_NOTIFY] = &Dialog::OnNotify;
	}

	virtual INT_PTR RunModal(HWND parent = NULL)
	{
		// std::function and std::bind don't support __stdcall
		// so just do the ASM yourself lol
		Win32Thunk<DLGPROC, Dialog> dlgProc(&Dialog::DlgProc, this);
		return DialogBoxW(NULL, MAKEINTRESOURCEW(_dialogID), parent, dlgProc.GetThunk());
	}

	virtual INT_PTR RunModeless(HWND parent = NULL, int accelID = 0)
	{
		MSG msg = { 0 };
		BOOL bRet = FALSE;
		HACCEL hAccel = LoadAcceleratorsW(NULL, MAKEINTRESOURCEW(accelID));
		_modeless = true;

		Win32Thunk<DLGPROC, Dialog> dlgProc(&Dialog::DlgProc, this);
		m_hwnd = CreateDialogW(NULL, MAKEINTRESOURCE(_dialogID), parent, dlgProc.GetThunk());
		ShowWindow(m_hwnd, SW_SHOW);

		while (IsWindow(m_hwnd) && GetMessageW(&msg, NULL, 0, 0) > 0)
		{
			if (TranslateAccelerator(m_hwnd, hAccel, &msg))
				continue;

			if (!IsDialogMessage(m_hwnd, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		return TRUE;
	}

protected:

	INT_PTR CloseDialog()
	{
		if (_modeless)
		{
			DestroyWindow(m_hwnd);
			m_hwnd = NULL;
		}
		else
			EndDialog(m_hwnd, 0);

		return TRUE;
	}

	INT_PTR DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (m_messages.count(message))
			return (this->*m_messages[message])(hDlg, message, wParam, lParam);

		return FALSE;
	}

	virtual INT_PTR OnInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		m_hwnd = hDlg;
		return TRUE;
	}

	virtual INT_PTR OnCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (m_events.count(LOWORD(wParam)))
			return (this->*m_events[LOWORD(wParam)])(hDlg, message, wParam, lParam);

		if (m_events.count(HIWORD(wParam)))
			return (this->*m_events[HIWORD(wParam)])(hDlg, message, wParam, lParam);

		return FALSE;
	}

	virtual INT_PTR OnNotify(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto notif = (LPNMHDR)lParam;

		if (m_notifs.count(std::make_pair(notif->code, notif->idFrom)))
			return (this->*m_notifs[std::make_pair(notif->code, notif->idFrom)])(hDlg, message, wParam, lParam);

		return FALSE;
	}

	virtual INT_PTR OnClose(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		return CloseDialog();
	}

public:
	mapMsgProc m_messages;      // Message handlers
	mapCtrlProc m_events;       // Event handlers
	mapNotifyProc m_notifs;		// Handlers just for WM_NOTIFY
	int _dialogID;              // Dialog resource ID
	bool _modeless = false;     // Modeless dialog
};

