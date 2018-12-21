#include "ModuleNodeCluster.h"
#include "ModuleNetworkManager.h"
#include "ModuleAgentContainer.h"
#include "Application.h"
#include "Log.h"
#include "Packets.h"
#include "imgui/imgui.h"
#include <sstream>
#include "ModuleTextures.h"

#include <d3d9.h>
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)


enum State {
	STOPPED,
	STARTING,
	RUNNING,
	STOPPING
};

bool ModuleNodeCluster::init()
{
	state = STOPPED;

	return true;
}

bool ModuleNodeCluster::start()
{
	state = STARTING;

	//IMG
	BackgroundEmpty = App->modTextures->BackgroundEmpty;
	Selector = App->modTextures->Selector;

	return true;
}

bool ModuleNodeCluster::update()
{
	bool ret = true;

	switch (state)
	{
	case STARTING:
		if (startSystem()) {
			state = RUNNING;
		} else {
			state = STOPPED;
			ret = false;
		}
		break;
	case RUNNING:
		runSystem();
		break;
	case STOPPING:
		stopSystem();
		state = STOPPED;
		break;
	}

	return ret;
}

bool ModuleNodeCluster::updateGUI()
{
	ImGui::Begin("Node cluster");
	ImGui::ShowDemoWindow();
	if (state == RUNNING)
	{
		// Number of sockets
		App->networkManager->drawInfoGUI();

		// Number of agents
		App->agentContainer->drawInfoGUI();

		ImGui::CollapsingHeader("ModuleNodeCluster", ImGuiTreeNodeFlags_DefaultOpen);

		int itemsCount = 0;
		for (auto node : _nodes) {
			itemsCount += (int)node->itemList().numItems();
		}
		ImGui::TextWrapped("# items in the cluster: %d", itemsCount);

		int missingItemsCount = 0;
		for (auto node : _nodes) {
			missingItemsCount += (int)node->itemList().numMissingItems();
		}
		ImGui::TextWrapped("# missing items in the cluster: %d", missingItemsCount);

		ImGui::Separator();

		if (ImGui::Button("Create random MCCs"))
		{
			for (NodePtr node : _nodes)
			{
				for (ItemId contributedItem = 0; contributedItem < MAX_ITEMS; ++contributedItem)
				{
					if (node->itemList().numItemsWithId(contributedItem) > 0 && GetTypeFromID(contributedItem) != node->GetType())
					{
						//int randNum = 0;
						for (ItemId constraintItem = 0; constraintItem < MAX_ITEMS; ++constraintItem)
						{
							if (node->itemList().numItemsWithId(constraintItem) == 0 /*&& GetTypeFromID(constraintItem) == node->GetType()*/)
							{
								//randNum++;
								//
								spawnMCC(node->id(), contributedItem, constraintItem);
							}
						}
						//int rands = rand() % randNum + 1;
						//int randNum2 = 0;
						//for (ItemId constraintItem = 0; constraintItem < MAX_ITEMS; ++constraintItem)
						//{
						//	if (node->itemList().numItemsWithId(constraintItem) == 0)
						//	{
						//		randNum2++;
						//		if (randNum2 == randNum)
						//		{
						//			spawnMCC(node->id(), contributedItem, constraintItem);
						//		}
						//	}
						//}
					}
				}
			}
		}

		if (ImGui::Button("Clear all agents"))
		{
			for (AgentPtr agent : App->agentContainer->allAgents())
			{
				agent->stop();
			}
		}

		ImGui::Separator();

		int nodeId = 0;
		for (auto &node : _nodes)
		{
			ImGui::PushID(nodeId);

			ImGuiTreeNodeFlags flags = 0;
			std::string nodeLabel = StringUtils::Sprintf("Node %d", nodeId);

			if (ImGui::CollapsingHeader(nodeLabel.c_str(), flags))
			{
				if (ImGui::TreeNodeEx("Items", flags))
				{
					auto &itemList = node->itemList();

					for (int itemId = 0; itemId < MAX_ITEMS; ++itemId)
					{
						unsigned int numItems = itemList.numItemsWithId(itemId);
						if (numItems == 1)
						{
							ImGui::Text("Item %d", itemId);
						}
						else if (numItems > 1)
						{
							ImGui::Text("Item %d (x%d)", itemId, numItems);
						}
					}

					ImGui::TreePop();
				}

				if (ImGui::TreeNodeEx("MCCs", flags))
				{
					for (auto agent : App->agentContainer->allAgents()) {
						MCC * mcc = agent->asMCC();
						if (mcc != nullptr && mcc->node()->id() == nodeId)
						{
							ImGui::Text("MCC %d", mcc->id());
							ImGui::Text(" - Contributed Item ID: %d", mcc->contributedItemId());
							ImGui::Text(" - Constraint Item ID: %d", mcc->constraintItemId());
						}
					}
					ImGui::TreePop();
				}

				if (ImGui::TreeNodeEx("MCPs", flags))
				{
					for (auto agent : App->agentContainer->allAgents()) {
						MCP * mcp = agent->asMCP();
						if (mcp != nullptr && mcp->node()->id() == nodeId)
						{
							ImGui::Text("MCP %d", mcp->id());
							ImGui::Text(" - Requested Item ID: %d", mcp->requestedItemId());
							ImGui::Text(" - Contributed Item ID: %d", mcp->contributedItemId());
						}
					}
					ImGui::TreePop();
				}
			}

			ImGui::PopID();
			nodeId++;
		}
	}

	ImGui::End();

	if (state == RUNNING)
	{
		// NODES / ITEMS MATRIX /////////////////////////////////////////////////////////

		ImGui::Begin("Nodes/Items Matrix");

		static ItemId selectedItem = 0;
		static unsigned int selectedNode = 0;
		static int comboItem = 0;

		ImGui::Text("Item ID ");
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.6f, 1.0f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 1.0f, 0.5f));

		for (ItemId itemId = 0U; itemId < MAX_ITEMS; ++itemId)
		{
			if (itemId == 4)
			{
				ImGui::PopStyleColor(3);
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.6f, 1.0f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 1.0f, 0.5f));
			}
			else if (itemId == 8)
			{
				ImGui::PopStyleColor(3);
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.6f, 1.0f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 1.0f, 0.5f));
			}

			ImGui::SameLine();
			std::ostringstream oss;
			oss << itemId;
			ImGui::Button(oss.str().c_str(), ImVec2(20, 20));
			if (itemId < MAX_ITEMS - 1) ImGui::SameLine();
		}
		ImGui::PopStyleColor(3);

		ImGui::Separator();

		for (auto nodeIndex = 0U; nodeIndex < _nodes.size(); ++nodeIndex)
		{
			ImGui::Text("Node %02u ", nodeIndex);
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(_nodes[nodeIndex]->GetName().c_str());
				ImGui::EndTooltip();
			}
			ImGui::SameLine();

			for (ItemId itemId = 0U; itemId < MAX_ITEMS; ++itemId)
			{
				unsigned int numItems = _nodes[nodeIndex]->itemList().numItemsWithId(itemId);

				if (numItems == 0)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.2f));
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 1.0f, 0.5f*numItems));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(.0f, 1.0f, 1.0f, 0.3f*numItems));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.0f, 1.0f, 1.0f, 0.2f*numItems));
				}

				const int buttonId = nodeIndex * MAX_ITEMS + itemId;
				std::ostringstream oss;
				oss << numItems;
				oss << "##" << buttonId;

				if (ImGui::Button(oss.str().c_str(), ImVec2(20, 20)))
				{
					if (numItems == 0)
					{
						selectedNode = nodeIndex;
						selectedItem = itemId;
						comboItem = 0;
						ImGui::OpenPopup("ItemOps");
					}
				}

				ImGui::PopStyleColor(3);

				if (itemId < MAX_ITEMS - 1) ImGui::SameLine();
			}
		}

		// Context menu to spawn agents
		if (ImGui::BeginPopup("ItemOps"))
		{
			int numberOfItems = _nodes[selectedNode]->itemList().numItemsWithId(selectedItem);

			// If it is a missing item...
			if (numberOfItems == 0)
			{
				int requestedItem = selectedItem;

				// Check if we have spare items
				std::vector<std::string> comboStrings;
				std::vector<int> itemIds;
				for (ItemId itemId = 0; itemId < MAX_ITEMS; ++itemId) {
					if (_nodes[selectedNode]->itemList().numItemsWithId(itemId) > 1)
					{
						std::ostringstream oss;
						oss << itemId;
						comboStrings.push_back(oss.str());
						itemIds.push_back(itemId);
					}
				}

				std::vector<const char *> comboCStrings;
				for (auto &s : comboStrings) { comboCStrings.push_back(s.c_str()); }

				if (itemIds.size() > 0)
				{
					ImGui::Text("Create MultiCastPetitioner?");
					ImGui::Separator();
					ImGui::Text("Node %d", selectedNode);
					ImGui::Text(" - Petition: %d", requestedItem);

					ImGui::Combo("Contribution", &comboItem, (const char **)&comboCStrings[0], (int)comboCStrings.size());
					if (ImGui::Button("Spawn MCP")) {
						int contributedItem = itemIds[comboItem];
						spawnMCP(selectedNode, requestedItem, contributedItem);
						ImGui::CloseCurrentPopup();
					}
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 0.0, 0.0, 1.0));
					ImGui::Text("No spare items available to create an MCP.");
					ImGui::PopStyleColor(1);
				}
			}

			ImGui::EndPopup();
		}

		ImGui::End();

		if (showInitialInfo)
		{
			ImGui::OpenPopup("showInitialInfo");
		}
		if (ImGui::BeginPopupModal("showInitialInfo", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Hello this is a our Project, in this project \nyou can test our SiSiMEX project!\n\n");
			ImGui::Text("---");
			ImGui::Text("In this demo, you have different players\nto make interchanges between them.\nEach player has his own type (Agility/Power/Intelligence)\nand the objective is to complete the specific armor set.\nLet's go and try it!\n\n");
			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); showInitialInfo = false; }
			ImGui::EndPopup();
		}

		ImGui::Begin("Character Menu", nullptr, ImGuiWindowFlags_NoCollapse);
		ImGui::Separator();

		static int user_selected = 0;
		static int requestid_current = -1;
		static int offer_current = -1;

		ImGui::Text("Select a User:");
		const char* items[] = { "Jordi", "Elliot", "Marc", "Kyra", "Jhon", "Xavier" };

		if (ImGui::Combo("##Users: ", &user_selected, items, IM_ARRAYSIZE(items)))
		{
			requestid_current = -1;
			offer_current = -1;
		}

		// Info Player ------------------------
		ImGui::Text("");
		ImGui::Text("User Info -----------------------------");
		ImGui::TextWrapped("User Name: "); ImGui::SameLine();
		ImGui::TextWrapped(items[user_selected]);
		if (_nodes[user_selected]->GetType() == TypeUser::AGILITY)
		{
			ImGui::Text("User Type: "); ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
			ImGui::TextWrapped("Agility");
			ImGui::PopStyleColor();
		}
		if (_nodes[user_selected]->GetType() == TypeUser::FORCE)
		{
			ImGui::Text("User Type: "); ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
			ImGui::TextWrapped("Power");
			ImGui::PopStyleColor();
		}
		if (_nodes[user_selected]->GetType() == TypeUser::INTELLIGENCE)
		{
			ImGui::Text("User Type: "); ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
			ImGui::TextWrapped("Intelligence");
			ImGui::PopStyleColor();
		}
		ImGui::TextWrapped("Armor Status: "); ImGui::SameLine();
		int numSelectablesSameType = 0;
		for (int n = 0; n < MAX_ITEMS; n++)
		{
			char buf[32];
			sprintf(buf, GetStringFromID_Type(n, _nodes[user_selected]->GetType()).c_str(), n);
			if (strcmp(buf, "Error...") != 0)
			{
				if (_nodes[user_selected]->itemList().numItemsWithId(n) == 0)
				{
					numSelectablesSameType++;
				}
			}
		}
		if (numSelectablesSameType == 0)
		{
			ImGui::TextColored(ImVec4(0.015, 0.77, 1, 1), "Completed");
		}
		else
		{
			ImGui::TextColored(ImVec4(1, 0.603, 0, 1), "Uncompleted");
		}
		ImGui::Text("");
		ImGui::Text("Items Info ---------------------------- ");
		ImGui_TextIDColor(0, user_selected);
		ImGui::Separator();




		// --------------------------------
		ImGui::Text("Create MultiCast Petitioner ------------------------");
		ImGui::Text("");
		ImGui::AlignFirstTextHeightToWidgets();
		ImGui::Text("Select a Request: "); ImGui::SameLine();
		//
		std::string nameButton = "Select Item";
		if (requestid_current != -1)
		{
			nameButton = GetStringFromID(requestid_current);
		}
		if (ImGui::Button(nameButton.c_str(), ImVec2(200, 20)))
			ImGui::OpenPopup("SelectRequest");


		if (ImGui::BeginPopup("SelectRequest"))
		{
			int numSelectables = 0;
			for (int n = 0; n < MAX_ITEMS; n++)
			{
				char buf[32];
				//std::string itemRequest = ;
				sprintf(buf, GetStringFromID_Type(n, _nodes[user_selected]->GetType()).c_str(), n);
				if (strcmp(buf, "Error...") != 0)
				{
					if (_nodes[user_selected]->itemList().numItemsWithId(n) == 0)
					{
						numSelectables++;
						if (ImGui::Selectable(buf, requestid_current == n))
							requestid_current = n;
					}
				}
			}
			if (numSelectables == 0)
			{
				if (ImGui::Selectable("You can't Request more Items", requestid_current == -1, ImGuiSelectableFlags_Disabled))
					requestid_current = -1;
			}
			ImGui::EndPopup();
		}

		// ---------------------------------------
		ImGui::Text("");
		ImGui::AlignFirstTextHeightToWidgets();
		ImGui::Text("Select a Offer: "); ImGui::SameLine();
		//
		std::string nameButton2 = "Select Offer";
		if (offer_current != -1)
		{
			nameButton2 = GetStringFromID(offer_current);
		}
		if (ImGui::Button(nameButton2.c_str(), ImVec2(200, 20)))
			ImGui::OpenPopup("SelectOffer");


		if (ImGui::BeginPopup("SelectOffer"))
		{
			for (int n = 0; n < MAX_ITEMS; n++)
			{
				char buf[32];
				sprintf(buf, GetStringFromID(n).c_str(), n);
				if (_nodes[user_selected]->itemList().numItemsWithId(n) > 0)
				{
					if (ImGui::Selectable(buf, offer_current == n))
						offer_current = n;
				}
			}
			ImGui::EndPopup();
		}
		ImGui::Text(""); ImGui::SameLine(420);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
		bool openButton = false;
		if (ImGui::Button("Spawn MCP", ImVec2(124, 40)))
		{
			openButton = true;
		}
		if (requestid_current == -1 || offer_current == -1)
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				std::string text = "First you need to select:";
				if (requestid_current == -1)
					text += "\n-Request item";
				if (offer_current == -1)
					text += "\n-Offer Item";
				ImGui::TextUnformatted(text.c_str());
				ImGui::EndTooltip();
			}
			openButton = false;
		}
		if (openButton)
		{
			spawnMCP(user_selected, requestid_current, offer_current);
			_nodes[user_selected]->itemList().UpdateItemUsed(requestid_current, true);
			requestid_current = -1;
			offer_current = -1;
		}
		ImGui::PopStyleColor();

		ImGui::End();
	}

	return true;
}

