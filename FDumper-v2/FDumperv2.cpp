#include "stdafx.h"
#include "FDumperv2.h"

#include "FADumper.h"

ThreadLock<std::shared_ptr<FDumperv2>> FDumperv2::m_instance = ThreadLock<std::shared_ptr<FDumperv2>>();

FDumperv2::FDumperv2(QWidget *parent)
	: QMainWindow(parent), m_pool(4)
{
	m_ui.setupUi(this);
	connect(m_ui.addToQueue, SIGNAL(clicked()), this, SLOT(OnAddQueueButton()));

	m_queueModel = std::make_unique<FAQueueModel>(this, 4);
	m_ui.queueTable->setModel(m_queueModel.get());

	m_reportModel = std::make_unique<FAReportModel>(this, 4);
	m_ui.reportView->setModel(m_reportModel.get());

}

void FDumperv2::OnAddQueueButton()
{
	auto username = m_ui.queueLineInput->text().toStdString();

	if (username.empty())
		return;

	DownloadContext context{};

	context.username = username;

	if (m_ui.allRatings->isChecked())
		context.ratings = ALL_RATINGS;
	else if (m_ui.SFWonly->isChecked())
		context.ratings = SFW_ONLY;
	else if (m_ui.NSFWonly->isChecked())
		context.ratings = NSFW_ONLY;

	if (m_ui.ctfilterMain->isChecked())
		context.gallery |= MAIN;
	if (m_ui.ctfilterScraps->isChecked())
		context.gallery |= SCRAPS;
	if (m_ui.ctfilterFavs->isChecked())
		context.gallery |= FAVORITES;

	if (context.gallery == 0)
	{
		//message error
		return;
	}

	auto dumper = [&](int threadid, DownloadContext ctx) -> void 
	{
		CFADumper dumper(m_config);
		dumper.Action(ctx);
	};

	

	m_pool.push(dumper, context);
}
