#pragma once

#include "CBaseDumper.h"

struct FASubmission;

class CFADumper : public CBaseDumper
{
	typedef CBaseDumper BaseClass;
public:
	CFADumper(ConfigMgr& mgr, const int item) : BaseClass(mgr), m_item(item) {}

	virtual int Action(const DownloadContext& ctx) override;

	int Download(const DownloadContext& ctx);

private:
	std::vector<FASubmission> GetMainGallery(const DownloadContext& ctx);
	std::vector<FASubmission> GetScrapGallery(const DownloadContext& ctx);

	int DownloadInternal(std::vector<FASubmission> gallery, const std::wstring& path, const DownloadContext& ctx);

	static CURLcode ThreadedImageDownload(int threadID, FASubmission submission, std::wstring path, ThreadLock<int>* progress, ThreadLock<int>* consolelock);

	const int m_item;
};

