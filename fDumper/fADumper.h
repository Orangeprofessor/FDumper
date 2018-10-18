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


struct FASubmission;

class CFADumper : public CBaseDumper
{
public:
	CFADumper(const std::string apiurl, std::string uname, std::wstring savedir,
		faRatingFlags rating, faGalleryFlags gallery, ctrl::ListView& imglist,
		ctrl::StatusBar& status, ctrl::ProgressBar& progress);
	~CFADumper();

	virtual int Download()override;

	int CurlDownload(const std::string& url, const std::wstring& dir);

	std::string m_imagename;
	bool m_isDownloading;
	size_t m_currentIdx;
	size_t m_totalImages;

private:
	std::vector<FASubmission> GetMainGallery(bool sfw);
	std::vector<FASubmission> GetScrapGallery(bool sfw);

	int DownloadInternal(std::vector<FASubmission> gallery);
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

