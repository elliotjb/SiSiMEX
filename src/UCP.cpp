#include "UCP.h"
#include "MCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"


// TODO: Make an enum with the states
enum State
{
	ST_INIT,
	ST_WAITING_ITEM_REQUEST,
	ST_WAITING_ITEM_CONSTRAINT,
	ST_FINISHED
};

UCP::UCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, const AgentLocation &uccLocation, unsigned int searchDepth) :
	Agent(node),
	_requestedItemId(requestedItemId),
	_contributedItemId(contributedItemId),
	_searchDepth(searchDepth)
{
	// TODO: Save input parameters
	_uccLocation = uccLocation;
	setState(ST_INIT);
}

UCP::~UCP()
{
}

void UCP::update()
{
	switch (state())
	{
	case ST_INIT:
		SendPacketToUCC();
		setState(ST_WAITING_ITEM_REQUEST);
		break;
	case ST_WAITING_ITEM_REQUEST:
		break;
	case ST_WAITING_ITEM_CONSTRAINT:
		break;
	case ST_FINISHED:
		break;
	default:
		break;
	}
}

void UCP::stop()
{
	// TODO: Destroy search hierarchy below this agent

	destroy();
}

void UCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
		// TODO: Handle packets
	case PacketType::RequestConstraint:
		if (state() == ST_WAITING_ITEM_REQUEST)
		{

			break;
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::RequestConstraint was unexpected";
		}

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool UCP::SendPacketToUCC()
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::RequestItem;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = _uccLocation.agentId;

	//PacketItemRequest packetData;
	//packetData.itemId = requestedItemId();

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	//packetData.Write(stream);

	return sendPacketToAgent(_uccLocation.hostIP, _uccLocation.hostPort, stream);
}
