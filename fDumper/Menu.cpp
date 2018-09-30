
#include "pch.h"

#include "Menu.h"

#include "fADumper.h"
#include "e621Dumper.h"

#include <libda3m0n/Misc/Utils.h>

#include <ShlObj.h>

CImMenu::CImMenu() : m_notReallyAPoolBecauseItsOnlyOneThread(1), searchbuff("Username..."), apiurl("faexport.boothale.net")
{
	status.assign("Idle");
}

CImMenu::~CImMenu()
{
}

void CImMenu::ImDraw()
{
#pragma warning( push )  
#pragma warning( disable : 4244 )
	bool open = true;
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
	if (ImGui::Begin("##MAINMENU", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
	{
		if (ImGui::BeginMenuBar())
		{
			bool settings = false;
			if (ImGui::BeginMenu("Settings"))
			{
				ImGui::MenuItem("Create username folder", NULL, &usernamefolder);
				ImGui::MenuItem("Create scraps folder", NULL, &scrapsfolder);

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}


		if (ImGui::BeginChild("##MAINQUEUE", ImVec2(0, 0), false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			int width = ImGui::GetWindowContentRegionWidth();
			ImGui::PushItemWidth(width);
			ImGui::InputText("##APIURL", apiurl, IM_ARRAYSIZE(apiurl));
			ImGui::PopItemWidth();

			if (ImGui::ListBoxHeader("##IMQUEUE", ImVec2(width, 170)))
			{
				ImGui::ListBoxFooter();
			}

		
			//ImGui::PushItemWidth(width);
			//ImGui::pushi
			//ImGui::ListBox("##IMQUEUE")
			

			static const char* ratings[] = { "All Raitings", "SFW Only", "NSFW Only" };
			static const char* sorting[] = { "All Galleries", "No Scraps", "Scraps Only" };
			ImGui::PushItemWidth(width / 2);
			ImGui::Combo("##RATINGS", &rating, ratings, ARRAYSIZE(ratings)); ImGui::SameLine();
			ImGui::PopItemWidth();
			ImGui::PushItemWidth((width / 2) - 5);
			ImGui::Combo("##SORTINGS", &sorttype, sorting, ARRAYSIZE(sorting));
			ImGui::PopItemWidth();

		
			if (pDumper)
			{
				if (pDumper->m_isDownloading.operator bool() == true)
				{
					totalimages = pDumper->m_totalImages.operator size_t();
					currentimage = pDumper->m_currentIdx.operator size_t();
				}
			}
			totalimages == 0 ? totalimages = 1 : totalimages;

			char buf[32];
			sprintf(buf, "%zd/%zd", currentimage, totalimages);
			ImGui::ProgressBar((float)((float)currentimage / (float)totalimages), ImVec2(-1.f, 0.f), buf);

			ImGui::PushItemWidth(width - 60);
			ImGui::InputText("##SEARCHBOX", searchbuff, ARRAYSIZE(searchbuff));
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::Button("(null)", ImVec2(50, 25));
			if (ImGui::Button("Pause Download", ImVec2(width / 2, 25)))
			{
				// suspend that bish

				
			}

			ImGui::SameLine();
			if (ImGui::Button("Resume Download", ImVec2(width / 2, 25)))
			{
				// resume that bish
			}


			if (ImGui::Button("Cancel", ImVec2(width / 2, 25)))
			{
				m_notReallyAPoolBecauseItsOnlyOneThread.stop();
			}

			ImGui::SameLine();
			if (ImGui::Button("Start", ImVec2(width / 2, 25)))
			{
				//if (pDumper)
				//	delete pDumper;

				std::wstring dest = OpenSaveDialog();

				if (!dest.empty())
				{
					if (usernamefolder) {
						dest += std::wstring(L"\\") + libdaemon::Utils::AnsiToWstring(searchbuff);
						CreateDirectory(dest.c_str(), NULL);
					}

					pDumper = new CFADumper(apiurl, searchbuff, dest, rating, sorttype, scrapsfolder);

					auto downloader = [&](int id) {
						pDumper->Download();
						status.assign("Idle");
					};

					m_notReallyAPoolBecauseItsOnlyOneThread.push(downloader);
				}	
			}

			if (pDumper)
			{
				status.assign(pDumper->m_status.operator std::string());
			}
			ImGui::Text(status.c_str());
			ImGui::EndChild();
		}
		ImGui::End();
	}
#pragma warning(pop)
}

std::wstring CImMenu::OpenSaveDialog()
{
	TCHAR szDir[MAX_PATH];
	BROWSEINFO bInfo;
	bInfo.hwndOwner = GetActiveWindow();
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
	bInfo.lpszTitle = L"Pwease sewect a fowdew UwU"; // Title of the dialog
	bInfo.ulFlags = 0;
	bInfo.lpfn = NULL;
	bInfo.lParam = 0;
	bInfo.iImage = -1;

	LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);

	SHGetPathFromIDList(lpItem, szDir);
	return szDir;
}

