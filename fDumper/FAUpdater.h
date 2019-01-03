#pragma once

#include "CBaseDumper.h"

struct FASubmission;

class CFAUpdater : public CBaseDumper
{
	typedef CBaseDumper BaseClass;
public:
	virtual void PrintDescription() override;
	virtual bool Argument(arg_t& arg) override;
	virtual void Action(arg_t& arg) override;

	int Update(std::string artist);
	void UpdateAll();

private:
	std::vector<int> GetUserMainGalleryPages(std::string user, int rating);
	std::vector<int> GetUserScrapGalleryPages(std::string user, int rating);
	std::vector<int> GetUserFavoriteGalleryPages(std::string user, int rating);

	int DownloadInternal(std::vector<FASubmission> gallery, std::wstring folder);

	static CURLcode ThreadedImageDownload(int threadID, FASubmission submission, std::wstring path, ThreadLock<int>* progress, ThreadLock<int>* consolelock);

private:
	bool m_allusers = false;
	bool m_nofaves = false;
	bool m_favesonly = false;
};