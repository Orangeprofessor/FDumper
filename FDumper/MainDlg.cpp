#include "pch.h"

#include "FADumper.h"

#include "MainDlg.h"
#include "UpdateDlg.h"
#include <gdiplus.h>
#include <ShlObj.h>

#include "FASubmission.h"

ThreadLock<CMainDlg*> CMainDlg::m_pDlg;

CMainDlg::CMainDlg() : Dialog(IDD_MAIN), m_dumperPool(std::thread::hardware_concurrency())
{
	m_messages[WM_INITDIALOG] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnInit);
	m_messages[WM_CLOSE] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnClose);

	m_messages[MSG_SETCOLOR] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnCustomColorChange);
	m_messages[MSG_LOADTHUMBNAIL] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnLoadThumbnail);
	m_messages[MSG_ADDTODOWNLOADLIST] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnAddToDownloadList);

	m_events[IDC_ADDTOQUEUE] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnUserSubmit);
	m_events[ID_SETTINGS_DOWNLOADS] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnSettings);
	m_events[ID_SETTINGS_APIADDRESS] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnChangeAPI);
	m_events[ID_SETTINGS_UPDATEGALLERIES] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnUpdate);
	m_events[ID_CTX_PAUSE] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnPause);
	m_events[ID_CTX_RESUME] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnResume);
	m_events[ID_CTX_CANCEL] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnCancel);

	m_notifs[std::make_pair(NM_RCLICK, IDC_QUEUE)] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnRClickQueueItem);
	m_notifs[std::make_pair(NM_CLICK, IDC_QUEUE)] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnLClickQueueItem);
	m_notifs[std::make_pair(NM_DBLCLK, IDC_GALLERYLIST)] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnLClickPreviewItem);
	m_notifs[std::make_pair(NM_DBLCLK, IDC_IMAGELIST)] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnLClickDownloadListItem);
	m_notifs[std::make_pair(LVN_INSERTITEM, IDC_QUEUE)] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnAddedToQueue);
	m_notifs[std::make_pair(NM_CUSTOMDRAW, IDC_QUEUE)] = static_cast<Dialog::fnDlgProc>(&CMainDlg::OnQueueCustomDraw);

	m_pDlg.operator=(this);
}

INT_PTR CMainDlg::OnInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Dialog::OnInit(hDlg, message, wParam, lParam);

	m_addtoqueue.Attach(hDlg, IDC_ADDTOQUEUE);
	HICON plusicon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_PLUS));
	SendMessage(m_addtoqueue.hwnd(), BM_SETIMAGE, IMAGE_ICON, (LPARAM)plusicon);
	DestroyIcon(plusicon);

	m_username.Attach(hDlg, IDC_USERNAME);

	m_gallery.Attach(hDlg, IDC_GALLERYLIST);

	m_imageList = ImageList_Create(100, 90, ILC_COLOR32, 0, 1);
	ListView_SetImageList(m_gallery.hwnd(), m_imageList, LVSIL_NORMAL);
	ImageList_SetImageCount(m_imageList, 1);

	m_downloadedList.Attach(hDlg, IDC_IMAGELIST);

	m_downloadedList.AddColumn(L"Name", 150, 0);
	m_downloadedList.AddColumn(L"Date Posted", 90, 1);
	m_downloadedList.AddColumn(L"Resolution", 90, 2);
	m_downloadedList.AddColumn(L"ID", 90, 3);

	ListView_SetExtendedListViewStyle(m_downloadedList.hwnd(), LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	m_userQueue.Attach(hDlg, IDC_QUEUE);

	m_userQueue.AddColumn(L"Username", 75, 0);
	m_userQueue.AddColumn(L"Status", 110, 1);
	m_userQueue.AddColumn(L"Total Images", 75, 2);
	m_userQueue.AddColumn(L"Progress", 70, 3);

	ListView_SetExtendedListViewStyle(m_userQueue.hwnd(), LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	m_filterMain.Attach(hDlg, IDC_FILTER_MAIN);
	m_filterScraps.Attach(hDlg, IDC_FILTER_SCRAPS);
	m_filterFavs.Attach(hDlg, IDC_FILTER_FAVS);

	m_allRatings.Attach(hDlg, IDC_RATINGS_ALL);
	m_sfwOnly.Attach(hDlg, IDC_RATINGS_SFW);
	m_nsfwOnly.Attach(hDlg, IDC_RATINGS_NSFW);

	m_allRatings.checked(true);
	m_filterMain.checked(true);

	m_config.Load();
	m_config.config().apiaddress = "http://faexport.boothale.net";


	return TRUE;
}

INT_PTR CMainDlg::OnClose(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// save wdir cfg
	m_config.Save();
	return Dialog::OnClose(hDlg, message, wParam, lParam);
}

INT_PTR CMainDlg::OnUserSubmit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto username = m_username.text();

	if (username.empty())
		return FALSE;
	
	int idx = m_userQueue.AddItem(username, 0, { L"Pending", L"0", L"0" });
	if (idx == 0)
		m_selectedIdx = idx;

	return TRUE;
}

