#pragma once

#include "CBaseDumper.h"

struct FASubmission;

class CFADumper : public CBaseDumper
{
	CFADumper(const CFADumper&) = delete;
	CFADumper& operator=(const CFADumper&) = delete;

	typedef CBaseDumper BaseClass;
public:
	CFADumper(ConfigMgr& mgr) : BaseClass(mgr) {}

	virtual int Action(const DownloadContext& ctx) override;

	int Download(const DownloadContext& ctx);

private:
	std::vector<FASubmission> GetMainGallery(const DownloadContext& ctx);
	std::vector<FASubmission> GetScrapGallery(const DownloadContext& ctx);
	std::vector<FASubmission> GetFavoritesGallery(const DownloadContext& ctx);

	int DownloadInternal(std::vector<FASubmission> gallery, const std::wstring& path, const DownloadContext& ctx);
	static CURLcode DownloadImage(int threadID, FASubmission submission, std::filesystem::path path, const DownloadContext& ctx);
};

