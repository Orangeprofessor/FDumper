#pragma once

#include "MainDlg.h"

class UpdateDlg : public Dialog
{
public:
	UpdateDlg();

private:
	MSG_HANDLER(OnInit);
	MSG_HANDLER(OnClose);

	ctrl::ListView m_userlist;

};