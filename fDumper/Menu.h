#pragma once

#include "resource.h"
#include <string>
#include <map>

#include "control/Dialog.hpp"
#include "control/EditBox.hpp"
#include "control/ListView.hpp"
#include "control/ComboBox.hpp"
#include "control/Button.hpp"

#include "ctpl_stl.hpp"


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
	MSG_HANDLER(OnCreateUsername);
	MSG_HANDLER(OnCreateGallery);

private:
	std::wstring OpenSaveDialog();

private:
	ctrl::EditBox m_apiurl;
	ctrl::ListView m_images;
	ctrl::ComboBox m_ratings;
	ctrl::ComboBox m_galleries;
	ctrl::EditBox m_username;
	ctrl::Button m_update;
	ctrl::Button m_startDL;
	ctrl::Button m_stopDL;

	ctpl::thread_pool m_notapool;
	std::thread m_dumperThread;

	bool createusername, creategallery;
	class CFADumper* pDumper = nullptr;
};