#include "pch.h"

#include "CBaseDumper.h"
#include <ShlObj.h>


void CBaseDumper::Main(const int argc, wchar_t* argv[])
{
	if (argc < 2)
	{
		PrintDescription();
		return;
	}

	arg_t arg = { argc, argv, 1 };
	if (!ReadArgs(arg))
	{
		log_console(xlog::LogLevel::error, "Invalid arguments!\n");
		PrintDescription();
		return;
	}

	// Implement retrying here

	CURL* pCurl = curl_easy_init();

	curl_easy_setopt(pCurl, CURLOPT_URL, m_api.c_str());
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10);
	curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1);

	auto res = curl_easy_perform(pCurl);

	std::string err = curl_easy_strerror(res);

	curl_easy_cleanup(pCurl);

	if (res != CURLE_OK) {
		log_console(xlog::LogLevel::error, "Connection error! %s, aborting...\n", err.c_str());
		return;
	}

	Process(arg);
}

void CBaseDumper::Process(arg_t& arg)
{
	wchar_t szDir[MAX_PATH];
	BROWSEINFO bInfo;
	bInfo.hwndOwner = GetActiveWindow();
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
	bInfo.lpszTitle = L"Pwease sewect a fowdew UwU"; // Title of the dialog
	bInfo.ulFlags = 0;
	bInfo.lpfn = NULL;
	bInfo.lParam = 0;
	bInfo.iImage = -1;

	LPITEMIDLIST lpItem = SHBrowseForFolderW(&bInfo);

	SHGetPathFromIDListW(lpItem, szDir);

	const_cast<std::wstring&>(m_path) = szDir;

	if (m_path.empty())
		return;

	Action(arg);
}

void CBaseDumper::PrintDescription()
{
	log_console(xlog::LogLevel::normal, "Other settings:\n"
		" --api-address		Custom API server to use (default is faexport.boothale.net)\n\n");
}

bool CBaseDumper::ReadArgs(arg_t& arg)
{
	m_api = "https://faexport.boothale.net";

	// primitive check for double dash
	while (arg.i < arg.c && *(arg.v[arg.i]) == '-' && *(arg.v[arg.i] + 1) == '-')
	{
		if (!Argument(arg))
		{
			log_console(xlog::LogLevel::error, "Unknown argument %s!\n", WstringToAnsi(arg.v[arg.i]).c_str());
			return false;
		}
	}
	return true;
}

bool CBaseDumper::ValidUser(std::string user)
{
	char urlbuff[200];
	sprintf_s(urlbuff, "%s/user/%s.json", m_api.c_str(), user.c_str());

	int httpcode; std::string temp;
	CURL* pCurl = curl_easy_init();

	curl_easy_setopt(pCurl, CURLOPT_URL, urlbuff);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writebuffercallback);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &temp);
	CURLcode code = curl_easy_perform(pCurl);

	if (code == CURLE_OK)
		curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &httpcode);
	else
		return curl_easy_cleanup(pCurl), false;

	curl_easy_cleanup(pCurl);

	if (httpcode != 200)
		return false;

	return true;
}


bool CBaseDumper::Argument(arg_t& arg)
{
	std::wstring s = arg.v[arg.i];
	if (!s.compare(L"--api-address"))
	{
		++arg.i;
		m_api = WstringToAnsi(arg.v[arg.i]);
		return ++arg.i <= arg.c;
	}
	else if (!s.compare(L"--debug"))
	{
		++arg.i;
		return true;
	}
	return false;
}