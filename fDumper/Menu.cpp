
#include "pch.h"

#include "Menu.h"
#include <regex>

#include "fADumper.h"

#include <libda3m0n/Misc/Utils.h>
#include <ShlObj.h>

CMainDialog::CMainDialog() : Dialog(IDD_MAIN)
{
	_messages[WM_INITDIALOG] = static_cast<Dialog::fnDlgProc>(&CMainDialog::OnInit);

	_events[IDC_START_DL] = static_cast<Dialog::fnDlgProc>(&CMainDialog::OnDLStart);
	_events[IDC_CANCEL_DL] = static_cast<Dialog::fnDlgProc>(&CMainDialog::OnDLStop);
	_events[IDC_PAUSE_DL] = static_cast<Dialog::fnDlgProc>(&CMainDialog::OnDLPause);
	_events[IDC_RESUME_DL] = static_cast<Dialog::fnDlgProc>(&CMainDialog::OnDLResume);
	_events[ID_SETTINGS_CHANGEAPIADDRESS] = static_cast<Dialog::fnDlgProc>(&CMainDialog::OnChangeAPI);
}

CMainDialog::~CMainDialog()
{
	TerminateThread(m_downloadThread.native_handle(), 0);
}

INT_PTR CMainDialog::OnInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Dialog::OnInit(hDlg, message, wParam, lParam);

	m_images.Attach(m_hwnd, IDC_IMAGE_LIST);
	m_ratings.Attach(m_hwnd, IDC_RATINGS);
	m_progress.Attach(m_hwnd, IDC_DL_PROGRESS);
	m_galleries.Attach(m_hwnd, IDC_GALLERIES);
	m_username.Attach(m_hwnd, IDC_USERNAME);
	m_update.Attach(m_hwnd, IDC_UPDATE_USERNAME);
	m_startDL.Attach(m_hwnd, IDC_START_DL);
	m_stopDL.Attach(m_hwnd, IDC_CANCEL_DL);
	m_pauseDL.Attach(m_hwnd, IDC_PAUSE_DL);
	m_resumeDL.Attach(m_hwnd, IDC_RESUME_DL);

	SetWindowText(m_hwnd, L"FDumper v0.2");

	m_images.AddColumn(L"Name", 160, 0);
	m_images.AddColumn(L"ID", 60, 1);
	m_images.AddColumn(L"Rating", 60, 2);
	ListView_SetExtendedListViewStyle(m_images.hwnd(), LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	m_ratings.Add("All Ratings");
	m_ratings.Add("SFW Only", 1);
	m_ratings.Add("NSFW Only", 2);

	m_galleries.Add("All Galleries");
	m_galleries.Add("No Scraps", 1);
	m_galleries.Add("Scraps Only", 2);

	m_ratings.selection(0);
	m_galleries.selection(0);

	m_status.Attach(CreateWindow(STATUSCLASSNAME, L"", WS_CHILD | WS_VISIBLE,
		0, 0, 0, 10, hDlg, NULL, GetModuleHandle(NULL), NULL));

	m_status.SetParts({ 120, -1 });
	m_status.SetText(0, L"Default Profile");
	m_status.SetText(1, L"Idle");

	HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TAILS));
	SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	DestroyIcon(hIcon);

	return TRUE;
}

INT_PTR CMainDialog::OnDLStart(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto dest = OpenSaveDialog();

	if (dest.empty())
		return FALSE;

	auto username = m_username.text();

	auto ratings = m_ratings.selection();
	auto galleries = m_ratings.selection();

	dest += std::wstring(L"\\") + username;
	CreateDirectory(dest.c_str(), NULL);

	pDumper = new CFADumper(api,
		libdaemon::Utils::WstringToAnsi(username), dest,
		(faRatingFlags)ratings, (faGalleryFlags)galleries, m_images, m_status, m_progress);

	m_downloadThread = std::thread([&]() {pDumper->Download(); });

	return TRUE;
}

INT_PTR CMainDialog::OnDLStop(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TerminateThread(m_downloadThread.native_handle(), 0);

	if (pDumper)
		delete pDumper;

	return TRUE;
}

INT_PTR CMainDialog::OnDLPause(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	SuspendThread(m_downloadThread.native_handle());

	return TRUE;
}

INT_PTR CMainDialog::OnDLResume(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	ResumeThread(m_downloadThread.native_handle());

	return TRUE;
}

INT_PTR CMainDialog::OnChangeAPI(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CAPIMenu menu(api);
	return menu.RunModal(m_hwnd);
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

CAPIMenu::CAPIMenu(std::string& api) : Dialog(IDD_APIMENU), m_ret(api)
{
	_messages[WM_INITDIALOG] = static_cast<Dialog::fnDlgProc>(&CAPIMenu::OnInit);

	_events[IDC_API_OK] = static_cast<Dialog::fnDlgProc>(&CAPIMenu::OnOk);
	_events[IDC_API_CANCEL] = static_cast<Dialog::fnDlgProc>(&CAPIMenu::OnCancel);
}

INT_PTR CAPIMenu::OnInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Dialog::OnInit(hDlg, message, wParam, lParam);

	SetWindowText(m_hwnd, L"Enter API Address...");

	m_api.Attach(m_hwnd, IDC_APIURL);

	m_api.text(libdaemon::Utils::AnsiToWstring(m_ret));

	return TRUE;
}

INT_PTR CAPIMenu::OnOk(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto input = m_api.text();

	// regex pattern
	std::string pattern = R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)";
	std::regex url_regex(pattern, std::regex::extended);

	bool validurl = std::regex_match(libdaemon::Utils::WstringToAnsi(input), url_regex);

	if (validurl)
	{
		CURL* pCurl = curl_easy_init();

		curl_easy_setopt(pCurl, CURLOPT_URL, libdaemon::Utils::WstringToAnsi(input).c_str());
		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10);
		curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1);

		auto res = curl_easy_perform(pCurl);

		std::string err = curl_easy_strerror(res);

		curl_easy_cleanup(pCurl);

		if (res != CURLE_OK)
		{
			std::wstring msg = L"Connection error! " + libdaemon::Utils::AnsiToWstring(err) + L", continue anyway?";
			if (Message::ShowQuestion(m_hwnd, msg, L"Connyection Wawnying UwU")) {
				//set
				m_ret.assign(libdaemon::Utils::WstringToAnsi(m_api.text()));
				return Dialog::OnClose(hDlg, message, wParam, lParam);
			}
		}
		else
		{
			//set
			m_ret.assign(libdaemon::Utils::WstringToAnsi(m_api.text()));
			return Dialog::OnClose(hDlg, message, wParam, lParam);
		}
	}
	else
	{
		Message::ShowError(m_hwnd, L"Specified URL is invalid!");
	}

	return TRUE;
}

INT_PTR CAPIMenu::OnCancel(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return Dialog::OnClose(hDlg, message, wParam, lParam);
}