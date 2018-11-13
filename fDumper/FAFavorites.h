#pragma once

#include "CBaseDumper.h"

struct FASubmission;

class CFAFavorites : public CBaseDumper
{
	typedef CBaseDumper BaseClass;
public:
	virtual void PrintDescription() override;
	virtual bool Argument(arg_t& arg) override;
	virtual void Action(arg_t& arg) override;

	void Download();

private:
	std::vector<FASubmission> GetFavoritesGallery();

	void DownloadInternal(std::vector<FASubmission> gallery);

	static CURLcode ThreadedImageDownload(int threadID, FASubmission submission, std::wstring path, ThreadLock<int>* progress);

protected:
	int m_rating = 0;
	std::string m_uHandle;
	std::wstring m_savedir;
};