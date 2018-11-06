#include "ModuleLogView.h"
#include "Log.h"
#include "imgui/imgui.h"

bool ModuleLogView::init()
{
	g_Log.addOutput(this);

	return true;
}

bool ModuleLogView::updateGUI()
{
	ImGui::Begin("Log View");

	for (auto i = 0U; i < allMessages.size(); ++i)
	{
		const char *line = allMessages[i].c_str();

		ImGui::TextWrapped("%s", line);
	}
	
	ImGui::End();

	return true;
}

void ModuleLogView::writeMessage(const std::string & message)
{
	allMessages.push_back(message);
}
