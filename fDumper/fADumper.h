#pragma once

#include "CBaseDumper.h"
#include "../contrib/curl/curl.h"

#include "../contrib/rapidjson/istreamwrapper.h"
#include "../contrib/rapidjson/document.h"


struct FASubmission;

class CFADumper : public CBaseDumper
{
	typedef CBaseDumper BaseClass;
public:

	virtual void PrintDescription() override;
	virtual bool Argument(arg_t& arg) override;
	virtual void Action(arg_t& arg) override;

	int Download();

private:
	std::vector<FASubmission> GetMainGallery();
	std::vector<FASubmission> GetScrapGallery();

	int DownloadInternal(std::vector<FASubmission> gallery);

protected:
	std::string m_uHandle;
	int m_rating = 0;
	int m_gallery = 0;
	std::wstring m_savedir;
};

