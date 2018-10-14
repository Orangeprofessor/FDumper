#pragma once

#include "resource.h"
#include <string>
#include <map>

#include "control/Dialog.hpp"
#include "control/EditBox.hpp"
#include "control/ListView.hpp"
#include "control/ComboBox.hpp"
#include "control/Button.hpp"
#include "control/StatusBar.hpp"
#include "control/ProgressBar.hpp"
#include "control/Message.hpp"

#include "ctpl_stl.hpp"

class CAPIMenu;

class CMainDialog : public Dialog
{
public:
	CMainDialog();
	~CMainDialog();
	
	MSG_HANDLER(OnInit);
	MSG_HANDLER(OnDLStart);
	MSG_HANDLER(OnDLStop);
	MSG_HANDLER(OnDLPause);
	MSG_HANDLER(OnDLResume);
	MSG_HANDLER(OnChangeAPI);

private:
	std::wstring OpenSaveDialog();

private:
	ctrl::ListView m_images;
	ctrl::ComboBox m_ratings;
	ctrl::ComboBox m_galleries;
	ctrl::ProgressBar m_progress;
	ctrl::EditBox m_username;
	ctrl::Button m_update;
	ctrl::Button m_startDL;
	ctrl::Button m_stopDL;
	ctrl::Button m_pauseDL;
	ctrl::Button m_resumeDL;
	ctrl::StatusBar m_status;

	std::string api = "faexport.boothale.net"; 

	bool createusername, creategallery;
	class CFADumper* pDumper = nullptr;
};


class CAPIMenu : public Dialog
{
public:
	CAPIMenu(std::string& api);

	MSG_HANDLER(OnInit);
	MSG_HANDLER(OnOk);
	MSG_HANDLER(OnCancel);

private:
	ctrl::EditBox m_api;
	std::string& m_ret;
};