#pragma once

#include "MainDlg.h"

class CUpdateDlg : public Dialog
{
	using BaseClass = Dialog;
public:
	CUpdateDlg(ConfigMgr& config);

private:
	MSG_HANDLER(OnInit);
	MSG_HANDLER(OnClose);





private:
	ctrl::ListView m_userlist;
	HIMAGELIST m_userpfplist;


	
	ConfigMgr& m_config;
};