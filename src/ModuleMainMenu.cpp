#include "ModuleMainMenu.h"
#include "ModuleAgentContainer.h"
#include "ModuleNodeCluster.h"
#include "ModuleYellowPages.h"
#include "Application.h"
#include "Log.h"
#include "imgui/imgui.h"

bool ModuleMainMenu::updateGUI()
{
	ImGui::Begin("Main menu");

	if (ImGui::Button("Node cluster"))
	{
		setEnabled(false);
		App->agentContainer->setEnabled(true);
		App->modNodeCluster->setEnabled(true);
	}

	if (ImGui::Button("Yellow Pages"))
	{
		setEnabled(false);
		App->modYellowPages->setEnabled(true);
	}

	ImGui::End();

	return true;
}