INT_PTR CMainDlg::OnAddedToQueue(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const auto item = (LPNMLISTVIEW)lParam;
	m_itemColors.push_back({ RGB(255, 150, 0), item->iItem });

	if (m_config.config().askforsave)
	{
		wchar_t szDir[MAX_PATH];
		BROWSEINFO bInfo;
		bInfo.hwndOwner = hDlg;
		bInfo.pidlRoot = NULL;
		bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
		bInfo.lpszTitle = L"Pwease sewect a fowdew UwU"; // Title of the dialog
		bInfo.ulFlags = 0;
		bInfo.lpfn = NULL;
		bInfo.lParam = 0;
		bInfo.iImage = -1;

		LPITEMIDLIST lpItem = SHBrowseForFolderW(&bInfo);

		SHGetPathFromIDListW(lpItem, szDir);

		std::wstring folder(szDir);

		if (folder.empty())
		{
			m_userQueue.RemoveItem(item->iItem);
			return FALSE;
		}	
	}

	faRatingFlags rating;

	if (m_allRatings.checked())
		rating = faRatingFlags::ALL_RATINGS;
	else if (m_sfwOnly.checked())
		rating = faRatingFlags::SFW_ONLY;
	else if (m_nsfwOnly.checked())
		rating = faRatingFlags::NSFW_ONLY;

	int gallery = 0;

	if (m_filterMain.checked())
		gallery |= faGalleryFlags::MAIN;
	if (m_filterScraps.checked())
		gallery |= faGalleryFlags::SCRAPS;
	if (m_filterFavs.checked())
		gallery |= faGalleryFlags::FAVORITES;


	if (gallery == 0)
	{
		Message::ShowError(m_hwnd, L"Please select a gallery");
		return FALSE;
	}

	auto dumper = [&](int threadid, int item, faRatingFlags rating, int gallery)
	{
		DownloadContext ctx;
		ctx.username = WstringToAnsi(m_userQueue.itemText(item));
		ctx.gallery = gallery;
		ctx.ratings = rating;
		const_cast<std::wstring&>(ctx.path) = m_config.config().defaultDir + L"\\" + AnsiToWstring(ctx.username);

		CFADumper dumper(m_config, item);
		dumper.Action(ctx);
	};

	m_dumperPool.push(dumper, item->iItem, rating, gallery);

	return TRUE;
}

INT_PTR CMainDlg::OnCustomColorChange(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto clrs = (cColors*)lParam;

	auto it = std::find_if(m_itemColors.begin(), m_itemColors.end(), [&](cColors clr) {return clrs->id == clr.id; });
	if (it == std::end(m_itemColors))
		return FALSE;

	it->cf = clrs->cf;

	ListView_RedrawItems(m_userQueue.hwnd(), clrs->id, clrs->id);

	return TRUE;
}