int ModuleNodeCluster::GetIDFromString(std::string nameItem)
{
	if (nameItem == "Uliana's Spirit")
	{
		return 0;
	}
	else if (nameItem == "Uliana's Heart")
	{
		return 1;
	}
	else if (nameItem == "Uliana's Fury")
	{
		return 2;
	}
	else if (nameItem == "Uliana's Destiny")
	{
		return 3;
	}
	else if (nameItem == "Pestilence Mask")
	{
		return 4;
	}
	else if (nameItem == "Pestilence Defense")
	{
		return 5;
	}
	else if (nameItem == "Pestilence Incantations")
	{
		return 6;
	}
	else if (nameItem == "Pestilence Boots")
	{
		return 7;
	}
	else if (nameItem == "Crown of the Light")
	{
		return 8;
	}
	else if (nameItem == "Heart of the Light")
	{
		return 9;
	}
	else if (nameItem == "Towers of the Light")
	{
		return 10;
	}
	else if (nameItem == "Foundation of the Light")
	{
		return 11;
	}
	return 0;
}

std::string ModuleNodeCluster::GetStringFromID(int idItem)
{
	switch (idItem)
	{
	case 0:
	{
		return "Uliana's Spirit";
		break;
	}
	case 1:
	{
		return "Uliana's Heart";
		break;
	}
	case 2:
	{
		return "Uliana's Fury";
		break;
	}
	case 3:
	{
		return "Uliana's Destiny";
		break;
	}
	case 4:
	{
		return "Pestilence Mask";
		break;
	}
	case 5:
	{
		return "Pestilence Defense";
		break;
	}
	case 6:
	{
		return "Pestilence Incantations";
		break;
	}
	case 7:
	{
		return "Pestilence Boots";
		break;
	}
	case 8:
	{
		return "Crown of the Light";
		break;
	}
	case 9:
	{
		return "Heart of the Light";
		break;
	}
	case 10:
	{
		return "Towers of the Light";
		break;
	}
	case 11:
	{
		return "Foundation of the Light";
		break;
	}
	}
	return "Error...";
}

