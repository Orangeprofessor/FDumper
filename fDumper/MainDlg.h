#pragma once

#include "resource.h"
#include "control/Dialog.hpp"
#include "control/ComboBox.hpp"
#include "control/EditBox.hpp"
#include "control/Button.hpp"
#include "control/ListView.hpp"
#include "control/Message.hpp"

#include "configmgr.h"

#include "ctpl_stl.hpp"


class MainDlg : public Dialog
{
public:
	MainDlg();

	static MainDlg* getInstance() {
		return m_pDlg.operator MainDlg *();
	}

private:
	MSG_HANDLER(OnInit);
	MSG_HANDLER(OnClose);

	MSG_HANDLER(OnUserSubmit);
	MSG_HANDLER(OnAddedToQueue);
	MSG_HANDLER(OnSettings);
	MSG_HANDLER(OnRClickQueueItem);

	MSG_HANDLER(OnLoadThumbnails);
	MSG_HANDLER(OnQueueCustomDraw);
	MSG_HANDLER(OnCustomColorChange);


	LRESULT ProcessCustomDraw(LPARAM lParam);

	ConfigMgr m_config;

	ctrl::ListView m_gallery;
	HIMAGELIST m_imageList;

public:
	ctrl::ListView m_downloadedList;
	ctrl::ListView m_userQueue;

	ctrl::Button m_filterMain;
	ctrl::Button m_filterScraps;
	ctrl::Button m_filterFavs;

	ctrl::Button m_allRatings;
	ctrl::Button m_sfwOnly;
	ctrl::Button m_nsfwOnly;

	ctrl::ComboBox m_ratings;
	ctrl::ComboBox m_galleries;

	ctrl::EditBox m_username;
	ctrl::Button m_addtoqueue;

private:

	ctpl::thread_pool m_dumperPool;

	std::vector<cColors> m_itemColors;

	static ThreadLock<MainDlg*> m_pDlg;
};

class CEditSavesDlg : public Dialog
{
public:
	CEditSavesDlg(ConfigMgr& config);

private:
	MSG_HANDLER(OnInit);

	MSG_HANDLER(OnCloseBtn);
	MSG_HANDLER(OnOkBtn);
	MSG_HANDLER(OnBrowseBtn);

	std::wstring m_newpath;
	bool m_bAsksave;
	ConfigMgr& m_config;

private:
	ctrl::EditBox m_currentDir;
	ctrl::Button m_browse;
	ctrl::Button m_alwaysask;
	ctrl::Button m_ok;
	ctrl::Button m_cancel;
};