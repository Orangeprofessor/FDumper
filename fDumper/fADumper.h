#pragma once

#include "CBaseDumper.h"
#include "curl/curl.h"

#include "rapidjson/istreamwrapper.h"
#include "rapidjson/document.h"

#include <mutex>
#include <atomic>

enum faRatingFlags : int
{
	ALL_RATINGS = 0,
	SFW_ONLY,
	NSFW_ONLY
};

enum faGalleryFlags : int
{
	ALL_GALLERIES = 0,
	NO_SCRAPS,
	SCRAPS_ONLY,
};

template<typename T>
class LockAssignable
{
public:
	LockAssignable& operator=(const T& t) {
		std::lock_guard<std::mutex> lk(m_mutex);
		m_protected = t;
		return *this;
	}
	operator T() const {
		std::lock_guard<std::mutex> lk(m_mutex);
		return m_protected;
	}
	T* operator->() {
		std::lock_guard<std::mutex> lk(m_mutex);
		return &m_protected;
	}
	inline void lock() {
		m_mutex.lock();
	}
	inline void unlock() {
		m_mutex.unlock();
	}

private:
	mutable std::mutex m_mutex;
	T m_protected{};
};

struct FASubmission;

class CFADumper : public CBaseDumper
{
public:
	CFADumper(const std::string apiurl, std::string uname, std::wstring savedir, int rating, int gallery, bool scrapfolder);
	~CFADumper();

	virtual int Download()override;

	int CurlDownload(const std::string& url, const std::wstring& dir);

	LockAssignable<std::string> m_imagename;
	std::atomic<size_t> m_currentIdx;
	std::atomic<size_t> m_totalImages;
	LockAssignable<bool> m_isDownloading;
	LockAssignable<std::string> m_status;

private:
	std::vector<FASubmission> GetMainGallery(bool sfw);
	std::vector<FASubmission> GetScrapGallery(bool sfw);

	int DownloadInternal(std::vector<FASubmission> gallery);
	static int ThreadedImageDownload(int id, FASubmission submission, CFADumper* self);
	static int ThreadedSubmissionDownload(int threadid, int id, LockAssignable<std::vector<FASubmission>>* gallery, CFADumper* self);
public:
	const std::string m_apiurl;
	std::string m_uHandle;
	std::wstring m_saveDirectory;
	faRatingFlags m_rating;
	faGalleryFlags m_gallery;
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