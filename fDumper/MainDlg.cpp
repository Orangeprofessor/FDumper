#include "pch.h"

#include "FADumper.h"

#include "MainDlg.h"
#include <gdiplus.h>
#include <ShlObj.h>

#include "FASubmission.h"

ThreadLock<MainDlg*> MainDlg::m_pDlg;

MainDlg::MainDlg() : Dialog(IDD_MAIN), m_dumperPool(std::thread::hardware_concurrency())
{
	m_messages[WM_INITDIALOG] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnInit);
	m_messages[WM_CLOSE] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnClose);

	m_messages[MSG_SETCOLOR] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnCustomColorChange);
	m_messages[MSG_LOADTHUMBNAILS] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnLoadThumbnails);
	m_messages[MSG_ADDTODOWNLOADLIST] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnAddToDownloadList);

	m_events[IDC_ADDTOQUEUE] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnUserSubmit);
	m_events[ID_SETTINGS_DOWNLOADS] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnSettings);


	m_notifs[std::make_pair(NM_RCLICK, IDC_QUEUE)] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnRClickQueueItem);
	m_notifs[std::make_pair(NM_CLICK, IDC_QUEUE)] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnLClickQueueItem);
	m_notifs[std::make_pair(LVN_INSERTITEM, IDC_QUEUE)] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnAddedToQueue);
	m_notifs[std::make_pair(NM_CUSTOMDRAW, IDC_QUEUE)] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnQueueCustomDraw);

	m_pDlg.operator=(this);
}

INT_PTR MainDlg::OnInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

INT_PTR MainDlg::OnClose(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// save wdir cfg
	m_config.Save();
	return Dialog::OnClose(hDlg, message, wParam, lParam);
}

INT_PTR MainDlg::OnUserSubmit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto username = m_username.text();

	if (username.empty())
		return FALSE;

	m_userQueue.AddItem(username, 0, { L"Pending", L"0", L"0" });

	return TRUE;
}

INT_PTR MainDlg::OnAddedToQueue(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const auto item = (LPNMLISTVIEW)lParam;
	m_itemColors.push_back({ RGB(255, 150, 0), item->iItem });

	auto dumper = [&](int threadid, int item)
	{
		DownloadContext ctx;
		ctx.username = WstringToAnsi(m_userQueue.itemText(item));
		ctx.gallery |= faGalleryFlags::MAIN;
		ctx.ratings = faRatingFlags::SFW_ONLY;
		const_cast<std::wstring&>(ctx.path) = m_config.config().defaultDir + L"\\" + AnsiToWstring(ctx.username);

		CFADumper dumper(m_config, item);
		dumper.Action(ctx);
	};

	m_dumperPool.push(dumper, item->iItem);

	return TRUE;
}

INT_PTR MainDlg::OnCustomColorChange(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto clrs = (cColors*)lParam;

	auto it = std::find_if(m_itemColors.begin(), m_itemColors.end(), [&](cColors clr) {return clrs->id == clr.id; });
	if (it == std::end(m_itemColors))
		return FALSE;

	it->cf = clrs->cf;

	ListView_RedrawItems(m_userQueue.hwnd(), clrs->id, clrs->id);

	return TRUE;
}

INT_PTR MainDlg::OnLoadThumbnails(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto thumbdata = (thumbnailData*)lParam;
	
	namespace fs = std::filesystem;
	using namespace Gdiplus;

	std::vector<fs::directory_entry> thumblist; 
	for (auto& file : fs::directory_iterator(thumbdata->workingdir))
	{
		if (!file.is_regular_file())
			continue;

		auto filename = file.path().filename().wstring();
		auto ext = filename.substr(filename.find_last_of(L".") + 1);

		if (!(!ext.compare(L"png") || !ext.compare(L"jpg"))) // make exception for items that cant be displayed in thumbnail form
			continue;

		thumblist.push_back(file);
	}
	
	ImageList_RemoveAll(m_imageList);

	ListView_DeleteAllItems(m_gallery.hwnd());
	ImageList_SetImageCount(m_imageList, thumblist.size());

	SendMessage(m_gallery.hwnd(), WM_SETREDRAW, FALSE, 0);

	auto fadatavec = thumbdata->data;

	for (auto& thumbnail : thumblist)
	{
		auto filename = thumbnail.path().filename().wstring();
		int index = m_gallery.AddImage(L"bluee is a bottom");

		auto it = std::find_if(fadatavec.begin(), fadatavec.end(), [&](faData dat) 
		{ 
			std::string s = dat.thumbnailURL.substr(dat.thumbnailURL.find_last_of("/") + 1);
			return filename.compare(AnsiToWstring(s)) == 0;
		});

		if (it == fadatavec.end())
			continue;

		std::wstring title = AnsiToWstring(it->title);

		ListView_SetItemText(m_gallery.hwnd(), index, NULL, (LPWSTR)title.c_str());

		HBITMAP hBitmap;
		std::unique_ptr<Bitmap> pbmPhoto;
		std::unique_ptr<Graphics> pgrPhoto;

		auto path = thumbnail.path().wstring();

		Bitmap image(path.c_str());

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

		pbmPhoto->GetHBITMAP(colorW, &hBitmap);

		auto result = ImageList_Replace(m_imageList, index, hBitmap, NULL);

		DeleteObject(hBitmap);
	}

	SendMessage(m_gallery.hwnd(), WM_SETREDRAW, TRUE, 0);

	return TRUE;
}

INT_PTR MainDlg::OnSettings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CEditSavesDlg settings(m_config);
	return settings.RunModal(hDlg);
}

INT_PTR MainDlg::OnRClickQueueItem(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto lpnmitem = (LPNMITEMACTIVATE)lParam;

	auto topmenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_QUEUE_MENU));
	auto menu = GetSubMenu(topmenu, 0);

	POINT cursorpos;
	GetCursorPos(&cursorpos);

	TrackPopupMenu(menu, TPM_LEFTALIGN, cursorpos.x, cursorpos.y, NULL, hDlg, nullptr);

	return TRUE;
}

INT_PTR MainDlg::OnLClickQueueItem(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto lpnmitem = (LPNMITEMACTIVATE)lParam;

	// switch context of all items in the prview gallery & download list

	auto vec = m_downloadListData.GetType()[lpnmitem->iItem];

	for (auto submission : vec)
	{
		auto title = AnsiToWstring(submission.GetSubmissionTitle());
		auto date = AnsiToWstring(submission.GetDatePosted());
		auto res = AnsiToWstring(submission.GetResolution());

		m_downloadedList.AddItem(title, NULL, { date, res, std::to_wstring(submission.GetSubmissionID()) });
	}

	return TRUE;
}

INT_PTR MainDlg::OnQueueCustomDraw(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG)ProcessCustomDraw(lParam));
	return TRUE;
}

INT_PTR MainDlg::OnAddToDownloadList(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto itemdata = (downloadListData*)lParam;



	return TRUE;
}

LRESULT MainDlg::ProcessCustomDraw(LPARAM lParam)
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
	cfg.askforsave = m_bAsksave;
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