INT_PTR CMainDlg::OnLoadThumbnail(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto thumbdata = (thumbnailData*)lParam;

	namespace fs = std::filesystem;
	using namespace Gdiplus;

	faData dat = thumbdata->data; dat.path = thumbdata->path;
	m_previewData[thumbdata->index].push_back(dat);

	if (m_selectedIdx != thumbdata->index)
		return FALSE;

	SendMessage(m_gallery.hwnd(), WM_SETREDRAW, FALSE, 0);

	fs::directory_entry file(thumbdata->path);

	int index = m_gallery.AddImage(L"bluee is a bottom");
	ImageList_SetImageCount(m_imageList, ImageList_GetImageCount(m_imageList) + index);

	std::wstring title = AnsiToWstring(thumbdata->data.title);

	ListView_SetItemText(m_gallery.hwnd(), index, NULL, (LPWSTR)title.c_str());

	std::unique_ptr<Bitmap> pbmPhoto;
	std::unique_ptr<Graphics> pgrPhoto;
	std::unique_ptr<void, decltype(&DeleteObject)> hBitmap(nullptr, DeleteObject);

	Bitmap image(thumbdata->path.c_str());

	const int thumbWidth = 100;
	const int thumbHeight = 90;

	int sourceWidth = image.GetWidth();
	int sourceHeight = image.GetHeight();

	int destX = 0,
		destY = 0;

	float nPercent = 0;
	float nPercentW = ((float)thumbWidth / (float)sourceWidth);
	float nPercentH = ((float)thumbHeight / (float)sourceHeight);

	if (nPercentH < nPercentW)
	{
		nPercent = nPercentH;
		destX = (int)((thumbWidth - (sourceWidth * nPercent)) / 2);
	}
	else
	{
		nPercent = nPercentW;
		destY = (int)((thumbHeight - (sourceHeight * nPercent)) / 2);
	}

	int destWidth = (int)(sourceWidth * nPercent);
	int destHeight = (int)(sourceHeight * nPercent);

	pbmPhoto = std::make_unique<Bitmap>(sourceWidth, sourceHeight, PixelFormat24bppRGB);
	pbmPhoto->SetResolution(image.GetHorizontalResolution(), image.GetVerticalResolution());

	pgrPhoto.reset(Graphics::FromImage(pbmPhoto.get()));

	Color colorW(255, 255, 255, 255);
	pgrPhoto->Clear(colorW);
	pgrPhoto->SetInterpolationMode(InterpolationModeHighQualityBicubic);
	pgrPhoto->DrawImage(&image, Rect(destX, destY, destWidth, destHeight));

	auto ref = hBitmap.get();
	pbmPhoto->GetHBITMAP(colorW, (HBITMAP*)&ref);

	auto result = ImageList_Replace(m_imageList, index, (HBITMAP)hBitmap.get(), NULL);

	SendMessage(m_gallery.hwnd(), WM_SETREDRAW, TRUE, 0);

	return TRUE;
}

INT_PTR CMainDlg::OnSettings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CEditSavesDlg settings(m_config);
	return settings.RunModal(hDlg);
}

INT_PTR CMainDlg::OnChangeAPI(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CAPIDlg changeapi(m_config);
	return changeapi.RunModal(hDlg);
}

INT_PTR CMainDlg::OnUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CUpdateDlg update(m_config);
	return update.RunModal(hDlg);
}

INT_PTR CMainDlg::OnRClickQueueItem(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto lpnmitem = (LPNMITEMACTIVATE)lParam;

	m_clickedIdx = lpnmitem->iItem;

	if (m_clickedIdx == -1)
		return FALSE;

	auto topmenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_QUEUE_MENU));
	auto menu = GetSubMenu(topmenu, 0);

	POINT cursorpos;
	GetCursorPos(&cursorpos);

	TrackPopupMenu(menu, TPM_LEFTALIGN, cursorpos.x, cursorpos.y, NULL, hDlg, nullptr);

	return TRUE;
}

INT_PTR CMainDlg::OnLClickQueueItem(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto lpnmitem = (LPNMITEMACTIVATE)lParam;

	if (lpnmitem->iItem == m_selectedIdx)
		return FALSE;

	// switch context of all items in the prview gallery & download list
	m_downloadedList.disable();

	m_downloadedList.reset();

	m_selectedIdx = lpnmitem->iItem;

	auto vec = m_downloadListData[lpnmitem->iItem];

	for (auto submission : vec)
	{
		auto title = AnsiToWstring(submission.GetSubmissionTitle());
		auto date = AnsiToWstring(submission.GetDatePosted());
		auto res = AnsiToWstring(submission.GetResolution());

		m_downloadedList.AddItem(title, NULL, { date, res, std::to_wstring(submission.GetSubmissionID()) });
	}

	m_downloadedList.enable();

	return TRUE;
}

INT_PTR CMainDlg::OnLClickPreviewItem(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto lpnmitem = (LPNMITEMACTIVATE)lParam;

	auto vec = m_previewData[m_selectedIdx];

	auto it = std::find_if(vec.begin(), vec.end(), [&](faData data) {
		auto cm = m_galleries.itemText(lpnmitem->iItem);
		return AnsiToWstring(data.title).compare(cm) != 0;
	});

	if (it == std::end(vec))
		return FALSE;

	ShellExecute(m_hwnd, L"open", it->path.c_str(), NULL, NULL, SW_SHOW);

	return TRUE;
}

INT_PTR CMainDlg::OnLClickDownloadListItem(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto lpnmitem = (LPNMITEMACTIVATE)lParam;

	LVFINDINFO info{};
	info.flags = LVFI_STRING;
	auto str = m_downloadedList.itemText(lpnmitem->iItem);
	info.psz = str.c_str();
	int pos = ListView_FindItem(m_gallery.hwnd(), -1, &info);

	if (pos == -1)
		return FALSE;

	ListView_EnsureVisible(m_gallery.hwnd(), pos, FALSE);

	return TRUE;
}

