#include "pch.h"

#include "fADumper.h"
#include "ctpl_stl.hpp"
#include "rapidxml/rapidxml_wrap.hpp"
#include "Log.h"

#include <libda3m0n/Misc/Utils.h>
#include <filesystem>
#include <ShlObj.h>



CFADumper::CFADumper(const std::string & apiurl) : m_apiurl(apiurl), m_pCurl(curl_easy_init())
{
}

CFADumper::~CFADumper()
{
	curl_easy_cleanup(m_pCurl);
}

int tDownload(int id, CFADumper::faPathUrl sel, CFADumper* self) {
	auto filename = sel.second;
	auto filenamepos = filename.find_last_of('/');
	filename.erase(0, filenamepos + 1);
	self->m_imagename.operator=(filename);
	int ret = self->CurlDownload(sel.second, sel.first);
	self->m_currentIdx.operator=(+1);
	return ret;
}

int CFADumper::Download()
{


	if (m_contentflags == SCRAPS_ONLY)
	{
		auto gallery = GetScrapGallery();
		m_currentIdx.operator=(1);
		m_totalImages.operator=(gallery.size());
		m_finishedDl.operator=(false);
		ctpl::thread_pool threads(4);

		size_t i = 0;
		int numthreads = 0;
		for (auto image : gallery)
		{
			if (numthreads < 3) {
				do
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				} while (threads.n_idle() >= 2);
			}

			threads.push(tDownload, image, this);
		}

		m_finishedDl.operator=(true);
		threads.stop(true);

		return 0;
	}


	auto maingallery = GetMainGallery();
	if (m_scrapflags != SCRAPS_ONLY) {
		auto scrapgallery = GetScrapGallery();
		maingallery.insert(maingallery.end(), scrapgallery.begin(), scrapgallery.end());
	}

	ctpl::thread_pool threads(4);

	size_t i = 0;
	int numthreads = 0;
	m_currentIdx.operator=(1);
	m_totalImages.operator=(maingallery.size());
	m_finishedDl.operator=(false);
	for (auto image : maingallery)
	{
		if (numthreads < 3) {
			do
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			} while (threads.n_idle() >= 2);
		}

		threads.push(tDownload, image, this);
	}

	m_finishedDl.operator=(true);
	threads.stop(true);

	return 0;
}

int CFADumper::QueryAPI()
{

	return 0;
}

std::vector<CFADumper::faPathUrl> CFADumper::GetMainGallery()
{
	wchar_t tempbuff[MAX_PATH];
	GetTempPath(sizeof(tempbuff), tempbuff);

	// Get submission pages
	std::vector<std::string> vecsubids;
	int curpage = 1;
	while (true)
	{
		char urlbuff[100];

		if (m_contentflags == SFW_ONLY) {
			std::sprintf(urlbuff, "%s/%s?sfw=1&page=%d", m_apiurl.c_str(), m_uHandle.c_str(), curpage);
		}
		else {
			std::sprintf(urlbuff, "%s/%s?page=%d", m_apiurl.c_str(), m_uHandle.c_str(), curpage);
		}

		std::wstring xmlpath;
		xmlpath.assign(tempbuff);
		xmlpath += L"\\" + libdaemon::Utils::AnsiToWstring(m_uHandle) + std::to_wstring(curpage) + std::wstring(L".xml");

		CurlDownload(urlbuff, xmlpath);

		acut::XmlDoc<char> xml;
		xml.read_from_file(xmlpath);
		

		int marks = 0;

		for (auto subids : xml.all_children_of("submissions")) {
			++marks;
			vecsubids.push_back(subids.value());
		}

		_wremove(xmlpath.c_str());

		++curpage;
		if (marks < 72)
			break;
	}

	curpage = 1;

	// redownload and compare
	if (m_contentflags == NSFW_ONLY)
	{
		std::vector<std::string> sfwids;
		while (true)
		{
			char urlbuff[100];

			std::sprintf(urlbuff, "%s/%s/gallery.xml?sfw=1&page=%d", m_apiurl.c_str(), m_uHandle.c_str(), curpage);

			std::wstring xmlpath;
			xmlpath.assign(tempbuff);
			xmlpath += L"\\" + libdaemon::Utils::AnsiToWstring(m_uHandle) + std::to_wstring(curpage) + std::wstring(L".xml");

			CurlDownload(urlbuff, xmlpath);

			acut::XmlDoc<char> xml;
			xml.read_from_file(xmlpath);

			int marks = 0;

			for (auto subids : xml.all_children_of("submissions")) {
				++marks;
				sfwids.push_back(subids.value());
			}

			_wremove(xmlpath.c_str());

			++curpage;
			if (marks < 72)
				break;
		}

		for (auto sfw : sfwids)
		{
			auto it = std::find(vecsubids.begin(), vecsubids.end(), sfw);

			if (it != std::end(vecsubids)) {
				vecsubids.erase(it);
			}
		}

	}

	// now since we have a bunch of ids, lets get the actual download link
	std::vector<faPathUrl> vecdlIDs;

	for (auto subid : vecsubids)
	{
		char subbuff[100];
		std::sprintf(subbuff, "%s/submission/%s.xml", m_apiurl.c_str(), subid.c_str());

		std::wstring subxmlpath;
		subxmlpath.assign(tempbuff);
		subxmlpath += L"\\" + std::wstring(L"fa") + libdaemon::Utils::AnsiToWstring(subid) + std::wstring(L".xml");

		CurlDownload(subbuff, subxmlpath);

		acut::XmlDoc<char> subxml;
		subxml.read_from_file(subxmlpath);


		for (auto subkey : subxml.all_children_of("submission"))
		{
			if (!subkey.name().compare("download"))
			{
				std::wstring filename = libdaemon::Utils::AnsiToWstring(subkey.value());
				auto filenamepos = filename.find_last_of('/');
				filename.erase(0, filenamepos + 1);
				vecdlIDs.push_back(faPathUrl(m_saveDirectory + L"\\" + filename, libdaemon::Utils::WstringToAnsi(filename)));
				break;
			}
		}
		_wremove(subxmlpath.c_str());
	}

	return vecdlIDs;
}

