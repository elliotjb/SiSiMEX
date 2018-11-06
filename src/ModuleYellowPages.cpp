#include "ModuleYellowPages.h"
#include "ModuleNetworkManager.h"
#include "Application.h"
#include "Packets.h"
#include "Log.h"
#include "imgui/imgui.h"

enum State {
	STOPPED,
	STARTING,
	RUNNING,
	STOPPING
};

bool ModuleYellowPages::init()
{
	state = STOPPED;

	return true;
}

bool ModuleYellowPages::start()
{
	state = STARTING;

	return true;
}

bool ModuleYellowPages::update()
{
	bool ret = true;

	switch (state)
	{
	case STARTING:
		if (startService()) {
			state = RUNNING;
		} else {
			state = STOPPED;
			ret = false;
		}
		break;
	case RUNNING:
		break;
	case STOPPING:
		stopService();
		state = STOPPED;
		break;
	}

	return ret;
}

bool ModuleYellowPages::updateGUI()
{
	ImGui::Begin("Yellow Pages");

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
	if (ImGui::CollapsingHeader("Registered MCCs", flags))
	{
		int imguiId = 0;
		for (auto &mcc : _mccByItem)
		{
			auto itemId = mcc.first;
			auto &agentLocations = mcc.second;

			if (ImGui::TreeNodeEx((void*)imguiId++, flags, "MCCs for item %d", (int)itemId))
			{
				for (auto &agentLocation : agentLocations)
				{
					ImGui::Text(" - %s:%d - agent:%d", agentLocation.hostIP.c_str(), agentLocation.hostPort, agentLocation.agentId);
				}

				ImGui::TreePop();
			}
		}
	}

	ImGui::End();

	return true;
}

bool ModuleYellowPages::stop()
{
	state = STOPPING;

	return true;
}

bool ModuleYellowPages::startService()
{
	iLog << "---------------------------------------------------";
	iLog << "              SiSiMEX: Yellow Pages                ";
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
	const int port = LISTEN_PORT_YP;
	SocketAddress bindAddress(port); // localhost:LISTEN_PORT_YP
	listenSocket->SetReuseAddress(true);
	int res = listenSocket->Bind(bindAddress);
	if (res != NO_ERROR) { return false; }
	iLog << " - Socket Bind to interface 127.0.0.1:" << LISTEN_PORT_YP;

	// Listen mode
	res = listenSocket->Listen();
	if (res != NO_ERROR) { return false; }
	iLog << " - Socket entered in Listen state...";

	// Add the socket to the manager
	App->networkManager->SetDelegate(this);
	App->networkManager->AddSocket(listenSocket);

	return true;
}

void ModuleYellowPages::stopService()
{
	// Nothing to do
}

void ModuleYellowPages::OnAccepted(TCPSocketPtr socket)
{
	// Nothing to do
}

void ModuleYellowPages::OnPacketReceived(TCPSocketPtr socket, InputMemoryStream &stream)
{
	iLog << "OnPacketReceived: ";

	// TODO: Deserialize PacketHeader (Make it in Packets.h first)
	PacketHeader inPacketHead;
	inPacketHead.Read(stream);

	if (inPacketHead.packetType == PacketType::RegisterMCC)
	{
		iLog << "PacketType::RegisterMCC";

		// TODO: Deserialize PacketRegisterMCC (make it in Packets.h first)
		uint16_t itemId = NULL_ITEM_ID;
		PacketRegisterMCC inPacketData = PacketRegisterMCC();
		inPacketData.Read(stream);

		// Register the MCC into the yellow pages
		AgentLocation mcc;
		mcc.hostIP = socket->RemoteAddress().GetIPString();
		mcc.hostPort = LISTEN_PORT_AGENTS;
		mcc.agentId = inPacketHead.srcAgentId;
		_mccByItem[inPacketData.itemId].push_back(mcc);

		// Host address
		std::string hostAddress = socket->RemoteAddress().GetString();

		//Some logging
		iLog << " - MCC Agent ID: " << inPacketHead.srcAgentId;
		iLog << " - Contributed Item ID: " << inPacketData.itemId;
		iLog << " - Remote host address: " << hostAddress;

		// TODO: Serialize and send PacketRegisterMCCAck (make it in Packets.h first)
		// 1 - Create an OutputMemoryStream
		OutputMemoryStream stream_o = OutputMemoryStream();
		// 2 - Create a PacketHeader and fill it
		PacketHeader pck = PacketHeader();
		pck.packetType = PacketType::RegisterMCCAck;
		pck.srcAgentId = inPacketHead.dstAgentId;
		pck.dstAgentId = inPacketHead.srcAgentId;
		pck.Write(stream_o);
		// 3 - Send the packet through the socket
		socket->SendPacket(stream_o.GetBufferPtr(), stream_o.GetSize());
	}
	else if (inPacketHead.packetType == PacketType::UnregisterMCC)
	{
		iLog << "PacketType::UnregisterMCC";

		// TODO: Deserialize PacketUnregisterMCC (make it in Packets.h first)
		uint16_t itemId = NULL_ITEM_ID;
		PacketUnregisterMCC inPacketData = PacketUnregisterMCC();
		inPacketData.Read(stream);

		// Unregister the MCC from the yellow pages
		std::list<AgentLocation> &mccs(_mccByItem[itemId]);
		for (auto it = mccs.begin(); it != mccs.end();) {
			if (it->agentId == inPacketHead.srcAgentId) {
				auto oldIt = it++;
				mccs.erase(oldIt);
				break;
			}
			else {
				++it;
			}
		}

		iLog << " - MCC Agent ID: " << inPacketHead.srcAgentId;
		iLog << " - Contributed Item ID: " << inPacketData.itemId;

		// TODO: Serialize and send PacketRegisterMCCAck (make the packet in Packets.h first)
		// 1 - Create an OutputMemoryStream
		OutputMemoryStream stream_o = OutputMemoryStream();
		// 2 - Create a PacketHeader and fill it
		PacketHeader pck = PacketHeader();
		pck.packetType = PacketType::UnregisterMCCAck;
		pck.srcAgentId = inPacketHead.dstAgentId;
		pck.dstAgentId = inPacketHead.srcAgentId;
		pck.Write(stream_o);
		// 3 - Send the packet through the socket
		socket->SendPacket(stream_o.GetBufferPtr(), stream_o.GetSize());
	}
}

void ModuleYellowPages::OnDisconnected(TCPSocketPtr socket)
{
	// Nothing to do
	iLog << "Socket disconnected gracefully";
}
