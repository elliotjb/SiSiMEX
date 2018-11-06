#include "ModuleNodeCluster.h"
#include "ModuleNetworkManager.h"
#include "ModuleAgentContainer.h"
#include "Application.h"
#include "Log.h"
#include "Packets.h"
#include "imgui/imgui.h"

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
		// TODO
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

	if (state == RUNNING)
	{
		int itemsCount = 0;
		for (auto node : _nodes) {
			itemsCount += (int)node->itemList().items().size();
		}
		ImGui::TextWrapped("# items in the cluster: %d", itemsCount);

		int missingItemsCount = 0;
		for (auto node : _nodes) {
			missingItemsCount += (int)node->itemList().getMissingItems().items().size();
		}
		ImGui::TextWrapped("# missing items in the cluster: %d", missingItemsCount);

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

					for (auto &item : itemList.items())
					{
						ImGui::Text("Item %d", item.id());
					}

					ImGui::TreePop();
				}

				if (ImGui::TreeNodeEx("MCCs", flags))
				{
					for (auto mcc : _mccs) {
						if (mcc->node()->id() == nodeId)
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
					static int itemId = 0;
					ImGui::InputInt("Request item", &itemId);
					if (ImGui::Button("Spawn MCP")) {
						// Not yet... next day ;-)
					}
					ImGui::TreePop();
				}
			}

			ImGui::PopID();
			nodeId++;
		}
	}

	ImGui::End();

	return true;
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
	iLog << "OnPacketReceived";

	// TODO 1: Declare a PacketHeader and deserialize it
	PacketHeader pck = PacketHeader();
	pck.Read(stream);

	// TODO 2: With the deserialized agent Id, get the agent in the system (ModuleAgentContainer::getAgent)
	AgentPtr agent = App->agentContainer->getAgent(pck.dstAgentId);

	// TODO 3: If the agent was found, redirect the input stream to its Agent::OnPacketReceived method
	if (agent != nullptr)
	{
		agent->OnPacketReceived(socket, pck, stream);
	}
}

void ModuleNodeCluster::OnDisconnected(TCPSocketPtr socket)
{
	// Nothing to do
}

bool ModuleNodeCluster::startSystem()
{
	iLog << "---------------------------------------------------";
	iLog << "               SiSiMEX: Node cluster               ";
	iLog << "---------------------------------------------------";
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

	//#define RANDOM
#ifdef RANDOM
	// Initialize nodes
	for (int i = 0; i < MAX_NODES; ++i)
	{
		// Create and intialize nodes
		NodePtr node = std::make_shared<Node>();
		node->initialize();
		_nodes.push_back(node);
	}

	// Create MCP agents for each node
	for (int i = 0; i < MAX_NODES; ++i)
	{
		// Spawn MCC (MultiCastContributors) one for each spare item
		NodePtr node = _nodes[i];
		ItemList spareItems = node->itemList().getSpareItems();
		for (auto item : spareItems.items()) {
			spawnMCC(i, item.id());
		}
	}
#else
	_nodes.push_back(std::make_shared<Node>((int)_nodes.size()));
	_nodes.push_back(std::make_shared<Node>((int)_nodes.size()));
	_nodes.push_back(std::make_shared<Node>((int)_nodes.size()));
	_nodes.push_back(std::make_shared<Node>((int)_nodes.size()));
	_nodes[0]->itemList().addItem(Item(0));
	_nodes[1]->itemList().addItem(Item(1));
	_nodes[2]->itemList().addItem(Item(2));
	_nodes[3]->itemList().addItem(Item(3));

	spawnMCC(1, 1, 2); // Node 1 offers 1 but wants 2
	spawnMCC(2, 2, 3); // Node 2 offers 2 but wants 3
	spawnMCC(3, 3, 0); // Node 3 offers 3 but wants 0

	spawnMCC(0, 0); // Node 0 offers 0
	//spawnMCP(0, 1); // Node 0 wants  1
#endif

	return true;
}

void ModuleNodeCluster::runSystem()
{
	// Check the results of agents
	std::vector<MCC*> mccsAlive;
	for (auto mcc : _mccs) {
		if (!mcc->isValid()) { continue; }
		if (mcc->negotiationFinished()) {
			Node *node = mcc->node();
			node->itemList().removeItem(mcc->contributedItemId());
			node->itemList().addItem(mcc->constraintItemId());
			mcc->stop();
		}
		else {
			mccsAlive.push_back(mcc);
		}
	}
	_mccs.swap(mccsAlive);
}

void ModuleNodeCluster::stopSystem()
{
}

void ModuleNodeCluster::spawnMCC(int nodeId, int contributedItemId, int constraintItemId)
{
	iLog << "Spawn MCC for node " << nodeId << " contributing item " << contributedItemId << " - constraint item " << constraintItemId;
	if (nodeId >= 0 && nodeId < (int)_nodes.size()) {
		NodePtr node = _nodes[nodeId];
		MCCPtr mcc = App->agentContainer->createMCC(node.get(), contributedItemId, constraintItemId);
		_mccs.push_back(mcc.get());
	}
	else {
		wLog << "Could not find node with ID " << nodeId;
	}
}
