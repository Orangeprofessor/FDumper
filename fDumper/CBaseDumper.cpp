#include "pch.h"

#include "CBaseDumper.h"
#include <ShlObj.h>

#include <regex>

void CBaseDumper::Main(const int argc, const char* argv[])
{
	if (argc < 3)
	{
		PrintDescription();
		return;
	}

	arg_t arg = { argc, argv, 2 };
	if (!ReadArgs(arg))
	{
		console_error("Invalid arguments!\n");
		PrintDescription();
		return;
	}

	// validate API
	std::string pattern = R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)";
	std::regex url_regex(pattern, std::regex::extended);

	bool validurl = std::regex_match(m_api, url_regex);

	if (!validurl)
	{
		console_error("Invalid API address: %s!", m_api.c_str());
		return;
	}

	CURL* pCurl = curl_easy_init();

	curl_easy_setopt(pCurl, CURLOPT_URL, m_api.c_str());
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10);
	curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1);

	auto res = curl_easy_perform(pCurl);

	std::string err = curl_easy_strerror(res);

	curl_easy_cleanup(pCurl);

	if (res != CURLE_OK) {
		console_error("Connection error! %s, aborting...", err.c_str());
		return;
	}

	Process(arg);
}

void CBaseDumper::Process(arg_t& arg)
{
	wchar_t szDir[MAX_PATH];
	BROWSEINFOW bInfo;
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
	console_print("--api-address (-API) custom API server to use (default is faexport.boothale.net)\n");
}

bool CBaseDumper::ReadArgs(arg_t& arg)
{
	m_api = "http://faexport.boothale.net";

	while (arg.i < arg.c && *(arg.v[arg.i]) == '-')
	{
		if (!Argument(arg))
		{
			console_error("Unkown argument %s!\n", arg.v[arg.i]);
			return false;
		}
	}
	return true;
}

bool CBaseDumper::Argument(arg_t& arg)
{
	std::string s = arg.v[arg.i];
	if (!s.compare("--api-address") || !s.compare("-API"))
	{
		++arg.i;
		m_api = arg.v[arg.i];
		return ++arg.i <= arg.c;
	}
	else if (!s.compare("--testcase"))
	{
		++arg.i;
		return true;
	}
	return false;
}