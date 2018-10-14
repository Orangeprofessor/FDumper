#pragma once

#include "CBaseDumper.h"
#include "curl/curl.h"

#include <libda3m0n/Misc/Utils.h>

#include "rapidjson/istreamwrapper.h"
#include "rapidjson/document.h"

#include "control/ProgressBar.hpp"
#include "control/ListView.hpp"
#include "control/StatusBar.hpp"

#include <mutex>
#include <atomic>

enum faRatingFlags : int
{
	ALL_RATINGS = 0,
	NSFW_ONLY,
	SFW_ONLY
};

enum faGalleryFlags : int
{
	ALL_GALLERIES = 0,
	NO_SCRAPS,
	SCRAPS_ONLY,
};

template<typename T>
class ThreadLock
{
public:
	ThreadLock& operator=(const T& t) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_protected = t;
		return *this;
	}
	operator T() const {
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_protected;
	}
	void execute(std::function<void(T)> callback) {
		assert(callback != nullptr);
		std::lock_guard<std::mutex> lock(m_mutex);
		callback(m_protected);
	}
private:
	mutable std::mutex m_mutex;
	T m_protected{};
};

template <typename T>
class ArithmeticThreadLock
{
public:
	ArithmeticThreadLock<T>() {
		static_assert(std::is_arithmetic<T>::value);
	}
	ArithmeticThreadLock& operator=(const T& t) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_protected = t;
		return *this;
	}
	operator T() const {
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_protected;
	}
	ArithmeticThreadLock& operator++() {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_protected++;
		return *this;
	}
	ArithmeticThreadLock& operator--() {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_protected--;
		return *this;
	}
	ArithmeticThreadLock& operator+=(const T t) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_protected += t;
		return *this;
	}
	ArithmeticThreadLock& operator-=(const T t) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_protected -= t;
		return *this;
	}
	void execute(std::function<void(T)>& callback) {
		assert(callback != nullptr);
		std::lock_guard<std::mutex> lock(m_mutex);
		callback(m_protected);
	}
private:
	mutable std::mutex m_mutex;
	T m_protected{};
};

struct FASubmission;

class CFADumper  : public CBaseDumper
{
public:
	CFADumper(const std::string apiurl, std::string uname, std::wstring savedir, 
		faRatingFlags rating, faGalleryFlags gallery, ctrl::ListView& imglist, 
		ctrl::StatusBar& status, ctrl::ProgressBar& progress);
	~CFADumper();

	virtual int Download()override;

	int CurlDownload(const std::string& url, const std::wstring& dir);

	ThreadLock<std::string> m_imagename;
	ThreadLock<bool> m_isDownloading;
	ArithmeticThreadLock<size_t> m_currentIdx;
	ArithmeticThreadLock<size_t> m_totalImages;

private:
	std::vector<FASubmission> GetMainGallery(bool sfw);
	std::vector<FASubmission> GetScrapGallery(bool sfw);

	int DownloadInternal(std::vector<FASubmission> gallery);
	static int ThreadedImageDownload(int id, FASubmission submission, CFADumper* self);
	static void ThreadedSubmissionDownload(int threadid, int id, ThreadLock<std::vector<FASubmission>>* gallery, CFADumper* self);
public:
	const std::string m_apiurl;
	std::string m_uHandle;
	std::wstring m_saveDirectory;
	faRatingFlags m_rating;
	faGalleryFlags m_gallery;
	ctrl::ListView& m_listview;
	ctrl::StatusBar& m_status;
	ctrl::ProgressBar& m_progress;
};

struct FASubmission
{
	FASubmission(int submission, CFADumper* dumper) : submissionID(submission), dumper(dumper)
	{
		Setup();
	}

	void Setup()
	{
		char urlbuff[200] = {};

		sprintf_s(urlbuff, "%s/submission/%d.json", dumper->m_apiurl.c_str(), submissionID);

		wchar_t tempbuff[MAX_PATH] = {};
		GetTempPath(MAX_PATH, tempbuff);

		std::wstring dlpath = tempbuff + std::wstring(L"\\") + std::to_wstring(submissionID) + std::wstring(L".json");

		if (dumper->CurlDownload(urlbuff, dlpath)) {
			// uhhh
			throw std::exception("wtf");
		}

		std::ifstream ifs(dlpath);
		rapidjson::IStreamWrapper isw(ifs);

		rapidjson::Document doc;
		doc.ParseStream(isw);

		ifs.close();

		auto fullimage = doc.FindMember("download");
		downloadURL = fullimage->value.GetString();

		auto imagetitle = doc.FindMember("title");
		title = imagetitle->value.GetString();

		auto imagerating = doc.FindMember("rating");
		imagerating->value.GetString() == std::string("General") ? rating = 1 : rating = 2;

		auto wtitle = libdaemon::Utils::AnsiToWstring(title);
		auto wid = std::to_wstring(submissionID);
		auto wrating = libdaemon::Utils::AnsiToWstring(imagerating->value.GetString());

		dumper->m_listview.AddItem(wtitle, static_cast<LPARAM>(submissionID), { wid, wrating });

		_wremove(dlpath.c_str());
	}

	inline std::string GetDownloadLink() { return downloadURL; }
	inline int GetSubmissionID() { return submissionID; }
	inline std::string GetSubmissionTitle() { return title; }
	inline int GetRating() { return rating; }

private:
	CFADumper* dumper;
	std::string downloadURL;
	std::string title;
	int rating;
	int submissionID;
};