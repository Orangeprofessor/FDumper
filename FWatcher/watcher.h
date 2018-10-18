#pragma once

#include "pch.h"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include <shellapi.h>

extern const std::string& g_API;

struct CArtistConfig
{
	std::string artistname;
	int rating;
	int gallery;
};

class CWatcher
{
public:
	CWatcher(const std::wstring& folder);
	~CWatcher();

	int WatchForUpdates(int updateinterval);
	void StopWatching();

	int AddToWatchList(const std::string& artist);
	int RemoveFromWatchList(const std::string& artist);
	bool SetupWatchList(const std::wstring& folder);

private:
	
private:
	bool stopWatching = false;
	rapidjson::Document m_watchJson;
	std::list<CArtistConfig> m_watchList;
	NOTIFYICONDATA m_iconData;
	const std::wstring& m_watchFolder;
};

class CArtist
{
public:
	CArtist(const CArtistConfig& config, const std::wstring& path);

	size_t GetHighestImageNumber();

	void Update(size_t highestidx);

private:


private:
	const CArtistConfig& m_config;
	const std::wstring& m_workingpath;
};