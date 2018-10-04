
#include "pch.h"

#include "Menu.h"

#include "fADumper.h"

#include <libda3m0n/Misc/Utils.h>
#include <ShlObj.h>



CMainDialog::CMainDialog() : Dialog(IDD_MAIN)
{
	_messages[WM_INITDIALOG] = static_cast<Dialog::fnDlgProc>(&CMainDialog::OnInit);

	_events[IDC_START_DL] = static_cast<Dialog::fnDlgProc>(&CMainDialog::OnDLStart);
	_events[IDC_CANCEL_DL] = static_cast<Dialog::fnDlgProc>(&CMainDialog::OnDLStop);
	_events[ID_SETTINGS_CREATEDOWNLOADFOLDER] = static_cast<Dialog::fnDlgProc>(&CMainDialog::OnCreateUsername);
	_events[ID_SETTINGS_CREATESUBGALLERYFOLDERS] = static_cast<Dialog::fnDlgProc>(&CMainDialog::OnCreateGallery);
}

CMainDialog::~CMainDialog()
{
	if (m_dumperThread.joinable())
		m_dumperThread.join();
}

INT_PTR CMainDialog::OnInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Dialog::OnInit(hDlg, message, wParam, lParam);

	m_apiurl.Attach(m_hwnd, IDC_API);
	m_images.Attach(m_hwnd, IDC_IMAGE_LIST);
	m_ratings.Attach(m_hwnd, IDC_RATINGS);
	m_galleries.Attach(m_hwnd, IDC_GALLERIES);
	m_username.Attach(m_hwnd, IDC_USERNAME);
	m_update.Attach(m_hwnd, IDC_UPDATE_USERNAME);
	m_startDL.Attach(m_hwnd, IDC_START_DL);
	m_stopDL.Attach(m_hwnd, IDC_CANCEL_DL);

	SetWindowText(m_hwnd, L"FDumper v0.2");

	m_images.AddColumn(L"Name", 100, 0);
	ListView_SetExtendedListViewStyle(m_images.hwnd(), LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	m_ratings.Add("All Ratings");
	m_ratings.Add("SFW Only", 1);
	m_ratings.Add("NSFW Only", 2);

	m_galleries.Add("All Galleries");
	m_galleries.Add("No Scraps", 1);
	m_galleries.Add("Scraps Only", 2);

	m_ratings.selection(0);
	m_galleries.selection(0);

	return TRUE;
}

INT_PTR CMainDialog::OnDLStart(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto dest = OpenSaveDialog();

	if (dest.empty())
		return FALSE;

	auto username = m_username.text();
	auto apiurl = m_apiurl.text();

	auto ratings = m_ratings.selection();
	auto galleries = m_ratings.selection();

	dest += std::wstring(L"\\") + username;
	CreateDirectory(dest.c_str(), NULL);

	pDumper = new CFADumper(libdaemon::Utils::WstringToAnsi(apiurl),
		libdaemon::Utils::WstringToAnsi(username), dest, ratings, galleries, true);

	auto downloader = [&](int id) {
		pDumper->Download();
		if (pDumper)
			delete pDumper;
	};

	m_notapool.push(downloader);

	return TRUE;
}

INT_PTR CMainDialog::OnDLStop(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	m_notapool.stop();

	if (pDumper)
		delete pDumper;

	return TRUE;
}

INT_PTR CMainDialog::OnDLPause(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

INT_PTR CMainDialog::OnDLResume(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

INT_PTR	CMainDialog::OnCreateUsername(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//auto menu = GetMenu(hDlg);

	//MENUITEMINFO menudata; ZeroMemory(&menudata, sizeof(menudata));
	//menudata.cbSize = sizeof(menudata);

	//GetMenuItemInfo(menu, 0, TRUE, &menudata);

	//menudata.chec

	return TRUE;
}

INT_PTR CMainDialog::OnCreateGallery(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

std::wstring CMainDialog::OpenSaveDialog()
{
	TCHAR szDir[MAX_PATH];
	BROWSEINFO bInfo;
	bInfo.hwndOwner = m_hwnd;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
	bInfo.lpszTitle = L"Pwease sewect a fowdew UwU"; // Title of the dialog
	bInfo.ulFlags = 0;
	bInfo.lpfn = NULL;
	bInfo.lParam = 0;
	bInfo.iImage = -1;

	LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);

	SHGetPathFromIDList(lpItem, szDir);
	return szDir;
}