INT_PTR CMainDlg::OnPause(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_selectedIdx == -1)
		return FALSE;

	cColors clrs{ RGB(255, 150, 0), m_selectedIdx };
	SendMessage(m_hwnd, MSG_SETCOLOR, NULL, (LPARAM)&clrs);

	m_userQueue.setText(L"Paused", m_selectedIdx, 1);

	auto& thread = m_dumperPool.get_thread(m_selectedIdx);

	SuspendThread(thread.native_handle());

	return TRUE;
}

INT_PTR CMainDlg::OnResume(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_selectedIdx == -1)
		return FALSE;

	auto& thread = m_dumperPool.get_thread(m_selectedIdx);

	ResumeThread(thread.native_handle());

	return TRUE;
}

INT_PTR CMainDlg::OnCancel(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

INT_PTR CMainDlg::OnQueueCustomDraw(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG)ProcessCustomDraw(lParam));
	return TRUE;
}

INT_PTR CMainDlg::OnAddToDownloadList(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto itemdata = (downloadListData*)lParam;

	if (!m_downloadedList.enabled())
		return FALSE;

	m_downloadListData[itemdata->index].push_back(itemdata->submission);

	if (m_selectedIdx == itemdata->index)
	{
		auto submission = itemdata->submission;

		auto title = AnsiToWstring(submission.GetSubmissionTitle());
		auto date = AnsiToWstring(submission.GetDatePosted());
		auto res = AnsiToWstring(submission.GetResolution());

		m_downloadedList.AddItem(title, NULL, { date, res, std::to_wstring(submission.GetSubmissionID()) });
	}

	return TRUE;
}

LRESULT CMainDlg::ProcessCustomDraw(LPARAM lParam)
{
	auto lplvcd = (LPNMLVCUSTOMDRAW)lParam;

	switch (lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT:
	{
		auto getrowcolor = [&](int id) -> int
		{
			auto it = std::find_if(m_itemColors.begin(), m_itemColors.end(), [&](cColors clr) {return id == clr.id; });
			if (it != std::end(m_itemColors)) {
				return std::distance(m_itemColors.begin(), it);
			}
			return -1;
		};

		int rowindex = getrowcolor(lplvcd->nmcd.dwItemSpec);
		if (rowindex != -1)
		{
			lplvcd->clrTextBk = m_itemColors.at(rowindex).cf;
			return CDRF_NEWFONT;
		}
		else
		{
			lplvcd->clrTextBk = GetBkColor(lplvcd->nmcd.hdc);
			return CDRF_NEWFONT;
		}
	}
	break;

	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
	{
		auto getrowcolor = [&](int id) -> int
		{
			auto it = std::find_if(m_itemColors.begin(), m_itemColors.end(), [&](cColors clr) {return id == clr.id; });
			if (it != std::end(m_itemColors)) {
				return std::distance(m_itemColors.begin(), it);
			}
			return -1;
		};

		int rowindex = getrowcolor(lplvcd->nmcd.dwItemSpec);
		if (rowindex != -1)
		{
			lplvcd->clrTextBk = m_itemColors.at(rowindex).cf;
			return CDRF_NEWFONT;
		}
		else
		{
			lplvcd->clrTextBk = GetBkColor(lplvcd->nmcd.hdc);
			return CDRF_NEWFONT;
		}
	}
	}

	return CDRF_DODEFAULT;
}

CEditSavesDlg::CEditSavesDlg(ConfigMgr& config) : Dialog(IDD_DOWNLOADS), m_config(config)
{
	m_messages[WM_INITDIALOG] = static_cast<Dialog::fnDlgProc>(&CEditSavesDlg::OnInit);

	m_events[IDC_DLSET_ASKSAVE] = static_cast<Dialog::fnDlgProc>(&CEditSavesDlg::OnAskSaveChecked);
	m_events[IDC_DLSET_CANCEL] = static_cast<Dialog::fnDlgProc>(&CEditSavesDlg::OnCloseBtn);
	m_events[IDC_DLSET_OK] = static_cast<Dialog::fnDlgProc>(&CEditSavesDlg::OnOkBtn);
	m_events[IDC_DLSET_BROWSE] = static_cast<Dialog::fnDlgProc>(&CEditSavesDlg::OnBrowseBtn);
}

