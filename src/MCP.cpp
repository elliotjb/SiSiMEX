#include "MCP.h"
#include "UCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"


enum State
{
	ST_INIT,
	ST_REQUESTING_MCCs,
	ST_ITERATING_OVER_MCCs,

	// TODO: Other states
	ST_WAITING_ACCEPTANCE,
	ST_NEGOTIATING,

	ST_NEGOTIATION_FINISHED = 10
};

MCP::MCP(Node *node, uint16_t requestedItemID, uint16_t contributedItemID, unsigned int searchDepth, unsigned int totalSearch) :
	Agent(node),
	_requestedItemId(requestedItemID),
	_contributedItemId(contributedItemID),
	_searchDepth(searchDepth),
	_totalSearch(totalSearch)
{
	setState(ST_INIT);
	_negotiationAccepted = false;
}

MCP::~MCP()
{
}

void MCP::update()
{
	switch (state())
	{
	case ST_INIT:
		queryMCCsForItem(_requestedItemId);
		setState(ST_REQUESTING_MCCs);
		break;

	case ST_ITERATING_OVER_MCCs:
		// TODO: Handle this state
		Requestnegotiation();
		break;

	// TODO: Handle other states
	case ST_REQUESTING_MCCs:
		break;

	case ST_WAITING_ACCEPTANCE:
		break;

	case ST_NEGOTIATING:
		if (_ucp.get())
		{
			if (_ucp.get()->state() == 10)
			{
				if (_ucp.get()->NegotiationAccepted())
				{
					// Negotiation Accepted -------
					_negotiationAccepted = true;
					setState(ST_NEGOTIATION_FINISHED);
				}
				else
				{
					if (_mccRegisterIndex < _mccRegisters.size() && _totalSearch < MAX_SEARCH)
					{
						_ucp.get()->stop();
						_mccRegisterIndex++;
						_totalSearch++;
						setState(ST_ITERATING_OVER_MCCs);
					}
					else
					{
						// Negotiation Canceled -------
						_negotiationAccepted = false;
						setState(ST_NEGOTIATION_FINISHED);
					}
				}
			}
		}
		break;
	case ST_NEGOTIATION_FINISHED:
	{

		break;
	}

	default:;
	}
}

void MCP::stop()
{
	// TODO: Destroy the underlying search hierarchy (UCP->MCP->UCP->...)
	destroyChildUCP();
	destroy();
}

void MCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::ReturnMCCsForItem:
		if (state() == ST_REQUESTING_MCCs)
		{
			// Read the packet
			PacketReturnMCCsForItem packetData;
			packetData.Read(stream);

			// Log the returned MCCs
			for (auto &mccdata : packetData.mccAddresses)
			{
				uint16_t agentId = mccdata.agentId;
				const std::string &hostIp = mccdata.hostIP;
				uint16_t hostPort = mccdata.hostPort;
				//iLog << " - MCC: " << agentId << " - host: " << hostIp << ":" << hostPort;
			}

			// Store the returned MCCs from YP
			_mccRegisters.swap(packetData.mccAddresses);
			int her = _mccRegisters.size();
			iLog << " - size: " << her;


			// Select the first MCC to negociate
			_mccRegisterIndex = 0;
			setState(ST_ITERATING_OVER_MCCs);

			socket->Disconnect();
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::ReturnMCCsForItem was unexpected.";
		}
		break;

	// TODO: Handle other packets
	case PacketType::RequestNegotiationResponse:
	{
		if (state() == ST_WAITING_ACCEPTANCE)
		{
			// Read the packet
			PacketRequestNegotiation packetData;
			packetData.Read(stream);

			if (packetData.accepted)
			{
				iLog << " - MPC: (" << id() << ") connected with MCC: (" << packetHeader.srcAgentId << ")";
				_totalSearch++;
				createChildUCP(packetData.uccLocation);
				setState(ST_NEGOTIATING);
			}
			else
			{
				_totalSearch++;
				// MCP "BUSY".
				_mccRegisterIndex++;
				setState(ST_ITERATING_OVER_MCCs);
			}
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::RequestNegotiationResponse was unexpected";
		}
		break;
	}
	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool MCP::negotiationFinished() const
{
	return state() == ST_NEGOTIATION_FINISHED;
}

bool MCP::negotiationAgreement() const
{
	return NegotiationAccepted(); // TODO: Did the child UCP find a solution?
}

bool MCP::Requestnegotiation()
{
	if (_mccRegisterIndex < _mccRegisters.size())
	{
		// Create message header and data
		PacketHeader packetHead;
		packetHead.packetType = PacketType::RequestNegotiation;
		packetHead.srcAgentId = id();
		packetHead.dstAgentId = _mccRegisters[_mccRegisterIndex].agentId;

		// Serialize message
		OutputMemoryStream stream;
		packetHead.Write(stream);

		setState(ST_WAITING_ACCEPTANCE);
		return sendPacketToAgent(_mccRegisters[_mccRegisterIndex].hostIP, _mccRegisters[_mccRegisterIndex].hostPort, stream);
	}
	else
	{
		setState(ST_NEGOTIATION_FINISHED);
		return false;
	}
}


bool MCP::queryMCCsForItem(int itemId)
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::QueryMCCsForItem;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketQueryMCCsForItem packetData;
	packetData.itemId = _requestedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	// 1) Ask YP for MCC hosting the item 'itemId'
	return sendPacketToYellowPages(stream);
}

void MCP::createChildUCP(const AgentLocation& location)
{
	//UCP CREATION
	Node* newNode = new Node(App->agentContainer->allAgents().size());
	_ucp = App->agentContainer->createUCP(newNode, requestedItemId(), contributedItemId(), location, searchDepth(), totalSearch());
}

void MCP::destroyChildUCP()
{
	if(_ucp != nullptr)
		_ucp->stop();
}