std::vector<CFADumper::faPathUrl> CFADumper::GetScrapGallery()
{
	wchar_t tempbuff[MAX_PATH];
	GetTempPath(sizeof(tempbuff), tempbuff);

	// Get submission pages
	std::vector<std::string> vecsubids;
	int curpage = 1;
	while (true)
	{
		char urlbuff[100];

		if (m_contentflags == SFW_ONLY) {
			std::sprintf(urlbuff, "%s/scraps/%s?sfw=1&page=%d", m_apiurl.c_str(), m_uHandle.c_str(), curpage);
		}
		else {
			std::sprintf(urlbuff, "%s/scrpas/%s?page=%d", m_apiurl.c_str(), m_uHandle.c_str(), curpage);
		}

		std::wstring xmlpath;
		xmlpath.assign(tempbuff);
		xmlpath += L"\\" + libdaemon::Utils::AnsiToWstring(m_uHandle) + std::to_wstring(curpage) + std::wstring(L".xml");

		CurlDownload(urlbuff, xmlpath);

		acut::XmlDoc<char> xml;
		xml.read_from_file(xmlpath);

		int marks = 0;

		for (auto subids : xml.all_children_of("submissions")) {
			++marks;
			vecsubids.push_back(subids.value());
		}

		_wremove(xmlpath.c_str());

		++curpage;
		if (marks < 72)
			break;
	}

	curpage = 1;

	// redownload and compare
	if (m_contentflags == NSFW_ONLY)
	{
		std::vector<std::string> sfwids;
		while (true)
		{
			char urlbuff[100];

			std::sprintf(urlbuff, "%s/scraps/%s?sfw=1&page=%d", m_apiurl.c_str(), m_uHandle.c_str(), curpage);

			std::wstring xmlpath;
			xmlpath.assign(tempbuff);
			xmlpath += L"\\" + libdaemon::Utils::AnsiToWstring(m_uHandle) + std::to_wstring(curpage) + std::wstring(L".xml");

			CurlDownload(urlbuff, xmlpath);

			acut::XmlDoc<char> xml;
			xml.read_from_file(xmlpath);

			int marks = 0;

			for (auto subids : xml.all_children_of("submissions")) {
				++marks;
				sfwids.push_back(subids.value());
			}

			_wremove(xmlpath.c_str());

			++curpage;
			if (marks < 72)
				break;
		}

		for (auto sfw : sfwids)
		{
			auto it = std::find(vecsubids.begin(), vecsubids.end(), sfw);

			if (it != std::end(vecsubids)) {
				vecsubids.erase(it);
			}
		}

	}

	// now since we have a bunch of ids, lets get the actual download link
	std::vector<faPathUrl> vecdlIDs;

	for (auto subid : vecsubids)
	{
		char subbuff[100];
		std::sprintf(subbuff, "%s/submission/%s.xml", m_apiurl.c_str(), subid.c_str());

		std::wstring subxmlpath;
		subxmlpath.assign(tempbuff);
		subxmlpath += L"\\" + std::wstring(L"fa") + libdaemon::Utils::AnsiToWstring(subid) + std::wstring(L".xml");

		CurlDownload(subbuff, subxmlpath);

		acut::XmlDoc<char> subxml;
		subxml.read_from_file(subxmlpath);


		for (auto subkey : subxml.all_children_of("submission"))
		{
			if (!subkey.name().compare("download"))
			{
				std::wstring filename = libdaemon::Utils::AnsiToWstring(subkey.value());
				auto filenamepos = filename.find_last_of('/');
				filename.erase(0, filenamepos + 1);
				vecdlIDs.push_back(faPathUrl(m_saveDirectory + L"\\" + filename, libdaemon::Utils::WstringToAnsi(filename)));
				break;
			}
		}
		_wremove(subxmlpath.c_str());
	}

	return vecdlIDs;
}


size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
};

int CFADumper::CurlDownload(const std::string & url, const std::wstring & dir)
{
	FILE* fp;
	CURLcode res;

	fp = _wfopen(dir.c_str(), L"wb");

	curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(m_pCurl, CURLOPT_NOPROGRESS, TRUE);
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, fp);

	res = curl_easy_perform(m_pCurl);
	if (res != CURLE_OK) {
		xlog::Error("Download failed! error %d\n", res);
	}

	std::fclose(fp);

	return res;
}

std::wstring CFADumper::OpenSaveDialog()
{
	TCHAR szDir[MAX_PATH];
	BROWSEINFO bInfo;
	bInfo.hwndOwner = GetActiveWindow();
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
	bInfo.lpszTitle = L"Pwease sewect a fowdew UwU"; // Title of the dialog
	bInfo.ulFlags = 0;
	bInfo.lpfn = NULL;
	bInfo.lParam = 0;
	bInfo.iImage = -1;

	LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);

	SHGetPathFromIDList(lpItem, szDir);
	return szDir;
}
