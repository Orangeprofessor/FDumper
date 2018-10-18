#include "pch.h"

#include <filesystem>
#include "FDumper/CBaseDumper.h"
#include "watcher.h"
#include "toaster.hpp"

#include "curl/curl.h"

const std::string& g_API = nullptr;


CWatcher::CWatcher(const std::wstring& folder) : m_watchFolder(folder) {}

CWatcher::~CWatcher()
{
}

int CWatcher::WatchForUpdates(int updateinterval)
{
	while (true)
	{
		if (stopWatching)
			break;

		for (auto artist : m_watchList)
		{
			namespace fs = std::filesystem;

			std::wstring artistfolder = m_watchFolder + AnsiToWstring(artist.artistname);

			bool exists;
			for (auto it = fs::directory_iterator(artistfolder); it != fs::directory_iterator(); ++it)
				exists = fs::status_known(it->status()) ? fs::exists(it->status()) : fs::exists(*it);

			if (!exists)
			{
				//error
				continue;
			}

			CArtist furry(artist, artistfolder);
			auto latestimage = furry.GetHighestImageNumber();
			furry.Update(latestimage);
		}


		std::this_thread::sleep_for(std::chrono::seconds(updateinterval));
		// TODO: toast me daddy

	}


	return 0;
}

void CWatcher::StopWatching()
{
	stopWatching = true;
}


bool CWatcher::SetupWatchList(const std::wstring & folder)
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
		else {
			return false;
		}
	}

	injson.open(documentpath);

	rapidjson::IStreamWrapper isw(injson);
	m_watchJson.ParseStream(isw);

	for (auto itr = m_watchJson.Begin(); itr != m_watchJson.End(); ++itr)
	{
		for (auto memberitr = itr->Begin(); memberitr != itr->End(); ++memberitr)
		{
			auto name = memberitr->FindMember("name")->value.GetString();
			auto rating = memberitr->FindMember("rating")->value.GetInt();
			auto gallery = memberitr->FindMember("gallery")->value.GetInt();

			m_watchList.push_back({ name, rating, gallery });
		}
	}

	return true;
}

CArtist::CArtist(const CArtistConfig& config, const std::wstring & path) : m_config(config), m_workingpath(path) {}

size_t CArtist::GetHighestImageNumber()
{
	namespace fs = std::filesystem;

	size_t highestIdx = 0;

	for (auto& p : fs::directory_iterator(m_workingpath))
	{
		if (!p.is_regular_file())
			continue;

		auto filename = p.path().filename().wstring();
		auto highest = filename.substr(filename.find_first_of(L".") + 1);

		auto idx = std::stoi(highest);

		if (highestIdx < idx)
			highestIdx = idx;
	}

	return highestIdx;
}

void CArtist::Update(size_t highestidx)
{
	std::vector<FASubmission> vecUpdateList;
	bool bfound = false;

	int curpage = 1;
	while (true)
	{
		char urlbuff[128] = {};
		if (m_config.rating == SFW_ONLY) {
			sprintf_s(urlbuff, "%s/user/%s/gallery.json?sfw=1&page=%d", g_API.c_str(), m_config.artistname.c_str(), curpage);
		}
		else {
			sprintf_s(urlbuff, "%s/user/%s/gallery.json?page=%d", g_API.c_str(), m_config.artistname.c_str(), curpage);
		}

		auto curlDownload = [&](const std::string& url, std::string& buffer) -> int
		{
			CURL* pCurl = curl_easy_init();

			curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writebuffercallback);
			curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, buffer);
			CURLcode code = curl_easy_perform(pCurl);

			curl_easy_cleanup(pCurl);
			return code;
		};

		std::string curlbuffer;
		curlDownload(urlbuff, curlbuffer);

		rapidjson::Document doc;
		doc.Parse(curlbuffer.c_str());

		for (auto it = doc.Begin(); it != doc.End(); ++it)
		{
			auto faID = std::stoi(it->GetString());
			FASubmission submission(faID); submission.Setup(g_API);

			auto url = submission.GetDownloadLink();
			auto filename = url.substr(url.find_last_of("/") + 1);
			auto highest = filename.substr(filename.find_first_of(".") + 1);

			auto index = std::stoi(highest);

			if (index == highestidx) {
				bfound = true;
				break;
			}

			vecUpdateList.push_back(submission);
		}

		if (bfound)
			break;

		if (doc.Size() < 72)
			break;
		++curpage;
	}

	// TODO: toast notification
	// how the fuck do you toast notification????
	using namespace WinToastLib;
	WinToastTemplate templ(WinToastTemplate::ImageAndText02);
	templ.setTextField(L"New Submissions!", WinToastTemplate::FirstLine);
	templ.setAudioOption(WinToastTemplate::AudioOption::Default);
	WinToast::instance()->showToast(templ, new CFWatcherNotifHandler());

	// Now download

	auto curlDownload = [&](const std::string& url, const std::wstring& path) -> int
	{
		FILE* fp = nullptr;
		_wfopen_s(&fp, path.c_str(), L"wb");

		CURL* pCurl = curl_easy_init();

		curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writefilecallback);
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);

		CURLcode res = curl_easy_perform(pCurl);

		return std::fclose(fp), res;
	};

	for (auto submission : vecUpdateList)
	{
		auto url = submission.GetDownloadLink();
		auto filename = url.substr(url.find_last_of("/") + 1);
		std::wstring download = m_workingpath + AnsiToWstring(filename);

		if (curlDownload(url, download)) {

		}
	}

}
