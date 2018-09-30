#include "pch.h"

#include <filesystem>
#include <libda3m0n/Misc/Utils.h>
#include "watcher.h"

CWatcher::CWatcher(const std::wstring& folder) : m_watchFolder(folder)
{
	if (SetupWatchList(folder))
	{
		
	}
}

CWatcher::~CWatcher()
{
	if (m_watchThread.joinable())
		m_watchThread.join();

}

int CWatcher::WatchForUpdates(int updateinterval)
{
	while (true)
	{
		for (auto artist : m_watchList)
		{
			std::wstring updatepath = m_watchFolder + L"\\" + libdaemon::Utils::AnsiToWstring(artist);
			// check if directory exists 
			{
				auto filestatus = std::filesystem::file_status();
				if (!(std::filesystem::status_known(filestatus) ? std::filesystem::exists(filestatus) : 
					std::filesystem::exists(updatepath)))
				{
					// do error
				}
			}

			FILETIME bestDate = { 0,0 };
			FILETIME curDate;
			std::string name;
			

		}


		std::this_thread::sleep_for(std::chrono::seconds(updateinterval));
	}


	return 0;
}

void CWatcher::StopWatching()
{
	
}

int CWatcher::AddToWatchList(const std::string & artist)
{
	//.AddMember<std::string>("", artist, m_watchJson.GetAllocator());
	m_watchList.push_back(artist);

	return 0;
}

int CWatcher::RemoveFromWatchList(const std::string & artist)
{
	//m_watchJson.RemoveMember(artist);
	m_watchList.remove(artist);

	return 0;
}

int CWatcher::SetupWatchList(const std::wstring & folder)
{
	std::wstring documentpath = folder + L"\\" + L"watchlist.json";

	std::ifstream injson(documentpath);

	if (injson.bad())
	{
		HWND hwnd = GetActiveWindow();
		std::wstring message = L"Watch list file does not exist! Create it?";
		std::wstring caption = L"Oospie";
		if (MessageBox(hwnd, message.c_str(), caption.c_str(), MB_OKCANCEL | MB_ICONWARNING) == IDOK)
		{
			auto temp = CreateFile(
				documentpath.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, CREATE_NEW, 0, NULL
			);
			CloseHandle(temp);
		}
		else{
			return -1;
		}		
	}

	injson.open(documentpath);

	rapidjson::IStreamWrapper isw(injson);
	m_watchJson.ParseStream(isw);

	for (auto itr = m_watchJson.Begin(); itr != m_watchJson.End(); ++itr)
	{
		std::string artist = itr->GetString();
		m_watchList.push_back(artist);
	}

	return 0;
}