INT_PTR CEditSavesDlg::OnInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Dialog::OnInit(hDlg, message, wParam, lParam);

	m_currentDir.Attach(hDlg, IDC_DLSET_SAVEPATH);
	m_browse.Attach(hDlg, IDC_DLSET_BROWSE);
	m_alwaysask.Attach(hDlg, IDC_DLSET_ASKSAVE);
	m_ok.Attach(hDlg, IDC_DLSET_OK);
	m_cancel.Attach(hDlg, IDC_DLSET_CANCEL);

	auto cfg = m_config.config();
	m_newpath = cfg.defaultDir;
	m_bAsksave = cfg.askforsave;

	m_currentDir.text(m_newpath);
	m_alwaysask.checked(m_bAsksave);

	if (m_bAsksave) {
		m_currentDir.disable();
		m_browse.disable();
	}
		
	return TRUE;
}


INT_PTR CEditSavesDlg::OnCloseBtn(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return Dialog::OnClose(hDlg, message, wParam, lParam);
}

INT_PTR CEditSavesDlg::OnOkBtn(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// save to cfg
	auto& cfg = m_config.config();
	cfg.defaultDir = m_newpath;
	cfg.askforsave = m_alwaysask.checked();
	return Dialog::OnClose(hDlg, message, wParam, lParam);
}

INT_PTR CEditSavesDlg::OnBrowseBtn(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	wchar_t szDir[MAX_PATH];
	BROWSEINFO bInfo;
	bInfo.hwndOwner = hDlg;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
	bInfo.lpszTitle = L"Pwease sewect a fowdew UwU"; // Title of the dialog
	bInfo.ulFlags = 0;
	bInfo.lpfn = NULL;
	bInfo.lParam = 0;
	bInfo.iImage = -1;

	LPITEMIDLIST lpItem = SHBrowseForFolderW(&bInfo);

	SHGetPathFromIDListW(lpItem, szDir);

	std::wstring folder(szDir);

	if (folder.empty())
		return FALSE;

	m_newpath = folder;
	m_currentDir.text(m_newpath);

	return TRUE;
}

INT_PTR CEditSavesDlg::OnAskSaveChecked(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	EnableWindow(m_currentDir.hwnd(), !m_alwaysask.checked());
	EnableWindow(m_browse.hwnd(), !m_alwaysask.checked());

	return TRUE;
}

CAPIDlg::CAPIDlg(ConfigMgr & config) : Dialog(IDD_API), m_config(config)
{
	m_messages[WM_INITDIALOG] = static_cast<Dialog::fnDlgProc>(&CAPIDlg::OnInit);

	m_events[IDC_API_OK] = static_cast<Dialog::fnDlgProc>(&CAPIDlg::OnOkBtn);
	m_events[IDC_API_CANCEL] = static_cast<Dialog::fnDlgProc>(&CAPIDlg::OnCloseBtn);
}

INT_PTR CAPIDlg::OnInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Dialog::OnInit(hDlg, message, wParam, lParam);

	m_webaddress.Attach(hDlg, IDC_API_ADDRESS);
	m_ok.Attach(hDlg, IDC_API_OK);
	m_cancel.Attach(hDlg, IDC_API_CANCEL);

	auto cfg = m_config.config();
	m_api = cfg.apiaddress;

	m_webaddress.text(AnsiToWstring(m_api));

	return TRUE;
}

INT_PTR CAPIDlg::OnCloseBtn(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return Dialog::OnClose(hDlg, message, wParam, lParam);
}

INT_PTR CAPIDlg::OnOkBtn(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// test address

	auto address = WstringToAnsi(m_webaddress.text());
	
	if (address.empty())
	{
		//do something here
		Message::ShowError(m_hwnd, L"Type something!");
		return FALSE;
	}

	// shoutouts to gorath
	address.append("/user/gorath.json");

	std::string buffer; int httpcode;
	std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);

	curl_easy_setopt(curl.get(), CURLOPT_URL, address.c_str());
	curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writebuffercallback);
	curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &buffer);
	curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 5);
	CURLcode code = curl_easy_perform(curl.get());

	if (code != CURLE_OK)
	{
		auto err = curl_easy_strerror(code);
		std::wstring msg = L"Connection error! " + AnsiToWstring(err);
		Message::ShowError(m_hwnd, msg);

		return FALSE;
	}

	curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &httpcode);

	if (httpcode != 200)
	{
		auto err = curl_easy_strerror(code);
		std::wstring msg = L"Connection error! " + AnsiToWstring(err);
		Message::ShowError(m_hwnd, msg);
		
		return FALSE;
	}

	m_config.config().apiaddress = address;

	return Dialog::OnClose(hDlg, message, wParam, lParam);
}