std::string ModuleNodeCluster::GetStringFromID_Type(int idItem, TypeUser type)
{
	if (type == TypeUser::AGILITY)
	{
		switch (idItem)
		{
		case 0:
		{
			return "Uliana's Spirit";
			break;
		}
		case 1:
		{
			return "Uliana's Heart";
			break;
		}
		case 2:
		{
			return "Uliana's Fury";
			break;
		}
		case 3:
		{
			return "Uliana's Destiny";
			break;
		}
		}
	}
	if (type == TypeUser::INTELLIGENCE)
	{
		switch (idItem)
		{
		case 4:
		{
			return "Pestilence Mask";
			break;
		}
		case 5:
		{
			return "Pestilence Defense";
			break;
		}
		case 6:
		{
			return "Pestilence Incantations";
			break;
		}
		case 7:
		{
			return "Pestilence Boots";
			break;
		}
		}
	}
	if (type == TypeUser::FORCE)
	{
		switch (idItem)
		{
		case 8:
		{
			return "Crown of the Light";
			break;
		}
		case 9:
		{
			return "Heart of the Light";
			break;
		}
		case 10:
		{
			return "Towers of the Light";
			break;
		}
		case 11:
		{
			return "Foundation of the Light";
			break;
		}
		}
	}
	return "Error...";
}

