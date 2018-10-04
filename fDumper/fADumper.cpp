#include "pch.h"

#include "fADumper.h"
#include "Log.h"
#include "ctpl_stl.hpp"

#include <ShlObj.h>
#include <libda3m0n/Misc/Utils.h>


CFADumper::CFADumper(const std::string apiurl, std::string uname, std::wstring savedir, int rating, int gallery, bool scrapfolder) : m_apiurl(apiurl), m_uHandle(uname),
m_saveDirectory(savedir), m_rating((faRatingFlags)rating), m_gallery((faGalleryFlags)gallery), m_scrapfolder(scrapfolder)
{

}

CFADumper::~CFADumper()
{

}

int CFADumper::Download()
{
	m_status.operator=("Downloading submission data...");

	if (m_gallery == faGalleryFlags::SCRAPS_ONLY)
	{
		if (m_scrapfolder)
		{
			m_saveDirectory += L"\\" + std::wstring(L"Scraps");
			CreateDirectory(m_saveDirectory.c_str(), NULL);
		}

		if (m_rating == faRatingFlags::NSFW_ONLY)
		{
			auto fullgallery = GetScrapGallery(false);

			fullgallery.erase(std::remove_if(fullgallery.begin(), fullgallery.end(), [&](FASubmission sub) {
				return sub.GetRating() == 1;
			}), fullgallery.end());

			// now download
			DownloadInternal(fullgallery);
		}
		else if (m_rating == faRatingFlags::SFW_ONLY)
		{
			auto gallery = GetScrapGallery(true);

			// download
			DownloadInternal(gallery);
		}
		else
		{
			auto fullgallery = GetScrapGallery(false);

			// download
			DownloadInternal(fullgallery);
		}


		return 0;
	}


	if (m_rating == faRatingFlags::NSFW_ONLY)
	{
		auto fullgallery = GetMainGallery(false);

		fullgallery.erase(std::remove_if(fullgallery.begin(), fullgallery.end(), [&](FASubmission sub) {
			return sub.GetRating() == 1;
		}), fullgallery.end());

		// now download
		DownloadInternal(fullgallery);

		// now do scraps if wanted
		if (m_gallery != faGalleryFlags::NO_SCRAPS)
		{
			auto fullscrapgallery = GetScrapGallery(false);

			fullscrapgallery.erase(std::remove_if(fullscrapgallery.begin(), fullscrapgallery.end(), [&](FASubmission sub) {
				return sub.GetRating() == 1;
			}), fullscrapgallery.end());

			if (m_scrapfolder)
			{
				m_saveDirectory += L"\\" + std::wstring(L"Scraps");
				CreateDirectory(m_saveDirectory.c_str(), NULL);
			}

			DownloadInternal(fullscrapgallery);
		}
	}
	else if (m_rating == faRatingFlags::SFW_ONLY)
	{
		auto gallery = GetMainGallery(true);

		DownloadInternal(gallery);

		if (m_gallery != faGalleryFlags::NO_SCRAPS) {
			auto scrapgallery = GetScrapGallery(true);
			if (m_scrapfolder)
			{
				m_saveDirectory += L"\\" + std::wstring(L"Scraps");
				CreateDirectory(m_saveDirectory.c_str(), NULL);
			}
			DownloadInternal(scrapgallery);
		}
	}
	else
	{
		auto fullgallery = GetMainGallery(false);

		DownloadInternal(fullgallery);

		if (m_gallery != faGalleryFlags::NO_SCRAPS) {
			auto scrapgallery = GetScrapGallery(false);
			if (m_scrapfolder)
			{
				m_saveDirectory += L"\\" + std::wstring(L"Scraps");
				CreateDirectory(m_saveDirectory.c_str(), NULL);
			}
			DownloadInternal(scrapgallery);
		}
	}

	return 0;
}

std::vector<FASubmission> CFADumper::GetMainGallery(bool sfw)
{
	LockAssignable<std::vector<FASubmission>> gallery;
	std::vector<int> tempIDs;

	wchar_t tempbuff[MAX_PATH] = {};
	GetTempPath(MAX_PATH, tempbuff);

	m_currentIdx.operator=(0);
	m_totalImages.operator=(0);
	m_isDownloading.operator=(true);

	int curpage = 1;
	while (true)
	{
		char urlbuff[200] = {};
		if (sfw) {
			std::sprintf(urlbuff, "%s/user/%s/gallery.json?sfw=1&page=%d", m_apiurl.c_str(), m_uHandle.c_str(), curpage);
		}
		else {
			std::sprintf(urlbuff, "%s/user/%s/gallery.json?page=%d", m_apiurl.c_str(), m_uHandle.c_str(), curpage);
		}

		std::wstring submissions;
		submissions.assign(tempbuff);
		submissions += L"\\" + libdaemon::Utils::AnsiToWstring(m_uHandle) + std::to_wstring(curpage) + std::wstring(L".json");

		if (CurlDownload(urlbuff, submissions)) {

		}

		std::ifstream ifs(submissions);
		rapidjson::IStreamWrapper isw(ifs);

		rapidjson::Document doc;
		doc.ParseStream(isw);

		ifs.close();

		for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
		{
			std::string jsonElm = itr->GetString();
			int subID = std::stoi(jsonElm);

			tempIDs.push_back(subID);
		}

		++curpage;
		if (doc.Size() < 72)
			break;
	}

	m_totalImages.operator=(tempIDs.size());

	ctpl::thread_pool threads(4);

	for (auto IDs : tempIDs) {

		threads.push(ThreadedSubmissionDownload, IDs, &gallery, this);
	}

	while (threads.n_idle() != threads.size())
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	threads.stop();

	m_isDownloading.operator=(false);

	return gallery;
}

