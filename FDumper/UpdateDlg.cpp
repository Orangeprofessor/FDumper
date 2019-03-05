#include "pch.h"

#include "UpdateDlg.h"

#include "FAUpdater.h"

CUpdateDlg::CUpdateDlg(ConfigMgr& config) : Dialog(IDD_UPDATE), m_config(config)
{
	m_messages[WM_INITDIALOG] = static_cast<BaseClass::fnDlgProc>(&CUpdateDlg::OnInit);
	m_messages[WM_CLOSE] = static_cast<BaseClass::fnDlgProc>(&CUpdateDlg::OnClose);


}

INT_PTR CUpdateDlg::OnInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BaseClass::OnInit(hDlg, message, wParam, lParam);

	m_userlist.Attach(hDlg, IDC_UPDATE_USERLIST);

	m_userpfplist = ImageList_Create(100, 90, ILC_COLOR32, 0, 1);
	ListView_SetImageList(m_userlist.hwnd(), m_userpfplist, LVSIL_NORMAL);
	ImageList_SetImageCount(m_userpfplist, 1);


	return TRUE;
}

INT_PTR CUpdateDlg::OnClose(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// do something here idk
	return BaseClass::OnClose(hDlg, message, wParam, lParam);
}