TypeUser ModuleNodeCluster::GetTypeFromID(int idItem)
{
	if (idItem >= 0 && idItem <= 3)
	{
		return TypeUser::AGILITY;
	}
	if (idItem >= 4 && idItem <= 7)
	{
		return TypeUser::INTELLIGENCE;
	}
	if (idItem >= 7 && idItem <= 11)
	{
		return TypeUser::FORCE;
	}
	return TypeUser::DEFAULT;
}

void ModuleNodeCluster::ImGui_TextIDColor(int id, int user_selected)
{
	for (int i = 0; i < MAX_ITEMS; i++)
	{
		if (strcmp(GetStringFromID(_nodes[user_selected]->itemList().numItemsWithId(i)).c_str(), "Error...") == 0)
		{
			continue;
		}
		if (_nodes[user_selected]->itemList().numItemsWithId(i) == 0)
		{
			continue;
		}
		//ImGui::Text("");
		ImGui::Separator();

		ImGui::Bullet();
		ImGui::Text("Item ID:   "); ImGui::SameLine();
		if (GetTypeFromID(i) == TypeUser::AGILITY)
		{
			ImGui::TextColored(ImVec4(0, 1, 0, 1), std::to_string(i).c_str());
			ImGui::Bullet();
			ImGui::Text("Name:      "); ImGui::SameLine();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), GetStringFromID(i).c_str());
			ImGui::Bullet();
			ImGui::Text("Ammount:   "); ImGui::SameLine();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), std::to_string(_nodes[user_selected]->itemList().numItemsWithId(i)).c_str());
		}
		if (GetTypeFromID(i) == TypeUser::FORCE)
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), std::to_string(i).c_str());
			ImGui::Bullet();
			ImGui::Text("Name:      "); ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 0, 0, 1), GetStringFromID(i).c_str());
			ImGui::Bullet();
			ImGui::Text("Ammount:   "); ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 0, 0, 1), std::to_string(_nodes[user_selected]->itemList().numItemsWithId(i)).c_str());
		}
		if (GetTypeFromID(i) == TypeUser::INTELLIGENCE)
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), std::to_string(i).c_str());
			ImGui::Bullet();
			ImGui::Text("Name:      "); ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 1, 0, 1), GetStringFromID(i).c_str());
			ImGui::Bullet();
			ImGui::Text("Ammount:   "); ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 1, 0, 1), std::to_string(_nodes[user_selected]->itemList().numItemsWithId(i)).c_str());
		}
		ImGui::Spacing();
	}
}

