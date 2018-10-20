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
	int UpdateAll();

private:
	std::vector<int> GetUserMainGalleryPages(std::string user, int rating);
	std::vector<int> GetUserScrapGalleryPages(std::string user, int rating);

	int DownloadInternal(std::vector<FASubmission> gallery, std::wstring folder);

private:
	bool m_allusers = false;
};