#pragma once

#include "fDumper.h"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include <shellapi.h>

class CWatcher
{
public:
	CWatcher(const std::wstring& folder);
	~CWatcher();

	int WatchForUpdates(int updateinterval);
	void StopWatching();

	int AddToWatchList(const std::string& artist);
	int RemoveFromWatchList(const std::string& artist);

private:
	int SetupWatchList(const std::wstring& folder);

private:
	rapidjson::Document m_watchJson;
	std::list<std::string> m_watchList;
	NOTIFYICONDATA m_iconData;
	std::thread m_watchThread;
	const std::wstring& m_watchFolder;
};