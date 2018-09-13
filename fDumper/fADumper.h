#pragma once

#include "CBaseDumper.h"
#include "curl/curl.h"

#include <mutex>

enum faContentFlags
{
	ALL = 0,
	SFW_ONLY,
	NSFW_ONLY
};

enum faScrapFlags
{
	SALL = 0,
	SCRAPS_ONLY,
	NOSCRAPS
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
private:
	mutable std::mutex m_mutex;
	T m_protected{};
};

class CFADumper : public CBaseDumper
{
public:
	using faPathUrl = std::pair<std::wstring, std::string>;
public:
	CFADumper(const std::string& apiurl);
	~CFADumper();

	virtual int Download()override;
	virtual int QueryAPI()override;

	std::vector<faPathUrl> Search(std::string tags);

	int CurlDownload(const std::string& url, const std::wstring& dir);

	LockAssignable<std::string> m_imagename;
	LockAssignable<size_t> m_currentIdx;
	LockAssignable<size_t> m_totalImages;
	LockAssignable<bool> m_finishedDl;
private:
	std::vector<faPathUrl> GetMainGallery();
	std::vector<faPathUrl> GetScrapGallery();

	std::wstring OpenSaveDialog();

private:
	const std::string& m_apiurl;

	std::string m_uHandle;
	std::wstring m_saveDirectory;

	CURL* m_pCurl;
};
