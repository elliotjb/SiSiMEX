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

	ST_IDLE,



	ST_FINISHED = 10
};

UCP::UCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, const AgentLocation &uccLocation, unsigned int searchDepth, unsigned int totalSearch) :
	Agent(node),
	_requestedItemId(requestedItemId),
	_contributedItemId(contributedItemId),
	_searchDepth(searchDepth),
	_totalSearch(totalSearch)
{
	// TODO: Save input parameters
	_uccLocation = uccLocation;
	setState(ST_INIT);
	_negotiationAccepted = false;
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
	case ST_IDLE:
	{
		if (_mcp.get())
		{
			if (_mcp->state() == 10)
			{
				if (_mcp->NegotiationAccepted())
				{
					_negotiationAccepted = true;
					setState(ST_FINISHED);

					SendPacketToUCCAccept();
				}
				else
				{
					_negotiationAccepted = false;
					setState(ST_FINISHED);

					SendPacketToUCCAccept();
				}
			}
		}
		break;
	}
	case ST_FINISHED:
		break;
	default:
		break;
	}
}

void UCP::stop()
{
	// TODO: Destroy search hierarchy below this agent
	if(_mcp != nullptr)
		_mcp->stop();
	destroy();
}

void UCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
		// TODO: Handle packets
	case PacketType::RequestConstraintResponse:
		if (state() == ST_WAITING_ITEM_REQUEST || state() == ST_IDLE)
		{		
			PacketConstraintResponse packetData;
			packetData.Read(stream);

			if (packetData.itemId == contributedItemId())
			{
				iLog << " - Accept Negotation: ";
				_negotiationAccepted = true;
				setState(ST_FINISHED);

				PacketHeader outPacketHead;
				PacketAcceptNegotiation packetNegot;
				OutputMemoryStream outStream;

				outPacketHead.packetType = PacketType::AcceptNegotiation;
				outPacketHead.srcAgentId = id();
				outPacketHead.dstAgentId = packetHeader.srcAgentId;

				packetNegot.acceptedNegotiation = _negotiationAccepted;

				outPacketHead.Write(outStream);
				packetNegot.Write(outStream);

				socket->SendPacket(outStream.GetBufferPtr(), outStream.GetSize());
			}
			else
			{
				iLog << " - Search another Negotation: " << "Search Depth" << _searchDepth;
				if (_totalSearch >= MAX_SEARCH)
				{
					iLog << " - STOP SEARCH: ";
					setState(ST_FINISHED);
					PacketHeader outPacketHead;
					PacketAcceptNegotiation packetNegot;
					OutputMemoryStream outStream;

					outPacketHead.packetType = PacketType::AcceptNegotiation;
					outPacketHead.srcAgentId = id();
					outPacketHead.dstAgentId = packetHeader.srcAgentId;

					packetNegot.acceptedNegotiation = _negotiationAccepted;

					outPacketHead.Write(outStream);
					packetNegot.Write(outStream);

					socket->SendPacket(outStream.GetBufferPtr(), outStream.GetSize());
				}
				else if (_searchDepth == 3)
				{
					iLog << " - MAX SEARCH DEPTH: ";
					setState(ST_FINISHED);
					PacketHeader outPacketHead;
					PacketAcceptNegotiation packetNegot;
					OutputMemoryStream outStream;

					outPacketHead.packetType = PacketType::AcceptNegotiation;
					outPacketHead.srcAgentId = id();
					outPacketHead.dstAgentId = packetHeader.srcAgentId;

					packetNegot.acceptedNegotiation = _negotiationAccepted;

					outPacketHead.Write(outStream);
					packetNegot.Write(outStream);

					socket->SendPacket(outStream.GetBufferPtr(), outStream.GetSize());
				}
				else
				{
					setState(ST_IDLE);
					_searchDepth += 1;
					Node* newNode = new Node(App->agentContainer->allAgents().size());
					_mcp = App->agentContainer->createMCP(newNode, packetData.itemId, contributedItemId(), _searchDepth, _totalSearch);
				}
			}

			break;
		}
		else
		{
			wLog << "UCP 1 - OnPacketReceived() - PacketType::RequestConstraint was unexpected";
		}

	default:
		wLog << "UCP 2 - OnPacketReceived() - Unexpected PacketType.";
	}
}

bool UCP::SendPacketToUCC()
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::RequestConstraint;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = _uccLocation.agentId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);

	return sendPacketToAgent(_uccLocation.hostIP, _uccLocation.hostPort, stream);
}

bool UCP::SendPacketToUCCAccept()
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::AcceptNegotiation;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = _uccLocation.agentId;

	PacketAcceptNegotiation packetNegot;
	packetNegot.acceptedNegotiation = _negotiationAccepted;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetNegot.Write(stream);

	return sendPacketToAgent(_uccLocation.hostIP, _uccLocation.hostPort, stream);
}