bool ModuleNodeCluster::stop()
{
	state = STOPPING;

	return true;
}

bool ModuleNodeCluster::cleanUp()
{
	return true;
}

void ModuleNodeCluster::OnAccepted(TCPSocketPtr socket)
{
	// Nothing to do
}

void ModuleNodeCluster::OnPacketReceived(TCPSocketPtr socket, InputMemoryStream & stream)
{
	//iLog << "OnPacketReceived";

	PacketHeader packetHead;
	packetHead.Read(stream);

	// Get the agent
	auto agentPtr = App->agentContainer->getAgent(packetHead.dstAgentId);
	if (agentPtr != nullptr)
	{
		agentPtr->OnPacketReceived(socket, packetHead, stream);
	}
	else
	{
		eLog << "Couldn't find agent: " << packetHead.dstAgentId;
	}
}

void ModuleNodeCluster::OnDisconnected(TCPSocketPtr socket)
{
	// Nothing to do
}

void ModuleNodeCluster::ShowHelpMarker(const char* desc, const char* icon)
{
	ImGui::TextDisabled(icon);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(450.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

bool ModuleNodeCluster::startSystem()
{
	iLog << "--------------------------------------------";
	iLog << "           SiSiMEX: Node Cluster            ";
	iLog << "--------------------------------------------";
	iLog << "";

	// Create listen socket
	TCPSocketPtr listenSocket = SocketUtil::CreateTCPSocket(SocketAddressFamily::INET);
	if (listenSocket == nullptr) {
		eLog << "SocketUtil::CreateTCPSocket() failed";
		return false;
	}
	iLog << " - Server Listen socket created";

	// Bind
	const int port = LISTEN_PORT_AGENTS;
	SocketAddress bindAddress(port); // localhost:LISTEN_PORT_AGENTS
	listenSocket->SetReuseAddress(true);
	int res = listenSocket->Bind(bindAddress);
	if (res != NO_ERROR) { return false; }
	iLog << " - Socket Bind to interface 127.0.0.1:" << LISTEN_PORT_AGENTS;

	// Listen mode
	res = listenSocket->Listen();
	if (res != NO_ERROR) { return false; }
	iLog << " - Socket entered in Listen state...";

	// Add the socket to the manager
	App->networkManager->SetDelegate(this);
	App->networkManager->AddSocket(listenSocket);


	NodePtr node = std::make_shared<Node>(0, "Jordi", TypeUser::INTELLIGENCE);
	node->itemList().initializeComplete();
	_nodes.push_back(node);
	node->itemList().addItem(0);
	node->itemList().addItem(2);
	node->itemList().addItem(3);
	node->itemList().addItem(5);
	node->itemList().addItem(7);
	node->itemList().addItem(8);
	node->itemList().addItem(10);
	node->itemList().addItem(11);

	node = std::make_shared<Node>(1, "Elliot", TypeUser::FORCE);
	node->itemList().initializeComplete();
	_nodes.push_back(node);
	node->itemList().addItem(0);
	node->itemList().addItem(1);
	node->itemList().addItem(2);
	node->itemList().addItem(4);
	node->itemList().addItem(6);
	node->itemList().addItem(7);
	node->itemList().addItem(10);


	node = std::make_shared<Node>(2, "Marc", TypeUser::AGILITY);
	node->itemList().initializeComplete();
	_nodes.push_back(node);
	node->itemList().addItem(1);
	node->itemList().addItem(3);
	node->itemList().addItem(4);
	node->itemList().addItem(5);
	node->itemList().addItem(7);
	node->itemList().addItem(9);
	node->itemList().addItem(11);


	node = std::make_shared<Node>(3, "Kyra", TypeUser::FORCE);
	node->itemList().initializeComplete();
	_nodes.push_back(node);
	node->itemList().addItem(0);
	node->itemList().addItem(1);
	node->itemList().addItem(3);
	node->itemList().addItem(4);
	node->itemList().addItem(6);
	node->itemList().addItem(7);
	node->itemList().addItem(8);
	node->itemList().addItem(11);

	node = std::make_shared<Node>(4, "Jhon", TypeUser::INTELLIGENCE);
	node->itemList().initializeComplete();
	_nodes.push_back(node);
	node->itemList().addItem(0);
	node->itemList().addItem(1);
	node->itemList().addItem(2);
	node->itemList().addItem(3);
	node->itemList().addItem(5);
	node->itemList().addItem(8);
	node->itemList().addItem(9);
	node->itemList().addItem(10);

	node = std::make_shared<Node>(5, "Xavier", TypeUser::AGILITY);
	node->itemList().initializeComplete();
	_nodes.push_back(node);
	node->itemList().addItem(2);
	node->itemList().addItem(4);
	node->itemList().addItem(5);
	node->itemList().addItem(6);
	node->itemList().addItem(9);
	node->itemList().addItem(10);
	node->itemList().addItem(11);

	return true;
}

void ModuleNodeCluster::runSystem()
{
	// Check the results of agents
	for (AgentPtr agent : App->agentContainer->allAgents())
	{
		if (!agent->isValid()) { continue; }

		// Update ItemList with finalized MCCs
		MCC *mcc = agent->asMCC();
		if (mcc != nullptr && mcc->negotiationFinished())
		{
			Node *node = mcc->node();
			node->itemList().removeItem(mcc->contributedItemId());
			node->itemList().addItem(mcc->constraintItemId());
			mcc->stop();
			iLog << "MCC exchange at Node " << node->id() << " ("
				<< node->GetName() << ") :"
				<< " -" << mcc->contributedItemId()
				<< " +" << mcc->constraintItemId();
		}

		// Update ItemList with MCPs that found a solution
		MCP *mcp = agent->asMCP();
		if (mcp != nullptr && mcp->negotiationFinished() && mcp->searchDepth() == 0)
		{
			Node *node = mcp->node();

			if (mcp->negotiationAgreement())
			{
				node->itemList().addItem(mcp->requestedItemId());
				node->itemList().removeItem(mcp->contributedItemId());
				iLog << "MCP exchange at Node " << node->id() << " ("
					<< node->GetName() << ") :"
					<< " -" << mcp->contributedItemId()
					<< " +" << mcp->requestedItemId();
			}
			else
			{
				wLog << "MCP exchange at Node " << node->id() << " (" << node->GetName() << ") not found:"
					<< " -" << mcp->contributedItemId()
					<< " +" << mcp->requestedItemId();
			}
			mcp->stop();
		}
	}

	// WARNING:
	// The list of items of each node can change at any moment if a multilateral exchange took place
	// The following lines looks for agents which, after an update of items, make sense no more, and stops them
	for (AgentPtr agent : App->agentContainer->allAgents())
	{
		if (!agent->isValid()) { continue; }

		Node *node = agent->node();
		MCC *mcc = agent->asMCC();
		if (mcc != nullptr && mcc->isIdling())
		{
			int numContributedItems = node->itemList().numItemsWithId(mcc->contributedItemId());
			int numRequestedItems = node->itemList().numItemsWithId(mcc->constraintItemId());
			if (numContributedItems < 1 || numRequestedItems > 0) 
			{ // if the contributed is not repeated at least once... or we already got the constraint
				mcc->stop();
			}
		}
	}
}

void ModuleNodeCluster::stopSystem()
{
}

void ModuleNodeCluster::spawnMCP(int nodeId, int requestedItemId, int contributedItemId)
{
	dLog << "Spawn MCP - node " << nodeId << " - req. " << requestedItemId << " - contrib. " << contributedItemId;
	if (nodeId >= 0 && nodeId < (int)_nodes.size()) {
		NodePtr node = _nodes[nodeId];
		App->agentContainer->createMCP(node.get(), requestedItemId, contributedItemId, 0, 0);
	}
	else {
		wLog << "Could not find node with ID " << nodeId;
	}
}

void ModuleNodeCluster::spawnMCC(int nodeId, int contributedItemId, int constraintItemId)
{
	dLog << "Spawn MCC - node " << nodeId << " contrib. " << contributedItemId << " - constr. " << constraintItemId;
	if (nodeId >= 0 && nodeId < (int)_nodes.size()) {
		NodePtr node = _nodes[nodeId];
		App->agentContainer->createMCC(node.get(), contributedItemId, constraintItemId);
	}
	else {
		wLog << "Could not find node with ID " << nodeId;
	}
}