std::vector<FASubmission> CFADumper::GetScrapGallery(bool sfw)
{
	LockAssignable<std::vector<FASubmission>> scraps;
	std::vector<int> tempIDs;

	wchar_t tempbuff[MAX_PATH] = {};
	GetTempPath(MAX_PATH, tempbuff);

	m_currentIdx.operator=(0);
	m_totalImages.operator=(0);
	m_isDownloading.operator=(true);

	int curpage = 1;
	while (true)
	{
		char urlbuff[200] = {};

		if (sfw) {
			std::sprintf(urlbuff, "%s/user/%s/scraps.json?sfw=1&page=%d", m_apiurl.c_str(), m_uHandle.c_str(), curpage);
		}
		else {
			std::sprintf(urlbuff, "%s/user/%s/scraps.json?page=%d", m_apiurl.c_str(), m_uHandle.c_str(), curpage);
		}

		std::wstring submissions;
		submissions.assign(tempbuff);
		submissions += L"\\" + libdaemon::Utils::AnsiToWstring(m_uHandle) + std::to_wstring(curpage) + std::wstring(L".json");

		if (CurlDownload(urlbuff, submissions)) {

		}

		std::ifstream ifs(submissions);
		rapidjson::IStreamWrapper isw(ifs);

		rapidjson::Document doc;
		doc.ParseStream(isw);

		ifs.close();

		for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
		{
			std::string jsonElm = itr->GetString();
			int subID = std::stoi(jsonElm);

			tempIDs.push_back(subID);
		}

		++curpage;
		if (doc.Size() < 72)
			break;
	}

	m_totalImages.operator=(tempIDs.size());

	ctpl::thread_pool threads(4);

	for (auto IDs : tempIDs) {
		threads.push(ThreadedSubmissionDownload, IDs, &scraps, this);
	}

	while (threads.n_idle() != threads.size())
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	threads.stop();

	m_isDownloading.operator=(false);

	return scraps;
}

__forceinline size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fwrite(ptr, size, nmemb, stream);
};

int CFADumper::CurlDownload(const std::string & url, const std::wstring & dir)
{
	FILE* fp = _wfopen(dir.c_str(), L"wb");

	CURL* pCurl = curl_easy_init();

	curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, TRUE);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);

	CURLcode res = curl_easy_perform(pCurl);
	if (res != CURLE_OK) {
		xlog::Error("Download failed! error %d\n", res);
	}

	std::fclose(fp);

	curl_easy_cleanup(pCurl);

	return res;
}

int CFADumper::DownloadInternal(std::vector<FASubmission> gallery)
{
	m_status.operator=("Downloading...");

	m_totalImages.operator=(gallery.size());
	m_currentIdx.operator=(1);
	m_isDownloading.operator=(true);

	ctpl::thread_pool threads(4);

	for (auto submission : gallery)
	{
		threads.push(ThreadedImageDownload, submission, this);
	}

	while (threads.n_idle() != threads.size()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	threads.stop();

	m_isDownloading.operator=(false);

	m_status.operator=("Idle");

	return 0;
}

int CFADumper::ThreadedImageDownload(int id, FASubmission submission, CFADumper* self)
{
	auto link = submission.GetDownloadLink();
	auto filename = link.substr(link.find_last_of("/") + 1);

	std::wstring savedir = self->m_saveDirectory + std::wstring(L"\\") +
		libdaemon::Utils::AnsiToWstring(filename);

	int code = self->CurlDownload(link, savedir);
	return self->m_currentIdx.operator++(), code;
}

int CFADumper::ThreadedSubmissionDownload(int threadid, int id, LockAssignable<std::vector<FASubmission>>* gallery, CFADumper * self)
{
	FASubmission submission(id, self);
	//gallery->operator->()->push_back(submission);

	auto gal = gallery->operator->();
	gallery->lock();
	gal->push_back(submission);
	gallery->unlock();

	auto curridx = self->m_currentIdx.operator size_t();
	self->m_currentIdx.operator=(++curridx);

	return 0;
}
