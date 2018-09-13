#pragma warning( push )  
#pragma warning(disable : 4244)

#include "pch.h"

#include "Menu.h"

#include "fADumper.h"
#include "e621Dumper.h"

CImMenu::CImMenu()
{
}

CImMenu::~CImMenu()
{
}

void CImMenu::ImDraw()
{
	bool open = true;
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
	if (ImGui::Begin("##MAINMENU", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Columns(2, "##MAINCOL", false);
		{
			if (ImGui::BeginChild("##MAINQUEUE", ImVec2(0, 0), false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
			{
				int width = ImGui::GetWindowContentRegionWidth();

				if (ImGui::ListBoxHeader("##IMQUEUE", ImVec2(width, 170)))
				{
					ImGui::ListBoxFooter();
				}


				static float progress = .5f;

				ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));

				float progress_saturated = (progress < 0.0f) ? 0.0f : (progress > 1.0f) ? 1.0f : progress;
				char buf[32];
				sprintf(buf, "%d/%d", (int)(progress_saturated * 1753), 1753);
				ImGui::ProgressBar(progress, ImVec2(-1.f, 0.f), buf);

				static char searchbuff[128] = { "search tags..." };
				ImGui::PushItemWidth(width - 40);
				ImGui::InputText("##SEARCHBOX", searchbuff, ARRAYSIZE(searchbuff));
				ImGui::PopItemWidth();
				ImGui::SameLine();
				ImGui::Button("(null)", ImVec2(38, 25));

				ImGui::Button("Pause Search", ImVec2(width / 2, 25));
				ImGui::SameLine();
				ImGui::Button("Resume Search", ImVec2(width / 2, 25));

				ImGui::Button("Pause Download", ImVec2(width / 2, 25));
				ImGui::SameLine();
				ImGui::Button("Resume Download", ImVec2(width / 2, 25));

				ImGui::Button("Cancel", ImVec2(width / 2, 25));
				ImGui::SameLine();
				ImGui::Button("Start", ImVec2(width / 2, 25));


				ImGui::EndChild();
			}
		}
		ImGui::NextColumn();
		{
			if (ImGui::BeginChild("##GALLERY", ImVec2(0, 0), false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
			{
				int width = ImGui::GetWindowContentRegionWidth();
				int height = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;

				if (ImGui::ListBoxHeader("##PREVIEWGAL", ImVec2(width, height)))
				{

					ImGui::ListBoxFooter();
				}

				ImGui::EndChild();
			}
		}

		ImGui::End();
	}
}

#pragma warning(pop)