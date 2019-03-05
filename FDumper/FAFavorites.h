#pragma once

#include "CBaseDumper.h"

struct FASubmission;

class CFAFavorites : public CBaseDumper
{
	typedef CBaseDumper BaseClass;
public:
	CFAFavorites(ConfigMgr& cfg) : BaseClass(cfg) {}

	virtual int Action(const DownloadContext& ctx) override;

	int Download();
	
private:
	std::vector<FASubmission> GetFavoritesGallery();

	int DownloadInternal(std::vector<FASubmission> gallery);

	static CURLcode ThreadedImageDownload(int threadID, FASubmission submission, std::wstring path, ThreadLock<int>* progress, ThreadLock<int>* consolelock);
};